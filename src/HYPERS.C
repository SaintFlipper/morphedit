/*****************************************************************************
 File:			hypers.c
 Description:	The hyperpresets module for the Morpheus editor.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 28/5/94	| Created.											| PW
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

HYPERPRESET_PARAMS	CurrentHyper;					// The hyperpreset being worked on (local copy)
int					nDisplayedHyper	= INDEX_NONE;	// Currently selected hyperpreset in list
BOOL				fDirtyHyper = FALSE;			// Whether the current hyperpreset has been modified

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

static BOOL StoreHyper (HWND hwndParent, int idHyper);
static BOOL AbandonHyper (HWND hwndParent, int idHyper);
static VOID FormatHyperName (LPSTR lpszBuf, int idHyper);

/*^L*/
/*****************************************************************************
 Function:		CreateAllHyperWindows
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Creates all the hyperpreset windows :-
				a)	Hyperpresets list
				b)	Main hyperpreset editor dialog
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 28/5/94 	| Created.											| PW
*****************************************************************************/

BOOL CreateAllHyperWindows (VOID)
{
	// Create the hyperpresets list window
	if (!CreateHypersListWindow ())
	{
		// Error
		return (FALSE);
	}

	// Create the main hyperpresets editor dialog
	if (!CreateCurrentHyperWindow ())
	{
		// Error
		DestroyHypersListWindow ();
		return (FALSE);
	}

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		DestroyAllHyperWindows
 Parameters:	None.
 Returns:		None.
 Description:	Destroys all the hyperpreset windows.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 28/5/94 	| Created.											| PW
*****************************************************************************/

VOID DestroyAllHyperWindows (VOID)
{
	DestroyCurrentHyperWindow ();
	DestroyHypersListWindow ();
}

/*^L*/
/*****************************************************************************
 Function:		CreateHypersListWindow
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Creates the hyperpresets list window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 28/5/94 	| Created.											| PW
*****************************************************************************/

BOOL CreateHypersListWindow (VOID)
{
	MDICREATESTRUCT	mc;
	WINDOWPLACEMENT	wp;

	// Create the window
	GetCreatePosition (HYPERPRESETS_LIST_WINDOW, FALSE, &wp);
	mc.szClass	= WCNAME_HYPERS_LIST;
	mc.szTitle	= "";
	mc.hOwner	= hInst;
	mc.x		= wp.rcNormalPosition.left;
	mc.y		= wp.rcNormalPosition.top;
	mc.cx		= wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	mc.cy		= wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	mc.style	= WS_CAPTION | WS_CLIPSIBLINGS | WS_MINIMIZEBOX | WS_SYSMENU |
					(wp.showCmd & SW_SHOWMINIMIZED ? WS_MINIMIZE : 0);
	mc.lParam	= 0;
	if ((hwndHypersList = (HWND)SendMessage (hwndClient,
			WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT)&mc)) == (HWND)NULL)
	{
		// Error
		MyMessageBox (NULL, ResourceString (IDS_ERROR_CREATING_HPS_LIST), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Reposition the window
	GetWindowPosition (HYPERPRESETS_LIST_WINDOW, hwndHypersList, FALSE);

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		DestroyHypersListWindow
 Parameters:	None.
 Returns:		None.
 Description:	Destroys the hyperpresets list window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 28/5/94 	| Created.											| PW
*****************************************************************************/

VOID DestroyHypersListWindow (VOID)
{
	// Save the window position
	SaveWindowPosition (HYPERPRESETS_LIST_WINDOW, hwndHypersList);

	// Destroy the window
	SendMessage (hwndClient, WM_MDIDESTROY, (WPARAM)hwndHypersList, 0);
	hwndHypersList = (HWND)NULL;
}

/*^L*/
/*****************************************************************************
 Function:		HypersListDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if processed, else FALSE.
 Description:	The hyperpresets list dialog window procedure.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 28/5/94 	| Created.											| PW
*****************************************************************************/

WINDOW_PROC HypersListDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	DWORD		dwVal;
	static int	cxWin, cyWin;
	static BOOL	fInitialised;

	switch (nMsg)
	{
		// Initialisation
		case WM_CREATE:
		{
			// Put the dialog int the window
			fInitialised = FALSE;
			dwVal = DialogInWindow (hWnd, "DLG_NAMES_LIST");
			cxWin = HIWORD (dwVal);
			cyWin = LOWORD (dwVal);
			fInitialised = TRUE;

			// Set my title
			SetWindowText (hWnd, ResourceString (IDS_HPS_LIST_WINDOW_TITLE));

#ifdef NEVER
			// Find the Save/Abandon button height
			GetClientRect (GetDlgItem (hWnd, IDC_PRESET_SAVE_CHANGES), &rcButton);

			// Find the dialog client size
			GetClientRect (hWnd, &rcDialog);

			// Move the listbox to just underneath the buttons and the full width of the dialog
			MoveWindow (GetDlgItem (hWnd, IDC_PRESETS_LIST),
						0, rcButton.bottom,
						rcDialog.right - rcDialog.left,
						rcDialog.bottom - rcButton.bottom,
						TRUE);
#endif

			// Reformat myself
			ReformatDialog (hWnd, RDF_THINFONT);

			// Set the column width
			{
				RECT	rcClient;

				GetClientRect (GetDlgItem (hWnd, IDC_PRESETS_LIST), &rcClient);
				SendDlgItemMessage (hWnd, IDC_PRESETS_LIST, LB_SETCOLUMNWIDTH, rcClient.right / 2, 0);
			}

			// Display any loaded hyperpreset list
			hwndHypersList = hWnd;
			DisplayHypersList ();

			// Set 'displayed hyperpreset' index to NONE
			nDisplayedHyper = INDEX_NONE;

			// Show the window
			// ShowWindow (hWnd, SW_SHOW);

			// Clean
			fDirtyHyper = FALSE;

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
						// If the current hyperpreset has been changed
						if (fDirtyHyper)
						{
							// Ask the user whether to send the hyperpreset back
							if (MyMessageBox (hWnd,
								ResourceString (IDS_MODIFIED_HP_NOT_SAVED),
								ResourceString (IDS_QUESTION_TITLE), MB_ICONQUESTION | MB_YESNO) == IDYES)
							{
								// Save it
								StoreHyper (hWnd, nDisplayedHyper);
							}
						}

						// Find out which the currently selected preset is
						nDisplayedHyper = SendDlgItemMessage (hWnd, IDC_PRESETS_LIST, LB_GETCURSEL, 0, 0);

						// If we are connected to the Morpheus
						if (fConnectedToMorpheus)
						{
							// Read the hyperpreset
							if (!MorpheusGetHyper (nDisplayedHyper))
							{
								// Error
								nDisplayedHyper = INDEX_NONE;
								break;
							}

							// Select that hyperpreset (for later editing)
							MorpheusSelectHyper (hWnd, nDisplayedHyper);
						}

						// Take a copy of the hyperpreset for editing purposes
						hmemcpy (&CurrentHyper, pMorpheusHyperPresets[nDisplayedHyper], sizeof (CurrentHyper));

						// Display the hyperpreset
						DisplayHyper (nDisplayedHyper);

						// Clean
						fDirtyHyper = FALSE;
					}
					return (0);
				}

				// Save changes to current preset
				case IDC_PRESET_SAVE_CHANGES:
				{
					// If the current hyperpreset has been changed
					if (fDirtyHyper)
					{
						// Save it
						StoreHyper (hWnd, nDisplayedHyper);
					}
					return (0);
				}

				// Abandon changes to current hyperpreset
				case IDC_PRESET_ABANDON_CHANGES:
				{
					// If the current preset has been changed
					if (fDirtyHyper)
					{
						// Abandon editing
						AbandonHyper (hWnd, nDisplayedHyper);
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
			// No hyperpreset selected
			nDisplayedHyper = INDEX_NONE;
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
 Function:		DisplayHypersList
 Parameters:	None.
 Returns:		None.
 Description:	Redisplays the whole hyperpresets list.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 28/5/94 	| Created.											| PW
*****************************************************************************/

VOID DisplayHypersList (VOID)
{
	UINT	i, num;
	char	szBuf[40];
	int		nSelected;

	// If there is no hyperpresets list OR there is no list window
	if ((pMorpheusHyperpresetList == NULL) || (hwndHypersList == (HWND)NULL))
	{
		// Exit
		return;
	}

	// Get the current selection
	nSelected = SendDlgItemMessage (hwndHypersList, IDC_PRESETS_LIST, LB_GETCURSEL, 0, 0);

	// Clear out the current list
	SendDlgItemMessage (hwndHypersList, IDC_PRESETS_LIST, LB_RESETCONTENT, 0, 0);

	// For each hyperpreset in the list
	num = ReadSysexWord (&pMorpheusHyperpresetList->wNumHyperpresets);
	for (i=0 ; i<num ; i++)
	{
		// Format the hyperpreset name
		FormatHyperName (&szBuf[0], i);

		// Add the hyperpreset name to the listbox
		SendDlgItemMessage (hwndHypersList, IDC_PRESETS_LIST, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)&szBuf[0]);
	}

	// Reselect
	SendDlgItemMessage (hwndHypersList, IDC_PRESETS_LIST, LB_SETCURSEL, nSelected, 0);
}

/*^L*/
/*****************************************************************************
 Function:		StoreHyper
 Parameters:	HWND	hwndParent		Parent window.
				int		idHyper			Hyper index to save.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Saves the specified hyperpreset.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 28/5/94 	| Created.											| PW
*****************************************************************************/

static BOOL StoreHyper (HWND hwndParent, int idHyper)
{
	int		nCurSel;

	// First - copy the local hyperpreset back to the global array
	hmemcpy (pMorpheusHyperPresets[idHyper], &CurrentHyper, sizeof (CurrentHyper));

	// If we are connected to the Morpheus
	if (fConnectedToMorpheus)
	{
		// Send it back to the Morpheus
		if (!MorpheusSendHyper (hwndParent, idHyper))
		{
			MyMessageBox (hwndParent, ResourceString (IDS_ERROR_SENDING_HP), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			return (FALSE);
		}
	}

	// Update the list of hyperpreset names from the loaded hyperpreset.
	CreateHyperNames (idHyper);

	// Replace the hyperpreset list name for this preset
	nCurSel = SendDlgItemMessage (hwndHypersList, IDC_PRESETS_LIST, LB_GETCURSEL, 0, 0);
	DisplayHypersList ();
	SendDlgItemMessage (hwndHypersList, IDC_PRESETS_LIST, LB_SETCURSEL, nCurSel, 0);

	// OK
	fDirtyHyper = FALSE;
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		AbandonHyper
 Parameters:	HWND	hwndParent		Parent window.
				int		idHyper			Hyperpreset index to save.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Abandons the current hyperpreset editing.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 28/5/94 	| Created.											| PW
*****************************************************************************/

static BOOL AbandonHyper (HWND hwndParent, int idHyper)
{
	// If we are connected to the Morpheus
	if (fConnectedToMorpheus)
	{
		// Re-read the hyperpreset
		MorpheusGetHyper (idHyper);

		// Select that hyperpreset (for later editing)
		MorpheusSelectHyper (hwndParent, idHyper);
	}

	// Take a copy of the hyperpreset for editing
	hmemcpy (&CurrentHyper, pMorpheusHyperPresets[idHyper], sizeof (CurrentHyper));

	// Display the hyperpreset
	DisplayHyper (idHyper);

	// OK
	fDirtyHyper = FALSE;
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		FormatHyperName
 Parameters:	LPSTR	lpszBuf
				int		idHyper
 Returns:		None.
 Description:	Formats the specified hyperpreset name.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 28/5/94 	| Created.											| PW
*****************************************************************************/

static VOID FormatHyperName (LPSTR lpszBuf, int idHyper)
{
	wsprintf (&lpszBuf[0], "%u: %s", idHyper, &pMorpheusHyperpresetList->Info[idHyper].szName[0]);
}

