/*****************************************************************************
 File:			midimap1.c
 Description:	Functions for the main MIDIMAP EDITOR dialog. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define	HANDLE_BUTTON(id,var)														\
		{																			\
			int	nNewVal = IsDlgButtonChecked (hWnd, id) ? 1 : 0;					\
			if (nNewVal != ReadSysexWord (&var))									\
            {																		\
				fDirtyMM = TRUE;													\
				WriteSysexWord (&var, nNewVal);										\
				MMSendParam (hWnd, &var);											\
			}																		\
		}

#define	HANDLE_COMBOBOX(id,var,newval)												\
		{																			\
			if (newval != ReadSysexWord (&var))										\
            {																		\
				fDirtyMM = TRUE;													\
				WriteSysexWord (&var, newval);										\
				MMSendParam (hWnd, &var);											\
			}																		\
		}

#define	HANDLE_AUCNTRL(id,var)						\
		if (ReadSysexWord (&var) != (int)lParam)	\
		{											\
			fDirtyMM = TRUE;						\
			WriteSysexWord (&var, (int)lParam);		\
			MMSendParam (hWnd, &var);				\
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

static LISTBOX_TAB_INFO	MMChannelListboxTabs[] =
{
	{IDC_MM_CHANNEL_LISTBOX_LABEL1,	0},
	{IDC_MM_CHANNEL_LISTBOX_LABEL2,	22},
	{IDC_MM_CHANNEL_LISTBOX_LABEL3,	110},
	{IDC_MM_CHANNEL_LISTBOX_LABEL4,	130},
	{IDC_MM_CHANNEL_LISTBOX_LABEL5,	150},
	{IDC_MM_CHANNEL_LISTBOX_LABEL6,	165}
};

static int		idParamSliders[2][6] =
{
	{
		IDC_MM_FXA_PARAM1, IDC_MM_FXA_PARAM2, IDC_MM_FXA_PARAM3, IDC_MM_FXA_PARAM4, IDC_MM_FXA_PARAM5, IDC_MM_FXA_PARAM6
	},
	{
		IDC_MM_FXB_PARAM1, IDC_MM_FXB_PARAM2, IDC_MM_FXB_PARAM3, IDC_MM_FXB_PARAM4, IDC_MM_FXB_PARAM5, IDC_MM_FXB_PARAM6
	}
};
static int		idParamLabels[2][6] =
{
	{
		IDC_MM_FXA_PARAM1_NAME, IDC_MM_FXA_PARAM2_NAME,
		IDC_MM_FXA_PARAM3_NAME, IDC_MM_FXA_PARAM4_NAME,
		IDC_MM_FXA_PARAM5_NAME, IDC_MM_FXA_PARAM6_NAME,
	},
	{
		IDC_MM_FXB_PARAM1_NAME, IDC_MM_FXB_PARAM2_NAME,
		IDC_MM_FXB_PARAM3_NAME, IDC_MM_FXB_PARAM4_NAME,
		IDC_MM_FXB_PARAM5_NAME, IDC_MM_FXB_PARAM6_NAME,
	}
};

static LPCSTR	lpszProgramMaps[]	= {"None", "Map 1", "Map 2", "Map 3", "Map 4"};
static LPCSTR	lpszMixBuses[]		= {"Main", "Sub 1", "Sub 2"};
static LPCSTR	lpszBanks[]			= {"Bank 0", "Bank 1", "Bank 2", "Bank 3", "Bank 4"};
static LPCSTR	lpszChanMixBuses[]	= {"Preset", "Main", "Sub1", "FxA", "FxB"};
static int		nCurrentEffect[2]	= {0, 0};

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

DIALOG_PROC MmChannelDefDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

static VOID FillChannelListBox (HWND hwndListBox);
static VOID MMSendParam (HWND hwndParent, LPVOID lpVar);
static VOID InitMMDialog (HWND hWnd);
static VOID MmEffectSelected (HWND hWnd, int idFx);

/*^L*/
/*****************************************************************************
 Function:		DisplayMM
 Parameters:	int		idMM			MMpreset to display.
 Returns:		None.
 Description:	Displays the specified midimap.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94		| Created.											| PW
*****************************************************************************/

