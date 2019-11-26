/*****************************************************************************
 File:			tuning.c
 Description:	The user tuning module for the Morpheus editor.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 17/6/94 	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define	COARSE_TUNE_BYTE(w)     ((w) >> 6)
#define	FINE_TUNE_BYTE(w)     	((w) & 0x3F)
#define	MAKE_TUNE_WORD(c,f)     (((int)(f)) & 0x3F) + (((int)(c)) << 6)

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

static LISTBOX_TAB_INFO		TuningListboxTabs[] =
{
	{IDC_USER_TUNING_LABEL1,	0},
	{IDC_USER_TUNING_LABEL2,	40},
	{IDC_USER_TUNING_LABEL3,	85}
};

/****************************************************************************/
/*								Global data									*/
/****************************************************************************/

// The working copy of the tuning table
TUNING_TABLE_PARAMS	TuningTableCopy;

// Whether the tuning table has been modified
BOOL				fDirtyTuning = FALSE;

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

static VOID FormatTuningNote (LPSTR lpszBuf, int nNote);
static BOOL StoreTuning (HWND hwndParent);
static BOOL AbandonTuning (HWND hwndParent);

/*^L*/
/*****************************************************************************
 Function:		CreateUserTuningWindow
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Creates the user tuning window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 17/6/94 	| Created.											| PW
*****************************************************************************/

