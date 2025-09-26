#include <iostream>
#include <vector>
#include <windows.h>
#include <process.h> // _beginthreadex를 위해 필요

// 전역 변수
// 각 Task의 완료를 알리는 Event 핸들
HANDLE g_hEvent_A_Done;
HANDLE g_hEvent_B_Done;

// 스레드 함수들
// 각 배치 작업을 수행할 스레드 함수
unsigned int __stdcall BatchTaskThread(void* pParam)
{
	char taskName = *(static_cast<char*>(pParam));

	switch (taskName)
	{
	case 'A':
		std::cout << "[Task A] 시작: 데이터 준비 작업을 수행합니다.\n";
		Sleep(2000); // 2초간 데이터 준비 작업 시뮬레이션
		std::cout << "[Task A] 완료: 다음 작업을 위해 신호를 보냅니다.\n";
		SetEvent(g_hEvent_A_Done); // Task B를 위해 "A 작업 완료" Event를 신호 상태로 만듦
		break;
	case 'B':
		std::cout << "[Task B] 대기: Task A가 완료되기를 기다립니다...\n";
		WaitForSingleObject(g_hEvent_A_Done, INFINITE); // "A 작업 완료" 신호가 올 때까지 대기

		std::cout << "[Task B] 시작: 데이터 처리 작업을 수행합니다.\n";
		Sleep(3000); // 3초간 데이터 처리 작업 시뮬레이션
		std::cout << "[Task B] 완료: 다음 작업을 위해 신호를 보냅니다.\n";
		SetEvent(g_hEvent_B_Done); // Task C를 위해 "B 작업 완료" Event를 신호 상태로 만듦
		break;
	case 'C':
		std::cout << "[Task C] 대기: Task B가 완료되기를 기다립니다...\n";
		WaitForSingleObject(g_hEvent_B_Done, INFINITE); // "B 작업 완료" 신호가 올 때까지 대기

		std::cout << "[Task C] 시작: 결과 리포팅 작업을 수행합니다.\n";
		Sleep(1500); // 1.5초간 리포팅 작업 시뮬레이션
		std::cout << "[Task C] 완료: 모든 작업이 끝났습니다.\n";
		break;
	}

	delete static_cast<char*>(pParam);	// 메인에서 전달한 파라미터 메모리 해제
	return 0;
}

int main()
{
	std::cout << "[조정자] 배치 프로세스를 시작합니다.\n";
	
	// Event 생성

	g_hEvent_A_Done = CreateEvent(NULL, FALSE, FALSE, NULL);
	g_hEvent_B_Done = CreateEvent(NULL, FALSE, FALSE, NULL);

	HANDLE hThreads[3]; // 3개의 작업 스레드 핸들을 저장

	//  각 Task에 대한 스레드 생성
	std::cout << "[조정자] Task A, B, C 스레드를 생성하고 작업을 지시합니다.\n";

	char* taskA = new char('A');
	char* taskB = new char('B');
	char* taskC = new char('C');

	hThreads[0] = (HANDLE)_beginthreadex(NULL, 0, BatchTaskThread, taskA, 0, NULL); // Task A
	hThreads[1] = (HANDLE)_beginthreadex(NULL, 0, BatchTaskThread, taskB, 0, NULL); // Task B
	hThreads[2] = (HANDLE)_beginthreadex(NULL, 0, BatchTaskThread, taskC, 0, NULL); // Task C

	// 최종 결과 대기
	std::cout << "[조정자] 모든 작업이 완료되기를 기다립니다...\n";
	// 마지막 작업(Task C)이 끝날 때까지만 기다리면 전체 배치가 완료된 것임
	WaitForSingleObject(hThreads[2], INFINITE);

	std::cout << "[조정자] 배치 프로세스가 모두 완료되었습니다.\n";

	// 리소스 정리
	CloseHandle(hThreads[0]);
	CloseHandle(hThreads[1]);
	CloseHandle(hThreads[2]);
	CloseHandle(g_hEvent_A_Done);
	CloseHandle(g_hEvent_B_Done);

	return 0;
}

