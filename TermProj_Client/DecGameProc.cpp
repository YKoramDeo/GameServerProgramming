#include "DecProc.h"
#include "DefaultInit.h"
#include "NetworkInit.h"

LRESULT CALLBACK DecGameProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
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
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:		PostQuitMessage(0);	break;
		case VK_LEFT:
		case VK_RIGHT:
		case VK_UP:
		case VK_DOWN:
			//gPlayer.Move(wParam);
			//gDrawMgr.Move(wParam);
			//ResetBoardSetting();
			SendMovePacket(wParam);
			break;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &sPaintStructure);
		memdc = CreateCompatibleDC(hdc);
		if (hscreen == NULL)
			hscreen = CreateCompatibleBitmap(hdc, sWindowBoundary.right, sWindowBoundary.bottom);
		oldscreen = (HBITMAP)SelectObject(memdc, hscreen);

		Rectangle(memdc, sWindowBoundary.left, sWindowBoundary.top, sWindowBoundary.right, sWindowBoundary.bottom);

		for (int yLine = gDrawMgr.GetBoardStartPos().y; yLine < gDrawMgr.GetBoardStartPos().y + gDrawMgr.GetSightRange(); ++yLine)
			for (int xColumn = gDrawMgr.GetBoardStartPos().x; xColumn < gDrawMgr.GetBoardStartPos().x + gDrawMgr.GetSightRange(); ++xColumn)
				gBoard[yLine][xColumn].Draw(memdc);

		gPlayer.Draw(memdc);

		/*
		{
			char buf[80];
			wsprintf(buf, "DrawMgr Board Start Pos [%d %d]", gDrawMgr.GetBoardStartPos().x, gDrawMgr.GetBoardStartPos().y);
			TextOut(memdc, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, buf, (int)strlen(buf));
		}
		*/

		StretchBlt(hdc, 0, 0, sWindowBoundary.right, sWindowBoundary.bottom, memdc, 0, 0, sWindowBoundary.right, sWindowBoundary.bottom, SRCCOPY);
		SelectObject(memdc, oldscreen);
		DeleteDC(memdc);
		EndPaint(hWnd, &sPaintStructure);
		break;
	case WM_SOCKET:
	   {
		if (WSAGETSELECTERROR(lParam)) {
			closesocket((SOCKET) wParam);
			DisplayErrCodeAndQuit("WM_SOCKET Error: ");
			break;
		}
		switch (WSAGETSELECTEVENT(lParam)) {
		case FD_READ:
			ReadPacket((SOCKET) wParam);
			ResetBoardSetting();
			InvalidateRect(hWnd, &sWindowBoundary, false);
			break;
		case FD_CLOSE:
			closesocket((SOCKET) wParam);
			DisplayErrCodeAndQuit("WM_SOCKET Close:");
			break;
		 }
	    }
	}
	return 0;
}

void InitializeClient(void)
{
	for (int yLine = 0; yLine < BOARD_LINE; ++yLine)
	{
		for (int xColumn = 0; xColumn < BOARD_COLUMN; ++xColumn)
		{
			gBoard[yLine][xColumn].SetIndex(xColumn, yLine);

			gBoard[yLine][xColumn].SetRadius(WINDOW_HEIGHT / gDrawMgr.GetSightRange() / 2);

			gBoard[yLine][xColumn].SetWindowPos(
				gBoard[0][0].GetRadius() + (gBoard[0][0].GetRadius() * 2 * xColumn),
				gBoard[0][0].GetRadius() + (gBoard[0][0].GetRadius() * 2 * yLine));

			if (yLine % 2 == 0 && xColumn % 2 == 0)
				gBoard[yLine][xColumn].SetType(rmsdyddl::FIGURE::BOARD::TYPE::BLACK);
			else if (yLine % 2 == 1 && xColumn % 2 == 1)
				gBoard[yLine][xColumn].SetType(rmsdyddl::FIGURE::BOARD::TYPE::BLACK);
			else
				gBoard[yLine][xColumn].SetType(rmsdyddl::FIGURE::BOARD::TYPE::WHITE);
		}
	}

	gPlayer.SetWindowPos(gBoard[gPlayer.GetBoardPos().y][gPlayer.GetBoardPos().x].GetWindowPos());
	gDrawMgr.SetBoardStartPos(0, 0);
	return;
}

void ResetBoardSetting(void)
{
	for (int yLine = 0; yLine < BOARD_LINE; ++yLine)
	{
		for (int xColumn = 0; xColumn < BOARD_COLUMN; ++xColumn)
		{
			gBoard[yLine][xColumn].SetWindowPos(
				// gBoard[0][0].GetRadius() + (gBoard[0][0].GetRadius() * 2 * xColumn) 
				//								- (gDrawStartPos.x * gBoard[0][0].GetRadius() * 2) 
				// 풀어씀. y도 동일.
				gBoard[0][0].GetRadius() * (1 + 2 * (xColumn - gDrawMgr.GetBoardStartPos().x)),
				gBoard[0][0].GetRadius() * (1 + 2 * (yLine - gDrawMgr.GetBoardStartPos().y))
			);
		}
	}

	gPlayer.SetWindowPos(gBoard[gPlayer.GetBoardPos().y][gPlayer.GetBoardPos().x].GetWindowPos());
	return;
}