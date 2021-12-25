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
		threadPool(size_t n);
		threadPool(const std::vector<int> affinity); // TODO
		~threadPool() { end(); }

		size_t threadNum()const { return _threads.size(); }

		void start();
		void end();

		void push(std::function<void()> func);

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