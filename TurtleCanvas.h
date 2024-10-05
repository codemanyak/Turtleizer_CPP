#pragma once
#ifndef TURTLECANVAS_H
#define TURTLECANVAS_H
/*
 * Fachhochschule Erfurt https://ai.fh-erfurt.de
 * Fachrichtung Angewandte Informatik
 * Project: Turtleizer_CPP (static C++ library for Windows)
 *
 * Object class, representing one (of potentially many) Turtles withing the
 * simple C++ emulation of the Turtleizer module coming with Structorizer
 * (http://structorizer.fisch.lu).
 * The intention is that several separately controllable (and subclassible)
 * Turtle objects may be created to share the drawing area.
 *
 * Author: Kay Gürtzig
 * Version: 11.0.1 (covering capabilities of Structorizer 3.30-12, functional GUI)
 *
 * History (add on top):
 * --------------------------------------------------------
 * 2024-10-04   Type modifications at MenuDef and chooseFileName(...)
 * 2021-04-20   CSV separator choice and coordinate input dialog implemented 
 * 2021-04-02   Scrolling, zooming, and background choice implemented
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
	// Informs the canvas that the next redrawing has to be done from scratch
	void setDirty();

private:
#ifdef UNICODE
	typedef LPCWSTR NameType;
	typedef wstring String;
#else
	typedef LPCSTR NameType;
	typedef string String;
#endif /*UNICODE*/
	// Number of choosable CSV separator characters
	static const unsigned short N_CSV_SEPARATORS = 5;
	// Menudefinition structure
	struct MenuDef {
		LPCTSTR caption;
		ACCEL accelerator;
		BOOL(*method)(bool);
		bool isCheck;
	};
	// Dialog item template structure (for file dialog customisation)
	struct TDlgItem {
		DLGITEMTEMPLATE dli;
		WORD class1;	// First WORD of class specification (0xFFFF)
		WORD class2;	// Second WORD of class specification (a predefined system class ordinal)
		WORD title;		// Set to 0 (= L""), title will be assigned dynamically
		WORD dummy;		// Follow-up word for the empty title
		WORD creatData;	// creation data, not used
	};
	// Dialog template structure for file dialog customisation
	static const struct TDlgSaveCSV {
		DLGTEMPLATE dlt;
		WORD menu;
		WORD classd;
		WCHAR title;		// There won't be a title
		TDlgItem fixItem;
		TDlgItem groupItem;
		TDlgItem radioItems[N_CSV_SEPARATORS];
	} tplSaveCSV;		// Custom template for the CSV SaveFile dialog
	// Dialog template structure for coordinate input
	static const struct TDlgInputCoord {
		DLGTEMPLATE dlt;
		WORD menu;
		WORD classd;
		WCHAR title;				// There won't be a title
		TDlgItem labelItem;			// Caption
		TDlgItem xyTextItems[2]/*[2]*/;	// label [and text items]
		TDlgItem buttons[2];		// Okay and Cancel button
	} tplDlgCoord;
	// Dialog template structure for radius input
	static const struct TDlgInputRadius {
		DLGTEMPLATE dlt;
		WORD menu;
		WORD classd;
		WCHAR title;				// There won't be a title
		TDlgItem labelItem;			// Caption
		//TDlgItem spinnerItem;		// input item with spinner (will be added dynamically)
		TDlgItem buttons[2];		// Okay and Cancel button
	} tplDlgRadius;
	static const UINT IDC_CUST_START = 200;		// First id for customer controls
	static const float MAX_ZOOM, MIN_ZOOM;		// Maximum and minimum zoom factor
	static const float ZOOM_RATE;				// Zoom change factor
	static const NameType WCLASS_NAME;			// Name of the window class
	static const int IDM_CONTEXT_MENU = 20000;	// Start identifier for context menu items
	static const MenuDef MENU_DEFINITIONS[];	// Context menu specification
	static const char* CSV_COL_HEADERS[];		// Table column headers for the CSV export
	static const char CSV_SEPARATORS[N_CSV_SEPARATORS];			// Choosable separator characters for CSV export
	static const NameType CSV_SEPARATOR_NAMES[N_CSV_SEPARATORS];// CSV separator description strings (radio button captions)
	static const NameType CSV_SEPARATOR;		// Caption for the separator radio button group
	static unsigned short ixCSVSepa;			// Index of the CSV separator last used
	TOOLINFO tooltipInfo;			// Tooltip info structure
	COLORREF customColors[16];		// Cache for user background colours
	HWND hCanvas;					// The handle of the canvas window (subwindow)
	HWND hFrame;					// The handle of the frame window
	Turtleizer* const pFrame;		// Reference to the owning Turtleizer
	HINSTANCE hInstance;			// Module instance handle
	HMENU hContextMenu;				// Context menu handle
	HWND hTooltip;					// Tooltip handle
	HACCEL hAccel;					// Handle of the accelerator table
	HCURSOR hArrow, hCross, hWait;	// Cursor handles
	HDC hdcScrCompat;				// memory DC for window buffering
	HBITMAP hBmpCompat;				// bitmap handle to memory DC 
	BITMAP bmp;						// bitmap data structure
	float zoomFactor;				// current zoom factor (1.0f corresponds to 100%)
	float snapRadius;				// Snap radius
	PointF displacement;			// Offset of the coordinate origin (never negative)
	PointF mouseCoord;				// Mouse position in turtle coordinates (snapped)
	POINT scrollPos;				// Current scroll position (in pixel units!)
	PointF* pDragStart;				// Start point of measuring line in turtle coords
	bool popupCoords;				// Whether coordinates are to be shown as popup
	bool showAxes;					// Whether coordinate axes are to be drawn
	bool snapLines;					// Snap mode (default: true)
	bool autoUpdate;				// Whether the window is to be updated on every movement
	bool tracksMouse;				// Set true while the mouse is inside the window
	bool mustRedraw;				// Flag indicating that the memory DC must be redrawn

	// Retrieves the responsible instance of this class from the frame
	static TurtleCanvas* getInstance();
	void adjustScrollbars();
	// Updates item visibility and checkboxes of the context menu
	void updateContextMenu();
	// Asks for a file name of the given types, writes the result path into filename
	//    and returns the index of the pure file name (without path) into the TCHAR
	//    array filename if successful, otherwise (i.e. if the user cancels or the
	//    call simply fails) returns 0xffffffff
	WORD chooseFileName(LPCTSTR filters, LPCTSTR defaultExt, LPTSTR fileName,
		LPOFNHOOKPROC lpHookProc = NULL, LPDLGTEMPLATE lpdt = NULL);
	// Callback method for refresh (WM_PAINT message event)
	VOID onPaint();
	// Callback method for context menu event
	VOID onContextMenu(int x, int y);
	// General callback method for command handling
	BOOL onCommand(WPARAM wParam, LPARAM lParam);
	// Callback method for scroll events
	VOID onScrollEvent(WORD scrollFlag, WORD pos, BOOL isVertical);
	// Callback method for mouse moving and dragging
	VOID onMouseMove(WORD X, WORD Y, BOOL isButtonDown);
	// Callback method for the save file dialog extension
	static UINT_PTR saveCSVHookProc(HWND hDlg, UINT msgId, WPARAM wParam, LPARAM lParam);
	// Callback method for Coordinate input dialog
	static BOOL CALLBACK DialogCoordProc(HWND hDlg, UINT msgId, WPARAM wParam, LPARAM lParam);
	// Callback method for Coordinate input dialog
	static BOOL CALLBACK DialogRadiusProc(HWND hDlg, UINT msgId, WPARAM wParam, LPARAM lParam);
	// Callback for EnumChildWindows to adjust the fonts
	static BOOL CALLBACK AdjustChildFontProc(HWND hDlg, LPARAM lParam);

	// Converts a turtle rectangle into a window rectangle
	RECT turtle2Window(const RectF& rect) const;
	// Retrieves the central point of the client area in turtle coordinates
	PointF getCenterCoord() const;
	// Scrolls such that the given turtle coordinate is in the scroll range, ideally in the center
	void scrollToCoord(const PointF& coord);

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

	// Returns NULL if text represents an int string or the pointer to the first unexpected character
	static TCHAR* checkIntString(LPCTSTR text);
};

#endif /*TURTLECANVAS_H*/