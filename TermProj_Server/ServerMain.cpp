#include "stdafx.h"
#include "protocol.h"

struct WSAOVERLAPPED_EX
{
	WSAOVERLAPPED	origin_over;				// 기존의 Overlapped 구조체
	WSABUF			wsabuf;						// 기존의 WSABUF
	int				operation;					// 현재 Send or Recv 연산을 진행하는 것인지 여부를 저장하는 변수
	unsigned char	iocp_buf[MAX_BUFF_SIZE];	// IOCP Send / Recv Buffer
};

struct Client
{
	bool				connect;					// 접속 여부를 묻는 변수
	Object				player;						// 게임상의 객체의 기본적인 정보를 저장하는 변수
	SOCKET				sock;						// accept를 통해서 생성된 Network 통신을 위한 client socket
	WSAOVERLAPPED_EX	recv_overlap;				// recv한 내용을 저장하기 위한 확장 overlapped 구조체
	unsigned char		packet_buf[MAX_BUFF_SIZE];	// recv되는 패킷이 조립되는 Buffer / Send 할 때는 사용되지 않으므로 확장 구조체에서 제외
	int					prev_packet_size;			// 이전에 받은 양을 저장하는 변수 / Send 할 때는 사용되지 않으므로 확장 구조체에서 제외
	int					curr_packet_size;			// 현재 받은 packet의 양
};

HANDLE ghIOCP;

void DisplayDebugText(std::string);
void DisplayErrMsg(char*, int);

void InitializeServerData(void);
void StopServer(void);
void AcceptThreadFunc(void);

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
	
	DisplayDebugText("Initialize Server Data Success!");
	return;
}

void StopServer(void)
{
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
	SOCKET accept_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == accept_sock)
	{
		DisplayErrMsg("Error :: AcceptThreadFunc :: Create Accept Socket Fail !!", WSAGetLastError());
		return;
	}
	// WSASocket(1, 2, 3, 4, 5, 6) : socket을 생성하는 함수
	// 1. af			 : address family - AF_INET만 사용 (그 밖에 AF_NETBIOS, AF_IRDA, AF_INET6가 존재.)
	// 2. type			 : 소켓의 타입 - tcp를 위해 SOCKET_STREAM사용 (SOCK_DGRAM : udp)
	// 3. protocol		 : 사용할 프로토콜의 종류 - IPPROTO_TCP (IPPROTO_UDP)
	// 4. lpPtotocolInfo : 프로토콜 정보 - 보통 NULL
	// 5. g				 : 예약
	// 6. dgFlags		 : 소켓의 속성 - 보통 0 (또는 WSA_PROTOCOL_OVERLAPPED)

	struct sockaddr_in listen_addr;
	ZeroMemory(&listen_addr, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(MY_SERVER_PORT);
	ZeroMemory(&listen_addr.sin_zero, 8);
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
	ret_val = ::bind(accept_sock, reinterpret_cast<sockaddr*>(&listen_addr), sizeof(listen_addr));
	if (SOCKET_ERROR == ret_val)
	{
		DisplayErrMsg("Error :: AcceptThreadFunc :: Bind Fail !!", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	// bind() : 소켓의 지역 IP주소와 지역 포트 번호를 결정, 
	//		  : C++11의 bind라는 함수와 혼동 때문에 :: 범위확인 연산자와 같이 사용
	// listen_addr : 지역 IP와 지역 포트 번호로 초기화한 소켓 주소 구조체를 전달.
	// bind 함수의 2번째 인자는 항상 (SOCKADDR*)형을 반환해야 함.

	ret_val = listen(accept_sock, SOMAXCONN);
	if (SOCKET_ERROR == ret_val)
	{
		DisplayErrMsg("Error :: AcceptThreadFunc :: Listen Fail !!",WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	// listen()  : 소켓의 TCP 포트 상태를 LISTENING상태로 바꾼다. 이는 클라이언트 접속을 받아들일 수 있는 상태가 됨을 의미
	// SOMAXCONN : 클라이언트의 접속 정보는 연결 큐에 저장되는데, backlog는 이 연결 큐의 길이를 나타냄.
	//			 : SOMAXCONN의 의미는 하부 프로토콜에서 지원 가능한 최댓값을 사용

	while (true)
	{
		int new_id = -1; // 접속한 클라이언트의 새 ID, 먼저 사용하지 않는 -1로 초기화
		struct sockaddr_in client_addr;
		int addr_size = sizeof(client_addr);

		SOCKET new_client_sock = WSAAccept(accept_sock, reinterpret_cast<SOCKADDR*>(&client_addr), &addr_size, NULL, NULL);
		if (INVALID_SOCKET == new_client_sock)
		{
			DisplayErrMsg("Error :: AcceptThreadFunc :: WSAAccept Fail !!", WSAGetLastError());
			break;
		}
		// WSAAccept()	: 접속한 클라이언트와 통신할 수 있도록 새로운 소켓을 생성해서 리턴 또한 접속한 클라이언트의 주소정보도 알려줌
		//				: 서버입장에서는 원격 IP주소와 원격 포트번호, 클라이언트 입장에서는 지역 IP주소와 지역 포트번호
		
		// ADD::접속한 클라이언트의 새로운 ID를 부여하는 구간
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
		// ADD::연결된 새로운 소켓 Recv수행.
	}
	return;
}

void WorkerThreadFunc(void)
{
	return;
}