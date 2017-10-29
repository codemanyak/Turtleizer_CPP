#pragma once

#ifndef TURTLEIZER_H
#define TURTLEIZER_H
/*
 * Fachhochschule Erfurt www.fh-erfurt.de/ai
 * Fachgebiet Angewandte Informatik
 * Modul Programmierung
 *
 * Objektklasse, die den Turtleizer des Structorizers
 * (http://structorizer.fisch.lu) ansatzweise für eine
 * einfache C-Umgebung nachbildet
 * Thema: Brückenkurs Programmierung
 * Autor: Kay Gürtzig
 * Version: 6
 *
 * Nutzung:
 * 1. Bibliothek in (Konsolen-)Anwendung einbinden (z. B. durch
 *    Einrichten eines Verweises innerhalb derselben Projektmappe).
 * 2. Registrieren des Turtleizer-Projektverzeichnisses als zusätzliches Include-Verzeichnisses
 * 3. Verwenden folgender grundsätzlicher Programmstruktur:
 *
 * #include "Turtleizer.h"
 * int main(int argc, char* argv)
 * {
 *     Turtleizer::startUP();
 *
 *     // Eigentlicher Programmcode wie z. B. aus dem Turtleizer exportiert
 *     // und unter Verwendung der Funktionen:
 *     //		forward(nPixel) oder fd(nPixel)
 *     //		backward(nPixel) oder bk(nPixel)
 *     //		left(degrees) oder rl(degrees)
 *     //		right(degrees) oder rr(degrees)
 *     //		gotoXY(x, y), gotoX(x), gotoY(y)
 *     //		penUp(), penDown()
 *     //		hideTurtle(), showTurtle()
 *     // Die Funktionen forward/fd und backward/bk können mit einem zweiten
 *     // Parameter vom Typ Turtleizer::TurtleColour ausgestattet werden, also
 *     // einem der folgenden Werte (wobei Turtleizer::TC_BLACK der Default ist):
 *     //		Turtleizer::TC_BLACK,
 *     //		Turtleizer::TC_RED,
 *     //		Turtleizer::TC_YELLOW,
 *     //		Turtleizer::TC_GREEN,
 *     //		Turtleizer::TC_LIGHTBLUE,
 *     //		Turtleizer::TC_BLUE,
 *     //		Turtleizer::TC_MAGENTA,
 *     //		Turtleizer::TC_GREY,
 *     //		Turtleizer::TC_ORANGE,
 *     //		Turtleizer::TC_VIOLET
 *
 *     Turtleizer::awaitClose();
 *     return 0;
 * }
 *
 * Das automatische Window-Update erfolgt anfangs nach jedem Zeichenschritt,
 * wird dann aber zwecks Beschleunigung mit wachsender Zahl der schon gezeichneten
 * Elemente seltener.
 * Mittels updateWindow(false) kann es ganz unterdrückt werden. Durch updateWindow(true)
 * wird es wieder erlaubt. Beide Aufrufe führen ein sofortiges Window-Update aus.
 *
 * Historie (oben ergänzen):
 * --------------------------------------------------------
 * 09.12.2016   VERSION 6: Dekomposition und API-Erweiterung für mehrere Turtles
 * 07.12.2016   VERSION 5: API-Anpassung an Structorizer 3.25-09: setPenColor, setBackground
 * 02.11.2016   VERSION 4: API-Anpassung an Structorizer 3.25-03: Trennung forward/fd,
 *              Elementanzahlabhängige update-Zyklen eingeführt
 * 07.10.2016   VERSION 3: Neue Methode awaitClose() statt shutDown()
 * 30.05.2015	VERSION 2: Zusatzparameter in Methode refresh(),
 *				neue Methode updateWindow(bool) und	Funktion updateTurtleWindow(bool),
 *				neues Attribut autoUpdate,
 *				neue Klassenkonstante VERSION
 * 29.09.2013	Vollständigung der Kommentierung
 * 27.09.2013	turtleImagePath, makeFilePath() ergänzt
 * 25.09.2013	turtleHeight, turtleWidth ergänzt
 * 20.09.2013	erstellt
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
		TC_BLACK, TC_RED, TC_YELLOW, TC_GREEN,
		TC_LIGHTBLUE, TC_BLUE, TC_MAGENTA, TC_GREY, TC_ORANGE, TC_VIOLET
	};
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
						 WPARAM wParam, LPARAM lParam);
	static const unsigned int DEFAULT_WINDOWSIZE_X = 500;
	static const unsigned int DEFAULT_WINDOWSIZE_Y = 500;
	static const unsigned int VERSION = 6;
	~Turtleizer(void);
	// Initialises and starts a Turtleizer window
	static void startUp(unsigned int sizeX = DEFAULT_WINDOWSIZE_X, unsigned int sizeY = DEFAULT_WINDOWSIZE_Y, HINSTANCE hInstance = NULL);
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

// Immediately updates the Turtleizer window i.e. refreshes all damaged regions.
// After having been called with argument false, automatic updates after every
// turtle movement will no longer be done, otherwise the turtle returns to the
// standard behaviour to update the window after every movement.
void updateTurtleWindow(bool automatic = true);

// Creates and adds a new turtle symbolized by the the icon specifed by the given imagPath
// at the given position to the Turtleizer
Turtle* addNewTurtle(int x, int y, LPCWSTR imagePath = NULL);


#endif /*TURTLEIZER_H*/