#pragma once
#include "DirectX12Base.h"

class DirectX12EnginePipline : public DirectX12Base
{
public:
    DirectX12EnginePipline(UINT width, UINT height, std::wstring name);
    ~DirectX12EnginePipline();

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();

private:
    static const UINT FrameCount = 2;

    // ∆ƒ¿Ã«¡∂Û¿Œ ∞¥√º
    ComPtr<IDXGISwapChain3> directX12_swapChain;
    ComPtr<ID3D12Device> directX12_device;
    ComPtr<ID3D12Resource> directX12_renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> directX12_commandAllocator;
    ComPtr<ID3D12CommandQueue> directX12_commandQueue;
    ComPtr<ID3D12DescriptorHeap> directX12_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> directX12_dsvHeap;
    ComPtr<ID3D12PipelineState> directX12_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> directX12_commandList;
    UINT directX12_rtvDescriptorSize = 0;
    UINT directX12_dsvDescriptorSize = 0;

    // µø±‚»≠ ∞¥√º
    UINT directX12_frameIndex = 0;
    HANDLE directX12_fenceEvent = nullptr;
    ComPtr<ID3D12Fence> directX12_fence;
    UINT64 directX12_fenceValue = 0;

    //ΩÃ±€≈Ê
    static DirectX12EnginePipline* s_app;

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();
};
