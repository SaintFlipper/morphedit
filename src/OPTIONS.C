/*****************************************************************************
 File:			options.c
 Description:	The Morpheus editor options module.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 8/10/94		| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define		MIDI_SECTION_NAME			"MIDI"
 #define	MIDI_INPUT_ENTRY_NAME		"MidiInputDevice"
 #define	MIDI_OUTPUT_ENTRY_NAME		"MidiOutputDevice"
 #define	MIDI_DEVICE_ID_NAME			"MidiDeviceID"
 #define	TEST_KBD_BASE_KEY			"TestKeyboardBaseKey"
 #define	TEST_KBD_MIDI_CHANNEL		"TestKeyboardMidiChannel"
 #define	TEST_KBD_MIDI_A				"TestKeyboardMidiControllerA"
 #define	TEST_KBD_MIDI_B				"TestKeyboardMidiControllerB"
 #define	TEST_KBD_MIDI_C				"TestKeyboardMidiControllerC"
 #define	TEST_KBD_MIDI_D				"TestKeyboardMidiControllerD"

#define		PRESET_BIG_OPTION			"BigPresetDialog"
 #define	PRESET_BIG_OPTION_DEFAULT	"YES"

#define		FILE_SECTION_NAME			"Files"
 #define	MAIN_STORAGE_PATH_OPTION	"MainStoragePath"
 #define	PRESET_BLOCK_PATH_OPTION	"PresetBlockPath"
 

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

static LPCSTR	pszDeviceIdNames[] =
{
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15"
};

static LPCSTR	pszMidiChannelNames[] =
{
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"
};

static LPCSTR	pszMidiControllerNames[] =
{
	"0",
	"1 (Mod. wheel)",
	"2 (Breath controller)",
	"3",
	"4 (Foot pedal)",
	"5 (Portamento time)",
	"6 (Data entry)",
	"7 (Volume)",
	"8 (Balance)",
	"9",
	"10 (Pan)",
	"11 (Expression)",
	"12", "13", "14",
	"15", "16", "17", "18", "19",
	"20", "21", "22", "23", "24",
	"25", "26", "27", "28", "29",
	"30", "31"
};

static LPCSTR	pszBaseOctaveNames[] =
{
	"C-2", "C-1", "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8"
};

static char		szTemporaryPath[MAXPATH];

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

static VOID			CreateMidiInList (HWND hWnd);
static VOID			CreateMidiOutList (HWND hWnd);
static void			BrowsePath (HWND hwndDlg, LPSTR lpszPath);
DIALOG_PROC			GetPathProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

/*^L*/
/*****************************************************************************
 Function:		LoadOptions
 Parameters:	None.
 Returns:		None.
 Description:	Load the options.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 24/4/94 	| Created.											| PW
*****************************************************************************/

