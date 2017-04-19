#include "NetworkInit.h"
#include "../Server/protocol.h"

SOCKETINFO gSockInfo;

void ProcessSocketMessage(HWND, UINT, WPARAM, LPARAM);
void InitializeNetworkData(void) 
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	gSockInfo.mSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(MY_SERVER_PORT);
	ServerAddr.sin_addr.s_addr = inet_addr(LOOPBACK_ADDRESS);

	int Result = WSAConnect(gSockInfo.mSock, (sockaddr *)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);

	WSAAsyncSelect(gSockInfo.mSock, gMainWindowHandle, WM_SOCKET, FD_CLOSE | FD_READ);

	gSockInfo.mSendWSABuf.buf = gSockInfo.mSendBuf;
	gSockInfo.mSendWSABuf.len = BUF_SIZE;
	gSockInfo.mRecvWSABuf.buf = gSockInfo.mRecvBuf;
	gSockInfo.mRecvWSABuf.len = BUF_SIZE;

	return;
}