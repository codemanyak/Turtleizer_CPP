#pragma once
#ifndef TURTLECANVAS_H
#define TURTLECANVAS_H
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
 * Version: 11.0.0 (covering capabilities of Structorizer 3.30-12, functional GUI)
 *
 * History (add on top):
 * --------------------------------------------------------
 * 2021-04-02   Scrolling and zooming implemented
 * 2021-03-31   created for VERSION 11.0.0 (to address the scrollbar mechanism, #6)
 */

#include <Windows.h>
#include <gdiplus.h>
#include <commctrl.h>
#include <string>

using std::string;
using std::wstring;
using namespace Gdiplus;

class Turtleizer;

class TurtleCanvas
{
public:
	TurtleCanvas(Turtleizer& frame, HWND hFrame);
	~TurtleCanvas();
	static LRESULT CALLBACK CanvasWndProc(HWND hWnd, UINT message,
		WPARAM wParam, LPARAM lParam);
	// Redraws the turtle canvas (with nElements lines), at least in the given coord rectangle rectF
	void redraw(const RectF& rectF, int nElements);
	// Redraws the turtle canvas (in the pixel rectangle pRect) and sets the autoUpdate mode according to automatic
	void redraw(bool automatic, const RECT* pRect = nullptr);
	// Resizes the window according to the frame client area
	void resize();
	// Zooms in or out by factor ZOOM_RATE
	void zoom(bool zoomIn);
	// Scrolls a short or large unit horizontally or vertically
	void scroll(bool horizontally, bool forward, bool large, unsigned int count = 1);
	// Is to update the scrollbars after drawing events or resizing
	// Returns whether the measuring mouse snaps to lines
	bool snapsToLines() const;
	// Returns the current zoom factor
	float getZoomFacor() const;
	// Returns the current drawing displacement
	PointF getDisplacement() const;
	// Returns the scroll intervals as rectangle in turtle coordinates
	RECT getScrollRect() const;
	// Translates accelerators as far as defined (returns whether it was handled)
	bool translateAccelerators(LPMSG pMessage) const;

private:
#ifdef UNICODE
	typedef LPCWSTR NameType;
	typedef wstring String;
#else
	typedef LPCSTR NameType;
	typedef string String;
#endif /*UNICODE*/
	// Menudefinition structure
	struct MenuDef {
		LPCWSTR caption;
		ACCEL accelerator;
		BOOL(*method)(bool);
		bool isCheck;
	};
	// Maximum and minimum zoom factor, zoom change factor
	static const float MAX_ZOOM, MIN_ZOOM, ZOOM_RATE;
	static const NameType WCLASS_NAME;			// Name of the window class
	static const int IDM_CONTEXT_MENU = 20000;	// Start identifier for context menu items
	static const MenuDef MENU_DEFINITIONS[];	// Context menu specification
	HWND hCanvas;								// The handle of the canvas window (subwindow)
	HWND hFrame;								// The handle of the frame window
	Turtleizer* const pFrame;					// Reference to the owning Turtleizer
	HINSTANCE hInstance;						// Module instance handle
	WNDCLASS wndClass;							// Holds the created window class
	HMENU hContextMenu;		// Context menu handle
	HACCEL hAccel;			// Handle of the accelerator table
	float zoomFactor;		// current zoom factor (1.0f corresponds to 100%)
	PointF displacement;	// Offset of the coordinate origin (never negative)
	POINT scrollPos;		// Current scroll position (in pixel units!)
	bool popupCoords;		// Whether coordinates are to be shown as popup
	bool showAxes;			// Whether coordinate axes are to be drawn
	bool snapLines;			// Snap mode (default: true)
	float snapRadius;		// Snap radius
	bool autoUpdate;		// Whether the window is to be updated on every movement

	// Retrieves the responsible instance of this class from the frame
	static TurtleCanvas* getInstance();
	void adjustScrollbars();
	// Updates item visibility and checkboxes of the context menu
	void updateContextMenu();
	// Callback method for refresh (OnPaint event)
	VOID onPaint(HDC hdc);
	// Callback method for context menu event
	VOID onContextMenu(int x, int y);
	// General callback method for command handling
	BOOL onCommand(WPARAM wParam, LPARAM lParam);
	// Callback method for scroll events
	VOID onScrollEvent(WORD scrollFlag, WORD pos, BOOL isVertical);

	// Retrieves the central point of the client area in turtle coordinates
	PointF getCenterCoord() const;
	// Retrieves the mouse position in turtle coordinates (possibly with snap)
	PointF getMouseCoord(bool snap) const;
	// Scrolls such that the given turtle coordinate is in the scroll range, ideally in the center
	void scrollToCoord(PointF coord);

	// Menu / accelerator handlers
	static BOOL handleGotoCoord(bool testOnly);
	static BOOL handleGotoTurtle(bool testOnly);
	static BOOL handleGotoHome(bool testOnly);
	static BOOL handleGotoOrigin(bool testOnly);
	static BOOL handleZoom100(bool testOnly);
	static BOOL handleZoomBounds(bool testOnly);
	static BOOL handleShowAll(bool testOnly);
	static BOOL handleToggleAxes(bool testOnly);
	static BOOL handleToggleTurtle(bool testOnly);
	static BOOL handleSetBackground(bool testOnly);
	static BOOL handleToggleCoords(bool testOnly);
	static BOOL handleToggleStatus(bool testOnly);
	static BOOL handleToggleSnap(bool testOnly);
	static BOOL handleSetSnapRadius(bool testOnly);
	static BOOL handleToggleUpdate(bool testOnly);
	static BOOL handleExportCSV(bool testOnly);
	static BOOL handleExportPNG(bool testOnly);
	static BOOL handleExportSVG(bool testOnly);

};

#endif /*TURTLECANVAS_H*/