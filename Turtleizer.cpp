#define _CRT_SECURE_NO_WARNINGS
#include "Turtleizer.h"
#include "resource.h"
/*
 * Fachhochschule Erfurt https://ai.fh-erfurt.de
 * Fachrichtung Angewandte Informatik
 * Project: Turtleizer_CPP (static C++ library for Windows)
 *
 * Author: Kay Gürtzig
 *
 * The automatic update of the drawing area is initially done after each drawing step,
 * but will then be done ever less frequently with the growing number of elements (traces)
 * to be rendered.
 * By invoking updateWindow(false) the regular update may be suppressed entirely. By
 * using updateWindow(true) you may re-enable the regular update.
 * BOTH call induce an immediate window update.
 *
 * History (add at top):
 * --------------------------------------------------------
 * 2024-10-05   VERSION 11.0.1: Explicit casts to avoid numeric conversion warnings,
 *              constructor accomplished
 * 2021-04-05   VERSION 11.0.0: Additions for #6 (GUI functionality ~ Structorizer 3.31 added)
 * 2019-07-02   VERSION 10.0.1: Fixed #1 (environment-dependent char array type), #2
 * 2018-10-23   VERSION 10.0.0: Now semantic version numbering with Version class.
 * 2018-07-30   VERSION 9: API adaptation to Structorizer 3.28-07: clear() procedure
 * 2017-10-29   VERSION 7: API adaptation to Structorizer 3.27:
 *              New methods/functions getX(), getY(), getOrientation()
 *              adapter functions now call startUp themselves if not done
 *              Comments translated to English, exposed on GitHub
 * 2016-12-09   VERSION 6: Decomposition and API extension for multiple Turtles
 * 2016-12-07   VERSION 5: API adaptation to Structorizer 3.25-09: setPenColor, setBackground
 * 2016-11-02   VERSION 4: API adaptation to Structorizer 3.25-03: separating forward/fd,
 *              element-count-dependent update cycles introduced
 * 2016-10-07   VERSION 3: New method awaitClose() instead of shutDown()
 * 2015-05-30	VERSION 2: ad0ditional arguments in method refresh(),
 *				new method updateWindow(bool) and function updateTurtleWindow(bool),
 *				new attribute autoUpdate,
 * 2015-04-30   refresh() calls reduced to improve performance
 * 2013-09-29	Accomplishment of refresh() by call of UpdateWindow()
 * 2013-09-27	turtleImagePath, makeFilePath() added for loading Turtle images
 * 2013-09-25.	flush inserted in onPaint() (for VS2012), turtleHeight, turtleWidth added
 * 2013-09-20	created
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <sstream>
 // Precaution for VS2012
#ifndef _MATH_DEFINES_DEFINED
#define M_PI 3.14159265358979323846
#endif /*_MATH_DEFINES_DEFINED*/

// Two-step conversion macro of string literals into wide-character string literals
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

const Turtleizer::Version Turtleizer::VERSION(11, 0, 1);

const Turtleizer::NameType Turtleizer::WCLASS_NAME = TEXT("Turtleizer");

const Color Turtleizer::colourTable[TC_VIOLET + 1] =
{
	Color(0, 0, 0),			// TC_BLACK
	Color(255, 0, 0),		// TC_RED
	Color(255, 255, 0),		// TC_YELLOW
	Color(0, 255, 0),		// TC_GREEN
	Color(0, 255, 255),		// TC_LIGHTBLUE = TC_CYAN
	Color(0, 0, 255),		// TC_BLUE
	Color(255, 0, 255),		// TC_MAGENTA
	Color(127, 127, 127),	// TC_GREY
	Color(255, 127, 0),		// TC_ORANGE
	Color(127, 0, 255)		// TC_VIOLETT
};

// We cannot rely on resource definitions as this is a static library...
const int Turtleizer::STATUSBAR_ICON_IDS[] = {
	IDI_HOME,
	IDI_TURTLE,
	-1,
	-1,
	IDI_MAGNIFIER,
	IDI_SNAP_LINES
};