VOID DisplayMM (int idMM)
{
	int					i;
	char				szName[20], szTitleBuf[100];
	int					nIndex, nNumParams, nNumDisplayParams;
	EFFECT_PARAM_INFO	*pParamInfo;
	int					idFx;
	static int			idEffects[] = {IDC_MM_FXA_TYPE, IDC_MM_FXB_TYPE};

	// Do nothing if the editing window doesn't exist
	if (hwndCurrentMM == (HWND)NULL)
	{
    	return;
	}

	// Do nothing if the midimap if not loaded
	if ((nDisplayedMM == INDEX_NONE) || (pMorpheusMidimaps[idMM] == NULL))
	{
		return;
	}

	// Display its name
	for (i=0 ; i<ARRAY_SIZE(CurrentMM.Name) ; i++)
	{
		szName[i] = WNUM (CurrentMM.Name[i]);
	}
	szName[i] = '\0';
	SendDlgItemMessage (hwndCurrentMM, IDC_MM_NAME, WM_SETTEXT, 0, (LPARAM)(LPCSTR)&szName[0]);

	// Select its program map
	SendDlgItemMessage (hwndCurrentMM, IDC_MM_PROGRAM_MAP, CB_SETCURSEL, WNUM (CurrentMM.ProgMap) + 1, 0);

	// Select its effect A type
	SendDlgItemMessage (hwndCurrentMM, IDC_MM_FXA_TYPE, CB_SETCURSEL,
						EffectNumberToIndex (FXA, WNUM (CurrentMM.FxAType)), 0);

	// Select its effect B type
	SendDlgItemMessage (hwndCurrentMM, IDC_MM_FXB_TYPE, CB_SETCURSEL,
						EffectNumberToIndex (FXB, WNUM (CurrentMM.FxBType)), 0);

	// Select its FxA bus
	SendDlgItemMessage (hwndCurrentMM, IDC_MM_FXA_BUS, CB_SETCURSEL, WNUM (CurrentMM.FxABus), 0);

	// Select its FxB bus
	SendDlgItemMessage (hwndCurrentMM, IDC_MM_FXB_BUS, CB_SETCURSEL, WNUM (CurrentMM.FxBBus), 0);

	// Select its FXA amount
	SendDlgItemMessage (hwndCurrentMM, IDC_MM_FXA_AMOUNT, SL_SET_CURRENT, WNUM (CurrentMM.FxAAmt), 0);

	// Select its FXB amount
	SendDlgItemMessage (hwndCurrentMM, IDC_MM_FXB_AMOUNT, SL_SET_CURRENT, WNUM (CurrentMM.FxBAmt), 0);

	// Select its FXB->FXA %
	SendDlgItemMessage (hwndCurrentMM, IDC_MM_B_TO_A_AMOUNT, SL_SET_CURRENT, WNUM (CurrentMM.BtoAAmt), 0);

	// For each effect channel
	for (idFx=FXA ; idFx<=FXB ; idFx++)
	{
		// Get its effect ID index
		nIndex = EffectNumberToIndex (idFx, ReadSysexWord (idFx == FXA ? &CurrentMM.FxAType : &CurrentMM.FxBType));

		// Save it
		nCurrentEffect[idFx] = nIndex;

		// How many parameters for this effect ?
		pParamInfo = GetEffectParamInfo (idFx, nIndex, &nNumParams);

		// Only allow up to 6 to be displayed
		nNumDisplayParams = min (nNumParams, ARRAY_SIZE (idParamSliders[0]));

		// For each parameter
		for (i=0 ; i<nNumDisplayParams ; i++)
		{
			// Set the parameter slider min, max, and default
			ShowWindow (GetDlgItem (hwndCurrentMM, idParamSliders[idFx][i]), SW_SHOW);
			InitSlider (hwndCurrentMM, idParamSliders[idFx][i],
							pParamInfo[i].nMin, pParamInfo[i].nMax,
							ReadSysexWord (idFx == FXA ? &CurrentMM.FxAParmVals[i] : &CurrentMM.FxBParmVals[i]));

			// Set the parameter label
			ShowWindow (GetDlgItem (hwndCurrentMM, idParamLabels[idFx][i]), SW_SHOW);
			SetWindowText (GetDlgItem (hwndCurrentMM, idParamLabels[idFx][i]), pParamInfo[i].lpszName);
		}

		// For each irrelevant parameter
		for (; i<ARRAY_SIZE(idParamSliders[0]) ; i++)
		{
			// Hide the parameter slider window
			ShowWindow (GetDlgItem (hwndCurrentMM, idParamSliders[idFx][i]), SW_HIDE);

			// Hide the parameter label
			ShowWindow (GetDlgItem (hwndCurrentMM, idParamLabels[idFx][i]), SW_HIDE);
		}
	}

	// Fill the channel listbox
	FillChannelListBox (GetDlgItem (hwndCurrentMM, IDC_MM_CHANNEL_LISTBOX));

	// Set the editing window title
	wsprintf (&szTitleBuf[0], ResourceString (IDS_EDIT_WINDOW_TITLE_MASK), (LPSTR)&szName[0]);
	SetWindowText (hwndCurrentMM, &szTitleBuf[0]);
}

