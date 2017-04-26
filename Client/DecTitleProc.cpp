#include "DecProc.h"

LRESULT CALLBACK DecTitleProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	static HDC hdc, memdc;
	static PAINTSTRUCT sPaintStructure;
	static RECT sWindowBoundary;
	static HBITMAP hscreen, oldscreen;

	switch (uMessage)
	{
	case WM_CREATE:
		GetClientRect(hWnd, &sWindowBoundary);
		InitializeClient();
		gEditHandle = CreateWindow(L"edit", NULL,
			WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL |
			ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY,
			0, 0, 0, 0, hWnd, (HMENU)100, gMainInstance, NULL);
		break;
	case WM_SIZE:
		GetClientRect(hWnd, &sWindowBoundary);
		hscreen = CreateCompatibleBitmap(hdc, sWindowBoundary.right, sWindowBoundary.bottom);
		break;
	
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &sPaintStructure);
		memdc = CreateCompatibleDC(hdc);
		if (hscreen == NULL)
			hscreen = CreateCompatibleBitmap(hdc, sWindowBoundary.right, sWindowBoundary.bottom);
		oldscreen = (HBITMAP)SelectObject(memdc, hscreen);

		Rectangle(memdc, sWindowBoundary.left, sWindowBoundary.top, sWindowBoundary.right, sWindowBoundary.bottom);

		StretchBlt(hdc, 0, 0, sWindowBoundary.right, sWindowBoundary.bottom, memdc, 0, 0, sWindowBoundary.right, sWindowBoundary.bottom, SRCCOPY);
		SelectObject(memdc, oldscreen);
		DeleteDC(memdc);
		EndPaint(hWnd, &sPaintStructure);
		break;
	}
	return 0;
}