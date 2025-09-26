#include <iostream>
#include <windows.h>
#include <process.h>
#include <vector>

#define MAX_QUEUE_SIZE 8
#define NUM_UNITS 3      // 생산자 스레드 수
#define NUM_ACTIONS 7    // 각 유닛이 생성할 액션 수

// --- 공유 자원 ---
int g_actionQueue[MAX_QUEUE_SIZE];
int g_itemCount = 0;
int g_writeIndex = 0;
int g_readIndex = 0;
CRITICAL_SECTION cs_queue;

// 생산자: AI 유닛 스레드
unsigned int __stdcall aiUnitThread(void* arg) {
	int unitId = *(int*)arg;
	srand(unitId); // 각 스레드마다 다른 시드값 부여

	for (int i = 0; i < NUM_ACTIONS; ++i) {
		int newAction = unitId * 100 + i; // 유닛 고유의 액션 생성
		bool success = false;

		while (!success) {
			// TODO 1: 큐에 접근하기 위해 크리티컬 섹션에 진입하세요.
			// ...
			EnterCriticalSection(&cs_queue);
			if (g_itemCount < MAX_QUEUE_SIZE) {
				// 큐에 자리가 있을 때의 로직
				g_actionQueue[g_writeIndex] = newAction;
				g_writeIndex = (g_writeIndex + 1) % MAX_QUEUE_SIZE;
				g_itemCount++;
				printf("유닛 %d: 액션 %d 큐에 등록. (현재 %d개)\n", unitId, newAction, g_itemCount);
				success = true;
			}
			else {
				// 큐가 가득 찼을 때의 로직
				printf(">> 유닛 %d: 큐가 가득 차 등록에 실패했습니다. 잠시 대기...\n", unitId);
			}

			// TODO 2: 큐 접근이 끝났으니 크리티컬 섹션을 빠져나오세요.
			// ...
			LeaveCriticalSection(&cs_queue);
			if (!success) {
				Sleep(100); // 큐가 꽉 찼으면 잠시 대기 후 재시도
			}
		}
		Sleep(rand() % 150); // 새로운 액션을 생각하는 시간
	}
	return 0;
}

// 소비자: 게임 엔진 스레드
unsigned int __stdcall gameEngineThread(void* arg) {
	int totalActionsToProcess = NUM_UNITS * NUM_ACTIONS;
	int processedCount = 0;

	while (processedCount < totalActionsToProcess) {
		// TODO 3: 큐에 접근하기 위해 크리티컬 섹션에 진입하세요.
		// ...
		EnterCriticalSection(&cs_queue);
		if (g_itemCount > 0) {
			// 큐에 아이템이 있을 때의 로직
			int action = g_actionQueue[g_readIndex];
			g_readIndex = (g_readIndex + 1) % MAX_QUEUE_SIZE;
			g_itemCount--;
			processedCount++;
			printf("엔진: 액션 %d 처리 완료. (남은 %d개, 총 %d개 처리)\n", action, g_itemCount, processedCount);
		}
		else {
			// 큐가 비어있을 때의 로직
			// (실제 엔진이라면 대기하겠지만, 여기서는 메시지만 출력)
			printf(" 큐가 비어있습니다... 대기 중 ... \n");
		}

		// TODO 4: 큐 접근이 끝났으니 크리티컬 섹션을 빠져나오세요.
		// ...
		LeaveCriticalSection(&cs_queue);
		Sleep(50); // 한 액션을 처리하는데 걸리는 시간
	}
	return 0;
}


int main()
{
	// TODO 5: 크리티컬 섹션을 초기화하세요.
	// ...
	InitializeCriticalSection(&cs_queue);
	std::vector<HANDLE> unitThreads;
	std::vector<int> unitIds(NUM_UNITS+1, 0);

	// 게임 엔진 스레드 시작
	HANDLE hEngineThread = (HANDLE)_beginthreadex(NULL, 0, gameEngineThread, NULL, 0, NULL);

	// AI 유닛 스레드들 시작
	for (int i = 0; i < NUM_UNITS; ++i) {
		unitIds[i] = i + 1;
		unitThreads.push_back((HANDLE)_beginthreadex(NULL, 0, aiUnitThread, &unitIds[i], 0, NULL));
	}

	WaitForMultipleObjects(unitThreads.size(), unitThreads.data(), TRUE, INFINITE);
	WaitForSingleObject(hEngineThread, INFINITE);

	for (HANDLE h : unitThreads) CloseHandle(h);
	CloseHandle(hEngineThread);

	// TODO 6: 크리티컬 섹션을 삭제하세요.
	// ...
	DeleteCriticalSection(&cs_queue);
	printf("\n모든 게임 액션 처리 완료.\n");
	return 0;
}