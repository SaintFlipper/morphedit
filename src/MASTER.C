/*****************************************************************************
 File:			master.c
 Description:	The master settings dialog handler.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 14/6/94	| Created.											| PW
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <windowsx.h>
#include <ctl3d.h>

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define	SENDMP(parm,val)							\
	if (fConnectedToMorpheus)						\
	{												\
		MorpheusSetParameter (hWnd, parm, val);		\
	}


/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

static LPCSTR	lpszVelocityCurves[] =
	{"Off", "Curve 1", "Curve 2", "Curve 3", "Curve 4", "Curve 5", "Curve 6", "Curve 7", "Curve 8"};
static LPCSTR	lpszMidiModes[] =
	{"Omni", "Poly", "Multi", "Mono"};
static struct
{
	int		idProgram;
	int		idVolume;
	int		idPan;
	int		idVolumeLabel;
	int		idPanLabel;
} idChannelProgram[16] =
{
	{IDC_MASTER_CHAN1_PROGRAM,	IDC_MASTER_CHAN1_VOLUME,	IDC_MASTER_CHAN1_PAN,	IDC_MASTER_CHAN1_VOLUME_TEXT,	IDC_MASTER_CHAN1_PAN_TEXT},
	{IDC_MASTER_CHAN2_PROGRAM,	IDC_MASTER_CHAN2_VOLUME,	IDC_MASTER_CHAN2_PAN,	IDC_MASTER_CHAN2_VOLUME_TEXT,	IDC_MASTER_CHAN2_PAN_TEXT},
	{IDC_MASTER_CHAN3_PROGRAM,	IDC_MASTER_CHAN3_VOLUME,	IDC_MASTER_CHAN3_PAN,	IDC_MASTER_CHAN3_VOLUME_TEXT,	IDC_MASTER_CHAN3_PAN_TEXT},
	{IDC_MASTER_CHAN4_PROGRAM,	IDC_MASTER_CHAN4_VOLUME,	IDC_MASTER_CHAN4_PAN,	IDC_MASTER_CHAN4_VOLUME_TEXT,	IDC_MASTER_CHAN4_PAN_TEXT},
	{IDC_MASTER_CHAN5_PROGRAM,	IDC_MASTER_CHAN5_VOLUME,	IDC_MASTER_CHAN5_PAN,	IDC_MASTER_CHAN5_VOLUME_TEXT,	IDC_MASTER_CHAN5_PAN_TEXT},
	{IDC_MASTER_CHAN6_PROGRAM,	IDC_MASTER_CHAN6_VOLUME,	IDC_MASTER_CHAN6_PAN,	IDC_MASTER_CHAN6_VOLUME_TEXT,	IDC_MASTER_CHAN6_PAN_TEXT},
	{IDC_MASTER_CHAN7_PROGRAM,	IDC_MASTER_CHAN7_VOLUME,	IDC_MASTER_CHAN7_PAN,	IDC_MASTER_CHAN7_VOLUME_TEXT,	IDC_MASTER_CHAN7_PAN_TEXT},
	{IDC_MASTER_CHAN8_PROGRAM,	IDC_MASTER_CHAN8_VOLUME,	IDC_MASTER_CHAN8_PAN,	IDC_MASTER_CHAN8_VOLUME_TEXT,	IDC_MASTER_CHAN8_PAN_TEXT},
	{IDC_MASTER_CHAN9_PROGRAM,	IDC_MASTER_CHAN9_VOLUME,	IDC_MASTER_CHAN9_PAN,	IDC_MASTER_CHAN9_VOLUME_TEXT,	IDC_MASTER_CHAN9_PAN_TEXT},
	{IDC_MASTER_CHAN10_PROGRAM,	IDC_MASTER_CHAN10_VOLUME,	IDC_MASTER_CHAN10_PAN,	IDC_MASTER_CHAN10_VOLUME_TEXT,	IDC_MASTER_CHAN10_PAN_TEXT},
	{IDC_MASTER_CHAN11_PROGRAM,	IDC_MASTER_CHAN11_VOLUME,	IDC_MASTER_CHAN11_PAN,	IDC_MASTER_CHAN11_VOLUME_TEXT,	IDC_MASTER_CHAN11_PAN_TEXT},
	{IDC_MASTER_CHAN12_PROGRAM,	IDC_MASTER_CHAN12_VOLUME,	IDC_MASTER_CHAN12_PAN,	IDC_MASTER_CHAN12_VOLUME_TEXT,	IDC_MASTER_CHAN12_PAN_TEXT},
	{IDC_MASTER_CHAN13_PROGRAM,	IDC_MASTER_CHAN13_VOLUME,	IDC_MASTER_CHAN13_PAN,	IDC_MASTER_CHAN13_VOLUME_TEXT,	IDC_MASTER_CHAN13_PAN_TEXT},
	{IDC_MASTER_CHAN14_PROGRAM,	IDC_MASTER_CHAN14_VOLUME,	IDC_MASTER_CHAN14_PAN,	IDC_MASTER_CHAN14_VOLUME_TEXT,	IDC_MASTER_CHAN14_PAN_TEXT},
	{IDC_MASTER_CHAN15_PROGRAM,	IDC_MASTER_CHAN15_VOLUME,	IDC_MASTER_CHAN15_PAN,	IDC_MASTER_CHAN15_VOLUME_TEXT,	IDC_MASTER_CHAN15_PAN_TEXT},
	{IDC_MASTER_CHAN16_PROGRAM,	IDC_MASTER_CHAN16_VOLUME,	IDC_MASTER_CHAN16_PAN,	IDC_MASTER_CHAN16_VOLUME_TEXT,	IDC_MASTER_CHAN16_PAN_TEXT},
};

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

static VOID InitMasterDialog (HWND hWnd);
static int FindMasterParam (int idParam);
static int FindMidimapParam (int idParam);
static VOID GetScratchChannelParams (int nChan, int *pnProgram, int *pnVolume, int *pnPan);
static VOID ChannelChange (HWND hWnd, WPARAM wParam, LPARAM lParam);

/*^L*/
/*****************************************************************************
 Function:		DisplayMasterSettings
 Parameters:	None.
 Returns:		None.
 Description:	Displays the master settings.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 14/6/94	| Created.											| PW
*****************************************************************************/

