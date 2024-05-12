#include "ContextMenu.h"

ContextMenu::ContextMenu() {
	m_pPopMenu = CreatePopupMenu();
	if (m_pPopMenu) {
		InsertMenuW(m_pPopMenu, (-1), MF_BYPOSITION, Item::Exit, L"Exit");
	}
}

ContextMenu::~ContextMenu() {
	if (m_pPopMenu) {
		DestroyMenu(m_pPopMenu);
		m_pPopMenu = NULL;
	}
}

void ContextMenu::Pop(HWND hwnd, int nX, int nY) {
	if (m_pPopMenu && hwnd) {
		SetForegroundWindow(hwnd);
		TrackPopupMenu(
			m_pPopMenu,
			TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON | TPM_VERNEGANIMATION,
			nX, nY, NULL, hwnd, NULL
		);
		PostMessageW(hwnd, WM_NULL, 0, 0);
	}
}

void ContextMenu::Pop(HWND hwnd, const POINT& pt) {
	Pop(hwnd, pt.x, pt.y);
}
