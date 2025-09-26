#include <iostream>
#include <windows.h>
#include <process.h>
#include <vector>

int potion_count = 10; // 공유 자원: 물약 10개
CRITICAL_SECTION cs;     // 크리티컬 섹션

unsigned int __stdcall usePotion(void* arg) 
{
	int playerId = *(int*)arg;

	// TODO: 크리티컬 섹션 시작
	// ...
	EnterCriticalSection(&cs);

	if (potion_count > 0) 
	{
		Sleep(10); // 물약을 마시는 데 시간이 걸리는 것을 시뮬레이션
		potion_count--;
		printf("Player %d가 물약을 사용했습니다. 남은 물약: %d\n", playerId, potion_count);
	}

	// TODO: 크리티컬 섹션 끝
	// ...
	LeaveCriticalSection(&cs);

	return 0;
}

int main()
{
	const int NUM_PLAYERS = 20;
	std::vector<HANDLE> threads;
	std::vector<int> playerIds(21,0);

	// TODO: 크리티컬 섹션 초기화
	// ...
	InitializeCriticalSection(&cs);

	for (int i = 0; i < NUM_PLAYERS; ++i) 
	{
		playerIds[i] = i + 1;
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, usePotion, &playerIds[i], 0, NULL);
		if (hThread) threads.push_back(hThread);
	}

	WaitForMultipleObjects(threads.size(), threads.data(), TRUE, INFINITE);

	for (HANDLE hThread : threads) CloseHandle(hThread);

	// TODO: 크리티컬 섹션 삭제
	// ...
	DeleteCriticalSection(&cs);

	printf("최종 남은 물약 개수: %d\n", potion_count);

	return 0;
}