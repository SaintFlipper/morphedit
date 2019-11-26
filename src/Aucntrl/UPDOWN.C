/*****************************************************************************
 File:			updown.c
 Description:	The UP-DOWN control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/4/94		| Created.											| PW
*****************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include "aucntrl.h"
#include "auc_i.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define	DEFAULT_UPDOWN_MIN		0
#define	DEFAULT_UPDOWN_MAX		127
#define	DEFAULT_UPDOWN_CURRENT	0

#define	STATE_NORMAL			0
#define	STATE_UP				1
#define	STATE_DOWN				2

#define	CONTROL_TIMER_ID		1
#define	FIRST_TIMEOUT_VALUE		(1000 / 2)
#define	REPEAT_TIMEOUT_VALUE	(1000 / 20)


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

	HWND	hwndNumber;					// Associated "value" window
	VALUE_DISPLAY_FUNCTION	pDispFn;	// Associated "display value" function
} UPDOWN_INFO, FAR *PUPDOWN_INFO;

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

static HBITMAP	hUpdownNormalBitmap;
static HBITMAP	hUpdownUpBitmap;
static HBITMAP	hUpdownDownBitmap;
static UINT		nUpdownBitmapWidth;
static UINT		nUpdownBitmapHeight;

static int		nCaptureState = STATE_NORMAL;
static BOOL		fFirstTimeout;

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

LONG FAR PASCAL UpdownControlProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static VOID NewUpdownValue (HWND hWnd, int nState, BOOL fTellParent);
static VOID DrawUpdownControl (HWND hWnd, int nState);
static VOID AddToCurrent (HWND hWnd, int nOffset);

/*^L*/
/*****************************************************************************
 Function:		UpdownControlInit
 Parameters:	HINSTANCE	hInst		Current instance.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Initialises the up-down control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/4/94 	| Created.											| PW
*****************************************************************************/

