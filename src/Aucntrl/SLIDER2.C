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
#include <aucntrl.h>
#include "auc_i.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define	DEFAULT_SLIDER_MIN		0
#define	DEFAULT_SLIDER_MAX		127
#define	DEFAULT_SLIDER_CURRENT	0

#define	TRACK_YOFFSET			10
#define	TRACK_LENGTH			50

#define	PAD_XOFFSET				11
#define	PAD_WIDTH				7
#define	PAD_HEIGHT				5

#define	CONTROL_TIMER_ID		1
#define	FIRST_TIMEOUT_VALUE		(1000 / 2)
#define	REPEAT_TIMEOUT_VALUE	(1000 / 20)

#define	MAX_TITLE_LEN			24

/****************************************************************************/
/*								Local types									*/
/****************************************************************************/

typedef struct
{
	int		nMin;						// Maximum value for control
	int		nMax;						// Minimum value for control
	int		nCurrent;					// Current value for control

	int		nWidth;						// Width of control
	int		nHeight;					// Height of control

	int		nYmin;						// Top of active track
	int		nYmax;						// Bottom of active track
	int		nYcurrent;					// Current Y

	BOOL	fRedraw;					// Whether to redraw

	char	szTitle[MAX_TITLE_LEN + 1];	// 6 char title + '\0'

	RECT    rectUpPad;					// "Up-pad" rectangle
	RECT    rectDownPad;				// "Down-pad" rectangle
} SLIDER_INFO, FAR *PSLIDER_INFO;

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

static HBITMAP	hBackgroundBitmap;			// Slider background bitmap
static UINT		nBackgroundWidth;			// Width of background bitmap
static UINT		nBackgroundHeight;			// Height of background bitmap

static HBITMAP	hSliderBitmap;				// Slider bitmap
static UINT		nSliderBitmapWidth;			// Slider bitmap width
static UINT		nSliderBitmapHeight;		// Slider bitmap height

static BOOL		fCapturedInUp = FALSE;		// TRUE if mouse is captured in UP pad
static BOOL		fCapturedInDown = FALSE;	// TRUE if mouse is captured in DOWN pad
static BOOL		fCapturedInBody = FALSE;	// TRUE if mouse is captured elsewhere
static BOOL		fFirstTimeout;

static HFONT	hFont = (HFONT)NULL;		// Title and value font
static UINT		nFontHeight = 10;			// Height of font in pixels

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

LONG FAR PASCAL SliderControlProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static BOOL InPad (LPRECT pRect, int x, int y);
static VOID NewSliderValue (HWND hWnd, BOOL fTellParent);
static VOID DrawSlider (HWND hWnd);
static VOID CurrentToY (PSLIDER_INFO pSliderInfo);
static VOID AddToCurrent (HWND hWnd, int nOffset);
static VOID TellParent (HWND hWnd);

/*^L*/
/*****************************************************************************
 Function:		SliderControlInit
 Parameters:	HINSTANCE	hInst		Current instance.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Initialises the slider control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 27/3/94 	| Created.											| PW
*****************************************************************************/