VOID DisplayMasterSettings (VOID)
{
	UINT	i;
	int		nProgram, nVolume, nPan;

	// If the dialog isn't displayed, or the master settings aren't loaded
	if ((hwndMaster == (HWND)NULL) || (pMasterSettings == NULL))
	{
		return;
	}

	// Set the master tuning slider
	SendDlgItemMessage (hwndMaster, IDC_MASTER_TUNE, SL_SET_CURRENT, FindMasterParam (MPARAM_MASTER_TUNE), 0);

	// Set the master transpose slider
	SendDlgItemMessage (hwndMaster, IDC_MASTER_TRANSPOSE, SL_SET_CURRENT, FindMasterParam (MPARAM_MASTER_TRANSPOSE), 0);

	// Set the bend range slider
	SendDlgItemMessage (hwndMaster, IDC_MASTER_BEND_RANGE, SL_SET_CURRENT, FindMasterParam (MPARAM_GLOBAL_BEND), 0);

	// Set the velocity curve combobox
	SendDlgItemMessage (hwndMaster, IDC_MASTER_VELOCITY_CURVE, CB_SETCURSEL, FindMasterParam (MPARAM_GLOBAL_VELCURVE), 0);

	// Set the MIDI mode combobox
	SendDlgItemMessage (hwndMaster, IDC_MASTER_MIDI_MODE, CB_SETCURSEL, FindMasterParam (MPARAM_MIDI_MODE), 0);

	// Set the MIDI ID up-down control
	SendDlgItemMessage (hwndMaster, IDC_MASTER_MIDI_ID, UD_SET_CURRENT, FindMasterParam (MPARAM_SYSEX_DEVICE_ID), 0);

	// Set the MIDI mode change enable checkbox
	CheckDlgButton (hwndMaster, IDC_MASTER_MIDI_MODE_CHANGE, FindMasterParam (MPARAM_MIDI_MODECHANGE_ENABLE));

	// Initialise the MIDI controller A up-down control
	SendDlgItemMessage (hwndMaster, IDC_MASTER_CONTROLLER_A, UD_SET_CURRENT, FindMasterParam (MPARAM_CONTROL_A), 0);

	// Initialise the MIDI controller B up-down control
	SendDlgItemMessage (hwndMaster, IDC_MASTER_CONTROLLER_B, UD_SET_CURRENT, FindMasterParam (MPARAM_CONTROL_B), 0);

	// Initialise the MIDI controller C up-down control
	SendDlgItemMessage (hwndMaster, IDC_MASTER_CONTROLLER_C, UD_SET_CURRENT, FindMasterParam (MPARAM_CONTROL_C), 0);

	// Initialise the MIDI controller D up-down control
	SendDlgItemMessage (hwndMaster, IDC_MASTER_CONTROLLER_D, UD_SET_CURRENT, FindMasterParam (MPARAM_CONTROL_D), 0);

	// Initialise the footswitch 1 up-down control
	SendDlgItemMessage (hwndMaster, IDC_MASTER_FOOTSWITCH_1, UD_SET_CURRENT, FindMasterParam (MPARAM_FOOTSWITCH_1), 0);

	// Initialise the footswitch 2 up-down control
	SendDlgItemMessage (hwndMaster, IDC_MASTER_FOOTSWITCH_2, UD_SET_CURRENT, FindMasterParam (MPARAM_FOOTSWITCH_2), 0);

	// Initialise the footswitch 3 up-down control
	SendDlgItemMessage (hwndMaster, IDC_MASTER_FOOTSWITCH_3, UD_SET_CURRENT, FindMasterParam (MPARAM_FOOTSWITCH_3), 0);

	// For each MIDI channel
	for (i=0 ; i<ARRAY_SIZE(idChannelProgram) ; i++)
	{
		// Get the channel's scratch midimap parameters
		GetScratchChannelParams (i, &nProgram, &nVolume, &nPan);

		// Set the program number
		SendDlgItemMessage (hwndMaster, idChannelProgram[i].idProgram, CB_SETCURSEL, nProgram, 0);

		// Set the volume
		SendDlgItemMessage (hwndMaster, idChannelProgram[i].idVolume, UD_SET_CURRENT, nVolume, 0);

		// Set the pan
		SendDlgItemMessage (hwndMaster, idChannelProgram[i].idPan, UD_SET_CURRENT, nPan, 0);
	}
}

