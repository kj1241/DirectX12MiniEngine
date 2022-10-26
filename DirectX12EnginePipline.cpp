#include "stdafx.h"
#include "DirectX12EnginePipline.h"

DirectX12EnginePipline::DirectX12EnginePipline(UINT width, UINT height, std::wstring name):DirectX12Base(width, height, name),directX12_frameIndex(0),directX12_rtvDescriptorSize(0), directX12_fenceValue(0)
{

}

DirectX12EnginePipline::~DirectX12EnginePipline()
{
}

void DirectX12EnginePipline::OnInit()
{
	LoadPipeline();
	LoadAssets();
}

void DirectX12EnginePipline::OnUpdate()
{
}

void DirectX12EnginePipline::OnRender()
{
    //장면을 랜더링하는데 필요한 모든 명령 목록을 기록
    PopulateCommandList();

    //커맨드 리스트를 실행
    ID3D12CommandList* ppCommandLists[] = { directX12_commandList.Get() };
    directX12_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // 프레임 제시
    ThrowIfFailed(directX12_swapChain->Present(1, 0));

    WaitForPreviousFrame();
}

void DirectX12EnginePipline::OnDestroy()
{
    //gpu가 더이상 리소스를 참조하지 않는지 확인 
    WaitForPreviousFrame();
    CloseHandle(directX12_fenceEvent);
}

void DirectX12EnginePipline::LoadPipeline()
{
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG) || defined(_DEBUG) 
    // 디버그 레이어 활설 (앱 및 기능/선택적 기능/그래픽도구)
    // 장치설정 디버그 계층 활성 활성자치 무효
    {
        ComPtr<ID3D12Debug> debugController; //디버그 컨트롤러
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG; //추가 디버그 레이어 활설
        }
    }
#endif
  
   

    ComPtr<IDXGIFactory4> factory;
    //ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    if (directX12_useWarpDevice) //WARP 장치로 대체합니다.
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

        // 하드웨어 장치 및 시도
        ThrowIfFailed(D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,  //레벨
            IID_PPV_ARGS(&directX12_device)
        ));
    }
    else //아니면 하드웨어 장비로 대체
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);

        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0, //레벨
            IID_PPV_ARGS(&directX12_device)
        ));
    }

    //명령 대기열으(큐)를 정의하고 생성
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ThrowIfFailed(directX12_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&directX12_commandQueue)));

    //스왑 체인을 정의하고 생성
    ComPtr<IDXGISwapChain1> swapChain;
    swapChain.Reset(); //기본쓰레기 값이 들어가 있는경우도 있어서 리셋한번 해줌

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = directX12_width;
    swapChainDesc.Height = directX12_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        directX12_commandQueue.Get(),        //스왑체인은 이미지를 처리할때 강제로 비워줘야 되기는 플러시를 해야되기에 대기열이 필요하다.
        WinAPI::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    ));

    // 전체화면 지원 안할꺼기 때문에.
    ThrowIfFailed(factory->MakeWindowAssociation(WinAPI::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain.As(&directX12_swapChain));
    directX12_frameIndex = directX12_swapChain->GetCurrentBackBufferIndex();

    // 설명자 힙을 생성
    {
        // 랜더 대상보기 설명자 힙 생성
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;  //스왑체인 버퍼 =프레임
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NodeMask = 0;
        ThrowIfFailed(directX12_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&directX12_rtvHeap)));

        directX12_rtvDescriptorSize = directX12_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        dsvHeapDesc.NodeMask = 0;
        ThrowIfFailed(directX12_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&directX12_dsvHeap)));

        directX12_dsvDescriptorSize = directX12_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    }

    // 프레임 리소스를 생성
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(directX12_rtvHeap->GetCPUDescriptorHandleForHeapStart());
        //각 프레임에 대한 rtv를 생성
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
    // 커맨드 리스트을 생성
    ThrowIfFailed(directX12_device->CreateCommandList(
        0, 
        D3D12_COMMAND_LIST_TYPE_DIRECT, 
        directX12_commandAllocator.Get(),  // 관련 명령 할당자
        nullptr, //초기 파이프라인 상태 오브젝트
        IID_PPV_ARGS(&directX12_commandList)));

    // 커맨드 리스트는 레코딩 상태에서 생성되지만 아무것도 존재하지 않음 
    // 레코드 상태에서 닫힐것을 예상함으로 지금 닫음.
    // 이것은 재설정하고 닫아야 하는 것
    ThrowIfFailed(directX12_commandList->Close());

    // 동기화 객체 생성
    {
        ThrowIfFailed(directX12_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&directX12_fence)));
        directX12_fenceValue = 1;

        // 프레임 동기화에 사용할 이벤트 핸들을 생성
        directX12_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (directX12_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }
    }
}

void DirectX12EnginePipline::PopulateCommandList()
{
    // 커맨드 리스트는 연결된 경우만 재설정 가능
    // 커멘드 리스트는 GPU처리를 완료했음으로 앱을 실행 
    // GPU처리를 위해 팬스 설정
    ThrowIfFailed(directX12_commandAllocator->Reset());

    // 특정명령에서 ExecuteCommandList() 호출되면, 해당 커맨드 리스트는 재설정 해야 함으로 다시 레코딩
    ThrowIfFailed(directX12_commandList->Reset(directX12_commandAllocator.Get(), directX12_pipelineState.Get()));

    // 백버퍼가 랜더 타겟으로 사용됨
    directX12_commandList->ResourceBarrier(1, &keep(CD3DX12_RESOURCE_BARRIER::Transition(directX12_renderTargets[directX12_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(directX12_rtvHeap->GetCPUDescriptorHandleForHeapStart(), directX12_frameIndex, directX12_rtvDescriptorSize);

    // 명령을 기록
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    directX12_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // 백버퍼에서 있던 내용을 화면으로 뿌려줌
    directX12_commandList->ResourceBarrier(1, &keep(CD3DX12_RESOURCE_BARRIER::Transition(directX12_renderTargets[directX12_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)));

    ThrowIfFailed(directX12_commandList->Close());
}

void DirectX12EnginePipline::WaitForPreviousFrame()
{
    //완료할때 까지 기다림

    // 시그널 보내면서 팬스값 증가
    const UINT64 fence = directX12_fenceValue;
    ThrowIfFailed(directX12_commandQueue->Signal(directX12_fence.Get(), fence));
    directX12_fenceValue++;

    // 프레임 완료될때까지 기다림
    if (directX12_fence->GetCompletedValue() < fence)
    {
        ThrowIfFailed(directX12_fence->SetEventOnCompletion(fence, directX12_fenceEvent));
        WaitForSingleObject(directX12_fenceEvent, INFINITE);
    }

    directX12_frameIndex = directX12_swapChain->GetCurrentBackBufferIndex();
}
