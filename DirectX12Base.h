#pragma once
#include "DirectX12Function.h"
#include "WinAPI.h"
#include "GameTimer.h"

class DirectX12Base
{
public:
    DirectX12Base(UINT width, UINT height, std::wstring name); //생성자
    virtual ~DirectX12Base(); //소멸자

    virtual void OnInit() = 0; //초기값
    virtual void OnUpdate() = 0; //업데이트
    virtual void OnRender() = 0; //랜더러
    virtual void OnDestroy() = 0; //파괴

    // 이벤트 핸들러를 재정의하여 특정 메시지 처리
    virtual void OnKeyDown(UINT8);// 매개변수 키값
    virtual void OnKeyUp(UINT8);// 매개변수 키값

    void CalculateFrameStats(); //계산프래임상태

    // 접근자
    UINT GetWidth() const;  //넓이
    UINT GetHeight() const;  //높이
    const WCHAR* GetTitle() const;   //타이틀

    void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc); //명령줄 인수 구분

    void GameTimeReset();
    void GameTimeStart();
    void GameTimeStop();
    void GameTimeTick();

protected:
    std::wstring GetAssetFullPath(LPCWSTR assetName);
    void GetHardwareAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false);
    void SetCustomWindowText(LPCWSTR text);

    // 뷰포트
    UINT directX12_width = 0;
    UINT directX12_height = 0;
    float directX12_aspectRatio = 0.0f;

    // 어뎁터 정보
    bool directX12_useWarpDevice = false;

    // 시간
    GameTimer gameTimer; //게임타이머

private:
    float fps = 0;
    float mspf = 0;

    std::wstring directX12_assetsPath;  // 루트자산경로 
    std::wstring directX12_title;  // 윈도우 타이틀

    int frameCnt = 0; //프레임 카운트
    float timeElapsed = 0.0f; //경과된 시간
};
