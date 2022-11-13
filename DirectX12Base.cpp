#include "stdafx.h"
#include "DirectX12Base.h"

//생성자
DirectX12Base::DirectX12Base(UINT width, UINT height, std::wstring name) : directX12_width(width), directX12_height(height), directX12_title(name), directX12_useWarpDevice(false)
{
	WCHAR assetsPath[512];
	GetAssetsPath(assetsPath, _countof(assetsPath)); //위치 알아내기
    directX12_assetsPath = assetsPath; //에셋을 불러오는 현재 위치 저장
    directX12_aspectRatio = static_cast<float>(width) / static_cast<float>(height); //종횡비 사이즈 저장
}

//소멸자
DirectX12Base::~DirectX12Base()
{
}

void DirectX12Base::OnKeyDown(UINT8) //키를 눌렀을때
{
}


void DirectX12Base::OnKeyUp(UINT8) //키를 떘을때
{
}

void DirectX12Base::CalculateFrameStats() //프레임 상태 계산
{
    static int frameCnt = 0; //프레임 카운트
    static float timeElapsed = 0.0f; //경과된 시간

    frameCnt++;
    //1초동안 평균
    if ((gameTimer.TotalTime() - timeElapsed) >= 1.0f)
    {
        fps = static_cast<float>(frameCnt); // fps = frameCnt / 1
        mspf = 1000.0f / fps;

        std::wstring text = L" fps: " + std::to_wstring(fps) + L" mspf: " + std::to_wstring(mspf);
        SetCustomWindowText(text.c_str());

        // 평균을위해 재설정
        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}

UINT DirectX12Base::GetWidth() const //넓이 얻어오기(변경금지)
{
	return directX12_width;
}

UINT DirectX12Base::GetHeight() const //높이 얻어오기(변경금지)
{
	return directX12_height;
}

const WCHAR* DirectX12Base::GetTitle() const //타이틀 얻어오기(변경금지)
{
	return directX12_title.c_str();
}

void DirectX12Base::ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc) //명령줄 인수 구분
{
	for (int i = 1; i < argc; ++i)
	{
        // '-warp' 또는 '/warp'를 argv와 비교하여 일치한다면
		if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0 || _wcsnicmp(argv[i], L"/warp", wcslen(argv[i])) == 0)
		{
            directX12_useWarpDevice = true; //warp 디바이스를 true로 바꿈
            directX12_title = directX12_title + L" (WARP)";
		}
	}
}

std::wstring DirectX12Base::GetAssetFullPath(LPCWSTR assetName)
{
	return directX12_assetsPath + assetName;
}

void DirectX12Base::GetHardwareAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter)
{
    *ppAdapter = nullptr; // 어댑터 포인트 초기화
    ComPtr<IDXGIAdapter1> adapter;
    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6)))) //팩토리에서 6버전을 지원하는지 검사
    {
        for (
            UINT adapterIndex = 0;  // 어뎁터 인덱스 번호 시작
            SUCCEEDED(factory6->EnumAdapterByGpuPreference(  //어뎁터 그룹 열거
                adapterIndex,  // 만약 그 인덱스 번호를 넣고 
                requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED, // 높은 퍼포먼의 어뎁터가 존재하면  높은순대로 정렬 없으면 없다고 표현
                IID_PPV_ARGS(&adapter))); //어뎁터 주소 가저온다.
            ++adapterIndex) //어뎁터 번호 증가
        {
            DXGI_ADAPTER_DESC1 desc; //비디오 카드 
            adapter->GetDesc1(&desc); //어뎁터 주소로 비디오 카드 가저오기

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) // 소프트웨어라면  넘어가고
            {
                // 기본 랜더 드라이버 어뎁터 선택 x
                // 소프트웨어 어뎁터가 필요하면 /warp 명령을 보냄
                continue;
            }

            // 하드웨어 어뎁터가 directx12 지원하는지 확인 실제 장치라 생성 x
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    if (adapter.Get() == nullptr) //어뎁터가
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) //소프트웨어 어뎁터면 넘어감
            {
                // 기본 랜더 드라이버 어뎁터 선택 x
                // 소프트웨어 어뎁터가 필요하면 /warp 명령을 보냄
                continue;
            }

            // 하드웨어 어뎁터가 directx12 지원하는지 확인 실제 장치라 생성 x
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }
    *ppAdapter = adapter.Detach();
}

void DirectX12Base::SetCustomWindowText(LPCWSTR text) //윈도우 상태표시창 표현
{
    std::wstring windowText = directX12_title + L": " + text;
	SetWindowText(WinAPI::GetHwnd(), windowText.c_str()); // 핸들값으로 윈도우 설정하기
}

void DirectX12Base::GameTimeReset()
{
    gameTimer.Reset();
}

void DirectX12Base::GameTimeStart()
{
    gameTimer.Start();
}

void DirectX12Base::GameTimeStop()
{
    gameTimer.Stop();
}

void DirectX12Base::GameTimeTick()
{
    gameTimer.Tick();
}
