#pragma once

#include <thread>
#include <vector>
#include <functional>
#include <atomic>

#include "threadsafe_queue.h"

namespace concurency
{
	/*
	    one queue, random distribution, possible reordering
	 
		job1 \               -> thread1
		job2 ----> one queue -> thread2
		job3 /               -> thread3
	*/

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


	/*
		a queue per thread, random distribution and ordered distribution
		it's possible to enforce order between jobs by using the same hash
		also less locks.

		job1 ----> queue 1 -> thread1
		job2 ----> queue 2 -> thread2
		job3 ----> queue 3 -> thread3
	*/
	class threadPoolOrdered final
	{
	public:
		threadPoolOrdered() = default;
		~threadPoolOrdered() { end(); }

		size_t threadNum()const { return _workers.size(); }

		/*
			start(5) - starts 5 threads
			start({1, 2, -1, -1, 5}) - starts 5 threads,
						one thread is pinned on cpu 1, another on cpu 2, etc
						-1 means thread is not pinned.
		*/
		void start(size_t numThreads);
		void start(const std::vector<int>&affinity);
		void end(); // finishes all the threads and cleans the task queue

		void push(const std::function<void()>& func); // random thread will handle it
		void push(const std::function<void()>&func, uint32_t hash); // a specific thread will handle it, equal hashes will be passed to the same thread

	private:
		struct worker final
		{
			worker(int affinity = -1) :_affinity(affinity) {}
			worker(worker&&) noexcept = default;
			worker& operator=(worker&& rHnd) noexcept = default;

			void setAffinity(int a) { _affinity = a; }

			void start(std::atomic<bool>& end);
			void end();

			void push(const std::function<void()>& func);

		private:
			threadsafe_queue<std::function<void()>> _queue;
			std::thread _thread;
			int _affinity{ -1 };

			worker(const worker&) = delete;
			worker& operator=(const worker&) = delete;
		};
		
		std::vector<worker> _workers;
		std::atomic<bool> _end{ true };

		threadPoolOrdered(const threadPoolOrdered&) = delete;
		threadPoolOrdered(const threadPoolOrdered&&) = delete;
		threadPoolOrdered& operator=(const threadPoolOrdered&) = delete;
		threadPoolOrdered& operator=(const threadPoolOrdered&&) = delete;
	};
}