/*^L*/
/*****************************************************************************
 Function:		CreateCurrentMMWindow
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Creates the midimap edit window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94		| Created.											| PW
*****************************************************************************/

BOOL CreateCurrentMMWindow (VOID)
{
	MDICREATESTRUCT	mc;
	WINDOWPLACEMENT	wp;

	// Create the window
	GetCreatePosition (CURRENT_MIDIMAP_WINDOW, FALSE, &wp);
	mc.szClass	= WCNAME_CURRENT_MM;
	mc.szTitle	= "";
	mc.hOwner	= hInst;
	mc.x		= wp.rcNormalPosition.left;
	mc.y		= wp.rcNormalPosition.top;
	mc.cx		= wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	mc.cy		= wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	mc.style	= WS_CHILD | WS_CAPTION | WS_CLIPSIBLINGS | WS_MINIMIZEBOX | WS_SYSMENU |
					(wp.showCmd & SW_SHOWMINIMIZED ? WS_MINIMIZE : 0);
	mc.lParam	= 0;
	if ((hwndCurrentMM = (HWND)SendMessage (hwndClient,
									WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT)&mc)) == (HWND)NULL)
	{
		// Error
		MyMessageBox (NULL, ResourceString (IDS_MM_CREATING_WINDOW), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Reposition the window
	GetWindowPosition (CURRENT_MIDIMAP_WINDOW, hwndCurrentMM, FALSE);

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		DestroyCurrentMMWindow
 Parameters:	None.
 Returns:		None.
 Description:	Destroys the midimap editing window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94		| Created.											| PW
*****************************************************************************/

VOID DestroyCurrentMMWindow (VOID)
{
	// Save the window position
	SaveWindowPosition (CURRENT_MIDIMAP_WINDOW, hwndCurrentMM);

	// Destroy the window
	SendMessage (hwndClient, WM_MDIDESTROY, (WPARAM)hwndCurrentMM, 0);
	hwndCurrentMM = (HWND)NULL;
}

/*^L*/
/*****************************************************************************
 Function:		CurrentMMDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if processed, else FALSE.
 Description:	The current midimap editor dialog.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94		| Created.											| PW
*****************************************************************************/

WINDOW_PROC CurrentMMDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	int			i;
	int			id;
	char		szName[200];
	static BOOL	fInitialised;
	DWORD		dwVal;
	static int	nDialogWidth, nDialogHeight;

	switch (nMsg)
	{
		// Initialisation
		case WM_CREATE:
		{
			// Uninitialised
			fInitialised = FALSE;

			// Put the dialog int the window
			dwVal = DialogInWindow (hWnd, "DLG_MIDIMAP");
			nDialogWidth	= HIWORD (dwVal);
			nDialogHeight	= LOWORD (dwVal);

			// Reformat myself
			ReformatDialog (hWnd, RDF_CENTRE | RDF_THINFONT);

			// Set the midimap channel listbox tab-stops
			SetListBoxTabs (hWnd, IDC_MM_CHANNEL_LISTBOX, &MMChannelListboxTabs[0], ARRAY_SIZE (MMChannelListboxTabs));

			// Initialise my controls
			InitMMDialog (hWnd);

			// Show the window
			ShowWindow (hWnd, SW_SHOW);

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
			// Do nothing if no midimap has been selected
			if (nDisplayedMM == INDEX_NONE)
			{
				return (TRUE);
			}

			// If it's an edit box change
			if (NCODE == EN_CHANGE)
			{
				// Invoke the handling macros
				switch (id = LOWORD (wParam))
				{
					case IDC_MM_NAME:
						// Fetch the edited name back
						SendDlgItemMessage (hWnd, id, WM_GETTEXT, sizeof (szName), (LPARAM)(LPSTR)&szName[0]);

						// Make sure the name is exactly the right length
						while (strlen (&szName[0]) < ARRAY_SIZE (CurrentMM.Name))
						{
							strcat (&szName[0], " ");
                        }

						// For each character in the stored name
						for (i=0 ; i<ARRAY_SIZE(CurrentMM.Name) ; i++)
						{
							// If the edited character is different from the stored one
							if (szName[i] != (char)ReadSysexWord (&CurrentMM.Name[i]))
							{
								fDirtyMM = TRUE;										// I feel dirty
								WriteSysexWord (&CurrentMM.Name[i], (int)szName[i]);	// Store the edited character
								MMSendParam (hWnd, &CurrentMM.Name[i]);					// Send the character
							}
						}

						return (0);
						
					default:
						break;
				}
			}
			// If it's a new COMBOBOX selection
			if (NCODE == CBN_SELCHANGE)
			{
				// Invoke the handling macros
				switch (id = LOWORD (wParam))
				{
					case IDC_MM_PROGRAM_MAP:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0) - 1;
						HANDLE_COMBOBOX (id, CurrentMM.ProgMap, i);
						break;

					case IDC_MM_FXA_BUS:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, CurrentMM.FxABus, i);
						break;

					case IDC_MM_FXB_BUS:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, CurrentMM.FxBBus, i);
						break;

					// Effect types require lots of work
					case IDC_MM_FXA_TYPE:
						MmEffectSelected (hWnd, FXA);
						break;

					case IDC_MM_FXB_TYPE:
						MmEffectSelected (hWnd, FXB);
						break;

					default:
						break;
				}

				return (0);
			}
			// Else if it's a new list box selection
			else if (NCODE == LBN_DBLCLK)
			{
				// Invoke the handling macros
				switch (id = LOWORD (wParam))
				{
					case IDC_MM_CHANNEL_LISTBOX:
					{
						DLGPROC	lpfnDialogProc;
						int		nIndex;

						// Get the channel number
						nIndex	= SendDlgItemMessage (hWnd, id, LB_GETCURSEL, 0, 0);

						// Call the editing dialog
						lpfnDialogProc = (DLGPROC)MakeProcInstance (MmChannelDefDialogProc, hInst);
						DialogBoxParam (hInst, "DLG_MM_CHANNEL", hWnd, lpfnDialogProc, (LPARAM)nIndex);
						FreeProcInstance ((FARPROC)lpfnDialogProc);

						break;
					}

					default:
						break;
				}
				return (0);
			}


			return (0);
		}

		// Slider selection
		case SLN_NEW_VALUE:
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
				case IDC_MM_FXA_PARAM1:
					HANDLE_AUCNTRL (id, CurrentMM.FxAParmVals[0]);
					break;
				case IDC_MM_FXA_PARAM2:
					HANDLE_AUCNTRL (id, CurrentMM.FxAParmVals[1]);
					break;
				case IDC_MM_FXA_PARAM3:
					HANDLE_AUCNTRL (id, CurrentMM.FxAParmVals[2]);
					break;
				case IDC_MM_FXA_PARAM4:
					HANDLE_AUCNTRL (id, CurrentMM.FxAParmVals[3]);
					break;
				case IDC_MM_FXA_PARAM5:
					HANDLE_AUCNTRL (id, CurrentMM.FxAParmVals[4]);
					break;
				case IDC_MM_FXA_PARAM6:
					HANDLE_AUCNTRL (id, CurrentMM.FxAParmVals[5]);
					break;
				case IDC_MM_FXA_PARAM7:
					HANDLE_AUCNTRL (id, CurrentMM.FxAParmVals[6]);
					break;
				case IDC_MM_FXA_PARAM8:
					HANDLE_AUCNTRL (id, CurrentMM.FxAParmVals[7]);
					break;

				case IDC_MM_FXB_PARAM1:
					HANDLE_AUCNTRL (id, CurrentMM.FxBParmVals[0]);
					break;
				case IDC_MM_FXB_PARAM2:
					HANDLE_AUCNTRL (id, CurrentMM.FxBParmVals[1]);
					break;
				case IDC_MM_FXB_PARAM3:
					HANDLE_AUCNTRL (id, CurrentMM.FxBParmVals[2]);
					break;
				case IDC_MM_FXB_PARAM4:
					HANDLE_AUCNTRL (id, CurrentMM.FxBParmVals[3]);
					break;
				case IDC_MM_FXB_PARAM5:
					HANDLE_AUCNTRL (id, CurrentMM.FxBParmVals[4]);
					break;
				case IDC_MM_FXB_PARAM6:
					HANDLE_AUCNTRL (id, CurrentMM.FxBParmVals[5]);
					break;
				case IDC_MM_FXB_PARAM7:
					HANDLE_AUCNTRL (id, CurrentMM.FxBParmVals[6]);
					break;
				case IDC_MM_FXB_PARAM8:
					HANDLE_AUCNTRL (id, CurrentMM.FxBParmVals[7]);
					break;


				case IDC_MM_FXA_AMOUNT:
					HANDLE_AUCNTRL (id, CurrentMM.FxAAmt);
					break;
				case IDC_MM_FXB_AMOUNT:
					HANDLE_AUCNTRL (id, CurrentMM.FxBAmt);
					break;

				case IDC_MM_B_TO_A_AMOUNT:
					HANDLE_AUCNTRL (id, CurrentMM.BtoAAmt);
					break;

				default:
					break;
			}
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
 Function:		MmEffectSelected
 Parameters:	HWND	hWnd			Dialog window handle.
				int		idFx			Effects channel.
 Returns:		None.
 Description:	Handles a new effect selection :-
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 7/6/94		| Created.											| PW
*****************************************************************************/

