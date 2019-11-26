/*****************************************************************************
 File:			preset2.c
 Description:	Functions for the main PRESET EDITOR dialog. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 29/3/94	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define	GETLPARAM(id,minval,maxval)													\
		{																			\
			char	szEditText[20];													\
			SendDlgItemMessage (hWnd, id, WM_GETTEXT, sizeof (szEditText), (LPARAM)(LPSTR)&szEditText[0]);	\
			lParam = atol (&szEditText[0]);											\
			if (lParam < minval)													\
			{																		\
				lParam = minval;													\
                SetDlgItemInt (hWnd, id, (int)lParam, TRUE);						\
			}																		\
			else if (lParam > maxval)												\
			{																		\
				lParam = maxval;													\
				SetDlgItemInt (hWnd, id, (int)lParam, TRUE);						\
			}																		\
		}

#define	HANDLE_BUTTON(id,var)														\
		{																			\
			int	nNewVal = SendDlgItemMessage (hWnd, id, BM_GETCHECK, 0, 0);			\
			if (nNewVal != ReadSysexWord (&var))									\
            {																		\
				fDirtyPreset = TRUE;												\
				WriteSysexWord (&var, nNewVal);										\
				PresetSendParam (hWnd, &var);										\
			}																		\
		}

#define	HANDLE_COMBOBOX(id,var,newval)												\
		{																			\
			if (newval != ReadSysexWord (&var))										\
            {																		\
				fDirtyPreset = TRUE;												\
				WriteSysexWord (&var, newval);										\
				PresetSendParam (hWnd, &var);										\
			}																		\
		}

#define	HANDLE_AUCNTRL(id,var)						\
		if (ReadSysexWord (&var) != (int)lParam)	\
		{											\
			fDirtyPreset = TRUE;					\
			WriteSysexWord (&var, (int)lParam);		\
			PresetSendParam (hWnd, &var);			\
		}


/****************************************************************************/
/*								Local types									*/
/****************************************************************************/

typedef enum {HIGH_DELTA, BOTH_ABS, LOW_DELTA} VALTYPE;

typedef struct
{
	int				nFG;				// Function generator being edited
	int				nStep;				// Function generator step being edited
} FG_EDIT_PARAM, FAR *PFG_EDIT_PARAM;

/****************************************************************************/
/*								Global data									*/
/****************************************************************************/

BOOL	fBigPresetDialog = FALSE;	// TRUE if whole preset is displayed in one big dialog

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

static LPCSTR pszNoteOnSources[] =
			{"Key", "Velocity", "Pitchwheel", "Control A", "Control B",
			"Control C", "Control D", "Mono pressure", "Free-run FG"};
static LPCSTR pszRealTimeSources[] =
			{"Pitchwheel", "Control A", "Control B", "Control C", "Control D", "Mono pressure",
			 "Poly pressure", "LFO 1", "LFO 2", "Aux env.", "FG 1", "FG 2", "Free-run FG"};
static LPCSTR pszPatchDests[] =
			{"BOff", "BPitch", "BPitch P", "BPitch S", "BVolume", "BVolume P", "BVolume S",
			 "BAttack", "BAttack P", "BAttack S", "BDecay", "BDecay P", "BDecay S",
			 "BRelease", "BRelease P", "BRelease S", "BXfade", "BLFO1 amt", "BLFO1 rate",
			 "BLFO2 amt", "BLFO2 rate", "BAux amt", "BAux att", "BAux dec", "BAux rel",
			 "NStart", "NStart P", "NStart S", "BPan", "BPan P", "BPan S", "NTone",
			 "NTone P", "NTone S", "BMorph", "BMorph P", "BMorph S", "NTrans2", "NTrans2 P", "NTrans2 S",
			 "BPort rt", "BPortRt P", "BPortRt S", "BFG1 amt", "BFG2 amt", "NFiltLev", "NFiltLev P",
			 "NFiltLev S", "NFreq trk", "NFreq trk P", "NFreq trk S"};

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

DIALOG_PROC NoteOnPatchDefDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
DIALOG_PROC RealTimePatchDefDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
DIALOG_PROC FuncGenDefDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

static VOID LoopStart (int nLayer, VALTYPE valtype, LONG nVal);
static VOID LoopLength (int nLayer, VALTYPE valtype, LONG nVal);

/*^L*/
/*****************************************************************************
 Function:		CreateCurrentPresetWindwow
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Creates the preset window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 25/5/94 	| Created.											| PW
*****************************************************************************/

