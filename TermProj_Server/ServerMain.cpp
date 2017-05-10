#include "stdafx.h"
#include "protocol.h"
#include "IOCP.h"

int main(int argc, char *argv[])
{
	// 01. Main문은 다음과 같이 동작하도록 한다.
	InitializeServerData();

	// 03. Main문에서 Accept Thread를 생성하는 선언을 한다.
	std::thread accept_thread;
	accept_thread = std::thread(AcceptThreadFunc);
	
	// 04. Main문에서 Worker Thread를 생성하는 선언을 한다.
	//	WorkerThread는 NUM_THREADS에서 6개로 지정했으니 이 thread를 관리하는 thread 컨테이너를 선언한다.
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