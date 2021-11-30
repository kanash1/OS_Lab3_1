#include <iostream>
#include <Windows.h>
#include <chrono>
#include <vector>
#include <iomanip>
#include <utility>
#include <atomic>
#undef max
#include "incontrol.h"

const size_t BLOCK_SIZE = 9'307'130;
const size_t ACCURACY = 100'000'000;
std::atomic<size_t> curr_iter{ 0 };

std::pair<long double, long long> pi_and_avg_time_calc(size_t);
DWORD WINAPI pi_subtotal_calc(LPVOID);

int main() {
	InputControler<size_t> input(std::cin);
	size_t threads_count = 0;

	std::cout << "Enter count of threads (max 64): ";
	input >> threads_count;

	if (input.GetSate() == InputState::NORMAL && threads_count <= 64) {
		auto pi_and_avg_time = pi_and_avg_time_calc(threads_count);
		std::cout << "Average time: " << pi_and_avg_time.second << " milliseconds\n"
			<< "Pi value: " << std::fixed << std::setprecision(15) << pi_and_avg_time.first << '\n';
	}
}

std::pair<long double, long long> pi_and_avg_time_calc(const size_t threads_count) {
	long long time = 0;
	long double pi = 0;

	for (size_t i = 0; i < 100; ++i) {
		pi = 0;
		curr_iter.store(0);
		std::vector<HANDLE> threads(threads_count);
		std::vector<long double> subtotals(threads_count);

		for (size_t j = 0; j < threads_count; ++j)
			threads[j] = CreateThread(nullptr, 0, pi_subtotal_calc, &subtotals[j], CREATE_SUSPENDED, nullptr);

		auto start = std::chrono::high_resolution_clock::now();

		for (auto& thread : threads)
			ResumeThread(thread);
		WaitForMultipleObjects(threads_count, threads.data(), TRUE, INFINITE);

		for (const auto& subtotal : subtotals)
			pi += subtotal;
		pi /= ACCURACY;

		auto stop = std::chrono::high_resolution_clock::now();

		time += std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		for (auto& thread : threads)
			CloseHandle(thread);
	}
	return std::make_pair(pi, time / 100);
}

DWORD WINAPI pi_subtotal_calc(LPVOID lpParam) {
	long double* ptr_subtotal = static_cast<long double*>(lpParam);
	size_t begin = curr_iter.fetch_add(BLOCK_SIZE);
	size_t end = begin + BLOCK_SIZE;
	long double x = 0;
	long double block_sum = 0;

	do {
		block_sum = 0;
		for (size_t i = begin; i < end && i < ACCURACY; ++i) {
			x = (i + 0.5) / ACCURACY;
			block_sum += (4 / (1 + x * x));
		}

		*ptr_subtotal += block_sum;
		begin = curr_iter.fetch_add(BLOCK_SIZE);
		end = begin + BLOCK_SIZE;

	} while (begin < ACCURACY);

	return 0;
}