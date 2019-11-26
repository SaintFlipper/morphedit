/*****************************************************************************
 File:			slider.c
 Description:	The SLIDER control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 31/3/94	| Created.											| PW
*****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <windowsx.h>
#include <windows.h>

#include <aucntrl.h>
#include "auc_i.h"

/****************************************************************************/
/*								Configuration								*/
/****************************************************************************/

#define	FIXED_BACKGROUND

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define	CURRENT_VALUE_X_OFFSET		0
#define	CURRENT_VALUE_Y_OFFSET		5

// ROP codes
#define	DSa							0x008800C6L
#define	DSx							0x00660046L

/****************************************************************************/
/*								Local types									*/
/****************************************************************************/

typedef struct
{
	BOOL	fSelected;					// Whether the control is currently selected
	int		nCurrent;					// Current value of the control
	HBITMAP	hBackground;				// Background bitmap
} PIN_INFO, FAR *PPIN_INFO;

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

static HBITMAP	hbmpNormalPin;			// Unselected pin
static HBITMAP	hbmpSelectedPin;		// Selected pin
#ifdef FIXED_BACKGROUND
static HBITMAP	hbmpBackground;			// Background
#endif
static POINT	nPinSize;				// Pixel size of a pin
static HFONT	hFont;					// Font to use for value numbers

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

LRESULT CALLBACK PinControlProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

static HBITMAP GrabBackground (HWND hWnd);
static BOOL TransBlt (HDC hdcD, int x, int y, int dx, int dy, HDC hdcS, int x0, int y0);
static VOID DrawPin (HWND hWnd, PPIN_INFO pPinInfo);

/*^L*/
/*****************************************************************************
 Function:		PatchPinControlInit
 Parameters:	HINSTANCE	hInst		Current instance.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Initialises the patchboard pin control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 18/5/94 	| Created.											| PW
*****************************************************************************/

