
#include "../threadpool.h"
#include "test_common.h"

int main(int argc, char* argv[])
{
	concurency::threadPool tp;

	std::vector<std::vector<int>> afinities{
		{1, 2, -1, -1, 5},
		{1, 2, -1, -1, 5, 6},
		{1, 2, -1},
		{1},
		{-1}
	};

	for (size_t i = 1; i < afinities.size(); i += 1)
	{
		tp.start(afinities[i]); // tp.end() is called inside testThreadpool
		if (int res = testCommon::testThreadpool(tp) != 0)
			return res;
	}
	
	return 0;
}