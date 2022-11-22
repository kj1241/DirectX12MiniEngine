#pragma once

//2022.11.22
//BMFontUI�Դϴ�.  
class BMFontUI
{
public:
    struct FontKerning
    {
        int firstid; // ù��° ���� 4byte
        int secondid; // �ι�° ���� 8byte
        float amount; // �ι������ڸ� ���ϰų� ���� x 12byte
        float pading; // ����ȭ�� ���� 4����Ʈ 16byte
    };

    struct FontChar
    {
        int id; //�����ڵ� id  4byte

        //�ؽ�Ʈ ��ǥ ���� 
        float u; // u texture ��ǥ  8byte 
        float v; // v texture ��ǥ  12byte
        float textWidth; // �ؽ�Ʈ ���� ����  16byte 
        float textHeight; // �ؽ�Ʈ ���� ����  20byte

        float width; //ȭ�� ��ǥ�� ���� ����  24byte
        float height; //ȭ�� ��ǥ�� ���� ����   28byte

        //�۲�ũ�⿡ ���� ����ȭ
        float xoffset; //���� Ŀ�� ũ�⿡�� ���� ���ʱ��� ������   32byte
        float yoffset; // ���� �������� ������ �� ���� ������    36byte
        float xadvance; //���� ���ڸ����ؼ� �̵��ؾߵǴ� ũ��  40byte
        //float pading[2]; //����ȭ�� ���� 8byte  48byte
        float page; // ������ ���� ����Ű�°�
        float channel; // ä��
    };
    BMFontUI();
    ~BMFontUI();

    float GetKerning(WCHAR first, WCHAR second);     // �ι��ڸ� ����ϴ� Ŀ���� ���� ��ȯ
    FontChar* GetChar(WCHAR c);     // �־��� fonchar�� ��ȯ

    void LoadFont(LPCWSTR filename, int windowWidth, int windowHeight);

    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandle();
    void SetSrvHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle);

    float GetLeftpadding();
    float GetRightpadding();
    float GetToppadding();
    float GetBottompadding();

    float GetLineHeight();

    std::wstring GetFontImage();
    ComPtr<ID3D12Resource> GetTextureBuffer();

    ComPtr<ID3D12Resource> textureBuffer; // �۲� �ؽ��� ���ҽ�

private:
    std::wstring name; //��Ʈ �̸�
    std::wstring fontImage;
    int size; //�۲�ũ�� ���γ��̴� (1,0)���
    float lineHeight=0; //�����ٷ� �󸶳� �̵��ؾ� �Ǵ���
    float baseHeight=0; //��� ���� ���̰� ����ȭ
    int textureWidth=0; //�۲� �ؽ��� �ʺ�
    int textureHeight=0; //�۲� �ؽ��� ����
    int numCharacters =0; //�۲� ���ڼ�
    FontChar* CharList = {}; // ���� ���
    int numKernings =0; // Ŀ�� ��
    FontKerning* KerningsList = {}; // Ŀ�� ���� ������ ���

    D3D12_GPU_DESCRIPTOR_HANDLE srvHandle; // �۲� srv

    //���� ü������ ��
    float leftpadding;
    float rightpadding;
    float toppadding;
    float bottompadding;
};