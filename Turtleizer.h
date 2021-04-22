#pragma once

#ifndef TURTLEIZER_H
#define TURTLEIZER_H
/*
 * Fachhochschule Erfurt https://ai.fh-erfurt.de
 * Fachrichtung Angewandte Informatik
 * Project: Turtleizer_CPP (static C++ library for Windows)
 *
 * Object class approximately emulating the Turtleizer component of Structorizer
 * (http://structorizer.fisch.lu) for a simple C++ environment on Windows (WinAPI)
 *
 * Author: Kay Gürtzig
 * Version: 11.0.0
 *
 * Usage:
 * 1. Configure a link to the compiled library (Turtleizer.lib) in your (Console) application
 *    (e.g. by establishing a project dependency within the same Solution folder).
 * 2. Register the Turtleizer project folder as additional include directory
 * 3. Adhere to the following programming paradigm:
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
 *     // argument of type Turtleizer::TurtleColour, i.e. with one of the constants
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
 *     // forward(...) works with a floating-point coordinate model, whereas
 *     // fd(...) adheres to a strict integer coordinate model which may rapidly
 *     // lead to noticeable biases, particularly with many short, traversal ways.
 *     // The same holds for the pair backward / bk, of course.
 *
 *     Turtleizer::awaitClose();
 *     return 0;
 * }
 *
 * The automatic update of the drawing area is initially done after each drawing step,
 * but will then be done ever less frequently with the growing number of elements (traces)
 * to be rendered. (To filter the elements by the damaged region does hardly cost less
 * efforts than to draw them rightaway. It already helped a lot to invalidate only the
 * affected region but was not sufficient.)
 * By invoking updateWindow(false) the regular update may be suppressed entirely. By
 * using updateWindow(true) you may re-enable the regular update.
 * BOTH calls induce an immediate window update.
 * 
 * Since version 11.0.0, the window has several GUI elements to allow zooming, scrolling,
 * measuring and picture export. The functions are available via a context menu and accel-
 * erator keys. A status bar is showing the main turtle's home and current position, the
 * extensions of the reachable part of the drawing and the scroll range, as well the
 * current zoom factor and the snap mode for measuring (either to lines or only to start
 * and end points of lines).
 *
 * History (add at top):
 * --------------------------------------------------------
 * 2021-04-21   VERSION 11.0.0: GUI extensions according to #6 (~ Structorizer 3.31)
 * 2019-07-02   VERSION 10.0.1: Fixed #1 (environment-dependent char array type), #2
 * 2018-10-23   VERSION 10.0.0: Now semantic version numbering with Version class.
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
 * 2013-09-25 	turtleHeight, turtleWidth added
 * 2013-09-20	initial version
 */
// START KGU 2021-04-05: Added for #6 in order to enable tracking tooltips (for measuring)
// (TODO: Keep this up to date if necessary!)
#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
// END KGU 2021-04-05
#include <windows.h>
#include <gdiplus.h>
#include <commctrl.h>
#include <cstdio>
#include <list>
#include <string>
using namespace Gdiplus;
using std::string;
using std::wstring;
using std::list;

// Setting this define to 1 enables some debug printf instructions
#define DEBUG_PRINT 0
#include "Turtle.h"
#include "TurtleCanvas.h"