Turtleizer* Turtleizer::pInstance = NULL;

HINSTANCE get_hInstance()
{
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery(get_hInstance, &mbi, sizeof(mbi));
	return reinterpret_cast<HINSTANCE>(mbi.AllocationBase);
}

Turtleizer::Turtleizer(String caption, unsigned int sizeX, unsigned int sizeY, HINSTANCE hInstance)
	: hWnd(NULL)
	, pCanvas(NULL)
	, hStatusbar(NULL)
	, gdiplusToken(NULL)
	, backgroundColour(Color::White)
	, showStatusbar(true)
	, statusbarPartWidths(nullptr)
	, msg{NULL, 0u, 0u, 0L, 0}
{
	// Initialize GDI+.
	GdiplusStartup(&this->gdiplusToken, &this->gdiplusStartupInput, NULL);

	if (hInstance == NULL) {
		hInstance = get_hInstance();
	}

	WNDCLASS wndClass = { 0 };
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = Turtleizer::WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = WCLASS_NAME;

	RegisterClass(&wndClass);

	this->hWnd = CreateWindow(
		WCLASS_NAME,			// window class name
		caption.c_str(),		// window caption
		WS_OVERLAPPEDWINDOW,	// window style
		CW_USEDEFAULT,			// initial x position
		CW_USEDEFAULT,			// initial y position
		sizeX,					// initial x size
		sizeY,					// initial y size
		NULL,					// parent window handle
		NULL,					// window menu handle
		hInstance,				// program instance handle
		NULL);					// creation parameters

#if DEBUG_PRINT
	char debug_buf[_MAX_PATH];
	_getcwd(debug_buf, 512);
	printf("Turtleizer: CWD = \"%s\"\n", debug_buf);
	char* envpath = getenv("PATH");
	printf("Turtleizer: PATH = \"%s\"\n", envpath);
	char* envlib = getenv("LIB");
	printf("Turtleizer: LIB = \"%s\"\n", envlib);
	system("set");
	printf("Current file: %s\n", __FILE__);
#endif /*DEBUG_PRINT*/

}

Turtleizer::~Turtleizer(void)
{
	GdiplusShutdown(this->gdiplusToken);
	for (Turtles::iterator itr = this->turtles.begin(); itr != this->turtles.end(); ++itr) {
		delete *itr;
		*itr = nullptr;
	}
}

// Returns the instance of the Turtleizer if there is any
Turtleizer* Turtleizer::getInstance()
{
	return pInstance;
}

// START KGU 2021-03-28: Enh. #6 (new GUI functions in correspondence to Structorizer)
void Turtleizer::setupWindowAddons(HINSTANCE hInstance)
{
	if (hInstance == NULL) {
		hInstance = get_hInstance();
	}

	// Ensure that the common control DLL is loaded.
	//InitCommonControls();	// is not found by the linker (though recommended to call)

	this->hStatusbar = CreateWindowEx(
		0,                       // no extended styles
		STATUSCLASSNAME,         // name of status bar class
		(PCTSTR)NULL,            // no text when first created
		SBARS_SIZEGRIP |         // includes a sizing grip
		WS_CHILD | WS_VISIBLE,   // creates a visible child window
		0, 0, 0, 0,              // ignores size and position
		this->hWnd,              // handle to parent window
		(HMENU)IDS_STATUSBAR,    // child window identifier
		hInstance,               // handle to application instance
		NULL);                   // no window creation data

	// Get the coordinates of the parent window's client area.
	RECT rcClient;
	GetClientRect(this->hWnd, &rcClient);
	const int nParts = sizeof(STATUSBAR_ICON_IDS) / sizeof(int);	// Number of status bar regions (temporarily)
	this->statusbarPartWidths = new int[nParts];
	int paParts[nParts];
	// FIXME
	// Calculate the right edge coordinate for each part, and
	// copy the coordinates to the array.
	int nWidth = rcClient.right / nParts;
	int rightEdge = nWidth;
	for (int i = 0; i < nParts; i++) {
		this->statusbarPartWidths[i] = nWidth;
		paParts[i] = rightEdge;
		rightEdge += nWidth;
	}

	// Tell the status bar to create the window parts.
	SendMessage(this->hStatusbar, SB_SETPARTS, (WPARAM)nParts, (LPARAM)
		paParts);
	bool hasSnapModeIcon = false;
	for (int i = 0; i < nParts; i++) {
		int iconId = STATUSBAR_ICON_IDS[i];
		if (iconId > 0) {
			// This will not work while Turtleizer_CPP is used as static library
			HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(STATUSBAR_ICON_IDS[i]));
			SendMessage(this->hStatusbar, SB_SETICON, i, (LPARAM)hIcon);
			if (i == nParts - 1) {
				hasSnapModeIcon = hIcon != NULL;
			}
		}
	}

	this->pCanvas = new TurtleCanvas(*this, this->hWnd);

}
// END KGU 2021-03-28

