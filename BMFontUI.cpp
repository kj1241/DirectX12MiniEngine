#include "stdafx.h"
#include "BMFontUI.h"

BMFontUI::BMFontUI() : size(0), lineHeight(0.0f), baseHeight(0.0f), textureWidth(0), textureHeight(0), numCharacters(0), numKernings(0)
{

}

BMFontUI::~BMFontUI()
{
	if (CharList)
	{
		delete[] CharList;
		CharList = nullptr;
	}
	if (KerningsList)
	{
		delete[] KerningsList;
		KerningsList = nullptr;
	}

}

float BMFontUI::GetKerning(WCHAR first, WCHAR second)
{
    for (int i = 0; i < numKernings; ++i)
    {
        if ((WCHAR)KerningsList[i].firstid == first && (WCHAR)KerningsList[i].secondid == second)
            return KerningsList[i].amount;
    }
    return 0.0f;
}

BMFontUI::FontChar* BMFontUI::GetChar(WCHAR c)
{
    for (int i = 0; i < numCharacters; ++i)
    {
        if (c == (WCHAR)CharList[i].id)
            return &CharList[i];
    }
    return nullptr;
}

void BMFontUI::LoadFont(LPCWSTR filename, int windowWidth, int windowHeight)
{
	//�� ���õǰ� �а�ʹ�... �ϵ� ����(�밡�ٷ��ϴµ� �� ���õǰ� �����ϴ¹�� ������...)
	//�̷��� ������� ���� ��Ʈ ���ʷ����ͺ��� �����ǾߵǴ°� �ƴѰ�?
	std::wifstream fin;
	fin.open(filename);

	std::wstring temp; // ������ ��
	int startpos; //������ġ 

	//��Ʈ �̸� ����
	fin >> temp >> temp;
	startpos = temp.find(L"\"") + 1;
	name = temp.substr(startpos, temp.size() - startpos - 1); //�̸� �ֱ�

	// ��Ʈ ������
	fin >> temp;  //size = 
	startpos = temp.find(L"=") + 1;
	size = std::stoi(temp.substr(startpos, temp.size() - startpos)); // ������ �ֱ�

	fin >> temp >> temp >> temp >> temp >> temp >> temp >> temp;  // bold, italic, charset, unicode, stretchH, smooth, aa ����

	fin >> temp; // ex padding=3,2,1,0  �е� ũ�� ���
	startpos = temp.find(L"=") + 1;
	temp = temp.substr(startpos, temp.size() - startpos); // 3,2,1,0

	startpos = temp.find(L",") + 1;
	toppadding = std::stoi(temp.substr(0, startpos)) / (float)windowWidth; // 3/������ ������

	temp = temp.substr(startpos, temp.size() - startpos); 
	startpos = temp.find(L",") + 1;
	rightpadding = std::stoi(temp.substr(0, startpos)) / (float)windowWidth; // 2/������ ������

	temp = temp.substr(startpos, temp.size() - startpos);
	startpos = temp.find(L",") + 1;
	bottompadding = std::stoi(temp.substr(0, startpos)) / (float)windowWidth; // 1/�����������

	temp = temp.substr(startpos, temp.size() - startpos);
	leftpadding = std::stoi(temp) / (float)windowWidth; //0/ ������ ������

	fin >> temp; // ex spacing=0,0

	fin >> temp >> temp; // ex common lineHeight=95
	startpos = temp.find(L"=") + 1; //lineHeight = 95
	lineHeight = (float)std::stoi(temp.substr(startpos, temp.size() - startpos)) / (float)windowHeight; //95 / ������ ����

	//����ȭ
	fin >> temp; // ex base=68
	startpos = temp.find(L"=") + 1;
	baseHeight = (float)std::stoi(temp.substr(startpos, temp.size() - startpos)) / (float)windowHeight; // 68/������ ����

	//�ؽ��� ���� ������
	fin >> temp; // ex scaleW=512
	startpos = temp.find(L"=") + 1;
	textureWidth = std::stoi(temp.substr(startpos, temp.size() - startpos)); // 512

	//�ؽ��� ���� ������
	fin >> temp; // ex scaleH=512
	startpos = temp.find(L"=") + 1;
	textureHeight = std::stoi(temp.substr(startpos, temp.size() - startpos)); // 512

	fin >> temp >> temp>> temp >> temp; // pages=1 packed=0 page id=0

	//�ؽ��� ���ϸ� ��������
	fin >> temp; // file="BMfont.png"
	startpos = temp.find(L"\"") + 1;
	fontImage = temp.substr(startpos, temp.size() - startpos - 1); // BMfont.png

	//���ڼ� ���
	fin >> temp >> temp; // ex chars count=97
	startpos = temp.find(L"=") + 1;
	numCharacters = std::stoi(temp.substr(startpos, temp.size() - startpos)); //97

	//ĳ���� ����Ʈ �ʱ�ȭ
	CharList = new FontChar[numCharacters];

	// �� ����ϰ� �ϴ� ���... ����
	for (int i = 0; i < numCharacters; ++i)
	{
		fin >> temp >> temp; // ex char id=0
		startpos = temp.find(L"=") + 1;
		CharList[i].id = std::stoi(temp.substr(startpos, temp.size() - startpos));

		fin >> temp; // x=392
		startpos = temp.find(L"=") + 1;
		CharList[i].u = (float)std::stoi(temp.substr(startpos, temp.size() - startpos)) / (float)textureWidth;

		fin >> temp; // y=340
		startpos = temp.find(L"=") + 1;
		CharList[i].v = (float)std::stoi(temp.substr(startpos, temp.size() - startpos)) / (float)textureHeight;

		fin >> temp; // width=47
		startpos = temp.find(L"=") + 1;
		temp = temp.substr(startpos, temp.size() - startpos);
		CharList[i].width = (float)std::stoi(temp) / (float)windowWidth; //������ ����
		CharList[i].textWidth = (float)std::stoi(temp) / (float)textureWidth; //�ؽ�Ʈ ����

		fin >> temp; // height=57
		startpos = temp.find(L"=") + 1;
		temp = temp.substr(startpos, temp.size() - startpos);
		CharList[i].height = (float)std::stoi(temp) / (float)windowHeight;
		CharList[i].textHeight = (float)std::stoi(temp) / (float)textureHeight;

		fin >> temp; // xoffset=-6
		startpos = temp.find(L"=") + 1;
		CharList[i].xoffset = (float)std::stoi(temp.substr(startpos, temp.size() - startpos)) / (float)windowWidth;

		fin >> temp; // yoffset=16
		startpos = temp.find(L"=") + 1;
		CharList[i].yoffset = (float)std::stoi(temp.substr(startpos, temp.size() - startpos)) / (float)windowHeight;

		fin >> temp; // xadvance=65
		startpos = temp.find(L"=") + 1;
		CharList[i].xadvance = (float)std::stoi(temp.substr(startpos, temp.size() - startpos)) / (float)windowWidth;

		fin >> temp; // page=0    
		startpos = temp.find(L"=") + 1;
		CharList[i].page = std::stoi(temp.substr(startpos, temp.size() - startpos));

		fin >> temp; // chnl=0
		startpos = temp.find(L"=") + 1;
		CharList[i].channel = std::stoi(temp.substr(startpos, temp.size() - startpos));
	}

	//Ŀ�μ� ���
	fin >> temp >> temp; // kernings count=96
	startpos = temp.find(L"=") + 1;
	numKernings = std::stoi(temp.substr(startpos, temp.size() - startpos));

	// Ŀ�� ����Ʈ�ʱ�ȭ
	KerningsList = new FontKerning[numKernings];

	for (int i = 0; i < numKernings; ++i)
	{
		fin >> temp >> temp; // kerning first=87
		startpos = temp.find(L"=") + 1;
		KerningsList[i].firstid = std::stoi(temp.substr(startpos, temp.size() - startpos));

		fin >> temp; // second=45
		startpos = temp.find(L"=") + 1;
		KerningsList[i].secondid = std::stoi(temp.substr(startpos, temp.size() - startpos));

		fin >> temp; // amount=-1 ���� ���
		startpos = temp.find(L"=") + 1;
		KerningsList[i].amount = (float)std::stoi(temp.substr(startpos, temp.size() - startpos)) / (float)windowWidth;
	}
}


D3D12_GPU_DESCRIPTOR_HANDLE BMFontUI::GetSrvHandle()
{
	return srvHandle;
}

void BMFontUI::SetSrvHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
    srvHandle = handle;
}

float BMFontUI::GetLeftpadding()
{
    return leftpadding;
}

float BMFontUI::GetRightpadding()
{
    return rightpadding;
}

float BMFontUI::GetToppadding()
{
    return toppadding;
}

float BMFontUI::GetBottompadding()
{
    return bottompadding;
}

float BMFontUI::GetLineHeight()
{
    return lineHeight;
}

std::wstring BMFontUI::GetFontImage()
{
	return fontImage;
}

ComPtr<ID3D12Resource> BMFontUI::GetTextureBuffer()
{
	return textureBuffer;
}


