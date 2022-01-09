
#include "../threadpool.h"

#include <set>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

struct testTaskRet_t : public concurency::threadPoolOrderedFuture::taskRet_t
{
	testTaskRet_t(std::thread::id id)
		:tid(id)
	{}
	std::thread::id tid;
};

int testTp(size_t numThreads)
{
	concurency::threadPoolOrderedFuture tp;
	tp.start(numThreads);

	std::vector<std::future<std::shared_ptr<concurency::threadPoolOrderedFuture::taskRet_t>>> futures;
	for (size_t i = 0; i < 1024; ++i)
	{
		concurency::threadPoolOrderedFuture::task_t t{ []() {
			auto tid = std::this_thread::get_id();
			std::cout << "executed from thread: " << tid << std::endl;
			return std::make_shared<testTaskRet_t>(tid);
		} };
		futures.push_back(t.get_future());
		tp.push(std::move(t));
	}

	std::set<std::thread::id> tids;
	for (auto& f : futures)
	{
		auto ptr = std::dynamic_pointer_cast<testTaskRet_t>(f.get());
		tids.insert(ptr->tid);
	}

	tp.end();

	if (tids.size() != numThreads)
		std::cout << "tids.size " << tids.size() << " != numThreads " << numThreads << std::endl;

	return static_cast<int>(numThreads) - static_cast<int>(tids.size());
}

int main(int argc, char* argv[])
{
	for (size_t i : {1, 2, 3, 4, 5})
	{
		if (int res = testTp(i))
			return res;
	}
	return 0;
}