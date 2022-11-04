#include "stdafx.h"
#include "DirectX12EnginePipline.h"

//생성자
DirectX12EnginePipline::DirectX12EnginePipline(UINT width, UINT height, std::wstring name):DirectX12Base(width, height, name),
directX12_frameIndex(0),
directX12_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
directX12_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
directX12_rtvDescriptorSize(0), 
directX12_fenceValue{} //directX12_fenceValue(0)-> directX12_fenceValue{} 배열로 변경
{
}

//소멸자
DirectX12EnginePipline::~DirectX12EnginePipline()
{
}

//초기화
void DirectX12EnginePipline::OnInit()
{
	LoadPipeline(); //파이프라인 로드
	LoadAssets(); //에셋 로드
}

//업데이트
void DirectX12EnginePipline::OnUpdate()
{
}

//랜더링
void DirectX12EnginePipline::OnRender()
{
    //장면을 랜더링하는데 필요한 모든 명령 목록을 기록
    PopulateCommandList();

    //커맨드 리스트를 실행
    ID3D12CommandList* ppCommandLists[] = { directX12_commandList.Get() };
    directX12_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // 프레임 제시
    ThrowIfFailed(directX12_swapChain->Present(1, 0));

    //다음 프레임으로 이동
    //WaitForPreviousFrame();
    MoveToNextFrame();
}

void DirectX12EnginePipline::OnDestroy()
{
    //gpu가 더이상 리소스를 참조하지 않는지 확인 
    WaitForPreviousFrame();
    CloseHandle(directX12_fenceEvent);
}

void DirectX12EnginePipline::LoadPipeline()  //파이프라인 로드
{
    UINT dxgiFactoryFlags = 0; //dxgi 팩토리 플레그

#if defined(_DEBUG) || defined(_DEBUG) 
    // 디버그 레이어 활설 (앱 및 기능/선택적 기능/그래픽도구)
    // 장치설정 디버그 계층 활성 활성자치 무효
    {
        ComPtr<ID3D12Debug> debugController; //디버그 컨트롤러
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer(); //디버깅 레이어 활성
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG; //추가 디버그 레이어 활설
        }
    }
#endif
  
    ComPtr<IDXGIFactory4> factory; //팩토리
    //ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory))); //팩토리 만들기 dxgi.dll 가저오기

    if (directX12_useWarpDevice) //WARP 장치로 대체합니다.
    {
        ComPtr<IDXGIAdapter> warpAdapter; // 레스터라이즈 Windows Advanced Rasterization Platform 세이더 기반 렌더링
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter))); //어뎁터

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
        GetHardwareAdapter(factory.Get(), &hardwareAdapter); //하드웨어 어뎁터

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

    //스왑체인 정의
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;  //버퍼 갯수
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

    // 설명자 힙을 생성 (cpu 가상 공간에 생성)
    {
        // 랜더 대상보기 설명자 힙 생성
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {}; //render target view 힙 
        rtvHeapDesc.NumDescriptors = FrameCount;  //스왑체인 버퍼 =프레임
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NodeMask = 0;
        ThrowIfFailed(directX12_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&directX12_rtvHeap)));

        directX12_rtvDescriptorSize = directX12_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc; //depth stencil veiew 힙 
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        dsvHeapDesc.NodeMask = 0;
        ThrowIfFailed(directX12_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&directX12_dsvHeap)));

        directX12_dsvDescriptorSize = directX12_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    }

    // 프레임 리소스를 생성
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(directX12_rtvHeap->GetCPUDescriptorHandleForHeapStart()); //cpu 가상 주소 공간에 생성
        //각 프레임에 대한 rtv를 생성
        for (UINT i = 0; i < FrameCount; ++i)
        {
            ThrowIfFailed(directX12_swapChain->GetBuffer(i, IID_PPV_ARGS(&directX12_renderTargets[i])));
            directX12_device->CreateRenderTargetView(directX12_renderTargets[i].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, directX12_rtvDescriptorSize);

            ThrowIfFailed(directX12_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&directX12_commandAllocator[i])));
        }
    }

    ThrowIfFailed(directX12_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&directX12_bundleAllocator))); //번들 할당 함
}

