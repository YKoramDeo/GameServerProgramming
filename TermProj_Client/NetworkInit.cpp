#include "NetworkInit.h"
#include "../TermProj_Server/protocol.h"

SOCKETINFO gSockInfo;

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
	DWORD iobyte, ioflag = 0;

	int ret = WSARecv(sock, &gSockInfo.mRecvWSABuf, 1, &iobyte, &ioflag, NULL, NULL);
	if (ret) DisplayErrCode("Read Packet : ");

	BYTE *ptr = reinterpret_cast<BYTE *>(gSockInfo.mRecvBuf);
	while (0 != iobyte)
	{
		if (0 == gSockInfo.mPacketSize) gSockInfo.mPacketSize = ptr[0];
		if (iobyte + gSockInfo.mSavedPacketSize >= gSockInfo.mPacketSize) {
			memcpy(gSockInfo.mPacketBuf + gSockInfo.mSavedPacketSize, ptr, gSockInfo.mPacketSize - gSockInfo.mSavedPacketSize);
			ProcessPacket(gSockInfo.mPacketBuf);
			ptr += gSockInfo.mPacketSize - gSockInfo.mSavedPacketSize;
			iobyte -= gSockInfo.mPacketSize - gSockInfo.mSavedPacketSize;
			gSockInfo.mPacketSize = 0;
			gSockInfo.mSavedPacketSize = 0;
		}
		else {
			memcpy(gSockInfo.mPacketBuf + gSockInfo.mSavedPacketSize, ptr, iobyte);
			gSockInfo.mSavedPacketSize += iobyte;
			iobyte = 0;
		}
	}

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


void ProcessPacket(char* ptr)
{
	switch (ptr[1])
	{
	default:
		DisplayErrCode("Unknown PACKET type \n");
		break;
	}
	return;
}

void SendMovePacket(const WPARAM wParam)
{
	BYTE direction = Const::MoveDirection::None;
	switch (wParam)
	{
	   case VK_LEFT	: direction = Const::MoveDirection::Left;	break;
	   case VK_RIGHT: direction = Const::MoveDirection::Right;	break;
	   case VK_UP	: direction = Const::MoveDirection::Up;		break;
	   case VK_DOWN	: direction = Const::MoveDirection::Down;	break;
	   default		: direction = Const::MoveDirection::None;	break;
	}
	CS_MOVE_PACKET packet;
	packet.size = sizeof(CS_MOVE_PACKET);
	packet.type = PacketType::CS_MOVE;
	packet.dir = direction;
	DWORD io_byte;
	
	unsigned char* packet_ptr = reinterpret_cast<unsigned char*>(&packet);
	gSockInfo.mSendWSABuf.len = packet_ptr[0];
	memcpy(gSockInfo.mSendWSABuf.buf, packet_ptr, packet_ptr[0]);
	int ret_val = WSASend(gSockInfo.mSock, &gSockInfo.mSendWSABuf, 1, &io_byte, 0, NULL, NULL);
	if (ret_val)
		DisplayErrCode("Error :: SendMovePacket Fail!!");
	return;
}