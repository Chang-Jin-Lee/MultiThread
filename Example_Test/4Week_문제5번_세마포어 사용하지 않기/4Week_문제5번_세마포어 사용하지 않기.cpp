#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <memory>
#include <process.h>
#include <windows.h> // Win32 API 헤더

// 데이터베이스 연결 풀 시뮬레이션
class DatabaseConnection
{
private:
	int connectionId;
	bool isConnected;

public:
	DatabaseConnection(int id) : connectionId(id), isConnected(false) {}

	bool Connect()
	{
		Sleep(100);  // 연결 시뮬레이션
		isConnected = true;
		std::cout << "  연결 " << connectionId << " 활성화\n";
		return true;
	}

	void Disconnect()
	{
		isConnected = false;
		Sleep(50);  // 연결 해제 시뮬레이션
		std::cout << "  연결 " << connectionId << " 비활성화\n";
	}

	void ExecuteQuery(const std::string& query)
	{
		if (isConnected)
		{
			std::cout << "  연결 " << connectionId << "에서 쿼리 실행: " << query << "\n";
			Sleep(500 + (rand() % 1000));  // 쿼리 실행 시뮬레이션
		}
	}

	int GetId() const { return connectionId; }
};

class DatabaseConnectionPool
{
private:
	std::vector<std::unique_ptr<DatabaseConnection>> connections;
	std::queue<DatabaseConnection*> availableConnections;
	CRITICAL_SECTION cs;

public:
	DatabaseConnectionPool(int poolSize) {
		InitializeCriticalSection(&cs);

		// 연결 객체들 생성
		for (int i = 0; i < poolSize; ++i)
		{
			auto conn = std::make_unique<DatabaseConnection>(i + 1);
			conn->Connect();

			EnterCriticalSection(&cs);
			availableConnections.push(conn.get());
			connections.push_back(std::move(conn));
			LeaveCriticalSection(&cs);
		}

		std::cout << "연결 풀 생성 완료 (크기: " << poolSize << ")\n\n";
	}

	~DatabaseConnectionPool()
	{
		// 모든 연결 해제
		for (auto& conn : connections)
		{
			conn->Disconnect();
		}

		DeleteCriticalSection(&cs);
	}

	DatabaseConnection* AcquireConnection(DWORD timeout = INFINITE)
	{
		DWORD elapsed = 0;
		const DWORD interval = 10; // 폴링 간격 (ms)

		DatabaseConnection* conn = nullptr;
		while (true) {
			EnterCriticalSection(&cs);
			if (!availableConnections.empty()) {
				conn = availableConnections.front();
				availableConnections.pop();
				LeaveCriticalSection(&cs);
				break;
			}
			LeaveCriticalSection(&cs);

			// 1. 연결이 없을 경우, 잠시 대기하고 재시도 (폴링)
			if (timeout != INFINITE && elapsed >= timeout) {
				return nullptr; // 타임아웃
			}
			Sleep(interval);
			elapsed += interval;
		}
		return conn;
	}

	void ReleaseConnection(DatabaseConnection* conn)
	{
		if (!conn) return;

		EnterCriticalSection(&cs);
		availableConnections.push(conn);
		LeaveCriticalSection(&cs);
	}

	size_t GetAvailableCount()
	{
		EnterCriticalSection(&cs);
		size_t count = availableConnections.size();
		LeaveCriticalSection(&cs);
		return count;
	}
};

// RAII 패턴으로 자동 해제 보장
class ScopedConnection
{
private:
	DatabaseConnectionPool* pool;
	DatabaseConnection* connection;

public:
	ScopedConnection(DatabaseConnectionPool* p, DWORD timeout = INFINITE)
		: pool(p), connection(nullptr)
	{
		connection = pool->AcquireConnection(timeout);
	}

	~ScopedConnection()
	{
		if (connection && pool) {
			pool->ReleaseConnection(connection);
		}
	}

	DatabaseConnection* Get() const { return connection; }
	bool IsValid() const { return connection != nullptr; }
};