static VOID MmEffectSelected (HWND hWnd, int idFx)
{
	static int			idEffects[] = {IDC_MM_FXA_TYPE, IDC_MM_FXB_TYPE};
	int					nIndex, nNumParams, nNumDisplayParams, i;
	EFFECT_PARAM_INFO	*pParamInfo;

	// If the selected effect index has changed
	nIndex = (int)SendDlgItemMessage (hWnd, idEffects[idFx], CB_GETCURSEL, 0, 0);
	if (nIndex != nCurrentEffect[idFx])
	{
		// Save this as the new effect index
		nCurrentEffect[idFx] = nIndex;

		// Mark as dirty
		fDirtyMM = TRUE;

		// Write the effect ID to memory, and send it to the Morpheus
		WriteSysexWord (idFx == FXA ? &CurrentMM.FxAType : &CurrentMM.FxBType, IndexToEffectNumber (idFx, nIndex));
		MMSendParam (hWnd, idFx == FXA ? &CurrentMM.FxAType : &CurrentMM.FxBType);

		// How many parameters for this effect ?
		pParamInfo = GetEffectParamInfo (idFx, nIndex, &nNumParams);

		// Only allow up to 6 to be displayed
		nNumDisplayParams = min (nNumParams, ARRAY_SIZE (idParamSliders[0]));

		// For each parameter
		for (i=0 ; i<nNumDisplayParams ; i++)
		{
			// Set the parameter slider min, max, and default
			ShowWindow (GetDlgItem (hWnd, idParamSliders[idFx][i]), SW_SHOW);
			InitSlider (hWnd, idParamSliders[idFx][i], pParamInfo[i].nMin, pParamInfo[i].nMax, pParamInfo[i].nDef);

			// Set the parameter label
			ShowWindow (GetDlgItem (hWnd, idParamLabels[idFx][i]), SW_SHOW);
			SetWindowText (GetDlgItem (hWnd, idParamLabels[idFx][i]), pParamInfo[i].lpszName);

			// Write the parameter default to memory, and send it to the Morpheus
			WriteSysexWord (idFx == FXA ? &CurrentMM.FxAParmVals[i] : &CurrentMM.FxBParmVals[i], pParamInfo[i].nDef);
			MMSendParam (hWnd, idFx == FXA ? &CurrentMM.FxAParmVals[i] : &CurrentMM.FxBParmVals[i]);
		}

		// For each irrelevant parameter
		for (; i<ARRAY_SIZE(idParamSliders[0]) ; i++)
		{
			// Hide the parameter slider window
			ShowWindow (GetDlgItem (hWnd, idParamSliders[idFx][i]), SW_HIDE);

			// Hide the parameter label
			ShowWindow (GetDlgItem (hWnd, idParamLabels[idFx][i]), SW_HIDE);
		}
	}
}

