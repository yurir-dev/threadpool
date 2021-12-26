#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <assert.h>

namespace concurency
{
	template <typename T>
	class threadsafe_queue final
	{
	public:
		threadsafe_queue() = default;
		threadsafe_queue(const threadsafe_queue& rHnd);
		~threadsafe_queue() = default;

		// blocking functions
		T& front();
		T pop_front();
		void pop_front(T& out);
		
		void push_back(const T& item);
		void push_back(T&& item);

		size_t size()const;
		bool empty()const;

		void clear();

	private:
		std::deque<T> _queue;
		mutable std::mutex _mutex;
		std::condition_variable _cond;
	};


	template <typename T>
	threadsafe_queue<T>::threadsafe_queue(const threadsafe_queue& rHnd)
	{
		std::unique_lock<std::mutex> mlock(rHnd._mutex);
		_queue = rHnd._queue;
	}

	template <typename T>
	T& threadsafe_queue<T>::front()
	{
		std::unique_lock<std::mutex> mlock(_mutex);
		while (_queue.empty())
		{
			_cond.wait(mlock);
		}
		return _queue.front();
	}

	template <typename T>
	void threadsafe_queue<T>::clear()
	{
		std::unique_lock<std::mutex> mlock(_mutex);
		while (!_queue.empty())
		{
			_queue.pop_front();
		}
		assert(_queue.empty());
	}

	template <typename T>
	T threadsafe_queue<T>::pop_front()
	{
		std::unique_lock<std::mutex> mlock(_mutex);
		while (_queue.empty())
		{
			_cond.wait(mlock);
		}

		T n = _queue.front();
		_queue.pop_front();
		return n;
	}

	template <typename T>
	void threadsafe_queue<T>::pop_front(T& out)
	{
		std::unique_lock<std::mutex> mlock(_mutex);
		while (_queue.empty())
		{
			_cond.wait(mlock);
		}

		out = _queue.front();
		_queue.pop_front();
	}

	template <typename T>
	void threadsafe_queue<T>::push_back(const T& item)
	{
		{
			std::unique_lock<std::mutex> mlock(_mutex);
			_queue.push_back(item);
		}
		// unlock before notificiation to minimize mutex context
		_cond.notify_one(); // notify one waiting thread

	}

	template <typename T>
	void threadsafe_queue<T>::push_back(T&& item)
	{
		{
			std::unique_lock<std::mutex> mlock(_mutex);
			_queue.push_back(std::move(item));
		}
		// unlock before notificiation to minimize mutex context
		_cond.notify_one(); // notify one waiting thread

	}

	template <typename T>
	size_t threadsafe_queue<T>::size()const
	{
		size_t size{0};
		{
			std::unique_lock<std::mutex> mlock(_mutex);
			size = _queue.size();
		}
		return size;
	}

	template <typename T>
	bool threadsafe_queue<T>::empty()const
	{
		return size() == 0;
	}

}