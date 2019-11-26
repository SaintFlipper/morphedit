/*****************************************************************************
 File:			hyper1.c
 Description:	Functions for the main HYPERPRESET EDITOR dialog. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 30/5/94	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define	HANDLE_BUTTON(id,var)														\
		{																			\
			int	nNewVal = SendDlgItemMessage (hWnd, id, BM_GETCHECK, 0, 0);			\
			if (nNewVal != ReadSysexWord (&var))									\
            {																		\
				fDirtyHyper = TRUE;													\
				WriteSysexWord (&var, nNewVal);										\
				HyperSendParam (hWnd, &var);										\
			}																		\
		}

#define	HANDLE_COMBOBOX(id,var,newval)												\
		{																			\
			if (newval != ReadSysexWord (&var))										\
            {																		\
				fDirtyHyper = TRUE;													\
				WriteSysexWord (&var, newval);										\
				HyperSendParam (hWnd, &var);										\
			}																		\
		}

#define	HANDLE_AUCNTRL(id,var)						\
		if (ReadSysexWord (&var) != (int)lParam)	\
		{											\
			fDirtyHyper = TRUE;						\
			WriteSysexWord (&var, (int)lParam);		\
			HyperSendParam (hWnd, &var);			\
		}


/****************************************************************************/
/*								Local types									*/
/****************************************************************************/

/****************************************************************************/
/*								Global data									*/
/****************************************************************************/

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

static LISTBOX_TAB_INFO	ZoneListboxTabs[] =
{
	{IDC_HYPER_ZONE_LB_LABEL1,	0},
	{IDC_HYPER_ZONE_LB_LABEL2,	20},
	{IDC_HYPER_ZONE_LB_LABEL3,	90},
	{IDC_HYPER_ZONE_LB_LABEL4,	110},
	{IDC_HYPER_ZONE_LB_LABEL5,	130},
	{IDC_HYPER_ZONE_LB_LABEL6,	170},
	{IDC_HYPER_ZONE_LB_LABEL7,	210},
	{IDC_HYPER_ZONE_LB_LABEL8,	240},
	{IDC_HYPER_ZONE_LB_LABEL9,	280},
	{IDC_HYPER_ZONE_LB_LABEL10,	320},
	{IDC_HYPER_ZONE_LB_LABEL11,	350},
	{IDC_HYPER_ZONE_LB_LABEL12,	380}
};
static LISTBOX_TAB_INFO	FreeRunFGListboxTabs[] =
{
	{IDC_HYPER_FG_LB_LABEL1,	0},
	{IDC_HYPER_FG_LB_LABEL2,	30},
	{IDC_HYPER_FG_LB_LABEL3,	60},
	{IDC_HYPER_FG_LB_LABEL4,	90},
	{IDC_HYPER_FG_LB_LABEL5,	130},
	{IDC_HYPER_FG_LB_LABEL6,	190},
	{IDC_HYPER_FG_LB_LABEL7,	220},
};
static LPCSTR	pszPortModes[] = {"Mono", "Poly 1 key", "Poly 2 key", "Poly 3 key", "Poly 4 key", "Poly 5 key"};

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

DIALOG_PROC ZoneDefDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
DIALOG_PROC FreeRunFuncGenDefDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

static VOID FillZoneListBox (HWND hwndListBox);
static VOID HyperSendParam (HWND hwndParent, LPVOID lpVar);

/*^L*/
/*****************************************************************************
 Function:		DisplayHyper
 Parameters:	int		idHyper			Hyperpreset to display.
 Returns:		None.
 Description:	Displays the specified hyperpreset.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 31/5/94	| Created.											| PW
*****************************************************************************/

VOID DisplayHyper (int idHyper)
{
	int		i;
	char	szName[20], szTitleBuf[100];

	// Do nothing if the editing window doesn't exist
	if (hwndCurrentHyper == (HWND)NULL)
	{
    	return;
	}

	// Do nothing if the hyperpreset if not loaded
	if ((nDisplayedHyper == INDEX_NONE) || (pMorpheusHyperPresets[idHyper] == NULL))
	{
		return;
	}

	// Display its name
	for (i=0 ; i<ARRAY_SIZE(CurrentHyper.Name) ; i++)
	{
		szName[i] = WNUM (CurrentHyper.Name[i]);
	}
	szName[i] = '\0';
	SendDlgItemMessage (hwndCurrentHyper, IDC_HYPER_NAME, WM_SETTEXT, 0, (LPARAM)(LPCSTR)&szName[0]);

	// Select its portamento mode
	SendDlgItemMessage (hwndCurrentHyper, IDC_HYPER_PORT_MODE, CB_SETCURSEL, WNUM (CurrentHyper.PortMode), 0);

	// Fill the free-running FG listbox
	DisplayFuncGenList (GetDlgItem (hwndCurrentHyper, IDC_HYPER_FG_LISTBOX), &CurrentHyper.FreeRunFG);

	// Fill the zone FG listbox
	FillZoneListBox (GetDlgItem (hwndCurrentHyper, IDC_HYPER_ZONE_LISTBOX));

	// Set the editing window title
	wsprintf (&szTitleBuf[0], ResourceString (IDS_EDIT_WINDOW_TITLE_MASK), (LPSTR)&szName[0]);
	SetWindowText (hwndCurrentHyper, &szTitleBuf[0]);
}