/*^L*/
/*****************************************************************************
 Function:		InitMMDialog
 Parameters:	HWND	hWnd			Dialog window handle.
 Returns:		None.
 Description:	Initialises the midimap editing dialog.
				This involves initialising the drop-down lists.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94		| Created.											| PW
*****************************************************************************/

static VOID InitMMDialog (HWND hWnd)
{
	UINT	i, nNumEffects;
	char	szEffectName[20];

	// Initialise the program map combobox
	for (i=0 ; i<ARRAY_SIZE(lpszProgramMaps) ; i++)
	{
		SendDlgItemMessage (hWnd, IDC_MM_PROGRAM_MAP, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)lpszProgramMaps[i]);
	}

	// Initialise the mix bus comboboxes
	for (i=0 ; i<ARRAY_SIZE(lpszMixBuses) ; i++)
	{
		SendDlgItemMessage (hWnd, IDC_MM_FXA_BUS, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)lpszMixBuses[i]);
		SendDlgItemMessage (hWnd, IDC_MM_FXB_BUS, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)lpszMixBuses[i]);
	}

	// Initialise the effect A combobox
	nNumEffects = NumEffects (FXA);
	for (i=0 ; i<nNumEffects ; i++)
	{
		lstrcpy (&szEffectName[0], EffectsName (FXA, i));
		SendDlgItemMessage (hWnd, IDC_MM_FXA_TYPE, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)&szEffectName[0]);
	}

	// Initialise the effect B combobox
	nNumEffects = NumEffects (FXB);
	for (i=0 ; i<nNumEffects ; i++)
	{
		lstrcpy (&szEffectName[0], EffectsName (FXB, i));
		SendDlgItemMessage (hWnd, IDC_MM_FXB_TYPE, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)&szEffectName[0]);
	}

	// Set up the effect A slider
	InitSlider (hWnd, IDC_MM_FXA_AMOUNT, 0, 100, 0);

	// Set up the effect B slider
	InitSlider (hWnd, IDC_MM_FXB_AMOUNT, 0, 100, 0);

	// Set up the B->A percentage slider
	InitSlider (hWnd, IDC_MM_B_TO_A_AMOUNT, 0, 101, 50);
}

