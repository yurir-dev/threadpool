#include "tp/threadpool.h"

#include <iostream>
#include <mutex>
#include <set>
#include <future>
#include <atomic>
#include <thread>


/*
	just run start/end and pushers from different threads
	and see it does not crash
*/
void test(size_t numThreads)
{
	using tp_t = concurency::threadPool<bool>;
	tp_t tp;

	auto starterEnder = [&tp](size_t numThreads, size_t iterations) {
		for (size_t i = 0; i < iterations; ++i)
		{
			tp.start(numThreads);
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			tp.end();
		}
	};

	auto pusher = [&tp](size_t iterations) {

		std::vector<std::future<bool>> futs;
		size_t rejectTasksCnt{0};

		for (size_t i = 0; i < iterations; ++i)
		{
			try
			{
				auto f = tp.push([]() {std::chrono::milliseconds(1); return true; });
				futs.push_back(std::move(f));
			}
			catch (std::logic_error&)
			{
				rejectTasksCnt++;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		for (auto& f : futs)
		{
			if (std::future_status::ready != f.wait_for(std::chrono::seconds(1)))
				throw std::logic_error("task was not executed in time");
		}
		std::cout << futs.size() << " tasks were executed. " << rejectTasksCnt << " were rejected" << std::endl;
	};

	std::vector<std::thread> threads; threads.resize(10);
	size_t index = 0;
	threads[index++] = std::thread{ starterEnder, numThreads, 1024 };
	for (size_t i = index; i < threads.size(); i++)
		threads[i] = std::thread{ pusher, 1024 };

	for (auto& t : threads)
		if (t.joinable())
			t.join();
}


int main(int /*argc*/, char* /*argv*/[])
{
	for (size_t i = 2; i < 5; i += 1)
	{
		test(i);	
	}
	return 0;
}