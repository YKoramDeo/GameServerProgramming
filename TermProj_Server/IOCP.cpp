#include "IOCP.h"
#include "ProcessRoutine.h"

HANDLE ghIOCP;
SOCKET gServerSock;
Client gClientsList[MAX_USER];

void DisplayDebugText(const std::string msg)
{
	std::cout << msg << std::endl;
	return;
}

void DisplayErrMsg(const char* str, const int err_no)
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
		gClientsList[new_id].player.pos.x = 0;
		gClientsList[new_id].player.pos.y = 0;
		gClientsList[new_id].sock = new_client_sock;
		memset(&gClientsList[new_id].recv_overlap.origin_over, 0, sizeof(WSAOVERLAPPED));
		// WSARecv, WSASend�� �ϱ� ������ Overlap ����ü�� �ʱ�ȭ�� ���־�� �Ѵ�.
		// �׷��� ������ ������ error number 6�� "�ڵ��� �����ϴ�."��� ������ �����´�.
		ZeroMemory(gClientsList[new_id].recv_overlap.iocp_buf, MAX_BUFF_SIZE);
		gClientsList[new_id].recv_overlap.wsabuf.buf = reinterpret_cast<CHAR*>(gClientsList[new_id].recv_overlap.iocp_buf);
		gClientsList[new_id].recv_overlap.wsabuf.len = MAX_BUFF_SIZE;
		gClientsList[new_id].recv_overlap.event_type = E_RECV;
		ZeroMemory(gClientsList[new_id].packet_buf, MAX_BUFF_SIZE);
		gClientsList[new_id].curr_packet_size = 0;
		gClientsList[new_id].prev_packet_size = 0;
		
		SendLoginOkPacket(new_id);

		// ADD::����� ���ο� ���� Recv����.
		DWORD recv_flag = 0;
		ret_val = WSARecv(new_client_sock, &gClientsList[new_id].recv_overlap.wsabuf, 1, NULL, &recv_flag, &gClientsList[new_id].recv_overlap.origin_over, NULL);
		if (0 != ret_val)
		{
			int err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no)
				DisplayErrMsg("Error :: AcceptThreadFunc :: WSARecv", err_no);
		}
		// WSARecv(1, 2, 3, 4, 5, 6) : socket���� ���� �����͸� �޴� ������ ó���ϴ� �Լ�
		// 1. SOCKET s : �񵿱� ������� �� ��� ����.
		// 2. LPWSABUF lpBuffers
		//	  DWORD dwBufferCount : WSABUF ����ü �迭�� ���� �ּҿ� �迭�� ���� ����
		//		typedef struct __WSABUF {
		//			u_long	len;			// ����
		//			char	*buf;			// ���� ���� �ּ�
		//		} WSABUF, *LPWSABUF;
		// 3. LPDWORD lpNumverOfBytesRecvd : �Լ� ȣ���� �����ϸ� �����ų� ���� ����Ʈ ���� ����.
		// 4. DWORD dwFlags : �ɼ����� MSG_* ������ ����� ������ �� �ִµ�, ���� send()�� recv() �Լ��� ������ ���ڿ� ���� ����� �Ѵ�. ��κ� 0 ���
		// 5. LPWSAOVERLAPPED lpOverlapped : WSAOVERLAPPED ����ü�� �ּ� ��.
		//		WSAOVERLAPPED ����ü�� �񵿱� ������� ���� ������ �ü���� �����ϰų�, �ü���� �񵿱� ����� ����� ���� ���α׷��� �˷��� �� ���
		//		WSAOVERLAPPED ����ü �� ó�� 4���� �ü���� ���������� ����Ѵ�. 
		//		typedef struct _WSAOVERLAPPED {
		//			DWORD Internal;
		//			DWORD InternalHigh;
		//			DWORD Offset;
		//			DWORD OffsetHigh;
		//			WSAEVENT hEvent;
		//		} WSAOVERLAPPED, *LPWSAOVERLAPPED;
		//		������ ������ hEvent�� �̺�Ʈ ��ü�� �ڵ� ������ Overlapped��(1)������ ����Ѵ�. 
		//		����� �۾��� �Ϸ�Ǹ� hEvent�� ����Ű�� �̺�Ʈ ��ü�� ��ȣ ���°� �ȴ�.
		// 6. LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine : ����� �۾��� �Ϸ�Ǹ� �ü���� �ڵ����� ȣ���� �Ϸ� ��ƾ(�ݹ� �Լ�)�� �ּ� ��.

		//ADD::�߰� �۾� �� �ʿ���
		for (int i = 0; i < MAX_USER; ++i) {
			if (i == new_id) continue;
			if (gClientsList[i].connect) {
				SendAddObjectPacket(i, new_id);
				SendAddObjectPacket(new_id, i);
			}
		}
		DisplayDebugText("AcceptThreadFunc :: Accept Success :)");
	}
	return;
}

