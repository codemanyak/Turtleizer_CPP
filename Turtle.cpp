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
 * Version: 9 (covering capabilities of Structorizer 3.28-07)
 *
 * History (app at top):
 * --------------------------------------------------------
 * 2018-07-30   VERSION 9: API adaptation to Structorizer 3.28-07: clear() procedure
 * 2017-10-29   New methods getX(), getY(), getOrientation() implemented
 * 2016-12-09   Created für VERSION 6
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstring>
#include "Turtle.h"
#include "Turtleizer.h"

// Two-step conversion macro of string literals into wide-character string literals
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

const LPCWSTR Turtle::TURTLE_IMAGE_FILE = TEXT("turtle.png");

Turtle::Turtle(int x, int y, LPCWSTR imagePath)
: turtleImagePath(NULL)
, turtleWidth(35)	// Just some default
, turtleHeight(35)	// Just some default
, pos(x, y)
, penIsDown(true)
, isVisible(true)
, orient(0.0)
, defaultColour(Color::Black)
, pTurtleizer(Turtleizer::getInstance())
{
	if (imagePath != NULL) {
		this->turtleImagePath = this->makeFilePath(imagePath, false);
	}
	else {
		this->turtleImagePath = this->makeFilePath();
	}
	// Store the size of the turtle symbol
	Image* image = new Image(this->turtleImagePath);
	if (image != NULL) {
		this->turtleWidth = image->GetWidth();
		this->turtleHeight = image->GetHeight();
		delete image;
	}
}

Turtle::~Turtle()
{
	delete this->turtleImagePath;
}

// Make the turtle move the given number of pixels forward.
void Turtle::forward(double pixels)
{
	this->forward(pixels, this->defaultColour);
}

// Make the turtle move the given number of pixels forward.
void Turtle::forward(double pixels, Color col)
{
	// FIXME: correct the angle
	PointF oldP(this->pos);
	double angle = M_PI * (90 + this->orient) / 180.0;
	this->pos.X += (REAL)(pixels * cos(angle));
	this->pos.Y -= (REAL)(pixels * sin(angle));
	if (this->penIsDown) {
		this->elements.push_back(TurtleLine(oldP.X, oldP.Y, this->pos.X, this->pos.Y, col));
	}
	this->refresh(oldP);
}

// Make the turtle move the given number of pixels forward.
void Turtle::fd(int pixels)
{
	this->fd(pixels, this->defaultColour);
}

// Make the turtle move the given number of pixels forward.
void Turtle::fd(int pixels, Color col)
{
	// FIXME: correct the angle
	this->pos.X = round(this->pos.X);
	this->pos.Y = round(this->pos.Y);
	PointF oldP(this->pos);
	double angle = M_PI * (90 + this->orient) / 180.0;
	this->pos.X += (REAL)round(pixels * cos(angle));
	this->pos.Y -= (REAL)round(pixels * sin(angle));
	if (this->penIsDown) {
		this->elements.push_back(TurtleLine(oldP.X, oldP.Y, this->pos.X, this->pos.Y, col));
	}
	this->refresh(oldP);
}

// Rotates the turtle to the left by some angle (degrees!).
void Turtle::left(double degrees)
{
	// FIXME: Normalise angle
	this->orient += degrees;
	// TO DO: Trigger damage
	if (this->isVisible) {
		this->refresh(this->pos);
	}
}

// Sets the turtle to the position (X,Y).
void Turtle::gotoXY(int x, int y)
{
	PointF oldP(this->pos);
	if (this->isVisible) {
		// If necessary, clear the turtle symbol and restore the drawing behind
		this->showTurtle(false);
		this->isVisible = true;
	}
	this->pos.X = (REAL)x;
	this->pos.Y = (REAL)y;
	if (this->isVisible) {
		this->refresh(this->pos);
	}
}

// Sets the X-coordinate of the turtle's position to a new value.
void Turtle::gotoX(int x)
{
	this->gotoXY(x, this->pos.Y);
}

// Sets the Y-coordinate of the turtle's position to a new value.
void Turtle::gotoY(int y)
{
	this->gotoXY(this->pos.X, y);
}

// The turtle lifts the pen up, so when moving no line will be drawn
void Turtle::penUp()
{
	this->penIsDown = false;
}

// The turtle sets the pen down, so a line is being drawn when moving
void Turtle::penDown()
{
	this->penIsDown = true;
}

// Show the turtle again
void Turtle::showTurtle(bool show)
{
	bool doRefresh = this->isVisible != show;
	this->isVisible = show;
	if (doRefresh) {
		this->refresh(this->pos, true);
	}
}

// Sets the default pen colour (used for moves without color argument) to the RGB values
void Turtle::setPenColor(unsigned char red, unsigned char green, unsigned char blue)
{
	this->defaultColour = Color(red, green, blue);
}

