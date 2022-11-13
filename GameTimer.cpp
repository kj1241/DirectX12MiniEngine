#include "stdafx.h"
#include "GameTimer.h"



GameTimer::GameTimer() : secondsPerCount(0.0), deltaTime(-1.0), baseTime(0),pausedTime(0), prevTime(0), currTime(0),stopTime(0), bStopTime(false)
{
	long long int countsPerSec=0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);  //프로그램 속도 측정
	secondsPerCount = 1.0 / static_cast<double>(countsPerSec);
}

GameTimer::~GameTimer()
{
}

float GameTimer::TotalTime() const
{
	if (bStopTime) //만약 중지되어있다면
		return static_cast<float>(((stopTime - pausedTime) - baseTime) * secondsPerCount);
	
	return static_cast<float>(((currTime - pausedTime) - baseTime) * secondsPerCount);
}

float GameTimer::DeltaTime() const
{
	return static_cast<float>(deltaTime);
}

void GameTimer::Reset()
{
	long long int currTime=0; //현재 시간 
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));

	baseTime = currTime; // 기본시간 현재시간 다시시작하는것임으로 이전시간 제거
	prevTime = currTime; // 현재시간 앞선시간
	stopTime = 0;        // 멈춘 시간 0
	bStopTime = false;   // 멈춰있지 않음
}

void GameTimer::Start()
{
	long long int startTime=0; //시작시간
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>( &startTime)); //형변환 LARGE_INTEGER  //프로그램 속도 측정

	//중지 및 시작 시작사이에 경과된 누적시간
	if (bStopTime) //만약 멈춰있다면
	{
		pausedTime += (startTime - stopTime); //멈춰있는시간
		prevTime = startTime; // 지금 시간은 이전시간으로 갱신
		stopTime = 0;  // 정지시간
		bStopTime = false; //멈춤 해제
	}

}

void GameTimer::Stop()
{
	if (!bStopTime) //멈춘게 아니라면
	{
		long long int currTime=0; //현재시간
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime)); //프로그램 속도 측정

		stopTime = currTime; //멈춘 시간
		bStopTime = true; //멈춰버리고 
	}
}

void GameTimer::Tick()
{
	if (bStopTime) //멈춰있다면
	{
		deltaTime = 0.0; //번위시간은 0
		return;
	}

	long long int currTime=0; //현재시간
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime)); //프로그램 속도 측정
	this->currTime = currTime; //현재시간 갱신

	deltaTime = (this->currTime - prevTime) * secondsPerCount; // (현재시간 - 이전시간) 초당 카운드 = 시간변화랑
	prevTime = this->currTime; //다음프레임 저장

	if (deltaTime < 0.0)  //절전모드인로 들어가게되면 셔플됨으로 음수값이 나올수 있음
	{
		deltaTime = 0.0;
	}

}


