#define _USE_MATH_DEFINES
#include "TurtleCanvas.h"
#include "Turtleizer.h"
/*
 * Fachhochschule Erfurt https://ai.fh-erfurt.de
 * Fachrichtung Angewandte Informatik
 * Module: Programming
 *
 * Object class, representing one (of potentially many) Turtles withing the
 * simple C++ emulation of the Turtleizer module coming with Structorizer
 * (http://structorizer.fisch.lu).
 * The intention is that several separately controllable (and subclassible)
 * Turtle objects may be created to share the drawing area.
 *
 * Theme: Prep course Programming Fundamentals / Object-oriented Programming
 * Author: Kay Gürtzig
 * Version: 11.0.0 (covering capabilities of Structorizer 3.30-12, functional GUI)
 *
 * History (add on top):
 * --------------------------------------------------------
 * 2021-04-19   Separator choice for CSV export implemented
 * 2021-04-07   Some revisions for redrawing and tooltip update
 * 2021-04-05   Measuring tooltip implemented.
 * 2021-04-03   SVG export implemented
 * 2021-04-02   Scrolling, zooming, and background choice implemented
 * 2021-03-29   Created for VERSION 11.0.0 (to address the scrollbar mechanism, #6)
 */

// Two-step conversion macro of string literals into wide-character string literals
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

#include <cmath>
#include <fstream>
#include <windowsx.h>

const TurtleCanvas::NameType TurtleCanvas::WCLASS_NAME = TEXT("TurtleCanvas");

const TurtleCanvas::TDlgSaveCSV TurtleCanvas::tplSaveCSV = {
	{
		WS_CHILD | WS_CLIPSIBLINGS | DS_3DLOOK | DS_CONTROL,
		0,
		2 + N_CSV_SEPARATORS,
		0, 0,		// relative horizontal and vertical position
		75, 170	// horizontal and vertical size
	},
	0,	// no menu
	0,	// standard dialog box class
	0,	// no title
	// static text control for positioning
	{{WS_CHILD | WS_VISIBLE | SS_LEFT, 0, 0, 0, 0, 150, stc32}, 0xFFFF, 0x0082, 0, 0},
	// group box (label)
	{{WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 0, 1, 0, 70, 20 * N_CSV_SEPARATORS, IDC_CUST_START}, 0xFFFF, 0x0080, 0, 0},
	// radio buttons
	{
		{{WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, 0, 10, 15, 50, 20, IDC_CUST_START+1}, 0xFFFF, 0x0080, 0, 0},
		{{WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 0, 10, 35, 50, 15, IDC_CUST_START+2}, 0xFFFF, 0x0080, 0, 0},
		{{WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 0, 10, 55, 50, 15, IDC_CUST_START+3}, 0xFFFF, 0x0080, 0, 0},
		{{WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 0, 10, 75, 50, 15, IDC_CUST_START+4}, 0xFFFF, 0x0080, 0, 0},
		{{WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 0, 10, 95, 50, 15, IDC_CUST_START+5}, 0xFFFF, 0x0080, 0, 0},
	}
};


// START KGU 2021-03-28: Enhancements for #6
const float TurtleCanvas::MAX_ZOOM = 2.0f;
const float TurtleCanvas::MIN_ZOOM = 0.01f;
const float TurtleCanvas::ZOOM_RATE = 0.9f;

/* This array specifies the menu captions, accelerator keys and handler methods for all
 * the GUI functions of Turtleizer according to issue #6.
 * The aim was to accept both the lower and upper case version of accelerator letters, but
 * no attempt to make this happen worked. To specify them as virtual keys was recommended
 * by the Microsoft doc but no VK_A ... VK_Z macros are defined in the WinAPI. The VkKeyScan
 * macro was suggested in a Stack Overflow forum, e.g. {FVIRTKEY, LOBYTE(VkKeyScanA('G'))}.
 * But in the event it does not make the accelerator case-insensitive, either - this way
 * only the lower-case letters are accepted, but at least the Ctrl-S combination works now
 * (it did not with {FCONTROL, 'S'}). The lower-case letters are more user-friendly than
 * their upper-case counterparts, so we leave it as is.
 * (An attempt to add a second entry with lower-case Ascii and same command id for
 * any plain upper-case letter entry at run-tome did not work, either, by the way
 * (it may have confused the accelerator map generator).
 */
const TurtleCanvas::MenuDef TurtleCanvas::MENU_DEFINITIONS[] = {
	{TEXT("Scroll to coordinate ...\tG"), {FVIRTKEY, LOBYTE(VkKeyScanA('G'))}, TurtleCanvas::handleGotoCoord, false},
	{TEXT("Scroll to turtle positon\tEnd"), {FVIRTKEY, VK_END}, TurtleCanvas::handleGotoTurtle, false},
	{TEXT("Scroll to home position\tPos1"), {FVIRTKEY, VK_HOME}, TurtleCanvas::handleGotoHome, false},
	{TEXT("Scroll to origin (0,0)\t0"), {0, '0'}, TurtleCanvas::handleGotoOrigin, false},
	{NULL, {}, nullptr, false},
	{TEXT("Reset zoom to 100%\t1"), {0, '1'}, TurtleCanvas::handleZoom100, false},
	{TEXT("Zoom to the bounds\tZ"), {FVIRTKEY, LOBYTE(VkKeyScanA('Z'))}, TurtleCanvas::handleZoomBounds, false},
	{NULL, {}, nullptr, false},
	{TEXT("Make all drawing visible\tA"),{FVIRTKEY, LOBYTE(VkKeyScanA('A'))}, TurtleCanvas::handleShowAll, false},
	{TEXT("Show axes of coordinates\tO"), {FVIRTKEY, LOBYTE(VkKeyScanA('O'))}, TurtleCanvas::handleToggleAxes, true},
	{NULL, {}, nullptr, false},
	{TEXT("Show turtle\tT"), {FVIRTKEY, LOBYTE(VkKeyScanA('T'))}, TurtleCanvas::handleToggleTurtle, true},
	{TEXT("Set background colour ...\tB"), {FVIRTKEY, LOBYTE(VkKeyScanA('B'))}, TurtleCanvas::handleSetBackground, false},
	{NULL, {}, nullptr, false},
	{TEXT("Show statusbar\tS"), {FVIRTKEY, LOBYTE(VkKeyScanA('S'))}, TurtleCanvas::handleToggleStatus, true},
	{TEXT("Pop up coordinates\tC"), {FVIRTKEY, LOBYTE(VkKeyScanA('C'))}, TurtleCanvas::handleToggleCoords, true},
	{TEXT("Snap lines (else: points only)\tL"), {FVIRTKEY, LOBYTE(VkKeyScanA('L'))}, TurtleCanvas::handleToggleSnap, true},
	{TEXT("Set measuring snap radius ...\tR"), {FVIRTKEY, LOBYTE(VkKeyScanA('R'))}, TurtleCanvas::handleSetSnapRadius, false},
	{NULL, {}, nullptr, false},
	{TEXT("Update on every turtle action\tU"), {FVIRTKEY, LOBYTE(VkKeyScanA('U'))}, TurtleCanvas::handleToggleUpdate, true},
	{NULL, {}, nullptr, false},
	{TEXT("Export drawing items as CSV ...\tX"), {FVIRTKEY, LOBYTE(VkKeyScanA('X'))}, TurtleCanvas::handleExportCSV, false},
	{TEXT("Export drawing as PNG ...\tCtrl+S"), {FCONTROL | FVIRTKEY, LOBYTE(VkKeyScanA('S'))}, TurtleCanvas::handleExportPNG, false},
	{TEXT("Export drawing as SVG ...\tV"), {FVIRTKEY, LOBYTE(VkKeyScanA('V'))}, TurtleCanvas::handleExportSVG, false}
};
// END KGU 2021-03-28