/*^L*/
/*****************************************************************************
 Function:		CreateCurrentHyperWindow
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Creates the hyperpreset edit window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 31/5/94 	| Created.											| PW
*****************************************************************************/

BOOL CreateCurrentHyperWindow (VOID)
{
	MDICREATESTRUCT	mc;
	WINDOWPLACEMENT	wp;

	// Create the window
	GetCreatePosition (CURRENT_HYPERPRESET_WINDOW, FALSE, &wp);
	mc.szClass	= WCNAME_CURRENT_HYPER;
	mc.szTitle	= "Current hyperpreset";
	mc.hOwner	= hInst;
	mc.x		= wp.rcNormalPosition.left;
	mc.y		= wp.rcNormalPosition.top;
	mc.cx		= wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	mc.cy		= wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	mc.style	= WS_CHILD | WS_CAPTION | WS_CLIPSIBLINGS | WS_MINIMIZEBOX | WS_SYSMENU |
					(wp.showCmd & SW_SHOWMINIMIZED ? WS_MINIMIZE : 0);
	mc.lParam	= 0;
	if ((hwndCurrentHyper = (HWND)SendMessage (hwndClient,
									WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT)&mc)) == (HWND)NULL)
	{
		// Error
		MyMessageBox (NULL, "Error creating current hyperpreset window", "Error", MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Reposition the window
	GetWindowPosition (CURRENT_HYPERPRESET_WINDOW, hwndCurrentHyper, FALSE);

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		DestroyCurrentHyperWindow
 Parameters:	None.
 Returns:		None.
 Description:	Destroys the hyperpreset editing window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 31/5/94 	| Created.											| PW
*****************************************************************************/

VOID DestroyCurrentHyperWindow (VOID)
{
	// Save the window position
	SaveWindowPosition (CURRENT_HYPERPRESET_WINDOW, hwndCurrentHyper);

	// Destroy the window
	SendMessage (hwndClient, WM_MDIDESTROY, (WPARAM)hwndCurrentHyper, 0);
	hwndCurrentHyper = (HWND)NULL;
}

/*^L*/
/*****************************************************************************
 Function:		CurrentHyperDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if processed, else FALSE.
 Description:	The current hyperpreset editor dialog.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 31/5/94 	| Created.											| PW
*****************************************************************************/

WINDOW_PROC CurrentHyperDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	int			i;
	int			id;
	char		szName[200];
	DWORD		dwVal;
	static int	nDialogWidth, nDialogHeight;
	static BOOL	fInitialised;

	switch (nMsg)
	{
		// Initialisation
		case WM_CREATE:
		{
			// Put the dialog int the window
			fInitialised = FALSE;
			dwVal = DialogInWindow (hWnd, "DLG_HYPERPRESET");
			nDialogWidth	= HIWORD (dwVal);
			nDialogHeight	= LOWORD (dwVal);
			fInitialised = TRUE;

			// Reformat myself
			ReformatDialog (hWnd, RDF_THINFONT);

			// Set the FG listbox tab stops
			SetListBoxTabs (hWnd, IDC_HYPER_FG_LISTBOX, &FreeRunFGListboxTabs[0], ARRAY_SIZE (FreeRunFGListboxTabs));

			// Set the ZONE listbox tab stops
			SetListBoxTabs (hWnd, IDC_HYPER_ZONE_LISTBOX, &ZoneListboxTabs[0], ARRAY_SIZE (ZoneListboxTabs));

			// Initialise my controls
			for (i=0 ; i<ARRAY_SIZE(pszPortModes) ; i++)
			{
				SendDlgItemMessage (hWnd, IDC_HYPER_PORT_MODE, CB_ADDSTRING, 0, (LPARAM)pszPortModes[i]);
			}
			SendDlgItemMessage (hWnd, IDC_HYPER_PORT_MODE, CB_SETCURSEL, 0, 0);

			// Show the window
			// ShowWindow (hWnd, SW_SHOW);

			return (0);
		}

		// Trying to change the dialog width
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO FAR	*lpmm = (MINMAXINFO FAR *)lParam;

			if (fInitialised)
			{
				lpmm->ptMinTrackSize.x	= nDialogWidth;
				lpmm->ptMinTrackSize.y	= nDialogHeight;
				lpmm->ptMaxTrackSize.x	= nDialogWidth;
				lpmm->ptMaxTrackSize.y	= nDialogHeight;
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

		// Buttons, combo boxes, listboxes, edit controls
		case WM_COMMAND:
		{
			// Do nothing if no hyperpreset has been selected
			if (nDisplayedHyper == INDEX_NONE)
			{
				return (TRUE);
			}

			// If it's a new COMBOBOX selection
			if (NCODE == CBN_SELCHANGE)
			{
				// Invoke the handling macros
				switch (id = LOWORD (wParam))
				{
					case IDC_HYPER_PORT_MODE:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, CurrentHyper.PortMode, i);
						break;

					default:
						break;
            	}
			}
			// Else if it's an edit box change
			else if (NCODE == EN_CHANGE)
			{
				// Invoke the handling macros
				switch (id = LOWORD (wParam))
				{
					case IDC_HYPER_NAME:
						// Fetch the edited name back
						SendDlgItemMessage (hWnd, id, WM_GETTEXT, sizeof (szName), (LPARAM)(LPSTR)&szName[0]);

						// Make sure the name is exactly the right length
						while (strlen (&szName[0]) < ARRAY_SIZE (CurrentHyper.Name))
						{
							strcat (&szName[0], " ");
                        }

						// For each character in the stored name
						for (i=0 ; i<ARRAY_SIZE(CurrentHyper.Name) ; i++)
						{
							// If the edited character is different from the stored one
							if (szName[i] != (char)ReadSysexWord (&CurrentHyper.Name[i]))
							{
								// I feel dirty
								fDirtyHyper = TRUE;

								// Store the edited character
								WriteSysexWord (&CurrentHyper.Name[i], (int)szName[i]);

								// Send the character
								HyperSendParam (hWnd, &CurrentHyper.Name[i]);
							}
						}

						return (0);
						
					default:
					{
						break;
					}
				}
			}
			// Else if it's a new list box selection
			else if (NCODE == LBN_DBLCLK)
			{
				// Invoke the handling macros
				switch (id = LOWORD (wParam))
				{
					case IDC_HYPER_ZONE_LISTBOX:
					{
						DLGPROC	lpfnDialogProc;
						int		nIndex;

						// Get the zone number
						nIndex	= SendDlgItemMessage (hWnd, id, LB_GETCURSEL, 0, 0);

						// Call the editing dialog
						lpfnDialogProc = (DLGPROC)MakeProcInstance (ZoneDefDialogProc, hInst);
						DialogBoxParam (hInst, "DLG_ZONE", hWnd, lpfnDialogProc, (LPARAM)nIndex);
						FreeProcInstance ((FARPROC)lpfnDialogProc);

						break;
					}
					case IDC_HYPER_FG_LISTBOX:
					{
					
						DLGPROC	lpfnDialogProc;
						int		nIndex;

						// Get the FG step number
						nIndex = SendDlgItemMessage (hWnd, id, LB_GETCURSEL, 0, 0);

						// Call the editing dialog
						lpfnDialogProc = (DLGPROC)MakeProcInstance (FreeRunFuncGenDefDialogProc, hInst);
						DialogBoxParam (hInst, "DLG_FG_EDIT", hWnd, lpfnDialogProc, (LPARAM)nIndex);
						FreeProcInstance ((FARPROC)lpfnDialogProc);

						break;
                    }

					default:
						break;
				}
			}


			return (TRUE);
		}

		// Close
		case WM_CLOSE:
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
 Function:		ZoneDefDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
                LPARAM	lParam
 Returns:		BOOL					TRUE if message processed.
 Description:	The hyperpreset zone editing dialog.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 31/5/94 	| Created.											| PW
*****************************************************************************/

DIALOG_PROC ZoneDefDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	int				i, id;
	static int		nIndex;
	char			szListEntry[100];
	static BOOL		fInitialised;
	static int		nPreset, nLowKey, nHighKey, nLowVel, nHighVel, nVelOffset, nVolume, nPan, nXpose, nCtune, nFtune;
	int				NumPresets;

	switch (nMsg)
	{
		// Initialisation
		case WM_INITDIALOG:
		{
			// Uninitialised
			fInitialised = FALSE;

			// Reformat myself
			ReformatDialog (hWnd, RDF_CENTRE | RDF_THINFONT);

			// Extract the zone number
			nIndex	= (int)lParam;

			// Fetch the current parameters
			nPreset		= WNUM (CurrentHyper.Zone[nIndex].Preset);
			nLowKey		= WNUM (CurrentHyper.Zone[nIndex].LowKey);
			nHighKey	= WNUM (CurrentHyper.Zone[nIndex].HighKey);
			nLowVel		= WNUM (CurrentHyper.Zone[nIndex].LowVel);
			nHighVel	= WNUM (CurrentHyper.Zone[nIndex].HighVel);
			nVelOffset	= WNUM (CurrentHyper.Zone[nIndex].VelOffset);
			nVolume		= WNUM (CurrentHyper.Zone[nIndex].Volume);
			nPan		= WNUM (CurrentHyper.Zone[nIndex].Pan);
			nXpose		= WNUM (CurrentHyper.Zone[nIndex].Xpose);
			nCtune		= WNUM (CurrentHyper.Zone[nIndex].CoarseTune);
			nFtune		= WNUM (CurrentHyper.Zone[nIndex].FineTune);

			// Set the dialog's title
			wsprintf (&szListEntry[0], "Zone %u", nIndex + 1);
			SetWindowText (hWnd, (LPCSTR)&szListEntry[0]);

			// Fill in the PRESET list (insert "NONE" before the real presets)
			NumPresets = WNUM (pMorpheusPresetList->wNumPresets);
			SendDlgItemMessage (hWnd, IDC_ZONE_PRESET, CB_ADDSTRING, 0, (LPARAM)ResourceString (IDS_NO_PRESET_NAME));
			for (i=0 ; i<NumPresets ; i++)
			{
				wsprintf (&szListEntry[0], "%u: %s", i, &pMorpheusPresetList->Info[i].szName[0]);
				SendDlgItemMessage (hWnd, IDC_ZONE_PRESET, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)&szListEntry[0]);
			}
			// Select the current preset
			SendDlgItemMessage (hWnd, IDC_ZONE_PRESET, CB_SETCURSEL, nPreset + 1, 0);

			// Initialise the low-key control
			InitUpDown (hWnd, IDC_ZONE_LOW_KEY, 0, 127, nLowKey,
						GetDlgItem (hWnd, IDC_ZONE_LOW_KEY_EDITBOX), lpfnDisplayNote);

			// Initialise the high-key control
			InitUpDown (hWnd, IDC_ZONE_HIGH_KEY, 0, 127, nHighKey,
						GetDlgItem (hWnd, IDC_ZONE_HIGH_KEY_EDITBOX), lpfnDisplayNote);


			// Initialise the low-velocity control
			InitSlider (hWnd, IDC_ZONE_LOW_VELOCITY, 0, 127, nLowVel);

			// Initialise the high-velocity control
			InitSlider (hWnd, IDC_ZONE_HIGH_VELOCITY, 0, 127, nHighVel);

			// Initialise the velocity-offset control
			InitSlider (hWnd, IDC_ZONE_VELOCITY_OFFSET, -126, 126, nVelOffset);

			// Initialise the volume control
			InitSlider (hWnd, IDC_ZONE_VOLUME, 0, 127, nVolume);

			// Initialise the pan control
			InitSlider (hWnd, IDC_ZONE_PAN, -14, 14, nPan);

			// Initialise the transpose control
			InitSlider (hWnd, IDC_ZONE_XPOSE, -36, 36, nXpose);

			// Initialise the coarse tune control
			InitSlider (hWnd, IDC_ZONE_COARSE_TUNE, -36, 36, nCtune);

			// Initialise the fine tune control
			InitSlider (hWnd, IDC_ZONE_FINE_TUNE, -36, 36, nFtune);

			// Initialised
			fInitialised = TRUE;

			return (TRUE);
		}

		// Slider / up-down
		case SLN_NEW_VALUE:
		case UDN_NEW_VALUE:
		{
			// Don't process until fully initialised
			if (!fInitialised)
			{
            	return (TRUE);
			}

			// Handle by control ID
			switch (id = LOWORD (wParam))
			{
            	// Sliders
				case IDC_ZONE_VOLUME:
					HANDLE_AUCNTRL (id, CurrentHyper.Zone[nIndex].Volume);
					break;

				case IDC_ZONE_PAN:
					HANDLE_AUCNTRL (id, CurrentHyper.Zone[nIndex].Pan);
					break;

				case IDC_ZONE_XPOSE:
					HANDLE_AUCNTRL (id, CurrentHyper.Zone[nIndex].Xpose);
					break;

				case IDC_ZONE_COARSE_TUNE:
					HANDLE_AUCNTRL (id, CurrentHyper.Zone[nIndex].CoarseTune);
					break;

				case IDC_ZONE_FINE_TUNE:
					HANDLE_AUCNTRL (id, CurrentHyper.Zone[nIndex].FineTune);
					break;

                // Up-downs
				case IDC_ZONE_LOW_KEY:
					HANDLE_AUCNTRL (id, CurrentHyper.Zone[nIndex].LowKey);
					break;

				case IDC_ZONE_HIGH_KEY:
					HANDLE_AUCNTRL (id, CurrentHyper.Zone[nIndex].HighKey);
					break;

				case IDC_ZONE_LOW_VELOCITY:
					HANDLE_AUCNTRL (id, CurrentHyper.Zone[nIndex].LowVel);
					break;

				case IDC_ZONE_HIGH_VELOCITY:
					HANDLE_AUCNTRL (id, CurrentHyper.Zone[nIndex].HighVel);
					break;

				case IDC_ZONE_VELOCITY_OFFSET:
					HANDLE_AUCNTRL (id, CurrentHyper.Zone[nIndex].VelOffset);
					break;

			}
			return (TRUE);
		}

		// Control
		case WM_COMMAND:
		{
			// If it's a new COMBOBOX selection
			if (NCODE == CBN_SELCHANGE)
			{
				// Invoke the handling macros
				switch (id = LOWORD (wParam))
				{
					// Preset
					case IDC_ZONE_PRESET:
					{
						// Read back the selected preset index and subtract 1, because the 1st item is "(None)"
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0) - 1;
						HANDLE_COMBOBOX (id, CurrentHyper.Zone[nIndex].Preset, i);
						break;
					}
				}

				return (TRUE);
			}
            // Else if it's a button
			else if (NCODE == BN_CLICKED)
			{
				// Only think about OK
				if (LOWORD (wParam) == IDOK)
				{
					// Update the listbox
					FillZoneListBox (GetDlgItem (hwndCurrentHyper, IDC_HYPER_ZONE_LISTBOX));

					// Re-select the current item
					SendDlgItemMessage (hwndCurrentHyper, IDC_HYPER_ZONE_LISTBOX, LB_SETCURSEL, nIndex, 0);

					// Bye bye
					EndDialog (hWnd, 0);
				}
            }
			return (TRUE);
		}

		// Control about to be drawn
		case WM_CTLCOLOR:
		{
			Ctl3dCtlColorEx (nMsg, wParam, lParam);
			break;
		}

		// Default
		default:
		{
			break;
        }
	}

	// Not processed
	return (FALSE);
}

