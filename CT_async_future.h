
#ifndef CT_ASYNC_H
#define CT_ASYNC_H

/*
���� thread ���࣬CT ��ʾ CTest
֪ʶ�㣺
    future    shared_future   
	thread_group  
*/
class CT_async
{
public:
	CT_async() {}
	virtual  ~CT_async() {}

	// shared_future��future �� wait ��ʽ�������Ƽ��� boost::bind �󶨺���
	void case2_1();

	// shared_future��future �� wait_for ��ʽ�������Ƽ��� boost::bind �󶨺���
	void case2_2();

	// wait_for_any ������һ�����������ͽ������
	void case3();

	// wait_for_all ������Ԫ�ض�Ӧ���̶߳����زŽ������
	void case3_1();

	// promise �������Ӧ��
	void case5();

	/*
	barrier դ����
	barrier �����ڶ��߳�ͬ�������߳�ִ�е� barrier::wait ʱ����ȴ���ֱ�������̶߳���������ȴ���ʱ����һ�����ִ�У�����һ������ redezvous��Լ��㣩������
	�����������ܵ��ϣ���դ��һ�򿪣���ƥ����ܷɱ�������
	*/
	void case6();

	// thread_specific_ptr , �˶���ʱ���о�
	void case7();

	int main();
};

#endif