BOOL PatchPinControlInit (HINSTANCE hInst)
{
	WNDCLASS	wc;
	BITMAP		bm;

	// Create the title + value font
	hFont = CreateFont (-7, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
						OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
						FF_DONTCARE, "MorphNumbers");
	if (hFont == (HFONT)NULL)
	{
		// Error
		return (FALSE);
	}

	// Load the normal pin bitmap
	if ((hbmpNormalPin = LoadBitmap (hInst, (LPCSTR)"IDB_PIN_NORMAL")) == (HBITMAP)NULL)
	{
		// Error
		return (FALSE);
	}

	// Load the selected pin bitmap
	if ((hbmpSelectedPin = LoadBitmap (hInst, (LPCSTR)"IDB_PIN_SELECTED")) == (HBITMAP)NULL)
	{
		// Error
		return (FALSE);
	}

#ifdef FIXED_BACKGROUND
	// Load the background
	if ((hbmpBackground = LoadBitmap (hInst, (LPCSTR)"IDB_PIN_BACKGROUND")) == (HBITMAP)NULL)
	{
		// Error
		return (FALSE);
	}
#endif

	// Find out how big the pin is
	GetObject (hbmpNormalPin, sizeof (BITMAP), (LPSTR)&bm);
	nPinSize.x	= bm.bmWidth;
	nPinSize.y	= bm.bmHeight;

	// Register the window class
	wc.lpszClassName	= PIN_CLASS_NAME;
	wc.style			= CS_GLOBALCLASS;
	wc.hCursor			= LoadCursor (NULL, IDC_ARROW);
	wc.hIcon			= NULL;
	wc.lpszMenuName		= NULL;
	wc.hbrBackground	= NULL;
	wc.hInstance		= hInst;
	wc.lpfnWndProc		= PinControlProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= sizeof (LONG);
	if (!RegisterClass (&wc))
	{
		return (FALSE);
	}

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		PatchPinControlTerm
 Parameters:	HINSTANCE	hInst		Current instance.
 Returns:		None.
 Description:	Unloads the patchboard pin control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 19/5/94 	| Created.											| PW
*****************************************************************************/

VOID PatchPinControlTerm (HINSTANCE hInst)
{
	// Get rid of the bitmaps
	DeleteObject (hbmpNormalPin);
	DeleteObject (hbmpSelectedPin);
#ifdef FIXED_BACKGROUND
	DeleteObject (hbmpBackground);
#endif

	// Get rid of the font
	DeleteObject (hFont);

	// Unregister the class
	UnregisterClass (PIN_CLASS_NAME, hInst);
}

/*^L*/
/*****************************************************************************
 Function:		PinControlProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		LONG					Window return value.
 Description:	The patchboard pin control procedure.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 18/5/94 	| Created.											| PW
*****************************************************************************/

LRESULT CALLBACK PinControlProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT			ps;
	PPIN_INFO			pPinInfo;

	// Retrieve what may be the control info
	pPinInfo = (PPIN_INFO)GetWindowLong (hWnd, 0);

	switch (nMsg)
	{
		// Creation
		case WM_CREATE:
		{
			// Allocate space for the control info
			pPinInfo = (PPIN_INFO)(LPVOID)GlobalAllocPtr (GPTR, sizeof (PIN_INFO));

            // If we got some memory
			if (pPinInfo != NULL)
			{
            	// Save the pointer in the window's extra memory
				SetWindowLong (hWnd, 0, (LONG)(LPVOID)pPinInfo);

				// Set up the default control info
				pPinInfo->nCurrent		= 0;
				pPinInfo->hBackground	= (HBITMAP)NULL;
				pPinInfo->fSelected		= FALSE;

				// OK
                return (0);
			}
			else
			{
				// No memory
                return (-1);
            }
		}

		// Subcontrol
		case WM_COMMAND:
		{
			break;
		}

		// Left button down
		case WM_LBUTTONDOWN:
		{
			pPinInfo->fSelected = TRUE;
			DrawPin (hWnd, pPinInfo);
			SendMessage (GetParent (hWnd), PINN_SELECTED, GETWNDID (hWnd), 0);
			break;
		}

		// Mouse up
		case WM_LBUTTONUP:
		{
			pPinInfo->fSelected = FALSE;
			DrawPin (hWnd, pPinInfo);
			break;
		}

		// New value
		case PIN_SET_CURRENT:
		{
			pPinInfo->nCurrent = (int)wParam;
			DrawPin (hWnd, pPinInfo);
			break;
		}

		// Gain focus
		case PIN_SET_FOCUS:
		{
			pPinInfo->fSelected = TRUE;
			DrawPin (hWnd, pPinInfo);
			break;
		}

		// Lose focus
		case PIN_KILL_FOCUS:
		{
			pPinInfo->fSelected = FALSE;
			DrawPin (hWnd, pPinInfo);
			break;
		}

		// Show control
		case WM_SHOWWINDOW:
		{
#ifdef FIXED_BACKGROUND
			// Use the fixed background bitmap
			pPinInfo->hBackground = hbmpBackground;
#else
			// Grab the control background area
			pPinInfo->hBackground = GrabBackground (hWnd);
#endif

			// Draw the control
			DrawPin (hWnd, pPinInfo);
			break;
		}

		// Erase background
		case WM_ERASEBKGND:
		{
			// Say we erased it
			return (TRUE);
		}

		// Repaint
		case WM_PAINT:
		{
			BeginPaint (hWnd, &ps);
			DrawPin (hWnd, pPinInfo);
			EndPaint (hWnd, &ps);
			break;
		}

		// Destroy
		case WM_DESTROY:
		{
#ifndef FIXED_BACKGROUND
			if (pPinInfo->hBackground)
			{
				DeleteObject (pPinInfo->hBackground);
			}
#endif
			GlobalFreePtr (pPinInfo);
			break;
		}

        // Default
		default:
        {
			return (DefWindowProc (hWnd, nMsg, wParam, lParam));
		}
	}

	return (0);
}

