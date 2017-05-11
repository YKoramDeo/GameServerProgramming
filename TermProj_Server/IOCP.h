#pragma once
#include "stdafx.h"
#include "protocol.h"

enum EVENTTYPE { E_RECV, E_SEND };

struct WSAOVERLAPPED_EX
{
	WSAOVERLAPPED	origin_over;				// 기존의 Overlapped 구조체
	WSABUF			wsabuf;						// 기존의 WSABUF
	EVENTTYPE		event_type;					// 현재 Send or Recv 연산을 진행하는 것인지 여부를 저장하는 변수
	unsigned char	iocp_buf[MAX_BUFF_SIZE];	// IOCP Send / Recv Buffer
};

struct Client
{
	bool				connect;					// 접속 여부를 묻는 변수
	Object				player;						// 게임상의 객체의 기본적인 정보를 저장하는 변수
	SOCKET				sock;						// accept를 통해서 생성된 Network 통신을 위한 client socket
	WSAOVERLAPPED_EX	recv_overlap;				// recv한 내용을 저장하기 위한 확장 overlapped 구조체
	unsigned char		packet_buf[MAX_PACKET_SIZE];	// recv되는 패킷이 조립되는 Buffer / Send 할 때는 사용되지 않으므로 확장 구조체에서 제외
	int					prev_packet_size;			// 이전에 받은 양을 저장하는 변수 / Send 할 때는 사용되지 않으므로 확장 구조체에서 제외
	int					curr_packet_size;			// 현재 받은 packet의 양
};

extern HANDLE ghIOCP;
extern SOCKET gServerSock;
extern Client gClientsList[MAX_USER];

void DisplayDebugText(const std::string);
void DisplayErrMsg(const char*, const int);

void InitializeServerData(void);
void StopServer(void);
void AcceptThreadFunc(void);
void WorkerThreadFunc(void);

void SendPacket(const int, const unsigned char*);
void SendAddObjectPacket(const int, const int);