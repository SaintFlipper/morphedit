/*****************************************************************************
 File:			files.c
 Description:	Functions for loading and saving Morpheus HYPERPRESET files. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 1/6/94		| Created.											| PW
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
 Function:		SaveHyper
 Parameters:	int		idHyper			Hyperpreset number.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Saves a single Morpheus hyperpreset.

				The format of a HYPERPRESET file is :-

				1)	File header (text)
				2)	Hyperpreset length
				3)	Hyperpreset data
				4)	Word checksum (sum of everything up to here).
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 2/6/94		| Created.											| PW
*****************************************************************************/

BOOL SaveHyper (int idHyper)
{
	HFILE		hFile;
	OFSTRUCT	os;
	WORD		wSum = 0;
	static char	szFileName[MAX_PATH];
	WORD		nLength = sizeof (HYPERPRESET_PARAMS);

	// If we aren't connected to the Morpheus
	if (!fConnectedToMorpheus)
	{
		// If it's not in memory
		if (pMorpheusHyperPresets[idHyper] == NULL)
		{
			// Then we can't continue
			MyMessageBox (hwndMain, ResourceString (IDS_NOT_CONNECTED_NO_HP),
							ResourceString (IDS_ERROR_TITLE),
							MB_ICONEXCLAMATION | MB_OK);
			return (FALSE);
		}
	}

	// Ask the user for a file name
	if (!MyGetSaveFileName (hwndMain,
							ResourceString (IDS_SAVE_HP_FILEBOX_TITLE),
//							ResourceString (IDS_SAVE_HP_FILEBOX_MASK),
							HYPERPRESETS_FILE_MASK,
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
	if (_lwrite (hFile, MFILE_HYPERPRESET_HEADER, sizeof (MFILE_HYPERPRESET_HEADER)) != sizeof (MFILE_HYPERPRESET_HEADER))
	{
		// Error
		_lclose (hFile);
		remove (&szFileName[0]);
		return (FALSE);
	}
	// Add to the checksum
	wSum += CalcChecksum (MFILE_HYPERPRESET_HEADER, sizeof (MFILE_HYPERPRESET_HEADER));

	// If it's not in memory
	if (pMorpheusHyperPresets[idHyper] == NULL)
	{
		// Fetch it from the Morpheus
		if (!MorpheusGetHyper (idHyper))
		{
			// Error fetching hyperpreset from Morpheus
			MyMessageBox (hwndMain,
				ResourceString (IDS_SAVE_HP_ERROR_FETCHING),
				ResourceString (IDS_ERROR_TITLE),
				MB_ICONEXCLAMATION | MB_OK);
			_lclose (hFile);
			remove (&szFileName[0]);
			return (FALSE);
		}
	}

	// Save it
	if (!WriteLengthAndData (hFile, &nLength, pMorpheusHyperPresets[idHyper], &wSum))
	{
		// Error saving this preset
		MyMessageBox (hwndMain, ResourceString (IDS_SAVE_HP_ERROR_DISK),
						ResourceString (IDS_ERROR_TITLE),
						MB_ICONEXCLAMATION | MB_OK);
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
 Function:		LoadHyper
 Parameters:	int		idHyper			Hyperpreset number.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Loads a single Morpheus hyperpreset.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 27/5/94		| Created.											| PW
*****************************************************************************/

BOOL LoadHyper (int idHyper)
{
	HFILE		hFile;
	OFSTRUCT	os;
	WORD		wSum = 0;
	static char	szFileName[MAX_PATH];
	WORD		nLength;
	char		szHeader[sizeof (MFILE_HYPERPRESET_HEADER)];

	// If we ARE connected to the Morpheus
	if (fConnectedToMorpheus)
	{
		// Make sure they mean it, because they're about to overwrite all their Morpheus hyperpresets
		if (MyMessageBox (hwndMain,
				ResourceString (IDS_LOAD_HP_ARE_YOU_SURE),
				ResourceString (IDS_QUESTION_TITLE),
				MB_ICONQUESTION | MB_YESNO) == IDNO)
		{
			// Return without an error
			return (TRUE);
        }
	}

	// Ask the user for a file name
	if (!MyGetLoadFileName (hwndMain,
							ResourceString (IDS_LOAD_HP_FILEBOX_TITLE),
//							ResourceString (IDS_LOAD_HP_FILEBOX_MASK),
							HYPERPRESETS_FILE_MASK,
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
		MyMessageBox (hwndMain, ResourceString (IDS_ERROR_FILE_OPEN),
						ResourceString (IDS_ERROR_TITLE),
						MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Read the file header
	if (_lread (hFile, &szHeader[0], sizeof (MFILE_HYPERPRESET_HEADER)) != sizeof (MFILE_HYPERPRESET_HEADER))
	{
		// Error
		MyMessageBox (hwndMain, ResourceString (IDS_ERROR_FILE_READ),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Did the header say the right thing ?
	if (memcmp (&szHeader[0], MFILE_HYPERPRESET_HEADER, sizeof (MFILE_HYPERPRESET_HEADER)))
	{
		// Error
		MyMessageBox (hwndMain, ResourceString (IDS_ERROR_WRONG_FILE_TYPE),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Is the file checksum OK ?
	if (!FileChecksumOk (hFile))
	{
		// Error
		MyMessageBox (hwndMain, ResourceString (IDS_ERROR_CORRUPT_FILE),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// If the hyperpreset is already in memory
	if (pMorpheusHyperPresets[idHyper] != NULL)
	{
		// Free it
		GlobalFreePtr (pMorpheusHyperPresets[idHyper]);
	}

	// Read the preset from the file
	if ((pMorpheusHyperPresets[idHyper] =
		ReadLengthAndData (hFile, &nLength, &wSum)) == NULL)
	{
		// Error loading hyperpreset
		MyMessageBox (	hwndMain,
						ResourceString (IDS_LOAD_HP_ERROR),
						ResourceString (IDS_ERROR_TITLE),
						MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Close the file
	_lclose (hFile);

	// Update the list of hyperpreset names from the loaded hyperpreset.
	CreateHyperNames (idHyper);

	// Display the list of hyperpresets
	DisplayHypersList ();

	// Display the hyperpreset
	DisplayHyper (idHyper);

	// If we are connected to the Morpheus
	if (fConnectedToMorpheus)
	{
		// Send the loaded hyperpreset to the Morpheus
		MorpheusSendHyper (hwndMain, idHyper);
	}

	// OK
	return (TRUE);
}

