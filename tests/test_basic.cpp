
#include "../threadpool.h"
#include "test_common.h"

#include <cmath>

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
	std::packaged_task < double (int, int) > task([](int a, int b) {
		return std::pow(a, b);
	});
	std::future<double> result = task.get_future();

	task(2, 9);
	//std::cout << "task_lambda:\t" << result.get() << '\n';
	return 0;

	if (auto res = testTp<concurency::threadPool>() != 0)
		return res;
	if (auto res = testTp<concurency::threadPoolOrdered>() != 0)
		return res;
	return 0;
}