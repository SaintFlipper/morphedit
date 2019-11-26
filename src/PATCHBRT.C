/*****************************************************************************
 File:			patchbrt.c
 Description:	The REAL-TIME patchbay module for the Morpheus editor.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 21/5/94	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define	NUM_REAL_TIME_SOURCES		13
#define	NUM_REAL_TIME_DESTINATIONS	35
#define	PIN_WIDTH					16
#define	PIN_HEIGHT					16

#define	HBAR_COLOUR					RGB(128,128,0)
#define	VBAR1_COLOUR				RGB(0,128,0)
#define	VBAR2_COLOUR				RGB(128,0,0)
#define	rgbBLACK					RGB(0,0,0)
#define	rgbWHITE					RGB(255,255,255)

/****************************************************************************/
/*								Local macros								*/
/****************************************************************************/

#define	HANDLE_AUCNTRL(id,var)						\
		if (ReadSysexWord (&var) != (int)lParam)	\
		{											\
			fDirtyPreset = TRUE;					\
			WriteSysexWord (&var, (int)lParam);		\
			PresetSendParam (hWnd, &var);			\
		}

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/


static LPCSTR pszRealTimeSources[] =
			{"Pitchwheel", "Control A", "Control B", "Control C", "Control D", "Mono pressure",
			 "Poly pressure", "LFO 1", "LFO 2", "Aux env.", "FG 1", "FG 2", "Free-run FG"};

static LPCSTR pszPatchDests[] =
			{"BPitch", "BPitch P", "BPitch S", "BVolume", "BVolume P", "BVolume S",
			 "BAttack", "BAttack P", "BAttack S", "BDecay", "BDecay P", "BDecay S",
			 "BRelease", "BRelease P", "BRelease S", "BXfade", "BLFO1 amt", "BLFO1 rate",
			 "BLFO2 amt", "BLFO2 rate", "BAux amt", "BAux att", "BAux dec", "BAux rel",
			 "NStart", "NStart P", "NStart S", "BPan", "BPan P", "BPan S", "NTone",
			 "NTone P", "NTone S", "BMorph", "BMorph P", "BMorph S", "NTrans2", "NTrans2 P", "NTrans2 S",
			 "BPort rt", "BPortRt P", "BPortRt S", "BFG1 amt", "BFG2 amt", "NFiltLev", "NFiltLev P",
			 "NFiltLev S", "NFreq trk", "NFreq trk P", "NFreq trk S"};

static int		nRealTimePatchIndex[NUM_REAL_TIME_DESTINATIONS];
static POINT	nPatchboardTopLeft;
static POINT	nPatchboardSize;
static POINT	nTotalSize;
static HFONT	hHorizontalRealTimeFont;
static HFONT	hRotatedRealTimeFont;
static HBITMAP	hbmpBackground;
static int		idDisplayedPreset = NO_PARAM;
static HWND		hwndPins[NUM_REAL_TIME_PATCHES] = {(HWND)NULL};
static HWND		hwndSelectedPin = (HWND)NULL;

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

DIALOG_PROC	RealTimePatchEditProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

