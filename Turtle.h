#pragma once
#ifndef TURTLE_H
#define TURTLE_H
/*
 * Fachhochschule Erfurt www.fh-erfurt.de/ai
 * Fachgebiet Angewandte Informatik
 * Modul Programmierung
 *
 * Object class, representing one (of potentially many) Turtles withing the
 * simple C++ emulation of the Turtleizer module coming with Structorizer
 * (http://structorizer.fisch.lu).
 * The intention is that several separately controllable (and subclassible)
 * Turtle objects may be created to share the drawing area.
 *
 * Theme: Prep course Programming Fundamentals / Object-oriented Programming
 * Author: Kay Gürtzig
 * Version: 7 (covering capabilities of Structorizer 3.27)
 *
 * History (add on top):
 * --------------------------------------------------------
 * 2017-10-29	new functions getX(), getY(), and getOrientation()
 * 2016-12-09	created
 */

#include <Windows.h>
#include <gdiplus.h>
#include <list>
using namespace Gdiplus;
using std::list;

class Turtleizer;

class Turtle
{
public:
	// Internal class tracking a line drawn by the turtle (for onPaint())
	class TurtleLine {
	public:
		friend class Turtle;
		void draw(Graphics& gr) const;
	private:
		TurtleLine(REAL x1, REAL y1, REAL x2, REAL y2, Color col);
		REAL x1, y1;	// from position
		REAL x2, y2;	// to position
		Color col;		// colour to draw with
	};

	Turtle(int x, int y, LPCWSTR imagePath = NULL);
	virtual ~Turtle();

	// Make the turtle move the given number of pixels forward (or backward if neg.) using pen colour.
	void forward(double pixels);
	// Make the turtle move the given number of pixels forward (or backward if neg.) using pen colour.
	void fd(int pixels);
	// Make the turtle move the given number of pixels forward (or backward if neg.) using given colour.
	void forward(double pixels, Color col);
	// Make the turtle move the given number of pixels forward (or backward if neg.) using given colour.
	void fd(int pixels, Color col);
	// Make the turtle move the given number of pixels forward (or backward if neg.) using pen colour.
	inline void backward(double pixels) { forward(-pixels); }
	// Make the turtle move the given number of pixels forward (or backward if neg.) using pen colour.
	inline void bk(int pixels) { fd(-pixels); }
	// Make the turtle move the given number of pixels forward (or backward if neg.) using given colour.
	inline void backward(double pixels, Color col) { forward(-pixels, col); }
	// Make the turtle move the given number of pixels forward (or backward if neg.) using given colour.
	inline void bk(int pixels, Color col) { fd(-pixels, col); }
	// Rotates the turtle to the left by some angle (degrees!).
	void left(double degrees);
	// Rotates the turtle to the left by some angle (degrees!).
	inline void right(double degrees) { left(-degrees); }
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
	// Sets the default pen colour (used for moves without color argument) to the RGB values
	void setPenColor(unsigned char red, unsigned char green, unsigned char blue);
	// Draws this turtle and its trayectory in 2D graphics gr
	void draw(Graphics& gr) const;
	// Returns the current horizontal pixel position in floating-point resolution
	double getX() const;
	// Returns the current vertical pixel position in floating-point resolution
	double getY() const;
	// Returns the current orientation in degrees from North (clockwise = positive)
	double getOrientation() const;

protected:
	// Typename for the list of tracked line elements
	typedef list<TurtleLine> Elements;
private:
	static const LPCWSTR TURTLE_IMAGE_FILE;		// File name of the turtle image
	const Turtleizer* pTurtleizer;				// The singleton Turtleizer instance
	LPCWSTR	turtleImagePath;					// The derived turtle file path
	UINT turtleWidth, turtleHeight;				// The turtle image extensions
	PointF pos;								// current turtle position
	double orient;							// current orientation in degrees
	bool penIsDown;							// Whether the pen is ready to draw
	bool isVisible;							// Whether the turtle itself ought to be visible
	Elements elements;						// List of lines drawn in this session
	Color defaultColour;					// Default colour for line segements withot explicit colour

	// Composes a file path from the path of this source file (project
	// folder) if and the given file name `filename´.
	// (if the image file name isn't given, the turtle image will be used)
	LPCWSTR makeFilePath(LPCWSTR filename = TURTLE_IMAGE_FILE, bool addProductPath = true) const;

protected:
	// Refresh the window (i. e. invalidate the region between oldPos and this->pos) 
	// If forceIconSize is set true, then the damaged region will be enlarged to include the
	// turtle symbol no matter if turtle is visible
	virtual void refresh(const PointF& oldPos, bool forceIconSize = false) const;

};

#endif /*TURTLE_H*/