// 클라이언트 스레드에 전달할 데이터 구조체
struct ClientThreadParams
{
	DatabaseConnectionPool* pool;
	int clientId;
};

// 클라이언트 스레드 함수
unsigned int __stdcall ClientThreadFunc(void* lpParam)
{
	ClientThreadParams* params = static_cast<ClientThreadParams*>(lpParam);
	DatabaseConnectionPool* pool = params->pool;
	int clientId = params->clientId;

	std::cout << "클라이언트 " << clientId << " 시작\n";

	// RAII 패턴으로 연결 관리
	ScopedConnection scopedConn(pool, 3000);  // 3초 타임아웃

	if (scopedConn.IsValid()) {
		DatabaseConnection* conn = scopedConn.Get();
		std::cout << "클라이언트 " << clientId << "가 연결 "
			<< conn->GetId() << " 획득\n";

		// 여러 쿼리 실행
		conn->ExecuteQuery("SELECT * FROM users WHERE id = " + std::to_string(clientId));
		conn->ExecuteQuery("UPDATE stats SET count = count + 1");

		std::cout << "클라이언트 " << clientId << " 작업 완료\n";
	}
	else
	{
		std::cout << "클라이언트 " << clientId << " 연결 획득 실패 (타임아웃)\n";
	}

	// scopedConn이 소멸되면서 자동으로 연결 반납

	delete params; // 스레드로 전달된 데이터 구조체 메모리 해제
	return 0;
}

// 모니터 스레드 함수
DWORD WINAPI MonitorThreadFunc(LPVOID lpParam)
{
	DatabaseConnectionPool* pool = static_cast<DatabaseConnectionPool*>(lpParam);
	for (int i = 0; i < 10; ++i) {
		Sleep(1000);
		std::cout << "[모니터] 사용 가능한 연결: "
			<< pool->GetAvailableCount() << "/3\n";
	}
	return 0;
}


// 연결 풀 테스트
void TestConnectionPool()
{
	const int POOL_SIZE = 3;
	const int CLIENT_COUNT = 8;

	DatabaseConnectionPool pool(POOL_SIZE);
	std::vector<HANDLE> clientHandles; // 스레드 핸들을 저장할 벡터

	std::cout << "=== 데이터베이스 연결 풀 테스트 ===\n";
	std::cout << "풀 크기: " << POOL_SIZE << "\n";
	std::cout << "클라이언트 수: " << CLIENT_COUNT << "\n\n";

	// 클라이언트 스레드 생성
	for (int i = 0; i < CLIENT_COUNT; ++i) {
		// 스레드에 전달할 파라미터를 동적으로 할당
		ClientThreadParams* params = new ClientThreadParams{ &pool, i };

		uintptr_t hThreadRaw = _beginthreadex(
			NULL,                   // 기본 보안 속성
			0,                      // 기본 스택 크기
			ClientThreadFunc,       // 스레드 함수
			params,                 // 스레드 함수에 전달할 인자
			0,                      // 즉시 실행
			NULL                    // 스레드 ID는 받지 않음
		);

		if (hThreadRaw != 0) {
			clientHandles.push_back((HANDLE)hThreadRaw);
		}
	}

	// 모니터 스레드 생성
	HANDLE hMonitorThread = CreateThread(NULL, 0, MonitorThreadFunc, &pool, 0, NULL);

	// 모든 클라이언트 스레드가 끝날 때까지 대기
	WaitForMultipleObjects(clientHandles.size(), clientHandles.data(), TRUE, INFINITE);

	// 모니터 스레드가 끝날 때까지 대기
	if (hMonitorThread)
	{
		WaitForSingleObject(hMonitorThread, INFINITE);
	}

	// 모든 스레드 핸들 닫기
	for (HANDLE h : clientHandles)
	{
		CloseHandle(h);
	}

	if (hMonitorThread)
	{
		CloseHandle(hMonitorThread);
	}

	std::cout << "\n모든 클라이언트 작업 완료\n";
}

int main()
{
	srand(static_cast<unsigned int>(time(NULL))); // rand() 함수 시드 초기화
	TestConnectionPool();
	return 0;
}