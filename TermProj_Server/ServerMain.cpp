#include "stdafx.h"
#include "protocol.h"

HANDLE ghIOCP;

void DisplayDebugText(std::string);
void DisplayErrMsg(char*, int);

void InitializeServerData(void);
void StopServer();
void AcceptThreadFunc();

int main()
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
	
	for (auto th : worker_threads)
	{
		th->join();
		delete th;
	}

	accept_thread.join();
	StopServer();
	return 0;
}

void DisplayDebugText(std::string str)
{
	std::cout << str << std::endl;
	return;
}

void DisplayErrMsg(char* str, int errNo)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errNo, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("%s", str); wprintf(L" %d Error :: %s\n", errNo, lpMsgBuf);
	LocalFree(lpMsgBuf);
	// FormatMessage() : �����ڵ忡 �����ϴ� ���� �޽����� ���� �� �ִ�.

	// FORMAT_MESSAGE_ALLOCATE_BUFFER			: ���� �޽����� ������ ������ �Լ��� �˾Ƽ� �Ҵ��Ѵٴ� �ǹ�
	// FORMAT_MESSAGE_FROM_SYSTEN				: OS�κ��� ���� �޽����� �����´ٴ� �ǹ�

	// MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT)	: ����ڰ� �����ǿ��� ������ �⺻���� �����޽��� ���� �� ����
	// lpMsgBuf �� ���� �޽����� ������ �����ε� �Լ��� �˾Ƽ� �Ҵ�, 
	// ���� �޽��� ����� ��ġ�� LocalFree() �Լ��� �̿��� �Ҵ��� �޸� ��ȯ �ʼ�
	return;
}

void InitializeServerData(void)
{
	// 02. �⺻���� server�� �����ϱ� ���� �ʱ�ȭ �۾��� �����Ѵ�.
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		DisplayDebugText("ERROR :: InitializeServerData :: [ WSAStartup ] Fail !!");
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
		DisplayDebugText("ERROR :: InitializeServerData :: [ CreateIoCompletionPort ] Fail !!");
		exit(EXIT_FAILURE);
	}
	
	// CreateIoCompletionPort()	: 1. ����� �Ϸ� ��Ʈ�� ����.
	//							: 2. ���ϰ� ����� �Ϸ���Ʈ�� ����.
	//								���ϰ� ����� �Ϸ���Ʈ�� �����صθ� �� ���Ͽ� ���� �񵿱� ����� ����� ����� �Ϸ���Ʈ�� ����.
	DisplayDebugText("Initialize Server Data Success!");
	return;
}

void StopServer()
{
	if (WSACleanup() != 0) {
		DisplayErrMsg("Error :: StopServer :: [ WSACleanup ] Fail !!", GetLastError());
		exit(EXIT_FAILURE);
	}

	// WSACleanup() : ���α׷� ���� �� ���� ���� �Լ�
	//				: ���� ����� �������� �ü���� �˸���, ���� ���Ҹ����� ��ȯ.

	return;
}

void AcceptThreadFunc()
{
	DisplayDebugText("AcceptThreadFunc :: Run!!");
	return;
}