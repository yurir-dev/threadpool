#pragma once

#include <thread>
#include <vector>
#include <functional>
#include <atomic>
#include <future>
#include <random>
#include <limits>
#include <memory>

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
	*/
	template<typename Ret_t>
	class threadPool final
	{
	public:
		typedef std::function<Ret_t()> task_t;

		threadPool() = default;
		~threadPool() { end(); }

		size_t threadNum()const { return _workers.size(); }

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
		void end(); // finishes all the threads and cleans the task queue

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

		std::vector<worker> _workers;
		std::atomic<bool> _end{ true };

		threadPool(const threadPool&) = delete;
		threadPool(const threadPool&&) = delete;
		threadPool& operator=(const threadPool&) = delete;
		threadPool& operator=(const threadPool&&) = delete;
	};

	template<typename Ret_t>
	void threadPool<Ret_t>::worker::start(std::atomic<bool>& end)
	{
		auto f = [this, &end]() {
			if (_affinity >= 0)
				setAffinity(_affinity);
			while (!end.load())
			{
				auto task = std::move(_queue.pop_front());
				if(task.valid())
					task();
			}
		};
		_thread = std::thread{ f };
	}
	template<typename Ret_t>
	void threadPool<Ret_t>::worker::end()
	{
		if (_thread.joinable())
		{
			_queue.push_back(std::packaged_task<Ret_t()>([]() {return Ret_t(); }));
			_thread.join();
		}
		_queue.clear();
	}
	template<typename Ret_t>
	std::future<Ret_t> threadPool<Ret_t>::worker::push(task_t&& t)
	{
		auto pt = std::packaged_task<Ret_t()>(t);
		auto future = pt.get_future();
		_queue.push_back(std::move(pt));
		return future;
	}

	template<typename Ret_t>
	void threadPool<Ret_t>::start(size_t numThreads)
	{
		std::vector<int> affinity;
		affinity.resize(numThreads);
		std::fill(affinity.begin(), affinity.end(), -1);
		start(affinity);
	}

	template<typename Ret_t>
	void threadPool<Ret_t>::start(const std::vector<int>& affinity)
	{
		_end.store(false);
		_workers.reserve(affinity.size());
		for (int a : affinity)
			_workers.emplace_back(a).start(_end);
	}

	template<typename Ret_t>
	void threadPool<Ret_t>::end()
	{
		_end.store(true);
		for (auto& w : _workers)
			w.end();
		_workers.clear();
	}

	template<typename Ret_t>
	std::future<Ret_t> threadPool<Ret_t>::push(task_t&& t)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<uint32_t> distrib(0, std::numeric_limits<uint32_t>::max());

		return push(std::forward<task_t>(t), distrib(gen));
	}

	template<typename Ret_t>
	std::future<Ret_t> threadPool<Ret_t>::push(task_t&& t, uint32_t hash)
	{
		if (_workers.size() == 0)
			throw std::logic_error("no available workers");
		return _workers[hash % _workers.size()].push(std::forward<task_t>(t));
	}
}