#pragma once
#include "DirectX12Base.h"

class DirectX12EnginePipline : public DirectX12Base
{
public:
    DirectX12EnginePipline(UINT width, UINT height, std::wstring name); //생성자
    ~DirectX12EnginePipline(); //소멸자

    virtual void OnInit();  //초기화
    virtual void OnUpdate();  //업데이트
    virtual void OnRender();  //랜더러
    virtual void OnDestroy();  //파괴할떄

private:
    static const UINT FrameCount = 2; //프레임 카운트

    // 파이프라인 객체
    ComPtr<IDXGISwapChain3> directX12_swapChain;  //스왑체인 백버퍼 -> 프론트버퍼
    ComPtr<ID3D12Device> directX12_device;  //디바이스
    ComPtr<ID3D12Resource> directX12_renderTargets[FrameCount];  //랜더타겟(프레임카운트)
    ComPtr<ID3D12CommandAllocator> directX12_commandAllocator;  //커멘드 할당
    ComPtr<ID3D12CommandQueue> directX12_commandQueue; //커맨드 큐
    ComPtr<ID3D12DescriptorHeap> directX12_rtvHeap; //rtv 힙
    ComPtr<ID3D12DescriptorHeap> directX12_dsvHeap; // dsv 힙
    ComPtr<ID3D12PipelineState> directX12_pipelineState; // 파이프라인 스테이트
    ComPtr<ID3D12GraphicsCommandList> directX12_commandList; // 커멘드 리스트
    UINT directX12_rtvDescriptorSize = 0; // rtv 서술자 사이즈
    UINT directX12_dsvDescriptorSize = 0; // dsv 서술자 사이즈

    // 동기화 객체
    UINT directX12_frameIndex = 0;  //프레임 인댁스
    HANDLE directX12_fenceEvent = nullptr;  //팬스 이벤트
    ComPtr<ID3D12Fence> directX12_fence; // 팬스
    UINT64 directX12_fenceValue = 0; // 팬스 값

    //싱글톤
    static DirectX12EnginePipline* s_app;

    void LoadPipeline();  //파이프라인 로드
    void LoadAssets();  //파이프라인 에셋
    void PopulateCommandList(); // 커멘드 리스트 마들기
    void WaitForPreviousFrame(); //이전 프레임 대기(나중에 지울꺼)
};