/*^L*/
/*****************************************************************************
 Function:		CreateMasterWindow
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Creates the master settings dialog.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 14/6/94	| Created.											| PW
*****************************************************************************/

BOOL CreateMasterWindow (VOID)
{
	MDICREATESTRUCT	mc;
	WINDOWPLACEMENT	wp;

	// If not connected to the Morpheus
	if (!fConnectedToMorpheus)
	{
		MyMessageBox (hwndMain,
					ResourceString (IDS_MSTR_NOT_CONNECTED),
					ResourceString (IDS_ERROR_TITLE), MB_ICONINFORMATION | MB_OK);
		return (FALSE);
	}

	// Get the master settings from the Morpheus
	if (!MorpheusGetMasterSettings ())
	{
		MyMessageBox (hwndMain,
					ResourceString (IDS_MSTR_ERROR_READING),
					ResourceString (IDS_ERROR_TITLE), MB_ICONINFORMATION | MB_OK);
		return (FALSE);
	}

	// Create the window
	GetCreatePosition (MASTER_SETTINGS_WINDOW, FALSE, &wp);
	mc.szClass	= WCNAME_MASTER;
	mc.szTitle	= "";
	mc.hOwner	= hInst;
	mc.x		= wp.rcNormalPosition.left;
	mc.y		= wp.rcNormalPosition.top;
	mc.cx		= wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	mc.cy		= wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	mc.style	= WS_CHILD | WS_CAPTION | WS_CLIPSIBLINGS | WS_MINIMIZEBOX | WS_SYSMENU |
					(wp.showCmd & SW_SHOWMINIMIZED ? WS_MINIMIZE : 0);
	mc.lParam	= 0;
	if ((hwndMaster = (HWND)SendMessage (hwndClient,
									WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT)&mc)) == (HWND)NULL)
	{
		// Error
		MyMessageBox (NULL, ResourceString (IDS_MSTR_ERROR_CREATING_WND), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Reposition the window
	GetWindowPosition (MASTER_SETTINGS_WINDOW, hwndMaster, FALSE);

	// Display the master settings
	DisplayMasterSettings ();

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		DestroyMasterWindow
 Parameters:	None.
 Returns:		None.
 Description:	Destroys the master settings dialog.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 14/6/94	| Created.											| PW
*****************************************************************************/

VOID DestroyMasterWindow (VOID)
{
	// Save the window position
	SaveWindowPosition (MASTER_SETTINGS_WINDOW, hwndMaster);

	// Destroy the window
	SendMessage (hwndClient, WM_MDIDESTROY, (WPARAM)hwndMaster, 0);
	hwndMaster = (HWND)NULL;
}

/*^L*/
/*****************************************************************************
 Function:		MasterDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if processed, else FALSE.
 Description:	The master settings dialog.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 14/6/94	| Created.											| PW
*****************************************************************************/

WINDOW_PROC MasterDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	int			i;
	int			id;
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
			dwVal = DialogInWindow (hWnd, "DLG_MASTER_SETTINGS");
			nDialogWidth	= HIWORD (dwVal);
			nDialogHeight	= LOWORD (dwVal);

			// Reformat myself
			ReformatDialog (hWnd, RDF_THINFONT);

			// Initialise my controls
			InitMasterDialog (hWnd);

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
			// If it's a BUTTON
			if (NCODE == BN_CLICKED)
			{
				// Invoke the handling macros
				switch (id = LOWORD (wParam))
				{
					case IDC_MASTER_MIDI_MODE_CHANGE:
						SENDMP (MPARAM_MIDI_MODECHANGE_ENABLE, IsDlgButtonChecked (hWnd, id));
						break;

					default:
						break;
            	}
			}
			// If it's a new COMBOBOX selection
			else if (NCODE == CBN_SELCHANGE)
			{
				// Invoke the handling macros
				switch (id = LOWORD (wParam))
				{
					case IDC_MASTER_VELOCITY_CURVE:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						SENDMP (MPARAM_GLOBAL_VELCURVE, i);
						break;

					case IDC_MASTER_MIDI_MODE:
						i = (int)SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
						SENDMP (MPARAM_MIDI_MODE, i);
						break;

					default:
						// Probably a channel change
						ChannelChange (hWnd, wParam, lParam);
						break;
            	}
			}

			return (0);
		}

		// Slider or up-down selection
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
				case IDC_MASTER_TUNE:
					SENDMP (MPARAM_MASTER_TUNE, (int)lParam);
					break;
				case IDC_MASTER_TRANSPOSE:
					SENDMP (MPARAM_MASTER_TRANSPOSE, (int)lParam);
					break;
				case IDC_MASTER_BEND_RANGE:
					SENDMP (MPARAM_GLOBAL_BEND, (int)lParam);
					break;

				// Up-down controls
				case IDC_MASTER_MIDI_ID:
					SENDMP (MPARAM_SYSEX_DEVICE_ID, (int)lParam);
					break;
				case IDC_MASTER_CONTROLLER_A:
					SENDMP (MPARAM_CONTROL_A, (int)lParam);
					break;
				case IDC_MASTER_CONTROLLER_B:
					SENDMP (MPARAM_CONTROL_B, (int)lParam);
					break;
				case IDC_MASTER_CONTROLLER_C:
					SENDMP (MPARAM_CONTROL_C, (int)lParam);
					break;
				case IDC_MASTER_CONTROLLER_D:
					SENDMP (MPARAM_CONTROL_D, (int)lParam);
					break;
				case IDC_MASTER_FOOTSWITCH_1:
					SENDMP (MPARAM_FOOTSWITCH_1, (int)lParam);
					break;
				case IDC_MASTER_FOOTSWITCH_2:
					SENDMP (MPARAM_FOOTSWITCH_2, (int)lParam);
					break;
				case IDC_MASTER_FOOTSWITCH_3:
					SENDMP (MPARAM_FOOTSWITCH_3, (int)lParam);
					break;

				default:
					// Probably a channel change
					ChannelChange (hWnd, wParam, lParam);
					break;
			}

			return (0);
		}

		// Control about to be drawn
		case WM_CTLCOLOR:
		{
			HBRUSH	hBrush;

			hBrush = Ctl3dCtlColorEx (nMsg, wParam, lParam);
			if (hBrush != (HBRUSH)FALSE)
			{
				return ((LPARAM)hBrush);
			}
			break;
		}

		// Close
		case WM_CLOSE:
		{
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
 Function:		InitMasterDialog
 Parameters:	HWND	hWnd			Dialog window handle.
 Returns:		None.
 Description:	Initialises the master settings dialog.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 14/6/94	| Created.											| PW
*****************************************************************************/

static VOID InitMasterDialog (HWND hWnd)
{
	UINT	i, n, NumPrograms;
	char	szListEntry[100];

	// Set up the master tuning slider
	InitSlider (hWnd, IDC_MASTER_TUNE, -64, 64, 0);

	// Set up the master transpose slider
	InitSlider (hWnd, IDC_MASTER_TRANSPOSE, -12, 12, 0);

	// Set up the bend range slider
	InitSlider (hWnd, IDC_MASTER_BEND_RANGE, 0, 12, 2);

	// Initialise the velocity curve combobox
	for (i=0 ; i<ARRAY_SIZE(lpszVelocityCurves) ; i++)
	{
		SendDlgItemMessage (hWnd, IDC_MASTER_VELOCITY_CURVE, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)lpszVelocityCurves[i]);
	}

	// Initialise the MIDI mode combobox
	for (i=0 ; i<ARRAY_SIZE(lpszMidiModes) ; i++)
	{
		SendDlgItemMessage (hWnd, IDC_MASTER_MIDI_MODE, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)lpszMidiModes[i]);
	}

	// Initialise the MIDI ID up-down control
	InitUpDown (hWnd, IDC_MASTER_MIDI_ID, 0, 15, 0, GetDlgItem (hWnd, IDC_MASTER_MIDI_ID_TEXT), lpfnDisplayNumber);

	// Initialise the MIDI controller A up-down control
	InitUpDown (hWnd, IDC_MASTER_CONTROLLER_A, 0, 31, 0, GetDlgItem (hWnd, IDC_MASTER_CONTROLLER_A_TEXT), lpfnDisplayNumber);

	// Initialise the MIDI controller B up-down control
	InitUpDown (hWnd, IDC_MASTER_CONTROLLER_B, 0, 31, 0, GetDlgItem (hWnd, IDC_MASTER_CONTROLLER_B_TEXT), lpfnDisplayNumber);

	// Initialise the MIDI controller C up-down control
	InitUpDown (hWnd, IDC_MASTER_CONTROLLER_C, 0, 31, 0, GetDlgItem (hWnd, IDC_MASTER_CONTROLLER_C_TEXT), lpfnDisplayNumber);

	// Initialise the MIDI controller D up-down control
	InitUpDown (hWnd, IDC_MASTER_CONTROLLER_D, 0, 31, 0, GetDlgItem (hWnd, IDC_MASTER_CONTROLLER_D_TEXT), lpfnDisplayNumber);

	// Initialise the footswitch 1 up-down control
	InitUpDown (hWnd, IDC_MASTER_FOOTSWITCH_1, 64, 79, 64, GetDlgItem (hWnd, IDC_MASTER_FOOTSWITCH_1_TEXT), lpfnDisplayNumber);

	// Initialise the footswitch 2 up-down control
	InitUpDown (hWnd, IDC_MASTER_FOOTSWITCH_2, 64, 79, 64, GetDlgItem (hWnd, IDC_MASTER_FOOTSWITCH_2_TEXT), lpfnDisplayNumber);

	// Initialise the footswitch 3 up-down control
	InitUpDown (hWnd, IDC_MASTER_FOOTSWITCH_3, 64, 79, 64, GetDlgItem (hWnd, IDC_MASTER_FOOTSWITCH_3_TEXT), lpfnDisplayNumber);

	// If the preset and hyperpreset lists are loaded
	if ((pMorpheusPresetList != NULL) && (pMorpheusHyperpresetList != NULL))
	{
		// For each MIDI channel
		for (i=0 ; i<ARRAY_SIZE(idChannelProgram) ; i++)
		{
			// Set the program number
			NumPrograms = WNUM (pMorpheusPresetList->wNumPresets) + WNUM (pMorpheusHyperpresetList->wNumHyperpresets);
			for (n=0 ; n<NumPrograms ; n++)
			{
				wsprintf (&szListEntry[0], "%s", ProgramName (n));
				SendDlgItemMessage (hWnd, idChannelProgram[i].idProgram, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)&szListEntry[0]);
			}

			// Set the volume control up
			InitUpDown (hWnd, idChannelProgram[i].idVolume, 0, 127, 0,
						GetDlgItem (hWnd, idChannelProgram[i].idVolumeLabel), lpfnDisplayNumber);

			// Set the pan control up
			InitUpDown (hWnd, idChannelProgram[i].idPan, -8, 7, 0,
						GetDlgItem (hWnd, idChannelProgram[i].idPanLabel), lpfnDisplayPresetPan);
		}
	}
}