// START KGU 2021-04-07: Issue #6 CSV export
const char* TurtleCanvas::CSV_COL_HEADERS[] = { "xFrom", "yFrom", "xTo", "yTo", "color" };
const char TurtleCanvas::CSV_SEPARATORS[N_CSV_SEPARATORS] = { ',', ';', '\t', ' ', ':' };
// END KGU 2021-04-07
// START KGU 2021-04-18: Separator configuration for CSV export (#6)
const TurtleCanvas::NameType TurtleCanvas::CSV_SEPARATOR = TEXT("Separator");
const TurtleCanvas::NameType TurtleCanvas::CSV_SEPARATOR_NAMES[N_CSV_SEPARATORS] = {
	TEXT("Comma"),
	TEXT("Semicolon"),
	TEXT("Tabulator"),
	TEXT("Blank"),
	TEXT("Colon")
};
unsigned short TurtleCanvas::ixCSVSepa = 0;
// END KGU 2021-04-18

TurtleCanvas::TurtleCanvas(Turtleizer& frame, HWND hFrame)
	: pFrame(&frame)
	, hFrame(hFrame)
	, hContextMenu(NULL)
	, hTooltip(NULL)
	, hAccel(NULL)
	, hArrow(NULL)
	, hCross(NULL)
	, hWait(NULL)
	, hdcScrCompat(NULL)
	, hBmpCompat(NULL)
	, tooltipInfo{ 0 }
	, snapLines(true) 
	, snapRadius(5.0f)
	, popupCoords(true)
	, showAxes(false)
	, zoomFactor(1.0)
	, autoUpdate(true)
	, scrollPos{0, 0}
	, pDragStart(NULL)
	, mouseCoord(0, 0)
	, tracksMouse(false)
	, mustRedraw(true)
{
	this->hArrow = LoadCursor(NULL, IDC_ARROW);
	this->hCross = LoadCursor(NULL, IDC_CROSS);
	this->hWait = LoadCursor(NULL, IDC_WAIT);

	WNDCLASS wndClass = { 0 };
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = TurtleCanvas::CanvasWndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = NULL;	// The cursor is handled explicitly
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = WCLASS_NAME;

	RegisterClass(&wndClass);

	RECT area;
	this->pFrame->getClientRect(area);

	this->hCanvas = CreateWindow(
		WCLASS_NAME,			// window class name
		NULL,					// window caption
		WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL,	// window style
		area.left,				// initial x position
		area.top,				// initial y position
		area.right - area.left,	// initial x size
		area.bottom - area.top,	// initial y size
		hFrame,					// parent window handle
		NULL,					// window menu handle
		hInstance,				// program instance handle
		NULL);					// creation parameters

	// Prepare Accelerators
	const int nMenuItems = sizeof(MENU_DEFINITIONS) / sizeof(MenuDef);
	ACCEL accels[nMenuItems];	// We may not use all of the elements
	int nAccels = 0;
	for (int i = 0; i < nMenuItems; i++) {
		const MenuDef* def = &MENU_DEFINITIONS[i];
		if (def->caption != NULL && def->accelerator.key != 0) {
			accels[nAccels] = def->accelerator;
			accels[nAccels++].cmd = IDM_CONTEXT_MENU + i;
		}
	}
	this->hAccel = CreateAcceleratorTable(accels, nAccels);

	// Put some initial custom colours
	int nCustColors = sizeof(customColors) / sizeof(COLORREF);
	for (int i = 0; i < nCustColors; i++) {
		customColors[i] = RGB(255, 255, 255);
	}

	// Create a tooltip
	// A tooltip control should not have the WS_CHILD style, nor should it 
	// have an id, otherwise its behavior will be adversely affected.
	this->hTooltip = CreateWindowEx(0/*WS_EX_TOPMOST*/, TOOLTIPS_CLASS, NULL,
		TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		this->hCanvas, NULL, this->hInstance, NULL);

	// Associate the tooltip with the measuring window
	this->tooltipInfo.cbSize = sizeof(TOOLINFO);
	this->tooltipInfo.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
	this->tooltipInfo.hwnd = this->hCanvas;
	this->tooltipInfo.uId = (UINT_PTR)this->hCanvas;
	this->tooltipInfo.lpszText = TEXT("(0, 0)");
	this->tooltipInfo.hinst = this->hInstance;

	//GetClientRect(this->hCanvas, &this->tooltipInfo.rect);	// FIXME this sensible?

	SendMessage(hTooltip, TTM_ADDTOOL, 0, (LPARAM)&this->tooltipInfo);

	// START KGU 2021-03-31: Issue #6
	adjustScrollbars();
	// END KGU 2021-03-21
}

TurtleCanvas::~TurtleCanvas()
{
	if (this->hAccel != NULL) {
		DestroyAcceleratorTable(this->hAccel);
	}
}

LRESULT TurtleCanvas::CanvasWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	TurtleCanvas* pInstance = getInstance();

	switch (message) {
	case WM_PAINT:
		pInstance->onPaint();	// instance-specific refresh
		pInstance->adjustScrollbars();
		return FALSE;
	case WM_CONTEXTMENU:
	{
		// FIXME extract the coordinates from lParam
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		pInstance->onContextMenu(x, y);
		return FALSE;
	}
	case WM_HSCROLL:
	case WM_VSCROLL:
	{
		pInstance->onScrollEvent(LOWORD(wParam), HIWORD(wParam), message == WM_VSCROLL);
		return FALSE;
	}
	case WM_MOUSEMOVE:
	{
		bool isButtonDown = (wParam & MK_LBUTTON) != 0;
		SetCursor(isButtonDown ? pInstance->hCross : pInstance->hArrow);
		if (!pInstance->tracksMouse && pInstance->hTooltip != NULL
			&& (pInstance->popupCoords || isButtonDown)) {
			TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
			tme.hwndTrack = pInstance->hCanvas;
			tme.dwFlags = TME_LEAVE;

			TrackMouseEvent(&tme);

			// Activate the tooltip.
			SendMessage(pInstance->hTooltip, TTM_TRACKACTIVATE, (WPARAM)TRUE,
				(LPARAM)&pInstance->tooltipInfo);

			pInstance->tracksMouse = true;

		}
		pInstance->onMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 
			isButtonDown && pInstance->hTooltip != NULL);
		return FALSE;
	}
	case WM_MOUSELEAVE: // The mouse pointer has left our window. Deactivate the tooltip.
	{
		if (pInstance->hTooltip != NULL) {
			SendMessage(pInstance->hTooltip, TTM_TRACKACTIVATE, (WPARAM)FALSE,
				(LPARAM)&pInstance->tooltipInfo);
		}
		pInstance->tracksMouse = false;
		return FALSE;
	}
	case WM_COMMAND:
		if (pInstance->onCommand(wParam, lParam)) {
			return FALSE;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

UINT_PTR TurtleCanvas::saveCSVHookProc(HWND hDlg, UINT msgId, WPARAM wParam, LPARAM lParam)
{
	TurtleCanvas* pInstance = getInstance();
	HWND hFileDlg = GetParent(hDlg);
	switch (msgId) {
	case WM_INITDIALOG:
	{
		RECT rcFDialog;
		SetDlgItemText(hDlg, 200, CSV_SEPARATOR);
		for (unsigned short i = 0; i < N_CSV_SEPARATORS; i++) {
			UINT idRBtn = IDC_CUST_START + i + 1;
			HWND hBtn = GetDlgItem(hDlg, idRBtn);
			if (hBtn != NULL) {
				SetDlgItemText(hDlg, idRBtn, CSV_SEPARATOR_NAMES[i]);
				if (i == ixCSVSepa) {
					CheckDlgButton(hDlg, idRBtn, BST_CHECKED);
				}
			}
		}
	}
		return FALSE;
	case WM_NOTIFY:
	{
		OFNOTIFY* pNotify = (OFNOTIFY*)lParam;
		UINT idFrom = pNotify->hdr.idFrom;
		switch (pNotify->hdr.code) {
		case CDN_FILEOK:
			for (unsigned short i = 0; i < N_CSV_SEPARATORS; i++) {
				if (IsDlgButtonChecked(hDlg, i + IDC_CUST_START + 1)) {
					ixCSVSepa = i;
				}
			}
			break;
		default:
			break;
		}
	}
		return FALSE;

	default:
		return FALSE;
	}
}


BOOL TurtleCanvas::CoordDialogProc(HWND hwndDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	// TODO
	const int txtLen = 80;
	TCHAR szCoord[txtLen]; // receives Coordinates.

	switch (message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			if (!GetDlgItemTextW(hwndDlg, 17, szCoord, txtLen))
				*szCoord = 0;

			// Fall through. 

		case IDCANCEL:
			EndDialog(hwndDlg, wParam);
			return TRUE;
		}
	}
	return FALSE;
}

