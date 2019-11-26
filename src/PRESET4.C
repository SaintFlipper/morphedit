/*****************************************************************************
 File:			preset4.c
 Description:	Functions for handling preset sub-block loading and saving.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 25/10/94	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define	IDC_DIRLISTBOX		1000

#define	FILE_VERSION		"1.000"
#define	PM_EXT_LENGTH		2

/****************************************************************************/
/*								Local types									*/
/****************************************************************************/

typedef enum
{
	PMT_FG1,
	PMT_FG2,
	PMT_PRIMARY,
	PMT_SECONDARY,
	PMT_AUX_ENV,
	PMT_NOTE_ON_MOD,
	PMT_REAL_TIME_MOD
} PRESET_MACRO_TYPE;

typedef struct
{
	LPCSTR				lpszTitle;			// Title to give the dialog
	LPCSTR				lpszExt;			// DOS extension associated with the block type
	POINT				ptTopLeft;			// Pointer position
	PRESET_MACRO_TYPE	type;				// Sub-preset type
	HWND				hwndParent;			// Parent window displaying the dialog
} SPD_DEF, FAR *PSPD_DEF;

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

static char	szFileVersion[] = FILE_VERSION;

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

DIALOG_PROC PresetMacroDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

static VOID ScreenToClientRect (HWND hWnd, LPRECT lpRect);
static BOOL LoadPresetMacro (LPCSTR lpszFileRoot, LPCSTR lpszFileExt, PRESET_MACRO_TYPE type);
static BOOL SavePresetMacro (LPCSTR lpszFileRoot, LPCSTR lpszFileExt, PRESET_MACRO_TYPE type);
static VOID SendChangedPresetParameters (PPRESET_PARAMS pOldPreset, PPRESET_PARAMS pNewPreset);

/*^L*/
/*****************************************************************************
 Function:		PresetRightMouseButton
 Parameters:	HWND	hWnd			Window that button went down in.
				int		x				Dialog X position of pointer.
				int		y				Dialog Y position of pointer.
 Returns:		None.
 Description:	Called by the preset dialog handlers when the right mouse
				button is pressed. Note that this may be called from either
				the main preset dialog, or from the patchbay windows. The
				window handle allows you to determine which it is.

				If it is being called from the main preset dialog, then it
				checks to see whether the pointer is within one of the
				supported control groups, which are :-

				Function generator 1
				Function generator 2
				Primary instrument
				Secondary instrument
				Auxiliary envelope
				Note on modulation routings
				Real time modulation routings

				If the pointer IS within one of these groups, then the
				preset macro dialog is displayed. This displays a list of
				files for the selected type of sub-preset, and allows the
				user to select and load one of them.

				For the patchbay windows, it can determined straight away
				which block loading dialog to display.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 25/10/94 	| Created.											| PW
*****************************************************************************/

