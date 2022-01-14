
#include "../tp/threadpool.h"

#include <set>
#include <string>
#include <vector>
#include <iostream>
#include <chrono>

struct testTaskRet_t
{
	testTaskRet_t(std::thread::id id)
		:tid(id)
	{}
	std::thread::id tid;
};

template<typename Ret_t>
int testTpWithFutures(size_t numThreads)
{
	typedef concurency::threadPool<Ret_t> tp_t;
	tp_t tp;
	tp.start(numThreads);

	std::set<std::thread::id> tids;
	std::mutex m;
	auto func = [&tids, &m]() -> Ret_t {
		auto tid = std::this_thread::get_id();
		{
			std::unique_lock<std::mutex> mlock(m);
			std::cout << "executed from thread: " << tid << std::endl;
			tids.insert(tid);

		}return Ret_t();
	};

	std::vector<std::future<Ret_t>> futures;
	for (size_t i = 0; i < 1024; ++i)
	{
		std::future<Ret_t> f = tp.push(func);
		futures.push_back(std::move(f));
	}

	for (auto& f : futures)
		Ret_t ret = f.get();

	tp.end();

	if (tids.size() != numThreads)
		std::cout << "tids.size " << tids.size() << " != numThreads " << numThreads << std::endl;

	return static_cast<int>(numThreads) - static_cast<int>(tids.size());
}

template<>
int testTpWithFutures<testTaskRet_t>(size_t numThreads)
{
	typedef concurency::threadPool<std::shared_ptr<testTaskRet_t>> tp_t;
	tp_t tp;
	tp.start(numThreads);

	auto func = []() {
		auto tid = std::this_thread::get_id();
		std::cout << "executed from thread: " << tid << std::endl;
		return std::make_shared<testTaskRet_t>(tid);
	};

	std::vector<std::future<std::shared_ptr<testTaskRet_t>>> futures;
	for (size_t i = 0; i < 1024; ++i)
	{
		auto future = tp.push(func);
		futures.push_back(std::move(future));
	}

	std::set<std::thread::id> tids;
	for (auto& f : futures)
	{
		auto ptr = f.get();
		tids.insert(ptr->tid);
	}

	tp.end();

	if (tids.size() != numThreads)
		std::cout << "tids.size " << tids.size() << " != numThreads " << numThreads << std::endl;

	return static_cast<int>(numThreads) - static_cast<int>(tids.size());
}


template<typename Ret_t>
int testTpNoFutures(size_t numThreads)
{
	typedef concurency::threadPool<Ret_t> tp_t;
	tp_t tp;
	tp.start(numThreads);

	size_t cnt{0};
	std::set<std::thread::id> tids;
	std::mutex m;
	auto func = [&tids, &cnt, &m]() -> Ret_t {
		auto tid = std::this_thread::get_id();
		{
			std::unique_lock<std::mutex> mlock(m);
			std::cout << "executed from thread: " << tid << std::endl;
			tids.insert(tid);
			cnt++;
		}
		
		return Ret_t();
	};

	const size_t iterations{1024};
	for (size_t i = 0; i < iterations; ++i)
	{
		{
			tp.push(func);
		}
	}

	while (cnt != iterations)
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

	tp.end();

	if (tids.size() != numThreads)
		std::cout << "tids.size " << tids.size() << " != numThreads " << numThreads << std::endl;

	return static_cast<int>(numThreads) - static_cast<int>(tids.size());
}
int main(int argc, char* argv[])
{
	for (size_t i : {1, 2, 3, 4, 5})
	{
		if (int res = testTpWithFutures<testTaskRet_t>(i) != 0)
			return res;
		if (int res = testTpNoFutures<void>(i) != 0)
			return res;
		if (int res = testTpWithFutures<bool>(i) != 0)
			return res;
		if (int res = testTpNoFutures<bool>(i) != 0)
			return res;
		if (int res = testTpWithFutures<int>(i) != 0)
			return res;
		if (int res = testTpNoFutures<int>(i) != 0)
			return res;
		if (int res = testTpWithFutures<std::string>(i) != 0)
			return res;
		if (int res = testTpNoFutures<std::string>(i) != 0)
			return res;
	}
	return 0;
}