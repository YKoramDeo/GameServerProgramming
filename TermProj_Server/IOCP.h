#pragma once
#include "stdafx.h"
#include "protocol.h"

enum EVENTTYPE { E_RECV, E_SEND };

struct WSAOVERLAPPED_EX
{
	WSAOVERLAPPED	origin_over;				// ������ Overlapped ����ü
	WSABUF			wsabuf;						// ������ WSABUF
	EVENTTYPE		event_type;					// ���� Send or Recv ������ �����ϴ� ������ ���θ� �����ϴ� ����
	unsigned char	iocp_buf[MAX_BUFF_SIZE];	// IOCP Send / Recv Buffer
};

struct Client
{
	bool				connect;					// ���� ���θ� ���� ����
	Object				player;						// ���ӻ��� ��ü�� �⺻���� ������ �����ϴ� ����
	SOCKET				sock;						// accept�� ���ؼ� ������ Network ����� ���� client socket
	WSAOVERLAPPED_EX	recv_overlap;				// recv�� ������ �����ϱ� ���� Ȯ�� overlapped ����ü
	unsigned char		packet_buf[MAX_PACKET_SIZE];	// recv�Ǵ� ��Ŷ�� �����Ǵ� Buffer / Send �� ���� ������ �����Ƿ� Ȯ�� ����ü���� ����
	int					prev_packet_size;			// ������ ���� ���� �����ϴ� ���� / Send �� ���� ������ �����Ƿ� Ȯ�� ����ü���� ����
	int					curr_packet_size;			// ���� ���� packet�� ��
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