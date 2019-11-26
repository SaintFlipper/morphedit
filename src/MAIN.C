/*****************************************************************************
 File:			main.c
 Description:	The main module for the Morpheus editor.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 16/3/94	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"
#include "version.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define	DIALOG_ERROR			1
#define	MAX_MODELESS_DIALOGS	10
#define	STATUS_BAR_HEIGHT		16

#define	IDM_FIRST_MDI_CHILD		1000

/****************************************************************************/
/*								Local types									*/
/****************************************************************************/

typedef enum {NONE, PRESETS, HYPERPRESETS, MIDIMAPS, TUNING, PROGMAPS, MASTER} ETYPE;

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

static HINSTANCE	hMorpheusFonts;								// Morpheus dialog fonts
static HINSTANCE	hAudioControlDll;							// Audio control DLL
static HWND 		*phwndAllWindows[] =
{
	&hwndPresetsList,	&hwndCurrentPreset,	&hwndNoteOnPatches,	&hwndRealTimePatches,
	&hwndHypersList,	&hwndCurrentHyper,	&hwndMMsList,		&hwndCurrentMM,
	&hwndMaster,		&hwndTuning,		&hwndProgramMap,	&hwndPiano,
};
static UINT			nNumModelessDialogs = 0;

static struct
{
	LPCSTR	lpszClassName;
	FARPROC	lpfnWindowProc;
	LPCSTR	lpszIconName;
} WindowClassDefs[] =
{
	{WCNAME_PRESETS_LIST,		(FARPROC)PresetsListDialogProc,			"ICO_NAME_LIST"},
	{WCNAME_CURRENT_PRESET,		(FARPROC)CurrentPresetDialogProc,		"ICO_PRESET"},
	{WCNAME_NOTE_ON_PATCHES,	(FARPROC)NoteOnPatchBayWindowProc,		"ICO_NOTE_ON_PATCHBAY"},
	{WCNAME_REAL_TIME_PATCHES,	(FARPROC)RealTimePatchBayWindowProc,	"ICO_REAL_TIME_PATCHBAY"},
	{WCNAME_HYPERS_LIST,		(FARPROC)HypersListDialogProc,			"ICO_NAME_LIST"},
	{WCNAME_CURRENT_HYPER,		(FARPROC)CurrentHyperDialogProc,		"ICO_HYPER"},
	{WCNAME_MMS_LIST,			(FARPROC)MMsListDialogProc,				"ICO_NAME_LIST"},
	{WCNAME_CURRENT_MM,			(FARPROC)CurrentMMDialogProc,			"ICO_MIDIMAP"},
	{WCNAME_MASTER,				(FARPROC)MasterDialogProc,				"ICO_MASTER"},
	{WCNAME_TUNING,				(FARPROC)TuningDialogProc,				"ICO_TUNING"},
	{WCNAME_PROGRAM_MAP,		(FARPROC)ProgramMapDialogProc,			"ICO_PROGRAM_MAP"},
	{WCNAME_PIANO,				(FARPROC)PianoWindowProc,				"ICO_PIANO"},
};

// Status bar info
static char			szStatusBarText[5][50];
static POINT		ClientArea;
static BOOL			fDrawStatusBar = TRUE;

/****************************************************************************/
/*								Global data									*/
/****************************************************************************/

HINSTANCE	hInst;									// My instance
char		szMyPath[MAXPATH];						// My path (with trailing '\')
char		szIniFileName[MAXPATH];					// INI file name

BOOL		fConnectedToMorpheus = FALSE;			// TRUE if connected to Morpheus
BOOL		fYielding;								// TRUE if we're yielding during a long operation (eg. SYSEX)

// Global options

int			idMidiInput;							// Input device index
int			idMidiOutput;							// Output device index
BYTE		bMidiDeviceID;							// Morpheus MIDI device ID
UINT		nKeyboardLeftKey = (12 * 4);			// Test keyboard : key at left of window
BYTE		bKbdMidiChannel = 0;					// Test keyboard : MIDI channel
int			nTestKbdControllers[4];					// Test keyboard : MIDI controller assignments
char		szPresetMacroDirectory[MAXPATH] = ".";	// Directory where preset macros are stored
char		szStorageDirectory[MAXPATH] = ".";		// Directory where sounds etc. are stored

// Window handles (indentation implies window hierarchy)

HWND		hwndMain = (HWND)NULL;					// Main window
 HWND		hwndClient = (HWND)NULL;				// MDI client
  HWND		hwndPresetsList = (HWND)NULL;			// Presets -> list of presets
  HWND		hwndCurrentPreset = (HWND)NULL;			// Current preset window
  HWND		hwndNoteOnPatches = (HWND)NULL;			// Current preset -> NOTE-ON patches
  HWND		hwndRealTimePatches = (HWND)NULL;		// Current preset -> REAL-TIME patches
  HWND		hwndHypersList = (HWND)NULL;			// Hyperpresets -> list window
  HWND		hwndCurrentHyper = (HWND)NULL;			// Current hyperpreset editor
  HWND		hwndMMsList = (HWND)NULL;				// Midimaps->list
  HWND		hwndCurrentMM = (HWND)NULL;				// Current midimap editor
  HWND		hwndMaster = (HWND)NULL;				// Master settings window
  HWND		hwndTuning = (HWND)NULL;				// User tuning window
  HWND		hwndProgramMap = (HWND)NULL;			// Program map window
  HWND		hwndPiano = (HWND)NULL;					// Piano keyboard

 HWND		hwndLongOperation = (HWND)NULL;			// Message box for long MIDI operations

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

