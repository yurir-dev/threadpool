
#include "../threadpool.h"
#include "test_common.h"


template <class TP>
int testTp()
{
	TP tp;
	for (size_t i = 1; i < 5; i += 1)
	{
		tp.start(i); // tp.end() is called inside testThreadpool
		if (int res = testCommon::testThreadpool<TP>(tp) != 0)
			return res;
	}
	return 0;
}

int main(int argc, char* argv[])
{
	if (auto res = testTp<concurency::threadPool>() != 0)
		return res;
	if (auto res = testTp<concurency::threadPoolOrdered>() != 0)
		return res;
	return 0;
}