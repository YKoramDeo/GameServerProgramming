#include "stdafx.h"
#include "protocol.h"
#include "IOCP.h"

int main(int argc, char *argv[])
{
	// 01. Main���� ������ ���� �����ϵ��� �Ѵ�.
	InitializeServerData();

	// 03. Main������ Accept Thread�� �����ϴ� ������ �Ѵ�.
	std::thread accept_thread;
	accept_thread = std::thread(AcceptThreadFunc);
	
	// 04. Main������ Worker Thread�� �����ϴ� ������ �Ѵ�.
	//	WorkerThread�� NUM_THREADS���� 6���� ���������� �� thread�� �����ϴ� thread �����̳ʸ� �����Ѵ�.
	std::vector<std::thread*> worker_threads;
	for (int i = 0; i < NUM_THREADS; ++i)
		worker_threads.push_back(new std::thread(WorkerThreadFunc));
	
	for (auto thread : worker_threads)
	{
		thread->join();
		delete thread;
	}

	accept_thread.join();
	StopServer();
	return 0;
}