VOID PresetRightMouseButton (HWND hWnd, int x, int y)
{
	static struct
	{
		int					idGroupBox;		// Preset group box surrounding the bit of interest
		SUBDIALOG			sub;			// Sub-dialog type that must be displayed to consider the groupbox
		LPCSTR				lpszTitle;		// Preset macro dialog title
		LPCSTR				lpszExt;		// DOS extension of sub-preset block files
		PRESET_MACRO_TYPE	type;			// Sub-preset type
	} Gdef[] =
	{
		{IDC_FG1_GROUPBOX,				FG,				"FG1 block",			"FG",		PMT_FG1},
		{IDC_FG2_GROUPBOX,				FG,				"FG2 block",			"FG",		PMT_FG2},
		{IDC_PRIMARY_GROUPBOX,			VOICES,			"Primary voice",		"VX",		PMT_PRIMARY},
		{IDC_SECONDARY_GROUPBOX,		VOICES,			"Secondary voice",		"VX",		PMT_SECONDARY},
		{IDC_AUX_ENV_GROUPBOX,			LFO_AUXENV,		"Aux env",				"AE",		PMT_AUX_ENV},
		{IDC_NOTE_ON_MODS_GROUPBOX,		ROUTINGS,		"Note-on patches",		"NO",		PMT_NOTE_ON_MOD},
		{IDC_REAL_TIME_MODS_GROUPBOX,	ROUTINGS,		"Real-time patches",	"RT",		PMT_REAL_TIME_MOD}
	};
	RECT		rcGroup;
	int			i;
	SPD_DEF		spd;
	DLGPROC		lpDialogProc;
	BOOL		fFound = FALSE;

	// If no preset is on display
	if (nDisplayedPreset == INDEX_NONE)
	{
		// Go no further
		return;
	}

	// If the pointer is in the NOTE-ON PATCHBAY
	if (hWnd == hwndNoteOnPatches)
	{
		// Search for the NOTE-ON preset definition
		for (i=0 ; i<ARRAY_SIZE(Gdef) ; i++)
		{
			if (Gdef[i].type == PMT_NOTE_ON_MOD)
			{
				break;
			}
		}

		// Set up a structure defining dialog information
		spd.lpszTitle	= Gdef[i].lpszTitle;
		spd.lpszExt		= Gdef[i].lpszExt;
		spd.ptTopLeft.x	= x;
		spd.ptTopLeft.y	= y;
		spd.type		= Gdef[i].type;
		spd.hwndParent	= hWnd;

		fFound = TRUE;
	}
	// Else if the pointer is in the REAL-TIME PATCHBAY
	else if (hWnd == hwndRealTimePatches)
	{
		// Search for the REAL-TIME preset definition
		for (i=0 ; i<ARRAY_SIZE(Gdef) ; i++)
		{
			if (Gdef[i].type == PMT_REAL_TIME_MOD)
			{
				break;
			}
		}

		// Set up a structure defining dialog information
		spd.lpszTitle	= Gdef[i].lpszTitle;
		spd.lpszExt		= Gdef[i].lpszExt;
		spd.ptTopLeft.x	= x;
		spd.ptTopLeft.y	= y;
		spd.type		= Gdef[i].type;
		spd.hwndParent	= hWnd;

		fFound = TRUE;
	}
	// Else if the pointer is in the PRESET DIALOG
	else if (hWnd == hwndCurrentPreset)
	{
		// For each possible sub-preset group supported
		for (i=0 ; i<ARRAY_SIZE (Gdef) ; i++)
		{
			// If the appropriate sub-preset block is on display
			if (IsPresetSubDialogDisplayed (Gdef[i].sub))
			{
				// Get the whereabouts of the sub-presets's group box
				GetWindowRect (GetDlgItem (hwndCurrentPreset, Gdef[i].idGroupBox), &rcGroup);

				// Convert from screen to dialog coordinates
				ScreenToClientRect (hwndCurrentPreset, &rcGroup);

				// If the pointer is within the groupbox
				if ((x > rcGroup.left) && (x < rcGroup.right) && (y > rcGroup.top) && (y < rcGroup.bottom))
				{
					// Set up a structure defining dialog information
					spd.lpszTitle	= Gdef[i].lpszTitle;
					spd.lpszExt		= Gdef[i].lpszExt;
					spd.ptTopLeft.x	= x;
					spd.ptTopLeft.y	= y;
					spd.type		= Gdef[i].type;
					spd.hwndParent	= hWnd;

					fFound = TRUE;
					break;
				}
			}
		}
	}

	// If we found a preset building block that the pointer is in
	if (fFound)
	{
		// DO the dialog
		lpDialogProc = (DLGPROC)MakeProcInstance ((FARPROC)PresetMacroDialogProc, hInst);
		DialogBoxParam (hInst, "DLG_PRESET_MACRO", hWnd, lpDialogProc, (LPARAM)(PSPD_DEF)&spd);
		FreeProcInstance ((FARPROC)lpDialogProc);
	}
}

