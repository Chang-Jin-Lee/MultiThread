#include <windows.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

class SystemCallBenchmark
{
public:
    static void benchmarkFileOperations() {
        const int iterations = 1000;
        std::vector<double> times;

        std::cout << "���� �ý��� �� ��ġ��ũ ����..." << std::endl;

        for (int i = 0; i < iterations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();

            // ���� ���� �ý��� ��
            HANDLE file = CreateFile(
                L"test_file.tmp",
                GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
                NULL
            );

            if (file != INVALID_HANDLE_VALUE) {
                CloseHandle(file);  // ���� �ݱ� �ý��� ��
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            times.push_back(duration.count());
        }

        // ��� ���
        double total = 0;
        for (double time : times) {
            total += time;
        }
        double average = total / times.size();

        std::cout << "��� ���� ����/���� �ð�: " << average << " ��s" << std::endl;
    }

    static void benchmarkMemoryOperations() {
        const int iterations = 10000;
        const size_t allocSize = 1024 * 1024;  // 1MB

        std::cout << "�޸� �ý��� �� ��ġ��ũ ����..." << std::endl;

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < iterations; ++i) {
            // �޸� �Ҵ� �ý��� ��
            void* ptr = VirtualAlloc(NULL, allocSize, MEM_COMMIT, PAGE_READWRITE);
            if (ptr) {
                // �޸� ���� �ý��� ��
                VirtualFree(ptr, 0, MEM_RELEASE);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << iterations << "ȸ �޸� �Ҵ�/���� �ð�: " << duration.count() << " ms" << std::endl;
        std::cout << "��� �ð�: " << (double)duration.count() / iterations << " ms" << std::endl;
    }

    static void benchmarkThreadOperations() {
        const int iterations = 100;

        std::cout << "������ �ý��� �� ��ġ��ũ ����..." << std::endl;

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < iterations; ++i) {
            // ������ ���� �ý��� ��
            HANDLE thread = CreateThread(NULL, 0, [](LPVOID) -> DWORD {
                return 0;  // ��� ����
                }, NULL, 0, NULL);

            if (thread) {
                WaitForSingleObject(thread, INFINITE);  // ������ ��� �ý��� ��
                CloseHandle(thread);  // �ڵ� �ݱ� �ý��� ��
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << iterations << "�� ������ ����/���� �ð�: " << duration.count() << " ms" << std::endl;
        std::cout << "��� �ð�: " << (double)duration.count() / iterations << " ms" << std::endl;
    }
};

void printSystemInfo() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    std::cout << "=== �ý��� ���� ===" << std::endl;
    std::cout << "���μ��� ��: " << sysInfo.dwNumberOfProcessors << std::endl;
    std::cout << "������ ũ��: " << sysInfo.dwPageSize << " bytes" << std::endl;
    std::cout << "���μ��� ��Ű��ó: ";

    switch (sysInfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64:
        std::cout << "x64 (AMD or Intel)" << std::endl;
        break;
    case PROCESSOR_ARCHITECTURE_ARM:
        std::cout << "ARM" << std::endl;
        break;
    case PROCESSOR_ARCHITECTURE_ARM64:
        std::cout << "ARM64" << std::endl;
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        std::cout << "Intel x86" << std::endl;
        break;
    default:
        std::cout << "Unknown (" << sysInfo.wProcessorArchitecture << ")" << std::endl;
    }
    std::cout << std::endl;
}

void showMenu() {
    std::cout << "=== �ý��� �� ��ġ��ũ �޴� ===" << std::endl;
    std::cout << "1. ���� �ý��� �� ��ġ��ũ" << std::endl;
    std::cout << "2. �޸� �ý��� �� ��ġ��ũ" << std::endl;
    std::cout << "3. ������ �ý��� �� ��ġ��ũ" << std::endl;
    std::cout << "4. ��ü ��ġ��ũ ����" << std::endl;
    std::cout << "0. ����" << std::endl;
    std::cout << "����: ";
}

void runAllBenchmarks() {
    std::cout << "\n=== ��ü ��ġ��ũ ���� ===" << std::endl;

    auto totalStart = std::chrono::high_resolution_clock::now();

    // 1. ���� �ý��� �� ��ġ��ũ
    std::cout << "\n[1/3] ";
    SystemCallBenchmark::benchmarkFileOperations();

    // 2. �޸� �ý��� �� ��ġ��ũ
    std::cout << "\n[2/3] ";
    SystemCallBenchmark::benchmarkMemoryOperations();

    // 3. ������ �ý��� �� ��ġ��ũ
    std::cout << "\n[3/3] ";
    SystemCallBenchmark::benchmarkThreadOperations();

    auto totalEnd = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(totalEnd - totalStart);

    std::cout << "\n=== ��ü ��ġ��ũ �Ϸ� ===" << std::endl;
    std::cout << "�� ���� �ð�: " << totalDuration.count() << " ms" << std::endl;
}

int main() {
    std::cout << "=== �ý��� �� ���� ��ġ��ũ ���α׷� ===" << std::endl;
    std::cout << "���� ���� ����ȭ�� ���� �ý��� �� ���� ����\n" << std::endl;

    // �ý��� ���� ���
    printSystemInfo();

    // �ܼ� ��� ���е� ����
    std::cout << std::fixed << std::setprecision(3);

    int choice;
    bool running = true;

    while (running) {
        showMenu();

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "�߸��� �Է��Դϴ�. ���ڸ� �Է����ּ���.\n" << std::endl;
            continue;
        }

        std::cout << std::endl;

        switch (choice) {
        case 1:
            SystemCallBenchmark::benchmarkFileOperations();
            break;

        case 2:
            SystemCallBenchmark::benchmarkMemoryOperations();
            break;

        case 3:
            SystemCallBenchmark::benchmarkThreadOperations();
            break;

        case 4:
            runAllBenchmarks();
            break;

        case 0:
            running = false;
            std::cout << "���α׷��� �����մϴ�." << std::endl;
            break;

        default:
            std::cout << "�߸��� �����Դϴ�. 0-4 ������ ���ڸ� �Է����ּ���." << std::endl;
            break;
        }

        if (running && choice >= 1 && choice <= 4) {
            std::cout << "\n����Ϸ��� Enter�� ��������...";
            std::cin.ignore();
            std::cin.get();
            std::cout << std::endl;
        }
    }

    return 0;
}