void WorkerThreadFunc(void)
{
	DWORD io_size, key_id;
	WSAOVERLAPPED_EX *overlap;
	bool result = false;
	int ret_val = 0;

	while (true)
	{
		result = GetQueuedCompletionStatus(ghIOCP, &io_size, reinterpret_cast<PULONG_PTR>(&key_id), reinterpret_cast<LPOVERLAPPED*>(&overlap), INFINITE);
		if (!result)
		{
			int err_no = WSAGetLastError();
			if (64 == err_no) DisconnectClient(key_id);
			else DisplayErrMsg("Error :: WorkerThreadFunc :: GetQueuedCompletionStatus Fail !!", WSAGetLastError());
			continue;
		}
		if (0 == io_size)
		{
			// �� �÷��̾�� ���� ���� �˸�
			DisconnectClient(key_id);
			continue;
		}
		if (E_RECV == overlap->event_type)
		{
			unsigned char *iocp_buf_ptr = overlap->iocp_buf;
			unsigned char completed_packet_buf[MAX_PACKET_SIZE];
			int received_size_to_process = io_size;
			int curr_packet_size = gClientsList[key_id].curr_packet_size;
			int prev_packet_size = gClientsList[key_id].prev_packet_size;
			while (0 < received_size_to_process)
			{
				if (0 == curr_packet_size) curr_packet_size = iocp_buf_ptr[0];
				if (curr_packet_size <= received_size_to_process + prev_packet_size)
				{
					// ��Ŷ�� �ϼ� ��ų �� �ִ� ��Ȳ�̸� ��Ŷ�� ��� ������ �׾Ƶΰ� �����صξ�� �Ѵ�.
					// �׷��� ��Ŷ�� �ϼ���Ű�� ��������� ������ �־�� �Ѵ�.
					// �����Ͱ� ��Ŷ������ ���� ���� �ƴϱ� ������ ��Ŷ������ ó���ϰ� ���� �����ʹ� ������ ������ �����ؾ� �Ѵ�.
					// ������ �� �����Ͱ� �������� ����ä�� ���ԵǸ� ������ ������ ����ְ� �ϳ��� ��Ŷ���� ���� ����� �־�� �Ѵ�.
					memcpy(completed_packet_buf, gClientsList[key_id].packet_buf, prev_packet_size);
					memcpy(completed_packet_buf + prev_packet_size, iocp_buf_ptr, curr_packet_size - prev_packet_size);
					// + �ϴ� ������ ���� ���� ���� ������ ���Ŀ� ������ �ؾ� �ϱ� ������ �� ���� ��ġ�� �����ϱ� ���� ����.
					ProcessPacket(static_cast<int>(key_id), completed_packet_buf);
					received_size_to_process -= (curr_packet_size - prev_packet_size);
					iocp_buf_ptr += (curr_packet_size - prev_packet_size);
					// ����� �����͸� �̿��Ͽ� ������ �Ͱ� ���Ͽ� �ϳ��� packet�� ó���ϱ� ������ �׷����� �ұ��ϰ� �����ִ� �����Ͱ� ���� �� ����.
					// �� ������ �����ϱ� ���Ͽ� �����ִ� ó���ؾ��� �����Ϳ� buffer�� ������ġ�� ��������
					curr_packet_size = 0;
					prev_packet_size = 0;
				}
				else {
					memcpy(gClientsList[key_id].packet_buf + prev_packet_size, iocp_buf_ptr, received_size_to_process);
					prev_packet_size += received_size_to_process;
					iocp_buf_ptr += received_size_to_process;
					received_size_to_process = 0;
					// packet���� ������ ���� �����͸� ���� buffer�� ������.
				}
			}
			gClientsList[key_id].curr_packet_size = curr_packet_size;
			gClientsList[key_id].prev_packet_size = prev_packet_size;
			// ũ�⿡ ���� ������ client������ ��� ���뿡 ����.
			DWORD recv_flag = 0;
			ret_val = WSARecv(gClientsList[key_id].sock, &gClientsList[key_id].recv_overlap.wsabuf,
				1, NULL, &recv_flag, &gClientsList[key_id].recv_overlap.origin_over,
				NULL);
			if (0 != ret_val) {
				int err_no = WSAGetLastError();
				if (WSA_IO_PENDING != err_no)
					DisplayErrMsg("Error :: WorkerThreadFunc :: WSARecv Fail !!", err_no);
			}
		}
		else if (E_SEND == overlap->event_type)
		{
			std::string text = "Send Complete to Client : " + std::to_string(static_cast<int>(key_id));
			//DisplayDebugText(text);
			if (io_size != overlap->iocp_buf[0])
			{
				DisplayDebugText("Error :: WorkerThreadFunc :: Incomplete Packet Send !!");
				exit(EXIT_FAILURE);
			}
			// ������ ������ �ü���� send ������ ���۰� ������� �ʰ� �޸𸮰� ���� ���� �κ������θ� ������ ����̴�.
			// ��ǻ� �̷��� ���۱��� �Դٴ� ���� �ü���� �̹� �Ѱ��� �ǹ��̴�. �׷��Ƿ� ����ó���ϰ� ��������� �Ѵ�.
			// �̷��� ���� �������� �������� send �������� ���� �����ؾ� �Ѵ�.
			delete overlap;
		}
		else
		{
			DisplayDebugText("WorkerThreadFunc :: Unknown GQCS Event Type!");
			exit(EXIT_FAILURE);
		}
	}
	return;
}