// Singleton class providing a drawing window with a "turtle"
// that may be moved around producing lines in its wake
class Turtleizer
{
public:
	friend class TurtleCanvas;
	enum TurtleColour {
		TC_BLACK, TC_RED, TC_YELLOW, TC_GREEN, TC_CYAN,
		TC_LIGHTBLUE = TC_CYAN, TC_BLUE, TC_MAGENTA, TC_GREY, TC_ORANGE, TC_VIOLET
	};
	class Version {
	public:
		Version(unsigned short major, unsigned short minor = 0, unsigned short bugfix = 0);
		operator string() const;
		bool operator<(const Version other) const;
		bool operator==(const Version other) const;
		bool operator!=(const Version other) const;
		bool operator<=(const Version other) const;
		bool operator>(const Version other) const;
		bool operator>=(const Version other) const;
	private:
		static const unsigned short N_LEVELS = 3;
		unsigned short levels[N_LEVELS];
	};
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
						 WPARAM wParam, LPARAM lParam);
	static const unsigned int DEFAULT_WINDOWSIZE_X = 500;
	static const unsigned int DEFAULT_WINDOWSIZE_Y = 500;
	static const Version VERSION;
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
	void refresh(const RectF& rect, int nElements);
	// Returns tre if the entire drawing are must be completely redrawn (beacuse e.g.
	// the window background has changed or some turtle cleared its trayectory
	bool isDirty() const;

	// Creates and adds a new turtle symbolized by the the icon specifed by the given imagPath
	// at the given position to the Turtleizer
	Turtle* addNewTurtle(int x, int y, LPCWSTR imagePath = NULL);

private:
	// Typename for the list of tracked line elements
	typedef list<Turtle*> Turtles;
#ifdef UNICODE
	typedef LPCWSTR NameType;
	typedef wstring String;
#else
	typedef LPCSTR NameType;
	typedef string String;
#endif /*UNICODE*/
	static const NameType WCLASS_NAME;			// Name of the window class
	static const Color colourTable[TC_VIOLET + 1];	// Look-up table for colour codes
	static const int IDS_STATUSBAR = 21000;		// Identifier for the status bar
	static const int STATUSBAR_ICON_IDS[];		// Ids of status bar part icons (also specifying the part count)
	static Turtleizer* pInstance;				// The singleton instance
	ULONG_PTR gdiplusToken;						// Token of the GDI+ session
	HWND hWnd;									// Window handle
	// START KGU 2021-03-28: Enh. #6 GUI extensions
	TurtleCanvas* pCanvas;						// Pointer to the drawing canvas object
	int* statusbarPartWidths;					// Array of statusbar part text widths
	HWND hStatusbar;							// Status bar handle
	// END KGU 2021-03-28
	MSG msg;									// Message instance for user interaction
	GdiplusStartupInput gdiplusStartupInput;	// Structure needed for GdiplusStartup
	Turtles turtles;						// List of turtles to be handled here
	Color backgroundColour;					// Current background colour
	Point home0;							// Home position of the standard turtle
	bool showStatusbar;						// Visibility of the statusbar
	// Hidden constructor - use Turtleizer::startUp() to create an instance!
	Turtleizer(String caption, unsigned int sizeX, unsigned int sizeY, HINSTANCE hInstance = NULL);
	// Callback method for refresh (OnPaint event)
	VOID onPaint(HDC hdc);
	// START KGU 2021-03-28: Enh. #6 GUI extensions
	// Updates the information on the status bar
	void updateStatusbar();
	// Sets up several window decorations like statusbar, which might require pInstance to be set
	void setupWindowAddons(HINSTANCE hInstance);
	// Retrieves the combined bounds of all turtles
	RectF getBounds() const;
	// END KGU 2021-03-28
	// START KGU 2021-03-31: Issue #6
	// Specifies the effective client area (without statusbar etc.)
	void getClientRect(RECT& rcClient) const;
	// Modifies coord to that of the nearest line point or bend within given radius
	bool snapToNearestPoint(PointF& coord, bool onLines, REAL radius) const;
	// END KGU 2021-03-31
	// Listener method (parallel thread) FIXME: better static?
	void listen();
	// Composes a file path from the path of this source file (project
	// folder) if and the given file name `filename´.
	// (if the image file name isn't given, the turtle image will be used)
	LPCWSTR getAbsolutePath(LPCWSTR filename) const;
};

/////////////////////// GLOBAL FUNCTIONS FOR SIMPLICITY ///////////////////////
// Calling any of these functions starts up the Turtleizer if it hadn't been //
///////////////////////////////////////////////////////////////////////////////

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