VOID LoadOptions (VOID)
{
	char		szEntry[100];
	UINT		i;
	UINT		nNumDevs;				// Number of MIDI devices
	MIDIINCAPS	MidiInCaps;				// Device capabilities
	MIDIOUTCAPS	MidiOutCaps;			// Device capabilities
	char		szPath[MAXPATH];

	// Try to read the input device name
	if (GetPrivateProfileString (MIDI_SECTION_NAME, MIDI_INPUT_ENTRY_NAME,
				"", &szEntry[0], sizeof (szEntry), &szIniFileName[0]) != 0)
	{
		// Get number of MIDI input devices
		nNumDevs = midiInGetNumDevs ();

		// For each device
		for (i=0 ; i<nNumDevs ; i++)
		{
			// Get its parameters
			midiInGetDevCaps (i, &MidiInCaps, sizeof (MIDIINCAPS));

			// If the name matches the profile string
			if (!strcmp (&MidiInCaps.szPname[0], &szEntry[0]))
			{
				// This is the entry
                break;
            }
		}

		// Store the found entry index (or 0 if none)
		idMidiInput = (i == nNumDevs) ? NO_PARAM : i;
	}
	else
	{
		// Default to no device
		idMidiInput = NO_PARAM;
    }

	// Try to read the output device name
	if (GetPrivateProfileString (MIDI_SECTION_NAME, MIDI_OUTPUT_ENTRY_NAME,
				"", &szEntry[0], sizeof (szEntry), &szIniFileName[0]) != 0)
	{
		// Get number of MIDI output devices
		nNumDevs = midiOutGetNumDevs ();

		// For each device
		for (i=0 ; i<nNumDevs ; i++)
		{
			// Get its parameters
			midiOutGetDevCaps (i, &MidiOutCaps, sizeof (MIDIOUTCAPS));

			// If the name matches the profile string
			if (!strcmp (&MidiOutCaps.szPname[0], &szEntry[0]))
			{
				// This is the entry
                break;
            }
		}

		// Store the found entry index (or 0 if none)
		idMidiOutput = (i == nNumDevs) ? NO_PARAM : i;
	}
	else
	{
		// Default to no device
		idMidiOutput = NO_PARAM;
	}

	// Get the Morpheus device ID (default to ID = 0)
	bMidiDeviceID = GetPrivateProfileInt (MIDI_SECTION_NAME, MIDI_DEVICE_ID_NAME, 0, &szIniFileName[0]);

	// Get the test keyboard MIDI channel
	bKbdMidiChannel = GetPrivateProfileInt (MIDI_SECTION_NAME, TEST_KBD_MIDI_CHANNEL, 0, &szIniFileName[0]);

	// Get the test keyboard base octave (=> key)
	nKeyboardLeftKey = GetPrivateProfileInt (MIDI_SECTION_NAME, TEST_KBD_BASE_KEY, 0, &szIniFileName[0]);

	// Get the test keyboard MIDI controller mapping
	nTestKbdControllers[0] = GetPrivateProfileInt (MIDI_SECTION_NAME, TEST_KBD_MIDI_A, 0, &szIniFileName[0]);
	nTestKbdControllers[1] = GetPrivateProfileInt (MIDI_SECTION_NAME, TEST_KBD_MIDI_B, 0, &szIniFileName[0]);
	nTestKbdControllers[2] = GetPrivateProfileInt (MIDI_SECTION_NAME, TEST_KBD_MIDI_C, 0, &szIniFileName[0]);
	nTestKbdControllers[3] = GetPrivateProfileInt (MIDI_SECTION_NAME, TEST_KBD_MIDI_D, 0, &szIniFileName[0]);

	// Get the "big preset dialog" flag
	GetPrivateProfileString (WINDOWS_SECTION_NAME, PRESET_BIG_OPTION,
				PRESET_BIG_OPTION_DEFAULT, &szEntry[0], sizeof (szEntry), &szIniFileName[0]);
	fBigPresetDialog = lstrcmpi (&szEntry[0], "YES") ? FALSE : TRUE;

	// Create the default load path
	lstrcpy (&szPath[0], &szMyPath[0]);
	if (szPath[lstrlen (&szPath[0]) - 1] == '\\')
	{
		szPath[lstrlen (&szPath[0]) - 1] = '\0';
	}

	// Get the main storage path option
	GetPrivateProfileString (FILE_SECTION_NAME, MAIN_STORAGE_PATH_OPTION,
				&szMyPath[0], &szStorageDirectory[0], MAXPATH, &szIniFileName[0]);

	// Get the preset building block storage path option
	GetPrivateProfileString (FILE_SECTION_NAME, PRESET_BLOCK_PATH_OPTION,
				&szPath[0], &szPresetMacroDirectory[0], MAXPATH, &szIniFileName[0]);
}

/*^L*/
/*****************************************************************************
 Function:		StoreOptions
 Parameters:	None.
 Returns:		None.
 Description:	Stores the options.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 24/4/94 	| Created.											| PW
*****************************************************************************/

