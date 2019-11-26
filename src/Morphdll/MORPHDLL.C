/*****************************************************************************
 File:			morphdll.c
 Description:	The Morpheus MIDI callback DLL. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 9/4/94	| Created.											| PW
*****************************************************************************/

#include <windowsx.h>
#include <windows.h>

#include <mmsystem.h>

#include <morphdll.h>

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

HMIDIIN		hmidiIn		= (HMIDIIN)NULL;		// MIDI input device handle
HMIDIOUT	hmidiOut	= (HMIDIOUT)NULL;		// MIDI output device handle
HWND		hwndIn		= (HWND)NULL;			// MIDI input notification window
HWND		hwndOut		= (HWND)NULL;			// MIDI output notification window

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

VOID CALLBACK morphdllMidiInputCallback (HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
VOID CALLBACK morphdllMidiOutputCallback (HMIDIOUT hMidiOut, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

#ifdef WINDOWS32

/*^L*/
/**************************************************************************
Function:		DllMain
Parameters:		HINSTANCE	hinstDLL	DLL instance.
				DWORD		fdwReason	Reason for calling entry point.
				LPVOID		lpvReserved	Not used.
Returns:		BOOL					TRUE if OK, else FALSE.
Description:	The DLL entry point.
History:
Date		| Description										| Name
------------+---------------------------------------------------+------
17/6/95		| Created.											| PW
**************************************************************************/

BOOLEXPORT DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return (TRUE);
}

#else

/*^L*/
/**************************************************************************
 Function:		LibMain
 Parameters:	HANDLE hModule			Out module handle.
				WORD wHeapSize			Heap size (from DEF file).
				LPSTR lpCmdLine			The command line.
 Returns:		int						1 if OK, 0 if not.
 Description:	The DLL entry point.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 5/4/94		| Created.											| PW
**************************************************************************/

int FAR PASCAL LibMain (HANDLE hModule, WORD wDataSeg, WORD wHeapSize, LPSTR lpCmdLine)
{
	// Heap stuff
	if (wHeapSize > 0)
	{
		UnlockData (0);
	}

	return (1);
}

/*^L*/
/**************************************************************************
 Function:		WEP
 Parameters:	int nParam				WEP_SYSTEMEXIT or WEP_FREE_DLL
 Returns:		int						1 if OK, 0 if not.
 Description:	The DLL exit point.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 5/4/94		| Created.											| PW
**************************************************************************/

INTEXPORT WEP (int nParam)
{
	return (1);
}

#endif

/*^L*/
/*****************************************************************************
 Function:		morphdllOpenMidiDevices
 Parameters:	int		idInput			Input device ID.
				int		idOutput		Output device ID.
 Returns:		BOOL					TRUE if successful, else FALSE.
 Description:	Opens both MIDI devices, using the local callback.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 9/4/94 	| Created.											| PW
*****************************************************************************/

BOOLEXPORT morphdllOpenMidiDevices (int idInput, int idOutput)
{
	// If either device is already open
	if ((hmidiIn != NULL) || (hmidiOut != NULL))
	{
		// Close them
		morphdllCloseMidiDevices ();
	}

	// Open the input device
	if (midiInOpen (&hmidiIn, idInput, (DWORD)morphdllMidiInputCallback, 0, CALLBACK_FUNCTION))
	{
		// Error
		return (FALSE);
	}

	// Open the output device
	if (midiOutOpen (&hmidiOut, idOutput, (DWORD)morphdllMidiOutputCallback, 0, CALLBACK_FUNCTION))
	{
		// Error
		midiInClose (hmidiIn);
		return (FALSE);
	}

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		morphdllCloseMidiDevices
 Parameters:	None.
 Returns:		BOOL					TRUE if successful, else FALSE.
 Description:	Closes the MIDI input and output devices.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 9/4/94 	| Created.											| PW
*****************************************************************************/

BOOLEXPORT morphdllCloseMidiDevices (VOID)
{
	BOOL	fOK = TRUE;

	// Close the input device
	if (midiInClose (hmidiIn))
	{
		// Error
		fOK = FALSE;
	}

	// Close the output device
	if (midiOutClose (hmidiOut))
	{
		// Error
		fOK = FALSE;
	}

	// Reset local handles
	hmidiIn		= (HMIDIIN)NULL;
	hmidiOut	= (HMIDIOUT)NULL;
	hwndIn		= (HWND)NULL;
	hwndOut		= (HWND)NULL;

	// OK
	return (fOK);
}

/*^L*/
/*****************************************************************************
 Function:		morphdllSetWindow
 Parameters:	HWND	hInput			Notification window for input.
				HWND	hInput			Notification window for output.
 Returns:		BOOL					TRUE if successful, else FALSE.
 Description:	Sets the input and output MIDI notification windows.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 9/4/94 	| Created.											| PW
*****************************************************************************/

VOIDEXPORT morphdllSetWindow (HWND hInput, HWND hOutput)
{
	// Store the window handles
	hwndIn		= hInput;
	hwndOut		= hOutput;
}

/*^L*/
/*****************************************************************************
 Function:		morphdllGetHandles
 Parameters:	LPHMIDIIN	phMidiIn	(Output) MIDI input handle.
				LPHMIDIOUT	phMidiOut	(Output) MIDI output handle.
 Returns:		None.
 Description:	Returns the open MIDI device handles.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 9/4/94 	| Created.											| PW
*****************************************************************************/

VOIDEXPORT morphdllGetHandles (LPHMIDIIN phMidiIn, LPHMIDIOUT phMidiOut)
{
	// Output the handles
	if (phMidiIn != NULL)
	{
		*phMidiIn = hmidiIn;
	}
	if (phMidiOut != NULL)
	{
		*phMidiOut = hmidiOut;
	}
}

/*^L*/
/*****************************************************************************
 Function:		morphdllMidiInputCallback
 Parameters:	HMIDIIN	hMidiIn
				UINT	wMsg
				DWORD	dwInstance
				DWORD	dwParam1
				DWORD	dwParam2
 Returns:		None.
 Description:	The MIDI input callback.
				Sends notification messages to the configured window. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 9/4/94 	| Created.											| PW
*****************************************************************************/

VOID CALLBACK morphdllMidiInputCallback (HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	UINT	nOutMsg;
	WPARAM	OutWparam;
	LPARAM	OutLparam;

	switch (wMsg)
	{
		case MIM_CLOSE:
		{
			nOutMsg		= MM_MIM_CLOSE;
			OutWparam	= (WPARAM)hMidiIn;
			OutLparam	= (LPARAM)0;
			break;
		}
		case MIM_DATA:
		{
			nOutMsg		= MM_MIM_DATA;
			OutWparam	= (WPARAM)hMidiIn;
			OutLparam	= (LPARAM)dwParam1;
			break;
        }
		case MIM_ERROR:
		{
			nOutMsg		= MM_MIM_ERROR;
			OutWparam	= (WPARAM)hMidiIn;
			OutLparam	= (LPARAM)dwParam1;
			break;
        }
		case MIM_LONGDATA:
		{
			nOutMsg		= MM_MIM_LONGDATA;
			OutWparam	= (WPARAM)hMidiIn;
			OutLparam	= (LPARAM)dwParam1;
			break;
        }
		case MIM_LONGERROR:
		{
			nOutMsg		= MM_MIM_LONGERROR;
			OutWparam	= (WPARAM)hMidiIn;
			OutLparam	= (LPARAM)dwParam1;
			break;
        }
		case MIM_OPEN:
		{
			nOutMsg		= MM_MIM_OPEN;
			OutWparam	= (WPARAM)hMidiIn;
			OutLparam	= (LPARAM)0;
			break;
		}
		default:
		{
			return;
		}
	}

	// Post the notification
	if (hwndIn != (HWND)NULL)
	{
		PostMessage (hwndIn, nOutMsg, OutWparam, OutLparam);
	}
}

/*^L*/
/*****************************************************************************
 Function:		morphdllMidiOutputCallback
 Parameters:	HMIDIIN	hMidiOut
				UINT	wMsg
				DWORD	dwInstance
				DWORD	dwParam1
				DWORD	dwParam2
 Returns:		None.
 Description:	The MIDI output callback.
				Sends notification messages to the configured window. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 9/4/94 	| Created.											| PW
*****************************************************************************/

VOID CALLBACK morphdllMidiOutputCallback (HMIDIOUT hMidiOut, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	UINT	nOutMsg;
	WPARAM	OutWparam;
	LPARAM	OutLparam;

	switch (wMsg)
	{
		case MOM_CLOSE:
		{
			nOutMsg		= MM_MOM_CLOSE;
			OutWparam	= (WPARAM)hMidiOut;
			OutLparam	= (LPARAM)0;
			break;
		}
		case MOM_DONE:
		{
			nOutMsg		= MM_MOM_DONE;
			OutWparam	= (WPARAM)hMidiOut;
			OutLparam	= (LPARAM)dwParam1;
			break;
        }
		case MOM_OPEN:
		{
			nOutMsg		= MM_MOM_OPEN;
			OutWparam	= (WPARAM)hMidiOut;
			OutLparam	= (LPARAM)0;
			break;
		}
		default:
		{
			return;
		}
	}

	// Post the notification
	if (hwndOut != (HWND)NULL)
	{
		PostMessage (hwndOut, nOutMsg, OutWparam, OutLparam);
	}
}
