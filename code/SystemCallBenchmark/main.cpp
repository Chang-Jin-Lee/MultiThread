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

        std::cout << "파일 시스템 콜 벤치마크 시작..." << std::endl;

        for (int i = 0; i < iterations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();

            // 파일 생성 시스템 콜
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
                CloseHandle(file);  // 파일 닫기 시스템 콜
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            times.push_back(duration.count());
        }

        // 통계 계산
        double total = 0;
        for (double time : times) {
            total += time;
        }
        double average = total / times.size();

        std::cout << "평균 파일 생성/삭제 시간: " << average << " μs" << std::endl;
    }

    static void benchmarkMemoryOperations() {
        const int iterations = 10000;
        const size_t allocSize = 1024 * 1024;  // 1MB

        std::cout << "메모리 시스템 콜 벤치마크 시작..." << std::endl;

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < iterations; ++i) {
            // 메모리 할당 시스템 콜
            void* ptr = VirtualAlloc(NULL, allocSize, MEM_COMMIT, PAGE_READWRITE);
            if (ptr) {
                // 메모리 해제 시스템 콜
                VirtualFree(ptr, 0, MEM_RELEASE);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << iterations << "회 메모리 할당/해제 시간: " << duration.count() << " ms" << std::endl;
        std::cout << "평균 시간: " << (double)duration.count() / iterations << " ms" << std::endl;
    }

    static void benchmarkThreadOperations() {
        const int iterations = 100;

        std::cout << "스레드 시스템 콜 벤치마크 시작..." << std::endl;

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < iterations; ++i) {
            // 스레드 생성 시스템 콜
            HANDLE thread = CreateThread(NULL, 0, [](LPVOID) -> DWORD {
                return 0;  // 즉시 종료
                }, NULL, 0, NULL);

            if (thread) {
                WaitForSingleObject(thread, INFINITE);  // 스레드 대기 시스템 콜
                CloseHandle(thread);  // 핸들 닫기 시스템 콜
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << iterations << "개 스레드 생성/종료 시간: " << duration.count() << " ms" << std::endl;
        std::cout << "평균 시간: " << (double)duration.count() / iterations << " ms" << std::endl;
    }
};

void printSystemInfo() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    std::cout << "=== 시스템 정보 ===" << std::endl;
    std::cout << "프로세서 수: " << sysInfo.dwNumberOfProcessors << std::endl;
    std::cout << "페이지 크기: " << sysInfo.dwPageSize << " bytes" << std::endl;
    std::cout << "프로세서 아키텍처: ";

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
    std::cout << "=== 시스템 콜 벤치마크 메뉴 ===" << std::endl;
    std::cout << "1. 파일 시스템 콜 벤치마크" << std::endl;
    std::cout << "2. 메모리 시스템 콜 벤치마크" << std::endl;
    std::cout << "3. 스레드 시스템 콜 벤치마크" << std::endl;
    std::cout << "4. 전체 벤치마크 실행" << std::endl;
    std::cout << "0. 종료" << std::endl;
    std::cout << "선택: ";
}

void runAllBenchmarks() {
    std::cout << "\n=== 전체 벤치마크 실행 ===" << std::endl;

    auto totalStart = std::chrono::high_resolution_clock::now();

    // 1. 파일 시스템 콜 벤치마크
    std::cout << "\n[1/3] ";
    SystemCallBenchmark::benchmarkFileOperations();

    // 2. 메모리 시스템 콜 벤치마크
    std::cout << "\n[2/3] ";
    SystemCallBenchmark::benchmarkMemoryOperations();

    // 3. 스레드 시스템 콜 벤치마크
    std::cout << "\n[3/3] ";
    SystemCallBenchmark::benchmarkThreadOperations();

    auto totalEnd = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(totalEnd - totalStart);

    std::cout << "\n=== 전체 벤치마크 완료 ===" << std::endl;
    std::cout << "총 실행 시간: " << totalDuration.count() << " ms" << std::endl;
}

int main() {
    std::cout << "=== 시스템 콜 성능 벤치마크 프로그램 ===" << std::endl;
    std::cout << "게임 서버 최적화를 위한 시스템 콜 성능 측정\n" << std::endl;

    // 시스템 정보 출력
    printSystemInfo();

    // 콘솔 출력 정밀도 설정
    std::cout << std::fixed << std::setprecision(3);

    int choice;
    bool running = true;

    while (running) {
        showMenu();

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "잘못된 입력입니다. 숫자를 입력해주세요.\n" << std::endl;
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
            std::cout << "프로그램을 종료합니다." << std::endl;
            break;

        default:
            std::cout << "잘못된 선택입니다. 0-4 사이의 숫자를 입력해주세요." << std::endl;
            break;
        }

        if (running && choice >= 1 && choice <= 4) {
            std::cout << "\n계속하려면 Enter를 누르세요...";
            std::cin.ignore();
            std::cin.get();
            std::cout << std::endl;
        }
    }

    return 0;
}