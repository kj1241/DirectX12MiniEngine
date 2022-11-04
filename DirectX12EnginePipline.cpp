#include "stdafx.h"
#include "DirectX12EnginePipline.h"

//������
DirectX12EnginePipline::DirectX12EnginePipline(UINT width, UINT height, std::wstring name):DirectX12Base(width, height, name),
directX12_frameIndex(0),
directX12_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
directX12_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
directX12_rtvDescriptorSize(0), 
directX12_fenceValue{} //directX12_fenceValue(0)-> directX12_fenceValue{} �迭�� ����
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
    //����� �������ϴµ� �ʿ��� ��� ��� ����� ���
    PopulateCommandList();

    //Ŀ�ǵ� ����Ʈ�� ����
    ID3D12CommandList* ppCommandLists[] = { directX12_commandList.Get() };
    directX12_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // ������ ����
    ThrowIfFailed(directX12_swapChain->Present(1, 0));

    //���� ���������� �̵�
    //WaitForPreviousFrame();
    MoveToNextFrame();
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

    //��� ��⿭��(ť)�� �����ϰ� ����
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

            ThrowIfFailed(directX12_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&directX12_commandAllocator[i])));
        }
    }

    ThrowIfFailed(directX12_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&directX12_bundleAllocator))); //���� �Ҵ� ��
}