BOOL SliderControlInit (HINSTANCE hInst)
{
	WNDCLASS	wc;
	BITMAP		bm;

	// Create the title + value font
	hFont = CreateFont (-8, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
						OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
						FF_DONTCARE, "Morpheus controls");
	if (hFont == (HFONT)NULL)
	{
		// Error
		return (FALSE);
	}

	// Load the slider bitmap
	if ((hSliderBitmap = LoadBitmap (hInst, (LPCSTR)"DIB_SCROLLER")) == (HBITMAP)NULL)
	{
		// Error
		return (FALSE);
	}

	// Find out how big the bitmap is
	GetObject (hSliderBitmap, sizeof (BITMAP), (LPSTR)&bm);
	nSliderBitmapWidth	= bm.bmWidth;
	nSliderBitmapHeight	= bm.bmHeight;

	// Load the background bitmap
	if ((hBackgroundBitmap = LoadBitmap (hInst, (LPCSTR)"DIB_BASE_100")) == (HBITMAP)NULL)
	{
		// Error
		DeleteObject (hSliderBitmap);
		return (FALSE);
	}

	// Find out how big the bitmap is
	GetObject (hBackgroundBitmap, sizeof (BITMAP), (LPSTR)&bm);
	nBackgroundWidth	= bm.bmWidth;
	nBackgroundHeight	= bm.bmHeight;

	// Register the window class
	wc.lpszClassName	= SLIDER_CLASS_NAME;
	wc.style			= CS_GLOBALCLASS;
	wc.hCursor			= LoadCursor (NULL, IDC_ARROW);
	wc.hIcon			= NULL;
	wc.lpszMenuName		= NULL;
	wc.hbrBackground	= NULL;
	wc.hInstance		= hInst;
	wc.lpfnWndProc		= SliderControlProc;
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
 Function:		SliderControlTerm
 Parameters:	HINSTANCE	hInst
 Returns:		None.
 Description:	Winds up the the slider control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/4/94 	| Created.											| PW
*****************************************************************************/

VOID SliderControlTerm (HINSTANCE hInst)
{
	// Get rid of the bitmaps
	DeleteObject (hBackgroundBitmap);
	DeleteObject (hSliderBitmap);

	// Get rid of the font
	DeleteObject (hFont);

	// Unregister the class
	UnregisterClass (SLIDER_CLASS_NAME, hInst);
}

/*^L*/
/*****************************************************************************
 Function:		SliderControlProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		LONG					Window return value.
 Description:	The slider control procedure.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 27/3/94 	| Created.											| PW
*****************************************************************************/

LRESULT CALLBACK SliderControlProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT			ps;
	RECT				rcWnd;
	PSLIDER_INFO		pSliderInfo;
	int					x, y;
	long				lRange;

	// Retrieve what may be the slider info
	pSliderInfo = (PSLIDER_INFO)GetWindowLong (hWnd, 0);

	switch (nMsg)
	{
		// Creation
		case WM_CREATE:
		{
			LPCREATESTRUCT	pCs = (LPCREATESTRUCT)lParam;

			// Check the window isn't too small
			if ((pCs->cx < SLIDER_CONTROL_WIDTH) || (pCs->cy < SLIDER_CONTROL_HEIGHT))
			{
				// Too small
				return (-1);
			}

			// Fix the window size
			pCs->cx = SLIDER_CONTROL_WIDTH;
			pCs->cy = SLIDER_CONTROL_HEIGHT;

			// Allocate space for the slider info
			pSliderInfo = (PSLIDER_INFO)(LPVOID)GlobalAllocPtr (GPTR, sizeof (SLIDER_INFO));

            // If we got some memory
			if (pSliderInfo != NULL)
			{
            	// Save the pointer in the window's extra memory
				SetWindowLong (hWnd, 0, (LONG)(LPVOID)pSliderInfo);

				// Set the default info
				pSliderInfo->nMin				= DEFAULT_SLIDER_MIN;
				pSliderInfo->nMax				= DEFAULT_SLIDER_MAX;
				pSliderInfo->nCurrent			= DEFAULT_SLIDER_CURRENT;

				pSliderInfo->nWidth				= pCs->cx;
				pSliderInfo->nHeight			= pCs->cy;

				pSliderInfo->nYmin				= TRACK_YOFFSET;
				pSliderInfo->nYmax				= TRACK_YOFFSET + TRACK_LENGTH;

				pSliderInfo->fRedraw			= TRUE;

				_fstrncpy (&pSliderInfo->szTitle[0], pCs->lpszName, MAX_TITLE_LEN);
				pSliderInfo->szTitle[MAX_TITLE_LEN] = '\0';

				pSliderInfo->rectUpPad.left		= PAD_XOFFSET;
				pSliderInfo->rectUpPad.right	= PAD_XOFFSET + PAD_WIDTH;
				pSliderInfo->rectUpPad.top		= 0;
				pSliderInfo->rectUpPad.bottom	= PAD_HEIGHT;

				pSliderInfo->rectDownPad.left	= PAD_XOFFSET;
				pSliderInfo->rectDownPad.right	= PAD_XOFFSET + PAD_WIDTH;
				pSliderInfo->rectDownPad.top	= (2 * TRACK_YOFFSET) + TRACK_LENGTH - PAD_HEIGHT;
				pSliderInfo->rectDownPad.bottom	= (2 * TRACK_YOFFSET) + TRACK_LENGTH;

				// Calculate the current Y from the slider value
				CurrentToY (pSliderInfo);

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
			// If the mouse isn't already captured
			if (!fCapturedInUp && !fCapturedInDown)
			{
				// If we are in the UP pad area
				if (InPad (&pSliderInfo->rectUpPad, LOWORD (lParam), HIWORD (lParam)))
				{
					// Increment the current value
					AddToCurrent (hWnd, 1);

					// Capture it in the UP pad 
					fCapturedInUp = TRUE;
					SetCapture (hWnd);

					// Start the repeater timeout
					fFirstTimeout = TRUE;
					SetTimer (hWnd, CONTROL_TIMER_ID, FIRST_TIMEOUT_VALUE, NULL);

					// That's it
					return (0);
				}
				// Else if we are in the DOWN pad area
				else if (InPad (&pSliderInfo->rectDownPad, LOWORD (lParam), HIWORD (lParam)))
				{
					// Decrement the current value
					AddToCurrent (hWnd, -1);

					// Capture it in the DOWN pad
					fCapturedInDown = TRUE;
					SetCapture (hWnd);

					// Start the repeater timeout
					fFirstTimeout = TRUE;
					SetTimer (hWnd, CONTROL_TIMER_ID, FIRST_TIMEOUT_VALUE, NULL);

					// That's it
					return (0);
				}
				else
				{
					// Capture it in the BODY
					SetCapture (hWnd);
					fCapturedInBody = TRUE;
				}
			}
			// Fall through
		}

		// Mouse moved
		case WM_MOUSEMOVE:
		{
			// If we have captured it
			if (fCapturedInBody)
			{
				// If we are above the active track top
				y = HIWORD (lParam);
				if (y < pSliderInfo->nYmin)
				{
					y = pSliderInfo->nYmin;
				}
                // Else if we are below the active track bottom
				else if (y > pSliderInfo->nYmax)
				{
					y = pSliderInfo->nYmax;
				}

				// Store the new Y value
				pSliderInfo->nYcurrent = y;

				// Offset y from 0 again
				y = y - pSliderInfo->nYmin;

				// Calculate the new value of the control from the position
				lRange = (long)pSliderInfo->nMax - (long)pSliderInfo->nMin;
				pSliderInfo->nCurrent = pSliderInfo->nMax -
					(int)(((long)y * lRange) / (long)TRACK_LENGTH);

				// Handle the new value
				NewSliderValue (hWnd, TRUE);
			}
			break;
		}

		// Repeat timeout
		case WM_TIMER:
		{
			// If we are in the UP pad
			if (fCapturedInUp)
			{
				// Increment the current value
				AddToCurrent (hWnd, 1);
			}
			// Else if we are in the DOWN pad
			else if (fCapturedInDown)
			{
				// Decrement the current value
				AddToCurrent (hWnd, -1);
			}
            // Else WHAT ARE WE DOING HERE ?
			else
			{
				KillTimer (hWnd, CONTROL_TIMER_ID);
				return (0);
			}

			// If it was the initial timeout
			if (fFirstTimeout)
            {
				// Kill the timer, and restart it faster
				fFirstTimeout = FALSE;
				KillTimer (hWnd, CONTROL_TIMER_ID);
				SetTimer (hWnd, CONTROL_TIMER_ID, REPEAT_TIMEOUT_VALUE, NULL);
			}
			break;
		}

		// Mouse up
		case WM_LBUTTONUP:
		{
        	// If it's captured
			if (fCapturedInBody || fCapturedInUp || fCapturedInDown)
			{
            	// Uncapture it
				ReleaseCapture ();
				fCapturedInBody = fCapturedInUp = fCapturedInDown = FALSE;

				// If we were in the UP or DOWN pad
				if (fCapturedInUp || fCapturedInDown)
				{
                	// Stop the repeat timer
					KillTimer (hWnd, CONTROL_TIMER_ID);
					return (0);
				}
			}
			break;
		}

		// New minimum value
		case SL_SET_MIN:
		{
			pSliderInfo->nMin = (int)wParam;
			if (pSliderInfo->nCurrent < pSliderInfo->nMin)
			{
				pSliderInfo->nCurrent = pSliderInfo->nMin;
			}

			// Redraw
			NewSliderValue (hWnd, FALSE);
			break;
		}

		// New maximum value
		case SL_SET_MAX:
		{
			pSliderInfo->nMax = (int)wParam;
			if (pSliderInfo->nCurrent > pSliderInfo->nMax)
			{
				pSliderInfo->nCurrent = pSliderInfo->nMax;
			}

			// Redraw
			NewSliderValue (hWnd, FALSE);
			break;
		}

		// New value
		case SL_SET_CURRENT:
		{
			pSliderInfo->nCurrent = (int)wParam;
			if (pSliderInfo->nCurrent < pSliderInfo->nMin)
			{
				pSliderInfo->nCurrent = pSliderInfo->nMin;
			}
			else if (pSliderInfo->nCurrent > pSliderInfo->nMax)
			{
				pSliderInfo->nCurrent = pSliderInfo->nMax;
			}
			// Redraw
			NewSliderValue (hWnd, (BOOL)lParam);
			break;
		}

		// Set the slider title
		case WM_SETTEXT:
		{
			LPCSTR	lpszTitle = (LPCSTR)lParam;

			_fstrncpy (&pSliderInfo->szTitle[0], lpszTitle, MAX_TITLE_LEN);
			pSliderInfo->szTitle[MAX_TITLE_LEN] = '\0';
			// Redraw
			NewSliderValue (hWnd, FALSE);
			break;
		}

		// Show control
		case WM_SHOWWINDOW:
		{
			// Redraw
			NewSliderValue (hWnd, FALSE);
			break;
		}

		// Switch painting on/off
		case WM_SETREDRAW:
		{
			// Store the flag
			pSliderInfo->fRedraw = (BOOL)wParam;
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
			// Redraw
			DrawSlider (hWnd);
			EndPaint (hWnd, &ps);
			break;
		}

		// Destroy
		case WM_DESTROY:
		{
			GlobalFreePtr ((LPVOID)pSliderInfo);
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
 Function:		CurrentToY
 Parameters:	PSLIDER_INFO	pSliderInfo	Slider info structure.
 Returns:		None.
 Description:	Calculates the slider Y position from it's current value.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 5/4/94 	| Created.											| PW
*****************************************************************************/

static VOID CurrentToY (PSLIDER_INFO pSliderInfo)
{
	long	lRange		= (long)pSliderInfo->nMax - (long)pSliderInfo->nMin;
	long	lCurrent	= (long)pSliderInfo->nCurrent;
	long	lNmax		= (long)pSliderInfo->nMax;

	// Calculate the new Y from the current value
	if (lRange == 0)
	{
		pSliderInfo->nYcurrent = (pSliderInfo->nYmin + pSliderInfo->nMax) / 2;
	}
	else
	{
		pSliderInfo->nYcurrent = pSliderInfo->nYmin + (int)(((lNmax - lCurrent) * (long)TRACK_LENGTH) / lRange);
	}
}

/*^L*/
/*****************************************************************************
 Function:		InPad
 Parameters:	LPRECT	pRect			Rectangle defining the pad.
				int		x				Current X
                int		y				Current Y
 Returns:		BOOL					TRUE if in pad, else FALSE.
 Description:	Looks to see if the mouse in is the specified rectangular pad.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 31/3/94 	| Created.											| PW
*****************************************************************************/

static BOOL InPad (LPRECT pRect, int x, int y)
{
	if ((x >= pRect->left) && (x < pRect->right) && (y >= pRect->top) && (y < pRect->bottom))
	{
		return (TRUE);
	}
	return (FALSE);
}

/*^L*/
/*****************************************************************************
 Function:		NewSliderValue
 Parameters:	HWND	hWnd			Control window.
				BOOL	fTellParent		Whether to notify the parent.
 Returns:		None.
 Description:	Redraws the control with its new value and sends a notification
				to the parent.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 31/3/94 	| Created.											| PW
*****************************************************************************/

static VOID NewSliderValue (HWND hWnd, BOOL fTellParent)
{
	PSLIDER_INFO	pSliderInfo = (PSLIDER_INFO)GetWindowLong (hWnd, 0);

	// Calculate the scroller Y from the current slider value
	CurrentToY (pSliderInfo);

	// If we are allowed to redraw
	if (pSliderInfo->fRedraw)
	{
		// Draw the slider
		DrawSlider (hWnd);
	}

	// Notify the parent
	if (fTellParent)
	{
		TellParent (hWnd);
	}
}

/*^L*/
/*****************************************************************************
 Function:		AddToCurrent
 Parameters:	HWND	hWnd			Control window.
				int		nOffset			Amount to add to current value.
 Returns:		None.
 Description:	Modifies the specified control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 13/4/94 	| Created.											| PW
*****************************************************************************/

static VOID AddToCurrent (HWND hWnd, int nOffset)
{
	PSLIDER_INFO	pSliderInfo = (PSLIDER_INFO)GetWindowLong (hWnd, 0);

	pSliderInfo->nCurrent += nOffset;
	if (pSliderInfo->nCurrent < pSliderInfo->nMin)
	{
		pSliderInfo->nCurrent = pSliderInfo->nMin;
	}
	else if (pSliderInfo->nCurrent > pSliderInfo->nMax)
	{
		pSliderInfo->nCurrent = pSliderInfo->nMax;
	}
	NewSliderValue (hWnd, TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		TellParent
 Parameters:	HWND	hWnd			Control window.
 Returns:		None.
 Description:	Passes the current value to the parent.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 13/4/94 	| Created.											| PW
*****************************************************************************/

static VOID TellParent (HWND hWnd)
{
	PSLIDER_INFO	pSliderInfo = (PSLIDER_INFO)GetWindowLong (hWnd, 0);

	SendMessage (GetParent (hWnd), SLN_NEW_VALUE,
					GetWindowWord (hWnd, GWW_ID), pSliderInfo->nCurrent);
}

/*^L*/
/*****************************************************************************
 Function:		DrawSlider
 Parameters:	HWND	hWnd			Control window.
 Returns:		None.
 Description:	Redraws the control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 31/3/94 	| Created.											| PW
*****************************************************************************/

static VOID DrawSlider (HWND hWnd)
{
	HDC				hDC, hdcMem, hdcOff;
	HBITMAP			hbmOld, hbmOff, hbmOldOff;
	POINT			pt;
	PSLIDER_INFO	pSliderInfo;
	int				nYslider, nXslider;
	char			szNumber[20];
	RECT			TextRect;
	int				OldMode;
	COLORREF		OldColour;
	HGDIOBJ			hOldFont;
	UINT			nY, nX, nTextHeight, nLen, i;
	char			szTitle[MAX_TITLE_LEN + 1];
	LPSTR			psz1, psz2;

	// Get the slider info
	pSliderInfo = (PSLIDER_INFO)GetWindowLong (hWnd, 0);

	// Calculate the top left position of the slider block
	nYslider = pSliderInfo->nYcurrent - (nSliderBitmapHeight / 2);
	nXslider = (nBackgroundWidth - nSliderBitmapWidth) / 2;

	// Get the current DC
	hDC = GetDC(hWnd);

	// Create an off screen DC and a bitmap the control size
	hdcOff = CreateCompatibleDC (hDC);
	hbmOff = CreateCompatibleBitmap (hDC, nBackgroundWidth, nBackgroundHeight);
	hbmOldOff = SelectObject (hdcOff, hbmOff);

	// Blit the slider background to the offscreen DC
	hdcMem = CreateCompatibleDC (hDC);
	hbmOld = SelectObject (hdcMem, hBackgroundBitmap);
	BitBlt (hdcOff, 0, 0, nBackgroundWidth, nBackgroundHeight, hdcMem, 0, 0, SRCCOPY);
	SelectObject (hdcMem, hbmOld);
	DeleteDC (hdcMem);

	// Blit the slider handle to the offscreen DC
	hdcMem = CreateCompatibleDC (hDC);
	hbmOld = SelectObject (hdcMem, hSliderBitmap);
	BitBlt (hdcOff, nXslider, nYslider, nSliderBitmapWidth, nSliderBitmapHeight, hdcMem, 0, 0, SRCCOPY);
	SelectObject (hdcMem, hbmOld);
	DeleteDC (hdcMem);

	// Calculate the bounding rectangle for the text box
	TextRect.top = nY = (2 * TRACK_YOFFSET) + TRACK_LENGTH + 2;
	TextRect.bottom = SLIDER_CONTROL_HEIGHT - 2;
	TextRect.left = nX = 2;
	TextRect.right = SLIDER_CONTROL_WIDTH - 2;

	// Output the current value
	itoa (pSliderInfo->nCurrent, &szNumber[0], 10);
	OldMode = SetBkMode (hdcOff, TRANSPARENT);
	OldColour = SetTextColor (hdcOff, RGB (0, 0, 0));
	hOldFont = SelectObject (hdcOff, hFont);
	ExtTextOut (hdcOff, nX, nY, ETO_CLIPPED, &TextRect, &szNumber[0], nLen = strlen (&szNumber[0]), NULL);
	nY += HIWORD (GetTextExtent (hdcOff, &szNumber[0], nLen)) - 2;

	// Break the title into 2 strings if necessary
	psz1 = &szTitle[0];
	psz2 = NULL;
	for (i=0 ; pSliderInfo->szTitle[i] != '\0' ; i++)
	{
		if ((szTitle[i] = pSliderInfo->szTitle[i]) == '\n')
		{
			szTitle[i] = '\0';
			psz2 = &szTitle[i + 1];
		}
	}
	szTitle[i] = '\0';

	// Output title string 1
	ExtTextOut (hdcOff, nX, nY, ETO_CLIPPED, &TextRect, psz1, nLen = strlen (psz1), NULL);

	// If there is a second title string
	if (psz2 != NULL)
	{
		// Output it
		nY += HIWORD (GetTextExtent (hdcOff, psz1, nLen)) - 2;
		ExtTextOut (hdcOff, nX, nY, ETO_CLIPPED, &TextRect, psz2, strlen (psz2), NULL);
	}

	SelectObject (hdcOff, hOldFont);
	SetTextColor (hdcOff, OldColour);
	SetBkMode (hdcOff, OldMode);

	// Blit the off-screen DC to the on-screen DC
	BitBlt (hDC, 0, 0, nBackgroundWidth, nBackgroundHeight, hdcOff, 0, 0, SRCCOPY);

	// Tidy up the off screen stuff
	SelectObject (hdcOff, hbmOldOff);
	DeleteObject (hbmOff);
	DeleteDC (hdcOff);

	// Get rid of the control's DC
	ReleaseDC (hWnd, hDC);
}
