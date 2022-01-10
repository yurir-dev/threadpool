#include "threadpool.h"
#include <iostream>
#include <mutex>


#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#elif defined (linux)
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#endif
#include <pthread.h>
#else
#warning "thread pinning is not supported for this OS"
#endif

void concurency::setAffinity(int cpuNum)
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

	static std::mutex m;
	std::unique_lock<std::mutex> mlock(m);
	std::cout << "thread: " << std::this_thread::get_id() << " pin on cpu " << cpuNum << " rc " << rc << std::endl;
#else
	std::cerr << "thread pinning is not supported for this OS" << std::endl;
#endif
}
