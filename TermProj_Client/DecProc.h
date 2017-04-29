#pragma once
#include "stdafx.h"

LRESULT CALLBACK DecGameProc(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK DecTitleProc(HWND, UINT, WPARAM, LPARAM);

void InitializeClient(void);

void ResetBoardSetting(void);