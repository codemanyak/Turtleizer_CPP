#define _CRT_SECURE_NO_WARNINGS
#include "Turtleizer.h"
/*
 * Fachhochschule Erfurt https://ai.fh-erfurt.de
 * Fachrichtung Angewandte Informatik
 * Module: Programming
 * Theme: Prep course Programming
 * Author: Kay Gürtzig
 *
 * The automatic update of the drawing area is initially done after each drawing step,
 * but then be done ever less frequently with the growing number of elements (traces)
 * to be rendered.
 * By invoking updateWindow(false) the regular update may be suppressed entirely. By
 * using updateWindow(true) you may re-enable the regular update.
 * BOTH call induce an immediate window update.
 *
 * History (add at top):
 * --------------------------------------------------------
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
// Precaution for VS2012
#ifndef _MATH_DEFINES_DEFINED
#define M_PI 3.14159265358979323846
#endif /*_MATH_DEFINES_DEFINED*/

// Two-step conversion macro of string literals into wide-character string literals
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

const Turtleizer::Version Turtleizer::VERSION(10, 0, 0);

const Turtleizer::NameType Turtleizer::WCLASS_NAME = TEXT("Turtleizer");

const Color Turtleizer::colourTable[TC_VIOLET + 1] =
{
	Color(0, 0, 0),	// TC_BLACK
	Color(255, 0, 0),	// TC_RED
	Color(255, 255, 0),	// TC_YELLOW
	Color(0, 255, 0),	// TC_GREEN
	Color(0, 255, 255),	// TC_LIGHTBLUE = TC_CYAN
	Color(0, 0, 255),	// TC_BLUE
	Color(255, 0, 255),	// TC_MAGENTA
	Color(127, 127, 127),	// TC_GREY
	Color(255, 127, 0),	// TC_ORANGE
	Color(127, 0, 255)	// TC_VIOLETT
};

Turtleizer* Turtleizer::pInstance = NULL;

Turtleizer::Turtleizer(String caption, unsigned int sizeX, unsigned int sizeY, HINSTANCE hInstance)
: hWnd(NULL)
, autoUpdate(true)
, gdiplusToken(NULL)
, backgroundColour(Color::White)
{
	// Initialize GDI+.
	GdiplusStartup(&this->gdiplusToken, &this->gdiplusStartupInput, NULL);

	wndClass.style          = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc    = Turtleizer::WndProc;
	wndClass.cbClsExtra     = 0;
	wndClass.cbWndExtra     = 0;
	wndClass.hInstance      = hInstance;
	wndClass.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName   = NULL;
	wndClass.lpszClassName  = WCLASS_NAME;

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

// Initialisation method wrapping the private constructor
Turtleizer* Turtleizer::startUp(unsigned int sizeX, unsigned int sizeY, HINSTANCE hInstance)
{
	const INT nCmdShow = SW_SHOWNORMAL;

	if (pInstance == NULL) {
		pInstance = new Turtleizer(WCLASS_NAME, sizeX, sizeY, hInstance);
		// ToDo set up the worker thread that is responding to the events
		pInstance->turtles.push_back(new Turtle(sizeX / 2, sizeY / 2));
	}
	ShowWindow(pInstance->hWnd, nCmdShow);
	UpdateWindow(pInstance->hWnd);
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
void Turtleizer::refresh(const RECT& rect, int nElements) const
{
	InvalidateRect(this->hWnd, &rect, TRUE);
	if (this->autoUpdate
		// START KGU4 2016-11-02: Reduce degrading of drawing speed with growing history
		&& (nElements % (nElements / 20 + 1) == 0)
		// END KGU4 2016-11-02
		) {
		UpdateWindow(this->hWnd);
	}
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
	switch(message)
	{
	case WM_PAINT:
#if DEBUG_PRINT
		printf("WM_PAINT...\n");	// DEBUG
#endif /*DEBUG_PRINT*/
		hdc = BeginPaint(hWnd, &ps);
		pInstance->onPaint(hdc);	// instanzabhängiges Refresh
		EndPaint(hWnd, &ps);
		return 0;
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

VOID Turtleizer::onPaint(HDC hdc)
{
	Graphics graphics(hdc);
#if DEBUG_PRINT
	printf("executing onPaint on window %x\n", this->hWnd);	// DEBUG
#endif /*DEBUG_PRINT*/

	// Draw the background
	graphics.Clear(this->backgroundColour);

	// Draw the recorded lines
	for (Turtles::const_iterator it(this->turtles.begin()); it != this->turtles.end(); ++it)
	{
		(*it)->draw(graphics);
	}

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
	while(GetMessage(&this->msg, NULL, 0, 0))
	{
		TranslateMessage(&this->msg);
		DispatchMessage(&this->msg);
	}
}

// Immediately updates the Turtleizer window i.e. refreshes all damaged regions.
// After having been called with argument false, automatic updates after every
// turtle movement will no longer be done, otherwise the turtle returns to the
// standard behaviour to update the window after every movement.
void Turtleizer::updateWindow(bool automatic)
{
	this->autoUpdate = automatic;
	UpdateWindow(this->hWnd);
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

