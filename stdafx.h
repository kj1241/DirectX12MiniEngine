#pragma once //�̸������ϵ� �ش�

#ifndef STDAFX_H // STDAFX_H�� ���� �ȵǾ� �ִٸ�
#define STDAFX_H // STDAFX_H�� ����
#endif //���� ��(pragma once)�� ����� ȿ��  ��������� �ߺ��Ǿ� �о �������� �������

#include <windows.h> //winAPI
#include <string> // string����
#include <d3d12.h> //directx12 
#include <dxgi1_6.h> //dxgi
#include <D3Dcompiler.h> //d3d �����Ϸ�
#include <DirectXMath.h> //DirectX Math �Լ��� ����ϱ����ؼ�
#include <wrl.h> //Microsoft ����� ���ؼ�
#include "d3dx12.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")