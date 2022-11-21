#pragma once
#include "DirectX12Function.h"
#include "WinAPI.h"
#include "GameTimer.h"

class DirectX12Base
{
public:
    DirectX12Base(UINT width, UINT height, std::wstring name); //������
    virtual ~DirectX12Base(); //�Ҹ���

    virtual void OnInit() = 0; //�ʱⰪ
    virtual void OnUpdate() = 0; //������Ʈ
    virtual void OnRender() = 0; //������
    virtual void OnDestroy() = 0; //�ı�

    // �̺�Ʈ �ڵ鷯�� �������Ͽ� Ư�� �޽��� ó��
    virtual void OnKeyDown(UINT8);// �Ű����� Ű��
    virtual void OnKeyUp(UINT8);// �Ű����� Ű��

    void CalculateFrameStats(); //��������ӻ���

    // ������
    UINT GetWidth() const;  //����
    UINT GetHeight() const;  //����
    const WCHAR* GetTitle() const;   //Ÿ��Ʋ

    void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc); //����� �μ� ����

    void GameTimeReset();
    void GameTimeStart();
    void GameTimeStop();
    void GameTimeTick();

protected:
    std::wstring GetAssetFullPath(LPCWSTR assetName);
    void GetHardwareAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false);
    void SetCustomWindowText(LPCWSTR text);

    // ����Ʈ
    UINT directX12_width = 0;
    UINT directX12_height = 0;
    float directX12_aspectRatio = 0.0f;

    // ��� ����
    bool directX12_useWarpDevice = false;

    // �ð�
    GameTimer gameTimer; //����Ÿ�̸�

private:
    float fps = 0;
    float mspf = 0;

    std::wstring directX12_assetsPath;  // ��Ʈ�ڻ��� 
    std::wstring directX12_title;  // ������ Ÿ��Ʋ

    int frameCnt = 0; //������ ī��Ʈ
    float timeElapsed = 0.0f; //����� �ð�
};