void TurtleCanvas::redraw(const RectF& rectF, int nElements)
{
	// Perform the coordinate transformations
	RECT rect;
	rect.left = (rectF.X + this->displacement.X) * this->zoomFactor - this->scrollPos.x;
	rect.top = (rectF.Y + this->displacement.Y) * this->zoomFactor - this->scrollPos.y;
	rect.right = rect.left + this->zoomFactor * rectF.Width;
	rect.bottom = rect.top + this->zoomFactor * rectF.Height;
	InvalidateRect(this->hCanvas, &rect, TRUE);
	if (this->autoUpdate
		// START KGU4 2016-11-02: Reduce degrading of drawing speed with growing history
		&& (nElements % (nElements / 20 + 1) == 0)
		// END KGU4 2016-11-02
		) {
		UpdateWindow(this->hCanvas);
	}
}

void TurtleCanvas::redraw(bool automatic, const RECT* pRect)
{
	if (pRect == nullptr) {
		RECT rcClient;
		GetClientRect(this->hCanvas, &rcClient);
		pRect = &rcClient;
	}
	InvalidateRect(this->hCanvas, pRect, FALSE);
	UpdateWindow(this->hCanvas);
	this->autoUpdate = automatic;
}

TurtleCanvas* TurtleCanvas::getInstance()
{
	if (Turtleizer::pInstance != nullptr) {
		return Turtleizer::pInstance->pCanvas;
	}
	return nullptr;
}

void TurtleCanvas::resize()
{
	RECT rcClient;
	this->pFrame->getClientRect(rcClient);
	MoveWindow(this->hCanvas, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, TRUE);
	this->adjustScrollbars();
}

void TurtleCanvas::zoom(bool zoomIn)
{
	PointF center = this->getCenterCoord();
	float oldZoom = this->zoomFactor;
	if (zoomIn) {
		this->zoomFactor = min(oldZoom / ZOOM_RATE, MAX_ZOOM);
	}
	else {
		this->zoomFactor = max(oldZoom * ZOOM_RATE, MIN_ZOOM);
	}
	if (this->zoomFactor == oldZoom) {
		return;
	}
	this->scrollToCoord(center);
	this->adjustScrollbars();
	this->redraw(this->autoUpdate);
}

void TurtleCanvas::scroll(bool horizontally, bool forward, bool large, unsigned int count)
{
	RECT rcClient;
	GetClientRect(this->hCanvas, &rcClient);
	RectF bounds = pFrame->getBounds();
	int width = rcClient.right - rcClient.left;
	int height = rcClient.bottom - rcClient.top;
	int xMax = max(width - 1, (int)ceil((bounds.X + bounds.Width + this->displacement.X) * this->zoomFactor));
	int yMax = max(height - 1, (int)ceil((bounds.Y + bounds.Height + this->displacement.Y) * this->zoomFactor));

	int unit = count * (large ? 50 : 10);
	if (!forward) {
		unit *= -1;
	}
	LONG newScr = 0;
	if (horizontally) {
		newScr = max(0, min(this->scrollPos.x + unit, xMax - width));
		if (newScr == this->scrollPos.x) {
			return;
		}
		this->scrollPos.x = newScr;
	}
	else {
		newScr = max(0, min(this->scrollPos.y + unit, yMax - height));
		if (newScr == this->scrollPos.y) {
			return;
		}
		this->scrollPos.y = newScr;
	}
	this->mustRedraw = true;
	// Force the mouse coordinate to be updated
	if (this->hTooltip != NULL) {
		SendMessage(this->hTooltip, TTM_TRACKACTIVATE, (WPARAM)FALSE,
			(LPARAM)&this->tooltipInfo);
	}
	this->tracksMouse = false;
	this->adjustScrollbars();
	this->redraw(this->autoUpdate);
	this->pFrame->updateStatusbar();
}

void TurtleCanvas::adjustScrollbars()
{
	SCROLLINFO scrInfo{
		sizeof(SCROLLINFO),		// cbSize
		SIF_ALL,				// fMask
		0						// nMin
	};
	RECT rcClient;
	GetClientRect(this->hCanvas, &rcClient);
	RectF bounds = pFrame->getBounds();
	int width = rcClient.right - rcClient.left;
	int height = rcClient.bottom - rcClient.top;
	int xMax = max(width - 1 + scrollPos.x, (int)ceil((bounds.X + bounds.Width + this->displacement.X) * this->zoomFactor));
	int yMax = max(height - 1 + scrollPos.y, (int)ceil((bounds.Y + bounds.Height + this->displacement.Y) * this->zoomFactor));

	// Set the horizontal scroll parameters
	//GetScrollInfo(this->hWnd, SB_HORZ, &scrInfo);
	scrInfo.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	scrInfo.nMax = xMax;
	scrInfo.nPage = width;
	scrInfo.nPos = this->scrollPos.x;
	SetScrollInfo(this->hCanvas, SB_HORZ, &scrInfo, TRUE);

	// Set the vertical scroll parameters
	//scrInfo.fMask = SIF_ALL;
	//GetScrollInfo(this->hWnd, SB_VERT, &scrInfo);
	scrInfo.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	scrInfo.nMax = yMax;
	scrInfo.nPage = height;
	scrInfo.nPos = this->scrollPos.y;
	SetScrollInfo(this->hCanvas, SB_VERT, &scrInfo, TRUE);
}

bool TurtleCanvas::snapsToLines() const
{
	return this->snapLines;
}

float TurtleCanvas::getZoomFacor() const
{
	return this->zoomFactor;
}

PointF TurtleCanvas::getDisplacement() const
{
	return this->displacement;
}

