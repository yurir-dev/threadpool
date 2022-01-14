#include "tp/threadpool.h"


int main(int argc, char* argv[])
{
	using tp_t = concurency::threadPool<bool>;
	tp_t tp;
	tp.start(3);
	tp.push([]() { return true; });
	std::future<bool> f1 = tp.push([]() { return true; }, 1);
	bool res1 = f1.get();
	tp.end();

	tp.start({-1, -1, 2, -1});
	tp.push([]() { return true; });
	std::future<bool> f2 = tp.push([]() { return true; }, 1024);
	bool res2 = f2.get();
	tp.end();

	return res1 && res2 ? 0 : -1;
}