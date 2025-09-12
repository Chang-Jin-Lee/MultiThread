#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

class KernelServiceMonitor 
{
public:
    static void monitorMemoryUsage() {
        PROCESS_MEMORY_COUNTERS_EX pmc;

        while (true) {
            if (GetProcessMemoryInfo(GetCurrentProcess(),
                (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {

                std::cout << "\n=== 메모리 사용량 (커널 관리) ===" << std::endl;
                std::cout << "물리 메모리 사용: " << pmc.WorkingSetSize / 1024 << " KB" << std::endl;
                std::cout << "가상 메모리 사용: " << pmc.PrivateUsage / 1024 << " KB" << std::endl;
                std::cout << "페이지 폴트 수: " << pmc.PageFaultCount << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }

    static void monitorThreads() {
        DWORD currentProcessId = GetCurrentProcessId();

        while (true) {
            HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
            if (snapshot == INVALID_HANDLE_VALUE) continue;

            THREADENTRY32 threadEntry;
            threadEntry.dwSize = sizeof(THREADENTRY32);

            int threadCount = 0;
            if (Thread32First(snapshot, &threadEntry)) {
                do {
                    if (threadEntry.th32OwnerProcessID == currentProcessId) {
                        threadCount++;
                    }
                } while (Thread32Next(snapshot, &threadEntry));
            }

            CloseHandle(snapshot);

            std::cout << "현재 프로세스의 스레드 수: " << threadCount << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }
};

// 전역 종료 플래그
std::atomic<bool> shouldExit(false);

// Ctrl+C 핸들러
BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        std::cout << "\n\n프로그램을 종료합니다..." << std::endl;
        shouldExit = true;
        return TRUE;
    }
    return FALSE;
}

int main() 
{
    // 콘솔 핸들러 설정 (Ctrl+C로 안전하게 종료)
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        std::cerr << "콘솔 핸들러 설정에 실패했습니다." << std::endl;
    }

    std::cout << "=== 커널 서비스 모니터 시작 ===" << std::endl;
    std::cout << "Ctrl+C를 눌러서 종료할 수 있습니다." << std::endl;

    try {
        // KernelServiceMonitor 클래스의 static 메소드들을 스레드로 실행
        std::thread memoryThread([]() {
            KernelServiceMonitor::monitorMemoryUsage();
            });

        std::thread threadMonitorThread([]() {
            KernelServiceMonitor::monitorThreads();
            });

        // 메인 스레드는 종료 신호를 대기
        while (!shouldExit) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // 스레드들이 무한루프이므로 detach 처리
        memoryThread.detach();
        threadMonitorThread.detach();

    }
    catch (const std::exception& e) {
        std::cerr << "프로그램 실행 중 오류 발생: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "프로그램이 정상적으로 종료되었습니다." << std::endl;
    return 0;
}