/*^L*/
/*****************************************************************************
 Function:		FindMasterParam
 Parameters:	int		idParam			Parameter number.
 Returns:		int						Parameter's value.
 Description:	Searches the loaded master settings for the specified
				parameter.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 14/6/94	| Created.											| PW
*****************************************************************************/

static int FindMasterParam (int idParam)
{
	LPWORD	lpwParamNumber, lpwBlockEnd;
	int		nValue = 0;

	// Calculate where the end of the global parameters is
	lpwBlockEnd = (LPWORD)_fmemchr (pMasterSettings, SYSEX_END, nMasterSettingsLength);

	// Create a pointer to the first parameter number in the buffer
	lpwParamNumber = (LPWORD)(&pMasterSettings[SYSEX_HEADER_LEN]);

	// Until the end of the block is reached
	while (lpwParamNumber < lpwBlockEnd)
	{
		// If the found parameter is the one we're looking for
		if (WNUM (lpwParamNumber[0]) == idParam)
		{
			// Extract its value
			nValue = WNUM (lpwParamNumber[1]);
			break;
		}

		// Step past this parameter
		lpwParamNumber += 2;
	}

	// Return the found value
	return (nValue);
}

/*^L*/
/*****************************************************************************
 Function:		FindMidimapParam
 Parameters:	int		idParam			Parameter number.
 Returns:		int						Parameter's value.
 Description:	Searches the loaded master settings for the specified
				parameter.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 14/6/94	| Created.											| PW
*****************************************************************************/

