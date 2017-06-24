
#include "CT_async_future.h"

#include < iostream >
#include < string >
#include <vector>



#include "LogicHead.h"
#include "BoostOften.h"

#include <boost/bind.hpp>
#include <boost/atomic.hpp>
#include <boost/thread.hpp>
#include <boost/thread/lock_factories.hpp>

// 注意：这一句与 using namespace std;  是有冲突的，导致编译时产生 “shared_ptr”: 不明确的符号。
// 例如 boost::shared_ptr 与 std::shared_ptr 都存在，编译器不知该选哪一个。
// 若 boost 和 std 都必需存在，折衷的解决方案是加前缀定义，例如：boost::shared_ptr aa(new int) 
//using namespace boost;

// 注意：这一句与 using namespace boost 是有冲突的，导致编译时产生 “shared_ptr”: 不明确的符号。
// 例如 boost::shared_ptr 与 std::shared_ptr 都存在，编译器不知该选哪一个。
// 若 boost 和 std 都必需存在，折衷的解决方案是加前缀定义，例如： std::shared_ptr aa(new int) 
//using namespace std;	

// 有形参时编译总是会报错
int fab_01(int n, int iSleep)
{
	if (iSleep>0)
		boost::this_thread::sleep_for(get_ms(iSleep));	// 线程睡眠等待一小段时间

	return n;
}

// shared_future、future 的 wait 方式，极力推荐用 boost::bind 绑定函数
void CT_async::case2_1()
{
	bool bRes = false;
	int  iRes = 0;
	int iTmp = 0;
	
	int const iFabResult = 202;
	int iSleepTime = 5000;			// 睡眠时间，ms

#if 0
	// 不赞成的绑定方式
	// 编译时会报错“error C2064: 项不会计算为接受 0 个参数的函数”
	// 这种写法只有当 fab_01 函数没有形参时才能通过编译
	boost::shared_future<int> f1 = boost::async(boost::launch::async, fab_01, iFabResult, iSleepTime);
#else
	/*
	推荐：用 bind 绑定函数的方式，下述是线程启动策略的描述
	launch::none					无意义，不能使用，会崩溃
	launch::async					立即启动线程函数
	launch::deferred				只有当 future对象显式调用 get 或 wait 时才会启动线程
	launch::any = async | deferred  两者选其一： async 和 deferred ，如何选并不确定
	*/
	boost::shared_future<int> f2 = boost::async(boost::launch::async, boost::bind(fab_01, iFabResult, iSleepTime));
#endif

	std::cout << "开始 wait " << std::endl;

	f2.wait();	// 等待，是死等，直至 fab_01 函数结束

	std::cout << "wait 结束" << std::endl;

	iRes = f2.get();
	std::cout << "get 已取值" << std::endl << std::endl;
	
#if 1
	// OK的,用 lambda 方式绑定无形参的函数
	boost::future<int> f4 = boost::async([]() { return 123; });  
#else
	// 编译时会报错“error C2064: 项不会计算为接受 0 个参数的函数”，带形参的不行。
	boost::future<int> f5 = boost::async([](int n) { return n; });
#endif
}

// shared_future、future 的 wait_for 方式，极力推荐用 boost::bind 绑定函数
void CT_async::case2_2()
{
	bool bRes = false;
	int  iRes = 0;
	int iTmp = 0;

	int const iFabResult = 202;
	int iSleepTime = 5000;			// 睡眠时间，ms

	/*
	推荐：用 bind 绑定函数的方式，下述是线程启动策略的描述
		launch::none					无意义，不能使用，会崩溃
		launch::async					立即启动线程函数
		launch::deferred				只有当 future对象显式调用 get 或 wait 时才会启动线程
		launch::any = async | deferred  两者选其一： async 和 deferred ，如何选并不确定
	*/
	boost::shared_future<int> f2 = boost::async(boost::launch::async, boost::bind(fab_01, iFabResult, iSleepTime));

	boost::future_status stStatus = f2.wait_for(get_ms(100));	// 有时间长度的等待
	switch (stStatus)
	{
	case boost::future_status::ready:		// 已经完成计算
		iTmp = 10;
		break;
	case boost::future_status::timeout:		// 因超时而失败
		iTmp = 20;
		break;
	case boost::future_status::deferred:	// 还未开始计算
		iTmp = 30;
		break;
	}

	bRes = f2.valid();	// 是否 已完成计算
	if (bRes)	// 已完成计算
	{
		std::cout << "即将 get" << std::endl;

		// 获取计算结果，即 fab_01 函数的返回值
		// 注意：此函数是一定会“等”到 fab_01 函数结束的，也就意味着是“阻塞”方式。
		iRes = f2.get();	

		std::cout << "get 已取值" << std::endl;
	}
}