/*^L*/
/*****************************************************************************
 Function:		GrabBackground
 Parameters:	HWND	hWnd			Control window.
 Returns:		HBITMAP					A bitmap containing the background.
 Description:	Grabs the background to a window
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 19/5/94 	| Created.											| PW
*****************************************************************************/

static HBITMAP GrabBackground (HWND hWnd)
{
	HDC		hdcParent, hdcOffScr;
	HBITMAP	hbmOld, hbm;
	RECT	rc;
	POINT	pt;

	GetClientRect (hWnd, &rc);
	hdcParent = GetDC (GetParent (hWnd));
	hdcOffScr = CreateCompatibleDC (hdcParent);
	hbm = CreateCompatibleBitmap (hdcParent, rc.right - rc.left, rc.bottom - rc.top);
	hbmOld = SelectObject (hdcOffScr, hbm);

	// grab a chunk of the main window dc
	pt.x = rc.left;
	pt.y = rc.top;
	ClientToScreen (hWnd, &pt);
	ScreenToClient (GetParent (hWnd), &pt);
	BitBlt (hdcOffScr, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hdcParent, pt.x, pt.y, SRCCOPY);
	SelectObject (hdcOffScr, hbmOld);
	DeleteDC (hdcOffScr);
	ReleaseDC (GetParent (hWnd), hdcParent);
	return (hbm);
}

/*^L*/
/*****************************************************************************
 Function:		TransBlt
 Parameters:	HDC		hdcD
				int		x
				int		y
				int		dx
				int		dy
				HDC		hdcS
				int		x0
				int		y0
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Performs a BitBlt from hdcS to hdcD device context, treating
				the destinations's background colour as background.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 19/5/94 	| Created.											| PW
*****************************************************************************/

static BOOL TransBlt (HDC hdcD, int x, int y, int dx, int dy, HDC hdcS, int x0, int y0)
{
	DWORD	rgbBk, rgbFg;
	DWORD	rgbBkS;
	HBITMAP	hbmMask;
	HDC		hdcMask;
	HBITMAP	hbmT;
	BOOL	f = FALSE;

	//
	//  Get the current DC color's
	//
	rgbBk = GetBkColor (hdcD);
	rgbFg = GetTextColor (hdcD);
	rgbBkS = GetBkColor (hdcS);
	SetTextColor (hdcD, BLACK);

	//
	//  make a memory DC for use in color conversion
	//
	hdcMask = CreateCompatibleDC (hdcS);
	if (!hdcMask)
    {
		return (FALSE);
    }

	//
	// create a mask bitmap and associated DC
	//
	hbmMask = CreateBitmap (dx, dy, 1, 1, NULL);
	if (!hbmMask)
    {
		goto errorDC;
	}

	// select the mask bitmap into the mono DC
	hbmT = SelectObject (hdcMask, hbmMask);

	// do a color to mono bitblt to build the mask
	// generate 1's where the source is equal to the background, else 0's
	SetBkColor (hdcS, rgbBk);
	BitBlt (hdcMask, 0, 0, dx, dy, hdcS, x0, y0, SRCCOPY);

	// do a MaskBlt to copy the bitmap to the dest
	SetBkColor (hdcD, WHITE);
	BitBlt (hdcD, x, y, dx, dy, hdcS, x0, y0, DSx);
	BitBlt (hdcD, x, y, dx, dy, hdcMask, 0, 0, DSa);
	BitBlt (hdcD, x, y, dx, dy, hdcS, x0, y0, DSx);
	f = TRUE;
	SelectObject (hdcMask, hbmT);
	DeleteObject (hbmMask);

	//
	// Restore the DC colors
	//
	SetBkColor (hdcS, rgbBkS);
	SetBkColor (hdcD, rgbBk);
	SetTextColor (hdcD, rgbFg);
	errorDC:
	DeleteDC (hdcMask);
	return (f);
}

/*^L*/
/*****************************************************************************
 Function:		DrawPin
 Parameters:	HWND		hWnd		Control window.
				PPIN_INFO	pPinInfo	Control information.
 Returns:		None.
 Description:	Draws the pin control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 19/5/94 	| Created.											| PW
*****************************************************************************/

