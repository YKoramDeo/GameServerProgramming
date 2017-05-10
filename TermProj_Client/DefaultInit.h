#pragma once
#include "stdafx.h"
#include "../TermProj_Server/protocol.h"

namespace rmsdyddl
{
	struct RGB
	{
	public:
		int red;
		int green;
		int blue;

		void SetRGB(const int&, const int&, const int&);
	};

	struct Object
	{
	public:
		virtual void SetBoundary(void);
		void SetWindowPos(const int&, const int&);
		void SetWindowPos(const POINT&);
		void SetRadius(const int&);

		POINT GetWindowPos(void) const { return this->window_pos; }
		RECT GetBoundary(void) const { return this->boundary; }
		int GetRadius(void) const { return this->radius; }

	private:
		POINT window_pos;
		RECT boundary;
		int radius;	// 정 사각형의 중심으로부터 한 변에 이르는 최단 거리
	};

	namespace FIGURE {
		namespace BOARD { enum TYPE { NONE, WHITE, BLACK }; }
	}
}

class CBoard : public rmsdyddl::Object
{
public:
	CBoard() : mIndex{ 0,0 }, mType(rmsdyddl::FIGURE::BOARD::TYPE::NONE)
	{
		this->SetWindowPos(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
		this->SetRadius(50);
		this->SetBoundary();
		this->mRGB.SetRGB(250, 250, 250);
	}

	void SetIndex(const int&, const int&);
	void SetType(const int&);

	POINT GetIndex(void) const { return this->mIndex; }
	int GetType(void) const { return this->mType; }
	rmsdyddl::RGB GetRGB(void) const { return this->mRGB; }

	void Draw(const HDC&);
private:
	POINT mIndex;
	int mType;
	rmsdyddl::RGB mRGB;
};

class CPlayer : public rmsdyddl::Object
{
public:
	CPlayer() : mBoardPos{ 0, 0 }
	{
		this->SetWindowPos(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
		this->SetRadius(30);
		this->SetBoundary();
		this->mRGB.SetRGB(0, 0, 200);
	}

	void SetBoardPos(const POINT&);
	void SetBoardPos(const int&, const int&);

	rmsdyddl::RGB GetRGB(void) const { return this->mRGB; }
	POINT GetBoardPos(void) const { return this->mBoardPos; }

	void Move(const WPARAM&);
	void Draw(const HDC&);
private:
	rmsdyddl::RGB mRGB;
	POINT mBoardPos;
};

class CDrawManager
{
public:
	CDrawManager() : mBoardStartPos{ 0,0 }, mSightRange(8) { }

	void SetBoardStartPos(const POINT&);
	void SetBoardStartPos(const int&, const int&);
	void SetSightRange(const int&);

	POINT GetBoardStartPos(void) const { return this->mBoardStartPos; }
	int GetSightRange(void) const { return this->mSightRange; }

	void Move(const WPARAM&);
private:
	POINT mBoardStartPos;
	int mSightRange;
};

enum SCENE
{
	TITLE = 0,
	GAME = 1
};

extern CBoard gBoard[BOARD_LINE][BOARD_COLUMN];
extern CPlayer gPlayer;
extern CDrawManager gDrawMgr;
extern SCENE gScene;