RECT TurtleCanvas::getScrollRect() const
{
	RECT rcClient;
	GetClientRect(this->hCanvas, &rcClient);
	float width = (rcClient.right - rcClient.left) / this->zoomFactor;
	float height = (rcClient.bottom - rcClient.top) / this->zoomFactor;
	float left = this->scrollPos.x / this->zoomFactor - this->displacement.X;
	float top = this->scrollPos.y / this->zoomFactor - this->displacement.Y;
	rcClient.left = (LONG)left;
	rcClient.right = (LONG)(left + width);
	rcClient.top = (LONG)top;
	rcClient.bottom = (LONG)(top + height);
	return rcClient;
}

bool TurtleCanvas::translateAccelerators(LPMSG pMessage) const
{
	bool done = false;
	if (this->hAccel != NULL) {
		done = TranslateAccelerator(
			this->hCanvas,	// handle to receiving window 
			this->hAccel,	// handle to active accelerator table 
			pMessage) != 0;	// message data
	}
	return done;
}

void TurtleCanvas::setDirty()
{
	this->mustRedraw = true;
}

VOID TurtleCanvas::onPaint()
{
	HCURSOR oldCursor = GetCursor();
	PAINTSTRUCT  ps;
	HDC hdc = BeginPaint(this->hCanvas, &ps);
	
	Graphics graphics(hdc);
#if DEBUG_PRINT
	printf("executing onPaint on window %x\n", this->hWnd);	// DEBUG
#endif /*DEBUG_PRINT*/


	if (mustRedraw && this->hdcScrCompat != NULL) {
		DeleteDC(this->hdcScrCompat);
		DeleteObject(this->hBmpCompat);	// FIXME: Might it already have been deleted?;
		this->hBmpCompat = NULL;
		this->hdcScrCompat = NULL;
	}
	if (this->hdcScrCompat == NULL) {
		// Create a memory DC as bitmap backup for the turtle drawings
		// (these do not need redrawing, only additions unless the
		// background gets changed or certain turtle trayectories get cleared.
		// Therefore we do not have to redraw all but only the most recently
		// added lines into the bitmap and may then do a bitblp.
		// On scrolling, zooming, or resizing, however, we will have to
		// produce a new bitmap from scratch.
		// 
		// Only the turtle images and the measuring lines are to be drawn
		// directly onto the screen afterwards. This should dramatically
		// accelerate drawing (with the price of a buffered bitmap)
		// screen. The normal DC provides a snapshot of the 
		// screen contents. The memory DC keeps a copy of this 
		// snapshot in the associated bitmap.
		SetCursor(this->hWait);
		this->hdcScrCompat = CreateCompatibleDC(hdc);
		if (this->hdcScrCompat == NULL) {
			this->mustRedraw = true;
		}
		else {
			// Retrieve the metrics for the bitmap associated with the 
			// regular device context. 
			this->bmp.bmBitsPixel =
				(BYTE)GetDeviceCaps(hdc, BITSPIXEL);
			this->bmp.bmPlanes = (BYTE)GetDeviceCaps(hdc, PLANES);
			this->bmp.bmWidth = GetDeviceCaps(hdc, HORZRES);
			this->bmp.bmHeight = GetDeviceCaps(hdc, VERTRES);

			// The width must be byte-aligned. 
			this->bmp.bmWidthBytes = ((bmp.bmWidth + 15) & ~15) / 8;

			// Create a bitmap for the compatible DC. 
			this->hBmpCompat = CreateBitmap(bmp.bmWidth, bmp.bmHeight,
				bmp.bmPlanes, bmp.bmBitsPixel, (CONST VOID*) NULL);

			// Select the bitmap for the compatible DC. 
			SelectObject(hdcScrCompat, hBmpCompat);
		}
	}
	// Draw the background
	//graphics.Clear(pFrame->backgroundColour);
	//BitBlt()
	// START KGU 2021-03-31: Enh. #6 Care for zooming, displacements and scroll viewport translation
	//graphics.TranslateTransform(this->displacement.X, this->displacement.Y);
	graphics.TranslateTransform(-this->scrollPos.x, -this->scrollPos.y);
	graphics.ScaleTransform(this->zoomFactor, this->zoomFactor);
	graphics.TranslateTransform(this->displacement.X, this->displacement.Y);
	// END KGU 2021-03-21

	if (this->hdcScrCompat != NULL) {
		Graphics grCompat(this->hdcScrCompat);
		if (mustRedraw) {
			grCompat.Clear(pFrame->backgroundColour);
		}
		grCompat.TranslateTransform(-this->scrollPos.x, -this->scrollPos.y);
		grCompat.ScaleTransform(this->zoomFactor, this->zoomFactor);
		grCompat.TranslateTransform(this->displacement.X, this->displacement.Y);

		// Draw / update the recorded lines (without the turtle images temselves)
		for (Turtleizer::Turtles::const_iterator it(pFrame->turtles.begin()); it != pFrame->turtles.end(); ++it)
		{
			(*it)->draw(grCompat, mustRedraw, false);
		}

		// Now copy the contents to the true context
		RECT* prect = &ps.rcPaint;	// FIXME in certain cases we might need the entire client area

		BitBlt(ps.hdc,
			prect->left, prect->top,
			(prect->right - prect->left),
			(prect->bottom - prect->top),
			this->hdcScrCompat,
			prect->left,
			prect->top,
			SRCCOPY);

	}

	// Now draw all things that may be switched off or moved directly on the screen

	// START KGU 2021-03-31: Enh. #6 Draw the axes crossing if specified directly
	if (this->showAxes) {
		RectF bounds = this->pFrame->getBounds();
		Pen pen(Color(0xff, 0xcc, 0xcc), 1 / this->zoomFactor);
		REAL dashPattern[] = { 2.0f /*/ this->zoomFactor*/, 2.0f /*/ this->zoomFactor*/ };
		pen.SetDashPattern(dashPattern, 2);
		graphics.DrawLine(&pen, (int)bounds.X, 0, (int)(bounds.X + bounds.Width), 0);
		graphics.DrawLine(&pen, 0, (int)bounds.Y, 0, (int)(bounds.Y + bounds.Height));
	}

	// END KGU 2021-03-31
	// Draw the measuring line.
	// It is of course more logical to do it after all the turtle lines, but in case
	// we can't use the memory DC with the fast BitBlt we will have to start with the
	// measuring line because the process is much slower than the mouse tracking such
	// that it may often get aborted before it is the turn to draw the measuring line
	// otherwise. Even at the beginning it is often not drawn completely, or remnants
	// stay on the window. :-(
	if (this->pDragStart != NULL && this->tracksMouse) {
		Pen pen(Color(0xcc, 0xcc, 0xff), 1 / this->zoomFactor);
		REAL dashPattern[] = { 4.0f /*/ this->zoomFactor*/, 4.0f /*/ this->zoomFactor*/ };
		pen.SetDashPattern(dashPattern, 2);
		graphics.DrawLine(&pen,
			(int)this->pDragStart->X, (int)this->pDragStart->Y,
			(int)this->mouseCoord.X, (int)this->mouseCoord.Y);
	}

	if (this->hdcScrCompat != NULL) {
		// Draw the turtle images directly on the true device context
		for (Turtleizer::Turtles::const_iterator it(pFrame->turtles.begin()); it != pFrame->turtles.end(); ++it)
		{
			(*it)->drawImage(graphics);
		}
	}
	else {
		// Unfortunately we must draw all directly...

		// Draw / update the recorded lines (without the turtle images temselves)
		for (Turtleizer::Turtles::const_iterator it(pFrame->turtles.begin()); it != pFrame->turtles.end(); ++it)
		{
			(*it)->draw(graphics);
		}
	}

	this->mustRedraw = false;

	// END KGU 2021-03-31
	EndPaint(this->hCanvas, &ps);
	SetCursor(oldCursor);
}