// wait_for_any ，任意一个函数结束就解除阻塞
void CT_async::case3()
{
	const int MAX_COUNT = 5;
	int iSleepTime = 1000;			// 睡眠时间，ms
	std::vector<boost::future<int>> vec;
	
	for (int i = 0; i < MAX_COUNT; ++i)
	{
	#if 0
		// 会报错“error C2280: “boost::future<R>::future(const boost::future<R> &)”: 尝试引用已删除的函数”
		boost::future<int>& tmp = boost::async( boost::launch::async, boost::bind(fab_01, 100 + i, iSleepTime));
		vec.push_back(tmp);
	#else
		// 直接插入是OK的
		vec.push_back(boost::async(boost::launch::async, boost::bind(fab_01, 100 + i, iSleepTime*(2*i+0))));
	#endif
	}

	std::cout << "等待 vec[3], vec[4], vec[2] 里面任意一个函数结束......" << std::endl;

	// vec[2], vec[1], vec[0] 对应的线程函数，任意一个函数结束就解除阻塞
	wait_for_any(vec[2], vec[1], vec[0]); 

	std::cout << "vec[2], vec[1], vec[0] 至少有一个结束啦！！！" << std::endl << std::endl;

	for (auto& x : vec)
	{
		if (x.valid())
			std::cout << x.get() << std::endl;
		else
			std::cout << "还未好" << std::endl;
	}
}

// wait_for_all ，所有元素对应的线程都返回才解除阻塞
void CT_async::case3_1()
{
	const int MAX_COUNT = 5;
	int iSleepTime = 1000;			// 睡眠时间，ms
	std::vector<boost::future<int>> vec;

	for (int i = 0; i < MAX_COUNT; ++i)
	{
	#if 0
		// 会报错“error C2280: “boost::future<R>::future(const boost::future<R> &)”: 尝试引用已删除的函数”
		boost::future<int>& tmp = boost::async( boost::launch::async, boost::bind(fab_01, 100 + i, iSleepTime));
		vec.push_back(tmp);
	#else
		// 直接插入是OK的
		vec.push_back(boost::async(boost::launch::async, boost::bind(fab_01, 100 + i, iSleepTime*i)));
	#endif
	}

	std::cout << "等待 vec.begin() 直至 vec.end() 都结束......" << std::endl;

	// 等待vec 里所有元素对应的线程都结束才解除阻塞
	wait_for_all(vec.begin(), vec.end());

	std::cout << "vec.begin() 直至 vec.end() 都结束啦！！！" << std::endl << std::endl;

	for (auto& x : vec)
	{
		if (x.valid())
			std::cout << x.get() << std::endl;
		else
			std::cout << "还未好" << std::endl;
	}
}

// promise 的最简易应用
void CT_async::case5()
{
	// 用 lambdy 方式定义一个线程函数
	// 使用 promise 作为线程函数输出参数
	auto func = [](int n, boost::promise<int>& pms)
	{
		pms.set_value(fab_01(n, 5000));
		//pms.set_value(boost::bind(fab_01, n, 5000));	// 这样不行，编译会报错
	};

	// promise 变量
	boost::promise<int> pms;

	// 启动线程，以 detach 不管的方式，因为后面要等计算结果，所以不用 join 方式
	boost::thread(func, 10, boost::ref(pms)).detach();

	printf("计算中，在等待结果...... \n");

	// 等待 future 计算结果
	// 本函数几乎不耗时间的
#if 1
	boost::future<int> f = pms.get_future();	// OK 的
#else
	auto f = pms.get_future();	// OK 的
#endif

	printf("pms.get_future 已结束 \n");

	// 以阻塞的方式等待线程函数结束，最耗时间啦
	int iRes = f.get();

	printf("f.get 结果是 %d \n", iRes);
}

/*
	barrier 栅栏。
	barrier 可用于多线程同步，当线程执行到 barrier::wait 时必须等待，直到所有线程都到达这个等待点时才能一齐继续执行，其另一个名字 redezvous（约会点）更形象
	就象赛马场的跑道上，将栅栏一打开，几匹马才能飞奔而出。
*/
void CT_async::case6()
{
	const int WAIT_COUNT = 5;	// 栅栏的等待数量

	boost::atomic<int> x;
	x = 0;

	boost::barrier br(WAIT_COUNT);	// 栅栏对象，形参设置了等待数量 WAIT_COUNT

	auto func = [&]()
	{
		//std::cout << "thread" << ++x << " arrived barrier." << std::endl;
		printf("thread %d 开始等待 \n", ++x);

		br.wait();

		//std::cout << "thread run." << std::endl;
		printf("本线程结束 \n");
	};

	boost::thread_group tg;
	for (int i = 0; i < WAIT_COUNT; ++i)
	{
		tg.create_thread(func);
	}
	tg.join_all();
}

// thread_specific_ptr , 此段暂时不研究
void CT_async::case7()
{
#if 0
	thread_specific_ptr<int> pi;

	auto func = [&]{
		pi.reset(new int());

		++(*pi);
		std::cout << "thread v=" << *pi << std::endl;
	};
	async(func);
	async(func);

	this_thread::sleep_for(100_ms);
#endif
}

int CT_async::main()
{
	//case2_1();
	//case2_2();

	//case3();
	//case3_1();

	//case5();

	case6();
	//case7();
	//case8();

	return 0;
}