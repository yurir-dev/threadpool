
#include "../threadpool.h"
#include "test_common.h"

template< class Rep, class Period >
static void getBusySpin(const std::chrono::duration<Rep, Period>& sleep_duration)
{
	const auto endTime = std::chrono::system_clock::now() + sleep_duration;
	while (auto now = std::chrono::system_clock::now() < endTime)
	{
	}
}

void testCPuUsageManually()
{
	std::vector<int> affinities{ 1, 3 };
	std::cout << "running busy loop on cpus: ";
	for (int i : affinities)
		std::cout << i << ",";
	std::cout << "\b \b" << std::endl;
	
	concurency::threadPool<bool> tp;
	tp.start(affinities);

	std::future<bool> f1 = tp.push([]() {
		getBusySpin(std::chrono::seconds(60));
		return true;
	}, 0);
	std::future<bool> f2 = tp.push([]() {
		getBusySpin(std::chrono::seconds(60));
		return true;
		}, 1);

	bool res = f1.get() && f2.get();
	res = res;

	tp.end();
}


int main(int argc, char* argv[])
{
	if (argc == 2 && std::string(argv[1]) == "manual")
	{
		testCPuUsageManually();
		return 1;
	}

	using tp_t = concurency::threadPool<void>;
	tp_t tp;

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
		if (int res = testCommon::testThreadpool<tp_t>(tp) != 0)
			return res;
	}
	
	return 0;
}