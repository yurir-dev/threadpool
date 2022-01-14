# threadpool

Threadpool implementation in C++, it keeps several threads on hold and accepts std::function<T()> to execute in the context of one of these threads.

1) during idle time threads don't waste cpu time, they wait for task using condition_variable.

2) every thread waits on it's own task queue, so callers won't wait a lot to push their tasks.

3) normally a random thread will handle a task, however, it's possible to enforce order by providing the same hash, 
	so some tasks will be executed by the same thread sequentially.

4) threadpool is templated on the return type, all tasks will return same type, it's possible to wait for it or just ignore it.


All the code is in tp/threadpool.h tp/threadsafe_queue.h
usage examples are in tests/test_*.cpp



from tests/test_interface.cpp


 // define threadpool that accepts std::function<bool()> 
 // start it with 3 threads

using tp_t = concurency::threadPool<bool>;

tp_t tp;

tp.start(3);


// push some tasks, wait for return value of one of them

tp.push([]() { return true; });

std::future<bool> f1 = tp.push([]() { return true; }, 1);

bool res1 = f1.get();


// shut it down

tp.end();




// restart threadpool with 4 threads, threads 1, 2 and 4 are not pinned, thread 3 will be pinned on core 2

tp.start({-1, -1, 2, -1});


// push some tasks, wait for return value of one of them

tp.push([]() { return true; });

std::future<bool> f2 = tp.push([]() { return true; }, 1024);

bool res2 = f2.get();


// shut it down

tp.end();