BOOL CreateUserTuningWindow (VOID)
{
	MDICREATESTRUCT	mc;
	WINDOWPLACEMENT	wp;

	// Create the window
	GetCreatePosition (USER_TUNING_WINDOW, FALSE, &wp);
	mc.szClass	= WCNAME_TUNING;
	mc.szTitle	= ResourceString (IDS_UT_WINDOW_TITLE);
	mc.hOwner	= hInst;
	mc.x		= wp.rcNormalPosition.left;
	mc.y		= wp.rcNormalPosition.top;
	mc.cx		= wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	mc.cy		= wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	mc.style	= WS_CHILD | WS_CAPTION | WS_CLIPSIBLINGS | WS_MINIMIZEBOX | WS_SYSMENU |
					(wp.showCmd & SW_SHOWMINIMIZED ? WS_MINIMIZE : 0);
	mc.lParam	= 0;
	if ((hwndTuning = (HWND)SendMessage (hwndClient,
									WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT)&mc)) == (HWND)NULL)
	{
		// Error
		MyMessageBox (NULL, ResourceString (IDS_UT_ERR_CREATING_WINDOW), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Display any loaded user tuning table
	DisplayTuning ();

	// Reposition the window
	GetWindowPosition (USER_TUNING_WINDOW, hwndTuning, FALSE);

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		DestroyUserTuningWindow
 Parameters:	None.
 Returns:		None.
 Description:	Destroys the user tuning window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 17/6/94 	| Created.											| PW
*****************************************************************************/

VOID DestroyUserTuningWindow (VOID)
{
	// Save the window position
	SaveWindowPosition (USER_TUNING_WINDOW, hwndTuning);

	// Destroy the window
	SendMessage (hwndClient, WM_MDIDESTROY, (WPARAM)hwndTuning, 0);
	hwndTuning = (HWND)NULL;
}

/*^L*/
/*****************************************************************************
 Function:		TuningDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if processed, else FALSE.
 Description:	The user tuning dialog window procedure.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 17/6/94 	| Created.											| PW
*****************************************************************************/

WINDOW_PROC TuningDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	char		szString[100];
	static int	nCurSel;
	int			nCoarse, nFine;
	int			n;
	DWORD		dwVal;
	static int	cxWin, cyWin;
	static BOOL	fInitialised;

	switch (nMsg)
	{
		// Initialisation
		case WM_CREATE:
		{
			// Put the dialog in the window
			fInitialised = FALSE;
			dwVal = DialogInWindow (hWnd, "DLG_USER_TUNING");
			cxWin = HIWORD (dwVal);
			cyWin = LOWORD (dwVal);

			// Set the sliders up
			InitSlider (hWnd, IDC_USER_TUNING_COARSE, 0, 127, 0);
			InitSlider (hWnd, IDC_USER_TUNING_FINE, 0, 63, 0);

			// Set the listbox tabs
			SetListBoxTabs (hWnd, IDC_USER_TUNING_LISTBOX, &TuningListboxTabs[0], ARRAY_SIZE (TuningListboxTabs));

			// Reformat myself
			ReformatDialog (hWnd, RDF_THINFONT);

			// Show the window
			ShowWindow (hWnd, SW_SHOW);

			// Clean
			fDirtyTuning = FALSE;

			fInitialised = TRUE;
			return (0);
		}

		// Trying to change the dialog width
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO FAR	*lpmm = (MINMAXINFO FAR *)lParam;

			if (fInitialised)
			{
				lpmm->ptMinTrackSize.x	= cxWin;
				lpmm->ptMinTrackSize.y	= cyWin;
				lpmm->ptMaxTrackSize.x	= cxWin;
				lpmm->ptMaxTrackSize.y	= cyWin;
			}
			break;
		}

		// Left button down
		case WM_LBUTTONDOWN:
		{
			// Put myself of top of the other children
			SetWindowPos (hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			break;
		}

		// Control
		case WM_COMMAND:
		{
			//
			switch (LOWORD (wParam))
			{
				// Preset list
				case IDC_PRESETS_LIST:
				{
					// Selecting a new key
					if (NCODE == LBN_SELCHANGE)
					{
						// Get the new selection
						nCurSel = SendDlgItemMessage (hWnd, IDC_USER_TUNING_LISTBOX, LB_GETCURSEL, 0, 0);

						// Get the coarse / fine tuning
						nCoarse	= COARSE_TUNE_BYTE (WNUM (TuningTableCopy.TuneValue[nCurSel]));
						nFine	= FINE_TUNE_BYTE (WNUM (TuningTableCopy.TuneValue[nCurSel]));

						// Set the slider positions
						SendDlgItemMessage (hWnd, IDC_USER_TUNING_COARSE, SL_SET_CURRENT, nCoarse, 0);
						SendDlgItemMessage (hWnd, IDC_USER_TUNING_FINE, SL_SET_CURRENT, nFine, 0);
					}
					break;
				}

				// Send changes to Morpheus
				case IDC_USER_TUNING_SEND:
				{
					// Save it
					StoreTuning (hWnd);
					break;
				}

				// Abandon changes to tuning table
				case IDC_USER_TUNING_ABORT:
				{
					// Abandon editing
					AbandonTuning (hWnd);
					break;
				}

				// Unknown control
				default:
				{
					break;
				}
			}

			return (0);
		}

		// Sliders
		case SLN_NEW_VALUE:
		{
			switch (LOWORD (wParam))
			{
				case IDC_USER_TUNING_COARSE:
				{
					// Store the new slider value
					nFine = FINE_TUNE_BYTE (WNUM (TuningTableCopy.TuneValue[nCurSel]));
					WriteSysexWord (&TuningTableCopy.TuneValue[nCurSel], MAKE_TUNE_WORD (lParam, nFine));

                    // Update the listbox
					FormatTuningNote (&szString[0], nCurSel);
					SendDlgItemMessage (hWnd, IDC_USER_TUNING_LISTBOX, WM_SETREDRAW, FALSE, 0);
					n = SendDlgItemMessage (hWnd, IDC_USER_TUNING_LISTBOX, LB_GETCURSEL, 0, 0);
					SendDlgItemMessage (hWnd, IDC_USER_TUNING_LISTBOX, LB_DELETESTRING, nCurSel, 0);
					SendDlgItemMessage (hWnd, IDC_USER_TUNING_LISTBOX, LB_INSERTSTRING, nCurSel, (LPARAM)(LPCSTR)&szString[0]);
					SendDlgItemMessage (hWnd, IDC_USER_TUNING_LISTBOX, LB_SETCURSEL, n, 0);
					SendDlgItemMessage (hWnd, IDC_USER_TUNING_LISTBOX, WM_SETREDRAW, TRUE, 0);
					break;
				}

				case IDC_USER_TUNING_FINE:
				{
					// Store the new slider value
					nCoarse = COARSE_TUNE_BYTE (WNUM (TuningTableCopy.TuneValue[nCurSel]));
					WriteSysexWord (&TuningTableCopy.TuneValue[nCurSel], MAKE_TUNE_WORD (nCoarse, lParam));

					// Update the listbox
					FormatTuningNote (&szString[0], nCurSel);
					SendDlgItemMessage (hWnd, IDC_USER_TUNING_LISTBOX, WM_SETREDRAW, FALSE, 0);
					n = SendDlgItemMessage (hWnd, IDC_USER_TUNING_LISTBOX, LB_GETCURSEL, 0, 0);
					SendDlgItemMessage (hWnd, IDC_USER_TUNING_LISTBOX, LB_DELETESTRING, nCurSel, 0);
					SendDlgItemMessage (hWnd, IDC_USER_TUNING_LISTBOX, LB_INSERTSTRING, nCurSel, (LPARAM)(LPCSTR)&szString[0]);
					SendDlgItemMessage (hWnd, IDC_USER_TUNING_LISTBOX, LB_SETCURSEL, n, 0);
					SendDlgItemMessage (hWnd, IDC_USER_TUNING_LISTBOX, WM_SETREDRAW, TRUE, 0);
					break;
				}

				default:
				{
					break;
				}
			}

			return (0);
        }

		// Close
		case WM_CLOSE:
		{
			break;
		}

		// Destruction
		case WM_DESTROY:
		{
			break;
		}

		// Control about to be drawn
		case WM_CTLCOLOR:
		{
			HBRUSH	hBrush;

			hBrush = Ctl3dCtlColorEx (nMsg, wParam, lParam);
			if (hBrush != (HBRUSH)FALSE)
			{
				return ((LRESULT)hBrush);
			}
			break;
		}

		// Default messages
		default:
		{
			break;
		}
	}

	// Unprocessed
	return (DefMDIChildProc (hWnd, nMsg, wParam, lParam));
}

/*^L*/
/*****************************************************************************
 Function:		DisplayTuning
 Parameters:	None.
 Returns:		None.
 Description:	Displays the user tuning table.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 17/6/94 	| Created.											| PW
*****************************************************************************/

VOID DisplayTuning (VOID)
{
	UINT	i;
	char	szBuf[100];
	int		nSelected;

	// If there is no tuning window
	if (hwndTuning == (HWND)NULL)
	{
		// Exit
		return;
	}

	// Take a local copy of the tuning table
	memcpy (&TuningTableCopy, &MorpheusTuningTable, sizeof (TUNING_TABLE_PARAMS));
	    
	// Get the current selection
	nSelected = SendDlgItemMessage (hwndTuning, IDC_USER_TUNING_LISTBOX, LB_GETCURSEL, 0, 0);

	// Clear out the current list
	SendDlgItemMessage (hwndTuning, IDC_USER_TUNING_LISTBOX, LB_RESETCONTENT, 0, 0);

	// For each MIDI note in the list
	for (i=0 ; i<NUM_MIDI_NOTES ; i++)
	{
		// Format the note tuning line
		FormatTuningNote (&szBuf[0], i);

		// Add the entry to the listbox
		SendDlgItemMessage (hwndTuning, IDC_USER_TUNING_LISTBOX, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)&szBuf[0]);
	}

	// Reselect
	SendDlgItemMessage (hwndTuning, IDC_USER_TUNING_LISTBOX, LB_SETCURSEL, nSelected, 0);
}

