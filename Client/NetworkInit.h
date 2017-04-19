#pragma once
#include "stdafx.h"

struct SOCKETINFO
{
	SOCKET	mSock;
	WSABUF	mSendWSABuf;
	char	mSendBuf[BUF_SIZE];
	WSABUF	mRecvWSABuf;
	char	mRecvBuf[BUF_SIZE];
	char	mPacketBuf[BUF_SIZE];
	DWORD	mPacketSize[BUF_SIZE];
	int		mSavedPacketSize;
};

void ProcessSocketMessage(HWND, UINT, WPARAM, LPARAM);
void InitializeNetworkData(void);
void DisplayText(char *, ...);
void DisplayErrCode(char*);
void DisplayErrCodeAndQuit(char*);
void ReadPacket(SOCKET);

extern SOCKETINFO gSockInfo;