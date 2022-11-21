#pragma once

//���� �ð��� �����ϴ� �Լ�
class GameTimer
{
public:
	GameTimer();//������
	~GameTimer(); // �Ҹ���

	float TotalTime()const; //�ѽð�
	float DeltaTime()const; //������ ��ȭ�ð�

	void Reset(); //�ٽý���
	void Start(); //����
	void Stop(); //����
	void Tick(); //�����ѽð�

private:
	bool bStopTime;
	long long int baseTime;// �⺻ �ð�
	long long int pausedTime;// �Ͻ����� �ð�
	long long int stopTime;  // ���� �ð�
	long long int prevTime;  // �ռ� �ð�
	long long int currTime;  // ���� �ð�

	double secondsPerCount; //�ʴ�ī��Ʈ 
	double deltaTime;  //�����ð�
};
