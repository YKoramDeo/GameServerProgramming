#pragma once
#define WIN32_LEAN_AND_MEAN
// stdafx.cpp : ���� ��������� ������ ���� ǥ�� �ý��� ���� �� ������Ʈ ���� ���� �����Դϴ�.

#pragma comment (lib, "ws2_32.lib")

#include <windows.h>
//#include <winsock.h>
#include <WinSock2.h>
#include <iostream>
#include <mutex>

#define WINDOW_POSITION_X	0
#define WINDOW_POSITION_Y	0
#define WINDOW_WIDTH		800
#define WINDOW_HEIGHT		800

#define BOARD_LINE			8
#define BOARD_COLUMN		8

#define WM_SOCKET			(WM_USER + 1)
#define	BUF_SIZE			1024
#define LOOPBACK_ADDRESS	"127.0.0.1"

extern HWND gMainWindowHandle;
extern HINSTANCE gMainInstance;
extern HWND gEditHandle;
extern std::mutex gMutex;