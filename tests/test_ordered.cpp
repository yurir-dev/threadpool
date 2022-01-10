#include "threadpool.h"

#include "../threadpool.h"
#include "test_common.h"

#include <chrono>
#include <thread>
#include <random>
#include <limits>
#include <atomic>

int test(size_t numThreads)
{
	size_t cntTarget{128};
	std::atomic<size_t> cnt{0};
	
	auto randomTask = []() {
		std::cout << "random thread " << std::this_thread::get_id() << std::endl;
	};

	concurency::threadPool<void> tp;
	tp.start(numThreads);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<uint32_t> distrib(0, std::numeric_limits<uint32_t>::max());
	auto hash = distrib(gen);

	for (size_t i = 0; i < cntTarget; ++i)
	{
		auto orderedTask = [&cnt, expected = i]() {
			if (cnt != expected)
				throw std::logic_error("unexpected cnt");
			std::cout << "ordered thread " << std::this_thread::get_id() << " cnt: " << cnt.load() << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			++cnt;
		};
		tp.push(orderedTask, hash);
		tp.push(randomTask);
	}

	while (cnt.load() < cntTarget)
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

	tp.end();

	return 0;
}


int main(int argc, char* argv[])
{
	for (size_t n : {1, 2, 3, 4, 5})
		test(n);

	return 0;
}