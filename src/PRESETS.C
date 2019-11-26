/*****************************************************************************
 File:			presets.c
 Description:	The presets module for the Morpheus editor.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 27/3/94	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

//static PRESET_PARAMS	SavedPreset;			// The saved copy of the current preset

/****************************************************************************/
/*								Global data									*/
/****************************************************************************/

// Copy of preset being edited
PRESET_PARAMS	EditPreset;

// Currently selected preset in list
int				nDisplayedPreset = INDEX_NONE;

// Whether the current preset has been modified
BOOL	fDirtyPreset = FALSE;

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

static BOOL StorePreset (HWND hwndParent, int idPreset);
static BOOL AbandonPreset (HWND hwndParent, int idPreset);
static VOID FormatPresetName (LPSTR lpszBuf, int idPreset);

/*^L*/
/*****************************************************************************
 Function:		CreateAllPresetWindows
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Creates all the preset windows :-
				a)	Presets list
				b)	Main preset editor dialog
				c)	Note-on patchbay
				d)	Real-time patchbay
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/5/94 	| Created.											| PW
*****************************************************************************/

BOOL CreateAllPresetWindows (VOID)
{
	// Create the presets list window
	if (!CreatePresetsListWindow ())
	{
		// Error
		return (FALSE);
	}

	// Create the main preset editor dialog
	if (!CreateCurrentPresetWindow ())
	{
		// Error
		DestroyPresetsListWindow ();
		return (FALSE);
	}

	// Create the note-on patchbay window
	if (!CreateNoteOnPatchBayWindow ())
	{
		// Error
		CreateCurrentPresetWindow ();
		DestroyPresetsListWindow ();
		return (FALSE);
	}

	// Create the real-time patchbay window
	if (!CreateRealTimePatchBayWindow ())
	{
		// Error
		DestroyNoteOnPatchBayWindow ();
		DestroyCurrentPresetWindow ();
		DestroyPresetsListWindow ();
		return (FALSE);
	}

	// Update PRESET display with internal information
//	UpdatePresetInternals ();

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		DestroyAllPresetWindows
 Parameters:	None.
 Returns:		None.
 Description:	Destroys all the preset windows.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/5/94 	| Created.											| PW
*****************************************************************************/

VOID DestroyAllPresetWindows (VOID)
{
	DestroyRealTimePatchBayWindow ();
	DestroyNoteOnPatchBayWindow ();
	DestroyCurrentPresetWindow ();
	DestroyPresetsListWindow ();
}

/*^L*/
/*****************************************************************************
 Function:		CreatePresetsListWindow
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Creates the presets list window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/5/94 	| Created.											| PW
*****************************************************************************/

BOOL CreatePresetsListWindow (VOID)
{
	MDICREATESTRUCT	mc;
	WINDOWPLACEMENT	wp;

	// Create the window
	GetCreatePosition (PRESETS_LIST_WINDOW, FALSE, &wp);
	mc.szClass	= WCNAME_PRESETS_LIST;
	mc.szTitle	= ResourceString (IDS_PRS_WINDOW_TITLE);
	mc.hOwner	= hInst;
	mc.x		= wp.rcNormalPosition.left;
	mc.y		= wp.rcNormalPosition.top;
	mc.cx		= wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	mc.cy		= wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	mc.style	= WS_CHILD | WS_CAPTION | WS_CLIPSIBLINGS | WS_MINIMIZEBOX | WS_SYSMENU |
					(wp.showCmd & SW_SHOWMINIMIZED ? WS_MINIMIZE : 0);
	mc.lParam	= 0;
	if ((hwndPresetsList = (HWND)SendMessage (hwndClient,
									WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT)&mc)) == (HWND)NULL)
	{
		// Error
		MyMessageBox (NULL, ResourceString (IDS_PRS_ERR_CREAT_WND), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Reposition the window
	GetWindowPosition (PRESETS_LIST_WINDOW, hwndPresetsList, FALSE);

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		DestroyPresetsListWindow
 Parameters:	None.
 Returns:		None.
 Description:	Destroys the presets list window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/5/94 	| Created.											| PW
*****************************************************************************/

VOID DestroyPresetsListWindow (VOID)
{
	// Save the window position
	SaveWindowPosition (PRESETS_LIST_WINDOW, hwndPresetsList);

	// Destroy the window
	SendMessage (hwndClient, WM_MDIDESTROY, (WPARAM)hwndPresetsList, 0);
	hwndPresetsList = (HWND)NULL;
}

/*^L*/
/*****************************************************************************
 Function:		PresetsListDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if processed, else FALSE.
 Description:	The presets list dialog window procedure.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 23/4/94 	| Created.											| PW
*****************************************************************************/

WINDOW_PROC PresetsListDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
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

			// Put the dialog in the window
			fInitialised = FALSE;
			dwVal = DialogInWindow (hWnd, "DLG_NAMES_LIST");
			cxWin = HIWORD (dwVal);
			cyWin = LOWORD (dwVal);
			fInitialised = TRUE;

			// Set my title
			SetWindowText (hWnd, ResourceString (IDS_PRS_WINDOW_TITLE));

			// Reformat myself
			ReformatDialog (hWnd, RDF_THINFONT);

			// Set the column width
			GetClientRect (GetDlgItem (hWnd, IDC_PRESETS_LIST), &rcClient);
			SendDlgItemMessage (hWnd, IDC_PRESETS_LIST, LB_SETCOLUMNWIDTH, rcClient.right / 2, 0);

			// Display any loaded preset list
			hwndPresetsList = hWnd;
			DisplayPresetsList ();

			// No preset selected
			nDisplayedPreset = INDEX_NONE;

			ShowWindow  (hWnd, SW_HIDE);

			// Clean
			fDirtyPreset = FALSE;

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
						// If the current preset has been changed
						if (fDirtyPreset)
						{
							// Ask the user whether to send the preset back
							if (MyMessageBox (hWnd,
								ResourceString (IDS_MODIFIED_PR_NOT_SAVED),
								ResourceString (IDS_QUESTION_TITLE), MB_ICONQUESTION | MB_YESNO) == IDYES)
							{
								// Save it
								StorePreset (hWnd, nDisplayedPreset);
							}
						}

						// Find out which the currently selected preset is
						nDisplayedPreset = SendDlgItemMessage (hWnd, IDC_PRESETS_LIST, LB_GETCURSEL, 0, 0);

						// If we are connected to the Morpheus
						if (fConnectedToMorpheus)
						{
							// Read the preset
							if (!MorpheusGetPreset (nDisplayedPreset))
							{
								// Error
								nDisplayedPreset = INDEX_NONE;
								break;
							}

							// Select that preset (for later editing)
							MorpheusSelectPreset (hWnd, nDisplayedPreset);
						}

						// Copy the selected preset for editing
						hmemcpy (&EditPreset, pMorpheusPresets[nDisplayedPreset], sizeof (EditPreset));

						// Display the preset
						DisplayPreset ();

						// Clean
						fDirtyPreset = FALSE;
					}
					return (0);
				}

				// Save changes to current preset
				case IDC_PRESET_SAVE_CHANGES:
				{
					// If the current preset has been changed
					if (fDirtyPreset)
					{
						// Save it
						StorePreset (hWnd, nDisplayedPreset);
					}
					return (0);
				}

				// Abandon changes to current preset
				case IDC_PRESET_ABANDON_CHANGES:
				{
					// If the current preset has been changed
					if (fDirtyPreset)
					{
						// Abandon editing
						AbandonPreset (hWnd, nDisplayedPreset);
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
			// No preset selected
			nDisplayedPreset = INDEX_NONE;
			break;
		}

		// Control about to be drawn
		case WM_CTLCOLOR:
		{
			HBRUSH	hBrush;

			if ((hBrush = Ctl3dCtlColorEx (nMsg, wParam, lParam)) != (HBRUSH)FALSE)
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
 Function:		DisplayPresetsList
 Parameters:	None.
 Returns:		None.
 Description:	Redisplays the whole presets list.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 25/4/94 	| Created.											| PW
*****************************************************************************/

VOID DisplayPresetsList (VOID)
{
	UINT	i, num;
	char	szBuf[40];
	int		nSelected, nErr;

	// If there is no presets list OR there is no list window
	if ((pMorpheusPresetList == NULL) || (hwndPresetsList == (HWND)NULL))
	{
		// Exit
		return;
	}

	// Get the current selection
	nSelected = SendDlgItemMessage (hwndPresetsList, IDC_PRESETS_LIST, LB_GETCURSEL, 0, 0);

	// Clear out the current list
	SendDlgItemMessage (hwndPresetsList, IDC_PRESETS_LIST, LB_RESETCONTENT, 0, 0);

	// For each preset in the list
	num = WNUM (pMorpheusPresetList->wNumPresets);
	for (i=0 ; i<num ; i++)
	{
		// Format the preset name
		FormatPresetName (&szBuf[0], i);

		// Add the preset name to the listbox
		nErr = SendDlgItemMessage (hwndPresetsList, IDC_PRESETS_LIST, LB_ADDSTRING,
									0, (LPARAM)(LPCSTR)&szBuf[0]);

		if ((nErr == LB_ERR) || (nErr == LB_ERRSPACE))
		{
			MyMessageBox (hwndMain, ResourceString (IDS_PRS_ERR_DISPLAYING_LIST), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		}
	}

	// Reselect
	SendDlgItemMessage (hwndPresetsList, IDC_PRESETS_LIST, LB_SETCURSEL, nSelected, 0);
}

/*^L*/
/*****************************************************************************
 Function:		StorePreset
 Parameters:	HWND	hwndParent		Parent window.
				int		idPreset		Preset index to save.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Saves the specified preset.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/5/94 	| Created.											| PW
*****************************************************************************/

static BOOL StorePreset (HWND hwndParent, int idPreset)
{
	int		nCurSel;

	// Copy the editable preset back to the main mempry copy
	hmemcpy (pMorpheusPresets[idPreset], &EditPreset, sizeof (EditPreset));

	// If we are connected to the Morpheus
	if (fConnectedToMorpheus)
	{
		// Send it back to the Morpheus
		if (!MorpheusSendPreset (hwndParent, idPreset))
		{
			MyMessageBox (hwndParent, ResourceString (IDS_PR_ERR_SENDING), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			return (FALSE);
		}
	}

	// Update the list of preset names from the loaded preset.
	CreatePresetNames (idPreset);

	// Display the updated list and re-select the current item
	nCurSel = SendDlgItemMessage (hwndPresetsList, IDC_PRESETS_LIST, LB_GETCURSEL, 0, 0);
	DisplayPresetsList ();
	SendDlgItemMessage (hwndPresetsList, IDC_PRESETS_LIST, LB_SETCURSEL, nCurSel, 0);

	// OK
	fDirtyPreset = FALSE;
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		AbandonPreset
 Parameters:	HWND	hwndParent		Parent window.
				int		idPreset		Preset index to save.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Abandons the current preset editing.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/5/94 	| Created.											| PW
*****************************************************************************/

static BOOL AbandonPreset (HWND hwndParent, int idPreset)
{
	// If a preset is on display
	if (idPreset != INDEX_NONE)
	{
		//  If the preset is loaded
		if (pMorpheusPresets[idPreset] != NULL)
		{
			// Take an edit copy again
			hmemcpy (&EditPreset, pMorpheusPresets[idPreset], sizeof (EditPreset));
		}

		// If we are connected to the Morpheus
		if (fConnectedToMorpheus)
		{
			// Select that preset (for later editing)
			MorpheusSelectPreset (hwndParent, idPreset);
		}

		// Display the preset
		DisplayPreset ();
	}

	// OK
	fDirtyPreset = FALSE;
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		FormatPresetName
 Parameters:	LPSTR	lpszBuf
				int		idPreset
 Returns:		None.
 Description:	Formats the specified preset name.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 27/4/94 	| Created.											| PW
*****************************************************************************/

static VOID FormatPresetName (LPSTR lpszBuf, int idPreset)
{
	wsprintf (&lpszBuf[0], "%u: %s", idPreset, (LPCSTR)&pMorpheusPresetList->Info[idPreset].szName[0]);
}

