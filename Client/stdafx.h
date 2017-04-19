#pragma once
#define WIN32_LEAN_AND_MEAN
// stdafx.cpp : 자주 사용하지만 수정이 적은 표준 시스템 포함 밑 프로젝트 관련 포함 파일입니다.

#include <windows.h>
#include <winsock.h>
#include <WinSock2.h>

#define WINDOW_POSITION_X	0
#define WINDOW_POSITION_Y	0
#define WINDOW_WIDTH		800
#define WINDOW_HEIGHT		800

#define BOARD_LINE			8
#define BOARD_COLUMN		8

#define WM_SOCKET			(WM_USER + 1)
#define	BUF_SIZE			1024

extern HWND gMainWindowHandle;
extern HINSTANCE gMainInstance;