/*^L*/
/*****************************************************************************
 Function:		FormatTuningNote
 Parameters:	None.
 Returns:		None.
 Description:	Displays the user tuning table.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 17/6/94 	| Created.											| PW
*****************************************************************************/

static VOID FormatTuningNote (LPSTR lpszBuf, int nNote)
{
	static LPCSTR	szNoteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	int				nOctave;
	int				len = 0;
	int				nCoarse, nFine;

	// First the MIDI note
	nOctave = (nNote / 12) - 2;
	len += wsprintf (&lpszBuf[len], "%s%d", szNoteNames[nNote % 12], nOctave);

	// Add the coarse + fine tuning
	nCoarse = COARSE_TUNE_BYTE (WNUM (TuningTableCopy.TuneValue[nNote]));
	nFine = FINE_TUNE_BYTE (WNUM (TuningTableCopy.TuneValue[nNote]));
	len += wsprintf (&lpszBuf[len], "\t%d\t%d", nCoarse, nFine);
}

/*^L*/
/*****************************************************************************
 Function:		StoreTuning
 Parameters:	HWND	hwndParent		Parent window.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Saves the tuning map.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 17/6/94 	| Created.											| PW
*****************************************************************************/

static BOOL StoreTuning (HWND hwndParent)
{
	// First - copy the local tuning table back to the global array
	hmemcpy (&MorpheusTuningTable, &TuningTableCopy, sizeof (TUNING_TABLE_PARAMS));

	// If we are connected to the Morpheus
	if (fConnectedToMorpheus)
	{
		// Send it back to the Morpheus
		if (!MorpheusSendTuning (hwndParent, &MorpheusTuningTable))
		{
			MyMessageBox (hwndParent, ResourceString (IDS_UT_ERR_SENDING), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			return (FALSE);
		}
	}

	// OK
	fDirtyTuning = FALSE;
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		AbandonTuning
 Parameters:	HWND	hwndParent		Parent window.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Abandons the tuning table edit.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 17/6/94 	| Created.											| PW
*****************************************************************************/

static BOOL AbandonTuning (HWND hwndParent)
{
	// Copy the main tuning table back to the local copy
	hmemcpy (&TuningTableCopy, &MorpheusTuningTable, sizeof (TUNING_TABLE_PARAMS));

	// Display the table again
	DisplayTuning ();

	// OK
	fDirtyTuning = FALSE;
	return (TRUE);
}
