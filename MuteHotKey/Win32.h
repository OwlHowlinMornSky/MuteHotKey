#pragma once

#include "framework.h"

void MyLoadString(HINSTANCE hInst);

bool MyRegisterClass(HINSTANCE hInst);

HWND MyCreateWindow(HINSTANCE hInst, int nCmdShow);

/**
 * @brief 添加托盘图标
 * @param hInst 当前实例
 * @param hWnd 所属窗口
 * @return 是否成功
*/
bool MyAddNotifyIcon(HINSTANCE hInst, HWND hWnd);

/*
*@brief 移除托盘图标
*/
void MyRemoveNotifyIcon();
