#include <iostream>
#include <vector>
#include <queue>
#include <windows.h>
#include <process.h> // _beginthreadex를 위해 필요

// -----------------------------------------------------------------------------
// 생산자-소비자 패턴을 위한 스레드 안전 큐 (Thread-Safe Queue)
// -----------------------------------------------------------------------------
// 설명: 이 클래스는 마치 물탱크처럼 동작합니다.
// - 생산자(Producer)는 물탱크에 물(데이터)을 채웁니다.
// - 소비자(Consumer)는 물탱크에서 물(데이터)을 빼서 사용합니다.
// - 세마포어(Semaphore): 물이 얼마나 찼는지, 빈 공간이 얼마나 남았는지 세는 카운터.
// - 이벤트(Event): 수위가 너무 낮거나 높을 때 울리는 경보 장치.
// -----------------------------------------------------------------------------
class MonitoredQueue {
private:
	std::queue<int> queue;
	CRITICAL_SECTION cs;         // 큐에 동시 접근을 막기 위한 자물쇠

	HANDLE hNotEmpty;            // 세마포어: 큐에 있는 아이템 개수 (소비자가 기다림)
	HANDLE hNotFull;             // 세마포어: 큐의 남은 공간 개수 (생산자가 기다림)
	HANDLE hLowWaterMark;        // 이벤트: 아이템 개수가 너무 적을 때 울리는 '낮은 수위' 경보
	HANDLE hHighWaterMark;       // 이벤트: 아이템 개수가 꽉 차갈 때 울리는 '높은 수위' 경보

	size_t capacity;             // 큐의 최대 용량
	size_t lowThreshold;         // 낮은 수위 경보 기준
	size_t highThreshold;        // 높은 수위 경보 기준