static int FindMidimapParam (int idParam)
{
	LPWORD	lpwParamNumber, lpwBlockEnd;
	int		nValue = 0;

	// Create a pointer to the first parameter number in the scratch midimap buffer
	lpwParamNumber = (LPWORD)(((LPBYTE)_fmemchr (pMasterSettings + 1, SYSEX_START, nMasterSettingsLength)) + SYSEX_HEADER_LEN);

	// Calculate where the end of the scratch midimap parameters is
	lpwBlockEnd = (LPWORD)_fmemchr (lpwParamNumber, SYSEX_END, nMasterSettingsLength);

	// Until the end of the block is reached
	while (lpwParamNumber < lpwBlockEnd)
	{
		// If the found parameter is the one we're looking for
		if (WNUM (lpwParamNumber[0]) == idParam)
		{
			// Extract its value
			nValue = WNUM (lpwParamNumber[1]);
			break;
		}

		// Step past this parameter
		lpwParamNumber += 2;
	}

	// Return the found value
	return (nValue);
}


/*^L*/
/*****************************************************************************
 Function:		GetScratchChannelParams
 Parameters:	int	nChan				MIDI channel number (0..15)
				int	*pnProgram			(Output) program number
				int	*pnVolume			(Output) volume
				int	*pnPan				(Output) pan
 Returns:		None.
 Description:	Gets the specified scratch midimap parameters.
				parameter.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 14/6/94	| Created.											| PW
*****************************************************************************/

