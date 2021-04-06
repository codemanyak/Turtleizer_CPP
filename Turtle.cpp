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
 * Version: 11.0.0 (covering capabilities of Structorizer 3.31, functional GUI)
 *
 * History (add at top):
 * --------------------------------------------------------
 * 2021-04-05   VERSION 11.0.0: New method for SVG export, nearest point search
 * 2021-04-02   VERSION 11.0.0: Enh. #6 (tracking of the bounds and new internal methods)
 * 2019-07-08   VERSION 10.0.1: Fixed #1 (environment-dependent char array type), #2, #3
 * 2018-10-23   VERSION 10.0.0: Casts added to avoid compiler warnings.
 * 2018-07-30   VERSION 9: API adaptation to Structorizer 3.28-07: clear() procedure
 * 2017-10-29   New methods getX(), getY(), getOrientation() implemented
 * 2016-12-09   Created for VERSION 6
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstring>
#include <cassert>
#include <iomanip>
#include "Turtle.h"
#include "Turtleizer.h"

// Two-step conversion macro of string literals into wide-character string literals
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

const LPCWSTR Turtle::TURTLE_IMAGE_FILE = WIDEN("turtle.png");

Turtle::Turtle(int x, int y, LPCWSTR imagePath)
	: turtleImagePath(nullptr)
	, turtleWidth(35)	// Just some default
	, turtleHeight(35)	// Just some default
	, pos((REAL)x, (REAL)y)
	, bounds((REAL)x, (REAL)y, (REAL)1, (REAL)1)
	, penIsDown(true)
	, isVisible(true)
	, orient(0.0)
	, defaultColour(Color::Black)
	, pTurtleizer(Turtleizer::getInstance())
	, lastDrawn(elements.cend())
	, nDrawn(0)
{
	if (imagePath != nullptr) {
		this->turtleImagePath = this->makeFilePath(imagePath, false);
	}
	else {
		this->turtleImagePath = this->makeFilePath();
	}
	// Store the size of the turtle symbol
	Image* image = new Image(this->turtleImagePath);
	if (image != nullptr) {
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
		if (this->elements.size() == 1) {
			this->lastDrawn = this->elements.cbegin();
		}
		RectF::Union(this->bounds, this->bounds, RectF(this->pos.X, this->pos.Y, 1, 1));
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
		if (this->elements.size() == 1) {
			this->lastDrawn = this->elements.cbegin();
		}
		RectF::Union(this->bounds, this->bounds, RectF(this->pos.X, this->pos.Y, 1, 1));
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
	this->gotoXY(x, (int)this->pos.Y);
}

// Sets the Y-coordinate of the turtle's position to a new value.
void Turtle::gotoY(int y)
{
	this->gotoXY((int)this->pos.X, y);
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
	RectF oldBounds(this->getBounds());
	this->elements.clear();
	this->bounds = RectF(this->pos.X, this->pos.Y, 1.0f, 1.0f);
	this->lastDrawn = this->elements.cend();
	this->nDrawn = 0;
	// START KGU 2021-04-05: issue #6 performance improvement
	//this->refresh(this->pos);
	this->pTurtleizer->refresh(bounds, -1);
	// END KGU 2021-04-05
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

RectF Turtle::getBounds() const
{
	// Ensure the current turtle position is contained by the bound.
	if (!this->bounds.Contains(this->pos)) {
		RectF myBounds(this->pos.X, this->pos.Y, 1.0f, 1.0f);
		RectF::Union(myBounds, myBounds, this->bounds);
		return myBounds;
	}
	return this->bounds;
}

bool Turtle::isTurtleShown() const
{
	return this->isVisible;
}

// Refreshes the window i.e. invalidates the region between `oldPos´
// and this->pos and then updates the window
void Turtle::refresh(const PointF& oldPos, bool forceIconSize) const
{
	// Consider rotation, so use maximum diagonal
	LONG halfIconSize = (forceIconSize || this->isVisible) ? (LONG)(max(this->turtleHeight, this->turtleWidth) / sqrt(2.0) +1) : 1L;
	// START KGU 2021-04-02: Issue #6 We must consider transformations - this is not the window RECT!
	//RECT rect;
	//rect.left = (LONG)floor(min(oldPos.X, this->pos.X)) - halfIconSize;
	//rect.right = (LONG)ceil(max(oldPos.X, this->pos.X)) + halfIconSize;
	//rect.top = (LONG)floor(min(oldPos.Y, this->pos.Y)) - halfIconSize;
	//rect.bottom = (LONG)ceil(max(oldPos.Y, this->pos.Y)) + halfIconSize;
	REAL left = floor(min(oldPos.X, this->pos.X)) - halfIconSize;
	REAL right = ceil(max(oldPos.X, this->pos.X)) + halfIconSize;
	REAL top = floor(min(oldPos.Y, this->pos.Y)) - halfIconSize;
	REAL bottom = ceil(max(oldPos.Y, this->pos.Y)) + halfIconSize;
	RectF rect(left, top, right - left, bottom - top);
	// END KGU 2021-04-02
	this->pTurtleizer->refresh(rect, this->elements.size());
}

// Composes a file path from the path of this source file (project
// folder) and the given file name.
// (if the image file name isn't given, the turtle image will be used)
LPCWSTR Turtle::makeFilePath(LPCWSTR filename, bool addProductPath) const
{
	assert(filename != nullptr);
	WCHAR delimiter = L'/';
	LPCWSTR pMyPath = __WFILE__;
	LPCWSTR pPosSlash = wcsrchr(pMyPath, L'/');
	LPCWSTR pPosBSlash = wcsrchr(pMyPath, L'\\');
	if (pPosSlash < pPosBSlash) {
		pPosSlash = pPosBSlash;
		delimiter = L'\\';
	}
	size_t pathLen = 0;
	if (addProductPath) {
		pathLen = (pPosSlash != nullptr) ? pPosSlash - pMyPath : wcslen(pMyPath);
	}
	size_t buffLen = pathLen + wcslen(filename) + 2;
	WCHAR* pFilePath = new WCHAR[buffLen];
	if (addProductPath) {
		wcsncpy_s(pFilePath, buffLen, pMyPath, pathLen);
		pFilePath[pathLen++] = delimiter;
	}
	wcscpy_s(&pFilePath[pathLen], buffLen - pathLen, filename);
	return pFilePath;
}

REAL Turtle::getNearestPoint(const PointF& coord, bool betweenEnds, double radius, PointF& nearest) const
{
	REAL minDist = -1.0;
	for (Elements::const_iterator it(this->elements.cbegin()); it != this->elements.cend(); ++it)
	{
		PointF cand;
		REAL dist = it->getNearestPoint(coord, betweenEnds, cand);
		if (dist == 0.0) {
			nearest = cand;
			return dist;
		}
		else if (dist < radius && (minDist < 0 || dist < minDist)) {
			nearest = cand;
			minDist = dist;
		}
	}
	return minDist;
}

void Turtle::draw(Graphics& gr, bool drawAll, bool withImage)
{
	// START KGU 2021-04-05: issue #6 performance improvement
	//for (Elements::const_iterator it(this->elements.cbegin()); it != this->elements.cend(); ++it)
	//{
	//	it->draw(gr);
	//}
	if (drawAll) {
		this->nDrawn = 0;
		this->lastDrawn = this->elements.cbegin();
	}
	int nElements = this->elements.size();
	Elements::const_iterator it(this->lastDrawn);
	if (nElements == this->nDrawn) {
		return;
	}
	else if (this->nDrawn > 0) {
		++it;
	}
	for (; this->nDrawn < nElements; ++it, this->nDrawn++) {
		it->draw(gr);
		if (this->nDrawn > 0) {
			++this->lastDrawn;
		}
	}
	// END KGU  2021-04-05

	// START KGU 2021-04-05: Issue #6, delegated to drawImage()
//	if (this->isVisible) {
//		Matrix transf;
//		gr.GetTransform(&transf);
//		//Gdiplus::REAL matrix[6];
//		// Display an image
//		//Image* image = new Image(L"Turtle.png");
//		Image* image = new Image(this->turtleImagePath);
//		// START KGU 2019-07-08 Workaround #3
//		//PointF pointF(-(REAL)this->turtleWidth / (REAL)2.0, -(REAL)this->turtleHeight / (REAL)2.0);
//		REAL scaleX = gr.GetDpiX() / image->GetHorizontalResolution();
//		REAL scaleY = gr.GetDpiY() / image->GetVerticalResolution();
//		PointF pointF(-(REAL)this->turtleWidth * scaleX / (REAL)2.0,
//			-(REAL)this->turtleHeight * scaleY / (REAL)2.0);
//		// END KGU 2019-07-08
//#if DEBUG_PRINT
//		printf("The width of the image is %u.\n", this->turtleWidth);
//		printf("The height of the image is %u.\n", this->turtleHeight);
//#endif /*DEBUG_PRINT*/
//		gr.TranslateTransform(this->pos.X, this->pos.Y);
//		gr.RotateTransform(-(REAL)this->orient);
//
//		//Matrix transf;
//		//gr.GetTransform(&transf);
//		//Gdiplus::Status status = transf.GetElements(matrix);
//
//		gr.DrawImage(image, pointF);
//		// Restore original transform
//		gr.ResetTransform();
//		gr.SetTransform(&transf);
//		gr.Flush();
//		delete image;
//	}
	if (withImage && this->isVisible) {
		this->drawImage(gr);
	}
	// END KGU 2021-04-05

}

// START KGU 2021-04-05: Issue #6 drawing of the icon separated
void Turtle::drawImage(Graphics& gr) const
{
	if (this->isVisible) {
		Matrix transf;
		gr.GetTransform(&transf);
		//Gdiplus::REAL matrix[6];
		// Display an image
		//Image* image = new Image(L"Turtle.png");
		Image* image = new Image(this->turtleImagePath);
		// START KGU 2019-07-08 Workaround #3
		//PointF pointF(-(REAL)this->turtleWidth / (REAL)2.0, -(REAL)this->turtleHeight / (REAL)2.0);
		REAL scaleX = gr.GetDpiX() / image->GetHorizontalResolution();
		REAL scaleY = gr.GetDpiY() / image->GetVerticalResolution();
		PointF pointF(-(REAL)this->turtleWidth * scaleX / (REAL)2.0,
			-(REAL)this->turtleHeight * scaleY / (REAL)2.0);
		// END KGU 2019-07-08
#if DEBUG_PRINT
		printf("The width of the image is %u.\n", this->turtleWidth);
		printf("The height of the image is %u.\n", this->turtleHeight);
#endif /*DEBUG_PRINT*/
		gr.TranslateTransform(this->pos.X, this->pos.Y);
		gr.RotateTransform(-(REAL)this->orient);

		//Matrix transf;
		//gr.GetTransform(&transf);
		//Gdiplus::Status status = transf.GetElements(matrix);

		gr.DrawImage(image, pointF);
		// Restore original transform
		gr.ResetTransform();
		gr.SetTransform(&transf);
		gr.Flush();
		delete image;
	}
}
// END KGU 2021-04-05

bool Turtle::hasElements() const
{
	return !this->elements.empty();
}

void Turtle::writeSVG(std::ostream& ostr, PointF offset, unsigned short scale) const
{
	/* In contrast to Structorizer TurtleBox, which exports the points
	 * as int coordinate pairs, we export them with real-number coordinates.
	 * The reason is that the line elements here have floating point
	 * coordinates, whereas they used to be stored with integer start and end
	 * coordinates in Structorizer.
	 * SVG paths are defined incrementally, i.e. via coordinate differences,
	 * so rounding the differences would definitely compromise the drawing.
	 * What we could do is to round the coordinates before computing the differences
	 * but then we should also round the points before detecting gaps in the paths.
	 * (In the event this is what Structorizer does, to some success.
	 * Anyway, we are on the safer side here, paths can get longer and surprisingly
	 * do get longer than on export from Structorizer's TurtleBox.
	 */
	PointF lastPt;
	Color lastCol;
	int nPoints = 0;
	ostr.fill('0');
	for (Elements::const_iterator it(this->elements.cbegin()); it != this->elements.cend(); ++it)
	{
			PointF from = it->getFrom();
			PointF to = it->getTo();
			Color col = it->getColor();
			if (nPoints == 0 || !lastPt.Equals(from)
				|| lastCol.GetValue() != col.GetValue()
				|| nPoints >= MAX_POINTS_PER_SVG_PATH) {
				if (nPoints > 0) {
					// End the previous path
					ostr << "\" />\n";
				}
				// Start a new path
				ostr << "    <path\n";
				ostr << "      style=\"stroke:#"
					<< std::hex << std::setw(6)
					<< (int)(col.GetValue() & 0xFFFFFF)
					<< std::dec << "\"\n";
				ostr << "      id=\"path" << std::setw(5) << nPoints << "\"\n";
				ostr << "      d=\"m "
					<< ((from.X + offset.X) * scale) << ","
					<< ((from.Y + offset.Y) * scale) << " ";
			}
			ostr << ((to.X - from.X) * scale) << ","
				<< ((to.Y - from.Y) * scale) << " ";
			lastPt = to;
			lastCol = col;
			nPoints++;
	}
	if (nPoints > 0) {
		ostr << "\" />\n";
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

REAL Turtle::TurtleLine::getNearestPoint(const PointF& pt, bool betweenEnds, PointF& nearest) const
{
	if (betweenEnds) {
		// We abuse a point for the direction vector
		PointF dvec(x2 - x1, y2 - y1);
		PointF pvec(pt.X - x1, pt.X - y1);
		double dlen2 = (dvec.X * dvec.X + dvec.Y * dvec.Y);
		double param = (pvec.X * dvec.X + pvec.Y * dvec.Y) / dlen2;
		if (param < 0) {
			nearest.X = x1;
			nearest.Y = y1;
		}
		else if (param * param > dlen2) {
			nearest.X = x2;
			nearest.Y = y2;
		}
		else {
			nearest.X = x1 + param * dvec.X;
			nearest.Y = y1 + param * dvec.Y;
		}
		double distX = nearest.X - pt.X;
		double distY = nearest.Y - pt.Y;
		return (REAL)sqrt(distX * distX + distY * distY);
	}
	else {
		double distX = x1 - pt.X;
		double distY = y1 - pt.Y;
		REAL dist1 = (REAL)sqrt(distX * distX + distY * distY);
		distX = x2 - pt.X;
		distY = y2 - pt.Y;
		REAL dist2 = (REAL)sqrt(distX * distX + distY * distY);
		if (dist2 > dist1) {
			nearest.X = x1;
			nearest.Y = y1;
			return dist1;
		}
		else {
			nearest.X = x2;
			nearest.Y = y2;
			return dist2;
		}
	}
	return 0;	// Not reachable
}

void Turtle::TurtleLine::draw(Gdiplus::Graphics& gr) const
{
	// Draw a line
	Pen pen(this->col);
	gr.DrawLine(&pen, this->x1, this->y1, this->x2, this->y2);
}