BOOL UpdownControlInit (HINSTANCE hInst)
{
	WNDCLASS	wc;
	BITMAP		bm;

	// Load the up-down bitmap(s)
	if ((hUpdownNormalBitmap = LoadBitmap (hInst, (LPCSTR)"DIB_UPDOWN_NORMAL")) == (HBITMAP)NULL)
	{
		// Error
		return (FALSE);
	}
	if ((hUpdownUpBitmap = LoadBitmap (hInst, (LPCSTR)"DIB_UPDOWN_UP")) == (HBITMAP)NULL)
	{
		// Error
		return (FALSE);
	}
	if ((hUpdownDownBitmap = LoadBitmap (hInst, (LPCSTR)"DIB_UPDOWN_DOWN")) == (HBITMAP)NULL)
	{
		// Error
		return (FALSE);
	}

	// Find out how big the bitmap is
	GetObject (hUpdownNormalBitmap, sizeof (BITMAP), (LPSTR)&bm);
	nUpdownBitmapWidth	= bm.bmWidth;
	nUpdownBitmapHeight	= bm.bmHeight;

	// Register the window class
	wc.lpszClassName	= UPDOWN_CLASS_NAME;
	wc.style			= CS_GLOBALCLASS;
	wc.hCursor			= LoadCursor (NULL, IDC_ARROW);
	wc.hIcon			= NULL;
	wc.lpszMenuName		= NULL;
	wc.hbrBackground	= NULL;
	wc.hInstance		= hInst;
	wc.lpfnWndProc		= UpdownControlProc;
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
 Function:		UpdownControlTerm
 Parameters:	HINSTANCE	hInst
 Returns:		None.
 Description:	Winds up the the UP-DOWN control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/4/94 	| Created.											| PW
*****************************************************************************/

VOID UpdownControlTerm (HINSTANCE hInst)
{
	// Get rid of the bitmaps
	DeleteObject (hUpdownNormalBitmap);
	DeleteObject (hUpdownUpBitmap);
	DeleteObject (hUpdownDownBitmap);

	// Unregister the class
	UnregisterClass (UPDOWN_CLASS_NAME, hInst);
}

/*^L*/
/*****************************************************************************
 Function:		UpdownControlProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		LONG					Window return value.
 Description:	The UP-DOWN control procedure.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/4/94 	| Created.											| PW
*****************************************************************************/

LRESULT CALLBACK UpdownControlProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT			ps;
	PUPDOWN_INFO		pUpdownInfo;

	// Retrieve what may be the control info
	pUpdownInfo = (PUPDOWN_INFO)GetWindowLong (hWnd, 0);

	switch (nMsg)
	{
		// Creation
		case WM_CREATE:
		{
			LPCREATESTRUCT	pCs = (LPCREATESTRUCT)lParam;

			// Fix the window size
			pCs->cx = nUpdownBitmapWidth;
			pCs->cy = nUpdownBitmapHeight;

			// Allocate space for the control info
			pUpdownInfo = (PUPDOWN_INFO)(LPVOID)GlobalAllocPtr (GPTR, sizeof (UPDOWN_INFO));

            // If we got some memory
			if (pUpdownInfo != NULL)
			{
            	// Save the pointer in the window's extra memory
				SetWindowLong (hWnd, 0, (LONG)(LPVOID)pUpdownInfo);

				// Set the default info
				pUpdownInfo->nMin				= DEFAULT_UPDOWN_MIN;
				pUpdownInfo->nMax				= DEFAULT_UPDOWN_MAX;
				pUpdownInfo->nCurrent			= DEFAULT_UPDOWN_CURRENT;

				pUpdownInfo->nWidth				= pCs->cx;
				pUpdownInfo->nHeight			= pCs->cy;

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
			if (nCaptureState == STATE_NORMAL)
			{
				// If we are in the UP pad area
				if (HIWORD (lParam) < (pUpdownInfo->nHeight / 2))
				{
					// If we aren't already at the top
					if (pUpdownInfo->nCurrent < pUpdownInfo->nMax)
					{
						// Increment the current value
						pUpdownInfo->nCurrent++;

						// Capture it
						SetCapture (hWnd);
						nCaptureState = STATE_UP;

						// Start the repeater timeout
						fFirstTimeout = TRUE;
						SetTimer (hWnd, CONTROL_TIMER_ID, FIRST_TIMEOUT_VALUE, NULL);

						// New control value
						NewUpdownValue (hWnd, nCaptureState, TRUE);
					}

					// That's it
					return (0);
				}

				// If we are in the DOWN pad area
				if (HIWORD (lParam) >= (pUpdownInfo->nHeight / 2))
				{
					// If we aren't already at the bottom
					if (pUpdownInfo->nCurrent > pUpdownInfo->nMin)
					{
						// Decrement the current value
						pUpdownInfo->nCurrent--;

						// Capture it
						SetCapture (hWnd);
						nCaptureState = STATE_DOWN;

						// Start the repeater timeout
						fFirstTimeout = TRUE;
						SetTimer (hWnd, CONTROL_TIMER_ID, FIRST_TIMEOUT_VALUE, NULL);

						// New control value
						NewUpdownValue (hWnd, nCaptureState, TRUE);
					}

					// That's it
					return (0);
				}
			}
            break;
		}

		// Mouse up
		case WM_LBUTTONUP:
		{
        	// If it's captured
			if (nCaptureState != STATE_NORMAL)
			{
            	// Uncapture it
				ReleaseCapture ();
				nCaptureState = STATE_NORMAL;

                // Redraw it
				DrawUpdownControl (hWnd, nCaptureState);

				// Stop the repeat timer
				KillTimer (hWnd, CONTROL_TIMER_ID);
				return (0);
			}
			break;
		}

		// Repeat timeout
		case WM_TIMER:
		{
			// If we are in the UP pad
			if (nCaptureState == STATE_UP)
			{
				// Increment the current value
				AddToCurrent (hWnd, 1);
			}
			// Else if we are in the DOWN pad
			else if (nCaptureState == STATE_DOWN)
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

		// New minimum value
		case UD_SET_MIN:
		{
			pUpdownInfo->nMin = (int)wParam;
			if (pUpdownInfo->nCurrent < pUpdownInfo->nMin)
			{
				pUpdownInfo->nCurrent = pUpdownInfo->nMin;
			}
			NewUpdownValue (hWnd, nCaptureState, FALSE);
			break;
		}

		// New maximum value
		case UD_SET_MAX:
		{
			pUpdownInfo->nMax = (int)wParam;
			if (pUpdownInfo->nCurrent > pUpdownInfo->nMax)
			{
				pUpdownInfo->nCurrent = pUpdownInfo->nMax;
			}
			NewUpdownValue (hWnd, nCaptureState, FALSE);
			break;
		}

		// New value
		case UD_SET_CURRENT:
		{
			pUpdownInfo->nCurrent = (int)wParam;
			if (pUpdownInfo->nCurrent < pUpdownInfo->nMin)
			{
				pUpdownInfo->nCurrent = pUpdownInfo->nMin;
			}
			else if (pUpdownInfo->nCurrent > pUpdownInfo->nMax)
			{
				pUpdownInfo->nCurrent = pUpdownInfo->nMax;
			}
			NewUpdownValue (hWnd, STATE_NORMAL, (BOOL)lParam);
			break;
		}

		// Set associated number displaying function + window
		case UD_SET_NUM_DISPLAY:
		{
			// Store the value window handle and function
			pUpdownInfo->hwndNumber	= (HWND)wParam;
			pUpdownInfo->pDispFn	= (VALUE_DISPLAY_FUNCTION)lParam;

			// If the "value" window is present
			if (pUpdownInfo->hwndNumber != (HWND)NULL)
			{
				// Call the conversion function to display it
				(* pUpdownInfo->pDispFn) (pUpdownInfo->hwndNumber, pUpdownInfo->nCurrent);
			}

			break;
		}

		// Show control
		case WM_SHOWWINDOW:
		{
			DrawUpdownControl (hWnd, nCaptureState);
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
			DrawUpdownControl (hWnd, nCaptureState);
			EndPaint (hWnd, &ps);
			break;
		}

		// Destroy
		case WM_DESTROY:
		{
			LocalFree ((HLOCAL)pUpdownInfo);
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
 Function:		NewUpdownValue
 Parameters:	HWND	hWnd			Control window.
 Returns:		None.
 Description:	Redraws the control with its new value and sends a notification
				to the parent.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/4/94 	| Created.											| PW
*****************************************************************************/

static VOID NewUpdownValue (HWND hWnd, int nState, BOOL fTellParent)
{
	PUPDOWN_INFO	pUpdownInfo = (PUPDOWN_INFO)GetWindowLong (hWnd, 0);

	// Draw the control
	DrawUpdownControl (hWnd, nState);

	// Notify the parent
	if (fTellParent)
	{
		SendMessage (GetParent (hWnd), UDN_NEW_VALUE, GETWNDID (hWnd), pUpdownInfo->nCurrent);
	}

	// If the "value" window is present
	if (pUpdownInfo->hwndNumber != (HWND)NULL)
	{
		// Call the conversion function to display it
		(* pUpdownInfo->pDispFn) (pUpdownInfo->hwndNumber, pUpdownInfo->nCurrent);
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
	PUPDOWN_INFO	pUpdownInfo = (PUPDOWN_INFO)GetWindowLong (hWnd, 0);

	// Modify the current value
	pUpdownInfo->nCurrent += nOffset;
	if (pUpdownInfo->nCurrent < pUpdownInfo->nMin)
	{
		pUpdownInfo->nCurrent = pUpdownInfo->nMin;
	}
	else if (pUpdownInfo->nCurrent > pUpdownInfo->nMax)
	{
		pUpdownInfo->nCurrent = pUpdownInfo->nMax;
	}

	// Notify the parent
	SendMessage (GetParent (hWnd), UDN_NEW_VALUE, GETWNDID (hWnd), pUpdownInfo->nCurrent);

	// If the "value" window is present
	if (pUpdownInfo->hwndNumber != (HWND)NULL)
	{
		// Call the conversion function to display it
		(* pUpdownInfo->pDispFn) (pUpdownInfo->hwndNumber, pUpdownInfo->nCurrent);
	}
}

/*^L*/
/*****************************************************************************
 Function:		DrawUpdownControl
 Parameters:	HWND	hWnd			Control window.
				int		nState			Current state.
 Returns:		None.
 Description:	Redraws the control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/4/94 	| Created.											| PW
*****************************************************************************/

static VOID DrawUpdownControl (HWND hWnd, int nState)
{
	BITMAP	bm;
	HDC		hDC, hdcMem;
	POINT	ptSize, ptOrg;
	HBITMAP	hBitMap;

	// Decide which bitmap to display
	if (nState == STATE_UP)
	{
		hBitMap = hUpdownUpBitmap;
	}
	else if (nState == STATE_DOWN)
	{
		hBitMap = hUpdownDownBitmap;
	}
	else
	{
		hBitMap = hUpdownNormalBitmap;
	}

	hDC = GetDC(hWnd);

	hdcMem = CreateCompatibleDC (hDC);
	SelectObject (hdcMem, hBitMap);
	SetMapMode (hdcMem, GetMapMode (hDC));

	GetObject (hBitMap, sizeof (BITMAP), (LPSTR)&bm);
	ptSize.x = bm.bmWidth;
	ptSize.y = bm.bmHeight;
	DPtoLP (hDC, &ptSize, 1);

	ptOrg.x = 0;
	ptOrg.y = 0;
	DPtoLP (hdcMem, &ptOrg, 1);

	BitBlt (hDC, 0, 0, ptSize.x, ptSize.y, hdcMem, ptOrg.x, ptOrg.y, SRCCOPY);

	DeleteDC (hdcMem);
	ReleaseDC (hWnd, hDC);
}