WINDOW_PROC			MainWindowProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
DIALOG_PROC			AboutDialog (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

static VOID			CloseMainChildren (ETYPE etype);
static BOOL			RegisterAllClasses (VOID);
static VOID			UnregisterAllClasses (VOID);
static VOID			PaintStatusBar (HWND hWnd);
static VOID			ConnectToMorpheus (HWND hWnd);

#if BETA
static BOOL BetaAbout (VOID);
#endif

/*^L*/
/*****************************************************************************
 Function:		WinMain
 Parameters:	HINSTANCE	hInstance		This instance handle.
				HINSTANCE	hPrevInstance	Any previous instance's handle.
				LPSTR		lpCmdLine		Command line.
				int			nCmdShow		How to display the main window.
 Returns:		int							Application return code.
 Description:	The application main entry point.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 16/3/94 	| Created.											| PW
*****************************************************************************/

int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG		Msg;
	char	szModuleName[MAXPATH], szDrive[MAXPATH], szDir[MAXDIR];

	// Save my instance
	hInst = hInstance;

	// Find my path
	GetModuleFileName (hInst, &szModuleName[0], sizeof (szModuleName));
	SPLITPATH (&szModuleName[0], &szDrive[0], &szDir[0], NULL, NULL);
	lstrcpy (&szMyPath[0], &szDrive[0]); 
	lstrcat (&szMyPath[0], &szDir[0]);

	// Initialise CTL3D
	Ctl3dRegister (hInstance);

	// Load the Morpheus fonts
	if (!AddFontResource ("morphdlg.fon"))
	{
		char	msg[512];
		wsprintf (msg, "Failed to load font 'morphdlg.fon'");
		MyMessageBox (NULL, msg, "Error", MB_ICONEXCLAMATION | MB_OK);
		return (-1);
	}

	// Initialise the audio controls
	if ((hAudioControlDll = LoadLibrary ("aucntrl.dll")) == (HINSTANCE)NULL)
	{
		// Error
		return (-1);
	}

	// Initialise the dialog utility module
	DlgUtilsInit ();

	// Create the INI file name including path
	strcpy (&szIniFileName[0], &szMyPath[0]);
	strcat (&szIniFileName[0], INI_FILE_NAME);

	// Load the other options
	LoadOptions ();

	// Open the MIDI devices
	if ((idMidiInput != NO_PARAM) && (idMidiOutput != NO_PARAM))
	{
		if (!morphdllOpenMidiDevices (idMidiInput, idMidiOutput))
		{
			// Error
			MyMessageBox (NULL, ResourceString (IDS_ERROR_OPENING_MIDI_DEVS), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			return (-1);
		}
	}
																							   
	// Register all classes
	if (!RegisterAllClasses ())
	{
		// Error
		MyMessageBox (NULL, ResourceString (IDS_ERROR_REG_CLASSES), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (-1);
	}

#if BETA
	// Display the BETA about box
	if (!BetaAbout ())
	{
		// Error
		return (-1);
	}
#endif
	
	// Create the main window
	if ((hwndMain = CreateWindow (
					MORPHEUS_MAIN_WINDOW_CLASS,
					ResourceString (IDS_MAIN_WINDOW_TITLE),
					WS_OVERLAPPED | WS_MAXIMIZE | WS_MAXIMIZEBOX | WS_MINIMIZEBOX |
						WS_SYSMENU | WS_THICKFRAME | WS_CLIPCHILDREN,
					CW_USEDEFAULT, CW_USEDEFAULT,
					CW_USEDEFAULT, CW_USEDEFAULT,
					HWND_DESKTOP,
					LoadMenu (hInst, "MENU_MAIN"),
					hInst,
					0)) == (HWND)NULL)
	{
		// Error
		MyMessageBox (NULL, ResourceString (IDS_ERROR_CREATING_MAIN_WND), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Create the test keyboard window
	CreatePianoWindow ();

	// Set the state of the "big preset dialog" flag
	SetBigPresetDialog (fBigPresetDialog);

	// Make the main window visible
	ShowWindow (hwndMain, SW_SHOWNA);

	// Try to read the Morpheus fixed internal information
	LoadInternals ();

	// Message processing loop
	while (GetMessage (&Msg, NULL, 0, 0))	// Until WM_QUIT message
	{
		TranslateMessage (&Msg);
		DispatchMessage (&Msg);
	}

	// Unregister all classes
	UnregisterAllClasses ();

	// Terminate the dialog utilities module
	DlgUtilsTerm ();

	// Unload DLLs
	FreeLibrary (hAudioControlDll);

	// Unload the Morpheus fonts
	RemoveFontResource ("morphdlg.fon");

	// Close the MIDI devices
	morphdllSetWindow ((HWND)NULL, (HWND)NULL);
	morphdllCloseMidiDevices ();

	// Uninitialise CTL3D
	Ctl3dUnregister (hInst);

	// Save the options
	StoreOptions ();

	// OK
	return (0);
}

/*^L*/
/*****************************************************************************
 Function:		MainWindowProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		LRESULT					Window proc return value.
 Description:	The main window procedure.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 21/3/94 	| Created.											| PW
*****************************************************************************/

WINDOW_PROC MainWindowProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	DLGPROC				pDlgProc;
	LRESULT				lRet;
	LPRECT				lpClientRect;
	static ETYPE		CurrentEdit = NONE;
	CLIENTCREATESTRUCT	ccs;

	switch (nMsg)
	{
		// Creation
		case WM_CREATE:
		{
			// Reposition myself if necessary
			hwndMain = hWnd;
			GetWindowPosition (MAIN_WINDOW, hWnd, TRUE);

			// Set the default non-client area status bar text
			strcpy (&szStatusBarText[0][0], ResourceString (IDS_MIDI_STATUS_IDLE));

			// Create the MDI client window
			ccs.hWindowMenu		= GetSubMenu (GetMenu (hWnd), 3);
			ccs.idFirstChild	= IDM_FIRST_MDI_CHILD;
			hwndClient = CreateWindow ("MDICLIENT", NULL,
							WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE |
								WS_VSCROLL | WS_HSCROLL,
/*
	NB. Much as it might be nice to include MDIS_ALLCHILDSTYLES, because it allows you to switch
	off thick frames, it seems to have the side effect that the 'Window' menu doesn't get
	the list of MDI windows.

							 | MDIS_ALLCHILDSTYLES,
 */
							0, 0, 0, 0, hWnd, (HMENU)1, hInst, (LPSTR)&ccs);

			return (0);
		}

		// Calculating client area size
		case WM_NCCALCSIZE:
		{
			lRet = DefWindowProc (hWnd, nMsg, wParam, lParam);
			lpClientRect = (LPRECT)lParam;
            if (!IsIconic (hWnd))
            {
				lpClientRect->bottom -= STATUS_BAR_HEIGHT;
				fDrawStatusBar = TRUE;
			}
			else
			{
				fDrawStatusBar = FALSE;
            }

			// Store the window client area
			ClientArea.x	= lpClientRect->right - lpClientRect->left;
			ClientArea.y	= lpClientRect->bottom - lpClientRect->top;

			return (lRet);
		}

		// Non-client painting
		case WM_NCPAINT:
		case WM_NCACTIVATE:
		{
			// Handle iconic case
			if (IsIconic (hWnd))
			{
				return (DefWindowProc (hWnd, nMsg, wParam, lParam));
			}

			// Draw most of the non-client bits
			lRet = DefWindowProc (hWnd, nMsg, wParam, lParam);

			// Draw the status bar area
			PaintStatusBar (hWnd);

            return (lRet);
		}

		// System command
		case WM_SYSCOMMAND:
		{
			// Do nothing if yielding
			if (fYielding)
				return (0);

			break;
		}
		
		// Controls
		case WM_COMMAND:
		{
			// Do nothing if yielding
			if (fYielding)
				return (0);

			switch (LOWORD (wParam))
			{
				// Menu: File->Load->All sounds
				case IDM_FILE_LOAD_ALL_SOUNDS:
				{
					// Load those sounds, why don't you
					LoadAllSounds ();
					break;
				}

				// Menu: File->Save->All sounds
				case IDM_FILE_SAVE_ALL_SOUNDS:
				{
					// Save those sounds, why don't you
					SaveAllSounds ();
					break;
				}

				// Menu: File->Load->Current preset
				case IDM_FILE_LOAD_CURRENT_PRESET:
				{
					if (nDisplayedPreset != INDEX_NONE)
					{
						LoadPreset (nDisplayedPreset);
					}
					break;
				}

				// Menu: File->Load->Current hyperpreset
				case IDM_FILE_LOAD_CURRENT_HYPERPRESET:
				{
					if (nDisplayedHyper != INDEX_NONE)
					{
						LoadHyper (nDisplayedHyper);
					}
					break;
				}

				// Menu: File->Load->Current midimap
				case IDM_FILE_LOAD_CURRENT_MIDIMAP:
				{
					if (nDisplayedMM != INDEX_NONE)
					{
						LoadMM (nDisplayedMM);
					}
					break;
				}

				// Menu: File->Load->Current midimap
				case IDM_FILE_LOAD_TUNING_TABLE:
				{
					LoadTuning ();
					break;
				}

				// Menu: File->Load->Current program map
				case IDM_FILE_LOAD_PROGRAM_MAP:
				{
					// If a program map is being displayed
					if (nSelectedProgramMap != INDEX_NONE)
					{
						// Load it
						LoadProgramMap (nSelectedProgramMap);
					}
					break;
				}

				// Menu: File->Save->Current preset
				case IDM_FILE_SAVE_CURRENT_PRESET:
				{
					if (nDisplayedPreset != INDEX_NONE)
					{
						SavePreset (nDisplayedPreset);
					}
					break;
				}

				// Menu: File->Save->Current hyperpreset
				case IDM_FILE_SAVE_CURRENT_HYPERPRESET:
				{
					if (nDisplayedHyper != INDEX_NONE)
					{
						SaveHyper (nDisplayedHyper);
					}
					break;
				}

				// Menu: File->Save->Current midimap
				case IDM_FILE_SAVE_CURRENT_MIDIMAP:
				{
					if (nDisplayedMM != INDEX_NONE)
					{
						SaveMM (nDisplayedMM);
					}
					break;
				}

				// Menu: File->Save->Tune table
				case IDM_FILE_SAVE_TUNING_TABLE:
				{
					SaveTuning ();
					break;
				}

				// Menu: File->Save->Current program map
				case IDM_FILE_SAVE_PROGRAM_MAP:
				{
					// If a program map is being displayed
					if (nSelectedProgramMap != INDEX_NONE)
					{
						// Save it
						SaveProgramMap (nSelectedProgramMap);
					}

					break;
				}

				// Menu: File->Export->Current preset
				case IDM_FILE_EXPORT_CURRENT_PRESET:
				{
					if (nDisplayedPreset != INDEX_NONE)
					{
						ExportPreset (nDisplayedPreset);
					}
					break;
				}

				// Menu: File->Export->Current hyperpreset
				case IDM_FILE_EXPORT_CURRENT_HYPERPRESET:
				{
					if (nDisplayedHyper != INDEX_NONE)
					{
						ExportHyper (nDisplayedHyper);
					}
					break;
				}

				// Menu: File->Exit
				case IDM_FILE_EXIT:
				{
					// Close child windows
					CloseMainChildren (CurrentEdit);

					// Close the piano window
					DestroyPianoWindow ();

					// Save the window position
					SaveWindowPosition (MAIN_WINDOW, hwndMain);

					// Destroy child windows, modeless dialogs, then, this window
					DestroyWindow (hWnd);
					PostQuitMessage (0);	// Quit the application
					return (0);
				}

				// Menu: Morpheus->Refresh all
				case IDM_REFRESH_ALL:
				{
					if (!MorpheusGetAll ())
					{
						MyMessageBox (NULL, ResourceString (IDS_ERROR_READING_MIDI_DATA), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
					}
					return (0);
				}

				// Menu: Morpheus->Refresh internals
				case IDM_REFRESH_INTERNALS:
				{
					// Fetch the Morpheus fixed internal information
					if (!MorpheusGetInternals ())
					{
						MyMessageBox (NULL, ResourceString (IDS_ERROR_READING_MIDI_DATA), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
					}

					// Save it
					if (!SaveInternals ())
					{
						MyMessageBox (NULL, ResourceString (IDS_ERROR_STORING_INTERNALS), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
					}

					return (0);
				}

				// Menu: Options->Connect to Morpheus
				case IDM_OPTIONS_CONNECT_TO_MORPHEUS:
				{
					// Do the connection
					ConnectToMorpheus (hWnd);

					return (0);
				}

				// Menu: Options->Big preset dialog
				case IDM_OPTIONS_BIG_PRESET_DIALOG:
				{
					DWORD	dwSize;
					
					// Invert the state of the big dialog flag
					fBigPresetDialog = fBigPresetDialog ? FALSE : TRUE;

					// Set the new preset dialog state
					dwSize = SetBigPresetDialog (fBigPresetDialog);

					// If the preset window is on display
					if (hwndCurrentPreset != (HWND)NULL)
					{
						// Tell the preset window its new size
						SendMessage (hwndCurrentPreset, WM_USER_SET_WINDOW_SIZE, 0, (LPARAM)dwSize);
						SetWindowPos (hwndCurrentPreset, hwndMain, 0, 0,
							HIWORD (dwSize), LOWORD (dwSize), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
					}

					return (0);
				}

				// Menu: Options->MIDI
				case IDM_OPTIONS_MIDI:
				{
					// Invoke the MIDI options dialog
					pDlgProc = MakeProcInstance (MidiOptionsDialog, hInst);
					DialogBox (hInst, "DLG_MIDI_OPTIONS", hWnd, pDlgProc);
					FreeProcInstance (pDlgProc);
					return (0);
				}

				// Menu: Edit->Presets
				case IDM_EDIT_PRESETS:
				{
					// If we aren't already editing presets
					if (CurrentEdit != PRESETS)
					{
						// Close child windows
						CloseMainChildren (CurrentEdit);

						// Create the preset editing windows
						if (!CreateAllPresetWindows ())
						{
							// Couldn't switch editor mode
							CurrentEdit = NONE;
							MyMessageBox (NULL, ResourceString (IDS_ERROR_CREATING_PS_WNDS), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
                            break;
						}
						CheckMenuItem (GetMenu (hWnd), IDM_EDIT_PRESETS, MF_CHECKED);
						CurrentEdit = PRESETS;
					}
					break;
				}

				// Menu: Edit->Hyperpresets
				case IDM_EDIT_HYPERPRESETS:
				{
					// If we aren't already editing hyperpresets
					if (CurrentEdit != HYPERPRESETS)
					{
						// Close child windows
						CloseMainChildren (CurrentEdit);

						// Create the hyperpreset editing windows
						if (!CreateAllHyperWindows ())
						{
							// Couldn't switch editor mode
							CurrentEdit = NONE;
							MyMessageBox (NULL, ResourceString (IDS_ERROR_CREATING_HPS_WNDS), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
                            break;
						}
						CheckMenuItem (GetMenu (hWnd), IDM_EDIT_HYPERPRESETS, MF_CHECKED);
						CurrentEdit = HYPERPRESETS;
					}
					break;
				}

				// Menu: Edit->Midimaps
				case IDM_EDIT_MIDIMAPS:
				{
					// If we aren't already editing midimaps
					if (CurrentEdit != MIDIMAPS)
					{
						// Close child windows
						CloseMainChildren (CurrentEdit);

						// Create the midmaps windows
						if (!CreateAllMMWindows ())
						{
							// Couldn't switch editor mode
							CurrentEdit = NONE;
							MyMessageBox (NULL, ResourceString (IDS_ERROR_CREATING_MMS_WNDS), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
							break;
						}
						CheckMenuItem (GetMenu (hWnd), IDM_EDIT_MIDIMAPS, MF_CHECKED);
						CurrentEdit = MIDIMAPS;
					}
					break;
				}

				// Menu: Edit->Master settings
				case IDM_EDIT_MASTER_SETTINGS:
				{
					// If we aren't already editing master settings
					if (CurrentEdit != MASTER)
					{
						// Close child windows
						CloseMainChildren (CurrentEdit);

						// Create the master settings dialog
						if (!CreateMasterWindow ())
						{
							// Couldn't switch editor mode
							CurrentEdit = NONE;
							break;
						}
						CheckMenuItem (GetMenu (hWnd), IDM_EDIT_MASTER_SETTINGS, MF_CHECKED);
						CurrentEdit = MASTER;
					}
					break;
				}

				// Menu: Edit->User tuning
				case IDM_EDIT_USER_TUNING:
				{
					// If we aren't already editing ther user tuning table
					if (CurrentEdit != TUNING)
					{
						// Close child windows
						CloseMainChildren (CurrentEdit);

						// Create the user tuning table editing window
						if (!CreateUserTuningWindow ())
						{
							// Couldn't switch editor mode
							CurrentEdit = NONE;
							break;
						}
						CheckMenuItem (GetMenu (hWnd), IDM_EDIT_USER_TUNING, MF_CHECKED);
						CurrentEdit = TUNING;
					}
					break;
				}

				// Menu: Edit->Program maps
				case IDM_EDIT_PROGRAM_MAPS:
				{
					// If we aren't already editing the program maps
					if (CurrentEdit != PROGMAPS)
					{
						// Close child windows
						CloseMainChildren (CurrentEdit);

						// Create the program map editing window
						if (!CreateProgramMapWindow ())
						{
							// Couldn't switch editor mode
							CurrentEdit = NONE;
							break;
						}
						CheckMenuItem (GetMenu (hWnd), IDM_EDIT_PROGRAM_MAPS, MF_CHECKED);
						CurrentEdit = PROGMAPS;
					}
					break;
				}

				// Menu: Window->Arrange icons
				case IDM_WINDOW_ARRANGE_ICONS:
				{
					SendMessage (hwndClient, WM_MDIICONARRANGE, 0, 0);
					break;
				}

				// Menu: Window->Cascade
				case IDM_WINDOW_CASCADE:
				{
					SendMessage (hwndClient, WM_MDICASCADE, MDITILE_SKIPDISABLED, 0);
					break;
				}

				// Menu: Help->About
				case IDM_HELP_ABOUT:
				{
					// Invoke the ABOUT dialog
					pDlgProc = MakeProcInstance (AboutDialog, hInst);
					DialogBox (hInst, "DLG_ABOUT", hWnd, pDlgProc);
					FreeProcInstance (pDlgProc);
					return (0);
				}

				// Unknown controls
				default:
				{
					break;
				}
			}

			break;
		}

		// Size
		case WM_SIZE:
		{
			break;
		}

		// Close
		case WM_CLOSE:
		{
			// Close child windows
			CloseMainChildren (CurrentEdit);

			// Close the piano window
			DestroyPianoWindow ();

			// Save the window position
			SaveWindowPosition (MAIN_WINDOW, hwndMain);

			// Destroy child windows, modeless dialogs, then, this window
			DestroyWindow (hWnd);
			PostQuitMessage (0);	// Quit the application
			return (0);
		}

		// Destruction
		case WM_DESTROY:
		{
			DestroyMenu (GetMenu (hWnd));
			break;
		}

		// System colour change
		case WM_SYSCOLORCHANGE:
		{
			Ctl3dColorChange ();
			break;
		}

		default:
		{
			break;
		}
	}

	// Default processing
	return (DefFrameProc (hWnd, hwndClient, nMsg, wParam, lParam));
}

/*^L*/
/*****************************************************************************
 Function:		CloseMainChildren
 Parameters:	ETYPE	etype			Current edit type.
 Returns:		None.
 Description:	Closes the appropriate set of editing windows.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/5/94 	| Created.											| PW
*****************************************************************************/

static VOID CloseMainChildren (ETYPE etype)
{
	switch (etype)
	{
		case PRESETS:
			CheckMenuItem (GetMenu (hwndMain), IDM_EDIT_PRESETS, MF_UNCHECKED);
			DestroyAllPresetWindows ();
			break;
		case HYPERPRESETS:
			CheckMenuItem (GetMenu (hwndMain), IDM_EDIT_HYPERPRESETS, MF_UNCHECKED);
			DestroyAllHyperWindows ();
			break;
		case MIDIMAPS:
			CheckMenuItem (GetMenu (hwndMain), IDM_EDIT_MIDIMAPS, MF_UNCHECKED);
			DestroyAllMMWindows ();
			break;
		case MASTER:
			CheckMenuItem (GetMenu (hwndMain), IDM_EDIT_MASTER_SETTINGS, MF_UNCHECKED);
			DestroyMasterWindow ();
			break;
		case TUNING:
			CheckMenuItem (GetMenu (hwndMain), IDM_EDIT_USER_TUNING, MF_UNCHECKED);
			DestroyUserTuningWindow ();
			break;
		case PROGMAPS:
			CheckMenuItem (GetMenu (hwndMain), IDM_EDIT_PROGRAM_MAPS, MF_UNCHECKED);
			DestroyProgramMapWindow ();
			break;
		case NONE:
		default:
			break;
	}
}

/*^L*/
/*****************************************************************************
 Function:		RegisterAllClasses
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Registers all classes required by the application.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 27/3/94 	| Created.											| PW
*****************************************************************************/

static BOOL RegisterAllClasses (VOID)
{
	WNDCLASS	WndClass;
	UINT		i;

	/*************************************************************************
		Main normal windows
	**************************************************************************/

	// Register the main window class
	WndClass.style			= 0;	// CS_NOCLOSE;
	WndClass.lpfnWndProc	= MainWindowProc;
	WndClass.cbClsExtra		= 0;
	WndClass.cbWndExtra		= 0;
	WndClass.hInstance		= hInst;
	WndClass.hIcon			= LoadIcon (hInst, "ICO_APP2");
	WndClass.hCursor		= LoadCursor (NULL, IDC_ARROW);
	WndClass.hbrBackground	= (HBRUSH)(COLOR_BACKGROUND + 1);
	WndClass.lpszMenuName	= NULL;
	WndClass.lpszClassName	= MORPHEUS_MAIN_WINDOW_CLASS;
	if (!RegisterClass (&WndClass))
	{
		// Error
		return (FALSE);
	}

	// Register the invisible MIDI sysex window class
	WndClass.style			= CS_BYTEALIGNWINDOW;
	WndClass.lpfnWndProc	= (WNDPROC)SysexDumpWindowProc;
	WndClass.cbClsExtra		= 0;
	WndClass.cbWndExtra		= 0;
	WndClass.hInstance		= hInst;
	WndClass.hIcon			= NULL;
	WndClass.hCursor		= NULL;
	WndClass.hbrBackground	= NULL;
	WndClass.lpszMenuName	= NULL;
	WndClass.lpszClassName	= MORPHEUS_SYSEX_WINDOW_CLASS;
	if (!RegisterClass (&WndClass))
	{
		// Error
		return (FALSE);
	}

	// Register the message eater
	WndClass.style         = 0;
	WndClass.lpfnWndProc   = (WNDPROC)InvisoWndProc;
	WndClass.cbClsExtra    = 0;
	WndClass.cbWndExtra    = 0;
	WndClass.hInstance     = hInst;
	WndClass.hIcon         = NULL;
	WndClass.hCursor       = LoadCursor (NULL, IDC_WAIT);
	WndClass.hbrBackground = GetStockObject (NULL_BRUSH);
	WndClass.lpszMenuName  = (LPSTR)NULL;
	WndClass.lpszClassName = WCNAME_MOUSE_EATER;
	if (!RegisterClass (&WndClass))
	{
		// Error
		return (FALSE);
	}

	// For each MDI window class to define
	for (i=0 ; i<ARRAY_SIZE(WindowClassDefs) ; i++)
	{
		WndClass.style			= CS_BYTEALIGNWINDOW;
		WndClass.lpfnWndProc	= (WNDPROC)WindowClassDefs[i].lpfnWindowProc;
		WndClass.cbClsExtra		= 0;
		WndClass.cbWndExtra		= 0;
		WndClass.hInstance		= hInst;
		WndClass.hIcon			= LoadIcon (hInst, WindowClassDefs[i].lpszIconName);
		WndClass.hCursor		= LoadCursor (NULL, IDC_ARROW);
		WndClass.hbrBackground	= (HBRUSH)(COLOR_BTNFACE + 1);
		WndClass.lpszMenuName	= NULL;
		WndClass.lpszClassName	= WindowClassDefs[i].lpszClassName;
		if (!RegisterClass (&WndClass))
		{
			// Error
			return (FALSE);
		}
	}

	// Register the static bitmap class
	if (!StaticBitmapControlInit (hInst))
	{
		// Error
		return (FALSE);
	}

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		UnregisterAllClasses
 Parameters:	None.
 Returns:		None.
 Description:	Unregisters all classes required by the application.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 27/3/94 	| Created.											| PW
*****************************************************************************/

static VOID UnregisterAllClasses (VOID)
{
	int		i;

	// Unregister the MAIN class
	UnregisterClass (MORPHEUS_MAIN_WINDOW_CLASS, hInst);

	// Unregister the invisible SYSEX class
	UnregisterClass (MORPHEUS_SYSEX_WINDOW_CLASS, hInst);

	// For each MDI window class to define
	for (i=0 ; i<ARRAY_SIZE(WindowClassDefs) ; i++)
	{
		// Unregister its class
		UnregisterClass (WindowClassDefs[i].lpszClassName, hInst);
	}
	
	// Unregister the static bitmap class
	StaticBitmapControlTerm (hInst);
}

/*^L*/
/*****************************************************************************
 Function:		AboutDialog
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if message processed, else FALSE
 Description:	The ABOUT dialog procedure.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 8/5/94 	| Created.											| PW
*****************************************************************************/

DIALOG_PROC AboutDialog (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	char			szRevision[50];
	static HFONT	hfontTitle;

	switch (nMsg)
	{
		case WM_INITDIALOG:
		{
			// Reformat myself
			ReformatDialog (hWnd, RDF_CENTRE | RDF_THINFONT);

			// Display the editor version
			wsprintf (	&szRevision[0],
						ResourceString (IDS_VERSION_MASK),
						VERSION_MAJOR, VERSION_MINOR, BETA ? " beta" : "");
#if defined (WINDOWS32)
			lstrcat (&szRevision[0], " (Win32)");
#else
			lstrcat (&szRevision[0], " (Win16)");
#endif
			SetDlgItemText (hWnd, IDC_ABOUT_EDITOR_VERSION, (LPCSTR)&szRevision[0]);

			// Display the Morpheus firmware version number
			wsprintf (	&szRevision[0], "%c.%c%c",
						MorpheusVersionData.Revision[0],
						MorpheusVersionData.Revision[1],
						MorpheusVersionData.Revision[2]);
			SetDlgItemText (hWnd, IDC_REVISION, (LPCSTR)&szRevision[0]);

			// Indicate whether a data card is installed
			SetDlgItemText (hWnd, IDC_DATA_CARD_INSTALLED,
				(LPCSTR)(WNUM (MorpheusConfigurationData.wNumPresets) > 256 ?
				ResourceString (IDS_YES_STRING) :
				ResourceString (IDS_NO_STRING)));

			// Create a font for the title and use it
			hfontTitle = CreateFont (-14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
			SendDlgItemMessage (hWnd, IDC_ABOUT_NAME, WM_SETFONT, (WPARAM)hfontTitle, MAKELPARAM (0, 0));
			
			return (TRUE);
		}

		// Control
		case WM_COMMAND:
		{
			if (LOWORD (wParam) == IDOK)
			{
				EndDialog (hWnd, 0);
				return (TRUE);
			}
			break;
		}

		// Destruction of the window
		case WM_DESTROY:
		{
			DeleteObject (hfontTitle);
			return (TRUE);
		}

		// Control about to be drawn
		case WM_CTLCOLOR:
		{
			Ctl3dCtlColorEx (nMsg, wParam, lParam);
			break;
		}

		default:
		{
			break;
		}
	}
	return (FALSE);
}

/*^L*/
/*****************************************************************************
 Function:		DisplayStatusText
 Parameters:	int		nIndex			Status bar box (0..4)
				LPCSTR	lpszString		Stringt to display.
 Returns:		None.
 Description:	Updates some status bar text.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 15/5/94 	| Created.											| PW
*****************************************************************************/

VOID DisplayStatusText (int nIndex, LPCSTR lpszString)
{
	// Make a copy of the string
	lstrcpy (&szStatusBarText[nIndex][0], lpszString);

	// Redraw the status bar
	PaintStatusBar (hwndMain);
}

/*^L*/
/*****************************************************************************
 Function:		PaintStatusBar
 Parameters:	HWND	hWnd			Parent window.
 Returns:		None.
 Description:	Paints the status bar, which is in the non-client area.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 15/5/94 	| Created.											| PW
*****************************************************************************/

static VOID PaintStatusBar (HWND hWnd)
{
	HDC			hDC;
	HBRUSH		hBrush, hOldBrush;
	HPEN		hOldPen;
	COLORREF	OldTextColour;
	int			nOldBkMode;
	HFONT		hOldFont;
	RECT		Rect;
	DWORD		dwStyle = GetWindowLong (hWnd, GWL_STYLE);

	// If we are to draw the status bar 
	if (fDrawStatusBar)
	{
		// Get the DC of the whole parent window
		hDC = GetWindowDC (hWnd);

		// Calculate where the status bar rectangle is with respect to the parent
		Rect.left	= 0;
		Rect.top	= ClientArea.y;

		if (dwStyle & WS_CAPTION)
		{
			Rect.top	+= GetSystemMetrics (SM_CYCAPTION);
		}

		if ((dwStyle & WS_THICKFRAME)/* && (!IsZoomed (hWnd))*/)
		{
			Rect.left	+= GetSystemMetrics (SM_CXFRAME);
			Rect.top	+= GetSystemMetrics (SM_CYFRAME);
		}

		if (GetMenu (hWnd) != (HMENU)NULL)
		{
			Rect.top	+= GetSystemMetrics (SM_CYMENU);
		}

		Rect.right	= Rect.left + ClientArea.x;
		Rect.bottom	= Rect.top + STATUS_BAR_HEIGHT;

		// Draw the background
		hBrush = CreateSolidBrush (GetSysColor (COLOR_BTNFACE));
		hOldBrush = SelectObject (hDC, hBrush);
		hOldPen = SelectObject (hDC, GetStockObject (BLACK_PEN));
		Rectangle (hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);
		SelectObject (hDC, hOldPen);
		SelectObject (hDC, hOldBrush);
		DeleteObject (hBrush);

		// Draw the status bar text
		OldTextColour = SetTextColor (hDC, GetSysColor (COLOR_BTNTEXT));
		nOldBkMode = SetBkMode (hDC, TRANSPARENT);
		hOldFont = SelectObject (hDC, hDialogFont);
		TextOut (hDC, Rect.left + 2, Rect.top + 2, &szStatusBarText[0][0], strlen (&szStatusBarText[0][0]));
		SelectObject (hDC, hOldFont);
		SetBkMode (hDC, nOldBkMode);
		SetTextColor (hDC, OldTextColour);

		// Release the DC
		ReleaseDC (hWnd, hDC);
	}
}

/*^L*/
/*****************************************************************************
 Function:		ConnectToMorpheus
 Parameters:	HWND	hWnd			Parent window.
 Returns:		None.
 Description:	1)	Asks the user whether they really want to connect, as
					they may not have a Morpheus connected. 
				2)	Checks whether any Presets, Hyperpresets, or Midimaps are
					already loaded. If they have then they will be overwritten
					by the connection, so again ask the user is they are sure.
				3)	Fetches the Morpheus internal setup, Presets list,
					Hyperpresets list, and Midimap list.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 15/6/94 	| Created.											| PW
*****************************************************************************/

static VOID ConnectToMorpheus (HWND hWnd)
{
	static HWND		hwndMouseEater;
	static HMENU	hmenuMain, hMenu;
	static DWORD	dwStyle;

	// If we are currently connected to the Morpheus
	if (fConnectedToMorpheus)
	{
		// Set to 'unconnected'
		fConnectedToMorpheus = FALSE;

		// Uncheck the menu option
		CheckMenuItem (GetMenu (hWnd), IDM_OPTIONS_CONNECT_TO_MORPHEUS, MF_UNCHECKED);

		// That's it
		return;
	}

	// Ask whether they are sure.
	if (MyMessageBox (hWnd, ResourceString (IDS_CONNECT_QUERY),
		ResourceString (IDS_QUESTION_TITLE), MB_ICONQUESTION | MB_YESNO) == IDNO)
	{
		// No
		return;
	}

	// If any names list exists
	if ((pMorpheusPresetList != NULL) || (pMorpheusHyperpresetList != NULL) || (pMorpheusMidimapList != NULL))
	{
		if (MyMessageBox (hWnd, ResourceString (IDS_CONNECT_OVERWRITE_QUERY),
			ResourceString (IDS_QUESTION_TITLE), MB_ICONQUESTION | MB_YESNO) == IDNO)
		{
			// No
			return;
		}
	}

	// Create the invisible mouse-eater
	hwndMouseEater = CreateMouseEater (hwndMain);

	// Disable the menu
	hmenuMain = GetMenu (hwndMain);
	SetMenu (hwndMain, (HMENU)NULL);

	// Get rid of the main window system menu
	dwStyle = GetWindowLong (hWnd, GWL_STYLE);
	SetWindowLong (hwndMain, GWL_STYLE, dwStyle & ~WS_SYSMENU);

//	EnableAllMenuItems (hwndMain, FALSE);

	// Fetch the Morpheus fixed internal information
	if (!MorpheusGetInternals ())
	{
		MyMessageBox (hWnd, ResourceString (IDS_ERROR_READING_MIDI_DATA),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		DestroyWindow (hwndMouseEater);
		SetMenu (hwndMain, hmenuMain);
		SetWindowLong (hwndMain, GWL_STYLE, dwStyle);
		return;
	}

	// Save it
	if (!SaveInternals ())
	{
		MyMessageBox (hWnd, ResourceString (IDS_ERROR_STORING_INTERNALS),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		DestroyWindow (hwndMouseEater);
		SetMenu (hwndMain, hmenuMain);
		SetWindowLong (hwndMain, GWL_STYLE, dwStyle);
		return;
	}

	// Read the name lists
	if (!MorpheusGetAll ())
	{
		MyMessageBox (hWnd, ResourceString (IDS_ERROR_READING_NAMELISTS), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		DestroyWindow (hwndMouseEater);
		SetMenu (hwndMain, hmenuMain);
		SetWindowLong (hwndMain, GWL_STYLE, dwStyle);
		return;
	}

	// Set to the 'connected' state
	fConnectedToMorpheus = TRUE;

	// Get rid of the mouse-eating window and reset the menu
	DestroyWindow (hwndMouseEater);
	SetMenu (hwndMain, hmenuMain);
	SetWindowLong (hwndMain, GWL_STYLE, dwStyle);

	// Check the menu option
	CheckMenuItem (GetMenu (hWnd), IDM_OPTIONS_CONNECT_TO_MORPHEUS, MF_CHECKED);
}

#if BETA

DIALOG_PROC BetaAboutDialog (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	char				szRevision[50];
	static HFONT		hfontTitle;
	static const char	BetaString[] =
				"This is pre-release Beta test software and comes "
				"with no explicit or implicit guarantee of suitability for "
				"any use or function. Please contact the author for "
				"a full release of this software, including printed "
				" manuals and support.";

	switch (nMsg)
	{
		case WM_INITDIALOG:
		{
			// Display the editor version
			wsprintf (	&szRevision[0],
						ResourceString (IDS_VERSION_MASK),
						VERSION_MAJOR, VERSION_MINOR, BETA ? "(Beta)" : "");
			SetDlgItemText (hWnd, IDC_ABOUT_EDITOR_VERSION, (LPCSTR)&szRevision[0]);

			// Display the BETA info
			SetDlgItemText (hWnd, IDC_ABOUT_EXTRA1, (LPCSTR)BetaString);
			
			// Check that someone hasn't knobbled the beta string
			if (lstrlen (BetaString) != sizeof (BetaString) - 1)
			{
				EndDialog (hWnd, -1);
				break;
			}

			// Check that someone hasn't deleted the beta control
			if (GetDlgItem (hWnd, IDC_ABOUT_EXTRA1) == (HWND)NULL)
			{
//				EndDialog (hWnd, -1);
//				break;
			}

			// Reformat myself
			ReformatDialog (hWnd, RDF_CENTRE | RDF_THINFONT);

			// Create a font for the title and use it
			hfontTitle = CreateFont (-20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
			SendDlgItemMessage (hWnd, IDC_ABOUT_NAME, WM_SETFONT, (WPARAM)hfontTitle, MAKELPARAM (0, 0));
			
			return (TRUE);
		}

		// Control
		case WM_COMMAND:
		{
			if (LOWORD (wParam) == IDOK)
			{
				EndDialog (hWnd, 0);
				return (TRUE);
			}
			break;
		}


		// Destruction of the window
		case WM_DESTROY:
		{
			DeleteObject (hfontTitle);

			return (TRUE);
		}

		// Control about to be drawn
		case WM_CTLCOLOR:
		{
			Ctl3dCtlColorEx (nMsg, wParam, lParam);
		}

		default:
		{
			break;
		}
	}
	return (FALSE);
}

static BOOL BetaAbout (VOID)
{
	DLGPROC		lpDialogProc;
	int			nRet;

	// Invoke the MIDI options dialog
	lpDialogProc = MakeProcInstance (BetaAboutDialog, hInst);
	nRet = DialogBox (hInst, "DLG_BETA_ABOUT", (HWND)NULL, lpDialogProc);
	FreeProcInstance (lpDialogProc);
	return (nRet == -1 ? FALSE : TRUE);
}

#endif