// Initialisation method wrapping the private constructor
Turtleizer* Turtleizer::startUp(unsigned int sizeX, unsigned int sizeY, HINSTANCE hInstance)
{
	const INT nCmdShow = SW_SHOWNORMAL;

	if (pInstance == NULL) {
		pInstance = new Turtleizer(WCLASS_NAME, sizeX, sizeY, hInstance);
		// ToDo set up the worker thread that is responding to the events
		pInstance->turtles.push_back(new Turtle(sizeX / 2, sizeY / 2));
		pInstance->home0 = Point(sizeX / 2, sizeY / 2);
		pInstance->setupWindowAddons(hInstance);
	}
	ShowWindow(pInstance->hWnd, nCmdShow);
	UpdateWindow(pInstance->hWnd);
	pInstance->updateStatusbar();
	return pInstance;
}

void Turtleizer::awaitClose()
{
	if (pInstance != NULL) {
		UpdateWindow(pInstance->hWnd);
	}
	Turtleizer::interact(NULL);
	if (pInstance != NULL) {
		delete pInstance;
		pInstance = NULL;
	}
}

// Make the turtle move the given number of pixels forward.
void Turtleizer::forward(double pixels)
{
	this->turtles.front()->forward(pixels);
}
// Make the turtle move the given number of pixels forward.
void Turtleizer::forward(double pixels, TurtleColour col)
{
	(this->turtles.front())->forward(pixels, colourTable[col]);
}

// Make the turtle move the given number of pixels forward.
void Turtleizer::fd(int pixels)
{
	(this->turtles.front())->fd(pixels);
}
// Make the turtle move the given number of pixels forward.
void Turtleizer::fd(int pixels, TurtleColour col)
{
	(this->turtles.front())->fd(pixels, colourTable[col]);
}

// Rotates the turtle to the left by some angle (degrees!).
void Turtleizer::left(double degrees)
{
	(this->turtles.front())->left(degrees);
}

// Sets the turtle to the position (X,Y).
void Turtleizer::gotoXY(int x, int y)
{
	(this->turtles.front())->gotoXY(x, y);
}

// Sets the X-coordinate of the turtle's position to a new value.
void Turtleizer::gotoX(int x)
{
	(this->turtles.front())->gotoX(x);
}

// Sets the Y-coordinate of the turtle's position to a new value.
void Turtleizer::gotoY(int y)
{
	(this->turtles.front())->gotoY(y);
}

// The turtle lifts the pen up, so when moving no line will be drawn
void Turtleizer::penUp()
{
	(this->turtles.front())->penUp();
}

// The turtle sets the pen down, so a line is being drawn when moving
void Turtleizer::penDown()
{
	(this->turtles.front())->penDown();
}