/*^L*/
/*****************************************************************************
 Function:		ScreenToClientRect
 Parameters:	HWND	hWnd			Window.
				LPRECT	lpRect			Rectangle to convert.
 Returns:		None.
 Description:	Converts a rectangle from screen coordinates to client
				coordinates within the specified window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 25/10/94 	| Created.											| PW
*****************************************************************************/

static VOID ScreenToClientRect (HWND hWnd, LPRECT lpRect)
{
	POINT	pt;

	// Convert the top left corner from screen to client coordinates
	pt.x	= lpRect->left;
	pt.y	= lpRect->top;
	ScreenToClient (hWnd, &pt);
    lpRect->left	= pt.x;
    lpRect->top		= pt.y;
	
	// Convert the bottom right corner from screen to client coordinates
	pt.x	= lpRect->right;
	pt.y	= lpRect->bottom;
	ScreenToClient (hWnd, &pt);
	lpRect->right	= pt.x;
	lpRect->bottom	= pt.y;
}

/*^L*/
/*****************************************************************************
 Function:		PresetMacroDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if processed, else FALSE.
 Description:	Preset macro load/save dialog procedure.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 25/10/94 	| Created.											| PW
*****************************************************************************/

DIALOG_PROC PresetMacroDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	char						szFileName[MAXPATH];
	char						szRoot[MAXPATH];
	PSPD_DEF					pSpd;
	HWND						hwndDirList;
	int							num, i, nMyTop, nMyLeft;
	char						szBuf[100];
	RECT						DlgRect, DesktopRect;
	static PRESET_MACRO_TYPE	MacroType;
	static LPCSTR				lpszExt;

	switch (nMsg)
	{
		// Creation
		case WM_INITDIALOG:
		{
			// Reformat myself
			ReformatDialog (hWnd, RDF_THINFONT);

			// Get a pointer to my definition
			pSpd = (PSPD_DEF)lParam;

			// Create the file spec for my kind of files
			wsprintf (&szFileName[0], "%s\\*.%s", &szPresetMacroDirectory[0], pSpd->lpszExt);

			// Create an invisible listbox for listing the directory into
			hwndDirList = CreateWindow ("LISTBOX", "", WS_CHILD | LBS_SORT, 0, 0, 0, 0, hWnd, (HMENU)IDC_DIRLISTBOX, hInst, 0);

			// Do the directory listing into the listbox
			SendMessage (hwndDirList, LB_DIR, DDL_READWRITE, (LPARAM)(LPCSTR)&szFileName[0]);

			// Copy the directory listing (minus extensions) to the dialog
			num = (int)SendMessage (hwndDirList, LB_GETCOUNT, 0, 0);
			for (i=0 ; i<num ; i++)
			{
				SendMessage (hwndDirList, LB_GETTEXT, i, (LPARAM)(LPCSTR)&szFileName[0]);
				SPLITPATH (&szFileName[0], NULL, NULL, &szRoot[0], NULL);
				strupr (&szRoot[0]);
				SendDlgItemMessage (hWnd, IDC_MACRO_NAME_LISTBOX, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)&szRoot[0]);
			}

			// Get rid of the invisible directory listbox
			DestroyWindow (hwndDirList);

			// Set my title
			SetWindowText (hWnd, pSpd->lpszTitle);

			// Extract the type of preset macro being loaded
			MacroType = pSpd->type;

			// Take a copy of the file extension
			lpszExt = pSpd->lpszExt;

			// Get my window area
			GetWindowRect (hWnd, &DlgRect);

			// Get the desktop area
			GetWindowRect (GetDesktopWindow (), &DesktopRect);

			// Convert the pointer position to screen coordinates
			ClientToScreen (pSpd->hwndParent, &pSpd->ptTopLeft);

			// If my bottom might go off the bottom of the screen
			if (pSpd->ptTopLeft.y + (DlgRect.bottom - DlgRect.top) > DesktopRect.bottom)
			{
				// My top should be my height above the screen bottom
				nMyTop = DesktopRect.bottom - (DlgRect.bottom - DlgRect.top);
			}
			else
			{
				// My top should be the same as the pointer
				nMyTop = pSpd->ptTopLeft.y;
			}

			// If my right might go off the edge of the screen
			if (pSpd->ptTopLeft.x + (DlgRect.right - DlgRect.left) + 5 > DesktopRect.right)
			{
				// Put me to the left of the pointer
				nMyLeft = pSpd->ptTopLeft.x - (DlgRect.right - DlgRect.left) - 5;
			}
			else
			{
				// Put me to the right of the pointer
				nMyLeft = pSpd->ptTopLeft.x + 5;
			}

			// Move myself
			MoveWindow (hWnd, nMyLeft, nMyTop, DlgRect.right - DlgRect.left, DlgRect.bottom - DlgRect.top, FALSE);

			return (TRUE);
		}

		// Controls
		case WM_COMMAND:
		{
			// Handle by control type
			switch (LOWORD (wParam))
			{
				// Listbox
				case IDC_MACRO_NAME_LISTBOX:
				{
					// If this is a listbox selection
					if (NCODE == LBN_SELCHANGE)
					{
						// Copy the selected name to the editbox
						i = SendDlgItemMessage (hWnd, IDC_MACRO_NAME_LISTBOX, LB_GETCURSEL, 0, 0);
						SendDlgItemMessage (hWnd, IDC_MACRO_NAME_LISTBOX, LB_GETTEXT, i, (LPARAM)(LPSTR)&szBuf[0]);
						SendDlgItemMessage (hWnd, IDC_MACRO_NAME, WM_SETTEXT, 0, (LPARAM)(LPSTR)&szBuf[0]);
					}
					// If this is a double click
					else if (NCODE == LBN_DBLCLK)
					{
						// Read back the editbox
						SendDlgItemMessage (hWnd, IDC_MACRO_NAME, WM_GETTEXT, ARRAY_SIZE (szBuf), (LPARAM)(LPSTR)&szBuf[0]);

						// Call the loading bit
						if (LoadPresetMacro (&szBuf[0], lpszExt, MacroType))
						{
							EndDialog (hWnd, 0);
						}
					}

					return (TRUE);
				}

				// Load
				case IDC_MACRO_LOAD:
				{
					// Read back the editbox
					SendDlgItemMessage (hWnd, IDC_MACRO_NAME, WM_GETTEXT, ARRAY_SIZE (szBuf), (LPARAM)(LPSTR)&szBuf[0]);

					// Call the loading bit
					if (LoadPresetMacro (&szBuf[0], lpszExt, MacroType))
					{
						EndDialog (hWnd, 0);
					}

					return (TRUE);
				}

				// Save
				case IDC_MACRO_SAVE:
				{
					// Read back the editbox
					SendDlgItemMessage (hWnd, IDC_MACRO_NAME, WM_GETTEXT, ARRAY_SIZE (szBuf), (LPARAM)(LPSTR)&szBuf[0]);

					// Call the saving bit
					if (SavePresetMacro (&szBuf[0], lpszExt, MacroType))
					{
						EndDialog (hWnd, 0);
					}

					return (TRUE);
				}

				// Cancel
				case IDCANCEL:
				{
					EndDialog (hWnd, 0);
					return (TRUE);
				}

				// Unknown controls
				default:
				{
					break;
				}
			}
		}

		// Control about to be drawn
		case WM_CTLCOLOR:
#ifdef WINDOWS32
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLORSCROLLBAR:
		case WM_CTLCOLORSTATIC:
#endif
		{
			Ctl3dCtlColorEx (nMsg, wParam, lParam);
			break;
		}

		// Others
		default:
		{
			break;
		}
	}

	// Unprocessed
	return (FALSE);
}

/*^L*/
/*****************************************************************************
 Function:		LoadPresetMacro
 Parameters:	LPCSTR				lpszFileRoot	Root of file to load.
				LPCSTR				lpszFileExt		Extension of file to load.
				PRESET_MACRO_TYPE	type			Macro type to load.
 Returns:		BOOL				TRUE if OK, else FALSE.
 Description:	Loads a preset macro file.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/10/94 	| Created.											| PW
*****************************************************************************/

static BOOL LoadPresetMacro (LPCSTR lpszFileRoot, LPCSTR lpszFileExt, PRESET_MACRO_TYPE type)
{
	LPBYTE	lpbStart, lpbEnd;
	char	szFileName[MAXPATH];
	char	szReadExt[PM_EXT_LENGTH];
	char	szReadVersion[sizeof (szFileVersion)];
	WORD	wSum, wReadSum;
	HFILE	hFile;
	static PRESET_PARAMS	EditPresetCopy;

	// Create the load file name
	wsprintf (&szFileName[0], "%s\\%s.%s", &szPresetMacroDirectory[0], lpszFileRoot, lpszFileExt);

	// Open the file
	if ((hFile = _lopen (&szFileName[0], OF_READ)) == HFILE_ERROR)
	{
		MyMessageBox (hwndMain, ResourceString (IDS_MACRO_ERR_OPEN), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Read the file extension to the header
	if (_lread (hFile, szReadExt, PM_EXT_LENGTH) == (UINT)HFILE_ERROR)
	{
		MyMessageBox (hwndMain, ResourceString (IDS_MACRO_ERR_READ), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Check that it says the right thing
	if (memcmp (szReadExt, lpszFileExt, PM_EXT_LENGTH))
	{
		MyMessageBox (hwndMain, ResourceString (IDS_MACRO_ERR_WRONG_TYPE), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Read the file version to the header
	if (_lread (hFile, szReadVersion, lstrlen (szFileVersion)) == (UINT)HFILE_ERROR)
	{
		MyMessageBox (hwndMain, ResourceString (IDS_MACRO_ERR_READ), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Check that it says the right thing
	if (memcmp (szReadVersion, szFileVersion, lstrlen (szFileVersion)))
	{
		MyMessageBox (hwndMain, ResourceString (IDS_MACRO_ERR_WRONG_VERSION), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Handle by macro type
	switch (type)
	{
		case PMT_FG1:
		{
			// Get the start and end of FG1
			lpbStart	= (LPBYTE)&EditPreset.FunctionGenerator[0];
			lpbEnd		= lpbStart + sizeof (EditPreset.FunctionGenerator[0]);
			break;
		}

		case PMT_FG2:
		{
			// Get the start and end of FG2
			lpbStart	= (LPBYTE)&EditPreset.FunctionGenerator[1];
			lpbEnd		= lpbStart + sizeof (EditPreset.FunctionGenerator[1]);
			break;
		}

		case PMT_PRIMARY:
		{
			// Get the start and end of LAYER1
			lpbStart	= (LPBYTE)&EditPreset.Layer[0];
			lpbEnd		= lpbStart + sizeof (EditPreset.Layer[0]);
			break;
		}

		case PMT_SECONDARY:
		{
			// Get the start and end of LAYER2
			lpbStart	= (LPBYTE)&EditPreset.Layer[1];
			lpbEnd		= lpbStart + sizeof (EditPreset.Layer[1]);
			break;
		}

		case PMT_AUX_ENV:
		{
			// Get the start and end of AUXENV
			lpbStart	= (LPBYTE)&EditPreset.AuxEnv;
			lpbEnd		= lpbStart + sizeof (EditPreset.AuxEnv);
			break;
		}

		case PMT_NOTE_ON_MOD:
		{
			// Get the start and end of NOTE-ON mods
			lpbStart	= (LPBYTE)&EditPreset.NoteOnModulation[0];
			lpbEnd		= lpbStart + sizeof (EditPreset.NoteOnModulation);
			break;
		}

		case PMT_REAL_TIME_MOD:
		{
			// Get the start and end of REAL-TIME mods
			lpbStart	= (LPBYTE)&EditPreset.RealTimeModulation[0];
			lpbEnd		= lpbStart + sizeof (EditPreset.RealTimeModulation);
			break;
		}
	}

	// Take a copy of the edit preset
	memcpy (&EditPresetCopy, &EditPreset, sizeof (PRESET_PARAMS));

	// Read the preset data
	if (_lread (hFile, lpbStart, lpbEnd - lpbStart) == (UINT)HFILE_ERROR)
	{
		MyMessageBox (hwndMain, ResourceString (IDS_MACRO_ERR_READ), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		memcpy (&EditPreset, &EditPresetCopy, sizeof (PRESET_PARAMS));
		return (FALSE);
	}

	// Calculate the checksum
	wSum = CalcChecksum (lpbStart, (WORD)(lpbEnd - lpbStart));

	// Read the CHECKSUM
	_lread (hFile, &wReadSum, sizeof (WORD));

	// Close the file
	_lclose (hFile);

	// If the checksums don't match
	if (wSum != wReadSum)
	{
		// Get back the edit preset copy
		memcpy (&EditPreset, &EditPresetCopy, sizeof (PRESET_PARAMS));
		return (FALSE);
	}

	// Update the preset display
	DisplayPreset ();

	// Send the new parameters to the Morpheus
	SendChangedPresetParameters (&EditPresetCopy, &EditPreset);

	// Mark the preset as dirty
	fDirtyPreset = TRUE;

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		SavePresetMacro
 Parameters:	LPCSTR				lpszFileRoot	Root of file to save.
				LPCSTR				lpszFileExt		Extension of file to save.
				PRESET_MACRO_TYPE	type			Macro type to save.
 Returns:		BOOL				TRUE if OK, else FALSE.
 Description:	Saves a preset macro file.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/10/94 	| Created.											| PW
*****************************************************************************/

static BOOL SavePresetMacro (LPCSTR lpszFileRoot, LPCSTR lpszFileExt, PRESET_MACRO_TYPE type)
{
	LPBYTE	lpbStart, lpbEnd;
	char	szFileName[MAXPATH];
	WORD	wSum;
	HFILE	hFile;

	// Create the save file name
	wsprintf (&szFileName[0], "%s\\%s.%s", &szPresetMacroDirectory[0], lpszFileRoot, lpszFileExt);

	// If the file already exists
	if ((hFile = _lopen (&szFileName[0], OF_READ)) != HFILE_ERROR)
	{
		// Close the file
		_lclose (hFile);

		// Ask the user whether to overwrite it
		if (MyMessageBox (hwndMain,
					ResourceString (IDS_MACRO_OVERWRITE_Q),
					ResourceString (IDS_QUESTION_TITLE),
					MB_ICONQUESTION | MB_YESNO) == IDNO)
		{
			// Exit
			return (FALSE);
		}
	}

	// Create the file
	if ((hFile = _lcreat (&szFileName[0], 0)) == HFILE_ERROR)
	{
		MyMessageBox (hwndMain, ResourceString (IDS_MACRO_ERR_CREATING), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Write the file extension to the header
	if (_lwrite (hFile, lpszFileExt, lstrlen (lpszFileExt)) == (UINT)HFILE_ERROR)
	{
		MyMessageBox (hwndMain, ResourceString (IDS_MACRO_ERR_WRITING), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Write the file version to the header
	if (_lwrite (hFile, szFileVersion, lstrlen (szFileVersion)) == (UINT)HFILE_ERROR)
	{
		MyMessageBox (hwndMain, ResourceString (IDS_MACRO_ERR_WRITING), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Handle by macro type
	switch (type)
	{
		case PMT_FG1:
		{
			// Get the start and end of FG1
			lpbStart	= (LPBYTE)&EditPreset.FunctionGenerator[0];
			lpbEnd		= lpbStart + sizeof (EditPreset.FunctionGenerator[0]);
			break;
		}

		case PMT_FG2:
		{
			// Get the start and end of FG2
			lpbStart	= (LPBYTE)&EditPreset.FunctionGenerator[1];
			lpbEnd		= lpbStart + sizeof (EditPreset.FunctionGenerator[1]);
			break;
		}

		case PMT_PRIMARY:
		{
			// Get the start and end of LAYER1
			lpbStart	= (LPBYTE)&EditPreset.Layer[0];
			lpbEnd		= lpbStart + sizeof (EditPreset.Layer[0]);
			break;
		}

		case PMT_SECONDARY:
		{
			// Get the start and end of LAYER2
			lpbStart	= (LPBYTE)&EditPreset.Layer[1];
			lpbEnd		= lpbStart + sizeof (EditPreset.Layer[1]);
			break;
		}

		case PMT_AUX_ENV:
		{
			// Get the start and end of AUXENV
			lpbStart	= (LPBYTE)&EditPreset.AuxEnv;
			lpbEnd		= lpbStart + sizeof (EditPreset.AuxEnv);
			break;
		}

		case PMT_NOTE_ON_MOD:
		{
			// Get the start and end of NOTE-ON mods
			lpbStart	= (LPBYTE)&EditPreset.NoteOnModulation[0];
			lpbEnd		= lpbStart + sizeof (EditPreset.NoteOnModulation);
			break;
		}

		case PMT_REAL_TIME_MOD:
		{
			// Get the start and end of REAL-TIME mods
			lpbStart	= (LPBYTE)&EditPreset.RealTimeModulation[0];
			lpbEnd		= lpbStart + sizeof (EditPreset.RealTimeModulation);
			break;
		}
	}

	// Write the preset data
	if (_lwrite (hFile, lpbStart, lpbEnd - lpbStart) == (UINT)HFILE_ERROR)
	{
		MyMessageBox (hwndMain, ResourceString (IDS_MACRO_ERR_WRITING), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Calculate and write the file checksum
	wSum = CalcChecksum (lpbStart, (WORD)(lpbEnd - lpbStart));

	// Write the CHECKSUM
	_lwrite (hFile, (LPVOID)&wSum, sizeof (WORD));

	// Close the file
	_lclose (hFile);

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		SendChangedPresetParameters
 Parameters:	PPRESET_PARAMS	pOldPreset
				PPRESET_PARAMS	pNewPreset
 Returns:		None.
 Description:	Looks for parameters in the specified NEW preset that aren't
				the same as the specified OLD preset. For each changed
				parameter, sends it to the Morpheus (if connected).
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/10/94 	| Created.											| PW
*****************************************************************************/

static VOID SendChangedPresetParameters (PPRESET_PARAMS pOldPreset, PPRESET_PARAMS pNewPreset)
{
	int		i, nNumParams;
	LPWORD	pOldParam, pNewParam;

	// If we aren't connected
	if (!fConnectedToMorpheus)
	{
		// Do nothing
		return;
	}

	// How many parameters are there ?
	nNumParams = (LPWORD)(&pNewPreset->CheckSum) - (LPWORD)(&pNewPreset->Name[0]);

	// For each parameter
	for (	i=0, pOldParam = (LPWORD)&pOldPreset->Name[0], pNewParam = (LPWORD)&pNewPreset->Name[0] ;
			i<nNumParams ;
			i++, pOldParam++, pNewParam++)
	{
		// If the parameter has changed
		if (*pOldParam != *pNewParam)
		{
			// Send the preset parameter
			PresetSendParam (hwndMain, pNewParam);
		}
	}
}

