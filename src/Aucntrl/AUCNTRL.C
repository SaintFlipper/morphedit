/*****************************************************************************
 File:			aucntrl.c
 Description:	Audio controls DLL main module.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 5/4/94		| Created.											| PW
*****************************************************************************/

#include <windows.h>
#include <aucntrl.h>
#include "auc_i.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

HANDLE	hDllInst;						// DLL instance

#if defined (WINDOWS32)

VOIDEXPORT AuCntrlDummy (VOID)
{
}

/**/
/***************************************************************************
Function:		DllMain
Parameters:		HINSTANCE	hinstDLL	DLL instance handle.
				DWORD		fdwReason	Reason for calling this entry point.
				LPVOID		lpvReserved	Not used.
Return value:	BOOL					TRUE if OK, else FALSE.
Description:	Initialises the ENCAPS DLL.
History:
Date		| Description										| Name
------------+---------------------------------------------------+-----
17/6/95		| Created.											| PW
***************************************************************************/

BOOLEXPORT WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		// Attaching
		case DLL_PROCESS_ATTACH:
		{
			// Save my instance
			hDllInst = hinstDLL;

			if (
				!SliderControlInit (hDllInst)	||		// Initialise SLIDER
				!UpdownControlInit (hDllInst)	||		// Initialise UP-DOWN
				!EnvelopeControlInit (hDllInst)	||		// Initialise ENVELOPE
				!PatchPinControlInit (hDllInst)			// Initialise PATCH PIN
				)
			{
				// Failure
				return (FALSE);
			}

			break;
		}

		// Detaching
		case DLL_PROCESS_DETACH:
		{
			// Wind up the controls
			SliderControlTerm (hDllInst);
			UpdownControlTerm (hDllInst);
			EnvelopeControlTerm (hDllInst);
			PatchPinControlTerm (hDllInst);

			break;
		}

		default:
		{
			break;
		}
	}

	// OK
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
	// Save our module handle
	hDllInst = hModule;

	// Heap stuff
	if (wHeapSize > 0)
	{
		UnlockData (0);
	}

	// Initialise SLIDER
	if (!SliderControlInit (hDllInst))
	{
		// Failure
		return (FALSE);
	}

	// Initialise UP-DOWN
	if (!UpdownControlInit (hDllInst))
	{
		// Failure
		return (FALSE);
	}

	// Initialise ENVELOPE
	if (!EnvelopeControlInit (hDllInst))
	{
		// Failure
		return (FALSE);
	}

	// Initialise PATCH PIN
	if (!PatchPinControlInit (hDllInst))
	{
		// Failure
		return (FALSE);
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

int FAR PASCAL _export WEP (int nParam)
{
	// Wind up the controls
	SliderControlTerm (hDllInst);
	UpdownControlTerm (hDllInst);
	EnvelopeControlTerm (hDllInst);
	PatchPinControlTerm (hDllInst);
	return (1);
}

#endif
