
#include "tp/threadpool.h"
#include "test_common.h"


int main(int /*argc*/, char* /*argv*/[])
{
	using tp_t = concurency::threadPool<void> ;
	tp_t tp;
	for (size_t i = 1; i < 5; i += 1)
	{
		tp.start(i); // tp.end() is called inside testThreadpool
		if (int res = testCommon::testThreadpool<tp_t>(tp) != 0)
			return res;
	}
	return 0;
}