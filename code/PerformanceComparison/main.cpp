#include <chrono>
#include <iostream>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector> // std::vector를 사용하려면 필요합니다.

class PerformanceComparison {
private:
    std::atomic<long long> atomic_counter{ 0 };
    long long normal_counter = 0;
    std::mutex counter_mutex;

public:
    // 순수 유저 모드 연산 (매우 빠름)
    void test_atomic_operations(int iterations) {
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < iterations; ++i) {
            atomic_counter.fetch_add(1, std::memory_order_relaxed);
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start).count();

        std::cout << "Atomic operations: " << duration
            << " microseconds for " << iterations << " operations\n";
    }

    // 커널 모드 전환이 발생할 수 있는 연산 (상대적으로 느림)
    void test_mutex_operations(int iterations) {
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < iterations; ++i) {
            std::lock_guard<std::mutex> lock(counter_mutex);
            normal_counter++;
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start).count();

        std::cout << "Mutex operations: " << duration
            << " microseconds for " << iterations << " operations\n";
    }

    // 멀티스레드 환경에서의 경합 테스트
    void test_contention(int thread_count, int iterations_per_thread) {
        std::cout << "\n=== 멀티스레드 경합 테스트 (" << thread_count << "개 스레드) ===\n";

        // Atomic 테스트
        atomic_counter.store(0);
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<std::thread> atomic_threads;
        for (int i = 0; i < thread_count; ++i) {
            atomic_threads.emplace_back([this, iterations_per_thread] {
                for (int j = 0; j < iterations_per_thread; ++j) {
                    atomic_counter.fetch_add(1, std::memory_order_relaxed);
                }
                });
        }

        for (auto& t : atomic_threads) {
            t.join();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto atomic_duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start).count();

        // Mutex 테스트
        normal_counter = 0;
        start = std::chrono::high_resolution_clock::now();

        std::vector<std::thread> mutex_threads;
        for (int i = 0; i < thread_count; ++i) {
            mutex_threads.emplace_back([this, iterations_per_thread] {
                for (int j = 0; j < iterations_per_thread; ++j) {
                    std::lock_guard<std::mutex> lock(counter_mutex);
                    normal_counter++;
                }
                });
        }

        for (auto& t : mutex_threads) {
            t.join();
        }

        end = std::chrono::high_resolution_clock::now();
        auto mutex_duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start).count();

        std::cout << "Atomic (" << thread_count << " threads): "
            << atomic_duration << " microseconds\n";
        std::cout << "Mutex (" << thread_count << " threads): "
            << mutex_duration << " microseconds\n";

        // 0으로 나누는 오류 방지
        if (atomic_duration > 0) {
            std::cout << "Performance ratio: "
                << (static_cast<double>(mutex_duration) / atomic_duration)
                << "x slower with mutex\n";
        }
    }
};

void run_performance_tests()
{
    PerformanceComparison perf;

    std::cout << "=== 단일 스레드 성능 테스트 ===\n";
    perf.test_atomic_operations(10000000); // 반복 횟수를 늘려 차이를 명확하게 함
    perf.test_mutex_operations(10000000);

    // 총 연산 횟수를 10,000,000으로 고정
    perf.test_contention(2, 5000000);
    perf.test_contention(4, 2500000);
    perf.test_contention(8, 1250000);
    perf.test_contention(16, 625000);
}



// ============== 추가된 main 함수 ==============
int main() 
{
    // 성능 테스트 함수를 호출합니다.
    run_performance_tests();

    // 프로그램이 성공적으로 종료되었음을 나타냅니다.
    return 0;
}