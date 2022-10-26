#pragma once
#include "DirectX12Base.h"

class DirectX12Base;

class WinAPI
{
public:
    WinAPI(); //생성자
    ~WinAPI(); //소멸자
    static bool Init(DirectX12Base* pDirectX, HINSTANCE hInstance, int nCmdShow); //초기화
    static int Run(DirectX12Base* pDirectX); //실행
    static HWND GetHwnd(); //window 핸들 얻기

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); //윈도우 프로시져

private:
    static HWND WinAPI_hwnd; //윈도우 핸들
};
