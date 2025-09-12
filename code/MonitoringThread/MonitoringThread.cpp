#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <windows.h>

void cpuIntensiveTask(int threadId, int seconds)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(seconds);

    while (std::chrono::high_resolution_clock::now() < end) {
        // CPU 집약적 작업 시뮬레이션
        volatile int dummy = 0;
        for (int i = 0; i < 1000000; ++i) {
            dummy += i;
        }
    }

    std::cout << "스레드 " << threadId << " 완료" << std::endl;
}

int main()
{
    std::cout << "현재 프로세스 ID: " << GetCurrentProcessId() << std::endl;
    std::cout << "Task Manager에서 이 PID를 찾아보세요!" << std::endl;

    std::cout << std::endl;
    std::cout << "10초 후 스레드 생성합니다" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "\n1단계: 단일 스레드로 실행 (10초)" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    cpuIntensiveTask(0, 8);

    std::cout << "\n2단계: 8개 스레드로 실행 (10초)" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back(cpuIntensiveTask, i, 8);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "관찰 완료. 엔터를 눌러 종료하세요." << std::endl;
    std::cin.get();
    return 0;
}