BOOL CreateCurrentPresetWindow (VOID)
{
	MDICREATESTRUCT	mc;
	WINDOWPLACEMENT	wp;

	GetCreatePosition (CURRENT_PRESET_WINDOW, FALSE, &wp);

	// Create the window
	mc.szClass	= WCNAME_CURRENT_PRESET;
	mc.szTitle	= ResourceString (IDS_PR_WINDOW_TITLE);
	mc.hOwner	= hInst;
	mc.x		= wp.rcNormalPosition.left;
	mc.y		= wp.rcNormalPosition.top;
	mc.cx		= wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	mc.cy		= wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	mc.style	= WS_CHILD | WS_CAPTION | WS_CLIPSIBLINGS | WS_MINIMIZEBOX | WS_SYSMENU |
					(wp.showCmd & SW_SHOWMINIMIZED ? WS_MINIMIZE : 0);
	mc.lParam	= 0;
	if ((hwndCurrentPreset = (HWND)SendMessage (hwndClient,
									WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT)&mc)) == (HWND)NULL)
	{
		// Error
		MyMessageBox (NULL, ResourceString (IDS_PR_ERR_CREAT_WND), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Reposition the window
	GetWindowPosition (CURRENT_PRESET_WINDOW, hwndCurrentPreset, FALSE);

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		DestroyCurrentPresetWindow
 Parameters:	None.
 Returns:		None.
 Description:	Destroys the current preset editing window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/5/94 	| Created.											| PW
*****************************************************************************/

VOID DestroyCurrentPresetWindow (VOID)
{
	// Save the window position
	SaveWindowPosition (CURRENT_PRESET_WINDOW, hwndCurrentPreset);

	// Destroy the window
	SendMessage (hwndClient, WM_MDIDESTROY, (WPARAM)hwndCurrentPreset, 0);
	hwndCurrentPreset = (HWND)NULL;
}

/*^L*/
/*****************************************************************************
 Function:		CurrentPresetDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if processed, else FALSE.
 Description:	The current preset dialog.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 29/3/94 	| Created.											| PW
*****************************************************************************/

WINDOW_PROC CurrentPresetDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	int				i;
	int				id;
//	PPRESET_PARAMS	pPreset;
	char			szName[200];
	static BOOL		fInitialised;
	DWORD			dwVal;
	static int		cxMin, cxMax, cyMin, cyMax;

	switch (nMsg)
	{
		// Initialisation
		case WM_CREATE:
		{
			LPCREATESTRUCT	lpCs = (LPCREATESTRUCT)lParam;
			
			// Quick - save my handle
			fInitialised = FALSE;
			hwndCurrentPreset = hWnd;

			// If we are 'big preset dialog' mode
			if (fBigPresetDialog)
			{
				// Put the dialog in the window
				dwVal = DialogInWindow (hWnd, "DLG_PRESET");
				cxMin = cyMax = HIWORD (dwVal);
				cyMin = cyMax = LOWORD (dwVal);

				// Reformat myself
				ReformatDialog (hWnd, RDF_THINFONT);
			}
			else
			{
				// Initlialise the sub-dialog stuff
				dwVal = InitPresetSubDialog ();
				cxMin = cyMax = HIWORD (dwVal);
				cyMin = cyMax = LOWORD (dwVal);
			}

			// Initialise my controls
			InitInstrumentsDialog (hWnd);
			InitModulatorsDialog (hWnd);
			InitMiscDialog (hWnd);
			fInitialised = TRUE;

			ShowWindow (hWnd, SW_HIDE);

			return (0);
		}

		// User "set window size" message
		case WM_USER_SET_WINDOW_SIZE:
		{
			cxMin = cyMax = HIWORD (lParam);
			cyMin = cyMax = LOWORD (lParam);
			return (0);
		}

		// Trying to change the dialog width
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO FAR	*lpmm = (MINMAXINFO FAR *)lParam;

			if (fInitialised)
			{
				lpmm->ptMinTrackSize.x	= cxMin;
				lpmm->ptMinTrackSize.y	= cyMin;
				lpmm->ptMaxTrackSize.x	= cxMax;
				lpmm->ptMaxTrackSize.y	= cyMax;
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
			// Do nothing if this is the initialisation phase
			if (!fInitialised)
			{
				return (0);
			}

			// Get the control ID
			id = LOWORD (wParam);

			// Try the toolbar buttons
			switch (id)
			{
				// First try the 'toolbar' buttons
				case IDC_PRESET_TOOLBAR_MAIN:
				case IDC_PRESET_TOOLBAR_VOICES:
				case IDC_PRESET_TOOLBAR_LFO_AUXENV:
				case IDC_PRESET_TOOLBAR_FG:
				{
					// Make sure it's a straight click
					if (NCODE == BN_CLICKED)
					{
						// Yes - switch to that sub-dialog type
						cxMin = cyMin = 0;
						cxMax = cyMax = 4000;
						dwVal = PresetSubDialog (id);

						// Store the new dialog dimensions
						cxMin = cxMax = HIWORD (dwVal);
						cyMin = cyMax = LOWORD (dwVal);

						// Initialise my controls
						fInitialised = FALSE;
						InitInstrumentsDialog (hWnd);
						InitModulatorsDialog (hWnd);
						InitMiscDialog (hWnd);
						fInitialised = TRUE;

						// If a preset is on display
						if (nDisplayedPreset != INDEX_NONE)
						{
							// Force the preset to be redisplayed
							DisplayPreset ();
						}
						
						return (0);
					}
				}

				default:
				{
					break;
				}
			}

			// If no preset is on display
			if (nDisplayedPreset == INDEX_NONE)
			{
				// Go no further
				return (0);
			}

			// If it's a button click
			if (NCODE == BN_CLICKED)
			{
				// Invoke the handling macros
				switch (id)
				{
					case IDC_INSTR1_REVERSE:
						HANDLE_BUTTON (id, EditPreset.Layer[0].SoundReverse);
						break;
					case IDC_INSTR1_NON_TRANSPOSE:
						HANDLE_BUTTON (id, EditPreset.Layer[0].NonTranspose);
                        break;
					case IDC_INSTR1_LOOP_ENABLE:
						HANDLE_BUTTON (id, EditPreset.Layer[0].Loop.Enable);
                        break;
					case IDC_FILTER1_REVERSE:
						HANDLE_BUTTON (id, EditPreset.Layer[0].Filter.Reverse);
                        break;
					case IDC_ALT_ENV1_ENABLE:
						HANDLE_BUTTON (id, EditPreset.Layer[0].AltEnv.Enable);
                        break;
					case IDC_INSTR2_REVERSE:
						HANDLE_BUTTON (id, EditPreset.Layer[1].SoundReverse);
                        break;
					case IDC_INSTR2_NON_TRANSPOSE:
						HANDLE_BUTTON (id, EditPreset.Layer[1].NonTranspose);
                        break;
					case IDC_INSTR2_LOOP_ENABLE:
						HANDLE_BUTTON (id, EditPreset.Layer[1].Loop.Enable);
                        break;
					case IDC_FILTER2_REVERSE:
						HANDLE_BUTTON (id, EditPreset.Layer[1].Filter.Reverse);
                        break;
					case IDC_ALT_ENV2_ENABLE:
						HANDLE_BUTTON (id, EditPreset.Layer[1].AltEnv.Enable);
                        break;
					default:
						break;
				}
			}
			// Else if it's a new COMBOBOX selection
			else if (NCODE == CBN_SELCHANGE)
			{
				// Invoke the handling macros
				switch (id = LOWORD (wParam))
				{
					case IDC_INSTR1_INSTRUMENT:
						i = IndexToInstrumentNumber ((int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0));
						HANDLE_COMBOBOX (id, EditPreset.Layer[0].LayerInstrument, i);
						break;
					case IDC_FILTER1_TYPE:
						i = IndexToFilterNumber ((int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0));
						HANDLE_COMBOBOX (id, EditPreset.Layer[0].Filter.Type, i);
						break;
					case IDC_PORTAMENTO1_SHAPE:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.Layer[0].PortamentoShape, i);
						break;
					case IDC_INSTR1_SOLO_MODE_CONTROL:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.Layer[0].SoloMode, i);
						break;
					case IDC_INSTR1_SOLO_PRIORITY_CONTROL:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.Layer[0].SoloPriority, i);
						break;
					case IDC_INSTR2_INSTRUMENT:
						i = IndexToInstrumentNumber ((int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0));
						HANDLE_COMBOBOX (id, EditPreset.Layer[1].LayerInstrument, i);
						break;
					case IDC_FILTER2_TYPE:
						i = IndexToFilterNumber ((int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0));
						HANDLE_COMBOBOX (id, EditPreset.Layer[1].Filter.Type, i);
						break;
					case IDC_PORTAMENTO2_SHAPE:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.Layer[1].PortamentoShape, i);
						break;
					case IDC_INSTR2_SOLO_MODE_CONTROL:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.Layer[1].SoloMode, i);
						break;
					case IDC_INSTR2_SOLO_PRIORITY_CONTROL:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.Layer[1].SoloPriority, i);
						break;

					case IDC_LFO1_SHAPE:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.Lfo[0].Shape, i);
						break;
					case IDC_LFO2_SHAPE:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.Lfo[1].Shape, i);
						break;
					case IDC_KBD_VELOCITY_CURVE:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.VelocityCurve, i);
						break;
					case IDC_KBD_TUNING:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.TuneTable, i);
						break;
					case IDC_PORTAMENTO_MODE:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.PortMode, i);
						break;

					case IDC_MISC_MIX_SELECT:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.MixBus, i);
						break;
					case IDC_MISC_FOOTSWITCH1_CONTROL:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.FootSwitchDestination[0], i);
						break;
					case IDC_MISC_FOOTSWITCH2_CONTROL:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.FootSwitchDestination[1], i);
						break;
					case IDC_MISC_FOOTSWITCH3_CONTROL:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.FootSwitchDestination[2], i);
						break;
					case IDC_XFADE_MODE:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.XFade.Mode, i);
						break;
					case IDC_XFADE_DIRECTION:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.XFade.Direction, i);
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
					case IDC_MISC_PRESET_NAME:
						// Fetch the edited name back
						SendDlgItemMessage (hWnd, id, WM_GETTEXT, sizeof (szName), (LPARAM)(LPSTR)&szName[0]);

						// Make sure the name is exactly the right length
						while (strlen (&szName[0]) < ARRAY_SIZE (EditPreset.Name))
						{
							strcat (&szName[0], " ");
                        }

						// For each character in the stored name
						for (i=0 ; i<ARRAY_SIZE(EditPreset.Name) ; i++)
						{
							// If the edited character is different from the stored one
							if (szName[i] != (char)ReadSysexWord (&EditPreset.Name[i]))
							{
								// I feel dirty
								fDirtyPreset = TRUE;

								// Store the edited character
								WriteSysexWord (&EditPreset.Name[i], (int)szName[i]);

								// Send the character
								PresetSendParam (hWnd, &EditPreset.Name[i]);
							}
						}

						break;
					default:
						break;
				}
			}
			// Else if it's a new list box selection
			else if (NCODE == LBN_DBLCLK)
			{
				// Invoke the handling macros
				switch (id = LOWORD (wParam))
				{
					case IDC_MISC_NOTE_ON_PATCH_LISTBOX:
					{
						DLGPROC		lpfnDialogProc;
						int			nPatchNumber;

						// Get the patch number
						nPatchNumber = SendDlgItemMessage (hWnd, id, LB_GETCURSEL, 0, 0);

						// Call the editing dialog
						lpfnDialogProc = (DLGPROC)MakeProcInstance (NoteOnPatchDefDialogProc, hInst);
						DialogBoxParam (hInst, "DLG_PATCH", hWnd, lpfnDialogProc, (LPARAM)nPatchNumber);
						FreeProcInstance ((FARPROC)lpfnDialogProc);

						break;
					}
					case IDC_MISC_REAL_TIME_PATCH_LISTBOX:
					{
						DLGPROC		lpfnDialogProc;
						int			nPatchNumber;

						// Get the patch number
						nPatchNumber = SendDlgItemMessage (hWnd, id, LB_GETCURSEL, 0, 0);

						// Call the editing dialog
						lpfnDialogProc = (DLGPROC)MakeProcInstance (RealTimePatchDefDialogProc, hInst);
						DialogBoxParam (hInst, "DLG_PATCH", hWnd, lpfnDialogProc, (LPARAM)nPatchNumber);
						FreeProcInstance ((FARPROC)lpfnDialogProc);

						break;
					}
					case IDC_FG1_LISTBOX:
					case IDC_FG2_LISTBOX:
					{
						FG_EDIT_PARAM		EditParams;
						DLGPROC				lpfnDialogProc;

						// Set up the edit parameters structure
						EditParams.nFG			= ((id == IDC_FG1_LISTBOX) ? 0 : 1);
						EditParams.nStep		= SendDlgItemMessage (hWnd, id, LB_GETCURSEL, 0, 0);

						// Call the editing dialog
						lpfnDialogProc = (DLGPROC)MakeProcInstance (FuncGenDefDialogProc, hInst);
						DialogBoxParam (hInst, "DLG_FG_EDIT", hWnd, lpfnDialogProc, (LPARAM)(PFG_EDIT_PARAM)&EditParams);
						FreeProcInstance ((FARPROC)lpfnDialogProc);

						break;
                    }

					default:
						break;
				}
			}


			return (0);
		}

		// ENTER pressed in an edit control
		case EN_ENTER:
		{
			char	szLongBuf[40], szHighBuf[20], szLowBuf[20];
			int		nLow;

			// If no preset is on display
			if (nDisplayedPreset == INDEX_NONE)
			{
				// Go no further
				return (0);
			}

			// Invoke the handling macros
			switch (id = LOWORD (wParam))
			{
				case IDC_INSTR1_LOOP_START_HIGH_VALUE:
				case IDC_INSTR1_LOOP_START_LOW_VALUE:
					// Read back both editboxes
					SendDlgItemMessage (hWnd, IDC_INSTR1_LOOP_START_HIGH_VALUE, WM_GETTEXT,
										10, (LPARAM)(LPSTR)&szHighBuf[0]);
					SendDlgItemMessage (hWnd, IDC_INSTR1_LOOP_START_LOW_VALUE, WM_GETTEXT,
										10, (LPARAM)(LPSTR)&szLowBuf[0]);
					nLow = atoi (&szLowBuf[0]);
					nLow = (nLow < 0) ? 0 : (nLow > 999 ? 999 : nLow);
					wsprintf (&szLongBuf[0], "%s%03d", (LPCSTR)&szHighBuf[0], nLow);
					LoopStart (0, BOTH_ABS, atol (&szLongBuf[0]));
					break;

				case IDC_INSTR1_LOOP_LENGTH_HIGH_VALUE:
				case IDC_INSTR1_LOOP_LENGTH_LOW_VALUE:
					// Read back both editboxes
					SendDlgItemMessage (hWnd, IDC_INSTR1_LOOP_LENGTH_HIGH_VALUE, WM_GETTEXT,
										10, (LPARAM)(LPSTR)&szHighBuf[0]);
					SendDlgItemMessage (hWnd, IDC_INSTR1_LOOP_LENGTH_LOW_VALUE, WM_GETTEXT,
										10, (LPARAM)(LPSTR)&szLowBuf[0]);
					nLow = atoi (&szLowBuf[0]);
					nLow = (nLow < 0) ? 0 : (nLow > 999 ? 999 : nLow);
					wsprintf (&szLongBuf[0], "%s%03d", (LPCSTR)&szHighBuf[0], nLow);
					LoopLength (0, BOTH_ABS, atol (&szLongBuf[0]));
					break;

				case IDC_INSTR2_LOOP_START_HIGH_VALUE:
				case IDC_INSTR2_LOOP_START_LOW_VALUE:
					// Read back both editboxes
					SendDlgItemMessage (hWnd, IDC_INSTR2_LOOP_START_HIGH_VALUE, WM_GETTEXT,
										10, (LPARAM)(LPSTR)&szHighBuf[0]);
					SendDlgItemMessage (hWnd, IDC_INSTR2_LOOP_START_LOW_VALUE, WM_GETTEXT,
										10, (LPARAM)(LPSTR)&szLowBuf[0]);
					nLow = atoi (&szLowBuf[0]);
					nLow = (nLow < 0) ? 0 : (nLow > 999 ? 999 : nLow);
					wsprintf (&szLongBuf[0], "%s%03d", (LPCSTR)&szHighBuf[0], nLow);
					LoopStart (1, BOTH_ABS, atol (&szLongBuf[0]));
					break;

				case IDC_INSTR2_LOOP_LENGTH_HIGH_VALUE:
				case IDC_INSTR2_LOOP_LENGTH_LOW_VALUE:
					// Read back both editboxes
					SendDlgItemMessage (hWnd, IDC_INSTR2_LOOP_LENGTH_HIGH_VALUE, WM_GETTEXT,
										10, (LPARAM)(LPSTR)&szHighBuf[0]);
					SendDlgItemMessage (hWnd, IDC_INSTR2_LOOP_LENGTH_LOW_VALUE, WM_GETTEXT,
										10, (LPARAM)(LPSTR)&szLowBuf[0]);
					nLow = atoi (&szLowBuf[0]);
					nLow = (nLow < 0) ? 0 : (nLow > 999 ? 999 : nLow);
					wsprintf (&szLongBuf[0], "%s%03d", (LPCSTR)&szHighBuf[0], nLow);
					LoopLength (1, BOTH_ABS, atol (&szLongBuf[0]));
					break;

#ifdef NEVER
				case IDC_INSTR1_LOOP_START_LOW_VALUE:
					GETLPARAM (0, 999);
					LoopStart (0, LOW_ABS, (int)lParam);
					break;

				case IDC_INSTR1_LOOP_LENGTH_HIGH_VALUE:
					GETLPARAM (-999, 999);
					LoopLength (0, HIGH_ABS, (int)lParam);
					break;

				case IDC_INSTR1_LOOP_LENGTH_LOW_VALUE:
					GETLPARAM (0, 999);
					LoopLength (0, LOW_ABS, (int)lParam);
					break;

				case IDC_INSTR2_LOOP_START_HIGH_VALUE:
					GETLPARAM (-999, 999);
					LoopStart (1, HIGH_ABS, (int)lParam);
					break;

				case IDC_INSTR2_LOOP_START_LOW_VALUE:
					GETLPARAM (0, 999);
					LoopStart (1, LOW_ABS, (int)lParam);
					break;

				case IDC_INSTR2_LOOP_LENGTH_HIGH_VALUE:
					GETLPARAM (-999, 999);
					LoopLength (1, HIGH_ABS, (int)lParam);
					break;

				case IDC_INSTR2_LOOP_LENGTH_LOW_VALUE:
					GETLPARAM (0, 999);
					LoopLength (1, LOW_ABS, (int)lParam);
					break;
#endif

				default:
					break;
			}

			return (0);
		}

		// Sliders and up-down controls
		case SLN_NEW_VALUE:
		case UDN_NEW_VALUE:
		{
			// Do nothing if this is the initialisation phase
			if (!fInitialised)
			{
				return (0);
			}

			// If no preset is on display
			if (nDisplayedPreset == INDEX_NONE)
			{
				// Go no further
				return (0);
			}

			// Invoke the handling macros
			switch (id = LOWORD (wParam))
			{
				// Sliders
				case IDC_INSTR1_COARSE_TUNE:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].CoarseTune);
					break;
				case IDC_INSTR1_FINE_TUNE:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].FineTune);
					break;
				case IDC_INSTR1_KEY_TRANSPOSE:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].KeyXpose);
					break;
				case IDC_INSTR1_DOUBLE_DETUNE:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].DoubleDetune);
					break;
				case IDC_INSTR1_SOUND_DELAY:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].SoundDelay);
					break;
				case IDC_INSTR1_SOUND_START:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].SoundStart);
					break;
				case IDC_FILTER1_LEVEL:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].Filter.Level);
					break;
				case IDC_FILTER1_MORPH:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].Filter.Morph);
					break;
				case IDC_FILTER1_FREQ_TRACK:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].Filter.FreqTrack);
					break;
				case IDC_FILTER1_TRANS2:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].Filter.Transform2);
					break;
				case IDC_PORTAMENTO1_RATE:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].PortamentoRate);
					break;
				case IDC_PAN1_POSITION:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].Pan);
					break;
				case IDC_INSTR1_VOLUME:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].Volume);
					break;
				case IDC_ALT_ENV1_ATTACK:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].AltEnv.Attack);
					SendDlgItemMessage (hWnd, IDC_ALT_ENV1_ENVELOPE, ENV_SET_ATTACK, (int)lParam, 0);
					break;
				case IDC_ALT_ENV1_HOLD:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].AltEnv.Hold);
					SendDlgItemMessage (hWnd, IDC_ALT_ENV1_ENVELOPE, ENV_SET_HOLD, (int)lParam, 0);
					break;
				case IDC_ALT_ENV1_DECAY:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].AltEnv.Decay);
					SendDlgItemMessage (hWnd, IDC_ALT_ENV1_ENVELOPE, ENV_SET_DECAY, (int)lParam, 0);
					break;
				case IDC_ALT_ENV1_SUSTAIN:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].AltEnv.Sustain);
					SendDlgItemMessage (hWnd, IDC_ALT_ENV1_ENVELOPE, ENV_SET_SUSTAIN, (int)lParam, 0);
					break;
				case IDC_ALT_ENV1_RELEASE:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].AltEnv.Release);
					SendDlgItemMessage (hWnd, IDC_ALT_ENV1_ENVELOPE, ENV_SET_RELEASE, (int)lParam, 0);
					break;

				case IDC_INSTR2_COARSE_TUNE:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].CoarseTune);
					break;
				case IDC_INSTR2_FINE_TUNE:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].FineTune);
					break;
				case IDC_INSTR2_KEY_TRANSPOSE:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].KeyXpose);
					break;
				case IDC_INSTR2_DOUBLE_DETUNE:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].DoubleDetune);
					break;
				case IDC_INSTR2_SOUND_DELAY:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].SoundDelay);
					break;
				case IDC_INSTR2_SOUND_START:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].SoundStart);
					break;
				case IDC_FILTER2_LEVEL:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].Filter.Level);
					break;
				case IDC_FILTER2_MORPH:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].Filter.Morph);
					break;
				case IDC_FILTER2_FREQ_TRACK:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].Filter.FreqTrack);
					break;
				case IDC_FILTER2_TRANS2:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].Filter.Transform2);
					break;
				case IDC_PORTAMENTO2_RATE:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].PortamentoRate);
					break;
				case IDC_PAN2_POSITION:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].Pan);
					break;
				case IDC_INSTR2_VOLUME:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].Volume);
					break;
				case IDC_ALT_ENV2_ATTACK:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].AltEnv.Attack);
					SendDlgItemMessage (hWnd, IDC_ALT_ENV2_ENVELOPE, ENV_SET_ATTACK, (int)lParam, 0);
					break;
				case IDC_ALT_ENV2_HOLD:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].AltEnv.Hold);
					SendDlgItemMessage (hWnd, IDC_ALT_ENV2_ENVELOPE, ENV_SET_HOLD, (int)lParam, 0);
					break;
				case IDC_ALT_ENV2_DECAY:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].AltEnv.Decay);
					SendDlgItemMessage (hWnd, IDC_ALT_ENV2_ENVELOPE, ENV_SET_DECAY, (int)lParam, 0);
					break;
				case IDC_ALT_ENV2_SUSTAIN:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].AltEnv.Sustain);
					SendDlgItemMessage (hWnd, IDC_ALT_ENV2_ENVELOPE, ENV_SET_SUSTAIN, (int)lParam, 0);
					break;
				case IDC_ALT_ENV2_RELEASE:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].AltEnv.Release);
					SendDlgItemMessage (hWnd, IDC_ALT_ENV2_ENVELOPE, ENV_SET_RELEASE, (int)lParam, 0);
					break;

				case IDC_LFO1_RATE:
					HANDLE_AUCNTRL (id, EditPreset.Lfo[0].Rate);
					break;
				case IDC_LFO1_DELAY:
					HANDLE_AUCNTRL (id, EditPreset.Lfo[0].Delay);
					break;
				case IDC_LFO1_VARIATION:
					HANDLE_AUCNTRL (id, EditPreset.Lfo[0].Variation);
					break;
				case IDC_LFO1_AMOUNT:
					HANDLE_AUCNTRL (id, EditPreset.Lfo[0].Amount);
					break;
				case IDC_FG1_AMOUNT:
					HANDLE_AUCNTRL (id, EditPreset.FunctionGenerator[0].Amount);
					break;
				case IDC_LFO2_RATE:
					HANDLE_AUCNTRL (id, EditPreset.Lfo[1].Rate);
					break;
				case IDC_LFO2_DELAY:
					HANDLE_AUCNTRL (id, EditPreset.Lfo[1].Delay);
					break;
				case IDC_LFO2_VARIATION:
					HANDLE_AUCNTRL (id, EditPreset.Lfo[1].Variation);
					break;
				case IDC_LFO2_AMOUNT:
					HANDLE_AUCNTRL (id, EditPreset.Lfo[1].Amount);
					break;
				case IDC_FG2_AMOUNT:
					HANDLE_AUCNTRL (id, EditPreset.FunctionGenerator[1].Amount);
					break;
				case IDC_KBD_PRESSURE_AMOUNT:
					HANDLE_AUCNTRL (id, EditPreset.PressureAmount);
					break;
				case IDC_KBD_BEND_RANGE:
					HANDLE_AUCNTRL (id, EditPreset.BendRange);
					break;
				case IDC_AUX_ENV_DELAY:
					HANDLE_AUCNTRL (id, EditPreset.AuxEnv.Delay);
					break;
				case IDC_AUX_ENV_ATTACK:
					HANDLE_AUCNTRL (id, EditPreset.AuxEnv.Attack);
					SendDlgItemMessage (hWnd, IDC_AUX_ENV_ENVELOPE, ENV_SET_ATTACK, (int)lParam, 0);
					break;
				case IDC_AUX_ENV_HOLD:
					HANDLE_AUCNTRL (id, EditPreset.AuxEnv.Hold);
					SendDlgItemMessage (hWnd, IDC_AUX_ENV_ENVELOPE, ENV_SET_HOLD, (int)lParam, 0);
					break;
				case IDC_AUX_ENV_DECAY:
					HANDLE_AUCNTRL (id, EditPreset.AuxEnv.Decay);
					SendDlgItemMessage (hWnd, IDC_AUX_ENV_ENVELOPE, ENV_SET_DECAY, (int)lParam, 0);
					break;
				case IDC_AUX_ENV_SUSTAIN:
					HANDLE_AUCNTRL (id, EditPreset.AuxEnv.Sustain);
					SendDlgItemMessage (hWnd, IDC_AUX_ENV_ENVELOPE, ENV_SET_SUSTAIN, (int)lParam, 0);
					break;
				case IDC_AUX_ENV_RELEASE:
					HANDLE_AUCNTRL (id, EditPreset.AuxEnv.Release);
					SendDlgItemMessage (hWnd, IDC_AUX_ENV_ENVELOPE, ENV_SET_RELEASE, (int)lParam, 0);
					break;
				case IDC_AUX_ENV_AMOUNT:
					HANDLE_AUCNTRL (id, EditPreset.AuxEnv.Amount);
					break;

				case IDC_MISC_MIDI_A_AMOUNT:
					HANDLE_AUCNTRL (id, EditPreset.CtrlAAmount);
					break;
				case IDC_MISC_MIDI_B_AMOUNT:
					HANDLE_AUCNTRL (id, EditPreset.CtrlBAmount);
					break;
				case IDC_MISC_MIDI_C_AMOUNT:
					HANDLE_AUCNTRL (id, EditPreset.CtrlCAmount);
					break;
				case IDC_MISC_MIDI_D_AMOUNT:
					HANDLE_AUCNTRL (id, EditPreset.CtrlDAmount);
					break;
				case IDC_XFADE_BALANCE:
					HANDLE_AUCNTRL (id, EditPreset.XFade.Balance);
					break;
				case IDC_XFADE_AMOUNT:
					HANDLE_AUCNTRL (id, EditPreset.XFade.Amount);
					break;

				// Up-down controls
				case IDC_INSTR1_LOW_KEY_CONTROL:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].LowKey);
					break;
				case IDC_INSTR1_HIGH_KEY_CONTROL:
					HANDLE_AUCNTRL (id, EditPreset.Layer[0].HighKey);
					break;

				case IDC_INSTR1_LOOP_START_HIGH:
					LoopStart (0, HIGH_DELTA, (int)lParam);
					break;
				case IDC_INSTR1_LOOP_START_LOW:
					LoopStart (0, LOW_DELTA, (int)lParam);
					break;

				case IDC_INSTR1_LOOP_LENGTH_HIGH:
					LoopLength (0, HIGH_DELTA, (int)lParam);
					break;
				case IDC_INSTR1_LOOP_LENGTH_LOW:
					LoopLength (0, LOW_DELTA, (int)lParam);
					break;

				case IDC_INSTR2_LOW_KEY_CONTROL:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].LowKey);
					break;
				case IDC_INSTR2_HIGH_KEY_CONTROL:
					HANDLE_AUCNTRL (id, EditPreset.Layer[1].HighKey);
					break;

				case IDC_INSTR2_LOOP_START_HIGH:
					LoopStart (1, HIGH_DELTA, (int)lParam);
					break;
				case IDC_INSTR2_LOOP_START_LOW:
					LoopStart (1, LOW_DELTA, (int)lParam);
					break;

				case IDC_INSTR2_LOOP_LENGTH_HIGH:
					LoopLength (1, HIGH_DELTA, (int)lParam);
					break;
				case IDC_INSTR2_LOOP_LENGTH_LOW:
					LoopLength (1, LOW_DELTA, (int)lParam);
					break;

				case IDC_XFADE_SWITCH_POINT_CONTROL:
					HANDLE_AUCNTRL (id, EditPreset.XFade.SwitchPoint);
					break;
				case IDC_KBD_LOW_KEY:
					HANDLE_AUCNTRL (id, EditPreset.LowKey);
					break;
				case IDC_KBD_HIGH_KEY:
					HANDLE_AUCNTRL (id, EditPreset.HighKey);
					break;
				case IDC_KBD_CENTRE:
					HANDLE_AUCNTRL (id, EditPreset.KeyboardCentre);
					break;

				default:
					break;
			}

				return (0);
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

			if ((hBrush = Ctl3dCtlColorEx (nMsg, wParam, lParam)) != (HBRUSH)FALSE)
			{
				return ((LRESULT)hBrush);
			}
			break;
		}

		// Right mouse button down
		case WM_RBUTTONDOWN:
		{
			// Call the handler
			PresetRightMouseButton (hWnd, LOWORD (lParam), HIWORD (lParam));
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
 Function:		PresetSendParam
 Parameters:	HWND	hwndParent		Parent window.
				LPVOID	lpVar			Address of variable within current
										preset structure.
 Returns:		None.
 Description:	Sends a single parameter to the Morpheus.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 7/5/94 	| Created.											| PW
*****************************************************************************/

VOID PresetSendParam (HWND hwndParent, LPVOID lpVar)
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
		start of the preset (param #0 is &EditPreset.Name[0]).
	 */
	wParamNumber = MPARAM_PRESET_BASE + (lpwVar - &EditPreset.Name[0]);

	// Set its value
	MorpheusSetParameter (hwndParent, (int)wParamNumber, ReadSysexWord (lpwVar));
}

/*^L*/
/*****************************************************************************
 Function:		NoteOnPatchDefDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
                LPARAM	lParam
 Returns:		BOOL					TRUE if message processed.
 Description:	The note-on patch editing dialog.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 10/5/94 	| Created.											| PW
*****************************************************************************/

DIALOG_PROC NoteOnPatchDefDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	int							i, id;
	static int					nIndex, nAmount;
	int							nSource, nDest;
	char						szListEntry[100], szPatchBuf[200];
	static BOOL					fInitialised;

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
			nIndex	= (int)lParam;
			nSource	= ReadSysexWord (&EditPreset.NoteOnModulation[nIndex].Source);
			nDest	= ReadSysexWord (&EditPreset.NoteOnModulation[nIndex].Destination);
			nAmount	= ReadSysexWord (&EditPreset.NoteOnModulation[nIndex].Amount);

			// Set the dialog's title
			wsprintf (&szListEntry[0], ResourceString (IDS_PR_NO_PATCH_WND_TITLE), nIndex + 1);
			SetWindowText (hWnd, (LPCSTR)&szListEntry[0]);

			// Fill in the SOURCE list
			for (i=0 ; i<ARRAY_SIZE(pszNoteOnSources) ; i++)
			{
				SendDlgItemMessage (hWnd, IDC_SOURCE, CB_ADDSTRING, 0, (LPARAM)pszNoteOnSources[i]);
			}

			// For each possible destination
			for (i=0 ; i<ARRAY_SIZE(pszPatchDests) ; i++)
			{
				// If this destination is permitted for note-on patches
				if ((pszPatchDests[i][0] == 'N') || (pszPatchDests[i][0] == 'B'))
				{
                	// Put it in the list
					SendDlgItemMessage (hWnd, IDC_DESTINATION, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)&pszPatchDests[i][1]);
				}
			}

			// Select the current SOURCE
			SendDlgItemMessage (hWnd, IDC_SOURCE, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)(LPCSTR)pszNoteOnSources[nSource]);

			// Select the current DESTINATION
			SendDlgItemMessage (hWnd, IDC_DESTINATION, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)(LPCSTR)&pszPatchDests[nDest][1]);

			// Set the AMOUNT slider
			SendDlgItemMessage (hWnd, IDC_AMOUNT, SL_SET_MIN, (WPARAM)-128, 0);
			SendDlgItemMessage (hWnd, IDC_AMOUNT, SL_SET_MAX, 127, 0);
			SendDlgItemMessage (hWnd, IDC_AMOUNT, SL_SET_CURRENT, nAmount, 0);

			// Initialised
			fInitialised = TRUE;

			return (TRUE);
		}

		// Slider
		case SLN_NEW_VALUE:
		{
			// Don't process until fully initialised
			if (!fInitialised)
			{
            	return (TRUE);
			}

			// Make sure it's the AMOUNT slider
			if (LOWORD (wParam) == IDC_AMOUNT)
			{
				HANDLE_AUCNTRL (id, EditPreset.NoteOnModulation[nIndex].Amount);
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
					// Source
					case IDC_SOURCE:
					{
						// Handle as usual
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.NoteOnModulation[nIndex].Source, i);
						break;
					}

					// Destination
					case IDC_DESTINATION:
					{
						// Get the DESTINATION string back
						i = SendDlgItemMessage (hWnd, IDC_DESTINATION, CB_GETCURSEL, 0, 0);
						SendDlgItemMessage (hWnd, IDC_DESTINATION, CB_GETLBTEXT, i, (LPARAM)(LPSTR)&szListEntry[0]);

						// For each possible destination
						for (nDest=0, i=0 ; i<ARRAY_SIZE(pszPatchDests) ; i++)
						{
							// If the selected entry matches it
							if (!lstrcmp (&szListEntry[0], &pszPatchDests[i][1]))
							{
								// Stop searching
								nDest = i;
								break;
							}
						}

						// If the DESTINATION has changed
						if (nDest != ReadSysexWord (&EditPreset.NoteOnModulation[nIndex].Destination))
						{
							// Write the DESTINATION back
							WriteSysexWord (&EditPreset.NoteOnModulation[nIndex].Destination, nDest);

							// Send to the Morpheus
							PresetSendParam (hWnd, &EditPreset.NoteOnModulation[nIndex].Destination);
						}

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
					FormatPatchLine (NO, nIndex, &szPatchBuf[0]);
					SendDlgItemMessage (hwndCurrentPreset, IDC_MISC_NOTE_ON_PATCH_LISTBOX, LB_DELETESTRING, nIndex, 0);
					SendDlgItemMessage (hwndCurrentPreset, IDC_MISC_NOTE_ON_PATCH_LISTBOX,
										LB_INSERTSTRING, nIndex, (LPARAM)(LPCSTR)&szPatchBuf[0]);
					SendDlgItemMessage (hwndCurrentPreset, IDC_MISC_NOTE_ON_PATCH_LISTBOX, LB_SETCURSEL, nIndex, 0);

					// Force a redraw of the patchbay
					DisplayNoteOnPatch ();

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
 Function:		RealTimePatchDefDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
                LPARAM	lParam
 Returns:		BOOL					TRUE if message processed.
 Description:	The real-time patch editing dialog.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 10/5/94 	| Created.											| PW
*****************************************************************************/

DIALOG_PROC RealTimePatchDefDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	int				i, id;
	static int		nIndex, nAmount;
	int				nSource, nDest;
	char			szListEntry[100], szPatchBuf[200];
	static BOOL		fInitialised;

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
			nIndex	= (int)lParam;
			nSource	= ReadSysexWord (&EditPreset.RealTimeModulation[nIndex].Source);
			nDest	= ReadSysexWord (&EditPreset.RealTimeModulation[nIndex].Destination);
			nAmount	= ReadSysexWord (&EditPreset.RealTimeModulation[nIndex].Amount);

			// Set the dialog's title
			wsprintf (&szListEntry[0], ResourceString (IDS_PR_RT_PATCH_WND_TITLE), nIndex + 1);
			SetWindowText (hWnd, (LPCSTR)&szListEntry[0]);

			// Fill in the SOURCE list
			for (i=0 ; i<ARRAY_SIZE(pszRealTimeSources) ; i++)
			{
				SendDlgItemMessage (hWnd, IDC_SOURCE, CB_ADDSTRING, 0, (LPARAM)pszRealTimeSources[i]);
			}

			// For each possible destination
			for (i=0 ; i<ARRAY_SIZE(pszPatchDests) ; i++)
			{
				// If this destination is permitted for note-on patches
				if ((pszPatchDests[i][0] == 'R') || (pszPatchDests[i][0] == 'B'))
				{
					// Put it in the list
					SendDlgItemMessage (hWnd, IDC_DESTINATION, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)&pszPatchDests[i][1]);
				}
			}

			// Select the current SOURCE
			SendDlgItemMessage (hWnd, IDC_SOURCE, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)(LPCSTR)pszRealTimeSources[nSource]);

			// Select the current DESTINATION
			SendDlgItemMessage (hWnd, IDC_DESTINATION, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)(LPCSTR)&pszPatchDests[nDest][1]);

			// Set the AMOUNT slider
			SendDlgItemMessage (hWnd, IDC_AMOUNT, SL_SET_MIN, (WPARAM)-128, 0);
			SendDlgItemMessage (hWnd, IDC_AMOUNT, SL_SET_MAX, 127, 0);
			SendDlgItemMessage (hWnd, IDC_AMOUNT, SL_SET_CURRENT, nAmount, 0);

			// Initialised
			fInitialised = TRUE;

			return (TRUE);
		}

		// Slider
		case SLN_NEW_VALUE:
		{
			// Don't process until fully initialised
			if (!fInitialised)
			{
				return (TRUE);
			}

			// Make sure it's the AMOUNT slider
			if (LOWORD (wParam) == IDC_AMOUNT)
			{
				HANDLE_AUCNTRL (id, EditPreset.RealTimeModulation[nIndex].Amount);
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
					// Source
					case IDC_SOURCE:
					{
						// Handle as usual
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.RealTimeModulation[nIndex].Source, i);
						break;
					}

					// Destination
					case IDC_DESTINATION:
					{
						// Get the DESTINATION string back
						i = SendDlgItemMessage (hWnd, IDC_DESTINATION, CB_GETCURSEL, 0, 0);
						SendDlgItemMessage (hWnd, IDC_DESTINATION, CB_GETLBTEXT, i, (LPARAM)(LPSTR)&szListEntry[0]);

						// For each possible destination
						for (nDest=0, i=0 ; i<ARRAY_SIZE(pszPatchDests) ; i++)
						{
							// If the selected entry matches it
							if (!lstrcmp (&szListEntry[0], &pszPatchDests[i][1]))
							{
								// Stop searching
								nDest = i;
								break;
							}
						}

						// If the DESTINATION has changed
						if (nDest != ReadSysexWord (&EditPreset.RealTimeModulation[nIndex].Destination))
						{
							// Write the DESTINATION back
							WriteSysexWord (&EditPreset.RealTimeModulation[nIndex].Destination, nDest);

							// Send to the Morpheus
							PresetSendParam (hWnd, &EditPreset.RealTimeModulation[nIndex].Destination);
						}

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
					FormatPatchLine (RT, nIndex, &szPatchBuf[0]);
					SendDlgItemMessage (hwndCurrentPreset, IDC_MISC_REAL_TIME_PATCH_LISTBOX, LB_DELETESTRING, nIndex, 0);
					SendDlgItemMessage (hwndCurrentPreset, IDC_MISC_REAL_TIME_PATCH_LISTBOX,
										LB_INSERTSTRING, nIndex, (LPARAM)(LPCSTR)&szPatchBuf[0]);
					SendDlgItemMessage (hwndCurrentPreset, IDC_MISC_REAL_TIME_PATCH_LISTBOX, LB_SETCURSEL, nIndex, 0);

					// Force a redraw of the patchbay
					DisplayRealTimePatch ();

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
 Function:		FuncGenDefDialogProc
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

DIALOG_PROC FuncGenDefDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	int							i, id;
	static PFG_EDIT_PARAM		pEditParams;
	static int					nFG, nStep, nLevel, nLevelType, nTime, nShape, nCondJump, nCondVal, nCondSeg;
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
			pEditParams = (PFG_EDIT_PARAM)lParam;
			nFG			= pEditParams->nFG;
			nStep		= pEditParams->nStep;
			nLevel		= ReadSysexWord (&EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].Level);
			nTime		= ReadSysexWord (&EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].Time);
			nShape		= ReadSysexWord (&EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].Shape);
			nCondJump	= ReadSysexWord (&EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].CondJump);
			nCondVal	= ReadSysexWord (&EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].CondValue);
			nCondSeg	= ReadSysexWord (&EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].DestSegment);

			// Set the dialog's title
			wsprintf (&szListEntry[0], ResourceString (IDS_PR_FG_STEP_WND_TITLE), nFG + 1, nStep + 1);
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
					WriteSysexWord (&EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].Level, i);
					PresetSendParam (hWnd, &EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].Level);

					break;
				}
				// Time
				case IDC_FG_TIME:
				{
					HANDLE_AUCNTRL (id, EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].Time);
					break;
				}
				// Conditional value
				case IDC_FG_COND_VAL:
				{
					HANDLE_AUCNTRL (id, EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].CondValue);
					break;
				}
				// Destination segment
				case IDC_FG_COND_SEG:
				{
					lParam--;			// Because the control thinks it's in the range 1..8
					HANDLE_AUCNTRL (id, EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].DestSegment);
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
						HANDLE_COMBOBOX (id, EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].Shape, i);
						break;
					}

					// Jump condition
					case IDC_FG_COND_JUMP:
					{
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						HANDLE_COMBOBOX (id, EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].CondJump, i);
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
						HWND	hwndListBox;

						// Update the parent's FG listbox
						hwndListBox = GetDlgItem (hwndCurrentPreset, (nFG == 0) ? IDC_FG1_LISTBOX : IDC_FG2_LISTBOX);
						DisplayFuncGenList (hwndListBox, &EditPreset.FunctionGenerator[nFG].Segments);
						SendMessage (hwndListBox, LB_SETCURSEL, nStep, 0);

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
								WriteSysexWord (&EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].Level, i);
								PresetSendParam (hWnd, &EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].Level);
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
								WriteSysexWord (&EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].Level, i);
								PresetSendParam (hWnd, &EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].Level);
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
								WriteSysexWord (&EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].Level, i);
								PresetSendParam (hWnd, &EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].Level);
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
								WriteSysexWord (&EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].Level, i);
								PresetSendParam (hWnd, &EditPreset.FunctionGenerator[nFG].Segments.Segment[nStep].Level);
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

