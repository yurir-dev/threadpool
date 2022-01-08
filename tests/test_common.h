#pragma once

#include <iostream>
#include <mutex>
#include <set>
#include <future>
#include <atomic>
#include <thread>

struct testCommon
{
	// returns 0 on success, -1 on failure
	template<typename ThreadPool_t>
	static int testThreadpool(ThreadPool_t& tp)
	{
		std::set<std::thread::id> ids;
		std::mutex m;
		auto func = [&m, &ids]() {
			std::unique_lock<std::mutex> mlock(m);
			ids.insert(std::this_thread::get_id());
			std::cout << "executed from thread: " << std::this_thread::get_id() << std::endl;
		};

		// launch pusher threads, they will start pushing at the same time
		std::atomic<size_t> runnungPushers{ 0 };
		std::promise<int> p;
		std::future<int> f = p.get_future();
		auto pusher = [&tp, &func, &runnungPushers, &f](size_t n) {
			runnungPushers++;
			f.wait(); // wait for signal
			for (size_t i = 0; i < n; ++i)
				tp.push(func);
		};

		// start pusher threads, wait till all threads are up, signal to start pushing
		std::vector<std::thread> pushersThreads;
		for (size_t i = 0; i < 10; i++)
			pushersThreads.push_back(std::thread{ pusher, 1024 });
		while (runnungPushers < pushersThreads.size()) { ; }
		p.set_value(1); // start pushing tasks to threadpool

		// wait untill all pushers are finished
		for (size_t i = 0; i < pushersThreads.size(); i++)
			if (pushersThreads[i].joinable())
				pushersThreads[i].join();

		const auto numThreads = tp.threadNum();

		// must end the threadpool threads here
		// because mutex in func above can be destroyed
		// while the func is still being executed inside the tp
		tp.end();

		// check that funcs were executed from different threads
		if (ids.size() == numThreads)
			return 0;
		return -1;
	}
};