VOID StoreOptions (VOID)
{
	MIDIINCAPS	MidiInCaps;				// Device capabilities
	MIDIOUTCAPS	MidiOutCaps;			// Device capabilities
	char		szNumBuf[20];

	// Write the input device name
	if (idMidiOutput == NO_PARAM)
	{
		WritePrivateProfileString (MIDI_SECTION_NAME, MIDI_INPUT_ENTRY_NAME, "None", &szIniFileName[0]);
	}
	else
	{
		midiInGetDevCaps (idMidiInput, &MidiInCaps, sizeof (MIDIINCAPS));
		WritePrivateProfileString (MIDI_SECTION_NAME, MIDI_INPUT_ENTRY_NAME, &MidiInCaps.szPname[0], &szIniFileName[0]);
	}

	// Write the output device name
	if (idMidiOutput == NO_PARAM)
	{
		WritePrivateProfileString (MIDI_SECTION_NAME, MIDI_OUTPUT_ENTRY_NAME, "None", &szIniFileName[0]);
	}
	else
	{
		midiOutGetDevCaps (idMidiOutput, &MidiOutCaps, sizeof (MIDIOUTCAPS));
		WritePrivateProfileString (MIDI_SECTION_NAME, MIDI_OUTPUT_ENTRY_NAME, &MidiOutCaps.szPname[0], &szIniFileName[0]);
	}

	// Write the Morpheus device ID
	wsprintf (&szNumBuf[0], "%u", bMidiDeviceID);
	WritePrivateProfileString (MIDI_SECTION_NAME, MIDI_DEVICE_ID_NAME, &szNumBuf[0], &szIniFileName[0]);

	// Write the test keyboard MIDI channel
	wsprintf (&szNumBuf[0], "%u", (UINT)bKbdMidiChannel);
	WritePrivateProfileString (MIDI_SECTION_NAME, TEST_KBD_MIDI_CHANNEL, &szNumBuf[0], &szIniFileName[0]);

	// Write the test keyboard base note
	wsprintf (&szNumBuf[0], "%u", (UINT)nKeyboardLeftKey);
	WritePrivateProfileString (MIDI_SECTION_NAME, TEST_KBD_BASE_KEY, &szNumBuf[0], &szIniFileName[0]);

	// Write the test keyboard MIDI controller mappings
	wsprintf (&szNumBuf[0], "%u", (UINT)nTestKbdControllers[0]);
	WritePrivateProfileString (MIDI_SECTION_NAME, TEST_KBD_MIDI_A, &szNumBuf[0], &szIniFileName[0]);
	wsprintf (&szNumBuf[0], "%u", (UINT)nTestKbdControllers[1]);
	WritePrivateProfileString (MIDI_SECTION_NAME, TEST_KBD_MIDI_B, &szNumBuf[0], &szIniFileName[0]);
	wsprintf (&szNumBuf[0], "%u", (UINT)nTestKbdControllers[2]);
	WritePrivateProfileString (MIDI_SECTION_NAME, TEST_KBD_MIDI_C, &szNumBuf[0], &szIniFileName[0]);
	wsprintf (&szNumBuf[0], "%u", (UINT)nTestKbdControllers[3]);
	WritePrivateProfileString (MIDI_SECTION_NAME, TEST_KBD_MIDI_D, &szNumBuf[0], &szIniFileName[0]);

	// Write the "big preset dialog" flag
	WritePrivateProfileString (WINDOWS_SECTION_NAME, PRESET_BIG_OPTION, fBigPresetDialog ? "YES" : "NO", &szIniFileName[0]);

	// Write the main storage path option
	SaveStoragePath (&szStorageDirectory[0]);

	// Write the preset building block storage path option
	WritePrivateProfileString (FILE_SECTION_NAME, PRESET_BLOCK_PATH_OPTION, &szPresetMacroDirectory[0], &szIniFileName[0]);
}

/*^L*/
/*****************************************************************************
 Function:		SaveStoragePath
 Parameters:	LPCSTR	lpszPath
 Returns:		None.
 Description:	Saves the specified file storage path.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 29/10/94 	| Created.											| PW
*****************************************************************************/

VOID SaveStoragePath (LPCSTR lpszPath)
{
	// Write the main storage path option
	WritePrivateProfileString (FILE_SECTION_NAME, MAIN_STORAGE_PATH_OPTION, lpszPath, &szIniFileName[0]);
}

/*^L*/
/*****************************************************************************
 Function:		MidiOptionsDialog
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if message processed, else FALSE
 Description:	The dialog procedure for MIDI options.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 16/3/94 	| Created.											| PW
*****************************************************************************/

