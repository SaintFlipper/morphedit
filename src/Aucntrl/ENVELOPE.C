/*****************************************************************************
 File:			envelope.c
 Description:	The ENVELOPE control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 31/3/94	| Created.											| PW
*****************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include "aucntrl.h"
#include "auc_i.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define	NORMALISED_WIDTH		500
#define	NORMALISED_HEIGHT		100
#define	SUSTAIN_WIDTH			99

#define	MIN_TIME				0
#define	MAX_TIME				99
#define	MIN_LEVEL				0
#define	MAX_LEVEL				99

/****************************************************************************/
/*								Local types									*/
/****************************************************************************/

typedef struct
{
	UINT	nWidth;						// Control width
	UINT	nHeight;					// Control height
	int		nAttack;
	int		nHold;
	int		nDecay;
	int		nSustain;
	int		nRelease;
} ENVELOPE_INFO, FAR *PENVELOPE_INFO;

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

LRESULT CALLBACK EnvelopeControlProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
static VOID DrawEnvelope (HWND hWnd);

/*^L*/
/*****************************************************************************
 Function:		EnvelopeControlInit
 Parameters:	HINSTANCE	hInst		Current instance.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Initialises the envelope control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 10/4/94 	| Created.											| PW
*****************************************************************************/

BOOL EnvelopeControlInit (HINSTANCE hInst)
{
	WNDCLASS	wc;

	// Register the window class
	wc.lpszClassName	= ENVELOPE_CLASS_NAME;
	wc.style			= CS_GLOBALCLASS;
	wc.hCursor			= LoadCursor (NULL, IDC_ARROW);
	wc.hIcon			= NULL;
	wc.lpszMenuName		= NULL;
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wc.hInstance		= hInst;
	wc.lpfnWndProc		= EnvelopeControlProc;
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
 Function:		EnvelopeControlTerm
 Parameters:	HINSTANCE	hInst		Current instance.
 Parameters:	None.
 Returns:		None.
 Description:	Winds up the the envelope control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 10/4/94 	| Created.											| PW
*****************************************************************************/

VOID EnvelopeControlTerm (HINSTANCE hInst)
{
	// Unregister the class
	UnregisterClass (ENVELOPE_CLASS_NAME, hInst);
}

/*^L*/
/*****************************************************************************
 Function:		EnvelopeControlProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		LONG					Window return value.
 Description:	The envelope control procedure.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 10/4/94 	| Created.											| PW
*****************************************************************************/

LRESULT CALLBACK EnvelopeControlProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT			ps;
	PENVELOPE_INFO		pEnvelopeInfo;
	int					nValue;

	// Retrieve what may be the control info
	pEnvelopeInfo = (PENVELOPE_INFO)GetWindowLong (hWnd, 0);

	switch (nMsg)
	{
		// Creation
		case WM_CREATE:
		{
			LPCREATESTRUCT	pCs = (LPCREATESTRUCT)lParam;

			// Allocate space for the control info
			pEnvelopeInfo = (PENVELOPE_INFO)(LPVOID)GlobalAllocPtr (GPTR, sizeof (ENVELOPE_INFO));

            // If we got some memory
			if (pEnvelopeInfo != NULL)
			{
            	// Save the pointer in the window's extra memory
				SetWindowLong (hWnd, 0, (LONG)(LPVOID)pEnvelopeInfo);

				// Set the default info
				pEnvelopeInfo->nWidth			= pCs->cx;
				pEnvelopeInfo->nHeight			= pCs->cy;
				pEnvelopeInfo->nAttack			= 0;
				pEnvelopeInfo->nHold			= 10;
				pEnvelopeInfo->nDecay			= 30;
				pEnvelopeInfo->nSustain			= 50;
				pEnvelopeInfo->nRelease			= 30;

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

		// New attack value
		case ENV_SET_ATTACK:
		{
			nValue = (int)wParam;
			if (nValue < MIN_TIME)
				nValue = MIN_TIME;
			else if (nValue > MAX_TIME)
				nValue = MAX_TIME;
			pEnvelopeInfo->nAttack = nValue;
			DrawEnvelope (hWnd);
			break;
		}

		// New hold value
		case ENV_SET_HOLD:
		{
			nValue = (int)wParam;
			if (nValue < MIN_TIME)
				nValue = MIN_TIME;
			else if (nValue > MAX_TIME)
				nValue = MAX_TIME;
			pEnvelopeInfo->nHold = nValue;
			DrawEnvelope (hWnd);
			break;
		}

		// New decay value
		case ENV_SET_DECAY:
		{
			nValue = (int)wParam;
			if (nValue < MIN_TIME)
				nValue = MIN_TIME;
			else if (nValue > MAX_TIME)
				nValue = MAX_TIME;
			pEnvelopeInfo->nDecay = nValue;
			DrawEnvelope (hWnd);
			break;
		}

		// New sustain value
		case ENV_SET_SUSTAIN:
		{
			nValue = (int)wParam;
			if (nValue < MIN_LEVEL)
				nValue = MIN_LEVEL;
			else if (nValue > MAX_LEVEL)
				nValue = MAX_LEVEL;
			pEnvelopeInfo->nSustain = nValue;
			DrawEnvelope (hWnd);
			break;
		}

		// New release value
		case ENV_SET_RELEASE:
		{
			nValue = (int)wParam;
			if (nValue < MIN_TIME)
				nValue = MIN_TIME;
			else if (nValue > MAX_TIME)
				nValue = MAX_TIME;
			pEnvelopeInfo->nRelease = nValue;
			DrawEnvelope (hWnd);
			break;
		}

		// Show control
		case WM_SHOWWINDOW:
		{
			DrawEnvelope (hWnd);
			break;
		}

		// Repaint
		case WM_PAINT:
		{
			BeginPaint (hWnd, &ps);
			DrawEnvelope (hWnd);
			EndPaint (hWnd, &ps);
			break;
		}

		// Erase background
		case WM_ERASEBKGND:
		{
			// Say we erased it
			return (TRUE);
		}

		// Destroy
		case WM_DESTROY:
		{
			LocalFree ((HLOCAL)pEnvelopeInfo);
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
 Function:		DrawEnvelope
 Parameters:	HWND	hWnd			Control window.
 Returns:		None.
 Description:	Redraws the control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 10/4/94 	| Created.											| PW
*****************************************************************************/

static VOID DrawEnvelope (HWND hWnd)
{
	PENVELOPE_INFO	pEnvelopeInfo = (PENVELOPE_INFO)GetWindowLong (hWnd, 0);
	HPEN			hOldPen, hTextPen, hNullPen;
	HBRUSH			hOldBrush, hBkBrush;
	HDC				hDC;
	int				x, y;

	// Get the control's DC
	hDC = GetDC (hWnd);

	// Create a brush of the background colour and a null pen
	hBkBrush	= CreateSolidBrush (GetBkColor (hDC));
	hNullPen	= GetStockObject (NULL_PEN);
	hOldBrush	= SelectObject (hDC, hBkBrush);
	hOldPen		= SelectObject (hDC, hNullPen);

	// Erase the background
	Rectangle (hDC, 0, 0, pEnvelopeInfo->nWidth + 1, pEnvelopeInfo->nHeight + 1);

	// Select a text-colour pen for line drawing
	hTextPen = CreatePen (PS_SOLID, 0, GetTextColor (hDC));
	SelectObject (hDC, hTextPen);

	// Start drawing the envelope at the left-bottom
	x = 0;
	y = pEnvelopeInfo->nHeight - 1;
	MoveToEx (hDC, x, y, NULL);

	// Calculate and draw to the top of the ATTACK
	x = (int)(((long)pEnvelopeInfo->nAttack * (long)pEnvelopeInfo->nWidth) /
				(long)NORMALISED_WIDTH);
	y = 0;
	LineTo (hDC, x, y);

	// Calculate and draw to the end of the HOLD
	x = x + (int)(((long)pEnvelopeInfo->nHold * (long)pEnvelopeInfo->nWidth) /
				(long)NORMALISED_WIDTH);
	y = 0;
	LineTo (hDC, x, y);

	// Calculate and draw to the end of the DECAY
	x = x + (int)(((long)pEnvelopeInfo->nDecay * (long)pEnvelopeInfo->nWidth) /
				(long)NORMALISED_WIDTH);
	y = (int)(((long)(MAX_LEVEL - pEnvelopeInfo->nSustain) * (long)pEnvelopeInfo->nHeight) /
				(long)NORMALISED_HEIGHT);
	if (y >= (int)pEnvelopeInfo->nHeight)
	{
		y = pEnvelopeInfo->nHeight - 1;
    }
	LineTo (hDC, x, y);

	// Calculate and draw to the end of the SUSTAIN
	x = x + (int)(((long)SUSTAIN_WIDTH * (long)pEnvelopeInfo->nWidth) /
				(long)NORMALISED_WIDTH);
	LineTo (hDC, x, y);

	// Calculate and draw to the end of the RELEASE
	x = x + (int)(((long)pEnvelopeInfo->nRelease * (long)pEnvelopeInfo->nWidth) /
				(long)NORMALISED_WIDTH);
	y = pEnvelopeInfo->nHeight - 1;
	LineTo (hDC, x, y);

	// Select the previous brush and pen, and delete all brushes and pens created
	SelectObject (hDC, hOldPen);
	SelectObject (hDC, hOldBrush);
	DeleteObject (hTextPen);
	DeleteObject (hBkBrush);

	// Delete the DC
	ReleaseDC (hWnd, hDC);
}
