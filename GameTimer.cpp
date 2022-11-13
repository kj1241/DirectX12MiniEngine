#include "stdafx.h"
#include "GameTimer.h"



GameTimer::GameTimer() : secondsPerCount(0.0), deltaTime(-1.0), baseTime(0),pausedTime(0), prevTime(0), currTime(0),stopTime(0), bStopTime(false)
{
	long long int countsPerSec=0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);  //���α׷� �ӵ� ����
	secondsPerCount = 1.0 / static_cast<double>(countsPerSec);
}

GameTimer::~GameTimer()
{
}

float GameTimer::TotalTime() const
{
	if (bStopTime) //���� �����Ǿ��ִٸ�
		return static_cast<float>(((stopTime - pausedTime) - baseTime) * secondsPerCount);
	
	return static_cast<float>(((currTime - pausedTime) - baseTime) * secondsPerCount);
}

float GameTimer::DeltaTime() const
{
	return static_cast<float>(deltaTime);
}

void GameTimer::Reset()
{
	long long int currTime=0; //���� �ð� 
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));

	baseTime = currTime; // �⺻�ð� ����ð� �ٽý����ϴ°������� �����ð� ����
	prevTime = currTime; // ����ð� �ռ��ð�
	stopTime = 0;        // ���� �ð� 0
	bStopTime = false;   // �������� ����
}

void GameTimer::Start()
{
	long long int startTime=0; //���۽ð�
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>( &startTime)); //����ȯ LARGE_INTEGER  //���α׷� �ӵ� ����

	//���� �� ���� ���ۻ��̿� ����� �����ð�
	if (bStopTime) //���� �����ִٸ�
	{
		pausedTime += (startTime - stopTime); //�����ִ½ð�
		prevTime = startTime; // ���� �ð��� �����ð����� ����
		stopTime = 0;  // �����ð�
		bStopTime = false; //���� ����
	}

}

void GameTimer::Stop()
{
	if (!bStopTime) //����� �ƴ϶��
	{
		long long int currTime=0; //����ð�
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime)); //���α׷� �ӵ� ����

		stopTime = currTime; //���� �ð�
		bStopTime = true; //��������� 
	}
}

void GameTimer::Tick()
{
	if (bStopTime) //�����ִٸ�
	{
		deltaTime = 0.0; //�����ð��� 0
		return;
	}

	long long int currTime=0; //����ð�
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime)); //���α׷� �ӵ� ����
	this->currTime = currTime; //����ð� ����

	deltaTime = (this->currTime - prevTime) * secondsPerCount; // (����ð� - �����ð�) �ʴ� ī��� = �ð���ȭ��
	prevTime = this->currTime; //���������� ����

	if (deltaTime < 0.0)  //��������η� ���ԵǸ� ���õ����� �������� ���ü� ����
	{
		deltaTime = 0.0;
	}

}


