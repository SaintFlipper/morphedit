/*****************************************************************************
 File:			preset3.c
 Description:	Functions for sub-preset dialogs.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/9/94	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/


/****************************************************************************/
/*								Local types									*/
/****************************************************************************/


/****************************************************************************/
/*								Global data									*/
/****************************************************************************/

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

// Which sub-dialog is currently on display
static SUBDIALOG	SubDialog = NO_SUBDIALOG;

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

/*^L*/
/*****************************************************************************
 Function:		InitPresetSubDialog
 Parameters:	None.
 Returns:		None.
 Description:	Initialises preset sub-dialog handling.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/9/94 	| Created.											| PW
*****************************************************************************/

DWORD InitPresetSubDialog (VOID)
{
	// Pretend that 'MAIN' has been selected
	return (PresetSubDialog (IDC_PRESET_TOOLBAR_MAIN));
}

/*^L*/
/*****************************************************************************
 Function:		TermPresetSubDialog
 Parameters:	None.
 Returns:		None.
 Description:	Terminates preset sub-dialog handling -
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/9/94 	| Created.											| PW
*****************************************************************************/

VOID TermPresetSubDialog (VOID)
{
	// If there is a sub-dialog on display
	if (SubDialog != NO_SUBDIALOG)
	{
		// Destroy its contents
		DestroyAllChildren (hwndCurrentPreset);
	}

	// Set displayed sub-dialog to NO_SUBDIALOG	
	SubDialog = NO_SUBDIALOG;
}

/*^L*/
/*****************************************************************************
 Function:		SetBigPresetDialog
 Parameters:	BOOL	fState			TRUE if preset to use big dialog.
 Returns:		None.
 Description:	1)	Sets the state of fBigPresetDialog.
				2)	Checks/unchecks the big dialog menu item.
				3)	If the preset window is on display then
					a)	Empty the dialog currently on display.
					b)	Fill the dialog with big/sub children.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/9/94 	| Created.											| PW
*****************************************************************************/

DWORD SetBigPresetDialog (BOOL fState)
{
	DWORD	dwVal = 0;
	
	// Set the state of the big dialog flag
	fBigPresetDialog = fState;

	// (un/)Check the menu option
	CheckMenuItem (GetMenu (hwndMain), IDM_OPTIONS_BIG_PRESET_DIALOG,
					fBigPresetDialog ? MF_CHECKED : MF_UNCHECKED);

	// If the preset window is on display
	if (hwndCurrentPreset != (HWND)NULL)
	{
		// Hide the window
		ShowWindow (hwndCurrentPreset, SW_HIDE);
		
		// Empty the dialog
		DestroyAllChildren (hwndCurrentPreset);

		// If we have gone BIG
		if (fBigPresetDialog)
		{
			// Create the whole preset dialog
			dwVal = DialogInWindow (hwndCurrentPreset, "DLG_PRESET");

			// Reformat myself
			ReformatDialog (hwndCurrentPreset, RDF_THINFONT);
		}
		else
		{
			// Pretend that 'MAIN' has been selected
			dwVal = PresetSubDialog (IDC_PRESET_TOOLBAR_MAIN);
		}

		// Initialise the dialog
		InitInstrumentsDialog (hwndCurrentPreset);
		InitModulatorsDialog (hwndCurrentPreset);
		InitMiscDialog (hwndCurrentPreset);

		// Show the window
		ShowWindow (hwndCurrentPreset, SW_SHOW);

		// If a preset is on display
		if (nDisplayedPreset != INDEX_NONE)
		{
			// Force the preset to be redisplayed
			DisplayPreset ();
		}
	}

	// Return the window dimension
	return (dwVal);
}

/*^L*/
/*****************************************************************************
 Function:		PresetSubDialog
 Parameters:	int		nToolbarButton	Tool-bar button that's been pressed.
 Returns:		DWORD					New dialog dimensions.
 Description:	Handles a preset sub-dialog toolbar selection.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/9/94 	| Created.											| PW
*****************************************************************************/

DWORD PresetSubDialog (int nToolbarButton)
{
	LPCSTR	lpszDialogName;
	DWORD	dwVal;

	ShowWindow (hwndCurrentPreset, SW_HIDE);

	// If there is a sub-dialog already on display
	if (SubDialog != NO_SUBDIALOG)
	{
		// Destroy its contents
		DestroyAllChildren (hwndCurrentPreset);
	}

	// Handle according to which button was pressed
	switch (nToolbarButton)
	{
		case IDC_PRESET_TOOLBAR_MAIN:
		{
			lpszDialogName = "DLG_PRESET_MAIN";
			SubDialog = MAIN;
			break;
		}
		
		case IDC_PRESET_TOOLBAR_VOICES:
		{
			lpszDialogName = "DLG_PRESET_VOICES";
			SubDialog = VOICES;
			break;
		}
		
		case IDC_PRESET_TOOLBAR_LFO_AUXENV:
		{
			lpszDialogName = "DLG_PRESET_LFO_AUXENV";
			SubDialog = LFO_AUXENV;
			break;
		}
		
		case IDC_PRESET_TOOLBAR_FG:
		{
			lpszDialogName = "DLG_PRESET_FG";
			SubDialog = FG;
			break;
		}

		default:
		{
			// Do no harm
			ShowWindow (hwndCurrentPreset, SW_SHOW);
			return (0);
		}
	}

	// Put the new dialog in the window (and resize)
	dwVal = DialogInWindow (hwndCurrentPreset, lpszDialogName);

	// Reformat myself
	ReformatDialog (hwndCurrentPreset, RDF_THINFONT);

	ShowWindow (hwndCurrentPreset, SW_SHOW);

	return (dwVal);
}

/*^L*/
/*****************************************************************************
 Function:		IsPresetSubDialogDisplayed
 Parameters:	SUBDIALOG	n			Subdialog type.
 Returns:		BOOL					TRUE is it's displayes, else FALSE.
 Description:	Determines whether a particular sub-dialog is on display.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/9/94 	| Created.											| PW
*****************************************************************************/

BOOL IsPresetSubDialogDisplayed (SUBDIALOG n)
{
	// MAIN
	if ((n == MAIN) && ((fBigPresetDialog) || (SubDialog == MAIN)))
	{
		return (TRUE);
	}
	// VOICES
	else if ((n == VOICES) && ((fBigPresetDialog) || (SubDialog == VOICES)))
	{
		return (TRUE);
	}
	// LFOs and Auxiliary envelope
	else if ((n == LFO_AUXENV) && ((fBigPresetDialog) || (SubDialog == LFO_AUXENV)))
	{
		return (TRUE);
	}
	// Function generators
	else if ((n == FG) && ((fBigPresetDialog) || (SubDialog == FG)))
	{
		return (TRUE);
	}
	// Routings (only displayed in the sub-preset MAIN page)
	else if ((n == ROUTINGS) && (!fBigPresetDialog) && (SubDialog == MAIN))
	{
		return (TRUE);
	}

	// Nope
	return (FALSE);
}

