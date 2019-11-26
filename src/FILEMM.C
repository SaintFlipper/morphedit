/*****************************************************************************
 File:			filmm.c
 Description:	Functions for loading and saving Morpheus MIDIMAP files. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 15/6/94	| Created.											| PW
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
 Function:		SaveMM
 Parameters:	int		idMM			Midimap number.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Saves a single Morpheus midimap.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 15/6/94	| Created.											| PW
*****************************************************************************/

BOOL SaveMM (int idMM)
{
	HFILE		hFile;
	OFSTRUCT	os;
	WORD		wSum = 0;
	static char	szFileName[MAX_PATH];
	WORD		nLength = sizeof (MIDIMAP_PARAMS);

	// If we aren't connected to the Morpheus
	if (!fConnectedToMorpheus)
	{
		// If it's not in memory
		if (pMorpheusMidimaps[idMM] == NULL)
		{
			// Then we can't continue
			MyMessageBox (hwndMain, ResourceString (IDS_NOT_CONNECTED_NO_MM),
				ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			return (FALSE);
		}
	}

	// Ask the user for a file name
	if (!MyGetSaveFileName (hwndMain,
				ResourceString (IDS_SAVE_MM_FILEBOX_TITLE),
//				ResourceString (IDS_SAVE_MM_FILEBOX_MASK),
				MIDIMAPS_FILE_MASK,
				&szFileName[0])
				)
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
	if (_lwrite (hFile, MFILE_MIDIMAP_HEADER, sizeof (MFILE_MIDIMAP_HEADER)) != sizeof (MFILE_MIDIMAP_HEADER))
	{
		// Error
		_lclose (hFile);
		remove (&szFileName[0]);
		return (FALSE);
	}
	// Add to the checksum
	wSum += CalcChecksum (MFILE_MIDIMAP_HEADER, sizeof (MFILE_MIDIMAP_HEADER));

	// If it's not in memory
	if (pMorpheusMidimaps[idMM] == NULL)
	{
		// Fetch it from the Morpheus
		if (!MorpheusGetMM (idMM))
		{
			// Error fetching midimap from Morpheus
			MyMessageBox (hwndMain,
					ResourceString (IDS_SAVE_MM_ERROR_FETCHING),
					ResourceString (IDS_ERROR_TITLE),
					MB_ICONEXCLAMATION | MB_OK);
			_lclose (hFile);
			remove (&szFileName[0]);
			return (FALSE);
		}
	}

	// Save it
	if (!WriteLengthAndData (hFile, &nLength, pMorpheusMidimaps[idMM], &wSum))
	{
		// Error saving this preset
		MyMessageBox (hwndMain, ResourceString (IDS_SAVE_MM_ERROR_DISK),
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
 Function:		LoadMM
 Parameters:	int		idMM			Midimap number.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Loads a single Morpheus midimap.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 15/6/94	| Created.											| PW
*****************************************************************************/

BOOL LoadMM (int idMM)
{
	HFILE		hFile;
	OFSTRUCT	os;
	WORD		wSum = 0;
	static char	szFileName[MAX_PATH];
	WORD		nLength;
	char		szHeader[sizeof (MFILE_MIDIMAP_HEADER)];

	// If we ARE connected to the Morpheus
	if (fConnectedToMorpheus)
	{
		// Make sure they mean it, because they're about to overwrite all their Morpheus midimaps
		if (MyMessageBox (hwndMain,
				ResourceString (IDS_LOAD_MM_ARE_YOU_SURE),
				ResourceString (IDS_QUESTION_TITLE),
				MB_ICONQUESTION | MB_YESNO) == IDNO)
		{
			// Return without an error
			return (TRUE);
        }
	}

	// Ask the user for a file name
	if (!MyGetLoadFileName (hwndMain,
							ResourceString (IDS_LOAD_MM_FILEBOX_TITLE),
//							ResourceString (IDS_LOAD_MM_FILEBOX_MASK),
							MIDIMAPS_FILE_MASK,
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
	if (_lread (hFile, &szHeader[0], sizeof (MFILE_MIDIMAP_HEADER)) != sizeof (MFILE_MIDIMAP_HEADER))
	{
		// Error
		MyMessageBox (hwndMain, ResourceString (IDS_ERROR_FILE_READ),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Did the header say the right thing ?
	if (memcmp (&szHeader[0], MFILE_MIDIMAP_HEADER, sizeof (MFILE_MIDIMAP_HEADER)))
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

	// If the midimap is already in memory
	if (pMorpheusMidimaps[idMM] != NULL)
	{
		// Free it
		GlobalFreePtr (pMorpheusMidimaps[idMM]);
	}

	// Read the midimap from the file
	if ((pMorpheusMidimaps[idMM] = ReadLengthAndData (hFile, &nLength, &wSum)) == NULL)
	{
		// Error loading midimap
		MyMessageBox (hwndMain, ResourceString (IDS_LOAD_MM_ERROR),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Close the file
	_lclose (hFile);

	// Update the list of midimap names from the loaded hyperpreset.
	CreateMMNames (idMM);

	// Display the list of midimaps
	DisplayMMsList ();

	// Display the midimap
	DisplayMM (idMM);

	// If we are connected to the Morpheus
	if (fConnectedToMorpheus)
	{
		// Send the loaded midimap to the Morpheus
		MorpheusSendMM (hwndMain, idMM);
	}

	// OK
	return (TRUE);
}

