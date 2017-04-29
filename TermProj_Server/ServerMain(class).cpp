#include "stdafx.h"
#include "protocol.h"

struct OverlapEx
{
	WSAOVERLAPPED	originalOverlap;			// 기존의 Overlapped 구조체
	WSABUF			wsaBuf;						// 기존의 WSABUF
	int				operation;					// 현재 Send or Recv 연산을 진행하는 것인지 여부를 저장하는 변수
	unsigned char	iocpBuf[MAX_BUFF_SIZE];		// IOCP Send / Recv Buffer
	int				packetSize;					// 받은 packet의 양을 저장하는 변수.
};

struct Client
{
	volatile bool	isConnect;
	SOCKET			socket;
	Player			player;
	OverlapEx		recvOverlap;
	unsigned char	packetBuf[MAX_BUFF_SIZE];	// Recv되는 패킷이 조립되는 Buffer / Send 에서는 사용하지 않으므로 확장 구조체에 포함되지 않음.
	int				previousDataSize;			// 이전의 받은 양을 저장하는 변수 / Send 에서는 사용하지 않으므로 확장 구조체에 포함되지 않음.
};

bool gShutdown = false;
HANDLE ghIOCP;
Client gClientsList[MAX_USER];

void DisplayErrMsg(char*, int);
void InitializeServer(void);
void StopServer(void);
void WorkerThreadFunc(void);
void AcceptThreadFunc(void);
void SendPacket(const int, const unsigned char*);
void ProcessPacket(int, unsigned char*);

void SendLoginPacket(int);

int main(int argc, char *argv[])
{
	std::vector<std::thread*> workerThreads;
	std::thread acceptThread;

	InitializeServer();
	acceptThread = std::thread(AcceptThreadFunc);
	for (int i = 0; i < NUM_THREADS; ++i)
		workerThreads.push_back(new std::thread(WorkerThreadFunc));

	while (!gShutdown) Sleep(1000);
	//  다음과 같이 Main Thread를 1초마다 잠들게 하여
	// Processor가 Main Thread를 점유하는 일을 최대한 저해하도록 한다.

	acceptThread.join();
	for (auto t : workerThreads)
	{
		t->join();
		delete t;
	}

	StopServer();
	return 0;
}

void DisplayErrMsg(char* msg, int errNo)
{
	WCHAR *lpMsgBuf;
	// FormatMessage() : 오류코드에 대응하는 오류 메시지를 얻을 수 있다.
	// msdn - The function finds the message definition in a message table resource vased on a message identifier and a language identifier.
	//		The funtion copies the formatted message text to an output buffer, processing ant embadded insert sequences if requested.

	// FORMAT_MESSAGE_ALLOCATE_BUFFER			: 오류 메시지를 저장할 공간을 함수가 알아서 할당한다는 의미
	// FORMAT_MESSAGE_FROM_SYSTEN				: OS로부터 오류 메시지를 가져온다는 의미

	// MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT)	: 사용자가 제어판에서 설정한 기본언어로 오류메시지 얻을 수 있음
	// lpMsgBuf 는 오류 메시지를 저장할 공간인데 함수가 알아서 할당, 
	// 오류 메시지 사용을 마치면 LocalFree() 함수를 이용해 할당한 메모리 반환 필수
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errNo, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("%s", msg); wprintf(L" %d Error :: %s\n", errNo, lpMsgBuf);
	LocalFree(lpMsgBuf);
	return;
}

void DisplayDebugText(std::string str)
{
	std::cout << str << std::endl;
	return;
}


