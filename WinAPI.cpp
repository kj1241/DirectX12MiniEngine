#include "stdafx.h"
#include "WinAPI.h"

HWND WinAPI::WinAPI_hwnd = nullptr; //�ڵ� �� �ʱ�ȭ

WinAPI::WinAPI() //������
{

}

WinAPI::~WinAPI() //�Ҹ���
{

}

bool WinAPI::Init(DirectX12Base* pDirectX, HINSTANCE hInstance, int nCmdShow) //�ʱⰪ
{
    //����� �ŰԺ��� ���� 
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    pDirectX->ParseCommandLineArgs(argv, argc);
    LocalFree(argv);
    // ������ Ŭ�� �ʱ�ȭ
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"DirectX12MiniEngine";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, static_cast<LONG>(pDirectX->GetWidth()), static_cast<LONG>(pDirectX->GetHeight()) }; //������ â����
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // â�� �ڵ� ����
    WinAPI_hwnd = CreateWindow(
        windowClass.lpszClassName,
        pDirectX->GetTitle(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,        // �θ�â ����
        nullptr,        // �޴� ������� ����
        hInstance,
        pDirectX); //���ν����� DirectXBase Ŭ���� �ּ� �ѱ��

    pDirectX->OnInit(); //�ʱ�ȭ

    ShowWindow(WinAPI_hwnd, nCmdShow); //������ �����ֱ�
    return true;
}

//WinAPI����
int WinAPI::Run(DirectX12Base* pDirectX)
{
    // ���� ����
    MSG msg = { 0 };
    pDirectX->GameTimeReset();

    while (msg.message != WM_QUIT) //�޽����� winAPI���ᰡ �ƴ϶��
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) // �޽����� ������ ó��
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else //�׷��� ������ ���ϸ��̼�/���� �۾��� ����
        {
            pDirectX->GameTimeTick();

            pDirectX->CalculateFrameStats();
            pDirectX->OnUpdate(); //������Ʈ
            pDirectX->OnRender(); //������
        }
    }
    pDirectX->OnDestroy(); //����
    return static_cast<char>(msg.wParam);    // WM_QUIT �޽����� ��ȯ
}

//�ڵ鰪�� ��� ���ؼ�
HWND WinAPI::GetHwnd()
{
    return WinAPI_hwnd;
}

//�� ���ν���
LRESULT WinAPI::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DirectX12Base* pDirectX = reinterpret_cast<DirectX12Base*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)); //�ڵ�� �Ѱ����� ����ϱ� ���ؼ�

    //�޽���
    switch (message)
    {
    case WM_CREATE: //â�� �����������
        {
            // ������ ����� DirectX12Base �� ����
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
        return 0;

    case WM_KEYDOWN: //Ű��ư�� ��������
        if (pDirectX)
        {
            pDirectX->OnKeyDown(static_cast<UINT8>(wParam));
        }
        return 0;

    case WM_KEYUP:  //Ű��ư��  �������
        if (pDirectX)
        {
            pDirectX->OnKeyUp(static_cast<UINT8>(wParam));
        }
        return 0;

        //case WM_PAINT: //������� �ʴ� ����: ���Լ��� �ٽ� �׸��¿��ε� run()�Լ����� ó���ϰ� �ֱ� ����
        //    return 0;

    case WM_DESTROY: //�ı��Ǿ�����
        PostQuitMessage(0);
        return 0;
    }

    // ����Ʈ�� ��� ��� �޽��� ó��
    return DefWindowProc(hWnd, message, wParam, lParam);
}
