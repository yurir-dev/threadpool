#pragma once

#include <thread>
#include <vector>
#include <future>
#include <random>
#include <limits>
#include <iostream>
#include <shared_mutex>

#include "threadsafe_queue.h"

namespace concurency
{
	void setAffinity(int cpuNum);

	/*
		executes functions that look like this: Ret_t func()
		
		example:
		concurency::threadPool<bool> tp_t;
		tp_t.start(5);
		std::future<bool> f = tp_t.push([](){return true;})
		f.get_future();
		tp.end()

		a queue per thread, random distribution and ordered distribution
		it's possible to enforce order between jobs by using the same hash

		job1 ----> queue 1 -> thread1
		job2 ----> queue 2 -> thread2
		job3 ----> queue 3 -> thread3

		the max number of thread is fixed to avoid resizing of internal vector of workers,
		if push() happens before start() or after end() an std::logic_error exception maybe thrown.
		all API functions are 100% thread safe.
	*/
	template<typename Ret_t, size_t maxNumThreads = 128>
	class threadPool final
	{
	public:
		typedef std::function<Ret_t()> task_t;

		threadPool()
		{
			_workers.resize(maxNumThreads);
		}
		~threadPool() { end(); }

		size_t threadNum()const { return _threadNum.load(); }
		constexpr size_t maxThreadNum()const { return maxNumThreads; }

		/*
			start(5) - starts 5 threads
			start({1, 2, -1, -1, 5}) - starts 5 threads,
						one thread is pinned on cpu 1, another on cpu 2, etc
						-1 means thread is not pinned.
		*/
		void start(size_t numThreads);
		void start(const std::vector<int>& affinity);
		
		/*
			blocking
			threads won't accept new tasks, pool will wait untill all current tasks are done.
		*/
		void end(); // finishes all the threads and cleans the task queues

		// returns future return of the func, so caller can wait for it or just ignore it
		std::future<Ret_t> push(task_t&& func); // random thread will handle it
		std::future<Ret_t> push(task_t&& func, uint32_t hash); // a specific thread will handle it, equal hashes will be passed to the same thread

	private:

		struct worker final
		{
			worker(int affinity = -1) :_affinity(affinity) {}
			worker(worker&&) noexcept = default;
			worker& operator=(worker&& rHnd) noexcept = default;

			void setCpuAffinity(int a) { _affinity = a; }

			void start(std::atomic<bool>& end);
			void end();

			std::future<Ret_t> push(task_t&& t);

		private:
			threadsafe_queue<std::packaged_task<Ret_t()>> _queue;
			std::thread _thread;
			int _affinity{ -1 };

			worker(const worker&) = delete;
			worker& operator=(const worker&) = delete;
		};

		std::atomic<size_t> _threadNum{0};	// number of current active workers
		std::vector<worker> _workers;
		std::atomic<bool> _end{ true };		// a flag for all workers
		std::shared_mutex _mtx;				// used to sync start/end and pushers

		threadPool(const threadPool&) = delete;
		threadPool(const threadPool&&) = delete;
		threadPool& operator=(const threadPool&) = delete;
		threadPool& operator=(const threadPool&&) = delete;
	};

	template<typename Ret_t, size_t maxNumThreads>
	void threadPool<Ret_t, maxNumThreads>::worker::start(std::atomic<bool>& endFlag)
	{
		auto f = [this, &endFlag]() {
			if (_affinity >= 0)
				setAffinity(_affinity);
			while (!endFlag.load())
			{
				auto task = std::move(_queue.pop_front());
				task();
			}

			while (!_queue.empty())
			{
				auto task = std::move(_queue.pop_front());
				task();
			}

		};
		end();
		_thread = std::thread{ f };
	}
	template<typename Ret_t, size_t maxNumThreads>
	void threadPool<Ret_t, maxNumThreads>::worker::end()
	{
		if (_thread.joinable())
		{
			_queue.push_back(std::packaged_task<Ret_t()>([]() {return Ret_t(); }));
			_thread.join();
		}
	}
	template<typename Ret_t, size_t maxNumThreads>
	std::future<Ret_t> threadPool<Ret_t, maxNumThreads>::worker::push(task_t&& t)
	{
		auto pt = std::packaged_task<Ret_t()>(t);
		auto future = pt.get_future();
		_queue.push_back(std::move(pt));
		return future;
	}

	template<typename Ret_t, size_t maxNumThreads>
	void threadPool<Ret_t, maxNumThreads>::start(size_t numThreads)
	{
		if (numThreads == 0)
			throw std::invalid_argument("numThreads can't be 0");

		std::vector<int> affinity;
		affinity.resize(numThreads);
		std::fill(affinity.begin(), affinity.end(), -1);
		start(affinity);
	}

	template<typename Ret_t, size_t maxNumThreads>
	void threadPool<Ret_t, maxNumThreads>::start(const std::vector<int>& affinity)
	{
		if (affinity.size() == 0)
			throw std::invalid_argument("requested numThreads can't be 0");
		if (affinity.size() > maxNumThreads)
			throw std::invalid_argument("requested numThreads can't be greater than maxNumThreads");

		std::lock_guard<std::shared_mutex> lock(_mtx);

		_end.store(false);
		for (size_t i = 0; i < affinity.size(); ++i)
		{
			auto& w = _workers[i];
			w.setCpuAffinity(affinity[i]);
			w.start(_end);
		}
		_threadNum.store(affinity.size());
	}

	template<typename Ret_t, size_t maxNumThreads>
	void threadPool<Ret_t, maxNumThreads>::end()
	{
		std::lock_guard<std::shared_mutex> lock(_mtx);

		const size_t threadNum = _threadNum.exchange(0);
		_end.store(true);
		for (size_t i = 0; i < threadNum; ++i)
			_workers[i].end();
	}

	template<typename Ret_t, size_t maxNumThreads>
	std::future<Ret_t> threadPool<Ret_t, maxNumThreads>::push(task_t&& t)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<uint32_t> distrib(0, std::numeric_limits<uint32_t>::max());

		return push(std::forward<task_t>(t), distrib(gen));
	}

	template<typename Ret_t, size_t maxNumThreads>
	std::future<Ret_t> threadPool<Ret_t, maxNumThreads>::push(task_t&& t, uint32_t hash)
	{
		// multiple pushers can enter, they will wait only when start/end is called
		std::shared_lock<std::shared_mutex> sharedLock(_mtx);

		const size_t n{ threadNum() };
		if (n == 0)
			throw std::logic_error("no available workers");
		return _workers[hash % n].push(std::forward<task_t>(t));
	}
}


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
#else
	std::cerr << "thread pinning is not supported for this OS" << std::endl;
#endif
}