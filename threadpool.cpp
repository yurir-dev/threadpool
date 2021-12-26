#include "threadpool.h"
#include <iostream>
#if defined(_WIN32)
#include <windows.h>
#elif defined (linux)
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#endif
#include <pthread.h>
#else
#warning "thread pinning is not supported for this OS"
#endif

using namespace concurency;

static void setAffinity(int cpuNum)
{
#if defined(_WIN32)
	DWORD mask = (1 << cpuNum);
	HANDLE th = GetCurrentThread();
	DWORD_PTR prev_mask = SetThreadAffinityMask(th, mask);
#elif defined (linux)
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpuNum, &cpuset);
	int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (rc != 0) {
		std::cerr << "Error calling pthread_setaffinity_np: " << rc << std::endl;
	}
#else
	std::cerr << "thread pinning is not supported for this OS" << std::endl;
#endif
}

void threadPool::start(size_t numThreads)
{
	std::vector<int> affinity;
	affinity.resize(numThreads);
	std::fill(affinity.begin(), affinity.end(), -1);
	start(affinity);
}
void threadPool::start(const std::vector<int>& affinity)
{
	auto thread_func = [this](int affinity) {

		if (affinity >= 0)
			setAffinity(affinity);

		while (!_end.load())
		{
			auto task = _queue.pop_front();
			task();
		}
	};

	_end.store(false);
	_threads.resize(affinity.size());
	for (size_t i = 0; i < _threads.size(); ++i)
	{
		_threads[i] = std::thread{ thread_func, affinity[i] };
	}
}
void threadPool::end()
{
	_end.store(true);
	for (size_t i = 0; i < _threads.size() * 2; ++i)
	{
		push([]() {}); // wake up threads
	}
	for (size_t i = 0; i < _threads.size(); ++i)
	{
		if (_threads[i].joinable())
			_threads[i].join();
	}
}

void threadPool::push(std::function<void()> func)
{
	_queue.push_back(func);
}
