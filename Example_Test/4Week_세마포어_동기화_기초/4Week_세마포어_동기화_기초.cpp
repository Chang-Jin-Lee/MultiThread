#include <windows.h>
#include <iostream>
#include <vector>
#include <process.h> // _beginthreadex, _endthreadex를 위해 필요

class ConnectionPool
{
private:
	HANDLE hSemaphore;
	static const int MAX_CONNECTIONS = 3;

public:
	ConnectionPool() 
	{
		// 세마포어 생성 (초기값: 3, 최대값: 3)
		hSemaphore = CreateSemaphore(
			NULL,              // 보안 속성
			MAX_CONNECTIONS,   // 초기 카운트
			MAX_CONNECTIONS,   // 최대 카운트
			NULL               // 이름
		);

		if (hSemaphore == NULL) throw std::runtime_error("세마포어 생성 실패");
	}

	~ConnectionPool() 
	{
		if (hSemaphore) CloseHandle(hSemaphore);
	}

	void UseConnection(int threadId) 
	{
		std::cout << "스레드 " << threadId << ": 연결 요청\n";

		// 사용 가능한 연결 대기
		DWORD waitResult = WaitForSingleObject(hSemaphore, INFINITE);

		if (waitResult == WAIT_OBJECT_0) 
		{
			std::cout << "스레드 " << threadId << ": 연결 획득\n";

			// 연결 사용 시뮬레이션
			Sleep(2000 + (rand() % 3000)); // 2-5초 랜덤 작업

			std::cout << "스레드 " << threadId << ": 연결 해제\n";

			// 세마포어 해제 (연결 반납)
			ReleaseSemaphore(hSemaphore, 1, NULL);
		}
	}
};

// _beginthreadex에 전달할 데이터를 담을 구조체
struct ThreadData
{
	ConnectionPool* pool;
	int threadId;
};

// 스레드 함수
// _beginthreadex는 static 멤버 함수 또는 전역 함수를 요구합니다.
unsigned int __stdcall ThreadFunction(void* pArguments)
{
	ThreadData* data = static_cast<ThreadData*>(pArguments);
	if (data)  data->pool->UseConnection(data->threadId);
	delete data; // 동적으로 할당된 데이터 해제
	_endthreadex(0);
	return 0;
}

// 사용 예제
void TestConnectionPool()
{
	ConnectionPool pool;
	const int NUM_THREADS = 5;
	std::vector<HANDLE> threadHandles;

	// 5개 스레드가 3개 연결을 두고 경쟁
	for (int i = 1; i <= NUM_THREADS; ++i) 
	{
		// 스레드에 전달할 데이터를 동적으로 생성
		ThreadData* data = new ThreadData{ &pool, i };

		HANDLE hThread = (HANDLE)_beginthreadex(
			NULL,                   // 보안 속성
			0,                      // 스택 크기 (0 = 기본값)
			ThreadFunction,         // 스레드 함수
			data,                   // 스레드 함수에 전달할 인자
			0,                      // 생성 플래그 (0 = 즉시 실행)
			NULL                    // 스레드 ID (필요 없는 경우 NULL)
		);

		if (hThread) threadHandles.push_back(hThread);
		else delete data; // 스레드 생성 실패 시 메모리 해제
	}

	// 모든 스레드가 끝날 때까지 대기
	WaitForMultipleObjects(threadHandles.size(), threadHandles.data(), TRUE, INFINITE);

	// 스레드 핸들 닫기
	for (HANDLE h : threadHandles) CloseHandle(h);
}

int main()
{
	srand(time(NULL));
	TestConnectionPool();
	return 0;
}