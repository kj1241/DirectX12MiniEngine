#pragma once

//게임 시간을 관장하는 함수
class GameTimer
{
public:
	GameTimer();//생성자
	~GameTimer(); // 소멸자

	float TotalTime()const; //총시간
	float DeltaTime()const; //프레임 변화시간

	void Reset(); //다시시작
	void Start(); //시작
	void Stop(); //멈춤
	void Tick(); //정밀한시간

private:
	bool bStopTime;
	long long int baseTime;// 기본 시간
	long long int pausedTime;// 일시정지 시간
	long long int stopTime;  // 멈춘 시간
	long long int prevTime;  // 앞선 시간
	long long int currTime;  // 현재 시간

	double secondsPerCount; //초당카운트 
	double deltaTime;  //변위시간
};
