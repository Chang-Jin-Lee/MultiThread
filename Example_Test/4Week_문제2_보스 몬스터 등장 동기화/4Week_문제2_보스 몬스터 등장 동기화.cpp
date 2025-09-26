#include <iostream>
#include <windows.h>
#include <process.h>

HANDLE g_hBossReadyEvent; // 보스 준비 완료 이벤트

// 보스 몬스터를 로딩하는 스레드 함수
unsigned int __stdcall loadBoss(void* arg) {
	printf("보스 몬스터를 로딩 중입니다...\n");
	Sleep(3000); // 3초 동안 로딩
	printf("보스 로딩 완료!\n");

	// TODO: 보스 준비 완료 이벤트를 Set 상태로 만드세요.
	// ...
	SetEvent(g_hBossReadyEvent);

	return 0;
}

// 플레이어가 전투에 참여하는 스레드 함수
unsigned int __stdcall enterBattle(void* arg) {
	int playerId = *(int*)arg;
	printf("Player %d, 보스 로딩을 기다리는 중...\n", playerId);

	// TODO: 보스 준비 완료 이벤트가 Set 상태가 될 때까지 기다리세요.
	// ...
	if (WaitForSingleObject(g_hBossReadyEvent, INFINITE) != WAIT_OBJECT_0) return false;

	printf("Player %d, 전투 시작!\n", playerId);
	return 0;
}

int main()
{
	// TODO: Manual-reset, non-signaled 상태의 이벤트를 생성하세요.
	// g_hBossReadyEvent = CreateEvent(...);

	g_hBossReadyEvent = CreateEvent(NULL, FALSE,FALSE, NULL);

	if (g_hBossReadyEvent == NULL) {
		printf("이벤트 생성 실패\n");
		return 1;
	}

	HANDLE hBossThread = (HANDLE)_beginthreadex(NULL, 0, loadBoss, NULL, 0, NULL);

	const int NUM_PLAYERS = 5;
	HANDLE hPlayerThreads[NUM_PLAYERS];
	int playerIds[NUM_PLAYERS];
	for (int i = 0; i < NUM_PLAYERS; ++i) {
		playerIds[i] = i + 1;
		hPlayerThreads[i] = (HANDLE)_beginthreadex(NULL, 0, enterBattle, &playerIds[i], 0, NULL);
	}

	WaitForSingleObject(hBossThread, INFINITE);
	WaitForMultipleObjects(NUM_PLAYERS, hPlayerThreads, TRUE, INFINITE);

	CloseHandle(hBossThread);
	for (int i = 0; i < NUM_PLAYERS; ++i) {
		CloseHandle(hPlayerThreads[i]);
	}
	// TODO: 생성한 이벤트를 닫으세요.
	// ...
	CloseHandle(g_hBossReadyEvent);

	return 0;
}