static VOID LoopStart (int nLayer, VALTYPE valtype, LONG nVal)
{
	LONG			lVal;
	int				nHigh, nLow;

	// Get the current MS and LS
	nHigh	= WNUM (EditPreset.Layer[nLayer].Loop.StartMS);
	nLow	= WNUM (EditPreset.Layer[nLayer].Loop.StartLS);

	// Fetch the current loop start as a long
	lVal = (LONG)nHigh * 1000L + (LONG)nLow;

	// If we're modifying MS
	if (valtype == HIGH_DELTA)
	{
		// Increment or decrement MS
		if (nVal == 1)
		{
			// Increment
			lVal += 1000L;
		}
		else
		{
			// Decrement
			lVal -= 1000L;
		}
		SendDlgItemMessage (hwndCurrentPreset, nLayer == 0 ? IDC_INSTR1_LOOP_START_HIGH : IDC_INSTR2_LOOP_START_HIGH,
							UD_SET_CURRENT, 0, FALSE);
	}
	// Else if we're modifying LS
	else if (valtype == LOW_DELTA)
	{
		// Increment or decrement LS
		if (nVal == 1)
		{
			// Increment
			lVal++;
		}
		else
		{
			// Decrement
			lVal--;
		}
		SendDlgItemMessage (hwndCurrentPreset, nLayer == 0 ? IDC_INSTR1_LOOP_START_LOW : IDC_INSTR2_LOOP_START_LOW,
							UD_SET_CURRENT, 0, FALSE);
	}
	// Else if we're directly setting the value
	else if (valtype == BOTH_ABS)
	{
		// Use lval directly
		lVal = nVal;
	}
#ifdef NEVER
	// Else if we're directly setting MS
	else if (valtype == HIGH_ABS)
	{
		// Calculate lval directly
		if (nVal < 0)
			lVal = (LONG)nVal * 1000L - (LONG)nLow;
		else
			lVal = (LONG)nVal * 1000L + (LONG)nLow;
	}
	// Else if we're directly setting LS
	else if (valtype == LOW_ABS)
	{
		// Calculate lval directly
		if (nHigh < 0)
			lVal = (LONG)nHigh * 1000L - (LONG)nVal;
		else
			lVal = (LONG)nHigh * 1000L + (LONG)nVal;
	}
#endif
	else
	{
		// ???
		return;
    }

	// Clip the value
	if (lVal > 999999L)
	{
    	lVal = 999999L;
	}
	else if (lVal < -999999L)
	{
		lVal = -999999L;
	}

	// Recalculate MS and LS
	nHigh = lVal / 1000L;
	nLow  = lVal % 1000L;

	// Write MS back to memory
	WriteSysexWord (&EditPreset.Layer[nLayer].Loop.StartMS, nHigh);
	PresetSendParam (hwndCurrentPreset, &EditPreset.Layer[nLayer].Loop.StartMS);

	// Write LS back to memory
	WriteSysexWord (&EditPreset.Layer[nLayer].Loop.StartLS, nLow);
	PresetSendParam (hwndCurrentPreset, &EditPreset.Layer[nLayer].Loop.StartLS);

	// Format and output
	DisplayPresetLoopStart (nLayer);

	// Mark as dirty
	fDirtyPreset = TRUE;
}