void DirectX12EnginePipline::LoadAssets()
{
    // 비어있는 루트서명만들기
    {
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc; //루트서명 정의
        rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ThrowIfFailed(directX12_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&directX12_rootSignature)));
    }

    // 셰이더 컴파일 및 로드를 포함하는 파이프라인 만들기
    {
        ComPtr<ID3DBlob> vertexShader; //버텍스 쉐이더
        ComPtr<ID3DBlob> pixelShader;  //픽쉘세이더

#if defined(_DEBUG)
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"../../shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr)); //버텍스 셰이더에 hlsl VSMain 담기
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"../../shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));  //픽셀 셰이더 PSMain 담기

        // 정점 입력 레이아웃 정의
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }, //위치
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } //색상
        };

        // PSO(그래픽스 파이프라인 상태 오브젝트)를 설명하고 생성
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) }; //GPU 프론트 엔드 레이아웃
        psoDesc.pRootSignature = directX12_rootSignature.Get(); //루트 시그널 포인트
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get()); // 버텍스 셰이더
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get()); //픽셀 셰이더
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // 레스터 라이즈: 기본
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // 블랜드 상태: 기본
        psoDesc.DepthStencilState.DepthEnable = FALSE;  //뎁스: 불가
        psoDesc.DepthStencilState.StencilEnable = FALSE; //스텐실: 불가
        psoDesc.SampleMask = UINT_MAX; //블랜드 상태의 멀티 셈플링 32개(bit)로 최대
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; //기본도형의 구조(테셀레이터) 삼각형
        psoDesc.NumRenderTargets = 1; //랜더 갯수
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; //랜더 대상의 형틀
        psoDesc.SampleDesc.Count = 1; //멀티셈플림 표본 품질의 갯수 1개
        ThrowIfFailed(directX12_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&directX12_pipelineState))); //그래픽 파이프라인 상태 만들기
    }

   
    // 커맨드 리스트을 생성
    ThrowIfFailed(directX12_device->CreateCommandList(
        0, 
        D3D12_COMMAND_LIST_TYPE_DIRECT, 
        directX12_commandAllocator[directX12_frameIndex].Get(),  // 관련 명령 할당자
        directX12_pipelineState.Get(), //초기 파이프라인 상태 오브젝트(nullptr -> directX12_pipelineState.Get())
        IID_PPV_ARGS(&directX12_commandList)));

    // 커맨드 리스트는 레코딩 상태에서 생성되지만 아무것도 존재하지 않음 
    // 레코드 상태에서 닫힐것을 예상함으로 지금 닫음.(재설정 해야됨)
    ThrowIfFailed(directX12_commandList->Close());

    // 버텍스 버퍼 생성
    {
        // 삼각형의 지오메트릭의 결정
        Vertex triangleVertices[] =
        {
            { { 0.0f, 0.25f * directX12_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }, // x = 0 y =0.25 * 종횡비 맞추기  c = 빨강
            { { 0.25f, -0.25f * directX12_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }, // x =0.25 y = -0.25 *종횡비 맞추기 c = 그린
            { { -0.25f, -0.25f * directX12_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } } // x = - 0.25 y=-0.25 * 종횡비 맞추기 c = 블루
        };

        const UINT vertexBufferSize = sizeof(triangleVertices);

        // GPU가 필요할때 마다 업로드 힙이 마샬링되기 때문에 버텍스 버퍼와 같은 정적데이터는 업로드 힙을 이용해야합니다.
        ThrowIfFailed(directX12_device->CreateCommittedResource(
            &keep(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
            D3D12_HEAP_FLAG_NONE,
            &keep(CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize)),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&directX12_vertexBuffer)));

        // 삼각형 데이터를 버텍스 버퍼에 복사합니다.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);        // CPU에서 이 리소스를 읽지 않는다
        ThrowIfFailed(directX12_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
        directX12_vertexBuffer->Unmap(0, nullptr);

        // 버택스 버퍼 뷰어를 초기화함
        directX12_vertexBufferView.BufferLocation = directX12_vertexBuffer->GetGPUVirtualAddress();
        directX12_vertexBufferView.StrideInBytes = sizeof(Vertex);
        directX12_vertexBufferView.SizeInBytes = vertexBufferSize;
    }

    // 번들을 만들고 기록
    {
        ThrowIfFailed(directX12_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, directX12_bundleAllocator.Get(), directX12_pipelineState.Get(), IID_PPV_ARGS(&directX12_bundle)));
        directX12_bundle->SetGraphicsRootSignature(directX12_rootSignature.Get()); //그래픽스 루트 서명 설정
        //업데이트에 있는 리스트를 가저옴
        directX12_bundle->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);//기본 유형(점 선 면)데이터 순서
        directX12_bundle->IASetVertexBuffers(0, 1, &directX12_vertexBufferView);//버텍스 버퍼에 대한 CPU 핸들 설정
        directX12_bundle->DrawInstanced(3, 1, 0, 0); //인덱싱 되지 않은 인스턴스 프리미티브 그리기
        ThrowIfFailed(directX12_bundle->Close());
    }

    // 동기화 객체 생성후 GPU에 업로드 될떄까지 기다림
    {
        ThrowIfFailed(directX12_device->CreateFence(directX12_fenceValue[directX12_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&directX12_fence)));
        //directX12_fenceValue[directX12_frameIndex] = 1;
        directX12_fenceValue[directX12_frameIndex]++;

        // 프레임 동기화에 사용할 이벤트 핸들을 생성
        directX12_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (directX12_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        //명령 목록이 실행될때까지 기다림 루프는 완료하지만 설정이 완료될떄까지 기다립니다.
        //WaitForPreviousFrame();
        WaitForGpu();
    }
}

