#include <windows.h>
#include <iostream>
#include <vector>
#include <thread>
#include <iomanip>

class UnsafeBankAccount
{
private:
	volatile int balance;  // volatile: 컴파일러 최적화 방지

public:
	UnsafeBankAccount(int initialBalance = 0) : balance(initialBalance) {}

	void Deposit(int amount) 
	{
		// 위험한 연산: 원자성이 보장되지 않음
		int temp = balance;        // 1. 읽기
		Sleep(1);                  // 2. 컨텍스트 스위치 유발
		balance = temp + amount;   // 3. 쓰기
	}

	void Withdraw(int amount)
	{
		int temp = balance;
		Sleep(1);
		if (temp >= amount) balance = temp - amount;
	}

	int GetBalance() const { return balance; }
};

void TestRaceCondition()
{
	const int THREAD_COUNT = 10;
	const int OPERATIONS_PER_THREAD = 100;
	const int DEPOSIT_AMOUNT = 10;

	UnsafeBankAccount account(0);
	std::vector<std::thread> threads;
	
	std::cout << "=== Race Condition 테스트 시작 ===\n";
	std::cout << "스레드 수: " << THREAD_COUNT << "\n";
	std::cout << "스레드당 입금 횟수: " << OPERATIONS_PER_THREAD << "\n";
	std::cout << "입금 금액: " << DEPOSIT_AMOUNT << "\n";
	std::cout << "예상 최종 잔액: " << (THREAD_COUNT * OPERATIONS_PER_THREAD * DEPOSIT_AMOUNT) << "\n\n";

	for (int i = 0; i < THREAD_COUNT; ++i)
	{
		threads.emplace_back([&account, OPERATIONS_PER_THREAD, DEPOSIT_AMOUNT, i]() {
			for (int j = 0; j < OPERATIONS_PER_THREAD; ++j)
			{
				account.Deposit(DEPOSIT_AMOUNT);
				if (j % 20 == 0) std::cout << "스레드 " << i << ": " << (j + 1) << "회 입금 완료\n";
			}
			});
	}

	// 모든 스레드 완료 대기
	for (auto& t : threads) t.join();

	std::cout << "\n=== 결과 ===\n";
	std::cout << "실제 최종 잔액: " << account.GetBalance() << "\n";
	std::cout << "예상 최종 잔액: " << (THREAD_COUNT * OPERATIONS_PER_THREAD * DEPOSIT_AMOUNT) << "\n";

	int expectedBalance = THREAD_COUNT * OPERATIONS_PER_THREAD * DEPOSIT_AMOUNT;
	if (account.GetBalance() != expectedBalance) 
		std::cout << "No! ..... Race Condition 발생! 데이터 손실: "<< (expectedBalance - account.GetBalance()) << "\n";
	else 
		std::cout << "Yes! .... 우연히 정확한 결과 (다시 실행해보세요)\n";
}

int main()
{
	TestRaceCondition();
	return 0;
}