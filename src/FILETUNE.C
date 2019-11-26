/*****************************************************************************
 File:			filetune.c
 Description:	Functions for loading and saving Morpheus USER TUNING files. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 20/6/94	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"
#include "morfile.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

/*^L*/
/*****************************************************************************
 Function:		SaveTuning
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Saves the tuning table.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 20/6/94	| Created.											| PW
*****************************************************************************/

BOOL SaveTuning (VOID)
{
	HFILE		hFile;
	OFSTRUCT	os;
	WORD		wSum = 0;
	static char	szFileName[MAX_PATH];
	WORD		nLength = sizeof (TUNING_TABLE_PARAMS);

	// If we aren't connected to the Morpheus
	if (!fConnectedToMorpheus)
	{
		// If it's not in memory
		if (MorpheusTuningTable.bHeader[0] == 0)
		{
			// Then we can't continue
			MyMessageBox (hwndMain, ResourceString (IDS_NOT_CONNECTED_NO_UT),
				ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			return (FALSE);
		}
	}

	// Ask the user for a file name
	if (!MyGetSaveFileName (hwndMain,
							ResourceString (IDS_SAVE_UT_FILEBOX_TITLE),
//							ResourceString (IDS_SAVE_UT_FILEBOX_MASK),
							USER_TUNING_FILE_MASK,
							&szFileName[0]))
	{
		// Hit CANCEL
		return (TRUE);
	}

	// Create the file (fixed name)
	memset (&os, 0, sizeof (OFSTRUCT));
	os.cBytes = sizeof (OFSTRUCT);
	if ((hFile = OpenFile (&szFileName[0], &os, OF_CREATE)) == HFILE_ERROR)
	{
		// Couldn't create the file
		return (FALSE);
	}

	// Write the file header
	if (_lwrite (hFile, MFILE_USER_TUNING_HEADER, sizeof (MFILE_USER_TUNING_HEADER)) != sizeof (MFILE_USER_TUNING_HEADER))
	{
		// Error
		_lclose (hFile);
		remove (&szFileName[0]);
		return (FALSE);
	}
	// Add to the checksum
	wSum += CalcChecksum (MFILE_USER_TUNING_HEADER, sizeof (MFILE_USER_TUNING_HEADER));

	// If it's not in memory
	if (MorpheusTuningTable.bHeader[0] == 0)
	{
		// Fetch it from the Morpheus
		if (!MorpheusGetTuning (&MorpheusTuningTable))
		{
			// Error fetching from Morpheus
			MyMessageBox (hwndMain, ResourceString (IDS_SAVE_UT_ERROR_FETCHING), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			_lclose (hFile);
			remove (&szFileName[0]);
			return (FALSE);
		}
	}

	// Save it
	if (!WriteLengthAndData (hFile, &nLength, &MorpheusTuningTable, &wSum))
	{
		// Error saving this preset
		MyMessageBox (hwndMain, ResourceString (IDS_SAVE_UT_ERROR_DISK), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		remove (&szFileName[0]);
		return (FALSE);
	}

	// Write the CHECKSUM
	_lwrite (hFile, (LPVOID)&wSum, sizeof (WORD));

	// Close the file
	_lclose (hFile);

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		LoadTuning
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Loads the tuning table.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 20/6/94	| Created.											| PW
*****************************************************************************/

BOOL LoadTuning (VOID)
{
	HFILE		hFile;
	OFSTRUCT	os;
	WORD		wSum = 0;
	static char	szFileName[MAX_PATH];
	WORD		nLength;
	LPVOID		lpData;
	char		szHeader[sizeof (MFILE_USER_TUNING_HEADER)];

	// If we ARE connected to the Morpheus
	if (fConnectedToMorpheus)
	{
		// Make sure they mean it, because they're about to overwrite all their Morpheus midimaps
		if (MyMessageBox (hwndMain,
				ResourceString (IDS_LOAD_UT_ARE_YOU_SURE),
				ResourceString (IDS_QUESTION_TITLE), MB_ICONQUESTION | MB_YESNO) == IDNO)
		{
			// Return without an error
			return (TRUE);
        }
	}

	// Ask the user for a file name
	if (!MyGetLoadFileName (hwndMain,
							ResourceString (IDS_LOAD_UT_FILEBOX_TITLE),
//							ResourceString (IDS_LOAD_UT_FILEBOX_MASK),
							USER_TUNING_FILE_MASK,
							&szFileName[0]))
	{
		// They hit CANCEL
		return (TRUE);
	}

	// Create the file (fixed name)
	memset (&os, 0, sizeof (OFSTRUCT));
	os.cBytes = sizeof (OFSTRUCT);
	if ((hFile = OpenFile (&szFileName[0], &os, OF_READ)) == HFILE_ERROR)
	{
		// Couldn't open the file
		MyMessageBox (hwndMain, ResourceString (IDS_ERROR_FILE_OPEN), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Read the file header
	if (_lread (hFile, &szHeader[0], sizeof (MFILE_USER_TUNING_HEADER)) != sizeof (MFILE_USER_TUNING_HEADER))
	{
		// Error
		MyMessageBox (hwndMain, ResourceString (IDS_ERROR_FILE_READ), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Did the header say the right thing ?
	if (memcmp (&szHeader[0], MFILE_USER_TUNING_HEADER, sizeof (MFILE_USER_TUNING_HEADER)))
	{
		// Error
		MyMessageBox (hwndMain, ResourceString (IDS_ERROR_WRONG_FILE_TYPE), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Is the file checksum OK ?
	if (!FileChecksumOk (hFile))
	{
		// Error
		MyMessageBox (hwndMain, ResourceString (IDS_ERROR_CORRUPT_FILE), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Read the table from the file
	if ((lpData = ReadLengthAndData (hFile, &nLength, &wSum)) == NULL)
	{
		// Error loading midimap
		MyMessageBox (hwndMain, ResourceString (IDS_LOAD_UT_ERROR), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}
	_fmemcpy (&MorpheusTuningTable, lpData, sizeof (TUNING_TABLE_PARAMS));

	// Close the file
	_lclose (hFile);

	// Display the tuning table
	DisplayTuning ();

	// If we are connected to the Morpheus
	if (fConnectedToMorpheus)
	{
		// Send the loaded tuning table to the Morpheus
		MorpheusSendTuning (hwndMain, &MorpheusTuningTable);
	}

	// OK
	return (TRUE);
}