static VOID LoopLength (int nLayer, VALTYPE valtype, LONG nVal)
{
	LONG			lVal;
	int				nHigh, nLow;

	// Get the current MS and LS
	nHigh	= WNUM (EditPreset.Layer[nLayer].Loop.SizeOffsetMS);
	nLow	= WNUM (EditPreset.Layer[nLayer].Loop.SizeOffsetLS);

	// Fetch the current loop start as a long
	lVal = (LONG)nHigh * 1000L + (LONG)nLow;

	// If we're modifying MS
	if (valtype == HIGH_DELTA)
	{
		// Increment or decrement MS
		if (nVal == 1)
		{
			// Increment
			lVal += 1000L;
		}
		else
		{
			// Decrement
			lVal -= 1000L;
		}
		SendDlgItemMessage (hwndCurrentPreset, nLayer == 0 ? IDC_INSTR1_LOOP_LENGTH_HIGH : IDC_INSTR2_LOOP_LENGTH_HIGH,
							UD_SET_CURRENT, 0, FALSE);
	}
	// Else if we're modifying LS
	else if (valtype == LOW_DELTA)
	{
		// Increment or decrement LS
		if (nVal == 1)
		{
			// Increment
			lVal++;
		}
		else
		{
			// Decrement
			lVal--;
		}
		SendDlgItemMessage (hwndCurrentPreset, nLayer == 0 ? IDC_INSTR1_LOOP_LENGTH_LOW : IDC_INSTR2_LOOP_LENGTH_LOW,
							UD_SET_CURRENT, 0, FALSE);
	}
	// Else if we're directly setting the value
	else if (valtype == BOTH_ABS)
	{
		// Use lval directly
		lVal = nVal;
	}
#ifdef NEVER
	// Else if we're directly setting MS
	else if (valtype == HIGH_ABS)
	{
		// Calculate lval directly
		if (nVal < 0)
			lVal = (LONG)nVal * 1000L - (LONG)nLow;
		else
	        lVal = (LONG)nVal * 1000L + (LONG)nLow;
	}
	// Else if we're directly setting LS
	else if (valtype == LOW_ABS)
	{
		// Calculate lval directly
        if (nHigh < 0)
			lVal = (LONG)nHigh * 1000L - (LONG)nVal;
		else
			lVal = (LONG)nHigh * 1000L + (LONG)nVal;
	}
#endif
	else
	{
		// ???
		return;
    }

	// Clip the value
	if (lVal > 999999L)
	{
    	lVal = 999999L;
	}
	else if (lVal < -999999L)
	{
		lVal = -999999L;
	}

	// Recalculate MS and LS
	nHigh = lVal / 1000L;
	nLow  = lVal % 1000L;

	// Write MS back to memory
	WriteSysexWord (&EditPreset.Layer[nLayer].Loop.SizeOffsetMS, nHigh);
	PresetSendParam (hwndCurrentPreset, &EditPreset.Layer[nLayer].Loop.SizeOffsetMS);

	// Write LS back to memory
	WriteSysexWord (&EditPreset.Layer[nLayer].Loop.SizeOffsetLS, nLow);
	PresetSendParam (hwndCurrentPreset, &EditPreset.Layer[nLayer].Loop.SizeOffsetLS);

	// Format and output
	DisplayPresetLoopLength (nLayer);

	// Mark as dirty
	fDirtyPreset = TRUE;
}

