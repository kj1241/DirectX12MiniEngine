#include "stdafx.h"
#include "WinAPI.h"

HWND WinAPI::WinAPI_hwnd = nullptr; //핸들 값 초기화

WinAPI::WinAPI() //생성자
{

}

WinAPI::~WinAPI() //소멸자
{

}

bool WinAPI::Init(DirectX12Base* pDirectX, HINSTANCE hInstance, int nCmdShow) //초기값
{
    //명령줄 매게변수 구분 
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    pDirectX->ParseCommandLineArgs(argv, argc);
    LocalFree(argv);
    // 윈도우 클라스 초기화
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"DirectX12MiniEngine";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, static_cast<LONG>(pDirectX->GetWidth()), static_cast<LONG>(pDirectX->GetHeight()) }; //윈도우 창범위
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // 창과 핸들 만듦
    WinAPI_hwnd = CreateWindow(
        windowClass.lpszClassName,
        pDirectX->GetTitle(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,        // 부모창 없음
        nullptr,        // 메뉴 사용하지 않음
        hInstance,
        pDirectX); //프로시저에 DirectXBase 클래스 주소 넘기기

    pDirectX->OnInit(); //초기화

    ShowWindow(WinAPI_hwnd, nCmdShow); //윈도우 보여주기
    return true;
}

//WinAPI실행
int WinAPI::Run(DirectX12Base* pDirectX)
{
    // 메인 루프
    MSG msg = { 0 };
    while (msg.message != WM_QUIT) //메시지가 winAPI종료가 아니라면
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) // 메시지가 있으면 처리
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else //그렇지 않으면 에니메이션/게임 작업을 수행
        {
            pDirectX->OnUpdate(); //업데이트
            pDirectX->OnRender(); //랜더링
        }
    }
    pDirectX->OnDestroy(); //제거
    return static_cast<char>(msg.wParam);    // WM_QUIT 메시지로 반환
}

//핸들값을 얻기 위해서
HWND WinAPI::GetHwnd()
{
    return WinAPI_hwnd;
}

//윈 프로시져
LRESULT WinAPI::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DirectX12Base* pDirectX = reinterpret_cast<DirectX12Base*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)); //핸들로 넘겼으면 사용하기 위해서

    //메시지
    switch (message)
    {
    case WM_CREATE: //창이 만들어졌으면
        {
            // 윈도우 만들시 DirectX12Base 를 저장
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
        return 0;

    case WM_KEYDOWN: //키버튼이 눌렸으면
        if (pDirectX)
        {
            pDirectX->OnKeyDown(static_cast<UINT8>(wParam));
        }
        return 0;

    case WM_KEYUP:  //키버튼이  때어젔으면
        if (pDirectX)
        {
            pDirectX->OnKeyUp(static_cast<UINT8>(wParam));
        }
        return 0;

        //case WM_PAINT: //사용하지 않는 이유: 이함수는 다시 그리는용인데 run()함수에서 처리하고 있기 때문
        //    return 0;

    case WM_DESTROY: //파괴되었을때
        PostQuitMessage(0);
        return 0;
    }

    // 디폴트값 대신 모든 메시지 처리
    return DefWindowProc(hWnd, message, wParam, lParam);
}
