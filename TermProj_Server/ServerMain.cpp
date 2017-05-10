#include "stdafx.h"
#include "protocol.h"

enum EVENTTYPE { E_RECV, E_SEND };

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
SOCKET gServerSock;
Client gClientsList[MAX_USER];

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
	
	gServerSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == gServerSock)
	{
		DisplayErrMsg("Error :: InitializeServerData :: Create Global Server Socket Fail !!", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	// WSASocket(1, 2, 3, 4, 5, 6) : socket�� �����ϴ� �Լ�
	// 1. af			 : address family - AF_INET�� ��� (�� �ۿ� AF_NETBIOS, AF_IRDA, AF_INET6�� ����.)
	// 2. type			 : ������ Ÿ�� - tcp�� ���� SOCKET_STREAM��� (SOCK_DGRAM : udp)
	// 3. protocol		 : ����� ���������� ���� - IPPROTO_TCP (IPPROTO_UDP)
	// 4. lpPtotocolInfo : �������� ���� - ���� NULL
	// 5. g				 : ����
	// 6. dgFlags		 : ������ �Ӽ� - ���� 0 (�Ǵ� WSA_PROTOCOL_OVERLAPPED)

	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(MY_SERVER_PORT);
	ZeroMemory(&server_addr.sin_zero, 8);
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
	ret_val = ::bind(gServerSock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	if (SOCKET_ERROR == ret_val)
	{
		DisplayErrMsg("Error :: InitializeServerData :: Bind Fail !!", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	// bind() : ������ ���� IP�ּҿ� ���� ��Ʈ ��ȣ�� ����, 
	//		  : C++11�� bind��� �Լ��� ȥ�� ������ :: ����Ȯ�� �����ڿ� ���� ���
	// listen_addr : ���� IP�� ���� ��Ʈ ��ȣ�� �ʱ�ȭ�� ���� �ּ� ����ü�� ����.
	// bind �Լ��� 2��° ���ڴ� �׻� (SOCKADDR*)���� ��ȯ�ؾ� ��.

	ret_val = listen(gServerSock, SOMAXCONN);
	if (SOCKET_ERROR == ret_val)
	{
		DisplayErrMsg("Error :: AcceptThreadFunc :: Listen Fail !!", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	// listen()  : ������ TCP ��Ʈ ���¸� LISTENING���·� �ٲ۴�. �̴� Ŭ���̾�Ʈ ������ �޾Ƶ��� �� �ִ� ���°� ���� �ǹ�
	// SOMAXCONN : Ŭ���̾�Ʈ�� ���� ������ ���� ť�� ����Ǵµ�, backlog�� �� ���� ť�� ���̸� ��Ÿ��.
	//			 : SOMAXCONN�� �ǹ̴� �Ϻ� �������ݿ��� ���� ������ �ִ��� ���

	DisplayDebugText("Initialize Server Data Success!");
	return;
}

void StopServer(void)
{
	for (auto client : gClientsList)
		if (client.connect)
			closesocket(client.sock);

	closesocket(gServerSock);

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
	int ret_val = 0;
	SOCKADDR_IN client_addr;
	ZeroMemory(&client_addr, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(MY_SERVER_PORT);
	client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int addr_size = sizeof(client_addr);
	
	while (true)
	{
		SOCKET new_client_sock = WSAAccept(gServerSock, reinterpret_cast<SOCKADDR*>(&client_addr), &addr_size, NULL, NULL);
		if (INVALID_SOCKET == new_client_sock)
		{
			DisplayErrMsg("Error :: AcceptThreadFunc :: WSAAccept Fail !!", WSAGetLastError());
			break;
		}
		// WSAAccept()	: ������ Ŭ���̾�Ʈ�� ����� �� �ֵ��� ���ο� ������ �����ؼ� ���� ���� ������ Ŭ���̾�Ʈ�� �ּ������� �˷���
		//				: �������忡���� ���� IP�ּҿ� ���� ��Ʈ��ȣ, Ŭ���̾�Ʈ ���忡���� ���� IP�ּҿ� ���� ��Ʈ��ȣ
		
		int new_id = -1; // ������ Ŭ���̾�Ʈ�� �� ID, ���� ������� �ʴ� -1�� �ʱ�ȭ
		// ADD::������ Ŭ���̾�Ʈ�� ���ο� ID�� �ο��ϴ� ����
		for (auto i = 0; i < MAX_USER; ++i)
		{
			if (gClientsList[i].connect == false)
			{
				new_id = i;
				break;
			}
		}

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
		gClientsList[new_id].connect = true;
		gClientsList[new_id].player.id = new_id;
		gClientsList[new_id].player.pos.x = 4;
		gClientsList[new_id].player.pos.y = 4;
		gClientsList[new_id].sock = new_client_sock;
		memset(&gClientsList[new_id].recv_overlap.origin_over, 0, sizeof(WSAOVERLAPPED));
		// WSARecv, WSASend�� �ϱ� ������ Overlap ����ü�� �ʱ�ȭ�� ���־�� �Ѵ�.
		// �׷��� ������ ������ error number 6�� "�ڵ��� �����ϴ�."��� ������ �����´�.
		ZeroMemory(gClientsList[new_id].recv_overlap.iocp_buf, MAX_BUFF_SIZE);
		gClientsList[new_id].recv_overlap.wsabuf.buf = reinterpret_cast<CHAR*>(gClientsList[new_id].recv_overlap.iocp_buf);
		gClientsList[new_id].recv_overlap.wsabuf.len = MAX_BUFF_SIZE;
		gClientsList[new_id].recv_overlap.operation = E_RECV;
		ZeroMemory(gClientsList[new_id].packet_buf, MAX_BUFF_SIZE);
		gClientsList[new_id].curr_packet_size = 0;
		gClientsList[new_id].prev_packet_size = 0;
		
		// ADD::����� ���ο� ���� Recv����.
		DWORD flags = 0;
		ret_val = WSARecv(new_client_sock, &gClientsList[new_id].recv_overlap.wsabuf, 1, NULL, &flags, &gClientsList[new_id].recv_overlap.origin_over, NULL);
		if (0 != ret_val)
		{
			int err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no)
				DisplayErrMsg("Error :: AcceptThreadFunc :: WSARecv", err_no);
		}

		DisplayDebugText("AcceptThreadFunc :: Accept Success :)");
	}
	return;
}

void WorkerThreadFunc(void)
{
	return;
}