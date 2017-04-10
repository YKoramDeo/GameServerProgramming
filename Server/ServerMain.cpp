#include "stdafx.h"
#include "protocol.h"

void InitializeServerData();

int main()
{
	// 1. Main문은 다음과 같이 동작하도록 한다.
	InitializeServerData();
	CreateAcceptThread();
	CreateWorkerThreads();
	StopServer();
	return 0;
}

void InitializeServerData()
{
	// 2. 기본적인 server를 동작하기 위한 초기화 작업을 진행한다.
	
	return;
}