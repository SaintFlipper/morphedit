/*****************************************************************************
 File:			dlgutils.c
 Description:	Dialog utility functions. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 29/3/94	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define MAXLEN_MENUNAME          64
#define MAXLEN_CLASSNAME         64
#define MAXLEN_CAPTIONTEXT       132
#define MAXLEN_TYPEFACE          64

#define	SMALL_DIALOG_FONT_NAME	"Morpheus dialog"
#define	SMALL_DIALOG_FONT_SIZE	8

#ifdef WINDOWS32
 #define GETHINST(hWnd)			((HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE))
 #define ROUNDPTR(ptr)			(LPVOID)(((DWORD)(ptr) + 3) & 0xFFFFFFFC)

#else
 #define GETHINST(hWnd)			((HINSTANCE) GetWindowWord( hWnd, GWW_HINSTANCE ))
#endif

#define DLGTOCLIENTX( x, units )   MulDiv( x, units, 4 )
#define DLGTOCLIENTY( y, units )   MulDiv( y, units, 8 )

/****************************************************************************/
/*								Local types									*/
/****************************************************************************/

typedef UINT (CALLBACK *COMMDLGFILEFN)(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

/****************************************************************************/
/*								Global data									*/
/****************************************************************************/

VALUE_DISPLAY_FUNCTION	lpfnDisplayNote;
VALUE_DISPLAY_FUNCTION	lpfnDisplayNumber;
VALUE_DISPLAY_FUNCTION	lpfnDisplayNoteAndNumber;
VALUE_DISPLAY_FUNCTION	lpfnDisplayBendRange;
VALUE_DISPLAY_FUNCTION	lpfnDisplayPresetPan;
COMMDLGFILEFN			lpfnSaveHookProc;
COMMDLGFILEFN			lpfnLoadHookProc;
HFONT					hDialogFont;				// Reformatted dialog font
BOOL					fAbortLongOperation = FALSE;

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

static WNDPROC		lpfnOldEditControlProc;
static DLGPROC		lpfnLongOperationDialog;
static LPCSTR		szNoteNames[] =		// Notes in the octave
{
	"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

VOIDEXPORT CALLBACK DisplayNote (HWND hWnd, int nValue);
VOIDEXPORT CALLBACK DisplayNumber (HWND hWnd, int nValue);
VOIDEXPORT CALLBACK DisplayNoteAndNumber (HWND hWnd, int nValue);
VOIDEXPORT CALLBACK DisplayBendRange (HWND hWnd, int nValue);
VOIDEXPORT CALLBACK DisplayPresetPan (HWND hWnd, int nValue);
DIALOG_PROC LongOperationDialog (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
DIALOG_PROC MyGetSaveFileNameDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
DIALOG_PROC MyGetLoadFileNameDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
WINDOW_PROC SubEditProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
static VOID DisplayCommDlgError (VOID);
static BOOL CreateChildren (HWND hDlg, LPSTR lpDlgItems, int dtItemCount, HFONT hFont, WORD wUnitsX, WORD wUnitsY);


/*^L*/
/*****************************************************************************
 Function:		DlgUtilsInit
 Parameters:	None.
 Returns:		None.
 Description:	Initialises this module.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 12/4/94 	| Created.											| PW
*****************************************************************************/

VOID DlgUtilsInit (VOID)
{
	HDC		hDC;
	int		OldMode;

	// Create the thin font
	hDC = GetDC (GetDesktopWindow ());
	OldMode = SetMapMode (hDC, MM_TEXT);
	hDialogFont = CreateFont (SMALL_DIALOG_FONT_SIZE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, SMALL_DIALOG_FONT_NAME);
	SetMapMode (hDC, OldMode);
	ReleaseDC (GetDesktopWindow (), hDC);

	// Create instances of COMMDLG hook functions
	lpfnSaveHookProc = (COMMDLGFILEFN)MakeProcInstance ((FARPROC)MyGetSaveFileNameDialogProc, hInst);
	lpfnLoadHookProc = (COMMDLGFILEFN)MakeProcInstance ((FARPROC)MyGetLoadFileNameDialogProc, hInst);

	// Create instances of window display functions
	lpfnDisplayNumber =
		(VALUE_DISPLAY_FUNCTION)MakeProcInstance ((FARPROC)DisplayNumber, hInst);
	lpfnDisplayNote =
		(VALUE_DISPLAY_FUNCTION)MakeProcInstance ((FARPROC)DisplayNote, hInst);
	lpfnDisplayNoteAndNumber =
		(VALUE_DISPLAY_FUNCTION)MakeProcInstance ((FARPROC)DisplayNoteAndNumber, hInst);
	lpfnDisplayBendRange =
		(VALUE_DISPLAY_FUNCTION)MakeProcInstance ((FARPROC)DisplayBendRange, hInst);
	lpfnDisplayPresetPan =
		(VALUE_DISPLAY_FUNCTION)MakeProcInstance ((FARPROC)DisplayPresetPan, hInst);
}

/*^L*/
/*****************************************************************************
 Function:		DlgUtilsTerm
 Parameters:	None.
 Returns:		None.
 Description:	Terminates this module.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 24/4/94 	| Created.											| PW
*****************************************************************************/

VOID DlgUtilsTerm (VOID)
{
	// Get rid of the dialog font
	DeleteObject (hDialogFont);

	// Delete proc instances
	FreeProcInstance ((FARPROC)lpfnSaveHookProc);
	FreeProcInstance ((FARPROC)lpfnLoadHookProc);

	FreeProcInstance ((FARPROC)lpfnDisplayNumber);
	FreeProcInstance ((FARPROC)lpfnDisplayNote);
	FreeProcInstance ((FARPROC)lpfnDisplayNoteAndNumber);
	FreeProcInstance ((FARPROC)lpfnDisplayBendRange);
}

/*^L*/
/*****************************************************************************
 Function:		ReformatDialog
 Parameters:	HWND	hWnd			Dialog window.
				WORD	wFlags			Reformatting flags.
 Returns:		None.
 Description:	Reformats the dialog :-
				If wFlags contains RDF_CENTRE then centre the dialog.
				If wFlags contains RDF_THINFONT then make the dialog font
					into MS Sans Serif 8 point. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 29/3/94 	| Created.											| PW
*****************************************************************************/

VOID ReformatDialog (HWND hWnd, WORD wFlags)
{
	HMENU	hMenu;
	DWORD	dwStyle;

	// Make me 3D
	Ctl3dSubclassDlgEx (hWnd, CTL3D_BUTTONS | CTL3D_LISTBOXES | CTL3D_EDITS | CTL3D_COMBOS |
							CTL3D_STATICTEXTS | CTL3D_STATICFRAMES | CTL3D_NODLGWINDOW);

	// If I have a system menu
	if ((dwStyle = GetWindowLong (hWnd, GWL_STYLE)) & WS_SYSMENU)
	{
		// Get the system menu handle
		hMenu = GetSystemMenu (hWnd, FALSE);

		// Delete the CLOSE, SIZE, and MAXIMIZE items from the menu
		RemoveMenu (hMenu, 5, MF_BYPOSITION);			// Separator

		DeleteMenu (hMenu, SC_CLOSE, MF_BYCOMMAND);
		DeleteMenu (hMenu, SC_SIZE, MF_BYCOMMAND);
		DeleteMenu (hMenu, SC_MAXIMIZE, MF_BYCOMMAND);
		DeleteMenu (hMenu, SC_TASKLIST, MF_BYCOMMAND);
		RemoveMenu (hMenu, 3, MF_BYPOSITION);			// Separator
	}

	dwStyle &= (~WS_MAXIMIZEBOX);
//	dwStyle &= (~WS_THICKFRAME);
	SetWindowLong (hWnd, GWL_STYLE, dwStyle);
//	SetWindowLong (hWnd, GWL_EXSTYLE, GetWindowLong (hWnd, GWL_EXSTYLE) | WS_EX_DLGMODALFRAME);

	// If RDF_CENTRE was specified
	if (wFlags & RDF_CENTRE)
	{
		RECT	rect;
		int		iX,iY,iNewX,iNewY;

		GetWindowRect (hWnd,(RECT far *)&rect);
		iX = GetSystemMetrics (SM_CXSCREEN);
		iY = GetSystemMetrics (SM_CYSCREEN);
		iNewX = (iX / 2) - ((rect.right - rect.left) / 2);
		iNewY = (iY / 2) - ((rect.bottom - rect.top) / 2);
		SetWindowPos (hWnd, NULL, max (iNewX, 0), max (iNewY, 0),
						rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
	}

//#ifdef NEVER
	// If RDF_THINFONT was specified
	if (wFlags & RDF_THINFONT)
	{
		HWND	hChild;

		// Get the first child window of the dialog
		hChild = GetWindow (hWnd, GW_CHILD);

		// For each child window
		while (hChild != (HWND)NULL)
		{
			// Set it's font
			SendMessage (hChild, WM_SETFONT, (WPARAM)hDialogFont, (LPARAM)MAKELONG (TRUE, 0));

			// Get the next child window
			hChild = GetWindow (hChild, GW_HWNDNEXT);
		}
	}
//#endif
}

/*^L*/
/*****************************************************************************
 Function:		DisplayNote
 Parameters:	HWND	hWnd			Control window
				int		nValue			MIDI note value
 Returns:		None.
 Description:	Displays a MIDI note value in textual format.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 11/4/94	| Created.											| PW
*****************************************************************************/

VOIDEXPORT CALLBACK DisplayNote (HWND hWnd, int nValue)
{
	char	szTotal[20];
	int		nOctave;

	nOctave = (nValue / 12) - 2;
	lstrcpy (&szTotal[0], szNoteNames[nValue % 12]);
	itoa (nOctave, &szTotal[lstrlen (&szTotal[0])], 10);
	SendMessage (hWnd, WM_SETTEXT, 0, (LPARAM)(LPCSTR)&szTotal[0]);
}

/*^L*/
/*****************************************************************************
 Function:		DisplayNumber
 Parameters:	HWND	hWnd			Control window
				int		nValue			Number
 Returns:		None.
 Description:	Displays an integer in a specified (control) window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 12/4/94	| Created.											| PW
*****************************************************************************/

VOIDEXPORT CALLBACK DisplayNumber (HWND hWnd, int nValue)
{
	char	szTotal[20];

	itoa (nValue, &szTotal[0], 10);
	SendMessage (hWnd, WM_SETTEXT, 0, (LPARAM)(LPCSTR)&szTotal[0]);
}

/*^L*/
/*****************************************************************************
 Function:		DisplayNoteAndNumber
 Parameters:	HWND	hWnd			Control window
				int		nValue			Number
 Returns:		None.
 Description:	Displays both a number and it's note value in the window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 14/5/94	| Created.											| PW
*****************************************************************************/

VOIDEXPORT CALLBACK DisplayNoteAndNumber (HWND hWnd, int nValue)
{
	char	szTotal[30];
	int		nOctave;

	// Turn the number to ASCII
	itoa (nValue, &szTotal[0], 10);

	// Add the note
	lstrcat (&szTotal[0], " (");
	nOctave = (nValue / 12) - 2;
	lstrcat (&szTotal[0], szNoteNames[nValue % 12]);
	itoa (nOctave, &szTotal[lstrlen (&szTotal[0])], 10);
	lstrcat (&szTotal[0], ")");

	// Write it
	SendMessage (hWnd, WM_SETTEXT, 0, (LPARAM)(LPCSTR)&szTotal[0]);
}

/*^L*/
/*****************************************************************************
 Function:		DisplayBendRange
 Parameters:	HWND	hWnd			Control window
				int		nValue			Value to display
 Returns:		None.
 Description:	Displays a pitch bend range.
				This is only a special case because :-
				a)	It seems a nice idea to display "±N" rather than "N"
				b)	A range of ±13 means "Global"
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 14/5/94	| Created.											| PW
*****************************************************************************/

VOIDEXPORT CALLBACK DisplayBendRange (HWND hWnd, int nValue)
{
	char	szTotal[20];

	if (nValue < 13)
	{
		szTotal[0] = '±';
		itoa (nValue, &szTotal[1], 10);
		SendMessage (hWnd, WM_SETTEXT, 0, (LPARAM)(LPCSTR)&szTotal[0]);
	}
	else
	{
		SendMessage (hWnd, WM_SETTEXT, 0, (LPARAM)ResourceString (IDS_GLOBAL));
	}
}

/*^L*/
/*****************************************************************************
 Function:		DisplayPresetPan
 Parameters:	HWND	hWnd			Control window
				int		nValue			Value to display
 Returns:		None.
 Description:	Displays a channel pan value.
				This takes the value +/-7, with -8="P"
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 15/6/94	| Created.											| PW
*****************************************************************************/

VOIDEXPORT CALLBACK DisplayPresetPan (HWND hWnd, int nValue)
{
	char	szNumBuf[20];

	// If it's -8
	if (nValue == -8)
	{
		// Special case
        szNumBuf[0] = 'P';
		szNumBuf[1] = '\0';
	}
	else
	{
		// Just convert to ASCII
		itoa (nValue, &szNumBuf[0], 10);
	}

	// Do the displaying
	SendMessage (hWnd, WM_SETTEXT, 0, (LPARAM)(LPCSTR)&szNumBuf[0]);
}

/**/
/***************************************************************************
Function:		MyDialogWindowProc
Parameters:		HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
Return value:	LRESULT
Description:	The window procedure for my dialogs.
				This is required because I have my own dialog class that
				over-rides the default so I can set the icon/background
                to use for minimisable dialogs.
History:
Date		| Description										| Name
------------+---------------------------------------------------+-----
26/5/93		| Created.											| PW
***************************************************************************/

WINDOW_PROC MyDialogWindowProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	// Call the default dialog handler
	return (DefDlgProc (hWnd, nMsg, wParam, lParam));
}

/**/
/***************************************************************************
Function:		DrawBitmap
Parameters:		HDC		hDC				Device context to draw into.
				HBITMAP	hBitmap			Loaded bitmap handle.
				int		x				X position to display bitmap at.
				int		y				Y position to display bitmap at.
Return value:	None.
Description:	Displays a bitmap.
History:

Date		| Description										| Name
------------+---------------------------------------------------+-----
24/4/93		| Created.											| PW
***************************************************************************/

VOID DrawBitmap (HDC hDC, HBITMAP hBitmap, int x, int y)
{
	BITMAP	bm;
	HDC		hdcMem;
	POINT	ptSize, ptOrg;

	hdcMem = CreateCompatibleDC (hDC);
	SelectObject (hdcMem, hBitmap);
	SetMapMode (hdcMem, GetMapMode (hDC));

	GetObject (hBitmap, sizeof (BITMAP), (LPSTR)&bm);
	ptSize.x = bm.bmWidth;
	ptSize.y = bm.bmHeight;
	DPtoLP (hDC, &ptSize, 1);

	ptOrg.x = 0;
	ptOrg.y = 0;
	DPtoLP (hdcMem, &ptOrg, 1);

	BitBlt (hDC, x, y, ptSize.x, ptSize.y, hdcMem, ptOrg.x, ptOrg.y, SRCCOPY);

	DeleteDC (hdcMem);
}

/*^L*/
/*****************************************************************************
 Function:		GetWindowPosition
 Parameters:	LPCSTR	lpszName		My window name.
				HWND	hWnd			My window handle.
				BOOL	fResize			TRUE if I can be sized.
 Returns:		None.
 Description:	Reads my window position from the INI file then moves me there.
				If my name isn't there, puts me at 0, 0.
				If my name begins with then I'm a dialog so don't change my size.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/5/94	| Created.											| PW
*****************************************************************************/

VOID GetWindowPosition (LPCSTR lpszName, HWND hWnd, BOOL fResize)
{
	char			szPosString[100];
	WINDOWPLACEMENT	wp;
	RECT			Rect;

	// If there isn't an initialisation string
	if (GetPrivateProfileString (WINDOWS_SECTION_NAME, lpszName, "", &szPosString[0], sizeof (szPosString), &szIniFileName[0]) == 0)
	{
		// If I can be sized
		if (fResize)
		{
			memset (&wp, 0, sizeof (wp));
			wp.length		= sizeof (WINDOWPLACEMENT);
			wp.flags		= 0;	// WPF_SETMINPOSITION;
            GetWindowRect (GetDesktopWindow (), &Rect);

			wp.showCmd = SW_SHOWNORMAL;
			wp.ptMinPosition.x = 100;
			wp.ptMinPosition.y = 100;
			wp.ptMaxPosition.x = -GetSystemMetrics (SM_CXFRAME);
			wp.ptMaxPosition.y = -GetSystemMetrics (SM_CYFRAME);
			wp.rcNormalPosition.left = 0;
			wp.rcNormalPosition.top = 0;
			wp.rcNormalPosition.right = Rect.right;
			wp.rcNormalPosition.bottom = Rect.bottom;
			SetWindowPlacement (hWnd, &wp);

			// Maximize me
//			ShowWindow (hWnd, SW_SHOWMAXIMIZED);
		}
		else
		{
			// Put me at (0, 0) but leave me the same size
			SetWindowPos (hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
	}
	else
	{
		// Decode the INI string into window placement values
		wp.length		= sizeof (WINDOWPLACEMENT);
		wp.flags		= WPF_SETMINPOSITION;
		sscanf (&szPosString[0],
				"%u %d %d %d %d %d %d %d %d",
				&wp.showCmd,
				&wp.ptMinPosition.x, &wp.ptMinPosition.y,
				&wp.ptMaxPosition.x, &wp.ptMaxPosition.y,
				&wp.rcNormalPosition.left, &wp.rcNormalPosition.top,
				&wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom);

		// If my size must be kept
		if (!fResize)
		{
			// Get my existing size
			GetWindowRect (hWnd, &Rect);

			// Make sure I am not being resized
			wp.rcNormalPosition.right	= wp.rcNormalPosition.left + (Rect.right - Rect.left);
			wp.rcNormalPosition.bottom	= wp.rcNormalPosition.top + (Rect.bottom - Rect.top);
		}

		// Set my placement
		SetWindowPlacement (hWnd, &wp);
	}

	ShowWindow (hWnd, SW_SHOW);
}

/*^L*/
/*****************************************************************************
 Function:		SaveWindowPosition
 Parameters:	LPCSTR	lpszName		My window name.
				HWND	hWnd			My window handle.
 Returns:		None.
 Description:	Saves my window position to the INI file.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/5/94	| Created.											| PW
*****************************************************************************/

VOID SaveWindowPosition (LPCSTR lpszName, HWND hWnd)
{
	char			szPosString[100];
	WINDOWPLACEMENT	wp;

	// Get its position
	wp.length = sizeof (WINDOWPLACEMENT);
	GetWindowPlacement (hWnd, &wp);

	// Create the positioning string
	wsprintf (	&szPosString[0],
				"%u %d %d %d %d %d %d %d %d",
				wp.showCmd,
				wp.ptMinPosition.x, wp.ptMinPosition.y,
				wp.ptMaxPosition.x, wp.ptMaxPosition.y,
				wp.rcNormalPosition.left, wp.rcNormalPosition.top,
				wp.rcNormalPosition.right, wp.rcNormalPosition.bottom);


	// Write it out
	WritePrivateProfileString (WINDOWS_SECTION_NAME, lpszName, &szPosString[0], &szIniFileName[0]);
}

/*^L*/
/*****************************************************************************
 Function:		GetCreatePosition
 Parameters:	LPCSTR	lpszName		My window name.
				BOOL	fResize			TRUE if I can be sized.
				LPWINDOWPLACEMENT	lpWp	(Output) window position.
 Returns:		None.
 Description:	Reads my window position from the INI file.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 30/10/94	| Created.											| PW
*****************************************************************************/

VOID GetCreatePosition (LPCSTR lpszName, BOOL fResize, LPWINDOWPLACEMENT lpWp)
{
	char			szPosString[100];
	RECT			Rect;

	// If there isn't an initialisation string
	if (GetPrivateProfileString (WINDOWS_SECTION_NAME, lpszName, "", &szPosString[0], sizeof (szPosString), &szIniFileName[0]) == 0)
	{
		// If I can be sized
		if (fResize)
		{
			memset (lpWp, 0, sizeof (WINDOWPLACEMENT));
			lpWp->length		= sizeof (WINDOWPLACEMENT);
			lpWp->flags		= 0;	// WPF_SETMINPOSITION;
			GetWindowRect (GetDesktopWindow (), &Rect);

			lpWp->showCmd = SW_SHOWNORMAL;
			lpWp->ptMinPosition.x = 100;
			lpWp->ptMinPosition.y = 100;
			lpWp->ptMaxPosition.x = -GetSystemMetrics (SM_CXFRAME);
			lpWp->ptMaxPosition.y = -GetSystemMetrics (SM_CYFRAME);
			lpWp->rcNormalPosition.left = 0;
			lpWp->rcNormalPosition.top = 0;
			lpWp->rcNormalPosition.right = Rect.right;
			lpWp->rcNormalPosition.bottom = Rect.bottom;
		}
		else
		{
			lpWp->showCmd = SW_SHOWNORMAL;
			lpWp->ptMinPosition.x = CW_USEDEFAULT;
			lpWp->ptMinPosition.y = CW_USEDEFAULT;
			lpWp->ptMaxPosition.x = CW_USEDEFAULT;
			lpWp->ptMaxPosition.y = CW_USEDEFAULT;
			lpWp->rcNormalPosition.left = 0;
			lpWp->rcNormalPosition.top = 0;
			lpWp->rcNormalPosition.right = CW_USEDEFAULT;
			lpWp->rcNormalPosition.bottom = CW_USEDEFAULT;
		}
	}
	else
	{
		// Decode the INI string into window placement values
		lpWp->length	= sizeof (WINDOWPLACEMENT);
		lpWp->flags		= WPF_SETMINPOSITION;
		sscanf (&szPosString[0],
				"%u %d %d %d %d %d %d %d %d",
				&lpWp->showCmd,
				&lpWp->ptMinPosition.x, &lpWp->ptMinPosition.y,
				&lpWp->ptMaxPosition.x, &lpWp->ptMaxPosition.y,
				&lpWp->rcNormalPosition.left, &lpWp->rcNormalPosition.top,
				&lpWp->rcNormalPosition.right, &lpWp->rcNormalPosition.bottom);
	}
}

/*^L*/
/*****************************************************************************
 Function:		AbortLongOperation
 Parameters:	None
 Returns:		BOOL					TRUE if operation should be aborted.
 Description:	Returns (and clears) the state of the 'abort long operation'
				flag.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 12/6/94	| Created.											| PW
*****************************************************************************/

BOOL AbortLongOperation (VOID)
{
	BOOL	fRetVal = fAbortLongOperation;

	fAbortLongOperation = FALSE;
	return (fRetVal);
}

/*^L*/
/*****************************************************************************
 Function:		SetListBoxTabs
 Parameters:	HWND				hWnd		Parent handle.
				int					idListBox	Listbox control ID.
				LISTBOX_TAB_INFO	*pLTI		Array of listbox tab stops.
                UINT				nNumTabs	Number of tab stops.
 Returns:		None.
 Description:	Sets a listbox's tab stop positions, and also repositions
				a set of labels over the listbox so that they are aligned
				with the listbox columns.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/6/94 	| Created.											| PW
*****************************************************************************/

VOID SetListBoxTabs (HWND hWnd, int idListBox, LISTBOX_TAB_INFO *pLTI, UINT nNumTabs)
{
	HFONT		hFont;
	HDC			hDC;
	UINT		nDialogBaseUnit, nTabSize, i;
	POINT		ListBoxXY, DialogXY, ControlXY;
	RECT		ControlRect;
	int			nTabStops[100];
	SIZE		size;

	// Calculate how big a dialog unit is
	hDC = GetDC (GetDlgItem (hWnd, idListBox));
	hFont = (HFONT)SendDlgItemMessage (hWnd, idListBox, WM_GETFONT, 0, 0);
	SelectObject (hDC, hFont);
	GetTextExtentPoint (hDC, ResourceString (IDS_ATOZ), 52, &size);
	nDialogBaseUnit = size.cx / 52;
	ReleaseDC (GetDlgItem (hWnd, idListBox), hDC);

	// Create an array of tab stop positions
	for (i=0 ; i<nNumTabs ; i++)
	{
		nTabStops[i] = pLTI[i].nTabPos;
	}

	// Set the tab stops for the listbox
	SendDlgItemMessage (hWnd, idListBox, LB_SETTABSTOPS, nNumTabs - 1, (LPARAM)(int far *)&nTabStops[1]);

	// For each listbox label to position
	ListBoxXY.x = ListBoxXY.y = 0;
	ClientToScreen (GetDlgItem (hWnd, idListBox), (LPPOINT)&ListBoxXY);
	DialogXY.x = DialogXY.y = 0;
	ClientToScreen (hWnd, (LPPOINT)&DialogXY);
	for (i=0 ; i<nNumTabs ; i++)
	{
		ControlXY.x = ControlXY.y = 0;
		ClientToScreen (GetDlgItem (hWnd, pLTI[i].idLabel), (LPPOINT)&ControlXY);
		GetWindowRect (GetDlgItem (hWnd, pLTI[i].idLabel), (LPRECT)&ControlRect);
		nTabSize = MulDiv (nDialogBaseUnit, nTabStops[i], 4);

		MoveWindow (GetDlgItem (hWnd, pLTI[i].idLabel),
					ListBoxXY.x - DialogXY.x + nTabSize, ControlXY.y - DialogXY.y,
					ControlRect.right - ControlRect.left, ControlRect.bottom - ControlRect.top,
					TRUE);
	}
}

/*^L*/
/*****************************************************************************
 Function:		MyGetSaveFileName
 Parameters:	HWND	hWnd			Parent handle.
				LPCSTR	lpszTitle		Dialog box title.
				LPCSTR	lpszFilter		Filter string.
				LPSTR	lpszFileName	File name (input and output).
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	My version of GetSaveFileName().
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/6/94 	| Created.											| PW
*****************************************************************************/

BOOL MyGetSaveFileName (HWND hWnd, LPCSTR lpszTitle, LPCSTR lpszFilter, LPSTR lpszFileName)
{
	OPENFILENAME	ofn;
	char			szDrive[MAXDRIVE], szPath[MAXPATH];

	// Use the Windows 3.1 Open File dialog to get the file name
	memset (&ofn, 0, sizeof (ofn));
	ofn.lStructSize		= sizeof (OPENFILENAME);
	ofn.hwndOwner		= hwndMain;
	ofn.hInstance		= hInst;
	ofn.lpstrFilter		= lpszFilter;
	ofn.nFilterIndex	= 1;
	ofn.lpstrFile		= lpszFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFileTitle	= NULL;
	ofn.nMaxFileTitle	= 0;
	ofn.lpstrInitialDir	= &szStorageDirectory[0];
	ofn.lpstrTitle		= lpszTitle;
	ofn.Flags			= OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | /*OFN_ENABLETEMPLATE | */OFN_ENABLEHOOK;
	ofn.lpfnHook		= lpfnSaveHookProc;
	ofn.lpTemplateName	= "FILEOPENORD";
	if (!GetSaveFileName (&ofn))
	{
//		DisplayCommDlgError ();
		return (FALSE);
	}

	SPLITPATH (lpszFileName, &szDrive[0], &szPath[0], NULL, NULL);
	wsprintf (&szStorageDirectory[0], "%s%s", &szDrive[0], &szPath[0]);
	if (szStorageDirectory[lstrlen (&szStorageDirectory[0]) - 1] == '\\')
	{
		szStorageDirectory[lstrlen (&szStorageDirectory[0]) - 1] = '\0';
	}
	SaveStoragePath (&szStorageDirectory[0]);

	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		MyGetSaveFileNameDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if message needs processing by
										COMMDLG also, else FALSE.
 Description:	The GetSaveFileName hook function.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/6/94 	| Created.											| PW
*****************************************************************************/

DIALOG_PROC MyGetSaveFileNameDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg)
	{
		case WM_INITDIALOG:
		{
			// Reformat myself
			ReformatDialog (hWnd, RDF_CENTRE | RDF_THINFONT);
            return (TRUE);
		}

		// Control about to be drawn
		case WM_CTLCOLOR:
		{
			Ctl3dCtlColorEx (nMsg, wParam, lParam);
			break;
		}

		default:
		{
        	break;
        }
	}

    return (FALSE);
}

/*^L*/
/*****************************************************************************
 Function:		MyGetLoadFileName
 Parameters:	HWND	hWnd			Parent handle.
				LPCSTR	lpszTitle		Dialog box title.
				LPCSTR	lpszFilter		Filter string.
				LPSTR	lpszFileName	File name (input and output).
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	My version of GetLoadFileName().
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/6/94 	| Created.											| PW
*****************************************************************************/

BOOL MyGetLoadFileName (HWND hWnd, LPCSTR lpszTitle, LPCSTR lpszFilter, LPSTR lpszFileName)
{
	OPENFILENAME	ofn;
	char			szDrive[MAXDRIVE], szPath[MAXPATH];

	// Use the Windows 3.1 Open File dialog to get the file name
	memset (&ofn, 0, sizeof (ofn));
	ofn.lStructSize		= sizeof (OPENFILENAME);
	ofn.hwndOwner		= hwndMain;
	ofn.hInstance		= hInst;
	ofn.lpstrFilter		= lpszFilter;
	ofn.nFilterIndex	= 1;
	ofn.lpstrFile		= lpszFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFileTitle	= NULL;
	ofn.nMaxFileTitle	= 0;
	ofn.lpstrInitialDir	= &szStorageDirectory[0];
	ofn.lpstrTitle		= lpszTitle;
	ofn.Flags			= OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | /*OFN_ENABLETEMPLATE | */OFN_ENABLEHOOK;
	ofn.lpfnHook		= lpfnLoadHookProc;
	ofn.lpTemplateName	= "FILEOPENORD";
	if (!GetOpenFileName (&ofn))
	{
//		DisplayCommDlgError ();
		return (FALSE);
	}

	SPLITPATH (lpszFileName, &szDrive[0], &szPath[0], NULL, NULL);
	wsprintf (&szStorageDirectory[0], "%s%s", &szDrive[0], &szPath[0]);
	if (szStorageDirectory[lstrlen (&szStorageDirectory[0]) - 1] == '\\')
	{
		szStorageDirectory[lstrlen (&szStorageDirectory[0]) - 1] = '\0';
	}
	SaveStoragePath (&szStorageDirectory[0]);

	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		MyGetLoadFileNameDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if message needs processing by
										COMMDLG also, else FALSE.
 Description:	The GetLoadFileName hook function.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/6/94 	| Created.											| PW
*****************************************************************************/

DIALOG_PROC MyGetLoadFileNameDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg)
	{
		case WM_INITDIALOG:
		{
			// Reformat myself
			ReformatDialog (hWnd, RDF_CENTRE | RDF_THINFONT);
            return (TRUE);
		}

		// Control about to be drawn
		case WM_CTLCOLOR:
		{
			Ctl3dCtlColorEx (nMsg, wParam, lParam);
			break;
		}

		default:
		{
        	break;
        }
	}

    return (FALSE);
}

static VOID DisplayCommDlgError (VOID)
{
	LPCSTR	lpszErrorMessage;

	switch (CommDlgExtendedError ())
	{
		case CDERR_FINDRESFAILURE:
			lpszErrorMessage = ResourceString (IDS_CDERR_FINDRESFAILURE);
            break;
		case CDERR_INITIALIZATION:
			lpszErrorMessage = ResourceString (IDS_CDERR_INITIALIZATION);
			break;
		case CDERR_LOADRESFAILURE:
			lpszErrorMessage = ResourceString (IDS_CDERR_LOADRESFAILURE);
            break;
		case CDERR_LOCKRESFAILURE:
			lpszErrorMessage = ResourceString (IDS_CDERR_LOCKRESFAILURE);
            break;
		case CDERR_LOADSTRFAILURE:
			lpszErrorMessage = ResourceString (IDS_CDERR_LOADSTRFAILURE);
            break;
		case CDERR_MEMALLOCFAILURE:
			lpszErrorMessage = ResourceString (IDS_CDERR_MEMALLOCFAILURE);
            break;
		case CDERR_MEMLOCKFAILURE:
			lpszErrorMessage = ResourceString (IDS_CDERR_MEMLOCKFAILURE);
            break;
		case CDERR_NOHINSTANCE:
			lpszErrorMessage = ResourceString (IDS_CDERR_NOHINSTANCE);
			break;  
		case CDERR_NOHOOK:
			lpszErrorMessage = ResourceString (IDS_CDERR_NOHOOK);
			break;
		case CDERR_NOTEMPLATE:
			lpszErrorMessage = ResourceString (IDS_CDERR_NOTEMPLATE);
			break; 
		case CDERR_REGISTERMSGFAIL:
			lpszErrorMessage = ResourceString (IDS_CDERR_REGISTERMSGFAIL);
			break; 
		case CDERR_STRUCTSIZE:
			lpszErrorMessage = ResourceString (IDS_CDERR_STRUCTSIZE);
            break;
		case CFERR_NOFONTS:
			lpszErrorMessage = ResourceString (IDS_CFERR_NOFONTS);
            break;
		case CFERR_MAXLESSTHANMIN:
			lpszErrorMessage = ResourceString (IDS_CFERR_MAXLESSTHANMIN);
			break; 
		case FNERR_BUFFERTOOSMALL:
			lpszErrorMessage = ResourceString (IDS_FNERR_BUFFERTOOSMALL);
			break; 
		case FNERR_INVALIDFILENAME:
			lpszErrorMessage = ResourceString (IDS_FNERR_INVALIDFILENAME);
            break;
		case FNERR_SUBCLASSFAILURE:
			lpszErrorMessage = ResourceString (IDS_FNERR_SUBCLASSFAILURE);
            break;
		case FRERR_BUFFERLENGTHZERO:
			lpszErrorMessage = ResourceString (IDS_FRERR_BUFFERLENGTHZERO);
            break;
		case PDERR_CREATEICFAILURE:
			lpszErrorMessage = ResourceString (IDS_PDERR_CREATEICFAILURE);
            break;
		case PDERR_DEFAULTDIFFERENT:
			lpszErrorMessage = ResourceString (IDS_PDERR_DEFAULTDIFFERENT);
			break;
		case PDERR_DNDMMISMATCH:
			lpszErrorMessage = ResourceString (IDS_PDERR_DNDMMISMATCH);
            break;
		case PDERR_GETDEVMODEFAIL:
			lpszErrorMessage = ResourceString (IDS_PDERR_GETDEVMODEFAIL);
			break; 
		case PDERR_INITFAILURE:
			lpszErrorMessage = ResourceString (IDS_PDERR_INITFAILURE);
            break;
		case PDERR_LOADDRVFAILURE:
			lpszErrorMessage = ResourceString (IDS_PDERR_LOADDRVFAILURE);
			break;
		case PDERR_NODEFAULTPRN:
			lpszErrorMessage = ResourceString (IDS_PDERR_NODEFAULTPRN);
            break;
		case PDERR_NODEVICES:
			lpszErrorMessage = ResourceString (IDS_PDERR_NODEVICES);
            break;
		case PDERR_PARSEFAILURE:
			lpszErrorMessage = ResourceString (IDS_PDERR_PARSEFAILURE);
        	break;
		case PDERR_PRINTERNOTFOUND:
			lpszErrorMessage = ResourceString (IDS_PDERR_PRINTERNOTFOUND);
            break;
		case PDERR_RETDEFFAILURE:
			lpszErrorMessage = ResourceString (IDS_PDERR_RETDEFFAILURE);
			break;
		case PDERR_SETUPFAILURE:
			lpszErrorMessage = ResourceString (IDS_PDERR_SETUPFAILURE);
			break;
		default:
			lpszErrorMessage = ResourceString (IDS_CDERR_UNKNOWN);
            break;
	}
	MyMessageBox (hwndMain, lpszErrorMessage,
		ResourceString (IDS_CDERR_TITLE), MB_ICONEXCLAMATION | MB_OK);

	
}

/*^L*/
/*****************************************************************************
 Function:		InitSlider
 Parameters:	HWND	hwndParent					Dialog window
				int		idControl					Slider control ID
				int		nMin						Minimum control value
				int		nMax						Maximum control value
				int		nDefault					Default control value
 Returns:		None.
 Description:	Initialises the instruments dialog
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 12/4/94	| Created.											| PW
*****************************************************************************/

VOID InitSlider (HWND hwndParent, int idControl, int nMin, int nMax, int nDefault)
{
	// Prevent redrawing
	SendDlgItemMessage (hwndParent, idControl, WM_SETREDRAW, (WPARAM)FALSE, 0);

	// Set the slider's minimum value
	SendDlgItemMessage (hwndParent, idControl, SL_SET_MIN, (WPARAM)nMin, 0);

	// Set the slider's maximum value
	SendDlgItemMessage (hwndParent, idControl, SL_SET_MAX, (WPARAM)nMax, 0);

	// Set the slider's current value
	SendDlgItemMessage (hwndParent, idControl, SL_SET_CURRENT, (WPARAM)nDefault, 0);

	// Allow redrawing again
	SendDlgItemMessage (hwndParent, idControl, WM_SETREDRAW, (WPARAM)TRUE, 0);

	// Force a redraw
	InvalidateRect (GetDlgItem (hwndParent, idControl), (LPRECT)NULL, FALSE);
	UpdateWindow (GetDlgItem (hwndParent, idControl));
}

/*^L*/
/*****************************************************************************
 Function:		InitUpDown
 Parameters:	HWND	hwndParent					Dialog window
				int		idControl					Ccontrol ID
				int		nMin						Minimum control value
				int		nMax						Maximum control value
				int		nDefault					Default control value
				HWND	hwndPartner					Partner (display) window or NULL if none
				VALUE_DISPLAY_FUNCTION	pfnDisplay	Partner output function
 Returns:		None.
 Description:	Initialises an UP-DOWN control.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 12/4/94	| Created.											| PW
*****************************************************************************/

VOID InitUpDown (HWND hwndParent, int idControl, int nMin, int nMax, int nDefault,
						HWND hwndPartner, VALUE_DISPLAY_FUNCTION pfnDisplay)
{
	SendDlgItemMessage (hwndParent, idControl, UD_SET_MIN, (WPARAM)nMin, 0);
	SendDlgItemMessage (hwndParent, idControl, UD_SET_MAX, (WPARAM)nMax, 0);
	SendDlgItemMessage (hwndParent, idControl, UD_SET_CURRENT, (WPARAM)nDefault, 0);
	if (hwndPartner != (HWND)NULL)
	{
		SendDlgItemMessage (hwndParent, idControl, UD_SET_NUM_DISPLAY, (WPARAM)hwndPartner, (LPARAM)pfnDisplay);
	}
}

/*^L*/
/*****************************************************************************
 Function:		InitListbox
 Parameters:	HWND	hwndControl					Control window handle
				char	**pszItems					Items to put into list box
				int		nNumItems					Number of items.
 Returns:		None.
 Description:	Initialises generic drop-down listbox.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 24/4/94	| Created.											| PW
*****************************************************************************/

VOID InitListbox (HWND hwndControl, LPCSTR *pszItems, int nNumItems)
{
	UINT		i;

	// Clear it out first
	SendMessage (hwndControl, CB_RESETCONTENT, 0, 0);

	// For each listbox item to add
	for (i=0 ; i<(UINT)nNumItems ; i++)
	{
    	// Add the listbox entry
		SendMessage (hwndControl, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)pszItems[i]);
	}

	// Select item 0 
	SendMessage (hwndControl, CB_SETCURSEL, 0, 0);
}

/*^L*/
/*****************************************************************************
 Function:		InterceptEditEnter
 Parameters:	HWND	hwndParent					Dialog window.
				int		idEditControl				Edit control ID.
 Returns:		None.
 Description:	Subclasses an edit control so that pressing ENTER sends
				a EN_ENTER (user) message to the parent.  
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 25/6/94	| Created.											| PW
*****************************************************************************/

VOID InterceptEditEnter (HWND hwndParent, int idEditControl)
{
	static FARPROC	lpfnSubEditProc = (WINDOW_PROC)NULL;

	if (lpfnSubEditProc == (WINDOW_PROC)NULL)
	{
		lpfnSubEditProc = MakeProcInstance ((FARPROC)SubEditProc, hInst);
	}
	lpfnOldEditControlProc =
		(WNDPROC)SetWindowLong (GetDlgItem (hwndParent, idEditControl), GWL_WNDPROC, (LONG)lpfnSubEditProc);
}

/*^L*/
/*****************************************************************************
 Function:		SubEditProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if message needs processing by
										COMMDLG also, else FALSE.
 Description:	The GetSaveFileName hook function.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 25/6/94 	| Created.											| PW
*****************************************************************************/

WINDOW_PROC SubEditProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	HWND	hwndParent;

	switch (nMsg)
	{
		case WM_GETDLGCODE:
        {
			return (DLGC_WANTALLKEYS | CallWindowProc ((FARPROC)lpfnOldEditControlProc, hWnd, nMsg, wParam, lParam));
		}

		case WM_CHAR:
        {
			// Process this message to avoid message beeps.
			if ((wParam == VK_RETURN) || (wParam == VK_TAB))
            {
				return (0);
            }
			else
            {
				return (CallWindowProc ((FARPROC)lpfnOldEditControlProc, hWnd, nMsg, wParam, lParam));
			}
        }

		case WM_KEYDOWN:
		{
			hwndParent = GetParent (hWnd);

			// If it's a TAB
			if (wParam == VK_TAB)
			{
				// Tell my parents to step onto the next control
				PostMessage (hwndParent, EN_ENTER, GETWNDID (hWnd), 0L);
				PostMessage (hwndParent, WM_NEXTDLGCTL, 0, 0L);
				return (FALSE);
			}
			// Else if it's ENTER
			else if (wParam == VK_RETURN)
			{
				// Tell my parents that ENTER's been hit
				PostMessage (hwndParent, EN_ENTER, GETWNDID (hWnd), 0L);
				return (FALSE);
			}

			return (CallWindowProc ((FARPROC)lpfnOldEditControlProc, hWnd, nMsg, wParam, lParam));
		}

		// Losing focus
		case WM_KILLFOCUS:
		{
			// Tell my parents that ENTER's been hit
			hwndParent = GetParent (hWnd);
			PostMessage (hwndParent, EN_ENTER, GETWNDID (hWnd), 0L);
			break;
		}

		default:
        {
			break;
        }
	}

	return (CallWindowProc ((FARPROC)lpfnOldEditControlProc, hWnd, nMsg, wParam, lParam));
}

#if defined (WINDOWS32)

/*^L*/
/***************************************************************************
Function:		DialogInWindow
Parameters:		HWND	hWnd			Window to use.
				LPCSTR	lpszDialogName	Dialog name.
Return value:	None.
Description:	Puts the specified dialog into an existing window.
History:
Date		| Description										| Name
------------+---------------------------------------------------+-----
24/5/94		| Created.											| PW
***************************************************************************/

DWORD DialogInWindow (HWND hWnd, LPCSTR lpszDialogName)
{
	int						i;
	WCHAR					ch;
	HDC						hDC;
	HFONT					hFont, hOldFont;
	HGLOBAL					hRes;
	HRSRC					hDlgRes;
	LOGFONT					lf;
	WCHAR					*lpDlgRes;
	RECT					rcWnd;
	TEXTMETRIC				tm;
	WORD					wUnitsX, wUnitsY;
	SIZE					size;

	// Local variables used when parsing the dialog template
	WORD					dtX, dtY, dtCX, dtCY;
	char					dtMenuName[MAXLEN_MENUNAME],
							dtClassName[MAXLEN_CLASSNAME],
							dtCaptionText[MAXLEN_CAPTIONTEXT],
							dtTypeFace[MAXLEN_TYPEFACE];
	WORD					dtItemCount;
	DWORD					dtStyle;
	WORD					dtPointSize;

	// PW
	DWORD					dwWinStyle, dwExtStyle;
	int						w, h;

	// Find, load, and lock the dialog resource
	hDlgRes		= FindResource (hInst, lpszDialogName, RT_DIALOG);
	hRes		= LoadResource (hInst, hDlgRes);
	lpDlgRes	= LockResource (hRes);

	// Get its style, item count, and initial position, size
	dtStyle		= *((DWORD FAR *)lpDlgRes)++;
	dwExtStyle	= *((DWORD FAR *)lpDlgRes)++;
	dtItemCount	= *((WORD FAR *)lpDlgRes)++;
	dtX			= *((WORD FAR *)lpDlgRes)++;
	dtY			= *((WORD FAR *)lpDlgRes)++;
	dtCX		= *((WORD FAR *)lpDlgRes)++;
	dtCY		= *((WORD FAR *)lpDlgRes)++;

	// Get menu name
	i = 0;
	while ((ch = *lpDlgRes++) != '\0')
    {
		dtMenuName[i++] = (char)ch;
    }
	dtMenuName[i] = '\0';

	// Get class name
	i = 0;
	while ((ch = *lpDlgRes++) != '\0')
    {
		dtClassName[i++] = (char)ch;
	}
	dtClassName[i] = '\0';

	// Get caption text
	i = 0;
	while ((ch = *lpDlgRes++) != '\0')
	{
		dtCaptionText[i++] = (char)ch;
	}
	dtCaptionText[i] = '\0';

	// Get point size
	dtPointSize = *((WORD FAR *)lpDlgRes)++;

	// Get face name
	i = 0;
	while ((ch = *lpDlgRes++) != '\0')
	{
		dtTypeFace[i++] = (char)ch;
	}
	dtTypeFace[ i ] = '\0' ;

	// Calculate dialog unit size
	hDC = GetDC (NULL);
	memset (&lf, 0, sizeof (LOGFONT));
	lstrcpy (lf.lfFaceName, dtTypeFace);
	lf.lfHeight = -MulDiv (dtPointSize, GetDeviceCaps (hDC, LOGPIXELSY), 72);
	lf.lfWeight = FW_BOLD;
	hFont = CreateFontIndirect (&lf);
	hOldFont = SelectObject (hDC, hFont);
	GetTextMetrics (hDC, &tm);
	GetTextExtentPoint (hDC, ResourceString (IDS_ATOZ), 52, &size);
	wUnitsX = size.cx / 52;
	wUnitsY = (WORD)tm.tmHeight;
	SelectObject (hDC, hOldFont);
	ReleaseDC (NULL, hDC);

	rcWnd.left = 0 ;
	rcWnd.top = 0 ;
	rcWnd.right = DLGTOCLIENTX (dtCX, wUnitsX);
	rcWnd.bottom = DLGTOCLIENTY (dtCY, wUnitsY);
	AdjustWindowRect (&rcWnd, dtStyle, FALSE);

	// Change my size to the same size as the dialog
	w = rcWnd.right - rcWnd.left;
	h = rcWnd.bottom - rcWnd.top;
	dwWinStyle = GetWindowLong (hWnd, GWL_STYLE);
	if (dwWinStyle & WS_BORDER)
	{
		w += 4 * GetSystemMetrics (SM_CXBORDER);
		h += 4 * GetSystemMetrics (SM_CYBORDER);
	}
	if (dwWinStyle & WS_CAPTION)
	{
//		h += GetSystemMetrics (SM_CYCAPTION);
	}
	SetWindowPos (hWnd, (HWND)NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	// Change my title to the dialog title
	SetWindowText (hWnd, &dtCaptionText[0]);

	// Create the children
	lpDlgRes = ROUNDPTR (lpDlgRes);
	CreateChildren (hWnd, (LPSTR)lpDlgRes, dtItemCount, hFont, wUnitsX, wUnitsY);

	// Free the resource
	UnlockResource (hRes);
	FreeResource (hRes);
	DeleteObject (hFont);

	// Return the dialog width & height
	return (MAKELONG (h, w));
}

/***************************************************************************
Function:		CreateChildren
Parameters:		HWND	hDlg
				LPSTR	lpDlgItems
				int		dtItemCount
				HFONT	hFont
				WORD	wUnitsX
				WORD	wUnitsY
Return value:	BOOL					TRUE if OK, else FALSE.
Description:	Create a dialog's child windows.
History:
Date		| Description										| Name
------------+---------------------------------------------------+-----
20/9/94		| Created.											| PW
***************************************************************************/

static BOOL CreateChildren (HWND hDlg, LPSTR lpDlgItems, int dtItemCount, HFONT hFont, WORD wUnitsX, WORD wUnitsY)
{
	int		i, nCtl;
	WCHAR	ch;
	HWND	hCtl;
	BOOL	fStandardControl;

	// Local vars used when parsing the dialog template
	WORD	dtID, dtX, dtY, dtCX, dtCY;
	char	dtClassName[MAXLEN_CLASSNAME], dtCaptionText[MAXLEN_CAPTIONTEXT];
	DWORD	dtStyle, dtExtStyle;
	WORD	nExtraStuff;

	int		w, h;

	// For each control
	for (nCtl=0 ; nCtl<dtItemCount ; nCtl++)
	{
		// Get its info
		dtStyle		= *((DWORD FAR *)lpDlgItems)++;
		dtExtStyle	= *((DWORD FAR *)lpDlgItems)++;
		dtX			= *((WORD FAR *)lpDlgItems)++;
		dtY			= *((WORD FAR *)lpDlgItems)++;
		dtCX		= *((WORD FAR *)lpDlgItems)++;
		dtCY		= *((WORD FAR *)lpDlgItems)++;
		dtID		= *((WORD FAR *)lpDlgItems)++;

		w = DLGTOCLIENTX (dtCX, wUnitsX);
		h = DLGTOCLIENTY (dtCY, wUnitsY);

		// Get class name
		fStandardControl = FALSE;
		i = 0;
		while ((ch = *((WCHAR FAR *)lpDlgItems)++) != '\0')
		{
			// If "Standard windows control prefix"
			if (ch == 0xFFFF)
			{
				// The next WORD specifies the control type
				ch = *((WCHAR FAR *)lpDlgItems)++;
				fStandardControl = TRUE;
				break;
			}

			dtClassName[i++] = (char)ch;
		}

		if (fStandardControl)
		{
			switch (ch & 0x7F)
			{
				case 0:
					lstrcpy (dtClassName, "BUTTON");
					break;

				case 1:
					lstrcpy (dtClassName, "EDIT");
					break;

				case 2:
					lstrcpy (dtClassName, "STATIC");
					break;

				case 3:
					lstrcpy (dtClassName, "LISTBOX");
					break;

				case 4:
					lstrcpy (dtClassName, "SCROLLBAR");
					break;

				case 5:
           			lstrcpy (dtClassName, "COMBOBOX");
					break;
			}
		}
		else
		{
			dtClassName[i] = '\0';
			if (!lstrcmp (&dtClassName[0], SLIDER_CLASS_NAME))
			{
				// Bodge the width and height to be exactly right
				w = 29;
				h = 100;
			}
		}

		// Get caption text
		i = 0 ;
		while ((ch = *((WCHAR FAR *)lpDlgItems)++) != '\0')
		{
			dtCaptionText[i++] = (char)ch;
		}
		dtCaptionText[i] = '\0';

		// Get the "extra stuff"
		nExtraStuff = *((WORD FAR *)lpDlgItems)++;

		// Create the child window / control
		hCtl = CreateWindow (dtClassName, dtCaptionText, dtStyle,
							DLGTOCLIENTX (dtX, wUnitsX), DLGTOCLIENTY (dtY, wUnitsY),
							w, h,
							hDlg, (HMENU)dtID, GETHINST (hDlg),
							(nExtraStuff == 0) ? NULL : (VOID FAR *)lpDlgItems);
		SendMessage (hCtl, WM_SETFONT, (WPARAM)hFont, (LPARAM) FALSE);

		// Step past the "extra stuff"
		*((WORD FAR *)lpDlgItems) += nExtraStuff;

		// Align the next control on a DWORD boundary
		lpDlgItems = ROUNDPTR (lpDlgItems);
	}

	return (TRUE);
}

#else

/*^L*/
/***************************************************************************
Function:		DialogInWindow
Parameters:		HWND	hWnd			Window to use.
				LPCSTR	lpszDialogName	Dialog name.
Return value:	None.
Description:	Puts the specified dialog into an existing window.
History:
Date		| Description										| Name
------------+---------------------------------------------------+-----
24/5/94		| Created.											| PW
***************************************************************************/

DWORD DialogInWindow (HWND hWnd, LPCSTR lpszDialogName)
{
	int						i;
	char					ch;
	HDC						hDC;
	HFONT					hFont, hOldFont;
	HGLOBAL					hRes;
	HRSRC					hDlgRes;
	LOGFONT					lf;
	LPSTR					lpDlgRes;
	RECT					rcWnd;
	TEXTMETRIC				tm;
	WORD					wUnitsX, wUnitsY;
	SIZE					size;

	// local variables used when parsing the dialog template
	int						dtX, dtY, dtCX, dtCY;
	char					dtMenuName[MAXLEN_MENUNAME],
							dtClassName[MAXLEN_CLASSNAME],
							dtCaptionText[MAXLEN_CAPTIONTEXT],
							dtTypeFace[MAXLEN_TYPEFACE];
	BYTE					dtItemCount;
	DWORD					dtStyle;
	WORD					dtPointSize;

	// PW
	DWORD					dwWinStyle;
	int						w, h;

	// Find, load, and lock the dialog resource
	hDlgRes		= FindResource (hInst, lpszDialogName, RT_DIALOG);
	hRes		= LoadResource (hInst, hDlgRes);
	lpDlgRes	= LockResource (hRes);

	// Get its style, item count, and initial position, size
	dtStyle		= *((DWORD FAR *)lpDlgRes)++;
	dtItemCount	= *((BYTE FAR *)lpDlgRes)++;
	dtX			= *((int FAR *)lpDlgRes)++;
	dtY			= *((int FAR *)lpDlgRes)++;
	dtCX		= *((int FAR *)lpDlgRes)++;
	dtCY		= *((int FAR *)lpDlgRes)++;

	// Get menu name
	i = 0;
	while ((ch = *lpDlgRes++) != '\0')
    {
		dtMenuName[i++] = ch;
    }
	dtMenuName[i] = '\0';

	// Get class name
	i = 0;
	while ((ch = *lpDlgRes++) != '\0')
    {
		dtClassName[i++] = ch;
	}
	dtClassName[i] = '\0';

	// Get caption text
	i = 0;
	while ((ch = *lpDlgRes++) != '\0')
    {
		dtCaptionText[i++] = ch;
	}
	dtCaptionText[i] = '\0';

	// Get point size
	dtPointSize = *((WORD FAR *)lpDlgRes)++;

	// Get face name
	i = 0;
	while ((ch = *lpDlgRes++) != '\0')
	{
		dtTypeFace[i++] = ch;
	}
	dtTypeFace[ i ] = '\0' ;

	// Calculate dialog unit size
	hDC = GetDC (NULL);
	memset (&lf, 0, sizeof (LOGFONT));
	lstrcpy (lf.lfFaceName, dtTypeFace);
	lf.lfHeight = -MulDiv (dtPointSize, GetDeviceCaps (hDC, LOGPIXELSY), 72);
	lf.lfWeight = FW_BOLD;
	hFont = CreateFontIndirect (&lf);
	hOldFont = SelectObject (hDC, hFont);
	GetTextMetrics (hDC, &tm);
	GetTextExtentPoint (hDC, ResourceString (IDS_ATOZ), 52, &size);
	wUnitsX = size.cx / 52;
	wUnitsY = (WORD)tm.tmHeight;
	SelectObject (hDC, hOldFont);
	ReleaseDC (NULL, hDC);

	rcWnd.left = 0 ;
	rcWnd.top = 0 ;
	rcWnd.right = DLGTOCLIENTX (dtCX, wUnitsX);
	rcWnd.bottom = DLGTOCLIENTY (dtCY, wUnitsY);
	AdjustWindowRect (&rcWnd, dtStyle, FALSE);

	// Change my size to the same size as the dialog
	w = rcWnd.right - rcWnd.left;
	h = rcWnd.bottom - rcWnd.top;
	dwWinStyle = GetWindowLong (hWnd, GWL_STYLE);
	if (dwWinStyle & WS_BORDER)
	{
		w += 4 * GetSystemMetrics (SM_CXBORDER);
		h += 4 * GetSystemMetrics (SM_CYBORDER);
	}
	if (dwWinStyle & WS_CAPTION)
	{
//		h += GetSystemMetrics (SM_CYCAPTION);
	}
	SetWindowPos (hWnd, (HWND)NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	// Create the children
	CreateChildren (hWnd, lpDlgRes, dtItemCount, hFont, wUnitsX, wUnitsY);

	// Free the resource
	UnlockResource (hRes);
	FreeResource (hRes);
	DeleteObject (hFont);

	// Return the dialog width & height
	return (MAKELONG (h, w));
}

/***************************************************************************
Function:		CreateChildren
Parameters:		HWND	hDlg
				LPSTR	lpDlgItems
				int		dtItemCount
				HFONT	hFont
				WORD	wUnitsX
				WORD	wUnitsY
Return value:	BOOL					TRUE if OK, else FALSE.
Description:	Create a dialog's child windows.
History:
Date		| Description										| Name
------------+---------------------------------------------------+-----
20/9/94		| Created.											| PW
***************************************************************************/

static BOOL CreateChildren (HWND hDlg, LPSTR lpDlgItems, int dtItemCount, HFONT hFont, WORD wUnitsX, WORD wUnitsY)
{
	int		i, nCtl;
	char	ch;
	HWND	hCtl;

	// Local vars used when parsing the dialog template
	int		dtID, dtX, dtY, dtCX, dtCY;
	char	dtClassName[MAXLEN_CLASSNAME], dtCaptionText[MAXLEN_CAPTIONTEXT];
	BYTE	dtInfoSize;
	DWORD	dtStyle;

	int		w, h;

	// For each control
	for (nCtl=0 ; nCtl<dtItemCount ; nCtl++)
	{
		// Get its info 
		dtX = *((int FAR *)lpDlgItems)++;
		dtY = *((int FAR *)lpDlgItems)++;
		dtCX = *((int FAR *)lpDlgItems)++;
		dtCY = *((int FAR *)lpDlgItems)++;
		dtID = *((int FAR *)lpDlgItems)++;
		dtStyle = *((DWORD FAR *)lpDlgItems)++;

		w = DLGTOCLIENTX (dtCX, wUnitsX);
		h = DLGTOCLIENTY (dtCY, wUnitsY);

		// Get class name
		i = 0;
		while ((ch = *lpDlgItems++) != '\0')
		{
			dtClassName[i++] = ch;
			if (ch & 0x80)
				break;
		}

		if (ch & 0x80)
		{
			switch (ch & 0x7F)
			{
				case 0:
					lstrcpy (dtClassName, "BUTTON");
					break;

				case 1:
					lstrcpy (dtClassName, "EDIT");
					break;

				case 2:
					lstrcpy (dtClassName, "STATIC");
					break;

				case 3:
					lstrcpy (dtClassName, "LISTBOX");
					break;

				case 4:
					lstrcpy (dtClassName, "SCROLLBAR");
					break;

				case 5:
           			lstrcpy (dtClassName, "COMBOBOX");
					break;
			}
		}
		else
		{
			dtClassName[i] = '\0';
			if (!lstrcmp (&dtClassName[0], SLIDER_CLASS_NAME))
			{
				// Bodge the width and height to be exactly right
				w = 29;
				h = 100;
			}
		}

		// Get caption text
		i = 0 ;
		while ((ch = *lpDlgItems++) != '\0')
		{
			dtCaptionText[i++] = ch;
		}
		dtCaptionText[i] = '\0';

		dtInfoSize = *((BYTE FAR *)lpDlgItems)++;

		// Create the child window / control
		hCtl = CreateWindow (dtClassName, dtCaptionText, dtStyle,
							DLGTOCLIENTX (dtX, wUnitsX), DLGTOCLIENTY (dtY, wUnitsY),
							w, h,
							hDlg, (HMENU)dtID, GETHINST (hDlg),
							(dtInfoSize == 0) ? NULL : (VOID FAR *)lpDlgItems);
		SendMessage (hCtl, WM_SETFONT, (WPARAM)hFont, (LPARAM) FALSE);

		// Bounce over child info structure
		lpDlgItems += dtInfoSize;
	}

	return (TRUE);
}

#endif

/***************************************************************************
Function:		DestroyAllChildren
Parameters:		HWND	hWnd
Return value:	None.
Description:	Destroy a window's children.
History:
Date		| Description										| Name
------------+---------------------------------------------------+-----
26/9/94		| Created.											| PW
***************************************************************************/

VOID DestroyAllChildren (HWND hWnd)
{
	HWND	hChild, hChildren[300];
	int		n = 0, i;

	// Get the first child window of the dialog
	hChild = GetWindow (hWnd, GW_CHILD);

	// For each child window
	while (hChild != (HWND)NULL)
	{
		// Store its handle
		hChildren[n++] = hChild;

		// Get the next born
		hChild = GetWindow (hChild, GW_HWNDNEXT);
	}

	// Destroy them
	for (i=0 ; i<n ; i++)
	{
		DestroyWindow (hChildren[i]);
	}
}

/***************************************************************************
Function:		InvisoWndProc
Parameters:		HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
Return value:	The usual.
Description:	Invisible window procedure.
History:
Date		| Description										| Name
------------+---------------------------------------------------+-----
Halloween94	| Created.											| PW
***************************************************************************/

WINDOW_PROC InvisoWndProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
   return DefWindowProc (hWnd, nMsg, wParam, lParam);
}

/***************************************************************************
Function:		hCreateInvisoWindow
Parameters:		HWND	hwndParent
Return value:	HWND					Invisible window.
Description:	Creates an invisible window over the top of the specified window.
History:
Date		| Description										| Name
------------+---------------------------------------------------+-----
Halloween94	| Created.											| PW
***************************************************************************/

HWND CreateMouseEater (HWND hwndParent)
{
	RECT	r;
	HWND	hWnd;

	// Get the parent window's area
	GetWindowRect (hwndParent, &r);

	// Create the invisible window
	hWnd = CreateWindowEx (WS_EX_TRANSPARENT, WCNAME_MOUSE_EATER, "", WS_CHILD | WS_VISIBLE,
							0, 0, r.right, r.bottom, hwndParent, (HMENU)42, hInst, NULL);

	// Cover all controls with the message eater
	BringWindowToTop (hWnd);

	// Set focus so that the hourglass appears
	SetFocus (hWnd);

	return (hWnd);
}


/***************************************************************************
Function:		EnableAllMenuItems
Parameters:		HWND	hwndParent
Return value:	None.
Description:	Disables all menu items of a specified window.
History:
Date		| Description										| Name
------------+---------------------------------------------------+-----
Halloween94	| Created.											| PW
***************************************************************************/

VOID EnableAllMenuItems (HWND hWnd, BOOL fEnable)
{
	HMENU	hmenuWindow, hSubMenu;
	int		nNumSubMenus, nSubMenu, id;
	int		nNumMenuItems, nMenuItem;

	// Get the window's menu
	hmenuWindow = GetMenu (hWnd);

	// How many sub-menus are there ?
	nNumSubMenus = GetMenuItemCount (hmenuWindow);

	// For each sub-menu
	for (nSubMenu=0 ; nSubMenu<nNumSubMenus ; nSubMenu++)
	{
		// Get the sub-menu handle
		hSubMenu = GetSubMenu (hmenuWindow, nSubMenu);

		// How many sub-menu items ?
		nNumMenuItems = GetMenuItemCount (hSubMenu);

		// For each sub-menu item
		for (nMenuItem=0 ; nMenuItem<nNumMenuItems ; nMenuItem++)
		{
			// Get the item's ID
			id = GetMenuItemID (hSubMenu, nMenuItem);

			// Enable / disable the item
			EnableMenuItem (hSubMenu, id, MF_BYCOMMAND | (fEnable ? MF_ENABLED : MF_GRAYED));
		}
	}
}


/***************************************************************************
Function:		ResourceString 
Parameters:		int		idString
Return value:	LPCSTR					String from resources.
Description:	Loads a resource string.
History:
Date		| Description										| Name
------------+---------------------------------------------------+-----
4/3/95		| Created.											| PW
***************************************************************************/

LPCSTR ResourceString (int idString)
{
#define	MAX_LOADED_STRINGS			4
#define	MAX_LOADED_STRING_LENGTH	512

	static char	szStrings[MAX_LOADED_STRINGS][MAX_LOADED_STRING_LENGTH];
	static int	nStringIndex = 0;
	LPSTR		lpszLoadedString;

	// Load this string
	LoadString (hInst, idString, lpszLoadedString = (LPSTR)&szStrings[nStringIndex][0], MAX_LOADED_STRING_LENGTH);

	// Step onto the buffer to copy next string to load
	if (++nStringIndex == MAX_LOADED_STRINGS)
	{
		nStringIndex = 0;
	}

	// Return the loaded string
	return ((LPCSTR)lpszLoadedString);
}