// Show the turtle again
void Turtleizer::showTurtle(bool show)
{
	(this->turtles.front())->showTurtle(show);
}

// Sets the Turtleizer background to the colour defined by the RGB values
void Turtleizer::setBackground(unsigned char red, unsigned char green, unsigned char blue)
{
	this->backgroundColour = Color(red, green, blue);
	this->pCanvas->setDirty();
	InvalidateRect(this->hWnd, NULL, TRUE);
	UpdateWindow(this->hWnd);
}

// Sets the default pen colour (used for moves without color argument) to the RGB values
void Turtleizer::setPenColor(unsigned char red, unsigned char green, unsigned char blue)
{
	(this->turtles.front())->setPenColor(red, green, blue);
}

// Wipes all drawn content from the Turtleizer canvas (without moving the turtle)
void Turtleizer::clear(bool allTurtles)
{
	if (!allTurtles) {
		this->turtles.front()->clear();
	}
	else {
		for (Turtle* pTurtle : this->turtles) {
			pTurtle->clear();
		}
	}
}

// Returns the current horizontal pixel position in floating-point resolution
double Turtleizer::getX() const
{
	return (this->turtles.front())->getX();
}

// Returns the current vertical pixel position in floating-point resolution
double Turtleizer::getY() const
{
	return (this->turtles.front())->getY();
}

// Returns the current orientation in degrees from North (clockwise = positive)
double Turtleizer::getOrientation() const
{
	return (this->turtles.front())->getOrientation();
}

// Refresh the window (i. e. invalidate the region between oldPos and this->pos) 
void Turtleizer::refresh(const RectF& rect, int nElements)
{
	if (nElements < 0) {
		// A turtle has cleared its traces such that all is to be redrawn completely
		this->pCanvas->setDirty();
	}
	pCanvas->redraw(rect, nElements);
}

// Creates and adds a new turtle symbolized by the the icon specifed by the given imagPath
// at the given position to the Turtleizer
Turtle* Turtleizer::addNewTurtle(int x, int y, LPCWSTR imagePath)
{
	Turtle* pTurtle = new Turtle(x, y, imagePath);
	if (pTurtle != nullptr) {
		this->turtles.push_back(pTurtle);
	}
	return pTurtle;
}


