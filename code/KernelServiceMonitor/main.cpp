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

                std::cout << "\n=== �޸� ��뷮 (Ŀ�� ����) ===" << std::endl;
                std::cout << "���� �޸� ���: " << pmc.WorkingSetSize / 1024 << " KB" << std::endl;
                std::cout << "���� �޸� ���: " << pmc.PrivateUsage / 1024 << " KB" << std::endl;
                std::cout << "������ ��Ʈ ��: " << pmc.PageFaultCount << std::endl;
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

            std::cout << "���� ���μ����� ������ ��: " << threadCount << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }
};

// ���� ���� �÷���
std::atomic<bool> shouldExit(false);

// Ctrl+C �ڵ鷯
BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        std::cout << "\n\n���α׷��� �����մϴ�..." << std::endl;
        shouldExit = true;
        return TRUE;
    }
    return FALSE;
}

int main() 
{
    // �ܼ� �ڵ鷯 ���� (Ctrl+C�� �����ϰ� ����)
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        std::cerr << "�ܼ� �ڵ鷯 ������ �����߽��ϴ�." << std::endl;
    }

    std::cout << "=== Ŀ�� ���� ����� ���� ===" << std::endl;
    std::cout << "Ctrl+C�� ������ ������ �� �ֽ��ϴ�." << std::endl;

    try {
        // KernelServiceMonitor Ŭ������ static �޼ҵ���� ������� ����
        std::thread memoryThread([]() {
            KernelServiceMonitor::monitorMemoryUsage();
            });

        std::thread threadMonitorThread([]() {
            KernelServiceMonitor::monitorThreads();
            });

        // ���� ������� ���� ��ȣ�� ���
        while (!shouldExit) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // ��������� ���ѷ����̹Ƿ� detach ó��
        memoryThread.detach();
        threadMonitorThread.detach();

    }
    catch (const std::exception& e) {
        std::cerr << "���α׷� ���� �� ���� �߻�: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "���α׷��� ���������� ����Ǿ����ϴ�." << std::endl;
    return 0;
}