DIALOG_PROC MidiOptionsDialog (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	int			idMidiInputLocal, idMidiOutputLocal, nTempKey;
	char		szDirectory[MAXPATH];

	switch (nMsg)
	{
		// Creation
		case WM_INITDIALOG:
		{
			// Reformat myself
			ReformatDialog (hWnd, RDF_THINFONT);

			// Get the names of the MIDI input devices
			CreateMidiInList (GetDlgItem (hWnd, IDC_MIDI_INPUTS_LIST));

			// Get the names of the MIDI output devices
			CreateMidiOutList (GetDlgItem (hWnd, IDC_MIDI_OUTPUTS_LIST));

			// Fill the Morpheus device ID list
			InitListbox (GetDlgItem (hWnd, IDC_DEVICE_ID), &pszDeviceIdNames[0], ARRAY_SIZE (pszDeviceIdNames));

			// Initialise the test keyboard MIDI channel list
			InitListbox (GetDlgItem (hWnd, IDC_TEST_KBD_MIDI_CHAN), &pszMidiChannelNames[0], ARRAY_SIZE (pszMidiChannelNames));

			// Initialise the test keyboard base octave list
			InitListbox (GetDlgItem (hWnd, IDC_TEST_KBD_BASE_OCTAVE), &pszBaseOctaveNames[0], ARRAY_SIZE (pszBaseOctaveNames));

			// Initialise the test keyboard MIDI controller mappings
			InitListbox (GetDlgItem (hWnd, IDC_TEST_KBD_MIDI_A), &pszMidiControllerNames[0], ARRAY_SIZE (pszMidiControllerNames));
			InitListbox (GetDlgItem (hWnd, IDC_TEST_KBD_MIDI_B), &pszMidiControllerNames[0], ARRAY_SIZE (pszMidiControllerNames));
			InitListbox (GetDlgItem (hWnd, IDC_TEST_KBD_MIDI_C), &pszMidiControllerNames[0], ARRAY_SIZE (pszMidiControllerNames));
			InitListbox (GetDlgItem (hWnd, IDC_TEST_KBD_MIDI_D), &pszMidiControllerNames[0], ARRAY_SIZE (pszMidiControllerNames));

			// Select the device ID entry
			SendDlgItemMessage (hWnd, IDC_DEVICE_ID, CB_SETCURSEL, bMidiDeviceID, 0);

			// Select the test keyboard MIDI channel
			SendDlgItemMessage (hWnd, IDC_TEST_KBD_MIDI_CHAN, CB_SETCURSEL, bKbdMidiChannel, 0);

			// Select the test keyboard base octave
			SendDlgItemMessage (hWnd, IDC_TEST_KBD_BASE_OCTAVE, CB_SETCURSEL, nKeyboardLeftKey / 12, 0);

			// Select the test keyboard MIDI controller mappings
			SendDlgItemMessage (hWnd, IDC_TEST_KBD_MIDI_A, CB_SETCURSEL, nTestKbdControllers[0], 0);
			SendDlgItemMessage (hWnd, IDC_TEST_KBD_MIDI_B, CB_SETCURSEL, nTestKbdControllers[1], 0);
			SendDlgItemMessage (hWnd, IDC_TEST_KBD_MIDI_C, CB_SETCURSEL, nTestKbdControllers[2], 0);
			SendDlgItemMessage (hWnd, IDC_TEST_KBD_MIDI_D, CB_SETCURSEL, nTestKbdControllers[3], 0);

			// Fill in the storage and preset block paths
			SendDlgItemMessage (hWnd, IDC_PRESET_BLOCK_PATH, WM_SETTEXT, 0, (LPARAM)(LPCSTR)&szPresetMacroDirectory[0]);

			// Allow Windows to set the control focus
			return (TRUE);
		}

		// Controls
		case WM_COMMAND:
		{
			switch (LOWORD (wParam))
			{
				// 'OK' button
				case IDOK:
				{
					// Read back the test keyboard options
					nTempKey = SendDlgItemMessage (hWnd, IDC_TEST_KBD_BASE_OCTAVE, CB_GETCURSEL, 0, 0) * 12;
					bKbdMidiChannel = (BYTE)SendDlgItemMessage (hWnd, IDC_TEST_KBD_MIDI_CHAN, CB_GETCURSEL, 0, 0);
					nTestKbdControllers[0] = SendDlgItemMessage (hWnd, IDC_TEST_KBD_MIDI_A, CB_GETCURSEL, 0, 0);
					nTestKbdControllers[1] = SendDlgItemMessage (hWnd, IDC_TEST_KBD_MIDI_B, CB_GETCURSEL, 0, 0);
					nTestKbdControllers[2] = SendDlgItemMessage (hWnd, IDC_TEST_KBD_MIDI_C, CB_GETCURSEL, 0, 0);
					nTestKbdControllers[3] = SendDlgItemMessage (hWnd, IDC_TEST_KBD_MIDI_D, CB_GETCURSEL, 0, 0);

					// If the test keyboard base key has changed
					if (nTempKey != (int)nKeyboardLeftKey)
					{
						// Save the new base key
						nKeyboardLeftKey = nTempKey;

						// Force a redraw of the keyboard
						InvalidateRect (hwndPiano, NULL, TRUE);
					}

					// Read back the storage and preset block paths
					SendDlgItemMessage (hWnd, IDC_PRESET_BLOCK_PATH, WM_GETTEXT, MAXPATH, (LPARAM)(LPSTR)&szPresetMacroDirectory[0]);

					// Read back device ID
					bMidiDeviceID = (BYTE)SendDlgItemMessage (hWnd, IDC_DEVICE_ID, CB_GETCURSEL, 0, 0);

					// Read back the Windows MIDI devices
					idMidiInputLocal	= SendDlgItemMessage (hWnd, IDC_MIDI_INPUTS_LIST, CB_GETCURSEL, 0, 0);
					idMidiOutputLocal	= SendDlgItemMessage (hWnd, IDC_MIDI_OUTPUTS_LIST, CB_GETCURSEL, 0, 0);
					
					// If either device has been changed
					if ((idMidiOutput != idMidiOutputLocal) || (idMidiInput != idMidiInputLocal))
					{
						// Open the MIDI devices
						if (!morphdllOpenMidiDevices (idMidiInputLocal, idMidiOutputLocal))
						{
							// Error
							MyMessageBox (NULL, ResourceString (IDS_ERROR_OPENING_MIDI_DEVS), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
							return (TRUE);
						}

						// Store the new MIDI devices
						idMidiInput		= idMidiInputLocal;
						idMidiOutput	= idMidiOutputLocal;
					}

					EndDialog (hWnd, 0);
					break;
				}

				// 'Cancel' button
				case IDCANCEL:
				{
					EndDialog (hWnd, 0);
					break;
				}

				// Browse preset building block path
				case IDC_BROWSE_PRESET_BLOCK_PATH:
				{
					// Read the directory back from the editbox
					SendDlgItemMessage (hWnd, IDC_PRESET_BLOCK_PATH, WM_GETTEXT, MAXPATH, (LPARAM)(LPSTR)&szDirectory[0]);

					// Browse it
					BrowsePath (hWnd, &szDirectory[0]);

					// Write the directory back to the editbox
					SendDlgItemMessage (hWnd, IDC_PRESET_BLOCK_PATH, WM_SETTEXT, 0, (LPARAM)(LPSTR)&szDirectory[0]);
					break;
				}

				default:
				{
					break;
				}
			}

			// Processed WM_COMMAND
			return (TRUE);
		}

		// Control about to be drawn
		case WM_CTLCOLOR:
		{
			Ctl3dCtlColorEx (nMsg, wParam, lParam);
			break;
		}

		// Default messages
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
 Function:		CreateMidiInList
 Parameters:	HWND	hWnd			Listbox handle.
 Returns:		None.
 Description:	Fills the specified listbox with MIDI intput device names.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 16/3/94 	| Created.											| PW
*****************************************************************************/

static VOID CreateMidiInList (HWND hWnd)
{
	UINT		i;
	UINT		nNumDevs;				// Number of MIDI devices
	MIDIINCAPS	MidiInCaps;				// Device capabilities

	// Get number of MIDI input devices
	nNumDevs = midiInGetNumDevs ();

	// Clear the listbox
	SendMessage (hWnd, CB_RESETCONTENT, 0, 0);

	// For each device
	for (i=0 ; i<nNumDevs ; i++)
	{
		// Get its parameters
		midiInGetDevCaps (i, &MidiInCaps, sizeof (MIDIINCAPS));

		// Write its name to the listbox
		SendMessage (hWnd, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)&MidiInCaps.szPname[0]);
	}

	// Select current device
	SendMessage (hWnd, CB_SETCURSEL, idMidiInput, 0);
}

/*^L*/
/*****************************************************************************
 Function:		CreateMidiOutList
 Parameters:	HWND	hWnd			Listbox handle.
 Returns:		None.
 Description:	Fills the specified listbox with MIDI output device names.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 16/3/94 	| Created.											| PW
*****************************************************************************/

static VOID CreateMidiOutList (HWND hWnd)
{
	UINT		i;
	UINT		nNumDevs;				// Number of MIDI devices
	MIDIOUTCAPS	MidiOutCaps;			// Device capabilities

	// Get number of MIDI output devices
	nNumDevs = midiOutGetNumDevs ();

	// Clear the listbox
	SendMessage (hWnd, CB_RESETCONTENT, 0, 0);

	// For each device
	for (i=0 ; i<nNumDevs ; i++)
	{
		// Get its parameters
		midiOutGetDevCaps (i, &MidiOutCaps, sizeof (MIDIOUTCAPS));

		// Write its name to the listbox
		SendMessage (hWnd, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)&MidiOutCaps.szPname[0]);
	}

	// Select current device
	SendMessage (hWnd, CB_SETCURSEL, idMidiOutput, 0);
}


/**/
/***************************************************************************
Function:		BrowsePath
Parameters:		HWND	hwndDlg			Dialog box window handle.
				LPSTR	lpszPath		Input/output path.
Return value:	None.
Description:	Allows the user to pick a new path.
History:

Date		| Description										| Name
------------+---------------------------------------------------+-----
29/7/93		| Created.											| PW
***************************************************************************/

static void BrowsePath (HWND hwndDlg, LPSTR lpszPath)
{
	char			szFileName[MAXPATH];
	char			szTempPath[MAXPATH];
	OPENFILENAME	OpenFileName;
	char			szMessage[200];
	DWORD			dwError;

//	strcpy (&szFileName[0], "*.*");
	strcpy (&szFileName[0], "");

	OpenFileName.lStructSize		= sizeof (OPENFILENAME);
	OpenFileName.hwndOwner			= (HWND)hwndDlg;
	OpenFileName.hInstance			= hInst;
	OpenFileName.lpstrFilter		= (LPCSTR)NULL;
	OpenFileName.lpstrCustomFilter	= (LPSTR)NULL;
	OpenFileName.nMaxCustFilter		= 0;
	OpenFileName.nFilterIndex		= 1;
	OpenFileName.lpstrFile			= (LPSTR)&szFileName[0];
	OpenFileName.nMaxFile			= sizeof (szFileName);
	OpenFileName.lpstrFileTitle		= (LPSTR)NULL;
	OpenFileName.nMaxFileTitle		= 0;
	OpenFileName.lpstrInitialDir	= &lpszPath[0];
	OpenFileName.lpstrTitle			= (LPCSTR)NULL;
	OpenFileName.Flags				= OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
	OpenFileName.lpstrDefExt		= (LPCSTR)NULL;
	OpenFileName.lpTemplateName		= (LPCSTR)"DLG_DIRECTORY_FILEOPEN";
	OpenFileName.lpfnHook			= (LPVOID)MakeProcInstance ((FARPROC)GetPathProc, hInst);

	lstrcpy ((LPSTR)&szTempPath[0], &lpszPath[0]);
	if (!GetOpenFileName ((OPENFILENAME far *)&OpenFileName))
	{
		lstrcpy (&lpszPath[0], (LPSTR)&szTempPath[0]);
		if ((dwError = CommDlgExtendedError ()) != 0)
		{
			wsprintf (&szMessage[0], ResourceString (IDS_COMDLG_RET_MASK), dwError);
			MyMessageBox ((HWND)NULL, &szMessage[0], ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		}
	}
	else
	{
		lstrcpy (&lpszPath[0], (LPSTR)&szTemporaryPath[0]);
    }
}

/**/
/***************************************************************************
Function:		GetPathProc
Parameters:		HWND	hWnd			Dialog box window handle.
				UINT	nMsg			Message to process.
				WPARAM	wParam
				WPARAM	wParam
Return value:	UINT					FALSE if common dialog should also
										process the message, else TRUE. 
Description:	Local dialog to process COMMDLG open-file messages.
History:

Date		| Description										| Name
------------+---------------------------------------------------+-----
15/7/93		| Created.											| PW
***************************************************************************/

DIALOG_PROC GetPathProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg)
	{
		case WM_INITDIALOG:
		{
			// Reformat myself
			ReformatDialog (hWnd, RDF_CENTRE | RDF_THINFONT);
			return (TRUE);
		}

		case WM_COMMAND:
		{
			switch (wParam)
			{
				case IDOK:
				{
					// Make sure it's a button down
					if (NCODE != BN_CLICKED)
                    	break;

					// Read back the current directory
					GetDlgItemText (hWnd, stc1, (LPSTR)&szTemporaryPath[0], sizeof (szTemporaryPath));
					PostMessage (hWnd, WM_COMMAND, IDABORT, (LPARAM)TRUE);
					return (FALSE);
				}

				default:
				{
					// Allow default processing
					return (FALSE);
				}
            }
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
		
		default:
		{
			// Allow default processing
			return (FALSE);
		}
	}
}

