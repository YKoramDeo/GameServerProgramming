#include "NetworkInit.h"
#include "DefaultInit.h"
#include "../TermProj_Server/protocol.h"

NETWORKINFO gNetworkInfo;

void InitializeNetworkData(void) 
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	gNetworkInfo.mSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(MY_SERVER_PORT);
	ServerAddr.sin_addr.s_addr = inet_addr(LOOPBACK_ADDRESS);

	int Result = WSAConnect(gNetworkInfo.mSock, (sockaddr *)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);

	WSAAsyncSelect(gNetworkInfo.mSock, gMainWindowHandle, WM_SOCKET, FD_CLOSE | FD_READ);

	gNetworkInfo.mSendWSABuf.buf = gNetworkInfo.mSendBuf;
	gNetworkInfo.mSendWSABuf.len = BUF_SIZE;
	gNetworkInfo.mRecvWSABuf.buf = gNetworkInfo.mRecvBuf;
	gNetworkInfo.mRecvWSABuf.len = BUF_SIZE;
	gNetworkInfo.mID = -1;
	return;
}

void ReadPacket(SOCKET sock)
{
	DWORD iobyte, ioflag = 0;

	int ret = WSARecv(sock, &gNetworkInfo.mRecvWSABuf, 1, &iobyte, &ioflag, NULL, NULL);
	if (ret) DisplayErrCode("Read Packet : ");

	BYTE *ptr = reinterpret_cast<BYTE *>(gNetworkInfo.mRecvBuf);
	while (0 != iobyte)
	{
		if (0 == gNetworkInfo.mPacketSize) gNetworkInfo.mPacketSize = ptr[0];
		if (iobyte + gNetworkInfo.mSavedPacketSize >= gNetworkInfo.mPacketSize) {
			memcpy(gNetworkInfo.mPacketBuf + gNetworkInfo.mSavedPacketSize, ptr, gNetworkInfo.mPacketSize - gNetworkInfo.mSavedPacketSize);
			ProcessPacket(gNetworkInfo.mPacketBuf);
			ptr += gNetworkInfo.mPacketSize - gNetworkInfo.mSavedPacketSize;
			iobyte -= gNetworkInfo.mPacketSize - gNetworkInfo.mSavedPacketSize;
			gNetworkInfo.mPacketSize = 0;
			gNetworkInfo.mSavedPacketSize = 0;
		}
		else {
			memcpy(gNetworkInfo.mPacketBuf + gNetworkInfo.mSavedPacketSize, ptr, iobyte);
			gNetworkInfo.mSavedPacketSize += iobyte;
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
	gNetworkInfo.mSendWSABuf.len = packet_ptr[0];
	memcpy(gNetworkInfo.mSendWSABuf.buf, packet_ptr, packet_ptr[0]);
	int ret_val = WSASend(gNetworkInfo.mSock, &gNetworkInfo.mSendWSABuf, 1, &io_byte, 0, NULL, NULL);
	if (ret_val)
		DisplayErrCode("Error :: SendMovePacket Fail!!");
	return;
}

void ProcessPacket(char* ptr)
{
	switch (ptr[1])
	{
	case PacketType::SC_LOGIN_OK:
	  {
		SC_LOGIN_OK_PACKET *received_data_ptr = reinterpret_cast<SC_LOGIN_OK_PACKET*>(ptr);
		gNetworkInfo.mID = received_data_ptr->id;
		gPlayer.SetBoardPos(received_data_ptr->x_pos, received_data_ptr->y_pos);
		break;
	  }
	case PacketType::SC_ADD_OBJECT:
	  {
		SC_ADD_OBJECT_PACKET *received_data_ptr = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(ptr);
		gOther[received_data_ptr->id].SetConnect(true);
		gOther[received_data_ptr->id].SetBoardPos(received_data_ptr->x_pos, received_data_ptr->y_pos);
		break;
	  }
	case PacketType::SC_REMOVE_OBJECT:
	{
		SC_REMOVE_OBJECT_PACKET *received_data_ptr = reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(ptr);
		gOther[received_data_ptr->id].SetConnect(false);
		break;
	}
	case PacketType::SC_POSITION_INFO:
	  {
		SC_POSITION_INFO_PACKET *received_data_ptr = reinterpret_cast<SC_POSITION_INFO_PACKET*>(ptr);
		if (received_data_ptr->id == gNetworkInfo.mID)
		{
			gPlayer.SetBoardPos(received_data_ptr->x_pos, received_data_ptr->y_pos);
			gDrawMgr.Move(received_data_ptr->dir);
		}
		else
			gOther[received_data_ptr->id].SetBoardPos(received_data_ptr->x_pos, received_data_ptr->y_pos);
		break;
	  }
	default:
		DisplayErrCode("Unknown PACKET type \n");
		break;
	}
	return;
}