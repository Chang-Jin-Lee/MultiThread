#include <iostream>
#include <windows.h>
#include <process.h>
#include <string>

struct ResourceData
{
	std::string name;
	bool isEssential; // 필수 리소스 여부
	int loadTime;     // 로딩 시간 (초)
	bool isLoaded;    // 로딩 완료 여부
};

unsigned __stdcall ResourceLoadThread(void* param)
{
	ResourceData* resource = (ResourceData*)param;

	std::cout << "[로딩 시작] " << resource->name;
	if (resource->isEssential) {
		std::cout << " (필수)";
	}
	std::cout << std::endl;

	for (int i = 1; i <= resource->loadTime; ++i) {
		Sleep(1000);
		std::cout << resource->name << " 로딩 중... ("
			<< i << "/" << resource->loadTime << ")" << std::endl;
	}

	resource->isLoaded = true;
	std::cout << "[로딩 완료] " << resource->name << std::endl;
	return 0;
}

int main()
{
	// 리소스 데이터 설정
	ResourceData resources[4] = {
		{"기본 텍스처", true, 2},    // 필수
		{"플레이어 모델", true, 3},   // 필수
		{"배경음악", false, 5},       // 선택적
		{"효과음", false, 4}          // 선택적
	};

	HANDLE hThreads[4];
	HANDLE hEssentialThreads[2]; // 필수 리소스만
	int essentialCount = 0;

	// 모든 리소스 로딩 스레드 생성
	for (int i = 0; i < 4; ++i) {
		hThreads[i] = (HANDLE)_beginthreadex(
			NULL, 0, ResourceLoadThread, &resources[i], 0, NULL
		);

		if (hThreads[i] == NULL) {
			std::cout << "스레드 " << i << " 생성 실패!" << std::endl;
			return -1;
		}

		// 1번 =======================================================================
		// 필수 리소스 핸들 따로 저장
		if (resources[i].isEssential) 
		{
			hEssentialThreads[essentialCount++] = hThreads[i];
		}
	}

	std::cout << "\n게임 리소스 로딩을 시작합니다..." << std::endl;
	std::cout << "필수 리소스 로딩 완료 시 게임이 시작됩니다.\n" << std::endl;

	// 2번 =======================================================================
	// 필수 리소스만 완료되기를 대기 (bWaitAll = TRUE)
	WaitForMultipleObjects(essentialCount, hEssentialThreads, TRUE, INFINITE);

	std::cout << "\n=== 필수 리소스 로딩 완료! 게임을 시작합니다! ===" << std::endl;
	std::cout << "선택적 리소스는 백그라운드에서 계속 로딩됩니다...\n" << std::endl;

	// 게임 시뮬레이션 (3초)
	for (int i = 1; i <= 3; ++i) {
		Sleep(1000);
		std::cout << "게임 플레이 중... (" << i << "/3)" << std::endl;
	}

	std::cout << "\n모든 리소스 로딩 완료를 기다립니다..." << std::endl;

	// 3번 =======================================================================
	// 나머지 모든 리소스 완료 대기
	DWORD hThreadsCount = sizeof(hThreads) / sizeof(hThreads[0]);
	WaitForMultipleObjects(hThreadsCount, hThreads, TRUE, INFINITE);

	std::cout << "\n모든 리소스 로딩이 완료되었습니다!" << std::endl;
	std::cout << "이제 모든 게임 기능을 사용할 수 있습니다." << std::endl;

	// 핸들 정리
	for (int i = 0; i < 4; ++i) {
		CloseHandle(hThreads[i]);
	}

	return 0;
}