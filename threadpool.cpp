#include "threadpool.h"

using namespace concurency;



threadPool::threadPool(size_t n)
{
	_threads.resize(n);
}
threadPool::threadPool(const std::vector<int> affinity)
{

}
void threadPool::start()
{
	auto thread_func = [this]() {
		while (!_end.load())
		{
			auto task = _queue.pop_front();
			task();
		}
	};

	_end.store(false);
	for (size_t i = 0; i < _threads.size(); ++i)
	{
		_threads[i] = std::thread{thread_func};
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
