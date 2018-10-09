#pragma once

#ifndef TURTLEIZER_H
#define TURTLEIZER_H
/*
 * Fachhochschule Erfurt www.fh-erfurt.de/ai
 * Fachrichtung Angewandte Informatik
 * Module: Programming
 *
 * Object class approximately emulating the Turtleizer component of Structorizer
 * (http://structorizer.fisch.lu) for a simple C++  environment
 * Theme: Prep course Programming
 * Author: Kay Gürtzig
 * Version: 9
 *
 * Usage:
 * 1. Configure a link to the compiled library (Turtleizer.lib) in your (Console) application
 *    (e.g. by establishing a project dependency within the same Solution folder).
 * 2. Register the Turtleizer project folder as additional include directory
 * 3. Adhere to the following programming paragigm:
 *
 * #include "Turtleizer.h"
 * int main(int argc, char* argv)
 * {
 *     // From version 7 on, you may omit this line unless you want to define the initial window size
 *     Turtleizer::startUP(<width>, <height>);
 *
 *     // Essential programming code (e.g. as obtained from Turtleizer export to C++)
 *     // using the following functions:
 *     //		forward(distPixel) or fd(nPixels)
 *     //		backward(distPixel) or bk(nPixels)
 *     //		left(degrees) or rl(degrees)
 *     //		right(degrees) or rr(degrees)
 *     //		gotoXY(x, y), gotoX(x), gotoY(y)
 *     //		penUp(), penDown()
 *     //		hideTurtle(), showTurtle()
 *     //		setPenColor(r, g, b)
 *     //		setBackground(r, g, b)
 *     //		clear()
 *     //		getX(), getY()
 *     //		getOrientation()
 *     // The functions forward/fd und backward/bk may be equipped with a second
 *     // Argument of type Turtleizer::TurtleColour, i.e. with one of the constants
 *     // (where Turtleizer::TC_BLACK is the default):
 *     //		Turtleizer::TC_BLACK,
 *     //		Turtleizer::TC_RED,
 *     //		Turtleizer::TC_YELLOW,
 *     //		Turtleizer::TC_GREEN,
 *     //		Turtleizer::TC_CYAN,
 *     //		Turtleizer::TC_BLUE,
 *     //		Turtleizer::TC_MAGENTA,
 *     //		Turtleizer::TC_GREY,
 *     //		Turtleizer::TC_ORANGE,
 *     //		Turtleizer::TC_VIOLET
 *     // Be aware that the functions forward and fd are not exactly equivalent:
 *     // forward(...) works with an floating-point coordinate model, whereas
 *     // fd(...) adheres to a strict integer coordinate model which may rapidly
 *     // lead to noticeable biases, particularly with many short, traversal ways.
 *     // The same holds for the pair backward / bk, of course.
 *
 *     Turtleizer::awaitClose();
 *     return 0;
 * }
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
 * 2018-10-09   New turtle symbol according to Structorizer versions >= 3.28
 * 2018-07-30   VERSION 9: New function clear() added (according to Structorizer 3.28-07)
 * 2018-07-18   VERSION 8: Colour constant TC_LIGHTBLUE renamed to TC_CYAN but kept as alias for TC_CYAN
 * 2017-10-29   VERSION 7: API adaptation to Structorizer 3.27:
 *              New methods/functions getX(), getY(), geOrientation()
 *              adapter functions now call startUp themselves if not done
 *              Comments translated to English, exposed on GitHub
 * 2016-12-09   VERSION 6: Decomposition and API extension for multiple Turtles
 * 2016-12-07   VERSION 5: API adaptation to Structorizer 3.25-09: setPenColor, setBackground
 * 2016-11-02   VERSION 4: API adaptation to Structorizer 3.25-03: separating forward/fd,
 *              element-count-dependent update cycles introduced
 * 2016-10-07   VERSION 3: New method awaitClose() instead of shutDown()
 * 2015-05-30	VERSION 2: additional arguments in method refresh(),
 *				new method updateWindow(bool) and function updateTurtleWindow(bool),
 *				new attribute autoUpdate,
 *				new class constant VERSION
 * 2013-09-29	Accomplishment of comments
 * 2013-09-27	turtleImagePath, makeFilePath() added
 * 2013-09-25.	turtleHeight, turtleWidth added
 * 2013-09-20	initial version
 */