// Wipes all drawn content of this turtle
void Turtle::clear()
{
	this->elements.clear();
	this->refresh(this->pos);
}

// Returns the current horizontal pixel position in floating-point resolution
double Turtle::getX() const
{
	return (double) this->pos.X;
}

// Returns the current vertical pixel position in floating-point resolution
double Turtle::getY() const
{
	return (double) this->pos.Y;
}

// Returns the current orientation in degrees from North (clockwise = positive)
double Turtle::getOrientation() const
{
	// TODO: Test the correct results
	double ori = this->orient;
	while (ori > 180) { ori -= 360; }
	while (ori < -180) { ori += 360; }
	return -ori;
}


// Refreshes the window i.e. invalidates the region between `oldPos´
// and this->pos and then updates the window
void Turtle::refresh(const PointF& oldPos, bool forceIconSize) const
{
	// Consider rotation, so use maximum diagonal
	LONG halfIconSize = (forceIconSize || this->isVisible) ? (max(this->turtleHeight, this->turtleWidth) / sqrt(2.0) +1) : 1;
	RECT rect;
	rect.left = (LONG)floor(min(oldPos.X, this->pos.X)) - halfIconSize;
	rect.right = (LONG)ceil(max(oldPos.X, this->pos.X)) + halfIconSize;
	rect.top = (LONG)floor(min(oldPos.Y, this->pos.Y)) - halfIconSize;
	rect.bottom = (LONG)ceil(max(oldPos.Y, this->pos.Y)) + halfIconSize;
	this->pTurtleizer->refresh(rect, this->elements.size());
}

// Composes a file path from the path of this source file (project
// folder) and the given file name.
// (if the image file name isn't given, the turtle image will be used
LPCWSTR Turtle::makeFilePath(LPCWSTR filename, bool addProductPath) const
{
	WCHAR delimiter = L'/';
	LPCWSTR pMyPath = __WFILE__;
	LPCWSTR pPosSlash = wcsrchr(pMyPath, L'/');
	LPCWSTR pPosBSlash = wcsrchr(pMyPath, L'\\');
	if (pPosSlash < pPosBSlash) {
		pPosSlash = pPosBSlash;
		delimiter = L'\\';
	}
	size_t pathLen = 0;
	if (addProductPath || filename == nullptr) {
		pathLen = (pPosSlash != NULL) ? pPosSlash - pMyPath : wcslen(pMyPath);
	}
	size_t buffLen = pathLen + wcslen(filename) + 2;
	WCHAR* pFilePath = new WCHAR[buffLen];
	if (addProductPath || filename == nullptr) {
		wcsncpy_s(pFilePath, buffLen, pMyPath, pathLen);
		pFilePath[pathLen++] = delimiter;
	}
	wcscpy_s(&pFilePath[pathLen], buffLen - pathLen, filename);
	return pFilePath;
}

void Turtle::draw(Graphics& gr) const
{
	for (Elements::const_iterator it(this->elements.cbegin()); it != this->elements.cend(); ++it)
	{
		it->draw(gr);
	}

	// Draw a text
	//SolidBrush  brush(Color(255, 0, 0, 255));
	//FontFamily  fontFamily(L"Times New Roman");
	//Font        font(&fontFamily, 24, FontStyleRegular, UnitPixel);
	//PointF      pointF(10.0f, 20.0f);
	//graphics.DrawString(L"I forced this damned thing!", -1, &font, pointF, &brush);

	if (this->isVisible) {
		Gdiplus::REAL matrix[6];
		// Display an image
		//Image* image = new Image(L"Turtle.png");
		Image* image = new Image(this->turtleImagePath);
		PointF pointF(-(REAL)this->turtleWidth / 2.0, -(REAL)this->turtleHeight / 2.0);
#if DEBUG_PRINT
		printf("The width of the image is %u.\n", this->turtleWidth);
		printf("The height of the image is %u.\n", this->turtleHeight);
#endif /*DEBUG_PRINT*/
		gr.TranslateTransform(this->pos.X, this->pos.Y);
		gr.RotateTransform(-this->orient);

		Matrix transf;
		gr.GetTransform(&transf);
		Gdiplus::Status status = transf.GetElements(matrix);

		gr.DrawImage(image, pointF);
		gr.ResetTransform();
		gr.Flush();
		delete image;
	}

}

Turtle::TurtleLine::TurtleLine(REAL x1, REAL y1, REAL x2, REAL y2, Color col)
: x1(x1)
, y1(y1)
, x2(x2)
, y2(y2)
, col(col)
{
}

void Turtle::TurtleLine::draw(Gdiplus::Graphics& gr) const
{
	// Draw a line
	Pen pen(this->col);
	gr.DrawLine(&pen, this->x1, this->y1, this->x2, this->y2);
}

