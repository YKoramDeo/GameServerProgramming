#include "stdafx.h"
#include "protocol.h"

void InitializeServerData();

int main()
{
	// 1. Main���� ������ ���� �����ϵ��� �Ѵ�.
	InitializeServerData();
	CreateAcceptThread();
	CreateWorkerThreads();
	StopServer();
	return 0;
}

void InitializeServerData()
{
	// 2. �⺻���� server�� �����ϱ� ���� �ʱ�ȭ �۾��� �����Ѵ�.
	
	return;
}