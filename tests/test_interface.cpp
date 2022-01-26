#include "tp/threadpool.h"

#include <algorithm>

int main(int /*argc*/, char* /*argv*/[])
{
	using tp_t = concurency::threadPool<bool>;
	tp_t tp;

	// push before start, throws exception
	try
	{
		auto f = tp.push([]() {return true; });
		std::cout << "should have caugth expected std::logic_error" << std::endl;
		return __LINE__;
	}
	catch (std::logic_error& ex)
	{
		std::cout << "caugth expected std::logic_error: " << ex.what() << std::endl;
	}

	// start with too many threads, throws exception
	try
	{
		tp.start(tp.maxThreadNum() + 1);
		std::cout << "should have caugth expected std::logic_error" << std::endl;
		return __LINE__;
	}
	catch (std::logic_error& ex)
	{
		std::cout << "caugth expected std::logic_error: " << ex.what() << std::endl;
	}

	// normal tasks pushing
	{
		tp.start(3);
		if (tp.threadNum() != 3)
		{
			std::cout << "num of thread is not 3: " << tp.threadNum() << std::endl;
			return __LINE__;
		}
		tp.push([]() { return true; });
		std::future<bool> f1 = tp.push([]() { return true; }, 1);
		if (!f1.get())
		{
			std::cout << "should return true " << __LINE__ << std::endl;
			return __LINE__;
		}
		tp.end();
		if (auto s = tp.threadNum() != 0)
		{
			std::cout << "threadNum should be 0 , " << s << "  " <<  __LINE__ << std::endl;
			return __LINE__;
		}
	}

	// push after end, throws exception
	try
	{
		auto f = tp.push([]() {return true; });
		std::cout << "should have caugth expected std::logic_error" << std::endl;
		return __LINE__;
	}
	catch (std::logic_error& ex)
	{
		std::cout << "caugth expected std::logic_error: " << ex.what() << std::endl;
	}

	// normal tasks pushing
	{
		tp.start({ -1, -1, 2, -1 });
		if (tp.threadNum() != 4)
		{
			std::cout << "num of thread is not 4: " << tp.threadNum() << std::endl;
			return __LINE__;
		}

		tp.push([]() { return true; });
		std::future<bool> f2 = tp.push([]() { return true; }, 1024);
		if (!f2.get())
		{
			std::cout << "should return true " << __LINE__ << std::endl;
			return __LINE__;
		}
		tp.end();
		if (auto s = tp.threadNum() != 0)
		{
			std::cout << "threadNum should be 0 , " << s << "  " << __LINE__ << std::endl;
			return __LINE__;
		}
	}

	return 0;
}