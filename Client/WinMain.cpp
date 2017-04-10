#include "stdafx.h"
#include "DecProc.h"

HINSTANCE g_hlnst;
LPCTSTR lpszClassName = "WndClass_CoramDeo";
LPCTSTR lpszClassTitle = "GameServerProgramming TermProj (feat.CoramDeo)";

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpszCmdLine, int nCmdShow)
{
	HWND	hWnd;
	MSG	 Message;
	g_hlnst = hInstance;	// ���� Instance�� ���� �۷ι� ������ �����Ͽ� Procedure������ ����� �� �ֵ��� ������.
	WNDCLASS WndClass;		// Window ����ü ����

	WndClass.cbClsExtra = 0;	// O/S��� ���� �޸�(class)
	WndClass.cbWndExtra = 0;	// O/S��� ���� �޸�(Window)
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	// Window ȭ�� â ��� ��
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);		// Ŀ�� ����
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);	// ������ ����
	WndClass.hInstance = hInstance;				// ���� ���α׷� ID
	WndClass.lpfnWndProc = (WNDPROC)WndProc;	// Window Procedure �Լ���
	WndClass.lpszClassName = lpszClassName;			// Ŭ���� �̸�
	WndClass.lpszMenuName = NULL;				// �޴� �̸�
	WndClass.style = CS_HREDRAW | CS_VREDRAW;	// Window ��� ��Ÿ�� -> ���� / ������ ��ȭ�� �ٽ� �׸�.

	RegisterClass(&WndClass);		// Window Class ���

	RECT rc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	int nWidth = rc.right - rc.left;
	int nHeight = rc.bottom - rc.top;

	hWnd = CreateWindow(
		lpszClassName,			// ������ Ŭ���� �̸�
		lpszClassTitle,			// ������ Ÿ��Ʋ �̸�
		WS_OVERLAPPEDWINDOW,	// ������ ��Ÿ��
		WINDOW_POSITION_X,		// ������ ��ġ, x��ǥ
		WINDOW_POSITION_Y,		// ������ ��ġ, y��ǥ
		nWidth,					// ������ ��
		nHeight,				// ������ ����
		NULL,					// �θ� ������ �ڵ�
		(HMENU)NULL,			// �޴��ڵ�
		hInstance,				// �������α׷� ID
		NULL					// ������ ������ ����
	);

	ShowWindow(hWnd, nCmdShow);	// ������ �������� ȭ�����

	UpdateWindow(hWnd);			// O/S�� WM_PAINT �޽��� ����

	while (GetMessage(&Message, 0, 0, 0)) // WinProc()���� PostQuitMessage() ȣ���� ������ ó��.
	{
		TranslateMessage(&Message);		// Shift 'a' -> �빮�� 'A'
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