static BOOL InitRealTimePatchWindow (HWND hWnd);
static VOID PaintBackground (HWND hWnd, HDC hDC);
static VOID PaintRealTimePatchbay (HWND hWnd, HDC hDC);
static VOID CreatePatchPins (VOID);
static VOID CreateSinglePin (int nPin, int nSource, int nDest, int nValue, BOOL fSelected);
static VOID LeftButton (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

/*^L*/
/*****************************************************************************
 Function:		CreateRealTimePatchBayWindow
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Creates the real-time patchbay window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 21/5/94 	| Created.											| PW
*****************************************************************************/

BOOL CreateRealTimePatchBayWindow (VOID)
{
	MDICREATESTRUCT	mc;
	WINDOWPLACEMENT	wp;

	GetCreatePosition (REAL_TIME_PATCHES_WINDOW, FALSE, &wp);

	// Create the window
	mc.szClass	= WCNAME_REAL_TIME_PATCHES;
	mc.szTitle	= ResourceString (IDS_RT_PATCH_WINDOW_TITLE);
	mc.hOwner	= hInst;
	mc.x		= wp.rcNormalPosition.left;
	mc.y		= wp.rcNormalPosition.top;
	mc.cx		= wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	mc.cy		= wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	mc.style	= WS_CHILD | WS_CAPTION | WS_CLIPSIBLINGS | WS_MINIMIZEBOX | WS_SYSMENU |
					(wp.showCmd & SW_SHOWMINIMIZED ? WS_MINIMIZE : 0);
	mc.lParam	= 0;
	if ((hwndRealTimePatches = (HWND)SendMessage (hwndClient,
									WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT)&mc)) == (HWND)NULL)
	{
		// Error
		MyMessageBox (NULL, ResourceString (IDS_RT_PATCH_ERR_CREAT_WND), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Reposition the window
	GetWindowPosition (REAL_TIME_PATCHES_WINDOW, hwndRealTimePatches, FALSE);

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		DestroyRealTimePatchBayWindow
 Parameters:	None.
 Returns:		None.
 Description:	Destroys the real-time patchbay window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/5/94 	| Created.											| PW
*****************************************************************************/

VOID DestroyRealTimePatchBayWindow (VOID)
{
	// Save the window position
	SaveWindowPosition (REAL_TIME_PATCHES_WINDOW, hwndRealTimePatches);

	// Destroy the window
	SendMessage (hwndClient, WM_MDIDESTROY, (WPARAM)hwndRealTimePatches, 0);
	hwndRealTimePatches = (HWND)NULL;
}

/*^L*/
/*****************************************************************************
 Function:		DisplayRealTimePatch
 Parameters:	NOne.
 Returns:		None.
 Description:	Forces the specified preset's note-on patches to be displayed.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 21/5/94 	| Created.											| PW
*****************************************************************************/

VOID DisplayRealTimePatch (VOID)
{
	// If the window doesn't exist
	if (hwndRealTimePatches == (HWND)NULL)
	{
		// Exit
		return;
	}

	// Create the pin windows
	CreatePatchPins ();

	// Force a redraw
	InvalidateRect (hwndRealTimePatches, NULL, TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		RealTimePatchBayWindowProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		LRESULT
 Description:	The REAL-TIME patchbay procedure.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 21/5/94 	| Created.											| PW
*****************************************************************************/

WINDOW_PROC RealTimePatchBayWindowProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	UINT		i;
	PAINTSTRUCT	ps;
	HDC			hDC;
	DLGPROC		DlgProc;
	HMENU		hMenu;

	switch (nMsg)
	{
		// Creating the window
		case WM_CREATE:
		{
			// Initialise
			if (!InitRealTimePatchWindow (hWnd))
			{
            	// Error
				return (-1);
			}

			// Change my size
			MoveWindow (hWnd, 0, 0,
						nTotalSize.x + 2 * GetSystemMetrics (SM_CXFRAME),
						nTotalSize.y + 2 * GetSystemMetrics (SM_CYFRAME) + GetSystemMetrics (SM_CYCAPTION),
						TRUE);

			// Do a NULL reformat t o get rid of the MAXIMIZE button
			ReformatDialog (hWnd, 0);

			// If I have a system menu
			if (GetWindowLong (hWnd, GWL_STYLE) & WS_SYSMENU)
			{
				// Get the system menu handle
				hMenu = GetSystemMenu (hWnd, FALSE);

				// Delete the CLOSE, SIZE, and MAXIMIZE items from the menu
				RemoveMenu (hMenu, 5, MF_BYPOSITION);			// Separator

				DeleteMenu (hMenu, SC_CLOSE, MF_BYCOMMAND);
				DeleteMenu (hMenu, SC_SIZE, MF_BYCOMMAND);
				DeleteMenu (hMenu, SC_MAXIMIZE, MF_BYCOMMAND);
			}

			ShowWindow  (hWnd, SW_HIDE);

			return (0);
		}

		// Trying to change the dialog width
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO FAR	*lpmm = (MINMAXINFO FAR *)lParam;
			int				cx, cy;

			cx = nTotalSize.x + 2 * GetSystemMetrics (SM_CXFRAME);
			cy = nTotalSize.y + 2 * GetSystemMetrics (SM_CYFRAME) + GetSystemMetrics (SM_CYCAPTION);

			lpmm->ptMinTrackSize.x	= cx;
			lpmm->ptMinTrackSize.y	= cy;
			lpmm->ptMaxTrackSize.x	= cx;
			lpmm->ptMaxTrackSize.y	= cy;
			break;
		}

		// Painting
		case WM_PAINT:
		{
			// Paint
			hDC = BeginPaint (hWnd, &ps);
			PaintRealTimePatchbay (hWnd, hDC);
			EndPaint (hWnd, &ps);
			return (0);
		}

		// Destruction
		case WM_DESTROY:
		{
			// Destroy any pins
			for (i=0 ; i<ARRAY_SIZE(hwndPins) ; i++)
			{
				if (hwndPins[i] != (HWND)NULL)
				{
					DestroyWindow (hwndPins[i]);
					hwndPins[i] = (HWND)NULL;
				}
			}

			// Destroy all created objects
			DeleteObject (hRotatedRealTimeFont);
			DeleteObject (hHorizontalRealTimeFont);
			DeleteObject (hbmpBackground);
			break;
		}

		// Left button down
		case WM_LBUTTONDOWN:
		{
			// Put myself of top of the other children
			SetWindowPos (hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

			// If no preset is on display
			if (nDisplayedPreset == INDEX_NONE)
			{
				// Go no further
				break;
			}

			// Call the handler
			LeftButton (hWnd, nMsg, wParam, lParam);

			break;
		}

		// Right button down
		case WM_RBUTTONDOWN:
		{
			PresetRightMouseButton (hWnd, LOWORD (lParam), HIWORD (lParam));
			break;
		}

		// A pin has been selected
		case PINN_SELECTED:
		{
			// If another pin is already selected
			if (hwndSelectedPin != (HWND)NULL)
			{
				// Kill the focus of the existing pin
				SendMessage (hwndSelectedPin, PIN_KILL_FOCUS, 0, 0);
			}

			// Extract the pin ID (also the child control ID) and save it as the newly selected window
			hwndSelectedPin = GetDlgItem (hWnd, LOWORD (wParam));

			// Run the patch editor
			DlgProc = MakeProcInstance ((FARPROC)RealTimePatchEditProc, hInst);
			DialogBoxParam (hInst, "DLG_PATCH_SLIDER", GetParent (hWnd), DlgProc, LOWORD (wParam));
			FreeProcInstance (DlgProc);

			return (0);;
		}

		// Default
		default:
		{
			break;
		}
	}

	// Default processing
	return (DefMDIChildProc (hWnd, nMsg, wParam, lParam));
}

/*^L*/
/*****************************************************************************
 Function:		InitRealTimePatchWindow
 Parameters:	HWND	hWnd
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Initialises the patchbay window stuff.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 20/5/94 	| Created.											| PW
*****************************************************************************/

static BOOL InitRealTimePatchWindow (HWND hWnd)
{
	int			nMaxExtent, nExtent;
	int			i, n;
	HFONT		hfntOld;
	HDC			hDC;
	LOGFONT		lf;
	int			nOldMode;

	// Fill up the list of patch destination indexes
	for (i=0, n=0 ; i<ARRAY_SIZE(pszPatchDests) ; i++)
	{
		if ((pszPatchDests[i][0] == 'B') || (pszPatchDests[i][0] == 'R'))
		{
			nRealTimePatchIndex[n++] = i;
		}
	}

	// Set the display mapping mode
	hDC = GetDC ((HWND)NULL);
	nOldMode = SetMapMode (hDC, MM_TEXT);

	// Create the horizontal font for the source titles
	memset (&lf, 0, sizeof (lf));
	lf.lfHeight			= 14;
	lf.lfWeight			= FW_NORMAL;
	lf.lfCharSet		= ANSI_CHARSET;
	lf.lfOutPrecision	= OUT_TT_PRECIS;
	lf.lfClipPrecision	= CLIP_LH_ANGLES | CLIP_DEFAULT_PRECIS;
	lf.lfQuality		= PROOF_QUALITY;
	lf.lfPitchAndFamily	= VARIABLE_PITCH | FF_DONTCARE;
	lf.lfEscapement		= 0;
	strcpy (&lf.lfFaceName[0], "Arial");
	if ((hHorizontalRealTimeFont = CreateFontIndirect (&lf)) == (HFONT)NULL)
	{
		return (FALSE);
	}

	// Create the rotated font for the destination titles
	memset (&lf, 0, sizeof (lf));
	lf.lfHeight			= 12;
	lf.lfWeight			= FW_NORMAL;
	lf.lfCharSet		= ANSI_CHARSET;
	lf.lfOutPrecision	= OUT_TT_PRECIS;
	lf.lfClipPrecision	= CLIP_LH_ANGLES | CLIP_DEFAULT_PRECIS;
	lf.lfQuality		= PROOF_QUALITY;
	lf.lfPitchAndFamily	= VARIABLE_PITCH | FF_DONTCARE;
	lf.lfEscapement		= 900;
	strcpy (&lf.lfFaceName[0], "Arial");
	if ((hRotatedRealTimeFont = CreateFontIndirect (&lf)) == (HFONT)NULL)
	{
		DeleteObject (hHorizontalRealTimeFont);
		return (FALSE);
	}

	// Reset the mapping mode
	SetMapMode (hDC, nOldMode);
	ReleaseDC ((HWND)NULL, hDC);

	// Run through the source titles to find the maximum string length
	hDC = GetDC (hWnd);
	hfntOld = SelectObject (hDC, hHorizontalRealTimeFont);
	nMaxExtent = 0;
	for (i=0 ; i<NUM_REAL_TIME_SOURCES ; i++)
	{
		SIZE	size;

		GetTextExtentPoint (hDC, &pszRealTimeSources[i][0], lstrlen (&pszRealTimeSources[i][0]), &size);
		nExtent = size.cx;
		if (nExtent > nMaxExtent)
		{
			nMaxExtent = nExtent;
		}
	}

	// Make the patchboard X offset 4 pixels longer than that
	nPatchboardTopLeft.x = nMaxExtent + 4;

	// Run through the destination titles to find the maximum string height
	SelectObject (hDC, hRotatedRealTimeFont);
	nMaxExtent = 0;
	for (i=0 ; i<NUM_REAL_TIME_DESTINATIONS ; i++)
	{
		if ((pszPatchDests[i][0] == 'B') || (pszPatchDests[i][0] == 'R'))
		{
			SIZE	size;

			GetTextExtentPoint (hDC, &pszPatchDests[i][1], lstrlen (&pszPatchDests[i][1]), &size);
			nExtent = size.cx;
			if (nExtent > nMaxExtent)
			{
				nMaxExtent = nExtent;
			}
		}
	}

	// Make the patchboard Y offset 8 pixels longer than that
	nPatchboardTopLeft.y = nMaxExtent + 8;

	// Clean up
	SelectObject (hDC, hfntOld);
	ReleaseDC (hWnd, hDC);

	// Calculate how big the board area should be
	nPatchboardSize.x = NUM_REAL_TIME_DESTINATIONS * PIN_WIDTH;
	nPatchboardSize.y = NUM_REAL_TIME_SOURCES * PIN_HEIGHT;

	// Calculate the total dimensions of the window
	nTotalSize.x = nPatchboardTopLeft.x + nPatchboardSize.x + 2;
	nTotalSize.y = nPatchboardTopLeft.y + nPatchboardSize.y + 2;

	// Load the bitmap for the background
	hbmpBackground = LoadBitmap (hInst, "BMP_PATCHBAY_BACKGROUND");

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		PaintRealTimePatchbay
 Parameters:	HWND	hWnd
				HDC		hDC
 Returns:		None.
 Description:	Repaints the entire REAL-TIME patchbay window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 21/5/94 	| Created.											| PW
*****************************************************************************/

static VOID PaintRealTimePatchbay (HWND hWnd, HDC hDC)
{
	// Paint the background
	PaintBackground (hWnd, hDC);
}

/*^L*/
/*****************************************************************************
 Function:		PaintBackground
 Parameters:	HWND	hWnd
				HDC		hDC
 Returns:		None.
 Description:	Repaints the entire REAL-TIME patchbay window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 20/5/94 	| Created.											| PW
*****************************************************************************/

static VOID PaintBackground (HWND hWnd, HDC hDC)
{
	int			i, ix, j;
	int			nYoffset, nXoffset;
	RECT		Rect;
	HFONT		hfntOld;
	int			nOldMode;
	COLORREF	OldCol;
	HPEN		hpenOld;

	// Save current parameters
	hfntOld		= SelectObject (hDC, hHorizontalRealTimeFont);
	nOldMode	= SetBkMode (hDC, TRANSPARENT);
	OldCol		= SetTextColor (hDC, GetSysColor (COLOR_WINDOWTEXT));
	hpenOld		= SelectObject (hDC, GetStockObject (NULL_PEN));

	/*
		Draw the background of the patch area
	 */
	for (j=0, nYoffset=nPatchboardTopLeft.y ; j<NUM_REAL_TIME_SOURCES ; j++, nYoffset+=PIN_HEIGHT)
    {
		for (i=0, nXoffset=nPatchboardTopLeft.x ; i<NUM_REAL_TIME_DESTINATIONS ; i++, nXoffset+=PIN_WIDTH)
		{
			DrawBitmap (hDC, hbmpBackground, nXoffset, nYoffset);
		}
	}

	/*
		Paint the DESTINATION labels
	 */
	nXoffset = nPatchboardTopLeft.x;
	SelectObject (hDC, hRotatedRealTimeFont);
	for (i=0 ; i<NUM_REAL_TIME_DESTINATIONS ; i++)
	{
		// Draw the label
		ix = nRealTimePatchIndex[i];
		TextOut (hDC, nXoffset, nPatchboardTopLeft.y - 5, &pszPatchDests[ix][1], lstrlen (&pszPatchDests[ix][1]));

		// Step right
		nXoffset += PIN_WIDTH;
	}

	/*
		Paint the SOURCE labels
	 */
	nYoffset = nPatchboardTopLeft.y;
	SelectObject (hDC, hHorizontalRealTimeFont);
	for (i=0 ; i<NUM_REAL_TIME_SOURCES ; i++)
	{
		// Create a bounding rectangle for the label
		Rect.left	= 2;
		Rect.top	= nYoffset;
		Rect.right	= nPatchboardTopLeft.x - 2;
		Rect.bottom	= nYoffset + PIN_HEIGHT;

		// Draw the label
		DrawText (hDC, pszRealTimeSources[i], lstrlen (pszRealTimeSources[i]), &Rect, DT_TOP | DT_RIGHT);

		// Step down
		nYoffset += PIN_HEIGHT;
	}

	// Restore previous parameters
	SelectObject (hDC, hfntOld);
	SetBkMode (hDC, nOldMode);
	SetTextColor (hDC, OldCol);
	SelectObject (hDC, hpenOld);
}

/*^L*/
/*****************************************************************************
 Function:		CreatePatchPins
 Parameters:	None.
 Returns:		None.
 Description:	Creates the REAL-TIME patchbay patchpins.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 21/5/94 	| Created.											| PW
*****************************************************************************/

static VOID CreatePatchPins (VOID)
{
	int		i;

	// Destroy any existing pins
	for (i=0 ; i<ARRAY_SIZE(hwndPins) ; i++)
	{
		if (hwndPins[i] != (HWND)NULL)
		{
			DestroyWindow (hwndPins[i]);
			hwndPins[i] = (HWND)NULL;
		}
	}

	// If no preset is on display
	if (nDisplayedPreset == INDEX_NONE)
	{
		// Go no further
		return;
	}

	// For each possible patch
	for (i=0 ; i<NUM_REAL_TIME_PATCHES ; i++)
	{
		// If the patch destination is not "OFF"
		if (WNUM (EditPreset.RealTimeModulation[i].Destination) != D_OFF)
		{
			// Create a single pin
			CreateSinglePin (i,
				WNUM (EditPreset.RealTimeModulation[i].Source),
				WNUM (EditPreset.RealTimeModulation[i].Destination) - 1,
				WNUM (EditPreset.RealTimeModulation[i].Amount),
				FALSE);
		}
	}
}

/*^L*/
/*****************************************************************************
 Function:		CreateSinglePin
 Parameters:	int		nPin			Pin index (into hwndPins[])
				int		nSource			Source index.
				int		nDest			Destination index.
				int		nValue			Current value.
				BOOL	fSelected		Whether selected or not.
 Returns:		None.
 Description:	Creates a single patchbay patchpin.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 21/5/94 	| Created.											| PW
*****************************************************************************/

static VOID CreateSinglePin (int nPin, int nSource, int nDest, int nValue, BOOL fSelected)
{
	int		ix;

	// What a pain - search the list of destinations for this destination
	for (ix=0 ; ix<NUM_REAL_TIME_DESTINATIONS ; ix++)
	{
		if (nRealTimePatchIndex[ix] == nDest)
		{
			break;
		}
	}
	if (ix == NUM_REAL_TIME_DESTINATIONS)
	{
		return;
	}

	// Create the window
	if ((hwndPins[nPin] = CreateWindow (PIN_CLASS_NAME, "", WS_CHILD | WS_VISIBLE,
					nPatchboardTopLeft.x + ix * PIN_WIDTH, nPatchboardTopLeft.y + nSource * PIN_HEIGHT,
					PIN_WIDTH, PIN_HEIGHT, hwndRealTimePatches, (HMENU)nPin, hInst, NULL)) != (HWND)NULL)
	{
		// Set its current value
		SendMessage (hwndPins[nPin], PIN_SET_CURRENT, nValue, 0);

		// If it is to be selected immediately
		if (fSelected)
		{
			// If there is already a selected pin
			if (hwndSelectedPin != (HWND)NULL)
			{
				// Kill the focus of the existing pin
				SendMessage (hwndSelectedPin, PIN_KILL_FOCUS, 0, 0);
			}

			// Set the focus of the new pin
			SendMessage (hwndSelectedPin = hwndPins[nPin], PIN_SET_FOCUS, 0, 0);
		}
	}
}

/*^L*/
/*****************************************************************************
 Function:		RealTimePatchEditProc
 Parameters:	HWND	hWnd			(the usual)
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if processed, else FALSE.
 Description:	The dialog procedure for editing a patch.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 21/5/94 	| Created.											| PW
*****************************************************************************/

DIALOG_PROC RealTimePatchEditProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	static int		nPatch;
	static BOOL		fInitialised;
	RECT			PinRect, DlgRect, DesktopRect;
	int				nMyLeft, nMyTop;
	char			szPatchBuf[100];

	switch (nMsg)
	{
		// Creation
		case WM_INITDIALOG:
		{
			// Not initialised yet
			fInitialised = FALSE;

			// Reformat myself
			ReformatDialog (hWnd, RDF_THINFONT);

			// Which PRESET and PATCH NUMBER is it ?
			nPatch = (int)lParam;

			// Find out where the patch pin is
			GetWindowRect (hwndPins[nPatch], &PinRect);

			// Get my window area
			GetWindowRect (hWnd, &DlgRect);

			// Get the desktop area
			GetWindowRect (GetDesktopWindow (), &DesktopRect);

			// If my bottom might go off the bottom of the screen
			if (PinRect.top + (DlgRect.bottom - DlgRect.top) > DesktopRect.bottom)
			{
				// My top should be my height above the screen bottom
				nMyTop = DesktopRect.bottom - (DlgRect.bottom - DlgRect.top);
			}
			else
			{
				// My top should be the same as my patch pin
				nMyTop = PinRect.top;
			}

			// If my right might go off the edge of the screen
			if (PinRect.right + (DlgRect.right - DlgRect.left) + 5 > DesktopRect.right)
			{
				// Put me to the left of the patch pin
				nMyLeft = PinRect.left - (DlgRect.right - DlgRect.left) - 5;
			}
			else
			{
				// Put me to the right of the patch pin
				nMyLeft = PinRect.right + 5;
			}

			// Move myself
			SetWindowPos (hWnd, HWND_TOP, nMyLeft, nMyTop, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

			// Set up the slider
			SendDlgItemMessage (hWnd, IDC_PATCH_LEVEL, SL_SET_MIN, (WPARAM)-128, 0);
			SendDlgItemMessage (hWnd, IDC_PATCH_LEVEL, SL_SET_MAX, 127, 0);
			SendDlgItemMessage (hWnd, IDC_PATCH_LEVEL, SL_SET_CURRENT, WNUM (EditPreset.RealTimeModulation[nPatch].Amount), 0);

			// Now initialised
			fInitialised = TRUE;

			return (TRUE);
		}

		// Control
		case WM_COMMAND:
		{
			switch (LOWORD (wParam))
			{
				// OK
				case IDOK:
				{
					// Kill the focus of the edited pin
					SendMessage (hwndSelectedPin, PIN_KILL_FOCUS, 0, 0);
					hwndSelectedPin = FALSE;

					// Update the main preset dialog listbox
					FormatPatchLine (RT, nPatch, &szPatchBuf[0]);
					SendDlgItemMessage (hwndCurrentPreset, IDC_MISC_REAL_TIME_PATCH_LISTBOX, LB_DELETESTRING, nPatch, 0);
					SendDlgItemMessage (hwndCurrentPreset, IDC_MISC_REAL_TIME_PATCH_LISTBOX,
										LB_INSERTSTRING, nPatch, (LPARAM)(LPCSTR)&szPatchBuf[0]);
					SendDlgItemMessage (hwndCurrentPreset, IDC_MISC_REAL_TIME_PATCH_LISTBOX, LB_SETCURSEL, nPatch, 0);

					// Exit
					EndDialog (hWnd, 0);
					return (TRUE);
				}

				// Delete
				case IDC_DELETE:
				{
					// Mark this patch's destination as OFF
					WriteSysexWord (&EditPreset.RealTimeModulation[nPatch].Destination, D_OFF);

					// And make sure the Morpheus knows
					PresetSendParam (hWnd, &EditPreset.RealTimeModulation[nPatch].Destination);

					// Mark the preset as dirty
					fDirtyPreset = TRUE;

					// Delete the pin window
					DestroyWindow (hwndPins[nPatch]);
					hwndPins[nPatch] = (HWND)NULL;

					// Update the main preset dialog listbox
					FormatPatchLine (RT, nPatch, &szPatchBuf[0]);
					SendDlgItemMessage (hwndCurrentPreset, IDC_MISC_REAL_TIME_PATCH_LISTBOX, LB_DELETESTRING, nPatch, 0);
					SendDlgItemMessage (hwndCurrentPreset, IDC_MISC_REAL_TIME_PATCH_LISTBOX,
										LB_INSERTSTRING, nPatch, (LPARAM)(LPCSTR)&szPatchBuf[0]);
					SendDlgItemMessage (hwndCurrentPreset, IDC_MISC_REAL_TIME_PATCH_LISTBOX, LB_SETCURSEL, nPatch, 0);


					// Exit
					EndDialog (hWnd, 0);
					return (TRUE);
				}

				// Unknown control
				default:
				{
                	break;
                }
			}

			break;
		}

		// Slider has been moved
		case SLN_NEW_VALUE:
		{
			// Only do the job if we are initialised
			if (fInitialised)
            {
				// Set the level on the pin window too
				SendMessage (hwndPins[nPatch], PIN_SET_CURRENT, (WPARAM)(int)(SHORT)LOWORD (lParam), 0);

				// Send the parameter 
				HANDLE_AUCNTRL (wParam, EditPreset.RealTimeModulation[nPatch].Amount);
			}

			return (TRUE);
		}

		// Control about to be drawn
		case WM_CTLCOLOR:
		{
			Ctl3dCtlColorEx (nMsg, wParam, lParam);
		}

		// Default
		default:
		{
        	break;
		}
	}

	// Unprocessed
	return (FALSE);
}

/*^L*/
/*****************************************************************************
 Function:		LeftButton
 Parameters:	HWND	hWnd			(the usual)
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		None.
 Description:	Handles the left mouse button.
				Has a look to see if we are within the patchboard area.
				If we ARE then it looks for a spare PATCH, and if it finds one
				then it creates a pin window and immediately pops up the
                editor for it.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 21/5/94 	| Created.											| PW
*****************************************************************************/

static VOID LeftButton (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	int				nPatch, nSource, nDest;
	int				x, y;
	DLGPROC			DlgProc;

	// Extract the mouse position
	x = LOWORD (lParam);
	y = HIWORD (lParam);

	// If we aren't within the patchboard area
	if ((x < nPatchboardTopLeft.x) || (x > nPatchboardTopLeft.x + nPatchboardSize.x) ||
		(y < nPatchboardTopLeft.y) || (y > nPatchboardTopLeft.y + nPatchboardSize.y))
	{
		return;
	}

	// Work out what the SOURCE and DESTINATION of the patch are
	nSource = ((y - nPatchboardTopLeft.y) / PIN_HEIGHT);
	nDest = nRealTimePatchIndex[((x - nPatchboardTopLeft.x) / PIN_WIDTH)];

	// For each possible patch
	for (nPatch=0 ; nPatch<NUM_NOTE_ON_PATCHES ; nPatch++)
	{
		// If the patch is unused
		if (hwndPins[nPatch] == (HWND)NULL)
		{
			// Create the patch pin
			CreateSinglePin (nPatch, nSource, nDest, 0, TRUE);

			// Set up the default patch in the current preset memory and send to the Morpheus
			WriteSysexWord (&EditPreset.RealTimeModulation[nPatch].Amount, 0);
			PresetSendParam (hWnd, &EditPreset.RealTimeModulation[nPatch].Amount);
			WriteSysexWord (&EditPreset.RealTimeModulation[nPatch].Source, nSource);
			PresetSendParam (hWnd, &EditPreset.RealTimeModulation[nPatch].Source);
			WriteSysexWord (&EditPreset.RealTimeModulation[nPatch].Destination, nDest + 1);
			PresetSendParam (hWnd, &EditPreset.RealTimeModulation[nPatch].Destination);

			// Run the patch editor
			DlgProc = MakeProcInstance ((FARPROC)RealTimePatchEditProc, hInst);
			DialogBoxParam (hInst, "DLG_PATCH_SLIDER", GetParent (hWnd), DlgProc, nPatch);
			FreeProcInstance (DlgProc);

			// Done
			break;
		}
	}
}

