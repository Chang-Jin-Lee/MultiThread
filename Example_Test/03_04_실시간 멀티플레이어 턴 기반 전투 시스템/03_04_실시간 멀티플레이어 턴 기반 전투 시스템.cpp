#include <iostream>
#include <windows.h>
#include <process.h>
#include <string>
#include <cstdlib>
#include <ctime>

struct PlayerAction {
	int playerId;
	std::string playerName;
	std::string action;
	bool hasSelected;
	int thinkingTime; // 사고 시간 (초)
};

unsigned __stdcall PlayerThinkingThread(void* param)
{
	PlayerAction* player = (PlayerAction*)param;

	std::cout << player->playerName << "이(가) 행동을 고민하고 있습니다..." << std::endl;

	// 랜덤한 사고 시간 (1-5초)
	Sleep(player->thinkingTime * 1000);

	// 랜덤 액션 선택
	std::string actions[] = { "공격", "방어", "스킬", "아이템", "도망" };
	player->action = actions[rand() % 5];
	player->hasSelected = true;

	std::cout << "[선택 완료] " << player->playerName << " -> " << player->action << std::endl;
	return 0;
}

int main()
{
	srand((unsigned int)time(NULL));

	// 플레이어 데이터 설정
	PlayerAction players[4];
	std::string names[] = { "전사 아서", "마법사 멀린", "도적 로빈", "성기사 갈라하드" };

	for (int i = 0; i < 4; ++i) {
		players[i] = { i + 1, names[i], "", false, (rand() % 5) + 1 };
	}

	HANDLE hThreads[4];

	std::cout << "=== 턴 기반 전투 시작! ===" << std::endl;
	std::cout << "제한 시간: 30초" << std::endl;
	std::cout << "모든 플레이어가 행동을 선택해주세요!\n" << std::endl;

	// 모든 플레이어 사고 스레드 시작
	for (int i = 0; i < 4; ++i) {
		hThreads[i] = (HANDLE)_beginthreadex(
			NULL, 0, PlayerThinkingThread, &players[i], 0, NULL
		);

		if (hThreads[i] == NULL) {
			std::cout << "플레이어 " << i + 1 << " 스레드 생성 실패!" << std::endl;
			return -1;
		}
	}

	// 30초 타임아웃으로 모든 플레이어 선택 대기
	DWORD waitResult = WaitForMultipleObjects(4, hThreads, TRUE, 30000); // 30초

	std::cout << "\n=== 선택 시간 종료! ===" << std::endl;

	if (waitResult == WAIT_OBJECT_0) {
		std::cout << "모든 플레이어가 시간 내에 행동을 선택했습니다!" << std::endl;
	}
	else if (waitResult == WAIT_TIMEOUT) {
		std::cout << "제한 시간이 초과되었습니다!" << std::endl;

		// 선택하지 않은 플레이어들 기본 행동 처리
		for (int i = 0; i < 4; ++i) {
			if (!players[i].hasSelected) {
				players[i].action = "기본 공격";
				std::cout << "[자동 선택] " << players[i].playerName << " -> 기본 공격" << std::endl;
			}
		}
	}
	else {
		std::cout << "대기 중 오류가 발생했습니다!" << std::endl;
	}

	// 전투 결과 출력
	std::cout << "\n=== 전투 진행 ===\n" << std::endl;
	for (int i = 0; i < 4; ++i) {
		std::cout << players[i].playerName << "의 행동: " << players[i].action << std::endl;
		Sleep(500); // 전투 연출
	}

	std::cout << "\n전투가 완료되었습니다!" << std::endl;

	// 핸들 정리
	for (int i = 0; i < 4; ++i) {
		CloseHandle(hThreads[i]);
	}

	return 0;
}