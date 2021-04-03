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
 * 2021-04-03   SVG export implemented
 * 2021-04-02   Scrolling, zooming, and background choice implemented
 * 2021-03-29   Created for VERSION 11.0.0 (to address the scrollbar mechanism, #6)
 */

// Two-step conversion macro of string literals into wide-character string literals
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

#include <fstream>

const TurtleCanvas::NameType TurtleCanvas::WCLASS_NAME = TEXT("TurtleCanvas");

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

TurtleCanvas::TurtleCanvas(Turtleizer& frame, HWND hFrame)
	: pFrame(&frame)
	, hFrame(hFrame)
	, hContextMenu(NULL)
	, hAccel(NULL)
	, snapLines(true) 
	, snapRadius(5.0f)
	, popupCoords(true)
	, showAxes(false)
	, zoomFactor(1.0)
	, autoUpdate(true)
	, scrollPos{0, 0}
{
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = TurtleCanvas::CanvasWndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
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
	HDC          hdc;
	PAINTSTRUCT  ps;

	TurtleCanvas* pInstance = getInstance();

	switch (message) {
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		pInstance->onPaint(hdc);	// instance-specific refresh
		EndPaint(hWnd, &ps);
		pInstance->adjustScrollbars();
		return 0;
	case WM_CONTEXTMENU:
	{
		// FIXME extract the coordinates from lParam
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		pInstance->onContextMenu(x, y);
		return 0;
	}
	case WM_HSCROLL:
	case WM_VSCROLL:
	{
		pInstance->onScrollEvent(LOWORD(wParam), HIWORD(wParam), message == WM_VSCROLL);
		return 0;
	}
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case VK_ADD:
		case VK_SUBTRACT:
			pInstance->zoom(wParam == VK_ADD);
			return 0;
		case VK_LEFT:
		case VK_RIGHT:
			pInstance->scroll(true, wParam == VK_RIGHT, false);
			return 0;
		case VK_UP:
		case VK_DOWN:
			pInstance->scroll(false, wParam == VK_DOWN, false);
			return 0;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	case WM_CHAR:
	{
		switch (wParam) {
		case 0x1b: // Escape
			// TODO close the window?
			return 0;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	case WM_COMMAND:
		if (pInstance->onCommand(wParam, lParam)) {
			return 0;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
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
	if (zoomIn) {
		this->zoomFactor = min(this->zoomFactor / ZOOM_RATE, MAX_ZOOM);
	}
	else {
		this->zoomFactor = max(this->zoomFactor * ZOOM_RATE, MIN_ZOOM);
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
	scrInfo.nTrackPos += this->scrollPos.x;	// Necessary at all?
	SetScrollInfo(this->hCanvas, SB_HORZ, &scrInfo, TRUE);

	// Set the vertical scroll parameters
	//scrInfo.fMask = SIF_ALL;
	//GetScrollInfo(this->hWnd, SB_VERT, &scrInfo);
	scrInfo.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	scrInfo.nMax = yMax;
	scrInfo.nPage = height;
	scrInfo.nPos = this->scrollPos.y;
	scrInfo.nTrackPos = this->scrollPos.y;	// Necessary at all?
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

VOID TurtleCanvas::onPaint(HDC hdc)
{
	Graphics graphics(hdc);
#if DEBUG_PRINT
	printf("executing onPaint on window %x\n", this->hWnd);	// DEBUG
#endif /*DEBUG_PRINT*/

	// Draw the background FIXME do we really have to redraw all?
	graphics.Clear(pFrame->backgroundColour);

	// START KGU 2021-03-31: Enh. #6 Care for zooming, displacements and scroll viewport translation
	//graphics.TranslateTransform(this->displacement.X, this->displacement.Y);
	graphics.TranslateTransform(-this->scrollPos.x, -this->scrollPos.y);
	graphics.ScaleTransform(this->zoomFactor, this->zoomFactor);
	graphics.TranslateTransform(this->displacement.X /** this->zoomFactor*/, this->displacement.Y /* this->zoomFactor*/);
	// END KGU 2021-03-21

	// Draw the recorded lines
	for (Turtleizer::Turtles::const_iterator it(pFrame->turtles.begin()); it != pFrame->turtles.end(); ++it)
	{
		(*it)->draw(graphics);
	}

	// START KGU 2021-03-31: Enh. #6 Draw the axes crossing if specified
	if (this->showAxes) {
		RectF bounds = this->pFrame->getBounds();
		Pen pen(Color(0xcc, 0xcc, 0xff), 1 / this->zoomFactor);
		REAL dashPattern[] = { 4.0f /*/ this->zoomFactor*/, 4.0f /*/ this->zoomFactor*/ };
		pen.SetDashPattern(dashPattern, 2);
		graphics.DrawLine(&pen, (int)bounds.X, 0, (int)(bounds.X + bounds.Width), 0);
		graphics.DrawLine(&pen, 0, (int)bounds.Y, 0, (int)(bounds.Y + bounds.Height));
	}
	// END KGU 2021-03-31

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
	InvalidateRect(this->hCanvas, &rcClient, FALSE);
	this->pFrame->updateStatusbar();
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

PointF TurtleCanvas::getMouseCoord(bool snap) const
{
	POINT ptMouse{0,0};
	BOOL done = TRUE;
	done = done && GetCursorPos(&ptMouse);
	done = done && ScreenToClient(this->hCanvas, &ptMouse);
	if (done) {
		RectF bounds = this->pFrame->getBounds();

		ptMouse.x = (LONG)((ptMouse.x + scrollPos.x) / this->zoomFactor - this->displacement.X);
		ptMouse.y = (LONG)((ptMouse.y + scrollPos.y) / this->zoomFactor - this->displacement.Y);
	}
	return PointF(ptMouse.x, ptMouse.y);
}

void TurtleCanvas::scrollToCoord(PointF coord)
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

	InvalidateRect(this->hCanvas, &rcClient, TRUE);
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
	// TODO change this when the dialog is implemented
	BOOL isToCheck = pInstance->popupCoords;
	if (testOnly) {
		return isToCheck;
	}
	pInstance->popupCoords = !isToCheck;
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
	// TODO change this when the dialog is implemented
	BOOL canDo = FALSE;
	if (!canDo || testOnly) {
		return canDo;
	}
	// TODO
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
	String fName = pInstance->chooseFileName(TEXT("All files\0*.*\0SVG files\0*.SVG\0"),
		TEXT("svg"), szFile);
	if (!fName.empty()) {
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
			ostr << "  <title>" << fName << "</title>\n";

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

TurtleCanvas::String TurtleCanvas::chooseFileName(LPTSTR filters, LPTSTR defaultExt, LPTSTR fileName)
{
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

	String pureName;
	if (GetSaveFileName(&ofn) == TRUE) {
		pureName = String(ofn.lpstrFile + ofn.nFileOffset);
		size_t posDot = pureName.rfind('.');
		pureName = pureName.substr(0, posDot);
	}
	return pureName;
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

