
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

// ע�⣺��һ���� using namespace std;  ���г�ͻ�ģ����±���ʱ���� ��shared_ptr��: ����ȷ�ķ��š�
// ���� boost::shared_ptr �� std::shared_ptr �����ڣ���������֪��ѡ��һ����
// �� boost �� std ��������ڣ����ԵĽ�������Ǽ�ǰ׺���壬���磺boost::shared_ptr aa(new int) 
//using namespace boost;

// ע�⣺��һ���� using namespace boost ���г�ͻ�ģ����±���ʱ���� ��shared_ptr��: ����ȷ�ķ��š�
// ���� boost::shared_ptr �� std::shared_ptr �����ڣ���������֪��ѡ��һ����
// �� boost �� std ��������ڣ����ԵĽ�������Ǽ�ǰ׺���壬���磺 std::shared_ptr aa(new int) 
//using namespace std;	

// ���β�ʱ�������ǻᱨ��
int fab_01(int n, int iSleep)
{
	if (iSleep>0)
		boost::this_thread::sleep_for(get_ms(iSleep));	// �߳�˯�ߵȴ�һС��ʱ��

	return n;
}

// shared_future��future �� wait ��ʽ�������Ƽ��� boost::bind �󶨺���
void CT_async::case2_1()
{
	bool bRes = false;
	int  iRes = 0;
	int iTmp = 0;
	
	int const iFabResult = 202;
	int iSleepTime = 5000;			// ˯��ʱ�䣬ms

#if 0
	// ���޳ɵİ󶨷�ʽ
	// ����ʱ�ᱨ��error C2064: ������Ϊ���� 0 �������ĺ�����
	// ����д��ֻ�е� fab_01 ����û���β�ʱ����ͨ������
	boost::shared_future<int> f1 = boost::async(boost::launch::async, fab_01, iFabResult, iSleepTime);
#else
	/*
	�Ƽ����� bind �󶨺����ķ�ʽ���������߳��������Ե�����
	launch::none					�����壬����ʹ�ã������
	launch::async					���������̺߳���
	launch::deferred				ֻ�е� future������ʽ���� get �� wait ʱ�Ż������߳�
	launch::any = async | deferred  ����ѡ��һ�� async �� deferred �����ѡ����ȷ��
	*/
	boost::shared_future<int> f2 = boost::async(boost::launch::async, boost::bind(fab_01, iFabResult, iSleepTime));
#endif

	std::cout << "��ʼ wait " << std::endl;

	f2.wait();	// �ȴ��������ȣ�ֱ�� fab_01 ��������

	std::cout << "wait ����" << std::endl;

	iRes = f2.get();
	std::cout << "get ��ȡֵ" << std::endl << std::endl;
	
#if 1
	// OK��,�� lambda ��ʽ�����βεĺ���
	boost::future<int> f4 = boost::async([]() { return 123; });  
#else
	// ����ʱ�ᱨ��error C2064: ������Ϊ���� 0 �������ĺ����������βεĲ��С�
	boost::future<int> f5 = boost::async([](int n) { return n; });
#endif
}

// shared_future��future �� wait_for ��ʽ�������Ƽ��� boost::bind �󶨺���
void CT_async::case2_2()
{
	bool bRes = false;
	int  iRes = 0;
	int iTmp = 0;

	int const iFabResult = 202;
	int iSleepTime = 5000;			// ˯��ʱ�䣬ms

	/*
	�Ƽ����� bind �󶨺����ķ�ʽ���������߳��������Ե�����
		launch::none					�����壬����ʹ�ã������
		launch::async					���������̺߳���
		launch::deferred				ֻ�е� future������ʽ���� get �� wait ʱ�Ż������߳�
		launch::any = async | deferred  ����ѡ��һ�� async �� deferred �����ѡ����ȷ��
	*/
	boost::shared_future<int> f2 = boost::async(boost::launch::async, boost::bind(fab_01, iFabResult, iSleepTime));

	boost::future_status stStatus = f2.wait_for(get_ms(100));	// ��ʱ�䳤�ȵĵȴ�
	switch (stStatus)
	{
	case boost::future_status::ready:		// �Ѿ���ɼ���
		iTmp = 10;
		break;
	case boost::future_status::timeout:		// ��ʱ��ʧ��
		iTmp = 20;
		break;
	case boost::future_status::deferred:	// ��δ��ʼ����
		iTmp = 30;
		break;
	}

	bRes = f2.valid();	// �Ƿ� ����ɼ���
	if (bRes)	// ����ɼ���
	{
		std::cout << "���� get" << std::endl;

		// ��ȡ���������� fab_01 �����ķ���ֵ
		// ע�⣺�˺�����һ���ᡰ�ȡ��� fab_01 ���������ģ�Ҳ����ζ���ǡ���������ʽ��
		iRes = f2.get();	

		std::cout << "get ��ȡֵ" << std::endl;
	}
}