void InitializeServer(void)
{
	WSADATA wsadata;
	_wsetlocale(LC_ALL, L"korean");

	// Initialize Winsock
	// WSAStartup() :  프로그램에서 사용할 췬속 버전을 요청함으로써 윈속 라이브러리(WS2_32.DLL)를 초기화 하는 역할
	// msdn - The WSAStartup function initiates use of Winsock DLL bt a process

	// MAKEWORD(2,2)	: 프로그램이 요구하는 최상위 윈속 버전, 하위 8비트에 주 버전, 상위 8비트에 부 버전을 넣어서 전달
	//					: 만일 윈속 3.2 버전을 사용을 요청한다면 0x0203 또는 MAKEWORD(3,2)를 사용
	// wsadata			: 윈도우 운영체제가 제공하는 윈속 구현에 관한 정보(프로그램이 실제로 사용하게 될 윈속 버전,
	//					: 시스템이 지원하는 윈속 최상위 버전 등)를 얻을 수 있음. 하지만 실제로 이런 정보 사용 X
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		std::cout << "InitializeServer :: WSAStartup Fail !!" << std::endl;
		exit(1);
	}

	// Create IOCP kernel object
	// if last argument is 0, Use kernel object by core count
	// CreateIoCompletionPort() : 입출력 완료 포트를 새로 생성.
	//							: 입출력 롼료 포트란 비동기 입출력 결과와 이 결과를 처리할 스레드에 관한 정보를 담고 있는 구조
	// msdn - Creates an input/output (I/O) completion port and associates it with a specified file handle, 
	//		or reates an I/O completion port that is not yet associated with a file handle, allowing association at a later time.
	ghIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (NULL == ghIOCP)
	{
		std::cout << "InitializeServer :: CreateIoCompletionPort Fail !!" << std::endl;
		exit(1);
	}

	return;
}

void StopServer(void)
{
	for (auto client : gClientsList)
	{
		if (client.isConnect)
			closesocket(client.socket);
	}

	WSACleanup();
	return;
}

void WorkerThreadFunc(void)
{
	DWORD ioSize, key;
	OverlapEx *overlap;
	BOOL result;

	while (true)
	{
		result = GetQueuedCompletionStatus(ghIOCP, &ioSize, reinterpret_cast<PULONG_PTR>(&key), reinterpret_cast<LPOVERLAPPED*>(&overlap), INFINITE);
		if (false == result)
		{
			// Error 처리
			DisplayErrMsg("WorkerThreadFunc :: GetQueuedCompletionStatus Fail !!", GetLastError());
			continue;
		}
		if (0 == ioSize)
		{
			closesocket(gClientsList[key].socket);

			// 각 플레이어에게 플레이어 접속 종료 알림
			// . . .

			gClientsList[key].isConnect = false;
			continue;
		}
		if (OP_RECV == overlap->operation)
		{
			unsigned char *buf_ptr = overlap->iocpBuf;
			int remained = ioSize;
			while (0 < remained)
			{
				if (0 == gClientsList[key].recvOverlap.packetSize)
					gClientsList[key].recvOverlap.packetSize = buf_ptr[0];
				int required = gClientsList[key].recvOverlap.packetSize - gClientsList[key].previousDataSize;
				// 패킷을 완성 시킬 수 있는가? 없는가?
				if (remained >= required)
				{
					// 완성을 시킬 수 있는 상황이면
					// 패킷을 어떠한 공간에다가 고이 모셔와야 한다.
					// 그래서 패킷을 완성시키는 저장공간이 별도로 있어야한다. 
					// 데이터가 패킷단위로 오는 것이 아니기 때문에 패킷단위로 처리하고 남은데이터는 그 별도의 공간에 저장해야
					// 다음의 온 데이터가 온전하지 못한 채로 오게되면 별도의 공간에 집어넣고 하나의 패킷으로 마저 만들어 주어야 한다.
					memcpy(gClientsList[key].packetBuf + gClientsList[key].previousDataSize, buf_ptr, required);
					// +하는 이유는 지난번에 받은 데이터 이후에 저장을 해야하기 때문에 그 시작위치로 옮겨줌.
					ProcessPacket(key, gClientsList[key].packetBuf);
					// Packet 처리
					remained -= required;
					// 날아 있는 것은 필요한 것을 제외하고
					buf_ptr += required;
					// ???
					gClientsList[key].recvOverlap.packetSize = 0;
				}
				else
				{
					// 패킷을 완성 시킬 수 없는 크기이다.
					memcpy(gClientsList[key].packetBuf + gClientsList[key].previousDataSize, buf_ptr, remained);
					// buf_ptr의 모든 것을 packet에 저장한다.
					gClientsList[key].previousDataSize += remained;
					// 이전의 데이터가 남아있는 사이즈만큼 늘어났다.
					remained = 0;
					// 현재 recv의 남아있는 size는 이미 저장을 해두었으므로 초기화
				}
			}
			DWORD flags = 0;
			WSARecv(gClientsList[key].socket,
				&gClientsList[key].recvOverlap.wsaBuf,
				1, NULL, &flags,
				reinterpret_cast<LPWSAOVERLAPPED>(&gClientsList[key].recvOverlap),
				NULL);
		}
		else if (OP_SEND == overlap->operation)
		{
			// ioSize하고 실제 보낸 크기 비교 후 소켓 접속 끊기
			delete overlap;
		}
		else
		{
			std::cout << "WorkerThreadFunc :: Unknown Event on worker_thread" << std::endl;
			continue;
		}
	}
	return;
}

