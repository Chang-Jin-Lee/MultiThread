#include <windows.h>
#include <iostream>
#include <process.h>

// C++에서의 스레드 생성 방법
class ThreadWorker
{
private:
	int m_workerId;
	bool m_bStop;
	HANDLE m_hThread;
	unsigned m_threadId;

public:
	ThreadWorker(int workerId) : m_workerId(workerId), m_bStop(false), m_hThread(NULL), m_threadId(0) {}
	
	~ThreadWorker()
	{
		Stop();
	}

	bool Start()
	{
		if (m_hThread != NULL) return false; // 이미 스레드가 실행 중임

		m_bStop = false;
		m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(
			NULL,               // 보안 속성
			0,                  // 기본 스택 크기
			ThreadProc,        // 스레드 함수
			this,              // 스레드 함수에 전달할 인자
			0,                  // 즉시 실행
			&m_threadId));     // 스레드 ID
		return m_hThread != NULL;
	}

	void Stop()
	{
		if (m_hThread == NULL) return;

		m_bStop = true;
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
		m_threadId = 0;
	}

private:
	static unsigned __stdcall ThreadProc(void* pParam)
	{
		ThreadWorker* worker = static_cast<ThreadWorker*>(pParam);
		return worker->WorkerFunction();
	}

	unsigned WorkerFunction()
	{
		std::cout << "워커 " << m_workerId << " 시작 (스레드 ID: " << m_threadId << ")" << std::endl;

		int iteration = 0;
		while (!m_bStop && iteration < 10)
		{
			std::cout << "워커 " << m_workerId << " 작업 중... " << ++iteration << std::endl;
			Sleep(500);
		}

		std::cout << "워커 " << m_workerId << " 종료" << std::endl;
		return 0;
	}
};

void DemonstrateThreadSafeWorker()
{
	std::cout << "=== 스레드 안전 워커 클래스 ===" << std::endl;

	ThreadWorker worker(100);

	if (worker.Start())
	{
		std::cout << "워커 시작됨" << std::endl;
		Sleep(3000);  // 3초 후 종료
		worker.Stop();
		std::cout << "워커 정리 완료\n" << std::endl;
	}
}

int main()
{
	DemonstrateThreadSafeWorker();
	return 0;
}