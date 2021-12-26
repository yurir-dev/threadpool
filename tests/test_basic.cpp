
#include "../threadpool.h"
#include "test_common.h"

int main(int argc, char* argv[])
{
	concurency::threadPool tp;
	tp.start(5);

	int res = testCommon::testThreadpool(tp);
	return res;
}