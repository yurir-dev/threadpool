#pragma once

#include <thread>
#include <vector>
#include <functional>
#include <atomic>

#include "threadsafe_queue.h"

namespace concurency
{
	class threadPool final
	{
	public:
		threadPool() = default;
		~threadPool() { end(); }

		size_t threadNum()const { return _threads.size(); }

		/*
			start(5) - starts 5 threads
			start({1, 2, -1, -1, 5}) - starts 5 threads,
						one thread is pinned on cpu 1, another on cpu 2, etc
						-1 means thread is not pinned.
		*/
		void start(size_t numThreads);
		void start(const std::vector<int>& affinity); 
		void end(); // finishes all the threads and cleans the task queue

		void push(const std::function<void()>& func);

	private:
		threadsafe_queue<std::function<void()>> _queue;
		std::vector<std::thread> _threads;
		std::atomic<bool> _end{ true };

		threadPool(const threadPool&) = delete;
		threadPool(const threadPool&&) = delete;
		threadPool& operator=(const threadPool&) = delete;
		threadPool& operator=(const threadPool&&) = delete;
	};

}