#include <windows.h>
#include <gdiplus.h>
#include <cstdio>
#include <list>
#include <string>
using namespace Gdiplus;
using std::wstring;
using std::list;

// Setting this define to 1 enables some debug printf instructions
#define DEBUG_PRINT 0
#include "Turtle.h"


// Singleton class providing a drawing window with a "turtle"
// that may be moved around producing lines in its wake
class Turtleizer
{
public:
	enum TurtleColour {
		TC_BLACK, TC_RED, TC_YELLOW, TC_GREEN, TC_CYAN,
		TC_LIGHTBLUE = TC_CYAN, TC_BLUE, TC_MAGENTA, TC_GREY, TC_ORANGE, TC_VIOLET
	};
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
						 WPARAM wParam, LPARAM lParam);
	static const unsigned int DEFAULT_WINDOWSIZE_X = 500;
	static const unsigned int DEFAULT_WINDOWSIZE_Y = 500;
	static const unsigned int VERSION = 9;
	~Turtleizer(void);
	// Initialises and starts a Turtleizer window
	static Turtleizer* startUp(unsigned int sizeX = DEFAULT_WINDOWSIZE_X, unsigned int sizeY = DEFAULT_WINDOWSIZE_Y, HINSTANCE hInstance = NULL);
	// Waits for someone closing the Turtleizer window and shuts Turtleizer down then
	static void awaitClose();
	// Deprecated API: Legacy synonym for awaitClose()
	static inline void shutDown() { awaitClose(); }
	// Returns the instance of the Turtleizer if there is any
	static Turtleizer* getInstance();
	// interactive thread - just waits for and reacts to user actions until closed
	static DWORD WINAPI interact(LPVOID lpParam);

	// Make the turtle move the given number of pixels forward (or backward if neg.) using pen colour.
	void forward(double pixels);
	// Make the turtle move the given number of pixels forward (or backward if neg.) using pen colour.
	void fd(int pixels);
	// Make the turtle move the given number of pixels forward (or backward if neg.) using given colour.
	void forward(double pixels, TurtleColour col);
	// Make the turtle move the given number of pixels forward (or backward if neg.) using given colour.
	void fd(int pixels, TurtleColour col);
	// Rotates the turtle to the left by some angle (degrees!).
	void left(double degrees);
	// Sets the turtle to the position (X,Y).
	void gotoXY(int x, int y);
	// Sets the X-coordinate of the turtle's position to a new value.
	void gotoX(int x);
	// Sets the Y-coordinate of the turtle's position to a new value.
	void gotoY(int y);
	// The turtle lifts the pen up, so when moving no line will be drawn
	void penUp();
	// The turtle sets the pen down, so a line is being drawn when moving
	void penDown();
	// Defines turtle visibility
	void showTurtle(bool show);
	// Sets the Turtleizer background to the colour defined by the RGB values
	void setBackground(unsigned char red, unsigned char green, unsigned char blue);
	// Sets the default pen colour (used for moves without color argument) to the RGB values
	void setPenColor(unsigned char red, unsigned char green, unsigned char blue);
	// Wipes all drawn content from the Turtleizer canvas (without moving the turtle)
	// Without argument, it will just forget the main turtle traces, wih argument true
	// the traces of all additional turtles are also erased.
	void clear(bool allTurtles = false);
	// Returns the current horizontal pixel position in floating-point resolution
	double getX() const;
	// Returns the current vertical pixel position in floating-point resolution
	double getY() const;
	// Returns the current orientation in degrees from North (clockwise = positive)
	double getOrientation() const;

	// Immediately updates the Turtleizer window i.e. refreshes all damaged regions.
	// After having been called with argument false, automatic updates after every
	// turtle movement will no longer be done, otherwise the turtle returns to the
	// standard behaviour to update the window after every movement.
	void updateWindow(bool automatic = true);
	// Refresh the window (i. e. invalidate the region rect) 
	void refresh(const RECT& rect, int nElements) const;

	// Creates and adds a new turtle symbolized by the the icon specifed by the given imagPath
	// at the given position to the Turtleizer
	Turtle* addNewTurtle(int x, int y, LPCWSTR imagePath = NULL);

