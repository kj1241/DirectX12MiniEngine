#include "stdafx.h"
#include "DirectX12Base.h"

//������
DirectX12Base::DirectX12Base(UINT width, UINT height, std::wstring name) : directX12_width(width), directX12_height(height), directX12_title(name), directX12_useWarpDevice(false)
{
	WCHAR assetsPath[512];
	GetAssetsPath(assetsPath, _countof(assetsPath)); //��ġ �˾Ƴ���
    directX12_assetsPath = assetsPath; //������ �ҷ����� ���� ��ġ ����
    directX12_aspectRatio = static_cast<float>(width) / static_cast<float>(height); //��Ⱦ�� ������ ����
}

//�Ҹ���
DirectX12Base::~DirectX12Base()
{
}

void DirectX12Base::OnKeyDown(UINT8) //Ű�� ��������
{
}


void DirectX12Base::OnKeyUp(UINT8) //Ű�� ������
{
}

void DirectX12Base::CalculateFrameStats() //������ ���� ���
{
    static int frameCnt = 0; //������ ī��Ʈ
    static float timeElapsed = 0.0f; //����� �ð�

    frameCnt++;
    //1�ʵ��� ���
    if ((gameTimer.TotalTime() - timeElapsed) >= 1.0f)
    {
        fps = static_cast<float>(frameCnt); // fps = frameCnt / 1
        mspf = 1000.0f / fps;

        std::wstring text = L" fps: " + std::to_wstring(fps) + L" mspf: " + std::to_wstring(mspf);
        SetCustomWindowText(text.c_str());

        // ��������� �缳��
        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}

UINT DirectX12Base::GetWidth() const //���� ������(�������)
{
	return directX12_width;
}

UINT DirectX12Base::GetHeight() const //���� ������(�������)
{
	return directX12_height;
}

const WCHAR* DirectX12Base::GetTitle() const //Ÿ��Ʋ ������(�������)
{
	return directX12_title.c_str();
}

void DirectX12Base::ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc) //����� �μ� ����
{
	for (int i = 1; i < argc; ++i)
	{
        // '-warp' �Ǵ� '/warp'�� argv�� ���Ͽ� ��ġ�Ѵٸ�
		if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0 || _wcsnicmp(argv[i], L"/warp", wcslen(argv[i])) == 0)
		{
            directX12_useWarpDevice = true; //warp ����̽��� true�� �ٲ�
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
    *ppAdapter = nullptr; // ����� ����Ʈ �ʱ�ȭ
    ComPtr<IDXGIAdapter1> adapter;
    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6)))) //���丮���� 6������ �����ϴ��� �˻�
    {
        for (
            UINT adapterIndex = 0;  // ��� �ε��� ��ȣ ����
            SUCCEEDED(factory6->EnumAdapterByGpuPreference(  //��� �׷� ����
                adapterIndex,  // ���� �� �ε��� ��ȣ�� �ְ� 
                requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED, // ���� �������� ��Ͱ� �����ϸ�  ��������� ���� ������ ���ٰ� ǥ��
                IID_PPV_ARGS(&adapter))); //��� �ּ� �����´�.
            ++adapterIndex) //��� ��ȣ ����
        {
            DXGI_ADAPTER_DESC1 desc; //���� ī�� 
            adapter->GetDesc1(&desc); //��� �ּҷ� ���� ī�� ��������

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) // ����Ʈ������  �Ѿ��
            {
                // �⺻ ���� ����̹� ��� ���� x
                // ����Ʈ���� ��Ͱ� �ʿ��ϸ� /warp ����� ����
                continue;
            }

            // �ϵ���� ��Ͱ� directx12 �����ϴ��� Ȯ�� ���� ��ġ�� ���� x
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    if (adapter.Get() == nullptr) //��Ͱ�
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) //����Ʈ���� ��͸� �Ѿ
            {
                // �⺻ ���� ����̹� ��� ���� x
                // ����Ʈ���� ��Ͱ� �ʿ��ϸ� /warp ����� ����
                continue;
            }

            // �ϵ���� ��Ͱ� directx12 �����ϴ��� Ȯ�� ���� ��ġ�� ���� x
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }
    *ppAdapter = adapter.Detach();
}

void DirectX12Base::SetCustomWindowText(LPCWSTR text) //������ ����ǥ��â ǥ��
{
    std::wstring windowText = directX12_title + L": " + text;
	SetWindowText(WinAPI::GetHwnd(), windowText.c_str()); // �ڵ鰪���� ������ �����ϱ�
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
