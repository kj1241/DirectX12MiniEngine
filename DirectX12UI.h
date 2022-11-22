#pragma once
#include "DirectX12Function.h"
#include "BMFontUI.h"

class DirectX12UI
{
public:
    DirectX12UI(int FrameCount);
    ~DirectX12UI();

    void LoadAssets(std::wstring assetsPath, ComPtr<ID3D12Device> directX12_device, ComPtr<ID3D12RootSignature> directX12_rootSignature);
    void LoadVertextBuffer(ComPtr<ID3D12Device> directX12_device, ComPtr<ID3D12GraphicsCommandList> directX12_commandList, ComPtr <ID3D12DescriptorHeap> directX12_dsvHeap);
    void LoadVertextBufferView();
    void UpdatePipline(ComPtr<ID3D12GraphicsCommandList> directX12_commandList, ComPtr<ID3D12DescriptorHeap> directX12_dsvHeap);

private:
    struct TextVertex { //���ؽ� �ؽ���
        XMFLOAT4 pos;
        XMFLOAT4 texCoord;
        XMFLOAT4 color;
    };

    int FrameCount=0; //������ ī���� �����
    int maxNumTextCharacters = 1024;

    ComPtr<ID3D12PipelineState> directX12_textPSO;
    std::vector<ComPtr<ID3D12Resource>> directX12_textVertexBuffer;
    std::vector<D3D12_VERTEX_BUFFER_VIEW> directX12_textVertexBufferView; //���ý����۸� ������ ����ŭ ����ϱ�����
    std::vector< UINT8*> directX12_textVBGPUAddress; //�������

    BMFontUI BMFont;
    UINT srvHandleSize;

    void RenderText(ID3D12GraphicsCommandList* directX12_commandList, ID3D12DescriptorHeap* directX12_dsvHeap, BMFontUI font, std::wstring text, XMFLOAT2 pos, XMFLOAT2 scale = XMFLOAT2(1.0f, 1.0f), XMFLOAT2 padding = XMFLOAT2(0.5f, 0.0f), XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

};



