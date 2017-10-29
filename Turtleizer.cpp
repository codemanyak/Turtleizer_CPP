#include "Turtleizer.h"
/*
 * Fachhochschule Erfurt www.fh-erfurt.de/ai
 * Fachgebiet Angewandte Informatik
 * Modul Programmierung
 *
 * Objektklasse, die den Turtleizer des Structorizers
 * (http://structorizer.fisch.lu)
 * für eine einfache C-/C++-Umgebung nachbildet, dabei aber
 * die Möglichkeit bietet, zusätzliche Turtles zu erzeugen und zu bewegen
 * Thema: Brückenkurs Programmierung
 * Autor: Kay Gürtzig
 * Version: 6
 *
 * Historie (oben ergänzen):
 * --------------------------------------------------------
 * 09.12.2016   VERSION 6: Dekomposition und API-Erweiterung für mehrere Turtles
 * 07.12.2016   VERSION 5: Methoden setPenColor, setBackground eingeführt (analog Structorizer 3.25-09)
 * 02.11.2016   VERSION 4: Methoden forward/backward getrennt von fd/bk, refresh() verbessert
 * 07.10.2016   VERSION 3: Neue Methode awaitClose() statt shutDown()
 * 30.04.2015	refresh()-Aufrufe eingeschränkt, um Performance zu verbessern
 * 29.09.2013	refresh() durch UpdateWindow()-Aufruf vervollständigt
 * 27.09.2013	makeFilePath() zum Laden des Turtle-Images
 * 25.09.2013	Flush in onPaint() ergänzt (für VS2012)
 * 20.09.2013	erstellt
 */

#define _USE_MATH_DEFINES
#include <cmath>
// Pre-caution for VS2012
#ifndef _MATH_DEFINES_DEFINED
#define M_PI 3.14159265358979323846
#endif /*_MATH_DEFINES_DEFINED*/

// Two-step conversion macro of string literals into wide-character string literals
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

const LPCWSTR Turtleizer::WCLASS_NAME = TEXT("Turtleizer");

const Color Turtleizer::colourTable[TC_VIOLET + 1] =
{
	Color(0, 0, 0),	// TC_BLACK
	Color(255, 0, 0),	// TC_RED
	Color(255, 255, 0),	// TC_YELLOW
	Color(0, 255, 0),	// TC_GREEN
	Color(0, 255, 255),	// TC_LIGHTBLUE
	Color(0, 0, 255),	// TC_BLUE
	Color(255, 0, 255),	// TC_MAGENTA
	Color(127, 127, 127),	// TC_GREY
	Color(255, 127, 0),	// TC_ORANGE
	Color(127, 0, 255)	// TC_VIOLETT
};

Turtleizer* Turtleizer::pInstance = NULL;

Turtleizer::Turtleizer(wstring caption, unsigned int sizeX, unsigned int sizeY, HINSTANCE hInstance)
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
void Turtleizer::startUp(unsigned int sizeX, unsigned int sizeY, HINSTANCE hInstance)
{
	const INT nCmdShow = SW_SHOWNORMAL;

	if (pInstance == NULL) {
		pInstance = new Turtleizer(WCLASS_NAME, sizeX, sizeY, hInstance);
		// ToDo set up the worker thread that is responding to the events
		pInstance->turtles.push_back(new Turtle(sizeX / 2, sizeY / 2));
	}
	ShowWindow(pInstance->hWnd, nCmdShow);
	UpdateWindow(pInstance->hWnd);

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


/////////////////// GLOBAL FUNCTIONS FOR SIMPLICITY ////////////////////
// None of them will work if the Turleizer hadn't been set up before  //
////////////////////////////////////////////////////////////////////////

// Make the turtle move the given number of pixels forward in real coordinates.
void forward(double pixels)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle != NULL) {
		pTurtle->forward(pixels);
	}
}

// Make the turtle move the given number of pixels forward.
void fd(int pixels)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle != NULL) {
		pTurtle->fd(pixels);
	}
}

// Make the turtle move the given number of pixels forward in real coordinates.
void forward(double pixels, Turtleizer::TurtleColour col)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle != NULL) {
		pTurtle->forward(pixels, col);
	}
}

// Make the turtle move the given number of pixels forward.
void fd(int pixels, Turtleizer::TurtleColour col)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle != NULL) {
		pTurtle->fd(pixels, col);
	}
}

// Rotates the turtle to the left by some angle (degrees!).
void left(double degrees)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle != NULL) {
		pTurtle->left(degrees);
	}
}

// Sets the turtle to the position (X,Y).
void gotoXY(int x, int y)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle != NULL) {
		pTurtle->gotoXY(x, y);
	}
}

// Sets the X-coordinate of the turtle's position to a new value.
void gotoX(int x)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle != NULL) {
		pTurtle->gotoX(x);
	}
}
// Sets the Y-coordinate of the turtle's position to a new value.
void gotoY(int y)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle != NULL) {
		pTurtle->gotoY(y);
	}
}

// The turtle lifts the pen up, so when moving no line will be drawn
void penUp()
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle != NULL) {
		pTurtle->penUp();
	}
}
// The turtle sets the pen down, so a line is being drawn when moving
void penDown()
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle != NULL) {
		pTurtle->penDown();
	}
}

// Hides the turtle
void hideTurtle()
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle != NULL) {
		pTurtle->showTurtle(false);
	}
}
// Show the turtle again
void showTurtle()
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle != NULL) {
		pTurtle->showTurtle(true);
	}
}

// Sets the Turtleizer background to the colour defined by the RGB values
void setBackground(unsigned char red, unsigned char green, unsigned char blue)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle != NULL) {
		pTurtle->setBackground(red, green, blue);
	}
}
// Sets the default pen colour (used for moves without color argument) to the RGB values
void setPenColor(unsigned char red, unsigned char green, unsigned char blue)
{
	Turtleizer* pTurtle = Turtleizer::getInstance();
	if (pTurtle != NULL) {
		pTurtle->setPenColor(red, green, blue);
	}
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
	Turtle* pTurtle = nullptr;
	Turtleizer* pTurtleizer = Turtleizer::getInstance();
	if (pTurtleizer != NULL) {
		pTurtle = pTurtleizer->addNewTurtle(x, y, imagePath);
	}
	return pTurtle;
}

