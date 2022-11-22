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
	//더 세련되게 읽고싶다... 일딴 구현(노가다로하는데 더 세련되게 구상하는방법 생각좀...)
	//이렇게 만들려면 차라리 폰트 제너레이터부터 구현되야되는거 아닌가?
	std::wifstream fin;
	fin.open(filename);

	std::wstring temp; // 임의의 글
	int startpos; //시작위치 

	//폰트 이름 추출
	fin >> temp >> temp;
	startpos = temp.find(L"\"") + 1;
	name = temp.substr(startpos, temp.size() - startpos - 1); //이름 넣기

	// 폰트 사이즈
	fin >> temp;  //size = 
	startpos = temp.find(L"=") + 1;
	size = std::stoi(temp.substr(startpos, temp.size() - startpos)); // 사이즈 넣기

	fin >> temp >> temp >> temp >> temp >> temp >> temp >> temp;  // bold, italic, charset, unicode, stretchH, smooth, aa 까지

	fin >> temp; // ex padding=3,2,1,0  패딩 크기 얻기
	startpos = temp.find(L"=") + 1;
	temp = temp.substr(startpos, temp.size() - startpos); // 3,2,1,0

	startpos = temp.find(L",") + 1;
	toppadding = std::stoi(temp.substr(0, startpos)) / (float)windowWidth; // 3/윈도우 사이즈

	temp = temp.substr(startpos, temp.size() - startpos); 
	startpos = temp.find(L",") + 1;
	rightpadding = std::stoi(temp.substr(0, startpos)) / (float)windowWidth; // 2/윈도우 사이즈

	temp = temp.substr(startpos, temp.size() - startpos);
	startpos = temp.find(L",") + 1;
	bottompadding = std::stoi(temp.substr(0, startpos)) / (float)windowWidth; // 1/윈도우사이즈

	temp = temp.substr(startpos, temp.size() - startpos);
	leftpadding = std::stoi(temp) / (float)windowWidth; //0/ 윈도우 사이즈

	fin >> temp; // ex spacing=0,0

	fin >> temp >> temp; // ex common lineHeight=95
	startpos = temp.find(L"=") + 1; //lineHeight = 95
	lineHeight = (float)std::stoi(temp.substr(startpos, temp.size() - startpos)) / (float)windowHeight; //95 / 윈도우 높이

	//정규화
	fin >> temp; // ex base=68
	startpos = temp.find(L"=") + 1;
	baseHeight = (float)std::stoi(temp.substr(startpos, temp.size() - startpos)) / (float)windowHeight; // 68/윈도우 높이

	//텍스쳐 넓이 얻어오기
	fin >> temp; // ex scaleW=512
	startpos = temp.find(L"=") + 1;
	textureWidth = std::stoi(temp.substr(startpos, temp.size() - startpos)); // 512

	//텍스쳐 높이 얻어오기
	fin >> temp; // ex scaleH=512
	startpos = temp.find(L"=") + 1;
	textureHeight = std::stoi(temp.substr(startpos, temp.size() - startpos)); // 512

	fin >> temp >> temp>> temp >> temp; // pages=1 packed=0 page id=0

	//텍스쳐 파일명 가져오기
	fin >> temp; // file="BMfont.png"
	startpos = temp.find(L"\"") + 1;
	fontImage = temp.substr(startpos, temp.size() - startpos - 1); // BMfont.png

	//문자수 얻기
	fin >> temp >> temp; // ex chars count=97
	startpos = temp.find(L"=") + 1;
	numCharacters = std::stoi(temp.substr(startpos, temp.size() - startpos)); //97

	//캐릭터 리스트 초기화
	CharList = new FontChar[numCharacters];

	// 더 깔끔하게 하는 방법... 없나
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
		CharList[i].width = (float)std::stoi(temp) / (float)windowWidth; //윈도우 넓이
		CharList[i].textWidth = (float)std::stoi(temp) / (float)textureWidth; //텍스트 넓이

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

	//커널수 얻기
	fin >> temp >> temp; // kernings count=96
	startpos = temp.find(L"=") + 1;
	numKernings = std::stoi(temp.substr(startpos, temp.size() - startpos));

	// 커널 리스트초기화
	KerningsList = new FontKerning[numKernings];

	for (int i = 0; i < numKernings; ++i)
	{
		fin >> temp >> temp; // kerning first=87
		startpos = temp.find(L"=") + 1;
		KerningsList[i].firstid = std::stoi(temp.substr(startpos, temp.size() - startpos));

		fin >> temp; // second=45
		startpos = temp.find(L"=") + 1;
		KerningsList[i].secondid = std::stoi(temp.substr(startpos, temp.size() - startpos));

		fin >> temp; // amount=-1 갯수 얻기
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