/*^L*/
/*****************************************************************************
 Function:		MmChannelDefDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
                LPARAM	lParam
 Returns:		BOOL					TRUE if message processed.
 Description:	The midimap channel editing dialog.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 7/6/94		| Created.											| PW
*****************************************************************************/

DIALOG_PROC MmChannelDefDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	int				i, id, NumPrograms;
	static int		nIndex;
	char			szListEntry[100];
	static BOOL		fInitialised;

	int				Program, Bank, Volume, Pan, MixBus;
	int				Enable, ProgChgEnbl, BankChgEnbl, VolCtlEnbl, PanCtlEnbl, PWhlEnbl;
	int				MPressEnbl, PPressEnbl, CtlAEnbl, CtlBEnbl, CtlCEnbl, CtlDEnbl, Ftsw1Enbl, Ftsw2Enbl, Ftsw3Enbl;

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
			Program			= WNUM (CurrentMM.Channel[nIndex].Program);
			Bank			= WNUM (CurrentMM.Channel[nIndex].Bank);
			Volume			= WNUM (CurrentMM.Channel[nIndex].Volume);
			Pan				= WNUM (CurrentMM.Channel[nIndex].Pan);
			MixBus			= WNUM (CurrentMM.Channel[nIndex].MixBus);
			Enable			= WNUM (CurrentMM.Channel[nIndex].Enable);
			ProgChgEnbl		= WNUM (CurrentMM.Channel[nIndex].ProgChgEnbl);
			BankChgEnbl		= WNUM (CurrentMM.Channel[nIndex].BankChgEnbl);
			VolCtlEnbl		= WNUM (CurrentMM.Channel[nIndex].VolCtlEnbl);
			PanCtlEnbl		= WNUM (CurrentMM.Channel[nIndex].PanCtlEnbl);
			PWhlEnbl		= WNUM (CurrentMM.Channel[nIndex].PWhlEnbl);
			MPressEnbl		= WNUM (CurrentMM.Channel[nIndex].MPressEnbl);
			PPressEnbl		= WNUM (CurrentMM.Channel[nIndex].PPressEnbl);
			CtlAEnbl		= WNUM (CurrentMM.Channel[nIndex].CtlAEnbl);
			CtlBEnbl		= WNUM (CurrentMM.Channel[nIndex].CtlBEnbl);
			CtlCEnbl		= WNUM (CurrentMM.Channel[nIndex].CtlCEnbl);
			CtlDEnbl		= WNUM (CurrentMM.Channel[nIndex].CtlDEnbl);
			Ftsw1Enbl		= WNUM (CurrentMM.Channel[nIndex].Ftsw1Enbl);
			Ftsw2Enbl		= WNUM (CurrentMM.Channel[nIndex].Ftsw2Enbl);
			Ftsw3Enbl		= WNUM (CurrentMM.Channel[nIndex].Ftsw3Enbl);

			// Set the dialog's title
			wsprintf (&szListEntry[0], ResourceString (IDS_MM_EDIT_CHANNEL_TITLE), nIndex + 1);
			SetWindowText (hWnd, (LPCSTR)&szListEntry[0]);

			// Fill in the PRESETS/HYPERPRESETS list
			NumPrograms = WNUM (pMorpheusPresetList->wNumPresets) + WNUM (pMorpheusHyperpresetList->wNumHyperpresets);
			for (i=0 ; i<NumPrograms ; i++)
			{
				wsprintf (&szListEntry[0], "%s", ProgramName (i));
				SendDlgItemMessage (hWnd, IDC_MM_CHANNEL_PROGRAM, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)&szListEntry[0]);
			}

			// Select the current preset
			SendDlgItemMessage (hWnd, IDC_MM_CHANNEL_PROGRAM, CB_SETCURSEL, Program, 0);

			// Initialise the channel BANK list
			for (i=0 ; i<ARRAY_SIZE(lpszBanks) ; i++)
			{
				SendDlgItemMessage (hWnd, IDC_MM_CHANNEL_BANK, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)lpszBanks[i]);
			}
			SendDlgItemMessage (hWnd, IDC_MM_CHANNEL_BANK, CB_SETCURSEL, Bank, 0);

			// Initialise the channel MIX BUS list
			for (i=0 ; i<ARRAY_SIZE(lpszChanMixBuses) ; i++)
			{
				SendDlgItemMessage (hWnd, IDC_MM_CHANNEL_MIX_BUS, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)lpszChanMixBuses[i]);
			}
			SendDlgItemMessage (hWnd, IDC_MM_CHANNEL_MIX_BUS, CB_SETCURSEL, MixBus + 1, 0);

			// Initialise the volume control
			InitSlider (hWnd, IDC_MM_CHANNEL_VOLUME, 0, 127, Volume);

			// Initialise the pan control
			InitSlider (hWnd, IDC_MM_CHANNEL_PAN, -8, 7, Pan);

			// Set the check boxes
			CheckDlgButton (hWnd, IDC_MM_CHANNEL_ENABLE, Enable);
			CheckDlgButton (hWnd, IDC_MM_CHANNEL_PROGCHG_ENBL, ProgChgEnbl);
			CheckDlgButton (hWnd, IDC_MM_CHANNEL_BANKCHG_ENBL, BankChgEnbl);
			CheckDlgButton (hWnd, IDC_MM_CHANNEL_VOLCTL_ENBL, VolCtlEnbl);
			CheckDlgButton (hWnd, IDC_MM_CHANNEL_PANCTL_ENBL, PanCtlEnbl);
			CheckDlgButton (hWnd, IDC_MM_CHANNEL_PWHL_ENBL, PWhlEnbl);
			CheckDlgButton (hWnd, IDC_MM_CHANNEL_MPRESS_ENBL, MPressEnbl);
			CheckDlgButton (hWnd, IDC_MM_CHANNEL_PPRESS_ENBL, PPressEnbl);
			CheckDlgButton (hWnd, IDC_MM_CHANNEL_CTLA_ENBL, CtlAEnbl);
			CheckDlgButton (hWnd, IDC_MM_CHANNEL_CTLB_ENBL, CtlBEnbl);
			CheckDlgButton (hWnd, IDC_MM_CHANNEL_CTLC_ENBL, CtlCEnbl);
			CheckDlgButton (hWnd, IDC_MM_CHANNEL_CTLD_ENBL, CtlDEnbl);
			CheckDlgButton (hWnd, IDC_MM_CHANNEL_FTSW1_ENBL, Ftsw1Enbl);
			CheckDlgButton (hWnd, IDC_MM_CHANNEL_FTSW2_ENBL, Ftsw2Enbl);
			CheckDlgButton (hWnd, IDC_MM_CHANNEL_FTSW3_ENBL, Ftsw3Enbl);

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
				case IDC_MM_CHANNEL_VOLUME:
					HANDLE_AUCNTRL (id, CurrentMM.Channel[nIndex].Volume);
					break;

				case IDC_MM_CHANNEL_PAN:
					HANDLE_AUCNTRL (id, CurrentMM.Channel[nIndex].Pan);
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
					// Program
					case IDC_MM_CHANNEL_PROGRAM:
					{
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, CurrentMM.Channel[nIndex].Program, i);
						break;
					}

					// Bank
					case IDC_MM_CHANNEL_BANK:
					{
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, CurrentMM.Channel[nIndex].Bank, i);
						break;
					}

					// Mix bus
					case IDC_MM_CHANNEL_MIX_BUS:
					{
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0) - 1;
						HANDLE_COMBOBOX (id, CurrentMM.Channel[nIndex].MixBus, i);
						break;
					}
				}

				return (TRUE);
			}
            // Else if it's a button
			else if (NCODE == BN_CLICKED)
			{
				// Only think about OK
				switch (id = LOWORD (wParam))
				{
					// "Finished"
					case IDOK:
                    {
						// Update the channel listbox
						FillChannelListBox (GetDlgItem (hwndCurrentMM, IDC_MM_CHANNEL_LISTBOX));

						// Re-select the current item
						SendDlgItemMessage (hwndCurrentMM, IDC_MM_CHANNEL_LISTBOX, LB_SETCURSEL, nIndex, 0);

						// Bye bye
						EndDialog (hWnd, 0);

                        break;
					}

					// The MIDI enables
					case IDC_MM_CHANNEL_ENABLE:
						HANDLE_BUTTON (id, CurrentMM.Channel[nIndex].Enable);
						break;
					case IDC_MM_CHANNEL_PROGCHG_ENBL:
						HANDLE_BUTTON (id, CurrentMM.Channel[nIndex].ProgChgEnbl);
						break;
					case IDC_MM_CHANNEL_BANKCHG_ENBL:
						HANDLE_BUTTON (id, CurrentMM.Channel[nIndex].BankChgEnbl);
						break;
					case IDC_MM_CHANNEL_VOLCTL_ENBL:
						HANDLE_BUTTON (id, CurrentMM.Channel[nIndex].VolCtlEnbl);
						break;
					case IDC_MM_CHANNEL_PANCTL_ENBL:
						HANDLE_BUTTON (id, CurrentMM.Channel[nIndex].PanCtlEnbl);
						break;
					case IDC_MM_CHANNEL_PWHL_ENBL:
						HANDLE_BUTTON (id, CurrentMM.Channel[nIndex].PWhlEnbl);
						break;
					case IDC_MM_CHANNEL_MPRESS_ENBL:
						HANDLE_BUTTON (id, CurrentMM.Channel[nIndex].MPressEnbl);
						break;
					case IDC_MM_CHANNEL_PPRESS_ENBL:
						HANDLE_BUTTON (id, CurrentMM.Channel[nIndex].PPressEnbl);
						break;
					case IDC_MM_CHANNEL_CTLA_ENBL:
						HANDLE_BUTTON (id, CurrentMM.Channel[nIndex].CtlAEnbl);
						break;
					case IDC_MM_CHANNEL_CTLB_ENBL:
						HANDLE_BUTTON (id, CurrentMM.Channel[nIndex].CtlBEnbl);
						break;
					case IDC_MM_CHANNEL_CTLC_ENBL:
						HANDLE_BUTTON (id, CurrentMM.Channel[nIndex].CtlCEnbl);
						break;
					case IDC_MM_CHANNEL_CTLD_ENBL:
						HANDLE_BUTTON (id, CurrentMM.Channel[nIndex].CtlDEnbl);
						break;
					case IDC_MM_CHANNEL_FTSW1_ENBL:
						HANDLE_BUTTON (id, CurrentMM.Channel[nIndex].Ftsw1Enbl);
						break;
					case IDC_MM_CHANNEL_FTSW2_ENBL:
						HANDLE_BUTTON (id, CurrentMM.Channel[nIndex].Ftsw2Enbl);
						break;
					case IDC_MM_CHANNEL_FTSW3_ENBL:
						HANDLE_BUTTON (id, CurrentMM.Channel[nIndex].Ftsw3Enbl);
						break;

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
 Function:		FillChannelListBox
 Parameters:	HWND	hwndListBox		Channel listbox window handle.
 Returns:		None.
 Description:	Fills the midimap channel listbox specified with information
				from the current midimap.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 7/6/94		| Created.											| PW
*****************************************************************************/

static VOID FillChannelListBox (HWND hwndListBox)
{
	UINT			i, nLen;
	char			szLineBuf[200];
	int				nProgram, nPan;

	// Clear the list
	SendMessage (hwndListBox, LB_RESETCONTENT, 0, 0);

	// For each CHANNEL (of 16)
	for (i=0 ; i<NUM_MIDI_CHANNELS ; i++)
	{
		// Format the line ...

		// The CHANNEL number (1..16)
		itoa (i + 1, &szLineBuf[0], 10);

		// The PROGRAM NAME				 
		lstrcat (&szLineBuf[0], "\t");
		nProgram = WNUM (CurrentMM.Channel[i].Program);
		lstrcat (&szLineBuf[0], ProgramName (nProgram));

		// The BANK NUMBER
		nLen = lstrlen (&szLineBuf[0]);
		wsprintf (&szLineBuf[nLen], "\t%d", WNUM (CurrentMM.Channel[i].Bank));

		// The VOLUME
		nLen = lstrlen (&szLineBuf[0]);
		wsprintf (&szLineBuf[nLen], "\t%d", WNUM (CurrentMM.Channel[i].Volume));

		// The PAN
		nLen = lstrlen (&szLineBuf[0]);
		if ((nPan = WNUM (CurrentMM.Channel[i].Pan)) == -8)
		{
			wsprintf (&szLineBuf[nLen], "\tP");
		}
		else
		{
			wsprintf (&szLineBuf[nLen], "\t%d", WNUM (CurrentMM.Channel[i].Pan));
		}

		// The MIX BUS
		nLen = lstrlen (&szLineBuf[0]);
		wsprintf (&szLineBuf[nLen], "\t%s", lpszChanMixBuses[WNUM (CurrentMM.Channel[i].MixBus) + 1]);

		// Put it in the listbox
		SendMessage (hwndListBox, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)&szLineBuf[0]);
	}
}

/*^L*/
/*****************************************************************************
 Function:		MMSendParam
 Parameters:	HWND	hwndParent		Parent window.
				LPVOID	lpVar			Address of variable within current
										midimap structure.
 Returns:		None.
 Description:	Sends a single midimap parameter to the Morpheus.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94		| Created.											| PW
*****************************************************************************/

static VOID MMSendParam (HWND hwndParent, LPVOID lpVar)
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
		start of the midimap (param #0 is &pPreset->Name[0]).
	 */
	wParamNumber = MPARAM_MIDIMAP_BASE + (lpwVar - &CurrentMM.Name[0]);

	// Set its value
	MorpheusSetParameter (hwndParent, (int)wParamNumber, ReadSysexWord (lpwVar));
}
