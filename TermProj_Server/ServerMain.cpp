#include "stdafx.h"
#include "protocol.h"

struct WSAOVERLAPPED_EX
{
	WSAOVERLAPPED	origin_over;				// ������ Overlapped ����ü
	WSABUF			wsabuf;						// ������ WSABUF
	int				operation;					// ���� Send or Recv ������ �����ϴ� ������ ���θ� �����ϴ� ����
	unsigned char	iocp_buf[MAX_BUFF_SIZE];	// IOCP Send / Recv Buffer
};

struct Client
{
	bool				connect;					// ���� ���θ� ���� ����
	Object				player;						// ���ӻ��� ��ü�� �⺻���� ������ �����ϴ� ����
	SOCKET				sock;						// accept�� ���ؼ� ������ Network ����� ���� client socket
	WSAOVERLAPPED_EX	recv_overlap;				// recv�� ������ �����ϱ� ���� Ȯ�� overlapped ����ü
	unsigned char		packet_buf[MAX_BUFF_SIZE];	// recv�Ǵ� ��Ŷ�� �����Ǵ� Buffer / Send �� ���� ������ �����Ƿ� Ȯ�� ����ü���� ����
	int					prev_packet_size;			// ������ ���� ���� �����ϴ� ���� / Send �� ���� ������ �����Ƿ� Ȯ�� ����ü���� ����
	int					curr_packet_size;			// ���� ���� packet�� ��
};

HANDLE ghIOCP;

void DisplayDebugText(std::string);
void DisplayErrMsg(char*, int);

void InitializeServerData(void);
void StopServer(void);
void AcceptThreadFunc(void);

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

void DisplayDebugText(std::string msg)
{
	std::cout << msg << std::endl;
	return;
}

void DisplayErrMsg(char* str, int err_no)
{
	WCHAR *msg_buf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&msg_buf, 0, NULL);
	printf("%s", str); wprintf(L" %d Error :: %s\n", err_no, msg_buf);
	LocalFree(msg_buf);
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

void StopServer(void)
{
	if (WSACleanup() != 0) {
		DisplayErrMsg("Error :: StopServer :: [ WSACleanup ] Fail !!", GetLastError());
		exit(EXIT_FAILURE);
	}
	// WSACleanup() : ���α׷� ���� �� ���� ���� �Լ�
	//				: ���� ����� �������� �ü���� �˸���, ���� ���Ҹ����� ��ȯ.

	return;
}

