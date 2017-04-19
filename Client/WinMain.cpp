#include "stdafx.h"
#include "DecProc.h"
#include "NetworkInit.h"

std::mutex gMutex;
HINSTANCE gMainInstance = NULL;
HWND gMainWindowHandle = NULL;
HWND gEditHandle = NULL;	// 편집 컨트롤		
LPCTSTR lpszClassName = L"WndClass_CoramDeo";
LPCTSTR lpszClassTitle = L"GameServerProgramming TermProj (feat.YKoramDeo)";

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpszCmdLine, int nCmdShow)
{
	HWND	hWnd;
	MSG	 Message;
	WNDCLASS WndClass;		// Window 구조체 정의

	WndClass.cbClsExtra = 0;	// O/S사용 여분 메모리(class)
	WndClass.cbWndExtra = 0;	// O/S사용 여분 메모리(Window)
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	// Window 화면 창 배경 색
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);		// 커서 유형
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);	// 아이콘 유형
	WndClass.hInstance = hInstance;				// 응용 프로그램 ID
	WndClass.lpfnWndProc = (WNDPROC)WndProc;	// Window Procedure 함수명
	WndClass.lpszClassName = lpszClassName;			// 클래스 이름
	WndClass.lpszMenuName = NULL;				// 메뉴 이름
	WndClass.style = CS_HREDRAW | CS_VREDRAW;	// Window 출력 스타일 -> 수직 / 수평의 변화시 다시 그림.

	RegisterClass(&WndClass);		// Window Class 등록

	RECT rc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	int nWidth = rc.right - rc.left;
	int nHeight = rc.bottom - rc.top;

	hWnd = CreateWindow(
		lpszClassName,			// 윈도우 클래스 이름
		lpszClassTitle,			// 윈도우 타이틀 이름
		WS_OVERLAPPEDWINDOW,	// 윈도우 스타일
		WINDOW_POSITION_X,		// 윈도우 위치, x좌표
		WINDOW_POSITION_Y,		// 윈도우 위치, y좌표
		nWidth,					// 윈도우 폭
		nHeight,				// 윈도우 높이
		NULL,					// 부모 윈도우 핸들
		(HMENU)NULL,			// 메뉴핸들
		hInstance,				// 응용프로그램 ID
		NULL					// 생성된 윈도우 정보
	);

	gMainWindowHandle = hWnd;
	gMainInstance = hInstance;	// 현재 Instance의 값을 글로벌 변수에 저장하여 Procedure에서도 사용할 수 있도록 저장함.

	ShowWindow(hWnd, nCmdShow);	// 생성된 윈도우의 화면출력

	UpdateWindow(hWnd);			// O/S에 WM_PAINT 메시지 전송

	InitializeNetworkData();

	while (GetMessage(&Message, 0, 0, 0)) // WinProc()에서 PostQuitMessage() 호출할 때까지 처리.
	{
		TranslateMessage(&Message);		// Shift 'a' -> 대문자 'A'
		DispatchMessage(&Message);		// WinMain -> WinProc
	}
	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		if (!DecProc(hWnd, uMessage, wParam, lParam))
			return DefWindowProc(hWnd, uMessage, wParam, lParam);
	}
	return 0;
}