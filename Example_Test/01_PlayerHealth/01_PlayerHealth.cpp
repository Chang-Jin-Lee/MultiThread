#include <iostream>
#include <windows.h>
#include <process.h>
using namespace std;

struct PlayerData
{
	int health;
	int maxHealth;
	bool isHealing;
};

unsigned __stdcall HealthRecoveryThread(void* param)
{
	PlayerData* player = (PlayerData*)param;

	std::cout << "체력 회복 시작! 현재 체력: " << player->health << "/" << player->maxHealth << std::endl;

	while (player->health < player->maxHealth) {
		Sleep(1000); // 1초 대기
		player->health += 10; // 10씩 회복
		if (player->health > player->maxHealth) {
			player->health = player->maxHealth;
		}

		std::cout << "체력 회복 중... " << player->health << "/" << player->maxHealth << std::endl;
	}

	player->isHealing = false;
	std::cout << "체력이 완전히 회복되었습니다!" << std::endl;
	return 0;
}

int main()
{
	PlayerData player = { 30, 100, false }; // 초기 체력 50, 최대 체력 100
	HANDLE hThread = (HANDLE)_beginthreadex(
		nullptr,
		0,
		HealthRecoveryThread,
		&player,
		0,
		nullptr
	);

	// hThread 체크
	if (hThread == nullptr) cout << "메인 게임 스레드 루프가 실행 중..." << endl;

	// 체력 회복이 끝날 때까지 게임을 대기상태로 유지
	WaitForSingleObject(hThread, INFINITE);

	cout << "체력 회복이 완료되었습니다." << endl;

	CloseHandle(hThread);
	return 0;
}