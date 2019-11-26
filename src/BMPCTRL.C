/*****************************************************************************
 File:			bmpctrl.c
 Description:	Bitmap custom control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 21/10/94	| Created.											| PW
*****************************************************************************/

#include <string.h>
#include <stdlib.h>

#include <windowsx.h>

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

/****************************************************************************/
/*								Local types									*/
/****************************************************************************/

/****************************************************************************/
/*								Global data									*/
/****************************************************************************/

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

LRESULT CALLBACK StaticBitmapControlProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

/*^L*/
/*****************************************************************************
 Function:		StaticBitmapControlInit
 Parameters:	HINSTANCE	hInst		DLL instance.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Creates the static bitmap control class.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 21/10/94 	| Created.											| PW
*****************************************************************************/

BOOL StaticBitmapControlInit (HINSTANCE hInst)
{
	WNDCLASS	wc;

	// Register the window class
	wc.lpszClassName	= STATIC_BITMAP_CLASS;
	wc.style			= CS_GLOBALCLASS;
	wc.hCursor			= LoadCursor (NULL, IDC_ARROW);
	wc.hIcon			= NULL;
	wc.lpszMenuName		= NULL;
	wc.hbrBackground	= NULL;
	wc.hInstance		= hInst;
	wc.lpfnWndProc		= StaticBitmapControlProc;
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
 Function:		StaticBitmapControlTerm
 Parameters:	HINSTANCE	hInst
 Returns:		None.
 Description:	Winds up the the static bitmap control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 21/10/94 	| Created.											| PW
*****************************************************************************/

VOID StaticBitmapControlTerm (HINSTANCE hInst)
{
	// Unregister the class
	UnregisterClass (STATIC_BITMAP_CLASS, hInst);
}

/*^L*/
/*****************************************************************************
 Function:		StaticBitmapControlProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		LONG					Window return value.
 Description:	The static bitmap control procedure.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 21/10/94 	| Created.											| PW
*****************************************************************************/

LRESULT CALLBACK StaticBitmapControlProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT			ps;
	HDC					hDC;
	HBITMAP				hBitmap;

	switch (nMsg)
	{
		// Creation
		case WM_CREATE:
		{
			LPCREATESTRUCT	pCs = (LPCREATESTRUCT)lParam;

			// Load the bitmap
			hBitmap	= LoadBitmap (hInst, pCs->lpszName);

			// Save the bitmap in the window's extra memory
			SetWindowLong (hWnd, 0, (LONG)hBitmap);

			// OK
			return (0);
		}

		// Subcontrol
		case WM_COMMAND:
		{
			break;
		}

		// Repaint
		case WM_PAINT:
		{
			hDC = BeginPaint (hWnd, &ps);
			DrawBitmap (hDC, (HBITMAP)GetWindowLong (hWnd, 0), 0, 0);
			EndPaint (hWnd, &ps);
			break;
		}

		// Destruction
		case WM_DESTROY:
		{
			// Unload the bitmap
			DeleteObject ((HBITMAP)GetWindowLong (hWnd, 0));
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