LRESULT CALLBACK Turtleizer::WndProc(HWND hWnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	HDC          hdc;
	PAINTSTRUCT  ps;

#if DEBUG_PRINT
	printf("Callback from Window %x started with message ", hWnd);
#endif /*DEBUG_PRINT*/
	switch (message)
	{
	case WM_PAINT:
#if DEBUG_PRINT
		printf("WM_PAINT...\n");	// DEBUG
#endif /*DEBUG_PRINT*/
		hdc = BeginPaint(hWnd, &ps);
		//pInstance->onPaint(hdc);	// instance-specific refresh
		//pInstance->pCanvas->repaint(hdc);
		EndPaint(hWnd, &ps);
		pInstance->updateStatusbar();
		return 0;
		// START KGU 2021-03-28/04-02: Enh. #6
	case WM_SIZE:
	{
		// Resize status bar
		// Get the Statusbar control handle which was previously stored in the 
		// user data associated with the parent window.
		SendMessage(pInstance->hStatusbar, WM_SIZE, 0, 0);
		pInstance->pCanvas->resize();
		return 0;
	}
	case WM_KEYDOWN:
	{
		SHORT shift = GetKeyState(VK_SHIFT);
		UINT count = lParam & 0xff;
		switch (wParam) {
		case VK_ADD:
		case VK_SUBTRACT:
			pInstance->pCanvas->zoom(wParam == VK_ADD);
			return 0;
		case VK_LEFT:
		case VK_RIGHT:
			pInstance->pCanvas->scroll(true, wParam == VK_RIGHT, shift & 0x8000, count);
			return 0;
		case VK_UP:
		case VK_DOWN:
			pInstance->pCanvas->scroll(false, wParam == VK_DOWN, shift & 0x8000, count);
			return 0;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	// END KGU 2021-03-28/04-02: Enh. #6
	case WM_DESTROY:
#if DEBUG_PRINT
		printf("WM_DESTROY...\n");	// DEBUG
#endif /*DEBUG_PRINT*/
		PostQuitMessage(0);
		return 0;
	default:
#if DEBUG_PRINT
		//printf("???...\n");
#endif /*DEBUG_PRINT*/
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
} // WndProc

//VOID Turtleizer::onPaint(HDC hdc)
//{
//	Graphics graphics(hdc);
//#if DEBUG_PRINT
//	printf("executing onPaint on window %x\n", this->hWnd);	// DEBUG
//#endif /*DEBUG_PRINT*/
//
//	// Draw the background FIXME do we really have to redraw all?
//	graphics.Clear(this->backgroundColour);
//
//	// START KGU 2021-03-31: Enh. #6 Care for zooming, displacements and scroll viewport translation
//	graphics.TranslateTransform(this->displacement.X, this->displacement.Y);
//	graphics.ScaleTransform(this->zoomFactor, this->zoomFactor);
//	graphics.TranslateTransform(-this->viewport.x, -this->viewport.y);
//	// END KGU 2021-03-21
//
//	// Draw the recorded lines
//	for (Turtles::const_iterator it(this->turtles.begin()); it != this->turtles.end(); ++it)
//	{
//		(*it)->draw(graphics);
//	}
//
//	// START KGU 2021-03-31: Enh. #6 Draw the axes crossing if specified
//	if (this->showAxes) {
//		RectF bounds = this->getBounds();
//		Pen pen(Color(0xcc,0xcc,0xff), 1/this->zoomFactor);
//		REAL dashPattern[] = {4.0f/this->zoomFactor, 4.0f/this->zoomFactor};
//		pen.SetDashPattern(dashPattern, 2);
//		graphics.DrawLine(&pen, (int)bounds.X, 0, (int)(bounds.X + bounds.Width), 0);
//		graphics.DrawLine(&pen, 0, (int)bounds.Y, 0, (int)(bounds.Y + bounds.Height));
//	}
//	// END KGU 2021-03-31
//
//}


void Turtleizer::updateStatusbar()
{

	if (this->showStatusbar) {

		HDC hdc = GetDC(this->hStatusbar);
		Graphics grsb(hdc);
		wchar_t statusBuffer[256];
		wsprintf(statusBuffer, L"(%i, %i)", this->home0.X, this->home0.Y);
		SendMessage(this->hStatusbar, SB_SETTEXT, 0, (LPARAM)statusBuffer);
		Turtle* turtle0 = this->turtles.front();
		double ori = turtle0->getOrientation();
		bool neg = ori < 0;
		int intgr = (int)floor(abs(ori));
		int frac = (int)((abs(ori) - intgr) * 100);
		wsprintf(statusBuffer, L"(%d, %d) %d.%02d°", (int)turtle0->getX(), (int)turtle0->getY(), intgr, frac);
		SendMessage(this->hStatusbar, SB_SETTEXT, 1, (LPARAM)statusBuffer);
		RectF bounds = this->getBounds();
		wsprintf(statusBuffer, L"%i x %i", (int)bounds.Width, (int)bounds.Height);
		SendMessage(this->hStatusbar, SB_SETTEXT, 2, (LPARAM)statusBuffer);
		// Get the scroll area bounds in turtle coords.
		RECT rcScroll = this->pCanvas->getScrollRect();
		wsprintf(statusBuffer, L"%d .. %d : %d .. %d",
			rcScroll.left, rcScroll.right, rcScroll.top, rcScroll.bottom);
		SendMessage(this->hStatusbar, SB_SETTEXT, 3, (LPARAM)statusBuffer);
		float zoomFactor = this->pCanvas->getZoomFacor();
		wsprintf(statusBuffer, L"%d.%d%%",
			(int)(zoomFactor * 100),
			((int)(zoomFactor * 1000)) % 10);
		SendMessage(this->hStatusbar, SB_SETTEXT, 4, (LPARAM)statusBuffer);
		bool hasSnapModeIcon = SendMessage(this->hStatusbar, SB_GETICON, 5, 0) != NULL;
		if (!hasSnapModeIcon) {
			wsprintf(statusBuffer, this->pCanvas->snapsToLines() ? L"+ → /" : L"+ → ▪");
			SendMessage(this->hStatusbar, SB_SETTEXT, 5, (LPARAM)statusBuffer);
		}

		// The LPtoDP and DPtoLP functions may be used to convert from logical points to device points
		// and from device points to logical points, respectively.
		HFONT hFont = (HFONT)SendMessage(this->hStatusbar, WM_GETFONT, 0, 0);
		Font* pFont = nullptr;
		int pos = 0;
		if (hFont != NULL) {
			bool resize = false;
			const int nParts = sizeof(STATUSBAR_ICON_IDS) / sizeof(int);
			int sepPositions[nParts];
			pFont = new Font(hdc, hFont);
			RectF bounds;
			// TODO Consider icon widths (in case there are icons)
			for (int i = 0; i < nParts; i++) {
				SendMessage(this->hStatusbar, SB_GETTEXT, i, (LPARAM)statusBuffer);
				grsb.MeasureString(statusBuffer, (INT)wcslen(statusBuffer), pFont, PointF(0.0f, 0.0f), &bounds);
				if (bounds.Width > this->statusbarPartWidths[i]
					|| bounds.Width < this->statusbarPartWidths[i] / 2) {
					resize = true;
				}
				sepPositions[i] = pos += (int)bounds.Width + 1;
			}
			if (resize) {
				SendMessage(this->hStatusbar, SB_SETPARTS, nParts, (LPARAM)&sepPositions);
				// Remember the new widths
				pos = 0;
				for (int i = 0; i < nParts; i++) {
					this->statusbarPartWidths[0] = sepPositions[i] - pos;
					pos = sepPositions[i];
				}
			}
			DeleteObject(pFont);
		}
		//ReleaseDC(this->hStatusbar, hdc);	// This a good idea?
	}

}

bool Turtleizer::snapToNearestPoint(PointF& coord, bool onLines, REAL radius) const
{
	REAL minDist = INFINITY;
	PointF nearest;
	for (Turtle* pTurtle : turtles) {
		PointF nearPt;
		REAL dist = pTurtle->getNearestPoint(coord, onLines, radius, nearPt);
		if (dist == 0) {
			coord = nearPt;
			return true;
		}
		else if (dist > 0 && dist < minDist) {
			nearest = nearPt;
			minDist = dist;
		}
	}
	if (minDist < INFINITY) {
		coord = nearest;
		return true;
	}
	return false;
}

RectF Turtleizer::getBounds() const
{
	RectF bounds;

	for (Turtles::const_iterator itr = this->turtles.begin(); itr != this->turtles.end(); ++itr) {
		RectF boundsI = (*itr)->getBounds();
		RectF::Union(bounds, bounds, boundsI);
	}

	return bounds;
}

DWORD WINAPI Turtleizer::interact(LPVOID lpParam)
{
	if (pInstance != NULL) {
		// This is a loop waiting for interactive shutting of the window
		pInstance->listen();
	}
	return 0;
}

// Member method 
void Turtleizer::listen() {
	while (GetMessage(&this->msg, NULL, 0, 0))
	{
		if (!this->pCanvas->translateAccelerators(&this->msg))
		{
			TranslateMessage(&this->msg);
			DispatchMessage(&this->msg);
		}
	}
}

void Turtleizer::getClientRect(RECT& rcClient) const
{
	GetClientRect(this->hWnd, &rcClient);
	if (this->showStatusbar) {
		RECT rcStatus;
		GetWindowRect(this->hStatusbar, &rcStatus);
		rcClient.bottom -= (rcStatus.bottom - rcStatus.top);
	}
}

// START KGU 2021-03-28: Enh. #6 workaround for resource access as a static library
LPCWSTR Turtleizer::getAbsolutePath(LPCWSTR filename) const
{
	WCHAR delimiter = L'/';
	LPCWSTR pMyPath = __WFILE__;
	LPCWSTR pPosSlash = wcsrchr(pMyPath, L'/');
	LPCWSTR pPosBSlash = wcsrchr(pMyPath, L'\\');
	if (pPosSlash < pPosBSlash) {
		pPosSlash = pPosBSlash;
		delimiter = L'\\';
	}
	size_t pathLen = (pPosSlash != nullptr) ? pPosSlash - pMyPath : wcslen(pMyPath);
	size_t buffLen = pathLen + wcslen(filename) + 2;
	WCHAR* pFilePath = new WCHAR[buffLen];
	wcsncpy_s(pFilePath, buffLen, pMyPath, pathLen);
	pFilePath[pathLen++] = delimiter;
	wcscpy_s(&pFilePath[pathLen], buffLen - pathLen, filename);
	return pFilePath;
}

// END KGU 2021-03-28

// Immediately updates the Turtleizer window i.e. refreshes all damaged regions.
// After having been called with argument false, automatic updates after every
// turtle movement will no longer be done, otherwise the turtle returns to the
// standard behaviour to update the window after every movement.
void Turtleizer::updateWindow(bool automatic)
{
	this->pCanvas->redraw(automatic);
}

Turtleizer::Version::Version(unsigned short major, unsigned short minor, unsigned short bugfix)
{
	this->levels[0] = major;
	this->levels[1] = minor;
	this->levels[2] = bugfix;
}

Turtleizer::Version::operator string() const
{
	char verStr[20];
	sprintf(verStr, "%d.%d.%d", this->levels[0], this->levels[1], this->levels[2]);
	return string(verStr);
}
bool Turtleizer::Version::operator<(const Version other) const
{
	for (unsigned short level = 0; level < N_LEVELS; level++) {
		if (this->levels[level] < other.levels[level]) {
			return true;
		}
		else if (this->levels[level] > other.levels[level]) {
			return false;
		}
	}
	return false;
}
bool Turtleizer::Version::operator==(const Version other) const
{
	for (unsigned short level = 0; level < N_LEVELS; level++) {
		if (this->levels[level] != other.levels[level]) {
			return false;
		}
	}
	return true;
}
bool Turtleizer::Version::operator!=(const Version other) const { return !(*this == other); }
bool Turtleizer::Version::operator<=(const Version other) const { return *this < other || *this == other; }
bool Turtleizer::Version::operator>(const Version other) const { return !(*this < other) && !(*this == other); }
bool Turtleizer::Version::operator>=(const Version other) const { return !(*this < other); }


/////////////////////// GLOBAL FUNCTIONS FOR SIMPLICITY ///////////////////////
// Calling any of these functions starts up the Turtleizer if it hadn't been //
///////////////////////////////////////////////////////////////////////////////

// Make the turtle move the given number of pixels forward in real coordinates.
void forward(double pixels)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle == NULL) {
		pTurtle = Turtleizer::startUp();
	}
	pTurtle->forward(pixels);
}

// Make the turtle move the given number of pixels forward.
void fd(int pixels)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle == NULL) {
		pTurtle = Turtleizer::startUp();
	}
	pTurtle->fd(pixels);
}