/*^L*/
/*****************************************************************************
 Function:		FreeRunFuncGenDefDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
                LPARAM	lParam
 Returns:		BOOL					TRUE if message processed.
 Description:	The function generator editing dialog.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 14/5/94 	| Created.											| PW
*****************************************************************************/

DIALOG_PROC FreeRunFuncGenDefDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	int							i, id;
	static int					nStep, nLevel, nLevelType, nTime, nShape, nCondJump, nCondVal, nCondSeg;
	static BOOL					fInitialised;
	char						szListEntry[100];

	switch (nMsg)
	{
		// Initialisation
		case WM_INITDIALOG:
		{
			// Uninitialised
			fInitialised = FALSE;

			// Reformat myself
			ReformatDialog (hWnd, RDF_CENTRE | RDF_THINFONT);

			// Extract the edit paramaters
			nStep		= (int)lParam;
			nLevel		= WNUM (CurrentHyper.FreeRunFG.Segment[nStep].Level);
			nTime		= WNUM (CurrentHyper.FreeRunFG.Segment[nStep].Time);
			nShape		= WNUM (CurrentHyper.FreeRunFG.Segment[nStep].Shape);
			nCondJump	= WNUM (CurrentHyper.FreeRunFG.Segment[nStep].CondJump);
			nCondVal	= WNUM (CurrentHyper.FreeRunFG.Segment[nStep].CondValue);
			nCondSeg	= WNUM (CurrentHyper.FreeRunFG.Segment[nStep].DestSegment);

			// Set the dialog's title
			wsprintf (&szListEntry[0], ResourceString (IDS_FREE_RUN_FG_STEP_TITLE), nStep + 1);
			SetWindowText (hWnd, (LPCSTR)&szListEntry[0]);

			// Decode the segment level
			nLevelType = (nLevel >> 8) & 3;
			nLevel = (int)(signed char)nLevel;

			// Initialise the LEVEL control
			InitSlider (hWnd, IDC_FG_LEVEL, -127, 127, nLevel);

			// Initialise the level type radio buttons
			switch (nLevelType)
			{
				case 0:
					CheckDlgButton (hWnd, IDC_FG_LEVEL_TYPE_0, 1);
                    break;
				case 1:
					CheckDlgButton (hWnd, IDC_FG_LEVEL_TYPE_1, 1);
                    break;
				case 2:
					CheckDlgButton (hWnd, IDC_FG_LEVEL_TYPE_2, 1);
                    break;
				case 3:
					CheckDlgButton (hWnd, IDC_FG_LEVEL_TYPE_3, 1);
                    break;
			}

			// Initialise the TIME control
			InitSlider (hWnd, IDC_FG_TIME, 0, 4095, nTime);

			// Fill in the SEGMENT SHAPE list and select the current one
			for (i=0 ; i<NUM_FG_SEGMENT_SHAPES ; i++)
			{
				SendDlgItemMessage (hWnd, IDC_FG_SHAPE, CB_ADDSTRING, 0, (LPARAM)FGShapeName (i));
			}
			SendDlgItemMessage (hWnd, IDC_FG_SHAPE, CB_SETCURSEL, nShape, 0);

			// Fill in the CONDITIONAL JUMP list and select the current one
			for (i=0 ; i<NUM_FG_JUMP_CONDITIONS ; i++)
			{
				SendDlgItemMessage (hWnd, IDC_FG_COND_JUMP, CB_ADDSTRING, 0, (LPARAM)FGConditionName (i));
			}
			SendDlgItemMessage (hWnd, IDC_FG_COND_JUMP, CB_SETCURSEL, nCondJump, 0);

			// Initialise the JUMP DESTINATION SEGMENT control
			InitUpDown (hWnd, IDC_FG_COND_SEG, 1, 8, nCondSeg + 1,
						GetDlgItem (hWnd, IDC_FG_COND_SEG_VALUE), lpfnDisplayNumber);

			// Initialise the CONDITIONAL VALUE control
			InitSlider (hWnd, IDC_FG_COND_VAL, -127, 127, nCondVal);

			// Initialised
			fInitialised = TRUE;

			return (TRUE);
		}

		// Slider / up-down control
		case SLN_NEW_VALUE:
		case UDN_NEW_VALUE:
		{
			// Don't process until fully initialised
			if (!fInitialised)
			{
            	return (TRUE);
			}

			// Handle according to which control
			switch (id = LOWORD (wParam))
			{
				// Level
				case IDC_FG_LEVEL:
				{
					// Store it
					nLevel = (int)lParam;
					i = (nLevelType << 8) + (nLevel & 0xFF);
					WriteSysexWord (&CurrentHyper.FreeRunFG.Segment[nStep].Level, i);
					HyperSendParam (hWnd, &CurrentHyper.FreeRunFG.Segment[nStep].Level);

					break;
				}
				// Time
				case IDC_FG_TIME:
				{
					HANDLE_AUCNTRL (id, CurrentHyper.FreeRunFG.Segment[nStep].Time);
					break;
                }
				// Conditional value
				case IDC_FG_COND_VAL:
				{
					HANDLE_AUCNTRL (id, CurrentHyper.FreeRunFG.Segment[nStep].CondValue);
					break;
				}
				// Destination segment
				case IDC_FG_COND_SEG:
				{
					lParam--;			// Because the control thinks it's in the range 1..8
					HANDLE_AUCNTRL (id, CurrentHyper.FreeRunFG.Segment[nStep].DestSegment);
					break;
				}
			}
			return (TRUE);
		}

		// Control
		case WM_COMMAND:
		{
			// Don't process until fully initialised
			if (!fInitialised)
			{
            	return (TRUE);
			}

			// If it's a new COMBOBOX selection
			if (NCODE == CBN_SELCHANGE)
			{
				// Invoke the handling macros
				switch (id = LOWORD (wParam))
				{
					// Segment shape
					case IDC_FG_SHAPE:
					{
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, CurrentHyper.FreeRunFG.Segment[nStep].Shape, i);
                        break;
					}

					// Jump condition
					case IDC_FG_COND_JUMP:
					{
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, CurrentHyper.FreeRunFG.Segment[nStep].CondJump, i);
                        break;
					}

					default:
					{
						break;
					}
				}
			}
            // If it's a button
			else if (NCODE == BN_CLICKED)
			{
				// Handle according to just which button it is
				switch (LOWORD (wParam))
				{
					// Finished
					case IDOK:
					{
						// Update the free-running listbox
						DisplayFuncGenList (GetDlgItem (hwndCurrentHyper, IDC_HYPER_FG_LISTBOX), &CurrentHyper.FreeRunFG);

						// Re-select the current item
						SendDlgItemMessage (hwndCurrentHyper, IDC_HYPER_FG_LISTBOX, LB_SETCURSEL, nStep, 0);

						// Bye bye
						EndDialog (hWnd, 0);
						break;
					}

					// Absolute level
					case IDC_FG_LEVEL_TYPE_0:
					{
						if (IsDlgButtonChecked (hWnd, IDC_FG_LEVEL_TYPE_0))
						{
							if (nLevelType != 0)
                            {
								nLevelType = 0;
								i = (nLevelType << 8) + (nLevel & 0xFF);;
								WriteSysexWord (&CurrentHyper.FreeRunFG.Segment[nStep].Level, i);
								HyperSendParam (hWnd, &CurrentHyper.FreeRunFG.Segment[nStep].Level);
							}
                        }
						break;
					}

					// Delta level
					case IDC_FG_LEVEL_TYPE_1:
					{
						if (IsDlgButtonChecked (hWnd, IDC_FG_LEVEL_TYPE_1))
						{
							if (nLevelType != 1)
                            {
								nLevelType = 1;
								i = (nLevelType << 8) + (nLevel & 0xFF);;
								WriteSysexWord (&CurrentHyper.FreeRunFG.Segment[nStep].Level, i);
								HyperSendParam (hWnd, &CurrentHyper.FreeRunFG.Segment[nStep].Level);
                            }
						}
						break;
					}

					// Random level
					case IDC_FG_LEVEL_TYPE_2:
					{
						if (IsDlgButtonChecked (hWnd, IDC_FG_LEVEL_TYPE_2))
						{
							if (nLevelType != 2)
                            {
								nLevelType = 2;
								i = (nLevelType << 8) + (nLevel & 0xFF);;
								WriteSysexWord (&CurrentHyper.FreeRunFG.Segment[nStep].Level, i);
								HyperSendParam (hWnd, &CurrentHyper.FreeRunFG.Segment[nStep].Level);
                            }
						}
						break;
					}

					// Random delta level
					case IDC_FG_LEVEL_TYPE_3:
					{
						if (IsDlgButtonChecked (hWnd, IDC_FG_LEVEL_TYPE_3))
						{
							if (nLevelType != 3)
                            {
								nLevelType = 3;
								i = (nLevelType << 8) + (nLevel & 0xFF);;
								WriteSysexWord (&CurrentHyper.FreeRunFG.Segment[nStep].Level, i);
								HyperSendParam (hWnd, &CurrentHyper.FreeRunFG.Segment[nStep].Level);
                            }
						}
						break;
					}

					default:
					{
                    	break;
					}
				}
			}
			return (TRUE);
		}

		// Control about to be drawn
		case WM_CTLCOLOR:
		{
			Ctl3dCtlColorEx (nMsg, wParam, lParam);
			break;
		}

		// Default
		default:
		{
			break;
        }
	}

	// Not processed
	return (FALSE);
}

