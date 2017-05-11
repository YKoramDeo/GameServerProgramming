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
	// FormatMessage() : 오류코드에 대응하는 오류 메시지를 얻을 수 있다.

	// FORMAT_MESSAGE_ALLOCATE_BUFFER			: 오류 메시지를 저장할 공간을 함수가 알아서 할당한다는 의미
	// FORMAT_MESSAGE_FROM_SYSTEN				: OS로부터 오류 메시지를 가져온다는 의미

	// MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT)	: 사용자가 제어판에서 설정한 기본언어로 오류메시지 얻을 수 있음
	// lpMsgBuf 는 오류 메시지를 저장할 공간인데 함수가 알아서 할당, 
	// 오류 메시지 사용을 마치면 LocalFree() 함수를 이용해 할당한 메모리 반환 필수
	return;
}

void InitializeServerData(void)
{
	// 02. 기본적인 server를 동작하기 위한 초기화 작업을 진행한다.
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		DisplayDebugText("ERROR :: InitializeServerData :: [ WSAStartup ] Fail !!");
		exit(EXIT_FAILURE);
		// exit()	: 프로그램에서 호스트 환경에 제어를 리턴함. atexit() 함수에 등록된 모든 함수를 역순으로 호출.
		//			: 프로그램을 종료하기 전에 버퍼를 모두 삭제하고 열린 파일을 모두 닫음. 
		//			: EXIT_SUCCESS 또는 0은 정상종료, 그렇지 않으면 다른 상태 값이 리턴.
	}
	// WSAStartup()	: 윈속 초기화 함수로, 프로그램에서 사용할 윈속 버전을 요청함으로써 윈속 라이브러리(WS2_32.DLL)를 초기화 하는 역할.
	//				: return - 성공하면 0, 실패하면 오류코드

	// MAKEWORD(2,2)	: 프로그램이 요구하는 최상위 윈속 버전. 하위 8비트에 주 버전, 상위 8비트에 부 버전을 넣어서 전달
	//					: 만일 윈속 3.2 버전을 사용을 요청한다면 0x0203 또는 MAKEWORD(3,2)를 사용
	// wsadata			: WSADATA 구조체를 전달하면 이를 통해 윈도우 운영체제가 제공하는 윈속 구현에 관한 정보를 얻을 수 있음.
	//					: (응용 프로그램이 실제로 사용하게 될 윈속 버전, 시스템이 지원하는 윈속 최상위 버전 등), 하지만 실제로 이런 정보는 사용하지 않음.

	ghIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (NULL == ghIOCP)
	{
		DisplayDebugText("ERROR :: InitializeServerData :: [ CreateIoCompletionPort ] Fail !!");
		exit(EXIT_FAILURE);
	}
	// CreateIoCompletionPort()	: 1. 입출력 완료 포트를 생성.
	//							: 2. 소켓과 입출력 완료포트를 연결.
	//								소켓과 입출력 완료포트를 연결해두면 이 소켓에 대한 비동기 입출력 결과가 입출력 완료포트에 저장.

	gServerSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == gServerSock)
	{
		DisplayErrMsg("Error :: InitializeServerData :: Create Global Server Socket Fail !!", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	// WSASocket(1, 2, 3, 4, 5, 6) : socket을 생성하는 함수
	// 1. af			 : address family - AF_INET만 사용 (그 밖에 AF_NETBIOS, AF_IRDA, AF_INET6가 존재.)
	// 2. type			 : 소켓의 타입 - tcp를 위해 SOCKET_STREAM사용 (SOCK_DGRAM : udp)
	// 3. protocol		 : 사용할 프로토콜의 종류 - IPPROTO_TCP (IPPROTO_UDP)
	// 4. lpPtotocolInfo : 프로토콜 정보 - 보통 NULL
	// 5. g				 : 예약
	// 6. dgFlags		 : 소켓의 속성 - 보통 0 (또는 WSA_PROTOCOL_OVERLAPPED)

	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(MY_SERVER_PORT);
	ZeroMemory(&server_addr.sin_zero, 8);
	// sockaddr_in : 소켓 주소 구조체 socket address structures
	//			   : 네트워크 프로그램에서 필요한 주소 정보를 담고 있는 구조체.
	//			   : 특히, 여기서 사용된 sockaddr_in은 IPv4에 사용된 소켓 주소 구조체.
	// 1. short		sin_family	: 주소 체계를 의미하며 AF_INET(인터넷 주소 체계, IPv4) 값을 사용한다.
	// 2. struct in_addr sin_addr 
	//			: IP 주소를 의미하며, 각각 32비트 in_addr 구조체를 사용한다.
	//			: 응용프로그램에서는 대개 32비트 단위로 접근하므로 S_un.S_addr필드를 사용하고 매크로를 통해 재정의 된 s_addr을 사용하면 편하다.
	//			: INADDR_ANY	: 서버의 지역 IP주소를 설정하는데 있어 다음과 같이 설정하면 서버가 IP주소를 2개 이상 보유할 경우,
	//						    : (multihomed host 라고 부름), 클라이언트가 어느 IP주소로 접속하든 받아들일 수 있도록 설정.
	// 3. u_short	sin_port	: 포트 번호를 의미, 부호 없는 16비트 정수 값을 사용한다.
	// 4. char		sin_zero[8] : 0으로 설정하면 되는 여유 공간이다.

	int ret_val = 0;
	ret_val = ::bind(gServerSock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	if (SOCKET_ERROR == ret_val)
	{
		DisplayErrMsg("Error :: InitializeServerData :: Bind Fail !!", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	// bind() : 소켓의 지역 IP주소와 지역 포트 번호를 결정, 
	//		  : C++11의 bind라는 함수와 혼동 때문에 :: 범위확인 연산자와 같이 사용
	// listen_addr : 지역 IP와 지역 포트 번호로 초기화한 소켓 주소 구조체를 전달.
	// bind 함수의 2번째 인자는 항상 (SOCKADDR*)형을 반환해야 함.

	ret_val = listen(gServerSock, SOMAXCONN);
	if (SOCKET_ERROR == ret_val)
	{
		DisplayErrMsg("Error :: AcceptThreadFunc :: Listen Fail !!", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	// listen()  : 소켓의 TCP 포트 상태를 LISTENING상태로 바꾼다. 이는 클라이언트 접속을 받아들일 수 있는 상태가 됨을 의미
	// SOMAXCONN : 클라이언트의 접속 정보는 연결 큐에 저장되는데, backlog는 이 연결 큐의 길이를 나타냄.
	//			 : SOMAXCONN의 의미는 하부 프로토콜에서 지원 가능한 최댓값을 사용

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
	// WSACleanup() : 프로그램 종료 시 윈속 종료 함수
	//				: 윈속 사용을 중지함을 운영체제에 알리고, 관련 리소르스를 반환.

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
		// WSAAccept()	: 접속한 클라이언트와 통신할 수 있도록 새로운 소켓을 생성해서 리턴 또한 접속한 클라이언트의 주소정보도 알려줌
		//				: 서버입장에서는 원격 IP주소와 원격 포트번호, 클라이언트 입장에서는 지역 IP주소와 지역 포트번호

		int new_id = -1; // 접속한 클라이언트의 새 ID, 먼저 사용하지 않는 -1로 초기화
						 // ADD::접속한 클라이언트의 새로운 ID를 부여하는 구간
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

		// Accept 받은 이후에 등록을 해주어야 함.
		HANDLE result = CreateIoCompletionPort(reinterpret_cast<HANDLE>(new_client_sock), ghIOCP, new_id, 0);
		if (NULL == result)
		{
			DisplayErrMsg("Error :: AcceptThreadFunc :: CreateIoCompletionPort Fail !!", WSAGetLastError());
			closesocket(new_client_sock);
			continue;
		}
		// CreateIoCompletionPort() : 소켓과 입출력 완료포트를 연결.
		//							: 소켓과 입출력 완료포트를 연결해두면 이 소켓에 대한 비동기 입출력 결과가 입출력 완료포트에 저장.

		// ADD::연결된 클라이언트 Network 정보 초기화
		gClientsList[new_id].connect = true;
		gClientsList[new_id].player.id = new_id;
		gClientsList[new_id].player.pos.x = 0;
		gClientsList[new_id].player.pos.y = 0;
		gClientsList[new_id].sock = new_client_sock;
		memset(&gClientsList[new_id].recv_overlap.origin_over, 0, sizeof(WSAOVERLAPPED));
		// WSARecv, WSASend를 하기 이전에 Overlap 구조체를 초기화를 해주어야 한다.
		// 그렇지 않으면 가끔식 error number 6인 "핸들이 없습니다."라는 오류를 보내온다.
		ZeroMemory(gClientsList[new_id].recv_overlap.iocp_buf, MAX_BUFF_SIZE);
		gClientsList[new_id].recv_overlap.wsabuf.buf = reinterpret_cast<CHAR*>(gClientsList[new_id].recv_overlap.iocp_buf);
		gClientsList[new_id].recv_overlap.wsabuf.len = MAX_BUFF_SIZE;
		gClientsList[new_id].recv_overlap.event_type = E_RECV;
		ZeroMemory(gClientsList[new_id].packet_buf, MAX_BUFF_SIZE);
		gClientsList[new_id].curr_packet_size = 0;
		gClientsList[new_id].prev_packet_size = 0;
		
		SendLoginOkPacket(new_id);

		// ADD::연결된 새로운 소켓 Recv수행.
		DWORD recv_flag = 0;
		ret_val = WSARecv(new_client_sock, &gClientsList[new_id].recv_overlap.wsabuf, 1, NULL, &recv_flag, &gClientsList[new_id].recv_overlap.origin_over, NULL);
		if (0 != ret_val)
		{
			int err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no)
				DisplayErrMsg("Error :: AcceptThreadFunc :: WSARecv", err_no);
		}
		// WSARecv(1, 2, 3, 4, 5, 6) : socket으로 받은 데이터를 받는 동작을 처리하는 함수
		// 1. SOCKET s : 비동기 입출력을 할 대상 소켓.
		// 2. LPWSABUF lpBuffers
		//	  DWORD dwBufferCount : WSABUF 구조체 배열의 시작 주소와 배열의 원소 개수
		//		typedef struct __WSABUF {
		//			u_long	len;			// 길이
		//			char	*buf;			// 버퍼 시작 주소
		//		} WSABUF, *LPWSABUF;
		// 3. LPDWORD lpNumverOfBytesRecvd : 함수 호출이 성공하면 보내거나 받은 바이트 수를 저장.
		// 4. DWORD dwFlags : 옵션으로 MSG_* 형태의 상수를 전달할 수 있는데, 각각 send()와 recv() 함수의 마지막 인자와 같은 기능을 한다. 대부분 0 사용
		// 5. LPWSAOVERLAPPED lpOverlapped : WSAOVERLAPPED 구조체의 주소 값.
		//		WSAOVERLAPPED 구조체는 비동기 입출력을 위한 정보를 운영체제에 전달하거나, 운영체제가 비동기 입출력 결과를 응용 프로그램에 알려줄 때 사용
		//		WSAOVERLAPPED 구조체 중 처음 4개는 운영체제가 내부적으로 사용한다. 
		//		typedef struct _WSAOVERLAPPED {
		//			DWORD Internal;
		//			DWORD InternalHigh;
		//			DWORD Offset;
		//			DWORD OffsetHigh;
		//			WSAEVENT hEvent;
		//		} WSAOVERLAPPED, *LPWSAOVERLAPPED;
		//		마지막 변수인 hEvent는 이벤트 객체의 핸들 값으로 Overlapped모델(1)에서만 사용한다. 
		//		입출력 작업이 완료되면 hEvent가 가리키는 이벤트 객체는 신호 상태가 된다.
		// 6. LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine : 입출력 작업이 완료되면 운영체제가 자동으로 호출할 완료 루틴(콜백 함수)의 주소 값.

		//ADD::추가 작업 더 필요함
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
			// 각 플레이어에게 접속 종료 알림
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
					// 패킷을 완성 시킬 수 있는 상황이면 패킷을 어떠한 공간에 쌓아두고 저장해두어야 한다.
					// 그래서 패킷을 완성시키는 저장공간이 별도로 있어야 한다.
					// 데이터가 패킷단위로 오는 것이 아니기 때문에 패킷단위로 처리하고 남은 데이터는 별도의 공간에 저장해야 한다.
					// 다음에 온 데이터가 온전하지 못한채로 오게되면 별도의 공간에 집어넣고 하나의 패킷으로 마저 만들어 주어야 한다.
					memcpy(completed_packet_buf, gClientsList[key_id].packet_buf, prev_packet_size);
					memcpy(completed_packet_buf + prev_packet_size, iocp_buf_ptr, curr_packet_size - prev_packet_size);
					// + 하는 이유는 지난 번에 받은 데이터 이후에 저장을 해야 하기 때문에 그 시작 위치를 지정하기 위한 연산.
					ProcessPacket(static_cast<int>(key_id), completed_packet_buf);
					received_size_to_process -= (curr_packet_size - prev_packet_size);
					iocp_buf_ptr += (curr_packet_size - prev_packet_size);
					// 날라온 데이터를 이용하여 기존의 것과 더하여 하나의 packet을 처리하긴 했지만 그럼에도 불구하고 남아있는 데이터가 있을 수 있음.
					// 그 내용을 저장하기 위하여 남아있는 처리해야할 데이터와 buffer의 시작위치를 변경해줌
					curr_packet_size = 0;
					prev_packet_size = 0;
				}
				else {
					memcpy(gClientsList[key_id].packet_buf + prev_packet_size, iocp_buf_ptr, received_size_to_process);
					prev_packet_size += received_size_to_process;
					iocp_buf_ptr += received_size_to_process;
					received_size_to_process = 0;
					// packet으로 만들지 못한 데이터를 남은 buffer에 저장함.
				}
			}
			gClientsList[key_id].curr_packet_size = curr_packet_size;
			gClientsList[key_id].prev_packet_size = prev_packet_size;
			// 크기에 대한 정보를 client정보를 담는 내용에 저장.
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
			// 다음의 동작은 운영체제의 send 데이터 버퍼가 비워지지 않고 메모리가 가득 차서 부부적으로만 보내진 경우이다.
			// 사실상 이러한 동작까지 왔다는 것은 운영체제가 이미 한계라는 의미이다. 그러므로 예외처리하고 끊어버려야 한다.
			// 이러한 일이 벌어지지 않으려면 send 데이터의 양을 조절해야 한다.
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