VOID TurtleCanvas::onContextMenu(int x, int y)
{
	if (this->hContextMenu == NULL) {
		const int nMenuItems = sizeof(MENU_DEFINITIONS) / sizeof(MenuDef);
		this->hContextMenu = CreatePopupMenu();
		for (int i = 0; i < nMenuItems; i++) {
			const MenuDef* def = &MENU_DEFINITIONS[i];
			if (def->caption != NULL) {
				int flags = MF_BYPOSITION | MF_STRING;
				bool test = true;
				if (def->method != nullptr) {
					test = def->method(true);
				}
				if (def->isCheck && test) {
					// It is a checkbox menu item
					// TODO find out whether the status is on
					flags |= MF_CHECKED;
				}
				else if (!def->isCheck && !test) {
					flags |= MF_DISABLED;
				}
				AppendMenu(hContextMenu, flags, IDM_CONTEXT_MENU + i, def->caption);
			}
			else {
				AppendMenu(hContextMenu, MF_BYPOSITION | MF_SEPARATOR, IDM_CONTEXT_MENU + i, NULL);
			}
		}
	}
	else {
		updateContextMenu();
	}
	RECT rc;			// client area of window 
	POINT pt = { x, y };	// location of mouse click 

	// Get the bounding rectangle of the client area. 
	GetClientRect(this->hCanvas, &rc);

	// Convert the mouse position to client coordinates. 
	ScreenToClient(this->hCanvas, &pt);

	// If the position is in the client area, display a  
	// shortcut menu. 
	if (PtInRect(&rc, pt))
	{
		ClientToScreen(this->hCanvas, &pt);
		SetForegroundWindow(hCanvas);
		TrackPopupMenu(this->hContextMenu,
			TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON,
			pt.x, pt.y, 0,
			hCanvas, NULL);
	}
}

BOOL TurtleCanvas::onCommand(WPARAM wParam, LPARAM lParam)
{
	BOOL done = FALSE;
#if DEBUG_PRINT
	printf("On Command high %d low %d\n", HIWORD(wParam), LOWORD(wParam));
#endif /*DEBUG_PRINT*/
	const int nMenuItems = sizeof(MENU_DEFINITIONS) / sizeof(MenuDef);
	UINT code = LOWORD(wParam);
	if (code >= IDM_CONTEXT_MENU && code < IDM_CONTEXT_MENU + nMenuItems) {
		const MenuDef* pDef = &MENU_DEFINITIONS[code - IDM_CONTEXT_MENU];
		if (pDef->method != nullptr) {
			done = (pDef->method)(false);
		}
	}
	return done;
}

VOID TurtleCanvas::onScrollEvent(WORD scrollAction, WORD pos, BOOL isVertical)
{
	RECT rcClient;
	GetClientRect(this->hCanvas, &rcClient);
	RectF bounds = this->pFrame->getBounds();
	int width = rcClient.right - rcClient.left;
	int height = rcClient.bottom - rcClient.top;
	int xMax = max(width - 1, (int)ceil((bounds.X + bounds.Width + this->displacement.X) * this->zoomFactor));
	int yMax = max(height - 1, (int)ceil((bounds.Y + bounds.Height + this->displacement.Y) * this->zoomFactor));

	switch (scrollAction) {
	case SB_TOP:
		this->scrollPos.x = 0;
		this->scrollPos.y = 0;
		break;
	case SB_BOTTOM:
		this->scrollPos.x = max(xMax - width, 0);
		this->scrollPos.y = max(yMax - height, 0);
		break;
	case SB_LINEDOWN:
		if (isVertical) {
			if (this->scrollPos.y >= yMax - height) { return; }
			this->scrollPos.y = min(this->scrollPos.y + 10, max(yMax - height, 0));
		}
		else {
			if (this->scrollPos.x >= xMax - width) { return; }
			this->scrollPos.x = min(this->scrollPos.x + 10, max(xMax - width, 0));
		}
		break;
	case SB_LINEUP:
		if (isVertical) {
			if (this->scrollPos.y == 0) { return; }
			this->scrollPos.y = max(this->scrollPos.y - 10, 0);
		}
		else {
			if (this->scrollPos.x == 0) { return; }
			this->scrollPos.x = max(this->scrollPos.x - 10, 0);
		}
		break;
	case SB_PAGEDOWN:
		if (isVertical) {
			if (this->scrollPos.y >= yMax - height) { return; }
			this->scrollPos.y = min(this->scrollPos.y + height, max(yMax - height, 0));
		}
		else {
			if (this->scrollPos.x >= xMax - width) { return; }
			this->scrollPos.x = min(this->scrollPos.x + width, max(xMax - width, 0));
		}
		break;
	case SB_PAGEUP:
		if (isVertical) {
			if (this->scrollPos.y == 0) { return; }
			this->scrollPos.y = max(this->scrollPos.y - height, 0);
		}
		else {
			if (this->scrollPos.x == 0) { return; }
			this->scrollPos.x = max(this->scrollPos.x - width, 0);
		}
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		if (isVertical) {
			this->scrollPos.y = pos;
		}
		else {
			this->scrollPos.x = pos;
		}
		break;
	}
	this->mustRedraw = true;
	InvalidateRect(this->hCanvas, &rcClient, FALSE);
	this->pFrame->updateStatusbar();
}