void AcceptThreadFunc(void)
{
	int retval = 0;
	struct sockaddr_in listenAddr;

	SOCKET acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == acceptSocket)
	{
		DisplayErrMsg("AcceptThreadFunc  :: acceptSocket Fail !!", WSAGetLastError());
		return;
	}

	// bind() : 소켓의 지역 IP주소와 지역 포트 번호를 결정, C++11의 bind라는 함수와 혼동 때문에 :: 범위확인 연산자와 같이 사용.
	// msdn - The bind functio associates a local address with a socket

	// listenAddr	: 소켓 주소 구조체를 지역 IP주소와 지역 포트 번호로 초기화하여 전달한다.
	// AF_INET		: 인터넷 주초 체계(IPv4)를 사용한다는 의미
	// INADDR_ANY	: 서버의 지역 IP주소를 설정하는데 있어 다음과 같이 설정을 하면 서버가 IP주소를 2 개 이상 보유한 경우 
	//				(multihomed host라고 부름), 클라이언트가 어느 IP 주소로 접속하든 받아들일 수 있음
	// bind 함수의 2번째 인자는 항상 (SOCKADDR*)형으로 반환해야 함
	ZeroMemory(&listenAddr, sizeof(listenAddr));
	listenAddr.sin_family = AF_INET;
	listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	listenAddr.sin_port = htons(MY_SERVER_PORT);
	ZeroMemory(&listenAddr.sin_zero, 8);
	retval = ::bind(acceptSocket, reinterpret_cast<sockaddr*>(&listenAddr), sizeof(listenAddr));
	if (SOCKET_ERROR == retval)
	{
		DisplayErrMsg("AcceptThreadFunc :: bind Fail !!", WSAGetLastError());
		exit(1);
	}
	// listen() : 소켓의 TCP 포트 상태를 LISTENING으로 바꾼다. 이는 클라이언트 접속을 받아들일 수 있는 상태가 됨을 의미
	// msdn - The listen function places a socket in a state in which it is listening for an incomming connection.

	// SOMAXCONN : 클라이언트의 접속 정보는 연결 큐에 저장되는데, backlog는 이 연결 큐의 길이를 나타냄.
	//			SOMAXCONN의 의미는 하부 프로토콜에서 지원 가능한 최댓값을 사용
	listen(acceptSocket, SOMAXCONN);
	if (SOCKET_ERROR == retval)
	{
		DisplayErrMsg("AcceptThreadFunc :: listen Fail !!", WSAGetLastError());
		exit(1);
	}

	while (true)
	{
		struct sockaddr_in clientAddr;
		int addrSize = sizeof(clientAddr);

		int newID = -1;
		// 접속한 클라이언트의 새 ID

		// accept() : 접속한 클라이언트와 통신할 수 있도록 새로운 소켓을 생성해서 리턴 또한 접속한 클라이언트의 주소정보도 알려줌
		//			: 서버입장에서는 원격 IP주소와 원격 포트번호, 클라이언트 입장에서는 지역 IP 주소와 지역 포트 번호
		// WSAAccept() : ?
		// msdn - The WSAAccept function conditionally accepts a connection based on the return value of a condition,
		//		provides quality of service flow specifications, and allows the transfer of connection data.
		SOCKET newClientSocket = WSAAccept(acceptSocket, reinterpret_cast<sockaddr*>(&clientAddr), &addrSize, NULL, NULL);
		if (INVALID_SOCKET == newClientSocket)
		{
			DisplayErrMsg("AcceptThreadFunc :: WSAAccept Fail !!", WSAGetLastError());
			break;
		}

		// 접속한 클라이언트의 새로운 ID 부여하는 구간 
		for (auto i = 0; i < MAX_USER; ++i)
		{
			if (gClientsList[i].isConnect == false)
			{
				newID = i;
				break;
			}
		}

		if (-1 == newID)
		{
			std::cout << "AcceptThread :: Maximum User Number fail !!" << std::endl;
			closesocket(newClientSocket);
			continue;
		}

		// Accept를 받은 이후에 등록을 해주어야 함
		HANDLE result = CreateIoCompletionPort(reinterpret_cast<HANDLE>(newClientSocket), ghIOCP, newID, 0);
		if (NULL == result)
		{
			DisplayErrMsg("AcceptThreadFunc :: CreateIoCompletionPort Fail !!", WSAGetLastError());
			closesocket(newClientSocket);
			continue;
		}

		gClientsList[newID].player.id = newID;
		gClientsList[newID].isConnect = true;
		gClientsList[newID].socket = newClientSocket;
		memset(&gClientsList[newID].recvOverlap.originalOverlap, 0, sizeof(WSAOVERLAPPED));
		// WSARecv, WSASend를 하기 이전에는 원래의 Overlap 구조체를 초기화를 해주어야 한다.
		// 그렇지 않으면 가끔식 err_number 6인 "핸들이 없습니다."라는 오류를 보내온다.
		ZeroMemory(gClientsList[newID].recvOverlap.iocpBuf, MAX_BUFF_SIZE);
		gClientsList[newID].recvOverlap.wsaBuf.buf = reinterpret_cast<CHAR*>(gClientsList[newID].recvOverlap.iocpBuf);
		gClientsList[newID].recvOverlap.wsaBuf.len = MAX_BUFF_SIZE;
		gClientsList[newID].recvOverlap.operation = OP_RECV;
		gClientsList[newID].recvOverlap.packetSize = 0;
		ZeroMemory(gClientsList[newID].packetBuf, MAX_BUFF_SIZE);
		gClientsList[newID].previousDataSize = 0;
		// newClient.player 내용 채우기
		gClientsList[newID].player.pos.x = 4;
		gClientsList[newID].player.pos.y = 4;

		// 새로운 소켓 Recv 수행
		DWORD flags = 0;
		// WSARecv() : 5번째, 6번째 두 인자를 NULL 값으로 사용하면 recv() 함수처럼 동기 함수로 동작
		retval = WSARecv(newClientSocket, &gClientsList[newID].recvOverlap.wsaBuf, 1, NULL, &flags, &gClientsList[newID].recvOverlap.originalOverlap, NULL);
		if (0 != retval)
		{
			int errNo = WSAGetLastError();
			if (WSA_IO_PENDING != errNo)
			{
				DisplayErrMsg("AcceptThreadFunc :: WSARecv", errNo);
			}
		}

		// Output
		std::cout << "AcceptThreadFunc :: Accept Success !!" << std::endl;
	}

	return;
}