// Make the turtle move the given number of pixels forward in real coordinates.
void forward(double pixels, Turtleizer::TurtleColour col)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle == NULL) {
		pTurtle = Turtleizer::startUp();
	}
	pTurtle->forward(pixels, col);
}

// Make the turtle move the given number of pixels forward.
void fd(int pixels, Turtleizer::TurtleColour col)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle == NULL) {
		pTurtle = Turtleizer::startUp();
	}
	pTurtle->fd(pixels, col);
}

// Rotates the turtle to the left by some angle (degrees!).
void left(double degrees)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle == NULL) {
		pTurtle = Turtleizer::startUp();
	}
	pTurtle->left(degrees);
}

// Sets the turtle to the position (X,Y).
void gotoXY(int x, int y)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle == NULL) {
		pTurtle = Turtleizer::startUp();
	}
	pTurtle->gotoXY(x, y);
}

// Sets the X-coordinate of the turtle's position to a new value.
void gotoX(int x)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle == NULL) {
		pTurtle = Turtleizer::startUp();
	}
	pTurtle->gotoX(x);
}
// Sets the Y-coordinate of the turtle's position to a new value.
void gotoY(int y)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle == NULL) {
		pTurtle = Turtleizer::startUp();
	}
	pTurtle->gotoY(y);
}

