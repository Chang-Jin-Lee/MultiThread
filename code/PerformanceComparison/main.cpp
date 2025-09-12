#include <chrono>
#include <iostream>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector> // std::vector�� ����Ϸ��� �ʿ��մϴ�.

class PerformanceComparison {
private:
    std::atomic<long long> atomic_counter{ 0 };
    long long normal_counter = 0;
    std::mutex counter_mutex;

public:
    // ���� ���� ��� ���� (�ſ� ����)
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

    // Ŀ�� ��� ��ȯ�� �߻��� �� �ִ� ���� (��������� ����)
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

    // ��Ƽ������ ȯ�濡���� ���� �׽�Ʈ
    void test_contention(int thread_count, int iterations_per_thread) {
        std::cout << "\n=== ��Ƽ������ ���� �׽�Ʈ (" << thread_count << "�� ������) ===\n";

        // Atomic �׽�Ʈ
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

        // Mutex �׽�Ʈ
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

        // 0���� ������ ���� ����
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

    std::cout << "=== ���� ������ ���� �׽�Ʈ ===\n";
    perf.test_atomic_operations(10000000); // �ݺ� Ƚ���� �÷� ���̸� ��Ȯ�ϰ� ��
    perf.test_mutex_operations(10000000);

    // �� ���� Ƚ���� 10,000,000���� ����
    perf.test_contention(2, 5000000);
    perf.test_contention(4, 2500000);
    perf.test_contention(8, 1250000);
    perf.test_contention(16, 625000);
}



// ============== �߰��� main �Լ� ==============
int main() 
{
    // ���� �׽�Ʈ �Լ��� ȣ���մϴ�.
    run_performance_tests();

    // ���α׷��� ���������� ����Ǿ����� ��Ÿ���ϴ�.
    return 0;
}