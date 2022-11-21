#pragma once
#include "DirectX12Function.h"

struct TextVertex {
    XMFLOAT4 pos;
    XMFLOAT4 texCoord;
    XMFLOAT4 color;
};

struct FontChar
{
    int id; //유니코드 id  4byte

    //텍스트 좌표 변형 
    float u; // u texture 좌표  8byte 
    float v; // v texture 좌표  12byte
    float twidth; // 텍스트 문자 넓이  16byte 
    float theight; // 텍스트 문자 높이  20byte

    float width; //화면 좌표의 문자 넓이  24byte
    float height; //화면 좌표의 문자 높이   28byte

    //글꼴크기에 따른 정규화
    float xoffset; //현재 커서 크기에서 문자 왼쪽까지 오프셋   32byte
    float yoffset; // 라인 맨위에서 문자의 맨 위로 오프셋    36byte
    float xadvance; //다음 문자를위해서 이동해야되는 크기  40byte
    float pading[2]; //최적화를 위한 8byte  48byte
};

struct FontKerning
{
    int firstid; // 첫번째 문자 4byte
    int secondid; // 두번째 문자 8byte
    float amount; // 두번쨰문자를 더하거나 뺄양 x 12byte
    float pading; // 최적화를 위한 4바이트 16byte
};

class FontUI
{
private:
    std::wstring name; //폰트 이름
    std::wstring fontImage;
    int size; //글꼴크기 라인높이는 (1,0)기반
    float lineHeight; //다음줄로 얼마나 이동해야 되는지
    float baseHeight; //모든 문자 높이가 정규화
    int textureWidth; //글꼴 텍스쳐 너비
    int textureHeight; //글꼴 텍스쳐 높이
    int numCharacters; //글꼴 문자수
    FontChar* CharList; // 문자 목록
    int numKernings; // 커닝 수
    FontKerning* KerningsList; // 커닝 값을 저장한 목록
    ID3D12Resource* textureBuffer; // 글꼴 텍스쳐 리소스
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandle; // 글꼴 srv

    //문자 체워지는 양
    float leftpadding;
    float toppadding;
    float rightpadding;
    float bottompadding;
 

    ComPtr<ID3D12PipelineState> textPSO;
    int maxNumTextCharacters = 1024;
    static const int frameBufferCount = 2;
    ID3D12Resource* textVertexBuffer[frameBufferCount];
    D3D12_VERTEX_BUFFER_VIEW textVertexBufferView[frameBufferCount]; //버택스버퍼
    UINT8* textVBGPUAddress[frameBufferCount]; //상수버퍼

    void RenderText(ComPtr<ID3D12GraphicsCommandList> directX12_commandList);
    FontUI LoadFont(LPCWSTR filename, int windowWidth, int windowHeight);

public:
    float GetKerning(wchar_t first, wchar_t second);     // 두문자를 사용하는 커닝의 양을 반환
    FontChar* GetChar(wchar_t c);     // 주어진 fonchar를 반환

    void LoadAssets(std::wstring assetsPath, ComPtr<ID3D12Device> directX12_device, ComPtr<ID3D12RootSignature> directX12_rootSignature);
    void LoadVertextBuffer( ComPtr<ID3D12Device> directX12_device);
    void LoadVertextBufferView();
    void UpdatePipline();


};



