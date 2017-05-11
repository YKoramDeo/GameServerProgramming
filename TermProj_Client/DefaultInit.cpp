#include "DefaultInit.h"
#include "../TermProj_Server/protocol.h"

CBoard gBoard[BOARD_LINE][BOARD_COLUMN];
CPlayer gPlayer;
CPlayer gOther[MAX_USER];
CDrawManager gDrawMgr;
SCENE gScene = SCENE::GAME;

void rmsdyddl::RGB::SetRGB(const int& r, const int& g, const int& b)
{
	this->red = r;
	this->green = g;
	this->blue = b;
	return;
}

void rmsdyddl::Object::SetWindowPos(const int& x, const int& y) { this->window_pos = { x, y };	this->SetBoundary();	return; }
void rmsdyddl::Object::SetWindowPos(const POINT& point) { this->window_pos = point;		this->SetBoundary();	return; }
void rmsdyddl::Object::SetRadius(const int& num) { this->radius = num;		return; }

void rmsdyddl::Object::SetBoundary(void)
{
	this->boundary.top = this->window_pos.y - this->radius;
	this->boundary.bottom = this->window_pos.y + this->radius;
	this->boundary.left = this->window_pos.x - this->radius;
	this->boundary.right = this->window_pos.x + this->radius;
	return;
}

void CBoard::SetIndex(const int& x, const int& y)
{
	this->mIndex = { x,y };
	return;
}

void CBoard::SetType(const int& type)
{
	this->mType = type;
	if (rmsdyddl::FIGURE::BOARD::TYPE::WHITE == this->mType)
		this->mRGB.SetRGB(250, 250, 250);
	else
		this->mRGB.SetRGB(50, 50, 50);
	return;
}

void CBoard::Draw(const HDC& dc)
{
	HBRUSH hBrush, oldBrush;

	hBrush = CreateSolidBrush(RGB(this->mRGB.red, this->mRGB.blue, this->mRGB.green));
	oldBrush = (HBRUSH)SelectObject(dc, hBrush);

	Rectangle(dc, this->GetBoundary().left, this->GetBoundary().top, this->GetBoundary().right, this->GetBoundary().bottom);
	
	/*
	{
		char buf[80];
		wsprintf(buf, "[%d %d]", this->GetIndex().x, this->GetIndex().y);
		TextOut(dc, this->GetWindowPos().x, this->GetWindowPos().y, buf, (int)strlen(buf));
	}
	*/

	SelectObject(dc, oldBrush);
	DeleteObject(hBrush);
	return;
}

void CPlayer::SetBoardPos(const POINT& pos)
{
	this->mBoardPos = pos;
	return;
}

void CPlayer::SetBoardPos(const int&x, const int& y)
{
	this->mBoardPos.x = x;
	this->mBoardPos.y = y;
	return;
}

void CPlayer::SetRGB(const int& red, const int& green, const int& blue)
{
	this->mRGB.SetRGB(red, green, blue);
	return;
}
void CPlayer::SetConnect(const bool& boolean)
{
	this->mConnect = boolean;
	return;
}

void CPlayer::Move(const WPARAM& wParam)
{
	switch (wParam)
	{
	case VK_LEFT:
		this->mBoardPos.x--;
		if (this->mBoardPos.x <= 0)
			this->mBoardPos.x = 0;
		break;
	case VK_RIGHT:
		this->mBoardPos.x++;
		if (this->mBoardPos.x >= BOARD_COLUMN)
			this->mBoardPos.x = BOARD_COLUMN - 1;
		break;
	case VK_UP:
		this->mBoardPos.y--;
		if (this->mBoardPos.y <= 0)
			this->mBoardPos.y = 0;
		break;
	case VK_DOWN:
		this->mBoardPos.y++;
		if (this->mBoardPos.y >= BOARD_LINE)
			this->mBoardPos.y = BOARD_LINE - 1;
		break;
	}
	return;
}

