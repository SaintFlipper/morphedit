/*****************************************************************************
 File:			progmap.c
 Description:	The program map editing module for the Morpheus editor.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/6/94 	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

static LPCSTR			lpszMapName[] = {"1", "2", "3", "4"};
static LISTBOX_TAB_INFO	ProgramMapListboxTabs[] =
{
	{IDC_PM_LABEL1,	0},
	{IDC_PM_LABEL2,	20},
};

/****************************************************************************/
/*								Global data									*/
/****************************************************************************/

// The working copy of the selected program map
PROGRAM_MAP_PARAMS	ProgramMapCopy;

// Whether the selected program map has been modified
BOOL				fDirtyProgramMap = FALSE;

// The currently selected program map
int					nSelectedProgramMap = INDEX_NONE;

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

static BOOL StoreProgramMap (HWND hwndParent, int nMap);
static BOOL AbandonProgramMap (HWND hwndParent, int nMap);

/*^L*/
/*****************************************************************************
 Function:		CreateProgramMapWindow
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Creates the program map window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/6/94 	| Created.											| PW
*****************************************************************************/

BOOL CreateProgramMapWindow (VOID)
{
	MDICREATESTRUCT	mc;
	WINDOWPLACEMENT	wp;

	// Create the window
	GetCreatePosition (PROGRAM_MAP_WINDOW, FALSE, &wp);
	mc.szClass	= WCNAME_PROGRAM_MAP;
	mc.szTitle	= ResourceString (IDS_PM_WINDOW_TITLE);
	mc.hOwner	= hInst;
	mc.x		= wp.rcNormalPosition.left;
	mc.y		= wp.rcNormalPosition.top;
	mc.cx		= wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	mc.cy		= wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	mc.style	= WS_CHILD | WS_CAPTION | WS_CLIPSIBLINGS | WS_MINIMIZEBOX | WS_SYSMENU |
					(wp.showCmd & SW_SHOWMINIMIZED ? WS_MINIMIZE : 0);
	mc.lParam	= 0;
	if ((hwndProgramMap = (HWND)SendMessage (hwndClient,
									WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT)&mc)) == (HWND)NULL)
	{
		// Error
		MyMessageBox (NULL, ResourceString (IDS_PM_ERR_CREAT_WND), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Display any loaded program map
	DisplayProgramMap ();

	// Reposition the window
	GetWindowPosition (PROGRAM_MAP_WINDOW, hwndProgramMap, FALSE);

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		DestroyProgramMapWindow
 Parameters:	None.
 Returns:		None.
 Description:	Destroys the program map window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/6/94 	| Created.											| PW
*****************************************************************************/

VOID DestroyProgramMapWindow (VOID)
{
	// Save the window position
	SaveWindowPosition (PROGRAM_MAP_WINDOW, hwndProgramMap);

	// Destroy the window
	SendMessage (hwndClient, WM_MDIDESTROY, (WPARAM)hwndProgramMap, 0);
	hwndProgramMap = (HWND)NULL;
}

/*^L*/
/*****************************************************************************
 Function:		ProgramMapDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if processed, else FALSE.
 Description:	The program map dialog window procedure.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/6/94 	| Created.											| PW
*****************************************************************************/

WINDOW_PROC ProgramMapDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	char		szString[100];
	static int	nCurEntry;
	int			n, NumPrograms;
	static BOOL	fInitialised;
	DWORD		dwVal;
	static int	cxWin, cyWin;

	switch (nMsg)
	{
		// Initialisation
		case WM_CREATE:
		{
			// Not initialised
			fInitialised = FALSE;

			// Put the dialog int the window
			dwVal = DialogInWindow (hWnd, "DLG_PROGRAM_MAP");
			cxWin = HIWORD (dwVal);
			cyWin = LOWORD (dwVal);

			// Select map number 0
			nSelectedProgramMap = 0;

			// No selected listbox entry
			nCurEntry = INDEX_NONE;

			// Set up the program map listbox tabstops
			SetListBoxTabs (hWnd, IDC_PM_LIST, &ProgramMapListboxTabs[0], ARRAY_SIZE (ProgramMapListboxTabs));

			// Fill up the program map number list
			for (n=0 ; n<ARRAY_SIZE(lpszMapName) ; n++)
			{
				SendDlgItemMessage (hWnd, IDC_PM_CURRENT_MAP, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)lpszMapName[n]);
			}
			SendDlgItemMessage (hWnd, IDC_PM_CURRENT_MAP, CB_SETCURSEL, 0, 0);

			// Fill up the output program list
			NumPrograms = WNUM (MorpheusConfigurationData.wNumPresets) + WNUM (MorpheusConfigurationData.wNumHyperpresets);
//			NumPrograms = WNUM (pMorpheusPresetList->wNumPresets) + WNUM (pMorpheusHyperpresetList->wNumHyperpresets);
			for (n=0 ; n<NumPrograms ; n++)
			{
				SendDlgItemMessage (hWnd, IDC_PM_OUTPUT_PROGRAM, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)ProgramName (n));
			}
			SendDlgItemMessage (hWnd, IDC_PM_OUTPUT_PROGRAM, CB_SETCURSEL, 0, 0);

			// Reformat myself
			ReformatDialog (hWnd, RDF_THINFONT);

			// Show the window
			ShowWindow (hWnd, SW_SHOW);

			// Clean
			fDirtyProgramMap = FALSE;

			// Initialised
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
				// Program mapping list
				case IDC_PM_LIST:
				{
					// Selecting a new map entry
					if (NCODE == LBN_SELCHANGE)
					{
						// Get the new list entry
						nCurEntry = SendDlgItemMessage (hWnd, IDC_PM_LIST, LB_GETCURSEL, 0, 0);

						// Select its output program in the combo box
						SendDlgItemMessage (hWnd, IDC_PM_OUTPUT_PROGRAM, CB_SETCURSEL,
											WNUM (ProgramMapCopy.Program[nCurEntry]), 0);
					}
					return (0);
				}

				// Selecting a different program map
				case IDC_PM_CURRENT_MAP:
				{
					// Selecting a new program map
					if (NCODE == CBN_SELCHANGE)
					{
						// Get the new selection
						n = SendDlgItemMessage (hWnd, IDC_PM_CURRENT_MAP, CB_GETCURSEL, 0, 0);

						// If this is a new map
						if (n != nSelectedProgramMap)
						{
							// If the old map is dirty
							if (fDirtyProgramMap)
							{
                            	// Ask the user what to do
								if (MyMessageBox (hWnd,
									ResourceString (IDS_MODIFIED_PM_NOT_SAVED),
									ResourceString (IDS_QUESTION_TITLE),
									MB_ICONQUESTION | MB_YESNO) == IDYES)
								{
                    	        	// Save it
									StoreProgramMap (hWnd, nSelectedProgramMap);
								}
							}

							// Store the selection index
							nSelectedProgramMap = n;

							// Display the newly selected map
							DisplayProgramMap ();

							// Mark as clean
							fDirtyProgramMap = FALSE;
						}
					}

					return (0);
				}

				// Selecting a different output program
				case IDC_PM_OUTPUT_PROGRAM:
				{
					// Selecting a new output program
					if (NCODE == CBN_SELCHANGE)
					{
						// Get the new selection
						n = SendDlgItemMessage (hWnd, IDC_PM_OUTPUT_PROGRAM, CB_GETCURSEL, 0, 0);

						// If no map or map-index is selected
						if ((nSelectedProgramMap == INDEX_NONE) || (nCurEntry == INDEX_NONE))
						{
							// Do nothing
							return (TRUE);
						}

						// If the new program is different from the stored one
						if (n != WNUM (ProgramMapCopy.Program[nCurEntry]))
						{
							// Store the new program index
							WriteSysexWord (&ProgramMapCopy.Program[nCurEntry], n);

							// Update the listbox
							wsprintf (&szString[0], "%d\t%s", nCurEntry, ProgramName (WNUM (ProgramMapCopy.Program[nCurEntry])));
							SendDlgItemMessage (hWnd, IDC_PM_LIST, WM_SETREDRAW, FALSE, 0);
							SendDlgItemMessage (hWnd, IDC_PM_LIST, LB_DELETESTRING, nCurEntry, 0);
							SendDlgItemMessage (hWnd, IDC_PM_LIST, LB_INSERTSTRING, nCurEntry, (LPARAM)(LPCSTR)&szString[0]);
							SendDlgItemMessage (hWnd, IDC_PM_LIST, LB_SETCURSEL, nCurEntry, 0);
							SendDlgItemMessage (hWnd, IDC_PM_LIST, WM_SETREDRAW, TRUE, 0);

							// Mark as dirty
							fDirtyProgramMap = TRUE;
						}
					}

					break;
				}

				// Send changes to Morpheus
				case IDC_PM_SEND:
				{
					// Save it
					StoreProgramMap (hWnd, nSelectedProgramMap);
					break;
				}

				// Abandon changes to tuning table
				case IDC_PM_ABANDON:
				{
					// Abandon editing
					AbandonProgramMap (hWnd, nSelectedProgramMap);
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

		// Close
		case WM_CLOSE:
		{
			break;
		}

		// Destruction
		case WM_DESTROY:
		{
			// Select no map
			nSelectedProgramMap = INDEX_NONE;

			// No selected listbox entry
			nCurEntry = INDEX_NONE;

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
 Function:		DisplayProgramMap
 Parameters:	None.
 Returns:		None.
 Description:	Displays the specified program map.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/6/94 	| Created.											| PW
*****************************************************************************/

VOID DisplayProgramMap (VOID)
{
	UINT	i;
	char	szBuf[100];
	int		nSelected;

	// If there is no program map window or no map is selected
	if ((hwndProgramMap == (HWND)NULL) || (nSelectedProgramMap == INDEX_NONE))
	{
		// Exit
		return;
	}

	// Take a local copy of the tuning table
	memcpy (&ProgramMapCopy, &MorpheusProgramMaps[nSelectedProgramMap], sizeof (PROGRAM_MAP_PARAMS));

	// Get the current listbox selection
	nSelected = SendDlgItemMessage (hwndProgramMap, IDC_PM_LIST, LB_GETCURSEL, 0, 0);

	// Clear out the current list
	SendDlgItemMessage (hwndProgramMap, IDC_PM_LIST, LB_RESETCONTENT, 0, 0);

	// For each input program number in the list
	for (i=0 ; i<ARRAY_SIZE(ProgramMapCopy.Program) ; i++)
	{
		// Format the program map entry
		wsprintf (&szBuf[0], "%d\t%s", i, ProgramName (WNUM (ProgramMapCopy.Program[i])));

		// Add the entry to the listbox
		SendDlgItemMessage (hwndProgramMap, IDC_PM_LIST, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)&szBuf[0]);
	}

	// Reselect the old listbox entry
	SendDlgItemMessage (hwndProgramMap, IDC_PM_LIST, LB_SETCURSEL, nSelected, 0);

	// If there IS a selected listbox entry
	if (nSelected != LB_ERR)
	{
		// Select its output program in the combo box
		SendDlgItemMessage (hwndProgramMap, IDC_PM_OUTPUT_PROGRAM, CB_SETCURSEL,
							WNUM (ProgramMapCopy.Program[nSelected]), 0);
	}
}

/*^L*/
/*****************************************************************************
 Function:		StoreProgramMap
 Parameters:	HWND	hwndParent		Parent window.
				int		nMap			Program map.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Saves the specified program map.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/6/94 	| Created.											| PW
*****************************************************************************/

static BOOL StoreProgramMap (HWND hwndParent, int nMap)
{
	// First - copy the local program map back to the global array
	memcpy (&MorpheusProgramMaps[nMap], &ProgramMapCopy, sizeof (PROGRAM_MAP_PARAMS));

	// If we are connected to the Morpheus
	if (fConnectedToMorpheus)
	{
		// Send it back to the Morpheus
		if (!MorpheusSendProgramMap (hwndParent, nMap))
		{
			MyMessageBox (hwndParent, ResourceString (IDS_PM_ERR_SENDING), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			return (FALSE);
		}
	}

	// OK
	fDirtyProgramMap = FALSE;
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		AbandonProgramMap
 Parameters:	HWND	hwndParent		Parent window.
				int		nMap			Program map.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Abandons the program map edit.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/6/94 	| Created.											| PW
*****************************************************************************/

static BOOL AbandonProgramMap (HWND hwndParent, int nMap)
{
	// Copy the global program map back to the local copy
	memcpy (&ProgramMapCopy, &MorpheusProgramMaps[nMap], sizeof (PROGRAM_MAP_PARAMS));

	// Display the program map again
	DisplayProgramMap ();

	// OK
	fDirtyProgramMap = FALSE;
	return (TRUE);
}
