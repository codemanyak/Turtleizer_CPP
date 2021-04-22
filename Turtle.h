#pragma once
#ifndef TURTLE_H
#define TURTLE_H
/*
 * Fachhochschule Erfurt https://ai.fh-erfurt.de
 * Fachrichtung Angewandte Informatik
 * Project: Turtleizer_CPP (static C++ library for Windows)
 *
 * Object class, representing one Turtle (of potentially many) within the
 * simple C++ emulation of the Turtleizer module coming with Structorizer
 * (http://structorizer.fisch.lu).
 * The intention is that several separately controllable (and subclassible)
 * Turtle objects may be created to share the drawing area.
 *
 * Author: Kay Gürtzig
 * Version: 11.0.0 (covering capabilities of Structorizer 3.31, functional GUI)
 *
 * History (add on top):
 * --------------------------------------------------------
 * 2021-04-07	VERSION 11.0.0: Enh. 6 - method writeElementsToCSV added
 * 2021-04-06   VERSION 11.0.0: Method draw decomposed to support memory HDC / bitblt
 * 2021-04-05	VERSION 11.0.0: New method for SVG export, nearest point search
 * 2021-04-02	VERSION 11.0.0: New methods isTurtleShown() and getBounds() for #6,
 *				signature of method refresh modified
 * 2018-07-30	VERSION 9: API adaptation to Structorizer 3.28-07: clear() procedure
 * 2017-10-29	new functions getX(), getY(), and getOrientation()
 * 2016-12-09	created
 */

#include <Windows.h>
#include <gdiplus.h>
#include <list>
#include <ostream>
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
		inline PointF getFrom() const { return PointF(x1, y1); }
		inline PointF getTo() const { return PointF(x2, y2); }
		inline Color getColor() const { return col; }
	private:
		TurtleLine(REAL x1, REAL y1, REAL x2, REAL y2, Color col);
		REAL x1, y1;	// from position
		REAL x2, y2;	// to position
		Color col;		// colour to draw with
		/* Identifies the nearest end point or point on line to the given
		 * coordinate pt, returns its distance or -1 and puts its coordinates into
		 * point nearest. */
		REAL getNearestPoint(const PointF& pt, bool betweenEnds, PointF& nearest) const;
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
	// Wipes all drawn content of this turtle
	void clear();

	// Returns the current horizontal pixel position in floating-point resolution
	double getX() const;
	// Returns the current vertical pixel position in floating-point resolution
	double getY() const;
	// Returns the current orientation in degrees from North (clockwise = positive)
	double getOrientation() const;

	// Returns true if the turtle visibiity is on
	bool isTurtleShown() const;
	// Returns the current drawing bounds of this turtle
	RectF getBounds() const;
	/* Searches the nearest end point or point on line within the given radius to
	 * given coordinate, returns its distance or -1 and puts its coordinates into
	 * point nearest (if such a point was found). (The result may be ambiguous.)
	 */
	REAL getNearestPoint(const PointF& coord, bool betweenEnds, double radius, PointF& nearest) const;

	// Draws the trayectory of this turtle (and possibly the turtle itself) in 2D graphics gr
	void draw(Graphics& gr, bool drawAll = true, bool withImage = true);
	// Draws this turtle (if visible) in 2D graphics gr
	void drawImage(Graphics& gr) const;
	// Reports whether this turtle has drawn elements
	bool hasElements() const;
	// Writes SVG descriptions of the elements to the given stream
	void writeSVG(std::ostream& ostr, PointF offset, unsigned short scale = 1) const;
	// Writes the CSV information of all gathered line elements to the given stream ostr
	void writeCSV(std::ostream& ostr, char separator) const;

protected:
	// Type name for the list of tracked line elements
	typedef list<TurtleLine> Elements;
private:
	static const int MAX_POINTS_PER_SVG_PATH = 800;
	static const LPCWSTR TURTLE_IMAGE_FILE;		// File name of the turtle image
	Turtleizer* const pTurtleizer;				// The singleton Turtleizer instance
	LPCWSTR	turtleImagePath;					// The derived turtle file path
	UINT turtleWidth, turtleHeight;				// The turtle image extensions
	Gdiplus::PointF pos;						// current turtle position
	Gdiplus::RectF bounds;						// current bounds of the trayectory
	double orient;							// current orientation in degrees
	Elements elements;						// List of lines drawn in this session
	Color defaultColour;					// Default colour for line segments without explicit colour
	Elements::const_iterator lastDrawn;		// Iterator to the last drawn element
	unsigned int nDrawn;					// Number of drawn elements so far
	bool penIsDown;							// Whether the pen is ready to draw
	bool isVisible;							// Whether the turtle itself ought to be visible

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