/*^L*/
/*****************************************************************************
 Function:		FillZoneListBox
 Parameters:	HWND	hwndListBox		Zone listbox handle.
 Returns:		None.
 Description:	Fills the zone listbox specified with information from the
				current hyperpreset.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 31/5/94 	| Created.											| PW
*****************************************************************************/

static VOID FillZoneListBox (HWND hwndListBox)
{
	UINT			i, nLen;
	char			szLineBuf[200];
	int				nPreset;

	// Clear the list
	SendMessage (hwndListBox, LB_RESETCONTENT, 0, 0);

	// For each ZONE (of 16)
	for (i=0 ; i<16 ; i++)
	{
		// Format the line ...

        // The ZONE number (1..16)
		itoa (i + 1, &szLineBuf[0], 10);

		// The PRESET NAME				 
		lstrcat (&szLineBuf[0], "\t");
		if ((nPreset = WNUM (CurrentHyper.Zone[i].Preset)) == -1)
		{
			lstrcat (&szLineBuf[0], ResourceString (IDS_NO_PRESET_NAME));
		}
		else
		{
			lstrcat (&szLineBuf[0], &pMorpheusPresetList->Info[nPreset].szName[0]);
		}

		// The VOLUME
		nLen = lstrlen (&szLineBuf[0]);
		wsprintf (&szLineBuf[nLen], "\t%d", WNUM (CurrentHyper.Zone[i].Volume));

		// The PAN
		nLen = lstrlen (&szLineBuf[0]);
		wsprintf (&szLineBuf[nLen], "\t%d", WNUM (CurrentHyper.Zone[i].Pan));

		// Low key
		lstrcat (&szLineBuf[0], "\t");
		nLen = lstrlen (&szLineBuf[0]);
		FormatNote (&szLineBuf[nLen], WNUM (CurrentHyper.Zone[i].LowKey));

		// High key
		lstrcat (&szLineBuf[0], "\t");
		nLen = lstrlen (&szLineBuf[0]);
		FormatNote (&szLineBuf[nLen], WNUM (CurrentHyper.Zone[i].HighKey));

		// Low velocity
		nLen = lstrlen (&szLineBuf[0]);
		wsprintf (&szLineBuf[nLen], "\t%d", WNUM (CurrentHyper.Zone[i].LowVel));

		// High velocity
		nLen = lstrlen (&szLineBuf[0]);
		wsprintf (&szLineBuf[nLen], "\t%d", WNUM (CurrentHyper.Zone[i].HighVel));

		// Velocity offset
		nLen = lstrlen (&szLineBuf[0]);
		wsprintf (&szLineBuf[nLen], "\t%d", WNUM (CurrentHyper.Zone[i].VelOffset));

		// Transpose
		nLen = lstrlen (&szLineBuf[0]);
		wsprintf (&szLineBuf[nLen], "\t%d", WNUM (CurrentHyper.Zone[i].Xpose));

		// Coarse tune
		nLen = lstrlen (&szLineBuf[0]);
		wsprintf (&szLineBuf[nLen], "\t%d", WNUM (CurrentHyper.Zone[i].CoarseTune));

		// Fine tune
		nLen = lstrlen (&szLineBuf[0]);
		wsprintf (&szLineBuf[nLen], "\t%d", WNUM (CurrentHyper.Zone[i].FineTune));

		// Put it in the listbox
		SendMessage (hwndListBox, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)&szLineBuf[0]);
	}
}

