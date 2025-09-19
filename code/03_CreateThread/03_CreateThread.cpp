#include <windows.h>
#include <strsafe.h>   // StringCchPrintfA
#include <iostream>
#include <vector>
using namespace std;

struct ThreadData {
	int threadId;
	const char* message;
	int count;
};

// 스레드에서 사용할 간단한 WinAPI 로그 함수
void Logf(const char* fmt, ...)
{
	char buf[512];
	va_list ap; va_start(ap, fmt);
	StringCchVPrintfA(buf, 512, fmt, ap);
	va_end(ap);

	DWORD written = 0;
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut && hOut != INVALID_HANDLE_VALUE) {
		WriteConsoleA(hOut, buf, (DWORD)strlen(buf), &written, nullptr);
	}
	// 디버그 뷰에서도 보이게 하려면:
	OutputDebugStringA(buf);
}

// 반드시 WINAPI 규약
DWORD WINAPI WorkerThread(LPVOID lpParam)
{
	ThreadData* data = static_cast<ThreadData*>(lpParam);

	Logf("WorkerThread 시작 - ID:%d %s\r\n", data->threadId, data->message);

	for (int i = 1; i <= data->count; ++i) {
		Logf("스레드 %d: 작업 %d/%d 수행 중...\r\n",
			data->threadId, i, data->count);
		Sleep(1000);
	}

	Logf("스레드 %d: 모든 작업 완료!\r\n", data->threadId);
	return data->threadId * 100;
}

int main()
{
	cout << "CreateThread 예제입니다" << endl;

	const int threadSize = 2;
	vector<ThreadData> threadDatas = {
		{1, "첫 번째 워커 스레드", 3},
		{2, "두 번째 워커 스레드", 5}
	};
	vector<DWORD>  threadIds(threadSize, 0);
	vector<HANDLE> threads(threadSize);

	// 스레드 생성
	for (int i = 0; i < threadSize; ++i)
	{
		threads[i] = CreateThread(
			nullptr,                 // 보안
			0,                       // 스택
			WorkerThread,            // 함수
			&threadDatas[i],         // 인자
			0,                       // 즉시 실행
			&threadIds[i]            // 스레드 ID
		);

		if (threads[i] == nullptr) {
			cout << i << "번째 스레드 생성 실패! 오류 코드: " << GetLastError() << endl;
			// 이미 만든 핸들 정리
			for (int j = 0; j < i; ++j) CloseHandle(threads[j]);
			return 1;
		}
	}

	cout << "스레드 생성 완료!" << endl;
	for (int i = 0; i < threadSize; ++i)
		cout << "스레드 " << i << " ID: " << threadIds[i] << endl;

	// 메인 스레드 작업
	cout << "메인 스레드 작업 시작!" << endl;
	for (int i = 1; i <= 4; ++i) {
		cout << "메인 스레드: 작업 " << i << "/4" << endl;
		Sleep(800);
	}
	cout << "메인 스레드 작업 완료\n\n";

	// 모든 스레드 완료 대기
	cout << "모든 스레드 완료 대기 중...\n";
	DWORD wr = WaitForMultipleObjects(
		threadSize,
		threads.data(),
		TRUE,
		INFINITE
	);

	if (wr == WAIT_OBJECT_0) {
		cout << "모든 스레드가 완료 상태가 되었습니다.\n";
		for (int i = 0; i < threadSize; ++i) {
			DWORD exitCode = 0;
			GetExitCodeThread(threads[i], &exitCode);
			cout << "스레드 " << i << " 종료 코드: " << exitCode << endl;
			CloseHandle(threads[i]);     // 종료 코드 확인 후 닫기
		}
	}
	else {
		cout << "스레드 대기 실패! 오류 코드: " << GetLastError() << endl;
		for (auto h : threads) if (h) CloseHandle(h);
		return 1;
	}

	cout << "모든 스레드 핸들 정리 완료.. 프로그램 종료" << endl;
	return 0;
}
