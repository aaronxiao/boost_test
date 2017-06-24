
#ifndef CT_ASYNC_H
#define CT_ASYNC_H

/*
测试 thread 的类，CT 表示 CTest
知识点：
    future    shared_future   
	thread_group  
*/
class CT_async
{
public:
	CT_async() {}
	virtual  ~CT_async() {}

	// shared_future、future 的 wait 方式，极力推荐用 boost::bind 绑定函数
	void case2_1();

	// shared_future、future 的 wait_for 方式，极力推荐用 boost::bind 绑定函数
	void case2_2();

	// wait_for_any ，任意一个函数结束就解除阻塞
	void case3();

	// wait_for_all ，所有元素对应的线程都返回才解除阻塞
	void case3_1();

	// promise 的最简易应用
	void case5();

	/*
	barrier 栅栏。
	barrier 可用于多线程同步，当线程执行到 barrier::wait 时必须等待，直到所有线程都到达这个等待点时才能一齐继续执行，其另一个名字 redezvous（约会点）更形象
	就象赛马场的跑道上，将栅栏一打开，几匹马才能飞奔而出。
	*/
	void case6();

	// thread_specific_ptr , 此段暂时不研究
	void case7();

	int main();
};

#endif