	// 큐의 현재 크기에 따라 수위 경보(Event)를 켜거나 끄는 도우미 함수
	void UpdateWaterMarks(size_t currentSize) 
	{
		if (currentSize <= lowThreshold) SetEvent(hLowWaterMark); // 수위가 낮음 -> 경보 ON
		else ResetEvent(hLowWaterMark); // 수위가 낮지 않음 -> 경보 OFF

		if (currentSize >= highThreshold) SetEvent(hHighWaterMark); // 수위가 높음 -> 경보 ON
		else ResetEvent(hHighWaterMark); // 수위가 높지 않음 -> 경보 OFF
	}

public:
	MonitoredQueue(size_t cap, size_t lowThresh, size_t highThresh)
		: capacity(cap), lowThreshold(lowThresh), highThreshold(highThresh) {

		InitializeCriticalSection(&cs);

		// 세마포어 생성
		// hNotEmpty: 처음엔 큐가 비어있으므로 0으로 시작
		hNotEmpty = CreateSemaphore(NULL, 0, capacity, NULL);
		// hNotFull: 처음엔 큐가 꽉 비어있으므로 capacity로 시작
		hNotFull = CreateSemaphore(NULL, capacity, capacity, NULL);

		// 이벤트 생성 (수동 리셋 모드)
		// hLowWaterMark: 큐가 비어있어 낮은 수위이므로 TRUE (신호 켜짐)로 시작
		hLowWaterMark = CreateEvent(NULL, TRUE, TRUE, NULL);
		// hHighWaterMark: 큐가 비어있어 높은 수위가 아니므로 FALSE (신호 꺼짐)로 시작
		hHighWaterMark = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	~MonitoredQueue() {
		DeleteCriticalSection(&cs);
		if (hNotEmpty) CloseHandle(hNotEmpty);
		if (hNotFull) CloseHandle(hNotFull);
		if (hLowWaterMark) CloseHandle(hLowWaterMark);
		if (hHighWaterMark) CloseHandle(hHighWaterMark);
	}

	// 생산자가 큐에 아이템을 추가하는 함수
	bool Enqueue(const int& item) 
	{
		// 1. 큐에 빈 공간이 생길 때까지 대기 (hNotFull 카운터가 0보다 커질 때까지)
		if (WaitForSingleObject(hNotFull, INFINITE) != WAIT_OBJECT_0) return false;

		// 2. 자물쇠를 걸고 큐에 아이템 추가
		EnterCriticalSection(&cs);
		queue.push(item);
		size_t currentSize = queue.size();
		LeaveCriticalSection(&cs);

		// 3. 수위 경보 상태 업데이트
		UpdateWaterMarks(currentSize);

		// 4. "아이템이 하나 추가됐다"고 신호 (hNotEmpty 카운터 1 증가)
		ReleaseSemaphore(hNotEmpty, 1, NULL);

		return true;
	}

	// 소비자가 큐에서 아이템을 가져가는 함수
	bool Dequeue(int& item) 
	{
		// 1. 큐에 아이템이 생길 때까지 대기 (hNotEmpty 카운터가 0보다 커질 때까지)
		if (WaitForSingleObject(hNotEmpty, INFINITE) != WAIT_OBJECT_0) return false;

		// 2. 자물쇠를 걸고 큐에서 아이템 꺼내기
		EnterCriticalSection(&cs);
		item = queue.front();
		queue.pop();
		size_t currentSize = queue.size();
		LeaveCriticalSection(&cs);

		// 3. 수위 경보 상태 업데이트
		UpdateWaterMarks(currentSize);

		// 4. "빈 공간이 하나 생겼다"고 신호 (hNotFull 카운터 1 증가)
		ReleaseSemaphore(hNotFull, 1, NULL);

		return true;
	}

	// 모니터 스레드가 경보 이벤트를 감시할 수 있도록 핸들을 반환
	HANDLE GetLowWaterMarkEvent() const { return hLowWaterMark; }
	HANDLE GetHighWaterMarkEvent() const { return hHighWaterMark; }
};

// -----------------------------------------------------------------------------
// 스레드 테스트 로직
// -----------------------------------------------------------------------------

// 모든 스레드에 공통적으로 필요한 데이터를 전달하기 위한 구조체
struct ThreadParams
{
	MonitoredQueue* queue;
	HANDLE hShutdownEvent; // 모든 스레드를 안전하게 종료시키기 위한 이벤트
};

// [역할 1] 생산자 스레드: 큐에 데이터를 계속해서 넣는 역할
unsigned int __stdcall ProducerThread(void* pParam) 
{
	ThreadParams* params = static_cast<ThreadParams*>(pParam);
	int item = 0;

	// 종료 신호(Shutdown Event)가 오기 전까지 계속 실행
	while (WaitForSingleObject(params->hShutdownEvent, 0) != WAIT_OBJECT_0) 
	{
		item++;
		std::cout << " -> 생산: " << item << std::endl;
		params->queue->Enqueue(item);
		Sleep(200); // 너무 빠르지 않게 조절
	}
	std::cout << "생산자 스레드 종료.\n";
	return 0;
}

// [역할 2] 소비자 스레드: 큐에서 데이터를 계속해서 빼는 역할
unsigned int __stdcall ConsumerThread(void* pParam)
{
	ThreadParams* params = static_cast<ThreadParams*>(pParam);
	int item = 0;

	// 종료 신호가 오기 전까지 계속 실행
	while (WaitForSingleObject(params->hShutdownEvent, 0) != WAIT_OBJECT_0) 
	{
		if (params->queue->Dequeue(item)) std::cout << "              <- 소비: " << item << std::endl;
		Sleep(500); // 생산자보다 조금 느리게 조절
	}
	std::cout << "소비자 스레드 종료.\n";
	return 0;
}

// [역할 3] 모니터 스레드: 큐의 수위를 감시하고 경보를 출력하는 역할
unsigned int __stdcall MonitorThread(void* pParam)
{
	ThreadParams* params = static_cast<ThreadParams*>(pParam);
	HANDLE events[] = {
		params->queue->GetLowWaterMarkEvent(),
		params->queue->GetHighWaterMarkEvent(),
		params->hShutdownEvent // 종료 신호도 함께 감시
	};

	while (true) {
		// 3개의 이벤트 중 하나라도 신호가 오면 즉시 깨어남
		DWORD result = WaitForMultipleObjects(3, events, FALSE, INFINITE);

		switch (result) 
		{
		case WAIT_OBJECT_0: // LowWaterMark 이벤트
			std::cout << "\n           🔵 [모니터] 경고: 큐가 거의 비었습니다!\n\n";
			break;
		case WAIT_OBJECT_0 + 1: // HighWaterMark 이벤트
			std::cout << "\n           🔴 [모니터] 경고: 큐가 거의 꽉 찼습니다!\n\n";
			break;
		case WAIT_OBJECT_0 + 2: // Shutdown 이벤트
			std::cout << "모니터 스레드 종료.\n";
			return 0; // 스레드 종료
		}
	}
	return 0;
}

void TestMonitoredQueue()
{
	const size_t CAPACITY = 10;
	const size_t LOW_THRESHOLD = 2;
	const size_t HIGH_THRESHOLD = 8;

	MonitoredQueue queue(CAPACITY, LOW_THRESHOLD, HIGH_THRESHOLD);

	// 모든 스레드를 한 번에 종료시키기 위한 이벤트 생성
	HANDLE hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	ThreadParams params = { &queue, hShutdownEvent };
	std::vector<HANDLE> hThreads;

	// 스레드 생성
	hThreads.push_back((HANDLE)_beginthreadex(NULL, 0, MonitorThread, &params, 0, NULL));
	hThreads.push_back((HANDLE)_beginthreadex(NULL, 0, ProducerThread, &params, 0, NULL));
	hThreads.push_back((HANDLE)_beginthreadex(NULL, 0, ConsumerThread, &params, 0, NULL));

	std::cout << "--- 모니터링 큐 테스트 시작 (10초 후 자동 종료) ---\n";
	Sleep(10000); // 10초 동안 시뮬레이션 실행

	// --- 종료 처리 ---
	std::cout << "\n--- 모든 스레드에 종료 신호를 보냅니다... ---\n";
	SetEvent(hShutdownEvent); // 모든 스레드에게 종료하라고 알림

	// 모든 스레드가 완전히 끝날 때까지 대기
	WaitForMultipleObjects(hThreads.size(), hThreads.data(), TRUE, INFINITE);

	std::cout << "--- 모든 스레드가 안전하게 종료되었습니다. ---\n";

	// 핸들 정리
	for (HANDLE h : hThreads) CloseHandle(h);
	CloseHandle(hShutdownEvent);
}

int main()
{
	TestMonitoredQueue();
	return 0;
}