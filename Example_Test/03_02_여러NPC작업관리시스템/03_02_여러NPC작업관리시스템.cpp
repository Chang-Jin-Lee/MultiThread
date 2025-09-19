#include <iostream>
#include <windows.h>
#include <process.h>
#include <string>

struct NPCData {
	std::string name;
	std::string job;
	int workTime; // 작업 시간 (초)
};

unsigned __stdcall NPCWorkThread(void* param) 
{
	NPCData* npc = static_cast<NPCData*>(param);

	std::cout << npc->name << "이(가) " << npc->job << " 작업을 시작합니다." << std::endl;

	for (int i = 1; i <= npc->workTime; ++i) {
		Sleep(1000);
		std::cout << npc->name << ": " << npc->job << " 진행 중... ("
			<< i << "/" << npc->workTime << ")" << std::endl;
	}

	std::cout << npc->name << "이(가) " << npc->job << " 작업을 완료했습니다!" << std::endl;
	return 0;
}

int main() {
	// NPC 데이터 설정
	NPCData npcs[3] = {
		{"상인 앤", "상점 운영", 3},
		{"경비병 밥", "마을 경비", 5},
		{"대장장이 찰리", "무기 제작", 4}
	};

	HANDLE hThreads[3];

	// 모든 NPC 스레드 생성
	for (int i = 0; i < 3; ++i) {
		hThreads[i] = (HANDLE)_beginthreadex(
			nullptr,
			0,
			NPCWorkThread,
			&npcs[i],
			0,
			nullptr
		);

		if (hThreads[i] == NULL) {
			std::cout << "스레드 " << i << " 생성 실패!" << std::endl;
			return -1;
		}
	}

	std::cout << "모든 NPC가 작업을 시작했습니다. 완료를 기다리는 중..." << std::endl;

	// 모든 NPC 작업 완료까지 대기
	WaitForMultipleObjectsEx(3, hThreads, TRUE, INFINITE, FALSE);

	std::cout << "\n 모든 NPC 작업이 완료되었습니다! 마을이 정상 운영됩니다." << std::endl;

	// 핸들 정리
	for (int i = 0; i < 3; ++i) CloseHandle(hThreads[i]);

	return 0;
}