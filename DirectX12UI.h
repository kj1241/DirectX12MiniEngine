#pragma once
#include "DirectX12Function.h"

struct TextVertex {
    XMFLOAT4 pos;
    XMFLOAT4 texCoord;
    XMFLOAT4 color;
};

struct FontChar
{
    int id; //�����ڵ� id  4byte

    //�ؽ�Ʈ ��ǥ ���� 
    float u; // u texture ��ǥ  8byte 
    float v; // v texture ��ǥ  12byte
    float twidth; // �ؽ�Ʈ ���� ����  16byte 
    float theight; // �ؽ�Ʈ ���� ����  20byte

    float width; //ȭ�� ��ǥ�� ���� ����  24byte
    float height; //ȭ�� ��ǥ�� ���� ����   28byte

    //�۲�ũ�⿡ ���� ����ȭ
    float xoffset; //���� Ŀ�� ũ�⿡�� ���� ���ʱ��� ������   32byte
    float yoffset; // ���� �������� ������ �� ���� ������    36byte
    float xadvance; //���� ���ڸ����ؼ� �̵��ؾߵǴ� ũ��  40byte
    float pading[2]; //����ȭ�� ���� 8byte  48byte
};

struct FontKerning
{
    int firstid; // ù��° ���� 4byte
    int secondid; // �ι�° ���� 8byte
    float amount; // �ι������ڸ� ���ϰų� ���� x 12byte
    float pading; // ����ȭ�� ���� 4����Ʈ 16byte
};

class FontUI
{
private:
    std::wstring name; //��Ʈ �̸�
    std::wstring fontImage;
    int size; //�۲�ũ�� ���γ��̴� (1,0)���
    float lineHeight; //�����ٷ� �󸶳� �̵��ؾ� �Ǵ���
    float baseHeight; //��� ���� ���̰� ����ȭ
    int textureWidth; //�۲� �ؽ��� �ʺ�
    int textureHeight; //�۲� �ؽ��� ����
    int numCharacters; //�۲� ���ڼ�
    FontChar* CharList; // ���� ���
    int numKernings; // Ŀ�� ��
    FontKerning* KerningsList; // Ŀ�� ���� ������ ���
    ID3D12Resource* textureBuffer; // �۲� �ؽ��� ���ҽ�
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandle; // �۲� srv

    //���� ü������ ��
    float leftpadding;
    float toppadding;
    float rightpadding;
    float bottompadding;
 

    ComPtr<ID3D12PipelineState> textPSO;
    int maxNumTextCharacters = 1024;
    static const int frameBufferCount = 2;
    ID3D12Resource* textVertexBuffer[frameBufferCount];
    D3D12_VERTEX_BUFFER_VIEW textVertexBufferView[frameBufferCount]; //���ý�����
    UINT8* textVBGPUAddress[frameBufferCount]; //�������

    void RenderText(ComPtr<ID3D12GraphicsCommandList> directX12_commandList);
    FontUI LoadFont(LPCWSTR filename, int windowWidth, int windowHeight);

public:
    float GetKerning(wchar_t first, wchar_t second);     // �ι��ڸ� ����ϴ� Ŀ���� ���� ��ȯ
    FontChar* GetChar(wchar_t c);     // �־��� fonchar�� ��ȯ

    void LoadAssets(std::wstring assetsPath, ComPtr<ID3D12Device> directX12_device, ComPtr<ID3D12RootSignature> directX12_rootSignature);
    void LoadVertextBuffer( ComPtr<ID3D12Device> directX12_device);
    void LoadVertextBufferView();
    void UpdatePipline();


};