static VOID DrawPin (HWND hWnd, PPIN_INFO pPinInfo)
{
	HDC			hdcOffScr, hdcControl, hDC, hdcBackground;
	HBITMAP		hbmOldControl, hbmControl, hbmOffScr, hbmOldOffScr;
	RECT		rcClient;
	int			x, nTextWidth, len, OldMode;
	char		szText[40];
	COLORREF	OldColour, OldOffScrBkColour;
	HFONT		hOldFont;
	SIZE		size;

	// Get the control's screen DC
	hDC = GetDC (hWnd);
	GetClientRect (hWnd, &rcClient);

	// Create an off screen DC
	hdcOffScr = CreateCompatibleDC (hDC);
	hbmOffScr = CreateCompatibleBitmap (hDC, nPinSize.x, nPinSize.y);
	hbmOldOffScr = SelectObject (hdcOffScr, hbmOffScr);

	// Create a DC for the background bitmap
	hdcBackground = CreateCompatibleDC (hDC);
	SelectObject (hdcBackground, pPinInfo->hBackground);

	// Copy the background to the offscreen DC
	BitBlt (hdcOffScr, 0, 0, nPinSize.x, nPinSize.y, hdcBackground, 0, 0, SRCCOPY);
	DeleteDC (hdcBackground);

	// Do a transparent BitBlt of the pin to the offscreen DC
	hbmControl = pPinInfo->fSelected ? hbmpSelectedPin : hbmpNormalPin;
	hdcControl = CreateCompatibleDC (hDC);
	hbmOldControl = SelectObject (hdcControl, hbmControl);
	OldOffScrBkColour = SetBkColor (hdcOffScr, RGB (0, 0, 255));	// transparency color is BLUE
	TransBlt (hdcOffScr, 0, 0, nPinSize.x, nPinSize.y, hdcControl, 0, 0);
	SetBkColor (hdcOffScr, OldOffScrBkColour);
	SelectObject (hdcControl, hbmOldControl);
	DeleteDC (hdcControl);

#ifdef NEVER
	///
	BitBlt (hDC, 0, 0, nPinSize.x, nPinSize.y, hdcOffScr, 0, 0, SRCCOPY);
	SelectObject (hdcOffScr, hbmOldOffScr);
	DeleteObject (hbmOffScr);
	DeleteDC (hdcOffScr);
	ReleaseDC (hWnd, hDC);
	return;
	///
#endif

	// Create the current value text
	itoa (pPinInfo->nCurrent, &szText[0], 10);

	// Select the right font, text colour, and background mode
	hOldFont = SelectObject (hdcOffScr, hFont);
	OldColour = SetTextColor (hdcOffScr, pPinInfo->fSelected ? WHITE : BLACK);
	OldMode = GetBkMode (hdcOffScr);
	SetBkMode (hdcOffScr, TRANSPARENT);

	// So how long is the text ?
	GetTextExtentPoint (hdcOffScr, &szText[0], len = strlen (&szText[0]), &size);
	nTextWidth = size.cx;

	// Work out what the text X offset must be to centre it in the pin
	x = rcClient.left + ((nPinSize.x - nTextWidth) / 2);

	// Draw the current value text
	TextOut (hdcOffScr, x, CURRENT_VALUE_Y_OFFSET, &szText[0], len);
	SetBkMode (hdcOffScr, OPAQUE/*OldMode*/);
	SetTextColor (hdcOffScr, OldColour);
	SelectObject (hdcOffScr, hOldFont);

	// Blit the offscreen dc stuff to the screen dc
	BitBlt (hDC, 0, 0, nPinSize.x, nPinSize.y, hdcOffScr, 0, 0, SRCCOPY);

	// Clear everything up
	SelectObject (hdcOffScr, hbmOldOffScr);
	DeleteObject (hbmOffScr);
	DeleteDC (hdcOffScr);
	ReleaseDC (hWnd, hDC);
}