void SendPacket(const int index, const unsigned char* packet)
{
	int retval = 0;
	OverlapEx *sendOverlap = new OverlapEx;
	memset(sendOverlap, 0, sizeof(OverlapEx));
	sendOverlap->operation = OP_SEND;
	memset(&sendOverlap->originalOverlap, 0, sizeof(WSAOVERLAPPED));
	ZeroMemory(sendOverlap->iocpBuf, MAX_BUFF_SIZE);
	sendOverlap->wsaBuf.buf = reinterpret_cast<CHAR*>(sendOverlap->iocpBuf);
	sendOverlap->wsaBuf.len = packet[0];
	memcpy(sendOverlap->iocpBuf, packet, packet[0]);
	retval = WSASend(gClientsList[index].socket, &sendOverlap->wsaBuf, 1, NULL, 0, &sendOverlap->originalOverlap, NULL);
	if (0 != retval)
	{
		int errNo = WSAGetLastError();
		if (WSA_IO_PENDING != errNo)
			DisplayErrMsg("SendPacket :: WSASend", errNo);
	}
	return;
}

void ProcessPacket(int key, unsigned char *packet)
{
	unsigned char packetType = packet[1];

	switch (packetType)
	{
	default:
		std::cout << "Unknown Packet Type Detected" << std::endl;
		return;
	}

	return;
}