void DisconnectClient(const int key_id)
{
	closesocket(gClientsList[key_id].sock);
	gClientsList[key_id].connect = false;

	SC_REMOVE_OBJECT_PACKET packet;
	packet.id = key_id;
	packet.size = sizeof(packet);
	packet.type = PacketType::SC_REMOVE_OBJECT;
	
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (i == key_id) continue;;
		if(gClientsList[i].connect)
			SendPacket(i, reinterpret_cast<unsigned char *>(&packet));
	}
	return;
}

void SendPacket(const int key_id, const unsigned char* packet)
{
	int ret_val = 0;
	WSAOVERLAPPED_EX *send_overlap = new WSAOVERLAPPED_EX;
	memset(send_overlap, 0, sizeof(WSAOVERLAPPED_EX));
	send_overlap->event_type = E_SEND;
	memset(&send_overlap->origin_over, 0, sizeof(WSAOVERLAPPED));
	ZeroMemory(send_overlap->iocp_buf, MAX_BUFF_SIZE);
	send_overlap->wsabuf.buf = reinterpret_cast<CHAR*>(send_overlap->iocp_buf);
	send_overlap->wsabuf.len = packet[0];
	memcpy(send_overlap->iocp_buf, packet, packet[0]);
	ret_val = WSASend(gClientsList[key_id].sock, &send_overlap->wsabuf, 1, NULL, 0, &send_overlap->origin_over, NULL);
	if (0 != ret_val) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			DisplayErrMsg("Error :: SendPacket :: WSASend Fail !!", err_no);
	}
	return;
}

void SendAddObjectPacket(const int target_client, const int new_client)
{
	SC_ADD_OBJECT_PACKET packet;
	packet.id = new_client;
	packet.size = sizeof(packet);
	packet.type = PacketType::SC_ADD_OBJECT;
	packet.x_pos = gClientsList[new_client].player.pos.x;
	packet.y_pos = gClientsList[new_client].player.pos.y;
	std::string text = "Send Add " + std::to_string(new_client) + " Object Packet to." + std::to_string(target_client);
	DisplayDebugText(text);
	SendPacket(target_client, reinterpret_cast<unsigned char *>(&packet));
	return;
}

void SendLoginOkPacket(const int new_client)
{
	SC_LOGIN_OK_PACKET packet;
	packet.id = new_client;
	packet.size = sizeof(packet);
	packet.type = PacketType::SC_LOGIN_OK;
	packet.x_pos = gClientsList[new_client].player.pos.x;
	packet.y_pos = gClientsList[new_client].player.pos.y;
	SendPacket(new_client, reinterpret_cast<unsigned char *>(&packet));
	return;
}