VOID TurtleCanvas::onMouseMove(WORD X, WORD Y, BOOL isButtonDown)
{

	RectF bounds = this->pFrame->getBounds();
	PointF ptMouse;
	ptMouse.X = (X + scrollPos.x) / this->zoomFactor - this->displacement.X;
	ptMouse.Y = (Y + scrollPos.y) / this->zoomFactor - this->displacement.Y;
	if (this->pDragStart != NULL) {
		RECT rcClient;
		GetClientRect(this->hCanvas, &rcClient);
		InvalidateRect(this->hCanvas, &rcClient, TRUE);
	}

	if (isButtonDown) {
		this->pFrame->snapToNearestPoint(ptMouse, this->snapLines, this->snapRadius);
	}

	if (!ptMouse.Equals(this->mouseCoord)) {

		bool updateTooltip = this->hTooltip != NULL
			&& (isButtonDown || this->popupCoords);
		this->mouseCoord = ptMouse;

		TCHAR tooltip[100];
		if (updateTooltip) {
			// By default fill the popup with the current coordinates
#if UNICODE
			swprintf(
#else
			sprintf(
#endif /*UNICODE*/
				tooltip, ARRAYSIZE(tooltip), TEXT("(%d, %d)"), (int)ptMouse.X, (int)ptMouse.Y
			);
		}
		if (isButtonDown) {
			if (this->pDragStart == NULL) {
				this->pDragStart = new PointF(ptMouse);
			}
			else {
				REAL x0 = min(this->pDragStart->X, ptMouse.X) - 10;
				REAL x1 = max(this->pDragStart->X, ptMouse.X) - 10;
				REAL y0 = min(this->pDragStart->Y, ptMouse.Y) + 10;
				REAL y1 = max(this->pDragStart->Y, ptMouse.Y) + 10;
				RectF measureRect(x0, y0, x1 - x0, y1 - y0);
				RECT damaged = this->turtle2Window(measureRect);
				this->redraw(true, &damaged);

				// fill the popup with length, delta, and orientation
				float deltaX = ptMouse.X - this->pDragStart->X;
				float deltaY = ptMouse.Y - this->pDragStart->Y;
				float dist = sqrtf(deltaX * deltaX + deltaY * deltaY);
				float ori = atan2f(deltaX, deltaY) * 180 / M_PI;
#if UNICODE
				swprintf(
#else
				sprintf(
#endif /*UNICODE*/
					tooltip, ARRAYSIZE(tooltip), TEXT("%0.2f (%d, %d) %0.2f°"),
					dist, (int)deltaX, (int)deltaY, ori
				);
			}
		}
		else if (this->pDragStart != NULL) {
			PointF* pDrag = this->pDragStart;
			this->pDragStart = NULL;
			delete(pDrag);
			// Don't refresh tooltip immediately
			updateTooltip = false;
		}

		// FIXME remove this after test
		//wprintf(L"%c %s\n", this->hTooltip != NULL ? '+' : '-', tooltip);
		// Raise the popup
		if (this->hTooltip != NULL && (this->popupCoords || this->pDragStart != NULL)) {

			if (updateTooltip) {
				this->tooltipInfo.lpszText = tooltip;
				SendMessage(this->hTooltip, TTM_SETTOOLINFO, 0, (LPARAM)&this->tooltipInfo);
				//SendMessage(this->hTooltip, TTM_UPDATETIPTEXT, 0, (LPARAM)&this->tooltipInfo);
			}

			// A little offset is necessary, otherwise we'll get immediately a MV_MOUSELEAVE message
			POINT pt = { X + 10, Y + 10};
			ClientToScreen(this->hCanvas, &pt);
			//printf("Position tooltip to (%d,%d)\n", X, Y);
			SendMessage(this->hTooltip, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(pt.x, pt.y));
		}
	}
}

RECT TurtleCanvas::turtle2Window(const RectF& rect) const
{
	RECT rcClient;
	GetClientRect(this->hCanvas, &rcClient);
	RectF bounds = this->pFrame->getBounds();
	int width = rcClient.right - rcClient.left;
	int height = rcClient.bottom - rcClient.top;
	RECT rcTarget;
	rcTarget.left = max(rcClient.left, (LONG)((rect.X + this->displacement.X) * this->zoomFactor - this->scrollPos.x));
	rcTarget.top = max(rcClient.top, (LONG)((rect.Y + this->displacement.Y) * this->zoomFactor - this->scrollPos.y));
	rcTarget.right = min(rcClient.right, (LONG)((rect.X + rect.Width + this->displacement.X) * this->zoomFactor - this->scrollPos.x));
	rcTarget.bottom = min(rcClient.bottom, (LONG)((rect.Y + rect.Height + this->displacement.Y) * this->zoomFactor - this->scrollPos.y));

	return rcTarget;
}

PointF TurtleCanvas::getCenterCoord() const
{
	RECT rcClient;
	GetClientRect(this->hCanvas, &rcClient);
	RectF bounds = this->pFrame->getBounds();
	int width = rcClient.right - rcClient.left;
	int height = rcClient.bottom - rcClient.top;

	int xCenter = this->scrollPos.x + width / 2;	// Pixel coords
	int yCenter = this->scrollPos.y + height / 2;	// Pixel coords

	return PointF{
		xCenter / this->zoomFactor - this->displacement.X,
		yCenter / this->zoomFactor - this->displacement.Y,
	};
}

void TurtleCanvas::scrollToCoord(const PointF& coord)
{
	RECT rcClient;
	GetClientRect(this->hCanvas, &rcClient);
	RectF bounds = this->pFrame->getBounds();
	int width = rcClient.right - rcClient.left;
	int height = rcClient.bottom - rcClient.top;
	int xMax = max(width - 1, (int)ceil((bounds.X + bounds.Width + this->displacement.X) * this->zoomFactor));
	int yMax = max(height - 1, (int)ceil((bounds.Y + bounds.Height + this->displacement.Y) * this->zoomFactor));

	scrollPos.x = max(0, min((LONG)((coord.X + this->displacement.X) * this->zoomFactor) - width/2, xMax - width));
	scrollPos.y = max(0, min((LONG)((coord.Y + this->displacement.Y) * this->zoomFactor) - height/2, yMax - height));

	if (this->hTooltip != NULL) {
		SendMessage(this->hTooltip, TTM_TRACKACTIVATE, (WPARAM)FALSE,
			(LPARAM)&this->tooltipInfo);
	}
	this->tracksMouse = false;
	this->mustRedraw = true;
	InvalidateRect(this->hCanvas, &rcClient, TRUE);
	UpdateWindow(this->hCanvas);
	this->pFrame->updateStatusbar();
}

BOOL TurtleCanvas::handleGotoCoord(bool testOnly)
{
#if DEBUG_PRINT
	printf("handleGotoCoord\n");
#endif /*DEBUG_PRINT*/
	// TODO change this when the dialog is implemented
	BOOL canDo = FALSE;
	if (!canDo || testOnly) {
		return canDo;
	}
	// TODO: Open an dialog to ask for the coordinates
	//if (DialogBox(this->hInstance,
	//	TEXT(""),
	//	this->hCanvas,
	//	(DLGPROC)CoordDialogProc) == IDOK)
	//{
	//	// Complete the command; szItemName contains the 
	//	// name of the item to delete. 
	//}

	//else
	//{
	//	// Cancel the command. 
	//}
	// START KGU 2021-03-31: Enh. #6
	getInstance()->adjustScrollbars();
	// END KGU 2021-03-31
	return TRUE;
}

BOOL TurtleCanvas::handleGotoTurtle(bool testOnly)
{
#if DEBUG_PRINT
	printf("handleGotoTurtle\n");
#endif /*DEBUG_PRINT*/
	TurtleCanvas* pInstance = getInstance();
	BOOL canDo = !pInstance->pFrame->turtles.empty();
	if (!canDo || testOnly) {
		return canDo;
	}
	Turtle* turtle0 = getInstance()->pFrame->turtles.front();
	PointF posTurtle(turtle0->getX(), turtle0->getY());
	getInstance()->scrollToCoord(posTurtle);
	return TRUE;
}

BOOL TurtleCanvas::handleGotoHome(bool testOnly)
{
#if DEBUG_PRINT
	printf("handleGotoHome\n");
#endif /*DEBUG_PRINT*/
	TurtleCanvas* pInstance = getInstance();
	BOOL canDo = TRUE;
	if (!canDo || testOnly) {
		return canDo;
	}
	// TODO scroll to home
	PointF home(
		pInstance->pFrame->home0.X,
		pInstance->pFrame->home0.Y
	);
	pInstance->scrollToCoord(home);
	return TRUE;
}

BOOL TurtleCanvas::handleGotoOrigin(bool testOnly)
{
#if DEBUG_PRINT
	printf("handleGotoOrigin\n");
#endif /*DEBUG_PRINT*/
	BOOL canDo = TRUE;
	if (!canDo || testOnly) {
		return canDo;
	}
	getInstance()->scrollToCoord(PointF(0,0));
	return TRUE;
}

BOOL TurtleCanvas::handleZoom100(bool testOnly)
{
#if DEBUG_PRINT
	printf("handleZoom100\n");
#endif /*DEBUG_PRINT*/
	TurtleCanvas* pInstance = getInstance();
	BOOL canDo = pInstance->zoomFactor != 1.0f;
	if (!canDo || testOnly) {
		return canDo;
	}
	// Try to keep current center coordinate
	PointF center = pInstance->getCenterCoord();
	pInstance->zoomFactor = 1.0f;
	pInstance->mustRedraw = true;
	pInstance->scrollToCoord(center);
	return TRUE;
}

BOOL TurtleCanvas::handleZoomBounds(bool testOnly)
{
#if DEBUG_PRINT
	printf("handleZoomBounds\n");
#endif /*DEBUG_PRINT*/
	BOOL canDo = TRUE;
	if (!canDo || testOnly) {
		return canDo;
	}
	TurtleCanvas *pInstance = getInstance();
	RECT rcClient;
	GetClientRect(pInstance->hCanvas, &rcClient);
	RectF bounds = pInstance->pFrame->getBounds();
	float zoomH = min(MAX_ZOOM, (rcClient.right - rcClient.left) / (bounds.Width + bounds.X + pInstance->displacement.X));
	float zoomV = min(MAX_ZOOM, (rcClient.bottom - rcClient.top) / (bounds.Height + bounds.Y + pInstance->displacement.Y));
	pInstance->zoomFactor = max(MIN_ZOOM, min(zoomH, zoomV));
	pInstance->scrollPos.x = 0;
	pInstance->scrollPos.y = 0;
	pInstance->mustRedraw = true;
	pInstance->redraw(pInstance->autoUpdate);
	pInstance->adjustScrollbars();
	pInstance->pFrame->updateStatusbar();
	return TRUE;
}

BOOL TurtleCanvas::handleShowAll(bool testOnly)
{
#if DEBUG_PRINT
	printf("handleShowAll\n");
#endif /*DEBUG_PRINT*/
	TurtleCanvas* pInstance = getInstance();
	RectF bounds = pInstance->pFrame->getBounds();
	BOOL canDo = bounds.X + pInstance->displacement.X < 0
		|| bounds.Y + pInstance->displacement.Y < 0;
	if (!canDo || testOnly) {
		return canDo;
	}
	PointF center = pInstance->getCenterCoord();
	pInstance->displacement = PointF(max(-bounds.X, 0), max(-bounds.Y, 0));
	pInstance->scrollToCoord(center);
	pInstance->adjustScrollbars();
	pInstance->redraw(pInstance->autoUpdate);
	pInstance->pFrame->updateStatusbar();
	return TRUE;
}

BOOL TurtleCanvas::handleToggleAxes(bool testOnly)
{
#if DEBUG_PRINT
	printf("handleToggleAxes\n");
#endif /*DEBUG_PRINT*/
	TurtleCanvas* pInstance = getInstance();
	// TODO change this when the dialog is implemented
	BOOL isToCheck = pInstance->showAxes;
	if (testOnly) {
		return isToCheck;
	}
	pInstance->showAxes = !isToCheck;
	pInstance->redraw(pInstance->autoUpdate);
	return TRUE;
}

BOOL TurtleCanvas::handleToggleTurtle(bool testOnly)
{
#if DEBUG_PRINT
	printf("handleToggleTurtle\n");
#endif /*DEBUG_PRINT*/
	Turtleizer* pInstance = getInstance()->pFrame;
	BOOL isToCheck = pInstance->turtles.front()->isTurtleShown();
	if (testOnly) {
		return isToCheck;
	}
	pInstance->showTurtle(!isToCheck);
	return TRUE;
}

BOOL TurtleCanvas::handleSetBackground(bool testOnly)
{
#if DEBUG_PRINT
	printf("handleSetBackground\n");
#endif /*DEBUG_PRINT*/
	TurtleCanvas* pInstance = getInstance();
	if (testOnly) {
		return TRUE;
	}
	
	CHOOSECOLOR config;
	config.lStructSize = sizeof(CHOOSECOLOR);
	config.hwndOwner = pInstance->hCanvas;
	config.hInstance = NULL;
	config.rgbResult = pInstance->pFrame->backgroundColour.ToCOLORREF();
	config.lpCustColors = pInstance->customColors;
	config.Flags = CC_RGBINIT;

	if (ChooseColor(&config)) {
		pInstance->pFrame->setBackground(
			GetRValue(config.rgbResult),
			GetGValue(config.rgbResult),
			GetBValue(config.rgbResult)
		);
		//pInstance->redraw(pInstance->autoUpdate);
	}
	return TRUE;
}

BOOL TurtleCanvas::handleToggleCoords(bool testOnly)
{
#if DEBUG_PRINT
	printf("handleToggleCoords\n");
#endif /*DEBUG_PRINT*/
	TurtleCanvas* pInstance = getInstance();
	BOOL isToCheck = pInstance->popupCoords;
	if (testOnly) {
		return isToCheck;
	}
	pInstance->popupCoords = !isToCheck;
	if (isToCheck) {
		// Switch off the tracking tooltip now
		if (pInstance->hTooltip != NULL) {
			SendMessage(pInstance->hTooltip, TTM_TRACKACTIVATE, (WPARAM)FALSE,
				(LPARAM)&pInstance->tooltipInfo);
		}
		pInstance->tracksMouse = false;
	}
	return TRUE;
}

BOOL TurtleCanvas::handleToggleStatus(bool testOnly)
{
	//printf("handleToggleStatus\n");
	Turtleizer* pInstance = getInstance()->pFrame;
	BOOL isToCheck = pInstance->showStatusbar;
	if (testOnly) {
		return isToCheck;
	}
	isToCheck = !isToCheck;
	ShowWindow(pInstance->hStatusbar, isToCheck ? SW_SHOW : SW_HIDE);
	pInstance->showStatusbar = isToCheck;
	if (isToCheck) {
		pInstance->updateStatusbar();
	}
	getInstance()->resize();
	return TRUE;
}

BOOL TurtleCanvas::handleToggleSnap(bool testOnly)
{
	//printf("handleToggleSnap\n");
	TurtleCanvas* pInstance = getInstance();
	// TODO change this when the dialog is implemented
	BOOL isToCheck = pInstance->snapLines;
	if (testOnly) {
		return isToCheck;
	}
	pInstance->snapLines = !isToCheck;
	pInstance->pFrame->updateStatusbar();
	return TRUE;
}

BOOL TurtleCanvas::handleSetSnapRadius(bool testOnly)
{
	//printf("handleSnapRadius\n");
	// TODO change this when the dialog is implemented
	BOOL canDo = FALSE;
	if (!canDo || testOnly) {
		return canDo;
	}
	// TODO
	return TRUE;
}

BOOL TurtleCanvas::handleToggleUpdate(bool testOnly)
{
#if DEBUG_PRINT
	printf("handleToggleUpdate\n");
#endif /*DEBUG_PRINT*/
	TurtleCanvas* pInstance = getInstance();
	BOOL isToCheck = pInstance->autoUpdate;
	if (testOnly) {
		return isToCheck;
	}
	pInstance->redraw(!isToCheck);
	return TRUE;
}

BOOL TurtleCanvas::handleExportCSV(bool testOnly)
{
#if DEBUG_PRINT
	printf("handleExportCSV\n");
#endif /*DEBUG_PRINT*/
	BOOL canDo = FALSE;
	TurtleCanvas* pInstance = getInstance();
	for (Turtle* pTurtle : pInstance->pFrame->turtles) {
		if (pTurtle->hasElements()) {
			canDo = TRUE;
			break;
		}
	}
	if (!canDo || testOnly) {
		return canDo;
	}
	TCHAR szFile[_MAX_PATH] = { 0 };       // The buffer for the file path
	WORD ixNameStart = pInstance->chooseFileName(
		TEXT("All files\0*.*\0Comma-separated values files\0*.csv\0Text files\0*.txt\0"),
		TEXT("csv"), szFile,
		(LPOFNHOOKPROC)saveCSVHookProc, (LPDLGTEMPLATE)&tplSaveCSV.dlt);
	if (ixNameStart != 0xFFFFFFFF) {
		HCURSOR oldCursor = GetCursor();
		SetCursor(pInstance->hWait);
		const unsigned short nCols = sizeof(CSV_COL_HEADERS) / sizeof(char*);
		char separator = CSV_SEPARATORS[ixCSVSepa];	// Chosen separator
		std::ofstream ostr(szFile);
		if (ostr.is_open()) {
			for (unsigned short col = 0; col < nCols; col++) {
				if (col > 0) {
					ostr << separator;
				}
				ostr << CSV_COL_HEADERS[col];
			}
			ostr << std::endl;
			for (Turtle* pTurtle : pInstance->pFrame->turtles) {
				pTurtle->writeCSV(ostr, separator);
			}
		}
		else {
			MessageBox(
				pInstance->hFrame,
				TEXT("File could not be opened."),
				TEXT("Export failed"),
				MB_ICONERROR | MB_OK
			);
		}
		SetCursor(oldCursor);
	}
	else {
		MessageBox(
			pInstance->hFrame,
			TEXT("No CSV export was done."),
			TEXT("Export canceled"),
			MB_ICONERROR | MB_OK
		);
	}
	return TRUE;
}

BOOL TurtleCanvas::handleExportPNG(bool testOnly)
{
#if DEBUG_PRINT
	printf("handleExportPNG\n");
#endif /*DEBUG_PRINT*/
	// TODO change this when the dialog is implemented
	BOOL canDo = FALSE;
	if (!canDo || testOnly) {
		return canDo;
	}
	// TODO
	return TRUE;
}

BOOL TurtleCanvas::handleExportSVG(bool testOnly)
{
#if DEBUG_PRINT
	printf("handleExportSVG\n");
#endif /*DEBUG_PRINT*/
	BOOL canDo = FALSE;
	TurtleCanvas* pInstance = getInstance();
	for (Turtle* pTurtle : pInstance->pFrame->turtles) {
		if (pTurtle->hasElements()) {
			canDo = TRUE;
			break;
		}
	}
	if (!canDo || testOnly) {
		return canDo;
	}
	TCHAR szFile[_MAX_PATH] = { 0 };       // The buffer for the file path
	RectF bounds = pInstance->pFrame->getBounds();
	WORD ixNameStart = pInstance->chooseFileName(TEXT("All files\0*.*\0SVG files\0*.SVG\0"),
		TEXT("svg"), szFile);
	if (ixNameStart != 0xFFFFFFFF) {
		HCURSOR oldCursor = GetCursor();
		SetCursor(pInstance->hWait);
		// TODO get the scale via the saveFile dialog...
		unsigned short scale = 1;
		RectF bounds = pInstance->pFrame->getBounds();
		PointF offset(-bounds.X, -bounds.Y);
		std::ofstream ostr(szFile);
		if (ostr.is_open()) {
			ostr << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
			ostr << "<!-- Created with Turtleizer_CPP"
				<< " (https://github.com/codemanyak/Turtleizer_CPP) -->\n";
			ostr << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\""
				<< (long)ceil(bounds.Width * scale) << "\" height=\""
				<< (long)ceil(bounds.Height * scale) << "\">\n";
			ostr << "  <title>" << szFile + ixNameStart << "</title>\n";

			/* Draw the background:
			 * The fill colour must not be given as hex code, otherwise the rectangle
			 * will always be black! */
			Color bg = pInstance->pFrame->backgroundColour;
			ostr << "    <rect style=\"fill:rgb("
				<< (int)bg.GetRed() << "," << (int)bg.GetGreen() << "," << (int)bg.GetBlue()
				<< ");fill-opacity:1\" ";
			ostr << " x=\"0\" y=\"0\" width=\"" << (long)ceil(bounds.Width * scale)
				<< "\" height=\"" << (long)ceil(bounds.Height * scale) << "\" ";
			ostr << "id=\"background\"/>\n";

			// Now export the elements
			ostr << "  <g id=\"elements\" style=\"fill:none;stroke-width:"
				<< scale << + "px;stroke-opacity:1:stroke-linejoin:miter\">\n";

			for (Turtle* pTurtle : pInstance->pFrame->turtles) {
				pTurtle->writeSVG(ostr, offset, scale);
			}

			ostr << "  </g>\n";
			ostr << "</svg>\n";
		}
		else {
			MessageBox(
				pInstance->hFrame,
				TEXT("File could not be opened."),
				TEXT("Export failed"),
				MB_ICONERROR | MB_OK
			);
		}
		SetCursor(oldCursor);
	}
	else {
		MessageBox(
			pInstance->hFrame,
			TEXT("No SVG export was done."),
			TEXT("Export canceled"),
			MB_ICONERROR | MB_OK
		);

	}
	return TRUE;
}

WORD TurtleCanvas::chooseFileName(LPTSTR filters, LPTSTR defaultExt, LPTSTR fileName,
	LPOFNHOOKPROC lpHookProc, LPDLGTEMPLATE lpdt)
{
	WORD nameIndex = 0xFFFFFFFF;	// start position of the mere file name within the path
	OPENFILENAME ofn;       // common dialog box structure

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = getInstance()->hCanvas;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrFilter = filters;
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrDefExt = defaultExt;
	ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
	if (lpdt != NULL) {
		ofn.Flags |= OFN_ENABLETEMPLATEHANDLE;
		ofn.hInstance = (HINSTANCE)lpdt;	//FIXME!
	}
	if (lpHookProc != NULL) {
		ofn.Flags |= OFN_ENABLEHOOK;
		ofn.lpfnHook = lpHookProc;
	}

	if (GetSaveFileName(&ofn) == TRUE) {
		nameIndex = ofn.nFileOffset;
	}
	return nameIndex;
}

void TurtleCanvas::updateContextMenu()
{
	const int nMenuItems = sizeof(MENU_DEFINITIONS) / sizeof(MenuDef);
	for (int i = 0; i < nMenuItems; i++) {
		const MenuDef* pDef = &MENU_DEFINITIONS[i];
		if (pDef->caption != NULL && pDef->method != nullptr) {
			bool test = pDef->method(true);
			if (pDef->isCheck) {
				// In this case the test result decides about the checkbox
				CheckMenuItem(this->hContextMenu, IDM_CONTEXT_MENU + i,
					(test ? MF_CHECKED : MF_UNCHECKED));
			}
			else {
				// Otherwise th test result decides about enaled or disabled mode
				EnableMenuItem(this->hContextMenu, IDM_CONTEXT_MENU + i,
					(test ? MF_ENABLED : MF_DISABLED));
			}
		}
	}
}

