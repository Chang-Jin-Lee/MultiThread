#include <iostream>
#include <windows.h>
#include <process.h>
#include <vector>

#define NUM_MINERS 5
#define GOLD_PER_TRIAL 10
#define NUM_TRIALS 5

int g_totalGold = 0; // 공유 자원: 팀의 총 골드
CRITICAL_SECTION cs_gold;

// 광부 스레드가 실행할 함수
unsigned int __stdcall mineGold(void* arg) {
	int minerId = *(int*)arg;

	for (int i = 0; i < NUM_TRIALS; ++i) {
		// 골드를 채굴하는 시간 시뮬레이션
		Sleep(10);

		// TODO 1: g_totalGold에 접근하기 전, 크리티컬 섹션에 진입하세요.
		// ...
		EnterCriticalSection(&cs_gold);

		// --- 경쟁 상태가 발생하는 구간 ---
		int currentGold = g_totalGold; // 1. 현재 골드량을 읽는다.
		printf("광부 %d, 현재 골드량 %d 확인 후 10골드 추가 시도...\n", minerId, currentGold);
		currentGold += GOLD_PER_TRIAL; // 2. 캔 골드를 더한다.
		Sleep(5); // 다른 스레드가 끼어들 수 있는 시간적인 틈을 만듦
		g_totalGold = currentGold;     // 3. 새로운 골드량으로 덮어쓴다.
		// ---------------------------------

		// TODO 2: g_totalGold 접근이 끝났으니, 크리티컬 섹션에서 빠져나오세요.
		// ...
		LeaveCriticalSection(&cs_gold);
	}
	return 0;
}

int main()
{
	// TODO 3: main 함수 시작 시, 크리티컬 섹션을 초기화하세요.
	// ...
	InitializeCriticalSection(&cs_gold);
	std::vector<HANDLE> threads;
	std::vector<int> minerIds(NUM_MINERS+1,0);

	printf("--- %d명의 광부가 채굴을 시작합니다! ---\n", NUM_MINERS);
	printf("기대 결과값: %d\n\n", NUM_MINERS * GOLD_PER_TRIAL * NUM_TRIALS);

	for (int i = 0; i < NUM_MINERS; ++i) {
		minerIds[i] = i + 1;
		threads.push_back((HANDLE)_beginthreadex(NULL, 0, mineGold, &minerIds[i], 0, NULL));
	}

	//TODO 4. 모든 스레드가 작업을 마칠 때까지 대기
	// ...
	WaitForMultipleObjects(NUM_MINERS, threads.data(), true, INFINITE);
	//if (WaitForSingleObject(hNotFull, INFINITE) != WAIT_OBJECT_0) return false;

	for (HANDLE h : threads) CloseHandle(h);

	// TODO 5: main 함수 종료 전, 크리티컬 섹션 리소스를 삭제하세요.
	// ...
	DeleteCriticalSection(&cs_gold);
	printf("\n--- 모든 채굴 작업 완료! ---\n");
	printf("최종 골드량: %d\n", g_totalGold);

	return 0;
}