void DirectX12EnginePipline::PopulateCommandList()
{
    // 커맨드 리스트는 연결된 경우만 재설정 가능
    // 커멘드 리스트는 GPU처리를 완료했음으로 앱을 실행 
    // GPU처리를 위해 팬스 설정
    ThrowIfFailed(directX12_commandAllocator[directX12_frameIndex]->Reset());

    // 특정명령에서 ExecuteCommandList() 호출되면, 해당 커맨드 리스트는 재설정 해야 함으로 다시 레코딩
    ThrowIfFailed(directX12_commandList->Reset(directX12_commandAllocator[directX12_frameIndex].Get(), directX12_pipelineState.Get()));

    // 필요한 상태를 설정
    directX12_commandList->SetGraphicsRootSignature(directX12_rootSignature.Get()); //그래픽스 루트 서명 설정
    directX12_commandList->RSSetViewports(1, &directX12_viewport); // 뷰포트 설정
    directX12_commandList->RSSetScissorRects(1, &directX12_scissorRect); // 화면 자르는 크기 설정

    // 백버퍼가 랜더 타겟으로 사용됨
    directX12_commandList->ResourceBarrier(1, &keep(CD3DX12_RESOURCE_BARRIER::Transition(directX12_renderTargets[directX12_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(directX12_rtvHeap->GetCPUDescriptorHandleForHeapStart(), directX12_frameIndex, directX12_rtvDescriptorSize); //cpu 가상주소 공간에 생성
    directX12_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr); // 랜더 타겟 설정

    // 명령을 기록(입력어셈블 단계)
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    directX12_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr); // 랜더 타겟 뷰어 클리어
    directX12_commandList->ExecuteBundle(directX12_bundle.Get());     //번들의 지정된 명령을 실행

    // 백버퍼에서 있던 내용을 화면으로 뿌려줌
    directX12_commandList->ResourceBarrier(1, &keep(CD3DX12_RESOURCE_BARRIER::Transition(directX12_renderTargets[directX12_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)));

    ThrowIfFailed(directX12_commandList->Close());
}

//MoveToNextFrame사용으로 더이상 함수를 사용하지 않음
void DirectX12EnginePipline::WaitForPreviousFrame()
{
    ////완료할때 까지 기다림

    //// 시그널 보내면서 팬스값 증가
    //const UINT64 fence = directX12_fenceValue;
    //ThrowIfFailed(directX12_commandQueue->Signal(directX12_fence.Get(), fence));
    //directX12_fenceValue++;

    //// 프레임 완료될때까지 기다림
    //if (directX12_fence->GetCompletedValue() < fence)
    //{
    //    ThrowIfFailed(directX12_fence->SetEventOnCompletion(fence, directX12_fenceEvent));
    //    WaitForSingleObject(directX12_fenceEvent, INFINITE);
    //}

    //directX12_frameIndex = directX12_swapChain->GetCurrentBackBufferIndex(); //현재 백퍼 인덱스의 번호로 바꿔줌
}

// 보류죽인 GPU 작업이 완료될때까지 기다림
void DirectX12EnginePipline::WaitForGpu()
{
    // 커멘드 큐에서 시그널 신호를 예약
    ThrowIfFailed(directX12_commandQueue->Signal(directX12_fence.Get(), directX12_fenceValue[directX12_frameIndex]));

    // 펜스가 처리될때까지 기다림
    ThrowIfFailed(directX12_fence->SetEventOnCompletion(directX12_fenceValue[directX12_frameIndex], directX12_fenceEvent));
    WaitForSingleObjectEx(directX12_fenceEvent, INFINITE, FALSE);

    // 처리가 완료되면 현재 프레임의 팬스 값을증가시킴
    directX12_fenceValue[directX12_frameIndex]++;
}

// 다음 프레임 랜더링을 준비
void DirectX12EnginePipline::MoveToNextFrame()
{
    // 대기열에서 커멘드 큐에 시그널 명령을 예약
    const UINT64 currentFenceValue = directX12_fenceValue[directX12_frameIndex]; //현재 값을 정의
    ThrowIfFailed(directX12_commandQueue->Signal(directX12_fence.Get(), currentFenceValue)); //핸재 값을 대기열에 예약

    // 프레임 인덱스를 업데이트
    directX12_frameIndex = directX12_swapChain->GetCurrentBackBufferIndex(); //백버퍼의 마지막 버퍼를 프레임인덱스로

    // 아직 프레임 랜더링할 준비가 안되있어 있다면 기다리기
    if (directX12_fence->GetCompletedValue() < directX12_fenceValue[directX12_frameIndex])
    {
        ThrowIfFailed(directX12_fence->SetEventOnCompletion(directX12_fenceValue[directX12_frameIndex], directX12_fenceEvent));
        WaitForSingleObjectEx(directX12_fenceEvent, INFINITE, FALSE);
    }

    // 다음 프레임의 팬스값을 설정
    directX12_fenceValue[directX12_frameIndex] = currentFenceValue + 1;
}