private:
	// Typename for the list of tracked line elements
	typedef list<Turtle*> Turtles;
	static const LPCWSTR WCLASS_NAME;			// Name of the window class
	static const Color colourTable[TC_VIOLET + 1];	// Look-up table for colour codes
	static Turtleizer* pInstance;				// The singleton instance
	ULONG_PTR gdiplusToken;						// Token of the GDI+ session
	HWND hWnd;									// Window handle
	MSG msg;									// Message instance for user interaction
	WNDCLASS wndClass;							// Keeps the created window class
	GdiplusStartupInput gdiplusStartupInput;	// Structure needed for GdiplusStartup
	bool autoUpdate;						// Whether the window is to be updated on every movement
	Turtles turtles;						// List of turtles to be handled here
	Color backgroundColour;					// Current background colour
	// Hidden constructor - use Turtleizer::startUp() to create an instance!
	Turtleizer(wstring caption, unsigned int sizeX, unsigned int sizeY, HINSTANCE hInstance = NULL);
	// Callback method for refresh (OnPaint event)
	VOID onPaint(HDC hdc);
	// Listener method (parallel thread) FIXME: better static?
	void listen();
};

/////////////////// GLOBAL FUNCTIONS FOR SIMPLICITY ////////////////////
// None of them will work if the Turleizer hadn't been set up before  //
////////////////////////////////////////////////////////////////////////

// Make the turtle move the given number of pixels forward in real coordinates.
void forward(double pixels);
// Make the turtle move the given number of pixels forward in real coordinates.
void forward(double pixels, Turtleizer::TurtleColour col);
// Make the turtle move the given number of pixels forward.
void fd(int pixels);
// Make the turtle move the given number of pixels forward.
void fd(int pixels, Turtleizer::TurtleColour col);

// Make the turtle move the given number of pixels backward in real coordinates.
inline void backward(double pixels)
{ forward(-pixels); }
// Make the turtle move the given number of pixels backward in real coordinates.
inline void backward(double pixels, Turtleizer::TurtleColour col)
{ forward(-pixels, col); }
// Make the turtle move the given number of pixels backward.
inline void bk(int pixels)
{ fd(-pixels); }
// Make the turtle move the given number of pixels backward.
inline void bk(int pixels, Turtleizer::TurtleColour col)
{ fd(-pixels, col); }

// Rotates the turtle to the left by some angle (degrees!).
void left(double degrees);
// Rotates the turtle to the left by some angle (degrees!).
inline void rl(double degrees) { left(degrees); }
// Rotates the turtle to the right by some angle (degrees!).
inline void right(double degrees) { left(-degrees); }
// Rotates the turtle to the right by some angle (degrees!).
inline void rr(double degrees) { left(-degrees); }

// Sets the turtle to the position (X,Y).
void gotoXY(int x, int y);
// Sets the X-coordinate of the turtle's position to a new value.
void gotoX(int x);
// Sets the Y-coordinate of the turtle's position to a new value.
void gotoY(int y);

// The turtle lifts the pen up, so when moving no line will be drawn
void penUp();
// The turtle sets the pen down, so a line is being drawn when moving
void penDown();

// Hides the turtle
void hideTurtle();
// Show the turtle again
void showTurtle();

// Sets the Turtleizer background to the colour defined by the RGB values
void setBackground(unsigned char red, unsigned char green, unsigned char blue);
// Sets the default pen colour (used for moves without color argument) to the RGB values
void setPenColor(unsigned char red, unsigned char green, unsigned char blue);

// Wipes all drawn content from the Turtleizer canvas (without moving the turtle)
// Without argument, it will just forget the main turtle traces, wih argument true
// the traces of all additional turtles are also erased.
void clear(bool allTurtles = false);

// Returns the current horizontal pixel position in floating-point resolution
double getX();
// Returns the current vertical pixel position in floating-point resolution
double getY();
// Returns the current orientation in degrees from North (clockwise = positive)
double getOrientation();

// Immediately updates the Turtleizer window i.e. refreshes all damaged regions.
// After having been called with argument false, automatic updates after every
// turtle movement will no longer be done, otherwise the turtle returns to the
// standard behaviour to update the window after every movement.
void updateTurtleWindow(bool automatic = true);

// Creates and adds a new turtle symbolized by the the icon specifed by the given imagPath
// at the given position to the Turtleizer
Turtle* addNewTurtle(int x, int y, LPCWSTR imagePath = NULL);


#endif /*TURTLEIZER_H*/
