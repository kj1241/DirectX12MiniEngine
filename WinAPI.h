#pragma once
#include "DirectX12Base.h"

class DirectX12Base;

class WinAPI
{
public:
    WinAPI(); //������
    ~WinAPI(); //�Ҹ���
    static bool Init(DirectX12Base* pDirectX, HINSTANCE hInstance, int nCmdShow); //�ʱ�ȭ
    static int Run(DirectX12Base* pDirectX); //����
    static HWND GetHwnd(); //window �ڵ� ���

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); //������ ���ν���

private:
    static HWND WinAPI_hwnd; //������ �ڵ�
};
