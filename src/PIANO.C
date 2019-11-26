/*****************************************************************************
 File:			piano.c
 Description:	Morpheus editor 'piano keyboard' window. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 4/6/94		| Created.											| PW
*****************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#include <morphdll.h>
#include <aucntrl.h>
#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define	IDC_VELOCITY			100
#define	IDC_MIDI_A				101
#define	IDC_MIDI_B				102
#define	IDC_MIDI_C				103
#define	IDC_MIDI_D				104

#define	KEYS_X_OFFSET			(5 * SLIDER_CONTROL_WIDTH)

#define	WHITE_KEY_WIDTH			16
#define	WHITE_KEY_HEIGHT		64
#define	BLACK_KEY_WIDTH			10
#define	BLACK_KEY_HEIGHT		32

// ROP codes
#define	DSa						0x008800C6L
#define	DSx						0x00660046L

#define	PIANO_WINDOW_CLASS		"PianoKeyboard"

/****************************************************************************/
/*								Local types									*/
/****************************************************************************/

typedef enum {BLACK_KEY, WHITE_KEY} KEY_TYPE;

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

// Key bitmaps
static HBITMAP	hbmpKeyAUp, hbmpKeyADown;
static HBITMAP	hbmpKeyBUp, hbmpKeyBDown;
static HBITMAP	hbmpKeyCUp, hbmpKeyCDown;
static HBITMAP	hbmpKeyDUp, hbmpKeyDDown;

// Key order within 1 octave
static struct
{
	KEY_TYPE	Type;
	HBITMAP		*phbmpUp;
	HBITMAP		*phbmpDown;
} KeyTypes[12] =
{
	{WHITE_KEY,	&hbmpKeyAUp, &hbmpKeyADown},
	{BLACK_KEY,	&hbmpKeyDUp, &hbmpKeyDDown},
	{WHITE_KEY,	&hbmpKeyBUp, &hbmpKeyBDown},
	{BLACK_KEY,	&hbmpKeyDUp, &hbmpKeyDDown},
	{WHITE_KEY,	&hbmpKeyCUp, &hbmpKeyCDown},
	{WHITE_KEY, &hbmpKeyAUp, &hbmpKeyADown},
	{BLACK_KEY,	&hbmpKeyDUp, &hbmpKeyDDown},
	{WHITE_KEY,	&hbmpKeyBUp, &hbmpKeyBDown},
	{BLACK_KEY,	&hbmpKeyDUp, &hbmpKeyDDown},
	{WHITE_KEY,	&hbmpKeyBUp, &hbmpKeyBDown},
	{BLACK_KEY,	&hbmpKeyDUp, &hbmpKeyDDown},
	{WHITE_KEY, &hbmpKeyCUp, &hbmpKeyCDown},
};

// Positions of keys
static struct
{
	RECT		rcKey;
	KEY_TYPE	Type;
	HBITMAP		hBitMapUp;
	HBITMAP		hBitMapDown;
} KeyInfo[NUM_MIDI_NOTES];

// Window size
static POINT	nWindowSize;

// Key that is currently down
static int		nDownKey = INDEX_NONE;

// Keyboard velocity
static UINT		nKeyboardVelocity = 127;

/****************************************************************************/
/*								Global data									*/
/****************************************************************************/


/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

static int InKey (int x, int y);
static VOID PressKey (HWND hWnd, int nMouseNote);
static VOID ReleaseKey (HWND hWnd, int nMouseNote);
static VOID SendController (int nController, int nValue);
static VOID CalculateKeyInfo (VOID);
static VOID PaintNote (HDC hDC, int nNote, BOOL fDown, UINT nBaseX);
static VOID PaintKeyboard (HDC hDC);
static BOOL TransBlt (HDC hdcD, int x, int y, int dx, int dy, HDC hdcS, int x0, int y0);

