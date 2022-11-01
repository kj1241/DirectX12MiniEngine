#include "stdafx.h"
#include "DirectX12EnginePipline.h"

//������
DirectX12EnginePipline::DirectX12EnginePipline(UINT width, UINT height, std::wstring name):DirectX12Base(width, height, name),directX12_frameIndex(0),directX12_rtvDescriptorSize(0), directX12_fenceValue(0)
{
}

//�Ҹ���
DirectX12EnginePipline::~DirectX12EnginePipline()
{
}

//�ʱ�ȭ
void DirectX12EnginePipline::OnInit()
{
	LoadPipeline(); //���������� �ε�
	LoadAssets(); //���� �ε�
}

//������Ʈ
void DirectX12EnginePipline::OnUpdate()
{
}

//������
void DirectX12EnginePipline::OnRender()
{
    //����� �������ϴµ� �ʿ��� ��� ���� ����� ���
    PopulateCommandList();

    //Ŀ�ǵ� ����Ʈ�� ����
    ID3D12CommandList* ppCommandLists[] = { directX12_commandList.Get() };
    directX12_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // ������ ����
    ThrowIfFailed(directX12_swapChain->Present(1, 0));

    WaitForPreviousFrame();
}

void DirectX12EnginePipline::OnDestroy()
{
    //gpu�� ���̻� ���ҽ��� �������� �ʴ��� Ȯ�� 
    WaitForPreviousFrame();
    CloseHandle(directX12_fenceEvent);
}

void DirectX12EnginePipline::LoadPipeline()  //���������� �ε�
{
    UINT dxgiFactoryFlags = 0; //dxgi ���丮 �÷���

#if defined(_DEBUG) || defined(_DEBUG) 
    // ����� ���̾� Ȱ�� (�� �� ���/������ ���/�׷��ȵ���)
    // ��ġ���� ����� ���� Ȱ�� Ȱ����ġ ��ȿ
    {
        ComPtr<ID3D12Debug> debugController; //����� ��Ʈ�ѷ�
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer(); //����� ���̾� Ȱ��
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG; //�߰� ����� ���̾� Ȱ��
        }
    }
#endif
  
    ComPtr<IDXGIFactory4> factory; //���丮
    //ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory))); //���丮 ����� dxgi.dll ��������

    if (directX12_useWarpDevice) //WARP ��ġ�� ��ü�մϴ�.
    {
        ComPtr<IDXGIAdapter> warpAdapter; // �����Ͷ����� Windows Advanced Rasterization Platform ���̴� ��� ������
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter))); //���

        // �ϵ���� ��ġ �� �õ�
        ThrowIfFailed(D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,  //����
            IID_PPV_ARGS(&directX12_device)
        ));
    }
    else //�ƴϸ� �ϵ���� ���� ��ü
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter); //�ϵ���� ���

        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0, //����
            IID_PPV_ARGS(&directX12_device)
        ));
    }

    //���� ��⿭��(ť)�� �����ϰ� ����
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ThrowIfFailed(directX12_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&directX12_commandQueue)));

    //���� ü���� �����ϰ� ����
    ComPtr<IDXGISwapChain1> swapChain; 
    swapChain.Reset(); //�⺻������ ���� �� �ִ°�쵵 �־ �����ѹ� ����

    //����ü�� ����
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;  //���� ����
    swapChainDesc.Width = directX12_width;
    swapChainDesc.Height = directX12_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        directX12_commandQueue.Get(),        //����ü���� �̹����� ó���Ҷ� ������ ������ �Ǳ�� �÷��ø� �ؾߵǱ⿡ ��⿭�� �ʿ��ϴ�.
        WinAPI::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    ));

    // ��üȭ�� ���� ���Ҳ��� ������.
    ThrowIfFailed(factory->MakeWindowAssociation(WinAPI::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain.As(&directX12_swapChain));
    directX12_frameIndex = directX12_swapChain->GetCurrentBackBufferIndex();

    // ������ ���� ���� (cpu ���� ������ ����)
    {
        // ���� ��󺸱� ������ �� ����
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {}; //render target view �� 
        rtvHeapDesc.NumDescriptors = FrameCount;  //����ü�� ���� =������
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NodeMask = 0;
        ThrowIfFailed(directX12_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&directX12_rtvHeap)));

        directX12_rtvDescriptorSize = directX12_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc; //depth stencil veiew �� 
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        dsvHeapDesc.NodeMask = 0;
        ThrowIfFailed(directX12_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&directX12_dsvHeap)));

        directX12_dsvDescriptorSize = directX12_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    }

    // ������ ���ҽ��� ����
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(directX12_rtvHeap->GetCPUDescriptorHandleForHeapStart()); //cpu ���� �ּ� ������ ����
        //�� �����ӿ� ���� rtv�� ����
        for (UINT i = 0; i < FrameCount; ++i)
        {
            ThrowIfFailed(directX12_swapChain->GetBuffer(i, IID_PPV_ARGS(&directX12_renderTargets[i])));
            directX12_device->CreateRenderTargetView(directX12_renderTargets[i].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, directX12_rtvDescriptorSize);
        }
    }
    ThrowIfFailed(directX12_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&directX12_commandAllocator)));
}

