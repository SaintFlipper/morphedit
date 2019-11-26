/*****************************************************************************
 File:			files.c
 Description:	Functions for loading and saving Morpheus PRESET files. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 8/5/94		| Created.											| PW
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
 Function:		SavePreset
 Parameters:	int		idPreset		Preset number.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Saves a single Morpheus preset.

				The format of a PRESET file is :-

				1)	File header (text)
				2)	Preset length
				3)	Preset data
				4)	Word checksum (sum of everything up to here).
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 27/5/94		| Created.											| PW
*****************************************************************************/

BOOL SavePreset (int idPreset)
{
	HFILE		hFile;
	OFSTRUCT	os;
	WORD		wSum = 0;
	static char	szFileName[MAX_PATH];
	WORD		nLength = sizeof (PRESET_PARAMS);

	// If we aren't connected to the Morpheus
	if (!fConnectedToMorpheus)
	{
		// If it's not in memory
		if (pMorpheusPresets[idPreset] == NULL)
		{
			// Then we can't continue
			MyMessageBox (hwndMain, ResourceString (IDS_NOT_CONNECTED_NO_PR),
				ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			return (FALSE);
		}
	}

	// Ask the user for a file name
	if (!MyGetSaveFileName (hwndMain,
							ResourceString (IDS_SAVE_PR_FILEBOX_TITLE),
//							ResourceString (IDS_SAVE_PR_FILEBOX_MASK),
							PRESETS_FILE_MASK,
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
	if (_lwrite (hFile, MFILE_PRESET_HEADER, sizeof (MFILE_PRESET_HEADER)) != sizeof (MFILE_PRESET_HEADER))
	{
		// Error
		_lclose (hFile);
		remove (&szFileName[0]);
		return (FALSE);
	}
	// Add to the checksum
	wSum += CalcChecksum (MFILE_PRESET_HEADER, sizeof (MFILE_PRESET_HEADER));

	// If it's not in memory
	if (pMorpheusPresets[idPreset] == NULL)
	{
		// Fetch it from the Morpheus
		if (!MorpheusGetPreset (idPreset))
		{
			// Error fetching preset from Morpheus
			MyMessageBox (hwndMain,
				ResourceString (IDS_SAVE_PR_ERROR_FETCHING),
				ResourceString (IDS_ERROR_TITLE),
				MB_ICONEXCLAMATION | MB_OK);
			_lclose (hFile);
			remove (&szFileName[0]);
			return (FALSE);
		}
	}

	// Save it
	if (!WriteLengthAndData (hFile, &nLength, pMorpheusPresets[idPreset], &wSum))
	{
		// Error saving this preset
		MyMessageBox (hwndMain, ResourceString (IDS_SAVE_PR_ERROR_DISK),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
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
 Function:		LoadPreset
 Parameters:	int		idPreset		Preset number.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Loads a single Morpheus preset.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 27/5/94		| Created.											| PW
*****************************************************************************/

BOOL LoadPreset (int idPreset)
{
	HFILE		hFile;
	OFSTRUCT	os;
	WORD		wSum = 0;
	static char	szFileName[MAX_PATH];
	WORD		nLength;
	char		szHeader[sizeof (MFILE_PRESET_HEADER)];

	// If we ARE connected to the Morpheus
	if (fConnectedToMorpheus)
	{
		// Make sure they mean it, because they're about to overwrite all their Morpheus presets
		if (MyMessageBox (hwndMain,
				ResourceString (IDS_LOAD_PR_ARE_YOU_SURE),
				ResourceString (IDS_QUESTION_TITLE),
				MB_ICONQUESTION | MB_YESNO) == IDNO)
		{
			// Return without an error
			return (TRUE);
        }
	}

	// Ask the user for a file name
	if (!MyGetLoadFileName (hwndMain,
							ResourceString (IDS_LOAD_PR_FILEBOX_TITLE),
//							ResourceString (IDS_LOAD_PR_FILEBOX_MASK),
							PRESETS_FILE_MASK,
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
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Read the file header
	if (_lread (hFile, &szHeader[0], sizeof (MFILE_PRESET_HEADER)) != sizeof (MFILE_PRESET_HEADER))
	{
		// Error
		MyMessageBox (hwndMain, ResourceString (IDS_ERROR_FILE_READ),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Did the header say the right thing ?
	if (memcmp (&szHeader[0], MFILE_PRESET_HEADER, sizeof (MFILE_PRESET_HEADER)))
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

	// If the preset is already in memory
	if (pMorpheusPresets[idPreset] != NULL)
	{
		// Free it
		GlobalFreePtr (pMorpheusPresets[idPreset]);
	}

	// Read the preset from the file
	if ((pMorpheusPresets[idPreset] = ReadLengthAndData (hFile, &nLength, &wSum)) == NULL)
	{
		// Error loading preset
		MyMessageBox (hwndMain, ResourceString (IDS_LOAD_PR_ERROR),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Close the file
	_lclose (hFile);

	// Update the list of preset names from the loaded preset.
	CreatePresetNames (idPreset);

	// Display the list of presets
	DisplayPresetsList ();

	// Display the preset
	DisplayPreset ();

	// If we are connected to the Morpheus
	if (fConnectedToMorpheus)
	{
		// Send the loaded preset to the Morpheus
		MorpheusSendPreset (hwndMain, idPreset);
	}

	// OK
	return (TRUE);
}