static VOID GetScratchChannelParams (int nChan, int *pnProgram, int *pnVolume, int *pnPan)
{
	// Get the program number
	*pnProgram = FindMidimapParam (MPARAM_MIDIMAP_BASE + 12 + (20 * nChan));

	// Get the volume
	*pnVolume = FindMidimapParam (MPARAM_MIDIMAP_BASE + 14 + (20 * nChan));

	// Get the pan
	*pnPan = FindMidimapParam (MPARAM_MIDIMAP_BASE + 15 + (20 * nChan));
}

/*^L*/
/*****************************************************************************
 Function:		ChannelChange
 Parameters:	HWND	hWnd
				WPARAM	wParam
				LPARAM	lParam
 Returns:		None.
 Description:	Processes a dialog WM_COMMAND message that is one of the
				scratch midimap channel controls.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 15/6/94	| Created.											| PW
*****************************************************************************/

static VOID ChannelChange (HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	UINT	i;
	int		idControl = LOWORD (wParam);
	int		nProgram;

	// For each channel
	for (i=0 ; i<ARRAY_SIZE(idChannelProgram) ; i++)
	{
		// If it's this channel's program that's changing
		if ((idChannelProgram[i].idProgram == idControl) && (NCODE == CBN_SELCHANGE))
		{
			// Get the new program number
			nProgram = SendDlgItemMessage (hWnd, idControl, CB_GETCURSEL, 0, 0);

			// Set the basic MIDI channel
			MorpheusSetParameter (hWnd, MPARAM_BASIC_CHANNEL, i);

			// Send a program change
			MorpheusSetParameter (hWnd, MPARAM_CURRENT_PROGRAM, nProgram);

			// Done
			return;
		}

		// If it's this channel's volume that's changed
		if (idChannelProgram[i].idVolume == idControl)
		{
			// Set the basic MIDI channel
			MorpheusSetParameter (hWnd, MPARAM_BASIC_CHANNEL, i);

			// Send a volume change
			MorpheusSetParameter (hWnd, MPARAM_CHANNEL_VOLUME, (int)lParam);

			// Done
			return;
		}

		// If it's this channel's pan that's changed
		if (idChannelProgram[i].idPan == idControl)
		{
			// Set the basic MIDI channel
			MorpheusSetParameter (hWnd, MPARAM_BASIC_CHANNEL, i);

			// Send a pan change
			MorpheusSetParameter (hWnd, MPARAM_CHANNEL_PAN, (int)lParam);

			// Done
			return;
		}
	}
}