// The turtle lifts the pen up, so when moving no line will be drawn
void penUp()
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle == NULL) {
		pTurtle = Turtleizer::startUp();
	}
	pTurtle->penUp();
}
// The turtle sets the pen down, so a line is being drawn when moving
void penDown()
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle == NULL) {
		pTurtle = Turtleizer::startUp();
	}
	pTurtle->penDown();
}

// Hides the turtle
void hideTurtle()
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle == NULL) {
		pTurtle = Turtleizer::startUp();
	}
	pTurtle->showTurtle(false);
}
// Show the turtle again
void showTurtle()
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle == NULL) {
		pTurtle = Turtleizer::startUp();
	}
	pTurtle->showTurtle(true);
}

// Sets the Turtleizer background to the colour defined by the RGB values
void setBackground(unsigned char red, unsigned char green, unsigned char blue)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle == NULL) {
		pTurtle = Turtleizer::startUp();
	}
	pTurtle->setBackground(red, green, blue);
}
// Sets the default pen colour (used for moves without color argument) to the RGB values
void setPenColor(unsigned char red, unsigned char green, unsigned char blue)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle == NULL) {
		pTurtle = Turtleizer::startUp();
	}
	if (pTurtle != NULL) {
		pTurtle->setPenColor(red, green, blue);
	}
}

