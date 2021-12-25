#include <iostream>
#include <mutex>
#include <set>
#include "../threadpool.h"

int main(int argc, char* argv[])
{
	const size_t numThreads = 5;
	concurency::threadPool tp{ numThreads };

	tp.start();

	std::set<std::thread::id> ids;
	std::mutex m;
	auto func = [&m, &ids]() {
		std::unique_lock<std::mutex> mlock(m);
		ids.insert(std::this_thread::get_id());
		std::cout << "executed from thread: " << std::this_thread::get_id() << std::endl;
	};

	for (size_t i = 0; i < 1024; ++i)
		tp.push(func);

	tp.end();

	if (ids.size() == numThreads && tp.threadNum() == numThreads)
		return 0;
	return -1;
}