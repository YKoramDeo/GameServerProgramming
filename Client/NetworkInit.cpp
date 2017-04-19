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

void ReadPacket(SOCKET sock)
{
	return;
}

void DisplayText(char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	char cBuf[BUF_SIZE + 256];
	vsprintf(cBuf, fmt, arg);

	gMutex.lock();
	int nLength = GetWindowTextLength(gEditHandle);
	SendMessage(gEditHandle, EM_SETSEL, nLength, nLength);
	SendMessage(gEditHandle, EM_REPLACESEL, FALSE, (LPARAM)cBuf);
	gMutex.unlock();

	va_end(arg);
	return;
}

void DisplayErrCodeAndQuit(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, 
		WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf,reinterpret_cast<LPCWSTR>(msg), MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
	return;
}

void DisplayErrCode(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	DisplayText("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
	return;
}