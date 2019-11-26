/*****************************************************************************
 File:			files.c
 Description:	Functions for loading and saving Morpheus ALL-SOUNDS files.
				The ALL-SOUNDS file format is :-

				1)		File header string.
				2)		Number of presets.
				3)		Number of hyperpresets.
				4)		Number of midimaps.
				5)		Preset length + data (* number of presets) 
				6)		Hyperpreset length + data (* number of hyperpresets)
				7)		Midimap length + data (* number of midimaps)
				8)		File checksum.

				If a file is loaded where there are too many presets,
				hyperpresets, or midimaps (ie. more than the configuration
				data says there should be), the user can cancel or
				only load as many as there should be.

				If the file has too FEW sounds or one or more types, the
				program refuses, since it would have to fill with blanks,
				which doesn't really have any meaning.

 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 10/6/94	| Created.											| PW
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
 Function:		SaveAllSounds
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Saves all Morpheus sounds.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 10/6/94	| Created.											| PW
*****************************************************************************/

BOOL SaveAllSounds (VOID)
{
	HFILE		hFile;
	OFSTRUCT	os;
	WORD		wSum = 0;
	static char	szFileName[MAX_PATH];
	WORD		i, NumPresets, NumHypers, NumMMs;
	WORD		nLength;

	// Make sure the presets list is loaded
	if (pMorpheusPresetList == NULL)
	{
		MyMessageBox (hwndMain, ResourceString (IDS_NOT_CONNECTED_NO_PR),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}
	NumPresets = WNUM (pMorpheusPresetList->wNumPresets);

	// If we aren't connected to the Morpheus
	if (!fConnectedToMorpheus)
	{
		// For each preset
		for (i=0 ; i<NumPresets ; i++)
		{
			// If it's not in memory
			if (pMorpheusPresets[i] == NULL)
			{
            	// Then we can't continue
				MyMessageBox (hwndMain,
						ResourceString (IDS_SAVE_ALL_NOT_CON_PRS),
						ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
				return (FALSE);
			}
		}
	}

	// Make sure the hyperpresets list is loaded
	if (pMorpheusHyperpresetList == NULL)
	{
		MyMessageBox (hwndMain, ResourceString (IDS_SAVE_ALL_HPS_NOT_LOADED),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}
	NumHypers = WNUM (pMorpheusHyperpresetList->wNumHyperpresets);

	// If we aren't connected to the Morpheus
	if (!fConnectedToMorpheus)
	{
		// For each hyperpreset
		for (i=0 ; i<NumHypers ; i++)
		{
			// If it's not in memory
			if (pMorpheusHyperPresets[i] == NULL)
			{
				// Then we can't continue
				MyMessageBox (hwndMain,
						ResourceString (IDS_SAVE_ALL_NOT_CON_HPS),
						ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
				return (FALSE);
			}
		}
	}

	// Make sure the midimaps list is loaded
	if (pMorpheusMidimapList == NULL)
	{
		MyMessageBox (hwndMain, ResourceString (IDS_SAVE_ALL_MMS_NOT_LOADED),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}
	NumMMs = WNUM (pMorpheusMidimapList->wNumMidimaps);

	// If we aren't connected to the Morpheus
	if (!fConnectedToMorpheus)
	{
		// For each midimap
		for (i=0 ; i<NumMMs ; i++)
		{
			// If it's not in memory
			if (pMorpheusMidimaps[i] == NULL)
			{
				// Then we can't continue
				MyMessageBox (hwndMain,
						ResourceString (IDS_SAVE_ALL_NOT_CON_MMS),
						ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
				return (FALSE);
			}
		}
	}

	// Ask the user for a file name
	if (!MyGetSaveFileName (hwndMain,
							ResourceString (IDS_SAVE_ALL_FILEBOX_TITLE),
							ALL_SOUNDS_FILE_MASK,
//							ResourceString (IDS_SAVE_ALL_FILEBOX_MASK),
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
	if (_lwrite (hFile, MFILE_SOUNDS_HEADER, sizeof (MFILE_SOUNDS_HEADER)) != sizeof (MFILE_SOUNDS_HEADER))
	{
		// Error
		_lclose (hFile);
		remove (&szFileName[0]);
		return (FALSE);
	}
	// Add to the checksum
	wSum += CalcChecksum (MFILE_SOUNDS_HEADER, sizeof (MFILE_SOUNDS_HEADER));

	// Write the number of presets
	if (_lwrite (hFile, (LPVOID)&NumPresets, sizeof (NumPresets)) != sizeof (NumPresets))
	{
		// Error
		_lclose (hFile);
		remove (&szFileName[0]);
		return (FALSE);
	}
	// Add to the checksum
	wSum += CalcChecksum (&NumPresets, sizeof (NumPresets));

	// Write the number of hyperpresets
	if (_lwrite (hFile, (LPVOID)&NumHypers, sizeof (NumHypers)) != sizeof (NumHypers))
	{
		// Error
		_lclose (hFile);
		remove (&szFileName[0]);
		return (FALSE);
	}
	// Add to the checksum
	wSum += CalcChecksum (&NumHypers, sizeof (NumHypers));

	// Write the number of midimaps
	if (_lwrite (hFile, (LPVOID)&NumMMs, sizeof (NumMMs)) != sizeof (NumMMs))
	{
		// Error
		_lclose (hFile);
		remove (&szFileName[0]);
		return (FALSE);
	}
	// Add to the checksum
	wSum += CalcChecksum (&NumMMs, sizeof (NumMMs));

	// For each preset
	nLength = sizeof (PRESET_PARAMS);
	for (i=0 ; i<NumPresets ; i++)
	{
		// If it's not in memory
		if (pMorpheusPresets[i] == NULL)
		{
			// If the user wants to abort
			if (AbortLongOperation ())
			{
				// Drop everything
				MyMessageBox (hwndMain, ResourceString (IDS_SAVE_ALL_ABORTED), "", MB_ICONINFORMATION | MB_OK);
				_lclose (hFile);
				remove (&szFileName[0]);
				return (FALSE);
			}

			// Fetch it from the Morpheus
			if (!MorpheusGetPreset (i))
			{
				// Error fetching preset from Morpheus
				MyMessageBox (hwndMain, ResourceString (IDS_SAVE_PR_ERROR_FETCHING), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
				_lclose (hFile);
				remove (&szFileName[0]);
				return (FALSE);
			}
		}

		// Save it
		if (!WriteLengthAndData (hFile, &nLength, pMorpheusPresets[i], &wSum))
		{
			// Error saving this preset
			MyMessageBox (hwndMain, ResourceString (IDS_SAVE_PR_ERROR_DISK), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			_lclose (hFile);
			remove (&szFileName[0]);
			return (FALSE);
		}
	}

	// For each hyperpreset
	nLength = sizeof (HYPERPRESET_PARAMS);
	for (i=0 ; i<NumHypers ; i++)
	{
		// If it's not in memory
		if (pMorpheusHyperPresets[i] == NULL)
		{
			// If the user wants to abort
			if (AbortLongOperation ())
			{
				// Drop everything
				MyMessageBox (hwndMain, ResourceString (IDS_SAVE_ALL_ABORTED), "", MB_ICONINFORMATION | MB_OK);
				_lclose (hFile);
				remove (&szFileName[0]);
				return (FALSE);
			}

			// Fetch it from the Morpheus
			if (!MorpheusGetHyper (i))
			{
				// Error fetching hyperpreset from Morpheus
				MyMessageBox (hwndMain, ResourceString (IDS_SAVE_HP_ERROR_FETCHING), ResourceString (IDS_ERROR_TITLE),
							MB_ICONEXCLAMATION | MB_OK);
				_lclose (hFile);
				remove (&szFileName[0]);
				return (FALSE);
			}
		}

		// Save it
		if (!WriteLengthAndData (hFile, &nLength, pMorpheusHyperPresets[i], &wSum))
		{
			// Error saving this hyperpreset
			MyMessageBox (hwndMain, ResourceString (IDS_SAVE_HP_ERROR_DISK), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			_lclose (hFile);
			remove (&szFileName[0]);
			return (FALSE);
		}
	}

	// For each midimap
	nLength = sizeof (MIDIMAP_PARAMS);
	for (i=0 ; i<NumMMs ; i++)
	{
		// If it's not in memory
		if (pMorpheusMidimaps[i] == NULL)
		{
			// If the user wants to abort
			if (AbortLongOperation ())
			{
				// Drop everything
				MyMessageBox (hwndMain, ResourceString (IDS_SAVE_ALL_ABORTED), "", MB_ICONINFORMATION | MB_OK);
				_lclose (hFile);
				remove (&szFileName[0]);
				return (FALSE);
			}

			// Fetch it from the Morpheus
			if (!MorpheusGetMM (i))
			{
				// Error fetching midimap from Morpheus
				MyMessageBox (hwndMain, ResourceString (IDS_SAVE_MM_ERROR_FETCHING), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
				_lclose (hFile);
				remove (&szFileName[0]);
				return (FALSE);
			}
		}

		// Save it
		if (!WriteLengthAndData (hFile, &nLength, pMorpheusMidimaps[i], &wSum))
		{
			// Error saving this midimap
			MyMessageBox (hwndMain, ResourceString (IDS_SAVE_MM_ERROR_DISK), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			_lclose (hFile);
			remove (&szFileName[0]);
			return (FALSE);
		}
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
 Function:		LoadAllSounds
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Loads all Morpheus sounds.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 10/6/94	| Created.											| PW
*****************************************************************************/

BOOL LoadAllSounds (VOID)
{
	HFILE		hFile;
	OFSTRUCT	os;
	WORD		wSum = 0;
	static char	szFileName[MAX_PATH];
	WORD		i, NumPresets, NumHypers, NumMMs, NumToLoad;
	WORD		nLength;
	char		szHeader[sizeof (MFILE_SOUNDS_HEADER)];

	// Make sure the configuration specified how many presets there are
	if ((WNUM (MorpheusConfigurationData.wNumPresets) == 0) ||
		(WNUM (MorpheusConfigurationData.wNumHyperpresets) == 0) ||
		(WNUM (MorpheusConfigurationData.wNumMidimaps) == 0))
	{
		MyMessageBox (hwndMain, ResourceString (IDS_LOAD_ALL_NO_CONFIG),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// If we ARE connected to the Morpheus
	if (fConnectedToMorpheus)
	{
		// Make sure they mean it, because they're about to overwrite all their Morpheus sounds
		if (MyMessageBox (hwndMain, ResourceString (IDS_LOAD_ALL_ARE_YOU_SURE),
			ResourceString (IDS_QUESTION_TITLE), MB_ICONQUESTION | MB_YESNO) == IDNO)
		{
			// Return without an error
			return (TRUE);
        }
	}

	// Ask the user for a file name
	if (!MyGetLoadFileName (hwndMain,
							ResourceString (IDS_LOAD_ALL_FILEBOX_TITLE),
//							ResourceString (IDS_LOAD_ALL_FILEBOX_MASK),
							ALL_SOUNDS_FILE_MASK,
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
	if (_lread (hFile, &szHeader[0], sizeof (MFILE_SOUNDS_HEADER)) != sizeof (MFILE_SOUNDS_HEADER))
	{
		// Error
		MyMessageBox (hwndMain, ResourceString (IDS_ERROR_FILE_READ),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// Did the header say the right thing ?
	if (memcmp (&szHeader[0], MFILE_SOUNDS_HEADER, sizeof (MFILE_SOUNDS_HEADER)))
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

	// Read the number of presets
	if (_lread (hFile, &NumPresets, sizeof (NumPresets)) != sizeof (NumPresets))
	{
		// Error
		MyMessageBox (hwndMain, ResourceString (IDS_ERROR_FILE_READ),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// If the number of presets in the file is greater than the configured number
	if (NumPresets > WNUM (MorpheusConfigurationData.wNumPresets))
	{
		// Error
		if (MyMessageBox (hwndMain, ResourceString (IDS_LOAD_ALL_TOO_MANY_PRS),
			ResourceString (IDS_WARNING_TITLE), MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL)
		{
			_lclose (hFile);
			return (FALSE);
		}
	}
	// Else if the number of presets in the file is less than the configured number
	else if (NumPresets < WNUM (MorpheusConfigurationData.wNumPresets))
	{
		// Error
		MyMessageBox (hwndMain, ResourceString (IDS_LOAD_ALL_TOO_FEW_PRS),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		{
			_lclose (hFile);
			return (FALSE);
		}
	}

	// Read the number of hyperpresets
	if (_lread (hFile, &NumHypers, sizeof (NumHypers)) != sizeof (NumHypers))
	{
		// Error
		MyMessageBox (hwndMain, ResourceString (IDS_ERROR_FILE_OPEN), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// If the number of hyperpresets in the file is greater than the configured number
	if (NumHypers > WNUM (MorpheusConfigurationData.wNumHyperpresets))
	{
		// Error
		if (MyMessageBox (hwndMain, ResourceString (IDS_LOAD_ALL_TOO_MANY_HPS),
			ResourceString (IDS_WARNING_TITLE), MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL)
		{
			_lclose (hFile);
			return (FALSE);
		}
	}
	// Else if the number of hyperpresets in the file is less than the configured number
	else if (NumHypers < WNUM (MorpheusConfigurationData.wNumHyperpresets))
	{
		// Error
		MyMessageBox (hwndMain, ResourceString (IDS_LOAD_ALL_TOO_FEW_HPS),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		{
			_lclose (hFile);
			return (FALSE);
		}
	}

	// Read the number of midimaps
	if (_lread (hFile, &NumMMs, sizeof (NumMMs)) != sizeof (NumMMs))
	{
		// Error
		MyMessageBox (hwndMain, ResourceString (IDS_ERROR_FILE_OPEN),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		return (FALSE);
	}

	// If the number of midimaps in the file is greater than the configured number
	if (NumMMs > WNUM (MorpheusConfigurationData.wNumMidimaps))
	{
		// Error
		if (MyMessageBox (hwndMain,	ResourceString (IDS_LOAD_ALL_TOO_MANY_MMS),
			ResourceString (IDS_WARNING_TITLE), MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL)
		{
			_lclose (hFile);
			return (FALSE);
		}
	}
	// Else if the number of midimaps in the file is less than the configured number
	else if (NumMMs < WNUM (MorpheusConfigurationData.wNumMidimaps))
	{
		// Error
		MyMessageBox (hwndMain, ResourceString (IDS_LOAD_ALL_TOO_FEW_HPS),
			ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		{
			_lclose (hFile);
			return (FALSE);
		}
	}

	// For each preset
	NumToLoad = WNUM (MorpheusConfigurationData.wNumPresets);
	for (i=0 ; i<NumPresets ; i++)
	{
		// If we are restricting the number of presets
		if (i > NumToLoad)
		{
			PPRESET_PARAMS	*pDummyPreset;

			// Throw away the preset
			pDummyPreset = ReadLengthAndData (hFile, &nLength, &wSum);
			GlobalFreePtr (pDummyPreset);
		}
		else
		{
			// If the preset is already in memory
			if (pMorpheusPresets[i] != NULL)
			{
				// Free it
				GlobalFreePtr (pMorpheusPresets[i]);
			}

			// Read the preset from the file
			if ((pMorpheusPresets[i] = ReadLengthAndData (hFile, &nLength, &wSum)) == NULL)
			{
				// Error loading preset
				MyMessageBox (hwndMain, ResourceString (IDS_LOAD_PR_ERROR), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
				_lclose (hFile);
				return (FALSE);
			}

			// If we are connected to the Morpheus
			if (fConnectedToMorpheus)
			{
				// Send the loaded preset to the Morpheus
				MorpheusSendPreset (hwndMain, i);
			}
		}
	}

	// For each hyperpreset
	NumToLoad = WNUM (MorpheusConfigurationData.wNumHyperpresets);
	for (i=0 ; i<NumHypers ; i++)
	{
		// If we are restricting the number of hyperpresets
		if (i > NumToLoad)
		{
			PHYPERPRESET_PARAMS	*pDummyHyper;

			// Throw away the hyperpreset
			pDummyHyper = ReadLengthAndData (hFile, &nLength, &wSum);
			GlobalFreePtr (pDummyHyper);
		}
		else
		{
			// If the hyperpreset is already in memory
			if (pMorpheusHyperPresets[i] != NULL)
			{
				// Free it
				GlobalFreePtr (pMorpheusHyperPresets[i]);
			}

			// Read the hyperpreset from the file
			if ((pMorpheusHyperPresets[i] = ReadLengthAndData (hFile, &nLength, &wSum)) == NULL)
			{
				// Error loading hyperpreset
				MyMessageBox (hwndMain, ResourceString (IDS_LOAD_HP_ERROR), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
				_lclose (hFile);
				return (FALSE);
			}

			// If we are connected to the Morpheus
			if (fConnectedToMorpheus)
			{
				// Send the loaded hyperpreset to the Morpheus
				MorpheusSendHyper (hwndMain, i);
			}
		}
	}

	// For each midimap
	NumToLoad = WNUM (MorpheusConfigurationData.wNumMidimaps);
	for (i=0 ; i<NumMMs ; i++)
	{
		// If we are restricting the number of midimaps
		if (i > NumToLoad)
		{
			PMIDIMAP_PARAMS	*pDummyMM;

			// Throw away the midimap
			pDummyMM = ReadLengthAndData (hFile, &nLength, &wSum);
			GlobalFreePtr (pDummyMM);
		}
		else
		{
			// If the midimap is already in memory
			if (pMorpheusMidimaps[i] != NULL)
			{
				// Free it
				GlobalFreePtr (pMorpheusMidimaps[i]);
			}

			// Read the midimap from the file
			if ((pMorpheusMidimaps[i] = ReadLengthAndData (hFile, &nLength, &wSum)) == NULL)
			{
				// Error loading midimap
				MyMessageBox (hwndMain, ResourceString (IDS_LOAD_MM_ERROR), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
				_lclose (hFile);
				return (FALSE);
			}

			// If we are connected to the Morpheus
			if (fConnectedToMorpheus)
			{
				// Send the loaded midimap to the Morpheus
				MorpheusSendMM (hwndMain, i);
			}
		}
	}

	// Close the file
	_lclose (hFile);

	// Create the lists of names from the loaded sounds.
	CreatePresetNames (INDEX_NONE);
	CreateHyperNames (INDEX_NONE);
	CreateMMNames (INDEX_NONE);

	// Display the lists of names
	DisplayPresetsList ();
	DisplayHypersList ();
	DisplayMMsList ();

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		CreatePresetNames
 Parameters:	int		nPreset			Preset number, or INDEX_NONE for all.
 Returns:		None.
 Description:	Creates a list of preset names from the loaded presets.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 8/5/94		| Created.											| PW
*****************************************************************************/

VOID CreatePresetNames (int nPreset)
{
	PPRESET_PARAMS	pPreset;
	WORD			i, n, num;

	// If we are doing ALL presets
	if (nPreset == INDEX_NONE)
	{
		// If there is already a preset list
		if (pMorpheusPresetList != NULL)
		{
			// Free it
			GlobalFreePtr (pMorpheusPresetList);
		}

		num = ReadSysexWord (&MorpheusConfigurationData.wNumPresets);

		// Allocate some memory for the list
		if ((pMorpheusPresetList = (PPRESET_LIST_PARAMS)GlobalAllocPtr (GMEM_FIXED,
				sizeof (PRESET_LIST_PARAMS) + (num - 1) * sizeof (pMorpheusPresetList->Info))) == NULL)
		{
			// Error loading preset
			MyMessageBox (hwndMain, ResourceString (IDS_OUT_OF_MEMORY), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			return;
		}

		// Fill in the list
		WriteSysexWord (&pMorpheusPresetList->wNumPresets, num);
		for (i=0 ; i<num ; i++)
		{
			pPreset = pMorpheusPresets[i];
			for (n=0 ; n<ARRAY_SIZE(pPreset->Name) ; n++)
			{
				pMorpheusPresetList->Info[i].szName[n] = (BYTE)pPreset->Name[n];
			}
			pMorpheusPresetList->Info[i].szName[n] = '\0';
		}
	}
	else
	// Updating a single preset
	{
		pPreset = pMorpheusPresets[nPreset];
		for (n=0 ; n<ARRAY_SIZE(pPreset->Name) ; n++)
		{
			pMorpheusPresetList->Info[nPreset].szName[n] = (BYTE)pPreset->Name[n];
		}
		pMorpheusPresetList->Info[nPreset].szName[n] = '\0';
	}
}

/*^L*/
/*****************************************************************************
 Function:		CreateHyperNames
 Parameters:	int		nHyper			Hyperpreset number, or INDEX_NONE for all.
 Returns:		None.
 Description:	Creates a list of hyperpreset names from the loaded hyperpresets.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 2/6/94		| Created.											| PW
*****************************************************************************/

VOID CreateHyperNames (int nHyper)
{
	PHYPERPRESET_PARAMS	pHyper;
	WORD				i, n, num;

	// If we're updating ALL names
	if (nHyper == INDEX_NONE)
    {
		// If there is already a hyperpreset list
		if (pMorpheusHyperpresetList != NULL)
		{
			// Free it
			GlobalFreePtr ((LPVOID)pMorpheusHyperpresetList);
		}

		// Extract the number of hyperpresets from the configuration data
		num = ReadSysexWord (&MorpheusConfigurationData.wNumHyperpresets);

		// Allocate some memory for the list
		if ((pMorpheusHyperpresetList = (PHYPERPRESET_LIST_PARAMS)GlobalAllocPtr (GMEM_FIXED,
				sizeof (HYPERPRESET_LIST_PARAMS) + (num - 1) * sizeof (pMorpheusHyperpresetList->Info))) == NULL)
		{
			// Out of memory
			MyMessageBox (hwndMain, ResourceString (IDS_OUT_OF_MEMORY), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			return;
		}

		// Fill in the list
		WriteSysexWord (&pMorpheusHyperpresetList->wNumHyperpresets, num);
		for (i=0 ; i<num ; i++)
		{
			pHyper = pMorpheusHyperPresets[i];
			for (n=0 ; n<ARRAY_SIZE(pHyper->Name) ; n++)
			{
				pMorpheusHyperpresetList->Info[i].szName[n] = (BYTE)pHyper->Name[n];
			}
			pMorpheusHyperpresetList->Info[i].szName[n] = '\0';
		}
	}
	else
	// Updating a single hyperpreset
	{
		pHyper = pMorpheusHyperPresets[nHyper];
		for (n=0 ; n<ARRAY_SIZE(pHyper->Name) ; n++)
		{
			pMorpheusHyperpresetList->Info[nHyper].szName[n] = (BYTE)pHyper->Name[n];
		}
		pMorpheusHyperpresetList->Info[nHyper].szName[n] = '\0';
	}
}

/*^L*/
/*****************************************************************************
 Function:		CreateMMNames
 Parameters:	int		nMM				Midimap number, or INDEX_NONE for all.
 Returns:		None.
 Description:	Creates a list of midimap names from the loaded midimaps.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 10/6/94	| Created.											| PW
*****************************************************************************/

VOID CreateMMNames (int nMM)
{
	PMIDIMAP_PARAMS	pMM;
	WORD			i, n, num;

	// if we're updating ALL names
    if (nMM == INDEX_NONE)
	{
		// If there is already a midimap list
		if (pMorpheusMidimapList != NULL)
		{
			// Free it
			GlobalFreePtr ((LPVOID)pMorpheusMidimapList);
		}

		// Extract the number of midimaps from the configuration data
		num = ReadSysexWord (&MorpheusConfigurationData.wNumMidimaps);

		// Allocate some memory for the list
		if ((pMorpheusMidimapList = (PMIDIMAP_LIST_PARAMS)GlobalAllocPtr (GMEM_FIXED,
				sizeof (MIDIMAP_LIST_PARAMS) + (num - 1) * sizeof (pMorpheusMidimapList->Info))) == NULL)
		{
			// Out of memory
			MyMessageBox (hwndMain, ResourceString (IDS_OUT_OF_MEMORY), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			return;
		}

		// Fill in the list
		WriteSysexWord (&pMorpheusMidimapList->wNumMidimaps, num);
		for (i=0 ; i<num ; i++)
		{
			pMM = pMorpheusMidimaps[i];
			for (n=0 ; n<ARRAY_SIZE(pMM->Name) ; n++)
			{
				pMorpheusMidimapList->Info[i].szName[n] = (BYTE)pMM->Name[n];
			}
			pMorpheusMidimapList->Info[i].szName[n] = '\0';
		}
	}
	else
    // We're updating a single name
	{
		pMM = pMorpheusMidimaps[nMM];
		for (n=0 ; n<ARRAY_SIZE(pMM->Name) ; n++)
		{
			pMorpheusMidimapList->Info[nMM].szName[n] = (BYTE)pMM->Name[n];
		}
		pMorpheusMidimapList->Info[nMM].szName[n] = '\0';
	}
}