void AcceptThreadFunc(void)
{
	SOCKET accept_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == accept_sock)
	{
		DisplayErrMsg("Error :: AcceptThreadFunc :: Create Accept Socket Fail !!", WSAGetLastError());
		return;
	}
	// WSASocket(1, 2, 3, 4, 5, 6) : socket�� �����ϴ� �Լ�
	// 1. af			 : address family - AF_INET�� ��� (�� �ۿ� AF_NETBIOS, AF_IRDA, AF_INET6�� ����.)
	// 2. type			 : ������ Ÿ�� - tcp�� ���� SOCKET_STREAM��� (SOCK_DGRAM : udp)
	// 3. protocol		 : ����� ���������� ���� - IPPROTO_TCP (IPPROTO_UDP)
	// 4. lpPtotocolInfo : �������� ���� - ���� NULL
	// 5. g				 : ����
	// 6. dgFlags		 : ������ �Ӽ� - ���� 0 (�Ǵ� WSA_PROTOCOL_OVERLAPPED)

	struct sockaddr_in listen_addr;
	ZeroMemory(&listen_addr, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(MY_SERVER_PORT);
	ZeroMemory(&listen_addr.sin_zero, 8);
	// sockaddr_in : ���� �ּ� ����ü socket address structures
	//			   : ��Ʈ��ũ ���α׷����� �ʿ��� �ּ� ������ ��� �ִ� ����ü.
	//			   : Ư��, ���⼭ ���� sockaddr_in�� IPv4�� ���� ���� �ּ� ����ü.
	// 1. short		sin_family	: �ּ� ü�踦 �ǹ��ϸ� AF_INET(���ͳ� �ּ� ü��, IPv4) ���� ����Ѵ�.
	// 2. struct in_addr sin_addr 
	//			: IP �ּҸ� �ǹ��ϸ�, ���� 32��Ʈ in_addr ����ü�� ����Ѵ�.
	//			: �������α׷������� �밳 32��Ʈ ������ �����ϹǷ� S_un.S_addr�ʵ带 ����ϰ� ��ũ�θ� ���� ������ �� s_addr�� ����ϸ� ���ϴ�.
	//			: INADDR_ANY	: ������ ���� IP�ּҸ� �����ϴµ� �־� ������ ���� �����ϸ� ������ IP�ּҸ� 2�� �̻� ������ ���,
	//						    : (multihomed host ��� �θ�), Ŭ���̾�Ʈ�� ��� IP�ּҷ� �����ϵ� �޾Ƶ��� �� �ֵ��� ����.
	// 3. u_short	sin_port	: ��Ʈ ��ȣ�� �ǹ�, ��ȣ ���� 16��Ʈ ���� ���� ����Ѵ�.
	// 4. char		sin_zero[8] : 0���� �����ϸ� �Ǵ� ���� �����̴�.

	int ret_val = 0;
	ret_val = ::bind(accept_sock, reinterpret_cast<sockaddr*>(&listen_addr), sizeof(listen_addr));
	if (SOCKET_ERROR == ret_val)
	{
		DisplayErrMsg("Error :: AcceptThreadFunc :: Bind Fail !!", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	// bind() : ������ ���� IP�ּҿ� ���� ��Ʈ ��ȣ�� ����, 
	//		  : C++11�� bind��� �Լ��� ȥ�� ������ :: ����Ȯ�� �����ڿ� ���� ���
	// listen_addr : ���� IP�� ���� ��Ʈ ��ȣ�� �ʱ�ȭ�� ���� �ּ� ����ü�� ����.
	// bind �Լ��� 2��° ���ڴ� �׻� (SOCKADDR*)���� ��ȯ�ؾ� ��.

	ret_val = listen(accept_sock, SOMAXCONN);
	if (SOCKET_ERROR == ret_val)
	{
		DisplayErrMsg("Error :: AcceptThreadFunc :: Listen Fail !!",WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	// listen()  : ������ TCP ��Ʈ ���¸� LISTENING���·� �ٲ۴�. �̴� Ŭ���̾�Ʈ ������ �޾Ƶ��� �� �ִ� ���°� ���� �ǹ�
	// SOMAXCONN : Ŭ���̾�Ʈ�� ���� ������ ���� ť�� ����Ǵµ�, backlog�� �� ���� ť�� ���̸� ��Ÿ��.
	//			 : SOMAXCONN�� �ǹ̴� �Ϻ� �������ݿ��� ���� ������ �ִ��� ���

	while (true)
	{
		int new_id = -1; // ������ Ŭ���̾�Ʈ�� �� ID, ���� ������� �ʴ� -1�� �ʱ�ȭ
		struct sockaddr_in client_addr;
		int addr_size = sizeof(client_addr);

		SOCKET new_client_sock = WSAAccept(accept_sock, reinterpret_cast<SOCKADDR*>(&client_addr), &addr_size, NULL, NULL);
		if (INVALID_SOCKET == new_client_sock)
		{
			DisplayErrMsg("Error :: AcceptThreadFunc :: WSAAccept Fail !!", WSAGetLastError());
			break;
		}
		// WSAAccept()	: ������ Ŭ���̾�Ʈ�� ����� �� �ֵ��� ���ο� ������ �����ؼ� ���� ���� ������ Ŭ���̾�Ʈ�� �ּ������� �˷���
		//				: �������忡���� ���� IP�ּҿ� ���� ��Ʈ��ȣ, Ŭ���̾�Ʈ ���忡���� ���� IP�ּҿ� ���� ��Ʈ��ȣ
		
		// ADD::������ Ŭ���̾�Ʈ�� ���ο� ID�� �ο��ϴ� ����
		if (-1 == new_id)
		{
			DisplayDebugText("AcceptThread :: Maximum User Number Sorry :(");
			closesocket(new_client_sock);
			continue;
		}

		// Accept ���� ���Ŀ� ����� ���־�� ��.
		HANDLE result = CreateIoCompletionPort(reinterpret_cast<HANDLE>(new_client_sock), ghIOCP, new_id, 0);
		if (NULL == result)
		{
			DisplayErrMsg("Error :: AcceptThreadFunc :: CreateIoCompletionPort Fail !!", WSAGetLastError());
			closesocket(new_client_sock);
			continue;
		}
		// CreateIoCompletionPort() : ���ϰ� ����� �Ϸ���Ʈ�� ����.
		//							: ���ϰ� ����� �Ϸ���Ʈ�� �����صθ� �� ���Ͽ� ���� �񵿱� ����� ����� ����� �Ϸ���Ʈ�� ����.

		// ADD::����� Ŭ���̾�Ʈ Network ���� �ʱ�ȭ
		// ADD::����� ���ο� ���� Recv����.
	}
	return;
}

void WorkerThreadFunc(void)
{
	return;
}