void DirectX12EnginePipline::LoadAssets()
{
    // ����ִ� ��Ʈ�������
    {
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc; //��Ʈ���� ����
        rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ThrowIfFailed(directX12_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&directX12_rootSignature)));
    }

    // ���̴� ������ �� �ε带 �����ϴ� ���������� �����
    {
        ComPtr<ID3DBlob> vertexShader; //���ؽ� ���̴�
        ComPtr<ID3DBlob> pixelShader;  //�Ƚ����̴�

#if defined(_DEBUG)
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"../../shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr)); //���ؽ� ���̴��� hlsl VSMain ���
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"../../shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));  //�ȼ� ���̴� PSMain ���

        // ���� �Է� ���̾ƿ� ����
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }, //��ġ
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } //����
        };

        // PSO(�׷��Ƚ� ���������� ���� ������Ʈ)�� �����ϰ� ����
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) }; //GPU ����Ʈ ���� ���̾ƿ�
        psoDesc.pRootSignature = directX12_rootSignature.Get(); //��Ʈ �ñ׳� ����Ʈ
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get()); // ���ؽ� ���̴�
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get()); //�ȼ� ���̴�
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // ������ ������: �⺻
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // ���� ����: �⺻
        psoDesc.DepthStencilState.DepthEnable = FALSE;  //����: �Ұ�
        psoDesc.DepthStencilState.StencilEnable = FALSE; //���ٽ�: �Ұ�
        psoDesc.SampleMask = UINT_MAX; //���� ������ ��Ƽ ���ø� 32��(bit)�� �ִ�
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; //�⺻������ ����(�׼�������) �ﰢ��
        psoDesc.NumRenderTargets = 1; //���� ����
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; //���� ����� ��Ʋ
        psoDesc.SampleDesc.Count = 1; //��Ƽ���ø� ǥ�� ǰ���� ���� 1��
        ThrowIfFailed(directX12_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&directX12_pipelineState))); //�׷��� ���������� ���� �����
    }

   
    // Ŀ�ǵ� ����Ʈ�� ����
    ThrowIfFailed(directX12_device->CreateCommandList(
        0, 
        D3D12_COMMAND_LIST_TYPE_DIRECT, 
        directX12_commandAllocator[directX12_frameIndex].Get(),  // ���� ��� �Ҵ���
        directX12_pipelineState.Get(), //�ʱ� ���������� ���� ������Ʈ(nullptr -> directX12_pipelineState.Get())
        IID_PPV_ARGS(&directX12_commandList)));

    // Ŀ�ǵ� ����Ʈ�� ���ڵ� ���¿��� ���������� �ƹ��͵� �������� ���� 
    // ���ڵ� ���¿��� �������� ���������� ���� ����.(�缳�� �ؾߵ�)
    ThrowIfFailed(directX12_commandList->Close());

    // ���ؽ� ���� ����
    {
        // �ﰢ���� ������Ʈ���� ����
        Vertex triangleVertices[] =
        {
            { { 0.0f, 0.25f * directX12_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }, // x = 0 y =0.25 * ��Ⱦ�� ���߱�  c = ����
            { { 0.25f, -0.25f * directX12_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // x =0.25 y = -0.25 *��Ⱦ�� ���߱� c = �׸�
            { { -0.25f, -0.25f * directX12_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } } // x = - 0.25 y=-0.25 * ��Ⱦ�� ���߱� c = ���
        };

        const UINT vertexBufferSize = sizeof(triangleVertices);

        // GPU�� �ʿ��Ҷ� ���� ���ε� ���� �������Ǳ� ������ ���ؽ� ���ۿ� ���� ���������ʹ� ���ε� ���� �̿��ؾ��մϴ�.
        ThrowIfFailed(directX12_device->CreateCommittedResource(
            &keep(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
            D3D12_HEAP_FLAG_NONE,
            &keep(CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize)),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&directX12_vertexBuffer)));

        // �ﰢ�� �����͸� ���ؽ� ���ۿ� �����մϴ�.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);        // CPU���� �� ���ҽ��� ���� �ʴ´�
        ThrowIfFailed(directX12_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
        directX12_vertexBuffer->Unmap(0, nullptr);

        // ���ý� ���� �� �ʱ�ȭ��
        directX12_vertexBufferView.BufferLocation = directX12_vertexBuffer->GetGPUVirtualAddress();
        directX12_vertexBufferView.StrideInBytes = sizeof(Vertex);
        directX12_vertexBufferView.SizeInBytes = vertexBufferSize;
    }

    // ������ ����� ���
    {
        ThrowIfFailed(directX12_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, directX12_bundleAllocator.Get(), directX12_pipelineState.Get(), IID_PPV_ARGS(&directX12_bundle)));
        directX12_bundle->SetGraphicsRootSignature(directX12_rootSignature.Get()); //�׷��Ƚ� ��Ʈ ���� ����
        //������Ʈ�� �ִ� ����Ʈ�� ������
        directX12_bundle->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);//�⺻ ����(�� �� ��)������ ����
        directX12_bundle->IASetVertexBuffers(0, 1, &directX12_vertexBufferView);//���ؽ� ���ۿ� ���� CPU �ڵ� ����
        directX12_bundle->DrawInstanced(3, 1, 0, 0); //�ε��� ���� ���� �ν��Ͻ� ������Ƽ�� �׸���
        ThrowIfFailed(directX12_bundle->Close());
    }

    // ����ȭ ��ü ������ GPU�� ���ε� �ɋ����� ��ٸ�
    {
        ThrowIfFailed(directX12_device->CreateFence(directX12_fenceValue[directX12_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&directX12_fence)));
        //directX12_fenceValue[directX12_frameIndex] = 1;
        directX12_fenceValue[directX12_frameIndex]++;

        // ������ ����ȭ�� ����� �̺�Ʈ �ڵ��� ����
        directX12_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (directX12_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        //��� ����� ����ɶ����� ��ٸ� ������ �Ϸ������� ������ �Ϸ�ɋ����� ��ٸ��ϴ�.
        //WaitForPreviousFrame();
        WaitForGpu();
    }
}

void DirectX12EnginePipline::PopulateCommandList()
{
    // Ŀ�ǵ� ����Ʈ�� ����� ��츸 �缳�� ����
    // Ŀ��� ����Ʈ�� GPUó���� �Ϸ��������� ���� ���� 
    // GPUó���� ���� �ҽ� ����
    ThrowIfFailed(directX12_commandAllocator[directX12_frameIndex]->Reset());

    // Ư����ɿ��� ExecuteCommandList() ȣ��Ǹ�, �ش� Ŀ�ǵ� ����Ʈ�� �缳�� �ؾ� ������ �ٽ� ���ڵ�
    ThrowIfFailed(directX12_commandList->Reset(directX12_commandAllocator[directX12_frameIndex].Get(), directX12_pipelineState.Get()));

    // �ʿ��� ���¸� ����
    directX12_commandList->SetGraphicsRootSignature(directX12_rootSignature.Get()); //�׷��Ƚ� ��Ʈ ���� ����
    directX12_commandList->RSSetViewports(1, &directX12_viewport); // ����Ʈ ����
    directX12_commandList->RSSetScissorRects(1, &directX12_scissorRect); // ȭ�� �ڸ��� ũ�� ����

    // ����۰� ���� Ÿ������ ����
    directX12_commandList->ResourceBarrier(1, &keep(CD3DX12_RESOURCE_BARRIER::Transition(directX12_renderTargets[directX12_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(directX12_rtvHeap->GetCPUDescriptorHandleForHeapStart(), directX12_frameIndex, directX12_rtvDescriptorSize); //cpu �����ּ� ������ ����
    directX12_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr); // ���� Ÿ�� ����

    // ����� ���(�Է¾���� �ܰ�)
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    directX12_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr); // ���� Ÿ�� ��� Ŭ����
    directX12_commandList->ExecuteBundle(directX12_bundle.Get());     //������ ������ ����� ����

    // ����ۿ��� �ִ� ������ ȭ������ �ѷ���
    directX12_commandList->ResourceBarrier(1, &keep(CD3DX12_RESOURCE_BARRIER::Transition(directX12_renderTargets[directX12_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)));

    ThrowIfFailed(directX12_commandList->Close());
}

//MoveToNextFrame������� ���̻� �Լ��� ������� ����
void DirectX12EnginePipline::WaitForPreviousFrame()
{
    ////�Ϸ��Ҷ� ���� ��ٸ�

    //// �ñ׳� �����鼭 �ҽ��� ����
    //const UINT64 fence = directX12_fenceValue;
    //ThrowIfFailed(directX12_commandQueue->Signal(directX12_fence.Get(), fence));
    //directX12_fenceValue++;

    //// ������ �Ϸ�ɶ����� ��ٸ�
    //if (directX12_fence->GetCompletedValue() < fence)
    //{
    //    ThrowIfFailed(directX12_fence->SetEventOnCompletion(fence, directX12_fenceEvent));
    //    WaitForSingleObject(directX12_fenceEvent, INFINITE);
    //}

    //directX12_frameIndex = directX12_swapChain->GetCurrentBackBufferIndex(); //���� ���� �ε����� ��ȣ�� �ٲ���
}

// �������� GPU �۾��� �Ϸ�ɶ����� ��ٸ�
void DirectX12EnginePipline::WaitForGpu()
{
    // Ŀ��� ť���� �ñ׳� ��ȣ�� ����
    ThrowIfFailed(directX12_commandQueue->Signal(directX12_fence.Get(), directX12_fenceValue[directX12_frameIndex]));

    // �潺�� ó���ɶ����� ��ٸ�
    ThrowIfFailed(directX12_fence->SetEventOnCompletion(directX12_fenceValue[directX12_frameIndex], directX12_fenceEvent));
    WaitForSingleObjectEx(directX12_fenceEvent, INFINITE, FALSE);

    // ó���� �Ϸ�Ǹ� ���� �������� �ҽ� ����������Ŵ
    directX12_fenceValue[directX12_frameIndex]++;
}

// ���� ������ �������� �غ�
void DirectX12EnginePipline::MoveToNextFrame()
{
    // ��⿭���� Ŀ��� ť�� �ñ׳� ����� ����
    const UINT64 currentFenceValue = directX12_fenceValue[directX12_frameIndex]; //���� ���� ����
    ThrowIfFailed(directX12_commandQueue->Signal(directX12_fence.Get(), currentFenceValue)); //���� ���� ��⿭�� ����

    // ������ �ε����� ������Ʈ
    directX12_frameIndex = directX12_swapChain->GetCurrentBackBufferIndex(); //������� ������ ���۸� �������ε�����

    // ���� ������ �������� �غ� �ȵ��־� �ִٸ� ��ٸ���
    if (directX12_fence->GetCompletedValue() < directX12_fenceValue[directX12_frameIndex])
    {
        ThrowIfFailed(directX12_fence->SetEventOnCompletion(directX12_fenceValue[directX12_frameIndex], directX12_fenceEvent));
        WaitForSingleObjectEx(directX12_fenceEvent, INFINITE, FALSE);
    }

    // ���� �������� �ҽ����� ����
    directX12_fenceValue[directX12_frameIndex] = currentFenceValue + 1;
}