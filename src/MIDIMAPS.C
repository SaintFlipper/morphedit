/*****************************************************************************
 File:			midimaps.c
 Description:	The MIDIMAPS module for the Morpheus editor.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94		| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

/****************************************************************************/
/*								Global data									*/
/****************************************************************************/

MIDIMAP_PARAMS		CurrentMM;						// The midimap being worked on (local copy)
int					nDisplayedMM	= INDEX_NONE;	// Currently selected midimap in list
BOOL				fDirtyMM = FALSE;				// Whether the current midimap has been modified

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

static BOOL StoreMM (HWND hwndParent, int idMM);
static BOOL AbandonMM (HWND hwndParent, int idMM);
static VOID FormatMMName (LPSTR lpszBuf, int idMM);

/*^L*/
/*****************************************************************************
 Function:		CreateAllMMWindows
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Creates all the midimap windows :-
				a)	MMpresets list
				b)	Main midimap editor dialog
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94		| Created.											| PW
*****************************************************************************/

BOOL CreateAllMMWindows (VOID)
{
	// Create the midimaps list window
	if (!CreateMMsListWindow ())
	{
		// Error
		return (FALSE);
	}

	// Create the main midimaps editor dialog
	if (!CreateCurrentMMWindow ())
	{
		// Error
		DestroyMMsListWindow ();
		return (FALSE);
	}

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		DestroyAllMMWindows
 Parameters:	None.
 Returns:		None.
 Description:	Destroys all the midimap windows.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94		| Created.											| PW
*****************************************************************************/

VOID DestroyAllMMWindows (VOID)
{
	DestroyCurrentMMWindow ();
	DestroyMMsListWindow ();
}

/*^L*/
/*****************************************************************************
 Function:		CreateMMsListWindow
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Creates the midimaps list window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94		| Created.											| PW
*****************************************************************************/

BOOL CreateMMsListWindow (VOID)
{
	MDICREATESTRUCT	mc;
	WINDOWPLACEMENT	wp;

	// Create the window
	GetCreatePosition (MIDIMAPS_LIST_WINDOW, FALSE, &wp);
	mc.szClass	= WCNAME_MMS_LIST;
	mc.szTitle	= "";
	mc.hOwner	= hInst;
	mc.x		= wp.rcNormalPosition.left;
	mc.y		= wp.rcNormalPosition.top;
	mc.cx		= wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	mc.cy		= wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	mc.style	= WS_CHILD | WS_CAPTION | WS_CLIPSIBLINGS | WS_MINIMIZEBOX | WS_SYSMENU |
					(wp.showCmd & SW_SHOWMINIMIZED ? WS_MINIMIZE : 0);
	mc.lParam	= 0;
	if ((hwndMMsList = (HWND)SendMessage (hwndClient,
									WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT)&mc)) == (HWND)NULL)
	{
		// Error
		MyMessageBox (NULL, ResourceString (IDS_MM_LIST_CREATING_WINDOW), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Reposition the window
	GetWindowPosition (MIDIMAPS_LIST_WINDOW, hwndMMsList, FALSE);

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		DestroyMMsListWindow
 Parameters:	None.
 Returns:		None.
 Description:	Destroys the midimaps list window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94		| Created.											| PW
*****************************************************************************/

VOID DestroyMMsListWindow (VOID)
{
	// Save the window position
	SaveWindowPosition (MIDIMAPS_LIST_WINDOW, hwndMMsList);

	// Destroy the window
	SendMessage (hwndClient, WM_MDIDESTROY, (WPARAM)hwndMMsList, 0);
	hwndMMsList = (HWND)NULL;
}

/*^L*/
/*****************************************************************************
 Function:		MMsListDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if processed, else FALSE.
 Description:	The midimaps list dialog window procedure.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94		| Created.											| PW
*****************************************************************************/

WINDOW_PROC MMsListDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	DWORD		dwVal;
	static int	cxWin, cyWin;
	static BOOL	fInitialised;

	switch (nMsg)
	{
		// Initialisation
		case WM_CREATE:
		{
			RECT	rcClient;

			// Put the dialog int the window
			fInitialised = FALSE;
			dwVal = DialogInWindow (hWnd, "DLG_MM_NAMES_LIST");
			cxWin = HIWORD (dwVal);
			cyWin = LOWORD (dwVal);
			fInitialised = TRUE;

			// Set my title
			SetWindowText (hWnd, ResourceString (IDS_MM_LIST_WINDOW_TITLE));

			// Reformat myself
			ReformatDialog (hWnd, RDF_THINFONT);

			// Set the column width
			GetClientRect (GetDlgItem (hWnd, IDC_PRESETS_LIST), &rcClient);
			SendDlgItemMessage (hWnd, IDC_PRESETS_LIST, LB_SETCOLUMNWIDTH, rcClient.right / 2, 0);

			// Display any loaded midimap list
			hwndMMsList = hWnd;
			DisplayMMsList ();

			// Set 'displayed midimap' index to NONE
			nDisplayedMM = INDEX_NONE;

			ShowWindow (hWnd, SW_HIDE);

			// Clean
			fDirtyMM = FALSE;

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
					// Double click
					if (NCODE == LBN_DBLCLK)
					{
						// If the current midimap has been changed
						if (fDirtyMM)
						{
							// Ask the user whether to send the midimap back
							if (MyMessageBox (hWnd,
								ResourceString (IDS_MODIFIED_MM_NOT_SAVED),
								ResourceString (IDS_QUESTION_TITLE),
								MB_ICONQUESTION | MB_YESNO) == IDYES)
							{
								// Save it
								StoreMM (hWnd, nDisplayedMM);
							}
						}

						// Find out which the currently selected preset is
						nDisplayedMM = SendDlgItemMessage (hWnd, IDC_PRESETS_LIST, LB_GETCURSEL, 0, 0);

						// If we are connected to the Morpheus
						if (fConnectedToMorpheus)
						{
							// Read the midimap
							if (!MorpheusGetMM (nDisplayedMM))
							{
								// Error
								nDisplayedMM = INDEX_NONE;
								break;
							}

							// Select that midimap (for later editing)
							MorpheusSelectMM (hWnd, nDisplayedMM);
						}

						// Take a copy of the midimap for editing purposes
						hmemcpy (&CurrentMM, pMorpheusMidimaps[nDisplayedMM], sizeof (CurrentMM));

						// Display the midimap
						DisplayMM (nDisplayedMM);

						// Clean
						fDirtyMM = FALSE;
					}
					return (0);
				}

				// Save changes to current preset
				case IDC_PRESET_SAVE_CHANGES:
				{
					// If the current midimap has been changed
					if (fDirtyMM)
					{
						// Save it
						StoreMM (hWnd, nDisplayedMM);
					}
					return (0);
				}

				// Abandon changes to current midimap
				case IDC_PRESET_ABANDON_CHANGES:
				{
					// If the current preset has been changed
					if (fDirtyMM)
					{
						// Abandon editing
						AbandonMM (hWnd, nDisplayedMM);
					}
					return (0);
				}

				// Unknown control
				default:
				{
					break;
				}
			}

			break;
		}

		// Close
		case WM_CLOSE:
		{
			break;
		}

		// Destruction
		case WM_DESTROY:
		{
			nDisplayedMM = INDEX_NONE;
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
 Function:		DisplayMMsList
 Parameters:	None.
 Returns:		None.
 Description:	Redisplays the whole midimaps list.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94		| Created.											| PW
*****************************************************************************/

VOID DisplayMMsList (VOID)
{
	UINT	i, num;
	char	szBuf[40];
	int		nSelected;

	// If there is no midimaps list OR there is no list window
	if ((pMorpheusMidimapList == NULL) || (hwndMMsList == (HWND)NULL))
	{
		// Exit
		return;
	}

	// Get the current selection
	nSelected = SendDlgItemMessage (hwndMMsList, IDC_PRESETS_LIST, LB_GETCURSEL, 0, 0);

	// Clear out the current list
	SendDlgItemMessage (hwndMMsList, IDC_PRESETS_LIST, LB_RESETCONTENT, 0, 0);

	// For each midimap in the list
	num = ReadSysexWord (&pMorpheusMidimapList->wNumMidimaps);
	for (i=0 ; i<num ; i++)
	{
		// Format the midimap name
		FormatMMName (&szBuf[0], i);

		// Add the midimap name to the listbox
		SendDlgItemMessage (hwndMMsList, IDC_PRESETS_LIST, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)&szBuf[0]);
	}

	// Reselect
	SendDlgItemMessage (hwndMMsList, IDC_PRESETS_LIST, LB_SETCURSEL, nSelected, 0);
}

/*^L*/
/*****************************************************************************
 Function:		StoreMM
 Parameters:	HWND	hwndParent		Parent window.
				int		idMM			MM index to save.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Saves the specified midimap.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94		| Created.											| PW
*****************************************************************************/

static BOOL StoreMM (HWND hwndParent, int idMM)
{
	int		nCurSel;

	// First - copy the local midimap back to the global array
	hmemcpy (pMorpheusMidimaps[idMM], &CurrentMM, sizeof (CurrentMM));

	// If we are connected to the Morpheus
	if (fConnectedToMorpheus)
	{
		// Send it back to the Morpheus
		if (!MorpheusSendMM (hwndParent, idMM))
		{
			MyMessageBox (hwndParent, ResourceString (IDS_MM_ERROR_SENDING), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			return (FALSE);
		}
	}

	// Update the list of midimap names from the loaded midimap.
	CreateMMNames (idMM);

	// Display the updated list and re-select the current item
	nCurSel = SendDlgItemMessage (hwndMMsList, IDC_PRESETS_LIST, LB_GETCURSEL, 0, 0);
	DisplayMMsList ();
	SendDlgItemMessage (hwndMMsList, IDC_PRESETS_LIST, LB_SETCURSEL, nCurSel, 0);

	// OK
	fDirtyMM = FALSE;
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		AbandonMM
 Parameters:	HWND	hwndParent		Parent window.
				int		idMM			MMpreset index to save.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Abandons the current midimap editing.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94		| Created.											| PW
*****************************************************************************/

static BOOL AbandonMM (HWND hwndParent, int idMM)
{
	// Recopy into the edit buffer
	hmemcpy (&CurrentMM, pMorpheusMidimaps[idMM], sizeof (CurrentMM));

	// If we are connected to the Morpheus
	if (fConnectedToMorpheus)
	{
		// Select that midimap (for later editing)
		MorpheusSelectMM (hwndParent, idMM);
	}

	// Display the midimap
	DisplayMM (idMM);

	// OK
	fDirtyMM = FALSE;
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		FormatMMName
 Parameters:	LPSTR	lpszBuf
				int		idMM
 Returns:		None.
 Description:	Formats the specified midimap name.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94		| Created.											| PW
*****************************************************************************/

static VOID FormatMMName (LPSTR lpszBuf, int idMM)
{
	wsprintf (&lpszBuf[0], "%u: %s", idMM, &pMorpheusMidimapList->Info[idMM].szName[0]);
}