// wait_for_any ������һ�����������ͽ������
void CT_async::case3()
{
	const int MAX_COUNT = 5;
	int iSleepTime = 1000;			// ˯��ʱ�䣬ms
	std::vector<boost::future<int>> vec;
	
	for (int i = 0; i < MAX_COUNT; ++i)
	{
	#if 0
		// �ᱨ��error C2280: ��boost::future<R>::future(const boost::future<R> &)��: ����������ɾ���ĺ�����
		boost::future<int>& tmp = boost::async( boost::launch::async, boost::bind(fab_01, 100 + i, iSleepTime));
		vec.push_back(tmp);
	#else
		// ֱ�Ӳ�����OK��
		vec.push_back(boost::async(boost::launch::async, boost::bind(fab_01, 100 + i, iSleepTime*(2*i+0))));
	#endif
	}

	std::cout << "�ȴ� vec[3], vec[4], vec[2] ��������һ����������......" << std::endl;

	// vec[2], vec[1], vec[0] ��Ӧ���̺߳���������һ�����������ͽ������
	wait_for_any(vec[2], vec[1], vec[0]); 

	std::cout << "vec[2], vec[1], vec[0] ������һ��������������" << std::endl << std::endl;

	for (auto& x : vec)
	{
		if (x.valid())
			std::cout << x.get() << std::endl;
		else
			std::cout << "��δ��" << std::endl;
	}
}

// wait_for_all ������Ԫ�ض�Ӧ���̶߳����زŽ������
void CT_async::case3_1()
{
	const int MAX_COUNT = 5;
	int iSleepTime = 1000;			// ˯��ʱ�䣬ms
	std::vector<boost::future<int>> vec;

	for (int i = 0; i < MAX_COUNT; ++i)
	{
	#if 0
		// �ᱨ��error C2280: ��boost::future<R>::future(const boost::future<R> &)��: ����������ɾ���ĺ�����
		boost::future<int>& tmp = boost::async( boost::launch::async, boost::bind(fab_01, 100 + i, iSleepTime));
		vec.push_back(tmp);
	#else
		// ֱ�Ӳ�����OK��
		vec.push_back(boost::async(boost::launch::async, boost::bind(fab_01, 100 + i, iSleepTime*i)));
	#endif
	}

	std::cout << "�ȴ� vec.begin() ֱ�� vec.end() ������......" << std::endl;

	// �ȴ�vec ������Ԫ�ض�Ӧ���̶߳������Ž������
	wait_for_all(vec.begin(), vec.end());

	std::cout << "vec.begin() ֱ�� vec.end() ��������������" << std::endl << std::endl;

	for (auto& x : vec)
	{
		if (x.valid())
			std::cout << x.get() << std::endl;
		else
			std::cout << "��δ��" << std::endl;
	}
}

// promise �������Ӧ��
void CT_async::case5()
{
	// �� lambdy ��ʽ����һ���̺߳���
	// ʹ�� promise ��Ϊ�̺߳����������
	auto func = [](int n, boost::promise<int>& pms)
	{
		pms.set_value(fab_01(n, 5000));
		//pms.set_value(boost::bind(fab_01, n, 5000));	// �������У�����ᱨ��
	};

	// promise ����
	boost::promise<int> pms;

	// �����̣߳��� detach ���ܵķ�ʽ����Ϊ����Ҫ�ȼ����������Բ��� join ��ʽ
	boost::thread(func, 10, boost::ref(pms)).detach();

	printf("�����У��ڵȴ����...... \n");

	// �ȴ� future ������
	// ��������������ʱ���
#if 1
	boost::future<int> f = pms.get_future();	// OK ��
#else
	auto f = pms.get_future();	// OK ��
#endif

	printf("pms.get_future �ѽ��� \n");

	// �������ķ�ʽ�ȴ��̺߳������������ʱ����
	int iRes = f.get();

	printf("f.get ����� %d \n", iRes);
}

/*
	barrier դ����
	barrier �����ڶ��߳�ͬ�������߳�ִ�е� barrier::wait ʱ����ȴ���ֱ�������̶߳���������ȴ���ʱ����һ�����ִ�У�����һ������ redezvous��Լ��㣩������
	�����������ܵ��ϣ���դ��һ�򿪣���ƥ����ܷɱ�������
*/
void CT_async::case6()
{
	const int WAIT_COUNT = 5;	// դ���ĵȴ�����

	boost::atomic<int> x;
	x = 0;

	boost::barrier br(WAIT_COUNT);	// դ�������β������˵ȴ����� WAIT_COUNT

	auto func = [&]()
	{
		//std::cout << "thread" << ++x << " arrived barrier." << std::endl;
		printf("thread %d ��ʼ�ȴ� \n", ++x);

		br.wait();

		//std::cout << "thread run." << std::endl;
		printf("���߳̽��� \n");
	};

	boost::thread_group tg;
	for (int i = 0; i < WAIT_COUNT; ++i)
	{
		tg.create_thread(func);
	}
	tg.join_all();
}

// thread_specific_ptr , �˶���ʱ���о�
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