/*^L*/
/*****************************************************************************
 Function:		FormatNote
 Parameters:	LPSTR	lpBuf
				int		Note
 Returns:		None.
 Description:	Formats a MIDI note as ASCII.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 31/5/94 	| Created.											| PW
*****************************************************************************/

VOID FormatNote (LPSTR lpBuf, int Note)
{
	char	szTotal[20];
	int		nOctave;
	static LPCSTR szNoteNames[] =		// Notes in the octave
	{
		"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
	};

	nOctave = (Note / 12) - 2;
	lstrcpy (&szTotal[0], szNoteNames[Note % 12]);
	itoa (nOctave, &szTotal[lstrlen (&szTotal[0])], 10);
	lstrcpy (lpBuf, &szTotal[0]);
}

/*^L*/
/*****************************************************************************
 Function:		HyperSendParam
 Parameters:	HWND	hwndParent		Parent window.
				LPVOID	lpVar			Address of variable within current
										hyperpreset structure.
 Returns:		None.
 Description:	Sends a single hyperpreset parameter to the Morpheus.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 31/5/94 	| Created.											| PW
*****************************************************************************/

static VOID HyperSendParam (HWND hwndParent, LPVOID lpVar)
{
	LPWORD			lpwVar = (LPWORD)lpVar;
	WORD			wParamNumber;

	// Do nothing if not connected to the Morpheus
	if (!fConnectedToMorpheus)
	{
		return;
	}

	/*
		The parameter number should simply be the WORD index of the parameter offset from the
		start of the hyperpreset (param #0 is &pPreset->Name[0]).
	 */
	wParamNumber = MPARAM_HYPERPRESET_BASE + (lpwVar - &CurrentHyper.Name[0]);

	// Set its value
	MorpheusSetParameter (hwndParent, (int)wParamNumber, ReadSysexWord (lpwVar));
}

