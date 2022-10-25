#pragma once

class WinAPI
{
public:
    WinAPI(); //������
    ~WinAPI(); //�Ҹ���
    static bool Init(HINSTANCE hInstance, int nCmdShow); //�ʱ�ȭ
    static int Run(); //����
    static HWND GetHwnd(); //window �ڵ� ���

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); //������ ���ν���

private:
    static HWND WinAPI_hwnd; //������ �ڵ�
};
