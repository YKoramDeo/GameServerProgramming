#pragma once
#include "stdafx.h"

struct NETWORKINFO
{
	int		mID;
	SOCKET	mSock;
	WSABUF	mSendWSABuf;
	char	mSendBuf[BUF_SIZE];
	WSABUF	mRecvWSABuf;
	char	mRecvBuf[BUF_SIZE];
	char	mPacketBuf[BUF_SIZE];
	DWORD	mPacketSize;
	int		mSavedPacketSize;
};

void InitializeNetworkData(void);
void DisplayText(char *, ...);
void DisplayErrCode(char*);
void DisplayErrCodeAndQuit(char*);
void ReadPacket(SOCKET);
void ProcessPacket(char*);

extern NETWORKINFO gNetworkInfo;

void SendMovePacket(const WPARAM wParam);