/*^L*/
/*****************************************************************************
 Function:		CreatePianoWindow
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Creates the piano window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 4/6/94 	| Created.											| PW
*****************************************************************************/

BOOL CreatePianoWindow (VOID)
{
	MDICREATESTRUCT	mc;
	WINDOWPLACEMENT	wp;

	// Create the window
	GetCreatePosition (PIANO_WINDOW, TRUE, &wp);
	mc.szClass	= WCNAME_PIANO;
	mc.szTitle	= ResourceString (IDS_KBD_WINDOW_TITLE);
	mc.hOwner	= hInst;
	mc.x		= wp.rcNormalPosition.left;
	mc.y		= wp.rcNormalPosition.top;
	mc.cx		= 200;
	mc.cy		= SLIDER_CONTROL_HEIGHT;
	mc.style	= WS_CHILD | WS_CAPTION | WS_CLIPSIBLINGS | WS_MINIMIZEBOX | WS_THICKFRAME | WS_SYSMENU |
					(wp.showCmd & SW_SHOWMINIMIZED ? WS_MINIMIZE : 0);
	mc.lParam	= 0;
	if ((hwndPiano = (HWND)SendMessage (hwndClient,
			WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT)&mc)) == (HWND)NULL)
	{
		// Error
		MyMessageBox (NULL, ResourceString (IDS_KBD_ERR_CREAT_WND), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Reposition the window
	GetWindowPosition (PIANO_WINDOW, hwndPiano, TRUE);

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		DestroyPianoWindow
 Parameters:	None.
 Returns:		None.
 Description:	Destroys the piano window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 4/6/94 	| Created.											| PW
*****************************************************************************/

VOID DestroyPianoWindow (VOID)
{
	// Save the window position
	SaveWindowPosition (PIANO_WINDOW, hwndPiano);

	// Destroy the window
	SendMessage (hwndClient, WM_MDIDESTROY, (WPARAM)hwndPiano, 0);
	hwndPiano = (HWND)NULL;
}

/*^L*/
/*****************************************************************************
 Function:		PianoWindowProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		LRESULT
 Description:	The piano keyboard window proc.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 4/6/94 	| Created.											| PW
*****************************************************************************/

WINDOW_PROC PianoWindowProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	HDC			hDC;
	HMENU		hMenu;
	int			x = 0;
	static BOOL	fInitialised;
	DWORD		dwStyle;

	switch (nMsg)
	{
		// Creation
		case WM_CREATE:
		{
			// Load the key bitmaps
			hbmpKeyAUp		= LoadBitmap (hInst, "BMP_KEYA_UP");
			hbmpKeyADown	= LoadBitmap (hInst, "BMP_KEYA_DOWN");
			hbmpKeyBUp  	= LoadBitmap (hInst, "BMP_KEYB_UP");
			hbmpKeyBDown	= LoadBitmap (hInst, "BMP_KEYB_DOWN");
			hbmpKeyCUp  	= LoadBitmap (hInst, "BMP_KEYC_UP");
			hbmpKeyCDown	= LoadBitmap (hInst, "BMP_KEYC_DOWN");
			hbmpKeyDUp  	= LoadBitmap (hInst, "BMP_KEYD_UP");
			hbmpKeyDDown	= LoadBitmap (hInst, "BMP_KEYD_DOWN");

			// Calculate the note positions
			CalculateKeyInfo ();

			// Get the system menu handle
			hMenu = GetSystemMenu (hWnd, FALSE);

			// Delete the CLOSE and MAXIMIZE items from the menu
			RemoveMenu (hMenu, 5, MF_BYPOSITION);			// Separator
			DeleteMenu (hMenu, SC_CLOSE, MF_BYCOMMAND);
			DeleteMenu (hMenu, SC_MAXIMIZE, MF_BYCOMMAND);

			// Get rid of the MAXIMIZE button
			dwStyle = GetWindowLong (hWnd, GWL_STYLE);
			SetWindowLong (hWnd, GWL_STYLE, dwStyle & ~WS_MAXIMIZEBOX);

			// Not initialised yet
			fInitialised = FALSE;

			// Create the velocity control
			CreateWindow (
					SLIDER_CLASS_NAME,
					ResourceString (IDS_VELOCITY_SHORTHAND),
					WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
					x, 0,
					SLIDER_CONTROL_WIDTH, SLIDER_CONTROL_HEIGHT,
					hWnd,
					(HMENU)IDC_VELOCITY,
					hInst,
					0);
			x += SLIDER_CONTROL_WIDTH;

			// Create the MIDI controllers
			CreateWindow (SLIDER_CLASS_NAME, "MIDI A", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
					x, 0, SLIDER_CONTROL_WIDTH, SLIDER_CONTROL_HEIGHT, hWnd, (HMENU)IDC_MIDI_A, hInst, 0);
			x += SLIDER_CONTROL_WIDTH;

			CreateWindow (SLIDER_CLASS_NAME, "MIDI B", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
					x, 0, SLIDER_CONTROL_WIDTH, SLIDER_CONTROL_HEIGHT, hWnd, (HMENU)IDC_MIDI_B, hInst, 0);
			x += SLIDER_CONTROL_WIDTH;

			CreateWindow (SLIDER_CLASS_NAME, "MIDI C", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
					x, 0, SLIDER_CONTROL_WIDTH, SLIDER_CONTROL_HEIGHT, hWnd, (HMENU)IDC_MIDI_C, hInst, 0);
			x += SLIDER_CONTROL_WIDTH;

			CreateWindow (SLIDER_CLASS_NAME, "MIDI D", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
					x, 0, SLIDER_CONTROL_WIDTH, SLIDER_CONTROL_HEIGHT, hWnd, (HMENU)IDC_MIDI_D, hInst, 0);
			x += SLIDER_CONTROL_WIDTH;

			// Set up the keyboard velocity control
			InitSlider (hWnd, IDC_VELOCITY, 0, 127, nKeyboardVelocity);

			// Set up the MIDI controllers
			InitSlider (hWnd, IDC_MIDI_A, -64, 63, 0);
			InitSlider (hWnd, IDC_MIDI_B, -64, 63, 0);
			InitSlider (hWnd, IDC_MIDI_C, -64, 63, 0);
			InitSlider (hWnd, IDC_MIDI_D, -64, 63, 0);

			// Now initialised
			fInitialised = TRUE;

			return (0);
		}

		// New AUCNTRL value
		case SLN_NEW_VALUE:
		case UDN_NEW_VALUE:
		{
			if (!fInitialised)
			{
				break;
			}

			switch (LOWORD (wParam))
			{
				// New keyboard velocity
				case IDC_VELOCITY:
				{
					nKeyboardVelocity = (UINT)lParam;
					return (TRUE);
                }

				// MIDI controller A
				case IDC_MIDI_A:
				{
					SendController (nTestKbdControllers[0], (int)lParam + 64);
					return (TRUE);
				}
				
				// MIDI controller B
				case IDC_MIDI_B:
				{
					SendController (nTestKbdControllers[1], (int)lParam + 64);
					return (TRUE);
				}
				
				// MIDI controller C
				case IDC_MIDI_C:
				{
					SendController (nTestKbdControllers[2], (int)lParam + 64);
					return (TRUE);
				}
				
				// MIDI controller D
				case IDC_MIDI_D:
				{
					SendController (nTestKbdControllers[3], (int)lParam + 64);
					return (TRUE);
				}

				default:
				{
                	break;
                }
			}
			break;
		}


		// Attempting to re-size
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO FAR	*lpMmInfo = (MINMAXINFO FAR *)lParam;
			UINT			nWidthOverhead, nHeightOverhead;

			if (fInitialised)
			{
				nWidthOverhead		= 2 * GetSystemMetrics (SM_CXFRAME);
				nHeightOverhead		= GetSystemMetrics (SM_CYCAPTION) + GetSystemMetrics (SM_CYFRAME);

				// Make it a minimum of 1 note wide, and a whole note high
				lpMmInfo->ptMinTrackSize.x	= KEYS_X_OFFSET + WHITE_KEY_WIDTH + nWidthOverhead;
				lpMmInfo->ptMinTrackSize.y	= SLIDER_CONTROL_HEIGHT + nHeightOverhead;

				// Make it a maximum of 128 notes wide, and a whole note high
				lpMmInfo->ptMaxTrackSize.x	=
						KEYS_X_OFFSET +
							KeyInfo[NUM_MIDI_NOTES - 1].rcKey.right -
								KeyInfo[nKeyboardLeftKey].rcKey.left +
									nWidthOverhead;
				lpMmInfo->ptMaxTrackSize.y	= SLIDER_CONTROL_HEIGHT + nHeightOverhead;
			}

			break;
		}

		// Moved the window
		case WM_WINDOWPOSCHANGED:
		{
			LPWINDOWPOS	lpwp = (LPWINDOWPOS)lParam;

			// How big is the new client area ?
			nWindowSize.x	= lpwp->cx;
			nWindowSize.y	= lpwp->cy;

			break;
		}

		// Paint
		case WM_PAINT:
		{
			PAINTSTRUCT	ps;

			// Get the DC
			hDC = BeginPaint (hWnd, &ps);

			// Paint the whole keyboard
			PaintKeyboard (hDC);

			// Release the DC
			EndPaint (hWnd, &ps);

			break;	//return (0);
		}

		// Left mouse button down
		case WM_LBUTTONDOWN:
		{
			UINT	x, y;
			int		nMouseNote;

			// Put myself of top of the other children
			SetWindowPos (hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

			// What (x, y) ?
			x = LOWORD (lParam);
			y = HIWORD (lParam);

			// If we're inside a key
			if ((nMouseNote = InKey (x, y)) != INDEX_NONE)
			{
				// If there's already a key down
				if (nDownKey != INDEX_NONE)
				{
					// Release that key
					ReleaseKey (hWnd, nDownKey);
				}

				// Then press this key
				PressKey (hWnd, nMouseNote);
			}

			break;
		}

		// Left mouse button up
		case WM_LBUTTONUP:
		{
			// If there's already a key down
			if (nDownKey != INDEX_NONE)
			{
				// Release that key
				ReleaseKey (hWnd, nDownKey);
			}

			break;
		}

		// Mouse moved
		case WM_MOUSEMOVE:
		{
			UINT	x, y;
			int		nMouseNote;

			// What (x, y) ?
            x = LOWORD (lParam);
			y = HIWORD (lParam);

			// If we're inside a key
			if ((nMouseNote = InKey (x, y)) != INDEX_NONE)
			{
				// If there's already a key down
				if (nDownKey != INDEX_NONE)
				{
					// If this key is different from the current one
					if (nMouseNote != nDownKey)
                    {
						// Release that key
						ReleaseKey (hWnd, nDownKey);
        	
						// Then press this key
						PressKey (hWnd, nMouseNote);
					}
				}
			}

			break;
		}

		// Destruction
		case WM_DESTROY:
		{
			// Destroy the bitmaps
			DeleteObject (hbmpKeyAUp);
			DeleteObject (hbmpKeyADown);
			DeleteObject (hbmpKeyBUp);
			DeleteObject (hbmpKeyBDown);
			DeleteObject (hbmpKeyCUp);
			DeleteObject (hbmpKeyCDown);
			DeleteObject (hbmpKeyDUp);
			DeleteObject (hbmpKeyDDown);
			return (0);
		}

		// Default
		default:
		{
        	break;
        }
	}

	// Default processing
	return (DefMDIChildProc (hWnd, nMsg, wParam, lParam));
}

/*^L*/
/*****************************************************************************
 Function:		PressKey
 Parameters:	HWND	hWnd			Piano window handle.
				int		nMouseNote		Key pressed.
 Returns:		None.
 Description:	Draws the key as selected and sends a MIDI note-on.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 4/6/94 	| Created.											| PW
*****************************************************************************/

static VOID PressKey (HWND hWnd, int nMouseNote)
{
	HDC			hDC;
	HMIDIIN		hMidiInput;				// Input device handle
	HMIDIOUT	hMidiOutput;			// Output device handle
	BYTE		bMidi[4];

	// Get the window DC
	hDC = GetDC (hWnd);

	// Paint the note
	PaintNote (hDC, nMouseNote, TRUE, KeyInfo[nKeyboardLeftKey].rcKey.left);

	// Release the window DC
	ReleaseDC (hWnd, hDC);

	// Mark this key as selected
	nDownKey = nMouseNote;

	// Find out what the MIDI device handles are
	morphdllGetHandles (&hMidiInput, &hMidiOutput);

	// Send a MIDI note-on
	bMidi[0] = 0x90 | bKbdMidiChannel;
	bMidi[1] = nMouseNote;
	bMidi[2] = (BYTE)(nKeyboardVelocity & 0x7F);
	bMidi[3] = 0;
	midiOutShortMsg (hMidiOutput, *(DWORD *)&bMidi[0]);
}

/*^L*/
/*****************************************************************************
 Function:		ReleaseKey
 Parameters:	HWND	hWnd			Piano window handle.
				int		nMouseNote		Key pressed.
 Returns:		None.
 Description:	Draws the key as unselected and sends a MIDI note-off.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 4/6/94 	| Created.											| PW
*****************************************************************************/

static VOID ReleaseKey (HWND hWnd, int nMouseNote)
{
	HDC			hDC;
	HMIDIIN		hMidiInput;				// Input device handle
	HMIDIOUT	hMidiOutput;			// Output device handle
	BYTE		bMidi[4];

	// Get the window DC
	hDC = GetDC (hWnd);

	// Paint the note
	PaintNote (hDC, nMouseNote, FALSE, KeyInfo[nKeyboardLeftKey].rcKey.left);

	// Release the window DC
	ReleaseDC (hWnd, hDC);

	// Mark no key as selected
	nDownKey = INDEX_NONE;

	// Find out what the MIDI device handles are
	morphdllGetHandles (&hMidiInput, &hMidiOutput);

	// Send a MIDI note-off
	bMidi[0] = 0x80 | bKbdMidiChannel;
	bMidi[1] = nMouseNote;
	bMidi[2] = (BYTE)(nKeyboardVelocity & 0x7F);
	bMidi[3] = 0;
	midiOutShortMsg (hMidiOutput, *(DWORD *)&bMidi[0]);
}

/*^L*/
/*****************************************************************************
 Function:		SendController
 Parameters:	int		nController		MIDI controller number.
				int		nValue			Controller value.
 Returns:		None.
 Description:	Sends a MIDI controller value.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 24/10/94 	| Created.											| PW
*****************************************************************************/

static VOID SendController (int nController, int nValue)
{
	HMIDIIN		hMidiInput;				// Input device handle
	HMIDIOUT	hMidiOutput;			// Output device handle
	BYTE		bMidi[4];

	// Find out what the MIDI device handles are
	morphdllGetHandles (&hMidiInput, &hMidiOutput);

	// Create and send the controller value
	bMidi[0] = 0xB0 | bKbdMidiChannel;
	bMidi[1] = (BYTE)nController;
	bMidi[2] = (BYTE)(nValue & 0x7F);
	bMidi[3] = 0;
	midiOutShortMsg (hMidiOutput, *(DWORD *)&bMidi[0]);
}

/*^L*/
/*****************************************************************************
 Function:		InKey
 Parameters:	int		x				Current x pos.
				int		y				Current y pos.
 Returns:		int						Note number, or INDEX_NONE.
 Description:	Calculated which key we are in.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 4/6/94 	| Created.											| PW
*****************************************************************************/

static int InKey (int x, int y)
{
	int		nNote;
	int		nBaseX;
	RECT	rcKey;
	POINT	ptMouse;

	ptMouse.x = x;
	ptMouse.y = y;

	/*
		First scan the BLACK notes
	 */

	// Start with the bottom note in the window
	nNote = nKeyboardLeftKey;

	// Get the X offset of that note
	nBaseX = KeyInfo[nNote].rcKey.left;

	// For every note whose left is within the window
	while ((KeyInfo[nNote].rcKey.left - nBaseX + KEYS_X_OFFSET) < nWindowSize.x)
	{
		// If the note is black
		if (KeyInfo[nNote].Type == BLACK_KEY)
		{
			// Get the key rectangle
			rcKey.left		= KeyInfo[nNote].rcKey.left - nBaseX + KEYS_X_OFFSET;
			rcKey.right		= KeyInfo[nNote].rcKey.right - nBaseX + KEYS_X_OFFSET;
			rcKey.top		= KeyInfo[nNote].rcKey.top;
			rcKey.bottom	= KeyInfo[nNote].rcKey.bottom;

			// If we are in the key rectangle
			if (PtInRect (&rcKey, ptMouse))
			{
				// Found it
                return (nNote);
			}
		}

		// Step onto the next note
		if (++nNote >= NUM_MIDI_NOTES)
		{
			break;
        }
	}

	/*
		Then scan the WHITE notes
	 */

	// Start with the bottom note in the window
	nNote = nKeyboardLeftKey;

	// Get the X offset of that note
	nBaseX = KeyInfo[nNote].rcKey.left;

	// For every note whose left is within the window
	while ((KeyInfo[nNote].rcKey.left - nBaseX) < nWindowSize.x)
	{
		// If the note is WHITE
		if (KeyInfo[nNote].Type == WHITE_KEY)
		{
			// Get the key rectangle
			rcKey.left		= KeyInfo[nNote].rcKey.left - nBaseX + KEYS_X_OFFSET;
			rcKey.right		= KeyInfo[nNote].rcKey.right - nBaseX + KEYS_X_OFFSET;
			rcKey.top		= KeyInfo[nNote].rcKey.top;
			rcKey.bottom	= KeyInfo[nNote].rcKey.bottom;

			// If we are in the key rectangle
			if (PtInRect (&rcKey, ptMouse))
			{
				// Found it
                return (nNote);
			}
		}

		// Step onto the next note
		if (++nNote >= NUM_MIDI_NOTES)
		{
			break;
		}
	}

	// Not found
	return (INDEX_NONE);
}

/*^L*/
/*****************************************************************************
 Function:		CalculateKeyInfo
 Parameters:	None.
 Returns:		None.
 Description:	Calculates the positions of all the keys.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 4/6/94 	| Created.											| PW
*****************************************************************************/

static VOID CalculateKeyInfo (VOID)
{
	UINT		i, nInOctave;
	UINT		xpos  = 0;

	// Set up key 0 by hand
	KeyInfo[0].Type			= WHITE_KEY;
	KeyInfo[0].rcKey.left	= xpos;
	KeyInfo[0].rcKey.top	= 0;
	KeyInfo[0].rcKey.right	= xpos + WHITE_KEY_WIDTH;
	KeyInfo[0].rcKey.bottom	= WHITE_KEY_HEIGHT;
	KeyInfo[0].hBitMapUp	= hbmpKeyAUp;
	KeyInfo[0].hBitMapDown	= hbmpKeyADown;

	// For each key
	for (i=nInOctave=1 ; i<ARRAY_SIZE(KeyInfo) ; i++)
	{
		// If we are stepping from a WHITE->BLACK
		if ((KeyInfo[i-1].Type == WHITE_KEY) && (KeyTypes[nInOctave].Type == BLACK_KEY))
		{
			// Calculate new X pos
			xpos += (WHITE_KEY_WIDTH - (BLACK_KEY_WIDTH / 2));
		}
		// If we are stepping from a BLACK->WHITE
		else if ((KeyInfo[i-1].Type == BLACK_KEY) && (KeyTypes[nInOctave].Type == WHITE_KEY))
		{
			// Calculate new X pos
			xpos += (BLACK_KEY_WIDTH / 2);
		}
		// If we are stepping from a WHITE->WHITE
		else if ((KeyInfo[i-1].Type == WHITE_KEY) && (KeyTypes[nInOctave].Type == WHITE_KEY))
		{
			// Calculate new X pos
			xpos += WHITE_KEY_WIDTH;
		}

		// Store the new key's type
		if ((KeyInfo[i].Type = KeyTypes[nInOctave].Type) == WHITE_KEY)
		{
			// Store the new key's position
			KeyInfo[i].rcKey.left	= xpos;
			KeyInfo[i].rcKey.top	= 0;
			KeyInfo[i].rcKey.right	= xpos + WHITE_KEY_WIDTH;
			KeyInfo[i].rcKey.bottom	= WHITE_KEY_HEIGHT;
		}
		else
		{
			// Store the new key's position
			KeyInfo[i].rcKey.left	= xpos;
			KeyInfo[i].rcKey.top	= 0;
			KeyInfo[i].rcKey.right	= xpos + BLACK_KEY_WIDTH;
			KeyInfo[i].rcKey.bottom	= BLACK_KEY_HEIGHT;
		}

		// Store the new key's bitmaps
		KeyInfo[i].hBitMapUp		= *KeyTypes[nInOctave].phbmpUp;
		KeyInfo[i].hBitMapDown		= *KeyTypes[nInOctave].phbmpDown;

		// Step the note-in-octave
		if (++nInOctave == ARRAY_SIZE (KeyTypes))
		{
			nInOctave = 0;
        }
	}
}

/*^L*/
/*****************************************************************************
 Function:		PaintKeyboard
 Parameters:	HDC		hDC				Keyboard window device context.
 Returns:		None.
 Description:	Redraws the whole keyboard.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 4/6/94 	| Created.											| PW
*****************************************************************************/

static VOID PaintKeyboard (HDC hDC)
{
	UINT	nNote;
	int		nBaseX;

	// Start with the bottom note in the window
	nNote = nKeyboardLeftKey;

	// Get the X offset of that note
	nBaseX = KeyInfo[nNote].rcKey.left;

	// For every note whose left is within the window
	while ((KeyInfo[nNote].rcKey.left - nBaseX + KEYS_X_OFFSET) < nWindowSize.x)
	{
		// Draw the note
		PaintNote (hDC, nNote, FALSE, nBaseX);

		// Step onto the next note
		if (++nNote >= NUM_MIDI_NOTES)
		{
			break;
		}
	}
}

/*^L*/
/*****************************************************************************
 Function:		PaintNote
 Parameters:	HDC		hDC				Keyboard window device context.
				int		nNote			Note number.
				BOOL	fDown			TRUE if key is down.
                UINT	nBaseX			X offset of window's left side.
 Returns:		None.
 Description:	Draws a single note.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 4/6/94 	| Created.											| PW
*****************************************************************************/

static VOID PaintNote (HDC hDC, int nNote, BOOL fDown, UINT nBaseX)
{
	HDC			hdcKey;
	HBITMAP		hbmKey;
	int			x, y, w, h;
	COLORREF	OldColour;
	HFONT		hOldFont;
	int			OldMode;
	char		szNote[10];

	// Create a DC for the key
	hdcKey = CreateCompatibleDC (hDC);
	hbmKey = fDown ? KeyInfo[nNote].hBitMapDown : KeyInfo[nNote].hBitMapUp;
	SelectObject (hdcKey, hbmKey);

	// Calculate where to put the note in the window
	x = KeyInfo[nNote].rcKey.left - nBaseX + KEYS_X_OFFSET;
	y = KeyInfo[nNote].rcKey.top;
	w = KeyInfo[nNote].rcKey.right - KeyInfo[nNote].rcKey.left;
	h = KeyInfo[nNote].rcKey.bottom - KeyInfo[nNote].rcKey.top;

	// Do a transparent BitBlt of the pin to the screen
	OldColour = SetBkColor (hDC, RGB (0, 0, 255));	// Transparency color is BLUE
	TransBlt (hDC, x, y, w, h, hdcKey, 0, 0);
	SetBkColor (hDC, OldColour);
	DeleteDC (hdcKey);

	// If this is a 'C'
	if ((nNote % 12) == 0)
	{
		// Print the note name just below the note
		hOldFont = SelectObject (hDC, hDialogFont);
		OldColour = SetTextColor (hDC, GetSysColor (COLOR_BTNTEXT));
		OldMode = SetBkMode (hDC, TRANSPARENT);
		FormatNote (&szNote[0], nNote);
		TextOut (hDC, x + 0, y + h + 4, &szNote[0], lstrlen (&szNote[0]));
		SelectObject (hDC, hOldFont);
		SetTextColor (hDC, OldColour);
		SetBkMode (hDC, OldMode);
	}
}

/*^L*/
/*****************************************************************************
 Function:		TransBlt
 Parameters:	HDC		hdcD
				int		x
				int		y
				int		dx
				int		dy
				HDC		hdcS
				int		x0
				int		y0
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Performs a BitBlt from hdcS to hdcD device context, treating
				the destinations's background colour as background.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 19/5/94 	| Created.											| PW
*****************************************************************************/

static BOOL TransBlt (HDC hdcD, int x, int y, int dx, int dy, HDC hdcS, int x0, int y0)
{
	DWORD	rgbBk, rgbFg;
	DWORD	rgbBkS;
	HBITMAP	hbmMask;
	HDC		hdcMask;
	HBITMAP	hbmT;
	BOOL	f = FALSE;

	//
	//  Get the current DC color's
	//
	rgbBk = GetBkColor (hdcD);
	rgbFg = GetTextColor (hdcD);
	rgbBkS = GetBkColor (hdcS);
	SetTextColor (hdcD, BLACK);

	//
	//  make a memory DC for use in color conversion
	//
	hdcMask = CreateCompatibleDC (hdcS);
	if (!hdcMask)
    {
		return (FALSE);
    }

	//
	// create a mask bitmap and associated DC
	//
	hbmMask = CreateBitmap (dx, dy, 1, 1, NULL);
	if (!hbmMask)
    {
		goto errorDC;
	}

	// select the mask bitmap into the mono DC
	hbmT = SelectObject (hdcMask, hbmMask);

	// do a color to mono bitblt to build the mask
	// generate 1's where the source is equal to the background, else 0's
	SetBkColor (hdcS, rgbBk);
	BitBlt (hdcMask, 0, 0, dx, dy, hdcS, x0, y0, SRCCOPY);

	// do a MaskBlt to copy the bitmap to the dest
	SetBkColor (hdcD, WHITE);
	BitBlt (hdcD, x, y, dx, dy, hdcS, x0, y0, DSx);
	BitBlt (hdcD, x, y, dx, dy, hdcMask, 0, 0, DSa);
	BitBlt (hdcD, x, y, dx, dy, hdcS, x0, y0, DSx);
	f = TRUE;
	SelectObject (hdcMask, hbmT);
	DeleteObject (hbmMask);

	//
	// Restore the DC colors
	//
	SetBkColor (hdcS, rgbBkS);
	SetBkColor (hdcD, rgbBk);
	SetTextColor (hdcD, rgbFg);
	errorDC:
	DeleteDC (hdcMask);
	return (f);
}