void DirectX12EnginePipline::LoadAssets()
{
    // Ŀ�ǵ� ����Ʈ�� ����
    ThrowIfFailed(directX12_device->CreateCommandList(
        0, 
        D3D12_COMMAND_LIST_TYPE_DIRECT, 
        directX12_commandAllocator.Get(),  // ���� ���� �Ҵ���
        nullptr, //�ʱ� ���������� ���� ������Ʈ
        IID_PPV_ARGS(&directX12_commandList)));

    // Ŀ�ǵ� ����Ʈ�� ���ڵ� ���¿��� ���������� �ƹ��͵� �������� ���� 
    // ���ڵ� ���¿��� �������� ���������� ���� ����.(�缳�� �ؾߵ�)
    ThrowIfFailed(directX12_commandList->Close());

    // ����ȭ ��ü ����
    {
        ThrowIfFailed(directX12_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&directX12_fence)));
        directX12_fenceValue = 1;

        // ������ ����ȭ�� ����� �̺�Ʈ �ڵ��� ����
        directX12_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (directX12_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }
    }
}

void DirectX12EnginePipline::PopulateCommandList()
{
    // Ŀ�ǵ� ����Ʈ�� ����� ��츸 �缳�� ����
    // Ŀ��� ����Ʈ�� GPUó���� �Ϸ��������� ���� ���� 
    // GPUó���� ���� �ҽ� ����
    ThrowIfFailed(directX12_commandAllocator->Reset());

    // Ư�����ɿ��� ExecuteCommandList() ȣ��Ǹ�, �ش� Ŀ�ǵ� ����Ʈ�� �缳�� �ؾ� ������ �ٽ� ���ڵ�
    ThrowIfFailed(directX12_commandList->Reset(directX12_commandAllocator.Get(), directX12_pipelineState.Get()));

    // ����۰� ���� Ÿ������ ����
    directX12_commandList->ResourceBarrier(1, &keep(CD3DX12_RESOURCE_BARRIER::Transition(directX12_renderTargets[directX12_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(directX12_rtvHeap->GetCPUDescriptorHandleForHeapStart(), directX12_frameIndex, directX12_rtvDescriptorSize); //cpu �����ּ� ������ ����

    // ������ ���
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    directX12_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // ����ۿ��� �ִ� ������ ȭ������ �ѷ���
    directX12_commandList->ResourceBarrier(1, &keep(CD3DX12_RESOURCE_BARRIER::Transition(directX12_renderTargets[directX12_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)));

    ThrowIfFailed(directX12_commandList->Close());
}

void DirectX12EnginePipline::WaitForPreviousFrame()
{
    //�Ϸ��Ҷ� ���� ��ٸ�

    // �ñ׳� �����鼭 �ҽ��� ����
    const UINT64 fence = directX12_fenceValue;
    ThrowIfFailed(directX12_commandQueue->Signal(directX12_fence.Get(), fence));
    directX12_fenceValue++;

    // ������ �Ϸ�ɶ����� ��ٸ�
    if (directX12_fence->GetCompletedValue() < fence)
    {
        ThrowIfFailed(directX12_fence->SetEventOnCompletion(fence, directX12_fenceEvent));
        WaitForSingleObject(directX12_fenceEvent, INFINITE);
    }

    directX12_frameIndex = directX12_swapChain->GetCurrentBackBufferIndex(); //���� ���� �ε����� ��ȣ�� �ٲ���
}