// Wipes all drawn content from the Turtleizer canvas (without moving the turtle)
void clear(bool allTurtles)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle != NULL) {
		pTurtle->clear(allTurtles);
	}
}

// Returns the X-coordinate of the default turtle's position.
double getX()
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle == NULL) {
		pTurtle = Turtleizer::startUp();
	}
	return pTurtle->getX();
}
// Returns the Y-coordinate of the default turtle's position.
double getY()
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle == NULL) {
		pTurtle = Turtleizer::startUp();
	}
	return pTurtle->getY();
}
// Returns the current orientation of the default turtle.
double getOrientation()
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle == NULL) {
		pTurtle = Turtleizer::startUp();
	}
	return pTurtle->getOrientation();
}


// Immediately updates the Turtleizer window i.e. refreshes all damaged regions.
// After having been called with argument false, automatic updates after every
// turtle movement will no longer be done, otherwise the turtle returns to the
// standard behaviour to update the window after every movement.
void updateTurtleWindow(bool automatic)
{
	Turtleizer* pTurtleizer = Turtleizer::getInstance();
	if (pTurtleizer != NULL) {
		pTurtleizer->updateWindow(automatic);
	}
}


// Creates and adds a new turtle symbolized by the the icon specifed by the given imagPath
// at the given position to the Turtleizer
Turtle* addNewTurtle(int x, int y, LPCWSTR imagePath)
{
	Turtleizer* pTurtleizer = Turtleizer::getInstance();
	if (pTurtleizer == NULL) {
		pTurtleizer = Turtleizer::startUp();
	}
	return pTurtleizer->addNewTurtle(x, y, imagePath);;
}

