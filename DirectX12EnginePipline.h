#pragma once
#include "DirectX12Base.h"

class DirectX12EnginePipline : public DirectX12Base
{
public:
    DirectX12EnginePipline(UINT width, UINT height, std::wstring name); //������
    ~DirectX12EnginePipline(); //�Ҹ���

    virtual void OnInit();  //�ʱ�ȭ
    virtual void OnUpdate();  //������Ʈ
    virtual void OnRender();  //������
    virtual void OnDestroy();  //�ı��ҋ�

private:
    static const UINT FrameCount = 2; //������ ī��Ʈ

    //���ý� ����
    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT4 color;
    };

    // ���������� ��ü
    CD3DX12_VIEWPORT directX12_viewport;
    CD3DX12_RECT directX12_scissorRect;

    ComPtr<IDXGISwapChain3> directX12_swapChain;  //����ü�� ����� -> ����Ʈ����
    ComPtr<ID3D12Device> directX12_device;  //����̽�
    ComPtr<ID3D12Resource> directX12_renderTargets[FrameCount];  //����Ÿ��(������ī��Ʈ)
    ComPtr<ID3D12CommandAllocator> directX12_commandAllocator[FrameCount];  //Ŀ��� �Ҵ�
    ComPtr<ID3D12CommandAllocator> directX12_bundleAllocator; //���� �Ҵ�
    ComPtr<ID3D12CommandQueue> directX12_commandQueue; //Ŀ�ǵ� ť
    ComPtr<ID3D12RootSignature> directX12_rootSignature; //��Ʈ ����
    ComPtr<ID3D12DescriptorHeap> directX12_rtvHeap; //rtv ��
    ComPtr<ID3D12DescriptorHeap> directX12_dsvHeap; // dsv ��
    ComPtr<ID3D12PipelineState> directX12_pipelineState; // ���������� ������Ʈ
    ComPtr<ID3D12GraphicsCommandList> directX12_commandList; // Ŀ��� ����Ʈ
    ComPtr<ID3D12GraphicsCommandList> directX12_bundle;  // ���� �����
    UINT directX12_rtvDescriptorSize = 0; // rtv ������ ������
    UINT directX12_dsvDescriptorSize = 0; // dsv ������ ������

    // �� ���ҽ�
    ComPtr<ID3D12Resource> directX12_vertexBuffer;  
    D3D12_VERTEX_BUFFER_VIEW directX12_vertexBufferView={};

    // ����ȭ ��ü
    UINT directX12_frameIndex = 0;  //������ �δ콺
    HANDLE directX12_fenceEvent = nullptr;  //�ҽ� �̺�Ʈ
    ComPtr<ID3D12Fence> directX12_fence; // �ҽ�
    UINT64 directX12_fenceValue[FrameCount]; // �ҽ� ��

    //�̱���
    static DirectX12EnginePipline* s_app;

    void LoadPipeline();  //���������� �ε�
    void LoadAssets();  //���������� ����
    void PopulateCommandList(); // Ŀ��� ����Ʈ �����
    void WaitForPreviousFrame(); //���� ������ ���(���߿� ���ﲨ)
    void MoveToNextFrame();
    void WaitForGpu();
};
