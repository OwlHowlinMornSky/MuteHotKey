#pragma once

#include "framework.h"

class ContextMenu {
public:
	enum Item {
		Exit = 40001,
		COUNT
	};

public:
	ContextMenu();
	~ContextMenu();

	void Pop(HWND hwnd, int nX, int nY);
	void Pop(HWND hwnd, const POINT& pt);

private:
	HMENU m_pPopMenu;
};
