#include "stdafx.h"
#include "protocol.h"

HANDLE ghIOCP;

void InitializeServerData(void);
void DisplayDebugText(std::string);

int main()
{
	// 01. Main���� ������ ���� �����ϵ��� �Ѵ�.
	InitializeServerData();
	CreateAcceptThread();
	CreateWorkerThreads();
	StopServer();
	return 0;
}

void DisplayDebugText(std::string str)
{
	std::cout << str << std::endl;
	return;
}

void InitializeServerData(void)
{
	// 02. �⺻���� server�� �����ϱ� ���� �ʱ�ȭ �۾��� �����Ѵ�.
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		DisplayDebugText("ERROR :: InitializeServerData :: WSAStartup Fail !!");
		exit(EXIT_FAILURE);
		// exit()	: ���α׷����� ȣ��Ʈ ȯ�濡 ��� ������. atexit() �Լ��� ��ϵ� ��� �Լ��� �������� ȣ��.
		//			: ���α׷��� �����ϱ� ���� ���۸� ��� �����ϰ� ���� ������ ��� ����. 
		//			: EXIT_SUCCESS �Ǵ� 0�� ��������, �׷��� ������ �ٸ� ���� ���� ����.
	}
	
	// WSAStartup()	: ���� �ʱ�ȭ �Լ���, ���α׷����� ����� ���� ������ ��û�����ν� ���� ���̺귯��(WS2_32.DLL)�� �ʱ�ȭ �ϴ� ����.
	//				: return - �����ϸ� 0, �����ϸ� �����ڵ�

	// MAKEWORD(2,2)	: ���α׷��� �䱸�ϴ� �ֻ��� ���� ����. ���� 8��Ʈ�� �� ����, ���� 8��Ʈ�� �� ������ �־ ����
	//					: ���� ���� 3.2 ������ ����� ��û�Ѵٸ� 0x0203 �Ǵ� MAKEWORD(3,2)�� ���
	// wsadata			: WSADATA ����ü�� �����ϸ� �̸� ���� ������ �ü���� �����ϴ� ���� ������ ���� ������ ���� �� ����.
	//					: (���� ���α׷��� ������ ����ϰ� �� ���� ����, �ý����� �����ϴ� ���� �ֻ��� ���� ��), ������ ������ �̷� ������ ������� ����.

	ghIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (NULL == ghIOCP)
	{
		DisplayDebugText("ERROR :: InitializeServerData :: CreateIoCompletionPort Fail !!");
		exit(EXIT_FAILURE);
	}
	
	// CreateIoCompletionPort()	: 1. ����� �Ϸ� ��Ʈ�� ����.
	//							: 2. ���ϰ� ����� �Ϸ���Ʈ�� ����.
	//								���ϰ� ����� �Ϸ���Ʈ�� �����صθ� �� ���Ͽ� ���� �񵿱� ����� ����� ����� �Ϸ���Ʈ�� ����.
	return;
}