void CPlayer::Draw(const HDC& dc)
{
	HBRUSH hBrush, oldBrush;

	hBrush = CreateSolidBrush(RGB(this->mRGB.red, this->mRGB.blue, this->mRGB.green));
	oldBrush = (HBRUSH)SelectObject(dc, hBrush);

	Rectangle(dc, this->GetBoundary().left, this->GetBoundary().top, this->GetBoundary().right, this->GetBoundary().bottom);

	/*
	{
		char buf[80];
		wsprintf(buf, "[%d %d]", this->GetBoardPos().x, this->GetBoardPos().y);
		TextOut(dc, this->GetWindowPos().x, this->GetWindowPos().y + 20, buf, (int)strlen(buf));
	}
	*/

	SelectObject(dc, oldBrush);
	DeleteObject(hBrush);
	return;
}

void CDrawManager::SetBoardStartPos(const POINT& pos)
{
	this->mBoardStartPos = pos;
	return;
}
void CDrawManager::SetBoardStartPos(const int&x, const int&y)
{
	this->mBoardStartPos.x = x;
	this->mBoardStartPos.y = y;
	return;
}

void CDrawManager::SetSightRange(const int& val)
{
	this->mSightRange = val;
	return;
}

void CDrawManager::Move(const WPARAM& wParam)
{
	// 예외처리 구문
	bool is_playerX_left_boundary = false;
	bool is_playerX_right_boundary = false;
	bool is_playerY_up_boundary = false;
	bool is_playerY_down_boundary = false;

	if (gPlayer.GetBoardPos().x < (this->mSightRange / 2) &&
		(0 == this->mBoardStartPos.x)) is_playerX_left_boundary = true;

	if (gPlayer.GetBoardPos().x >= BOARD_COLUMN - (this->mSightRange / 2) &&
		((BOARD_COLUMN - this->mSightRange) == this->mBoardStartPos.x)) is_playerX_right_boundary = true;

	if (gPlayer.GetBoardPos().y < (this->mSightRange / 2) &&
		(0 == this->mBoardStartPos.y)) is_playerY_up_boundary = true;

	if (gPlayer.GetBoardPos().y >= BOARD_LINE - (this->mSightRange / 2) &&
		((BOARD_COLUMN - this->mSightRange) == this->mBoardStartPos.y)) is_playerY_down_boundary = true;

	if (is_playerY_up_boundary && (is_playerX_left_boundary || is_playerX_right_boundary)) return;
	if (is_playerY_down_boundary && (is_playerX_left_boundary || is_playerX_right_boundary)) return;

	RECT boundary;

	boundary.left = 0;
	boundary.right = BOARD_COLUMN - this->mSightRange;
	boundary.top = 0;
	boundary.bottom = BOARD_LINE - this->mSightRange;

	switch (wParam)
	{
	case VK_LEFT:
		if (is_playerX_left_boundary || is_playerX_right_boundary) break;
		this->mBoardStartPos.x--;
		if (this->mBoardStartPos.x <= boundary.left)
			this->mBoardStartPos.x = 0;
		break;
	case VK_RIGHT:
		if (is_playerX_left_boundary || is_playerX_right_boundary) break;
		this->mBoardStartPos.x++;
		if (this->mBoardStartPos.x >= boundary.right)
			this->mBoardStartPos.x = boundary.right;
		break;
	case VK_UP:
		if (is_playerY_down_boundary || is_playerY_up_boundary) break;
		this->mBoardStartPos.y--;
		if (this->mBoardStartPos.y <= boundary.top)
			this->mBoardStartPos.y = 0;
		break;
	case VK_DOWN:
		if (is_playerY_down_boundary || is_playerY_up_boundary) break;
		this->mBoardStartPos.y++;
		if (this->mBoardStartPos.y >= boundary.bottom)
			this->mBoardStartPos.y = boundary.bottom;
		break;
	}
	return;
}