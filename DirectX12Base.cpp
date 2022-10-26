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
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        for (
            UINT adapterIndex = 0;
            SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                adapterIndex,
                requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                IID_PPV_ARGS(&adapter)));
            ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // 기본 랜더 드라이버 어뎁터 선택 x
                // 소프트웨어 어뎁터가 필요하면 /warp 명령을 보냄
                continue;
            }

            // 어뎁터가 directx12 지원하는지 확인 실제 장치라 생성 x
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    if (adapter.Get() == nullptr)
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // 기본 랜더 드라이버 어뎁터 선택 x
                // 소프트웨어 어뎁터가 필요하면 /warp 명령을 보냄
                continue;
            }

            // 어뎁터가 directx12 지원하는지 확인 실제 장치라 생성 x
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
	SetWindowText(WinAPI::GetHwnd(), windowText.c_str());
}
