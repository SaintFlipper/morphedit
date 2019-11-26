/*****************************************************************************
 File:			midirx.c
 Description:	The high-level MIDI receive module.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 8/4//94	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define	PARAM_BUG

#define	MAX_EFFECT_PARAMS		10
#define	EFFECT_PARAM_NAME_LEN	11

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

// Array of parameter names
static char					ParamNames[MAX_EFFECT_PARAMS][EFFECT_PARAM_NAME_LEN + 1];
static EFFECT_PARAM_INFO	EffectParamInfo[MAX_EFFECT_PARAMS] =
{
	{&ParamNames[0][0],		0,	0,	0},
	{&ParamNames[1][0],		0,	0,	0},
	{&ParamNames[2][0],		0,	0,	0},
	{&ParamNames[3][0],		0,	0,	0},
	{&ParamNames[4][0],		0,	0,	0},
	{&ParamNames[5][0],		0,	0,	0},
	{&ParamNames[6][0],		0,	0,	0},
	{&ParamNames[7][0],		0,	0,	0},
	{&ParamNames[8][0],		0,	0,	0},
	{&ParamNames[9][0],		0,	0,	0},
};

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

static BOOL MorpheusGetAllProgramMaps (VOID);


/*^L*/
/*****************************************************************************
 Function:		MorpheusGetAll
 Parameters:	None.	
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Reads the following Morpheus data :-
				1)	List of preset names
				2)	List of hyperpreset names
				3)	List of midimap names
				4)	Tuning table
				5)	Master settings
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 8/4/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusGetAll (VOID)
{
	// Make absolutely sure there are no old RX buffers hanging around
	MidiRxFreeAll ();

	/*
		Read the following :-
		a)	List of preset names.
		b)	List of hyperpreset names.
		c)	List of midimap names.
		d)	User tuning table.
		e)	All program maps.
	 */
	if (
		!MorpheusGetPresetsList ()					||
		!MorpheusGetHyperpresetsList ()				||
		!MorpheusGetMidimapsList ()					||
		!MorpheusGetTuning (&MorpheusTuningTable)	||
		!MorpheusGetAllProgramMaps ()
		)
	{
		MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));
		MidiRxFreeAll ();
		return (FALSE);
	}

	// Now that we've got everything from the Morpheus, any existing selection is invalid
	nDisplayedPreset	= INDEX_NONE;
	nDisplayedHyper		= INDEX_NONE;
	nDisplayedMM		= INDEX_NONE;

	// Update MIDI status
	MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusGetInternals
 Parameters:	None.	
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Reads the following Morpheus data :-
				1)	List of preset names
				2)	List of hyperpreset names
				3)	List of midimap names
				4)	Tuning table
				5)	Master settings
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 25/4/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusGetInternals (VOID)
{
	// Make absolutely sure there are no old RX buffers hanging around
//	MidiRxFreeInternals ();

	/*
		1)	Read all the fixed information; the instrument list, filter list, effects list, version info
	 */
	if (
		!MorpheusGetVersionData ()			||
		!MorpheusGetConfigurationData ()	||
		!MorpheusGetInstrumentList ()		||
		!MorpheusGetFilterList ()			||
		!MorpheusGetEffectList ()
		)
	{
		MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));
//		MidiRxFreeInternals ();			// Panic - free everything allocated
		return (FALSE);
	}

	// Update PRESET display with internal information
    UpdatePresetInternals ();

	// OK
	MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		MidiRxFreeAll
 Parameters:	None.	
 Returns:		None.
 Description:	Frees all allocated Morpheus RX data buffers.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 24/4/94 	| Created.											| PW
*****************************************************************************/

VOID MidiRxFreeAll (VOID)
{
	UINT	i;

	// If there is a preset list hanging around
	if (pMorpheusPresetList != NULL)
	{
		// Free it
		GlobalFreePtr ((LPVOID)pMorpheusPresetList);
		pMorpheusPresetList = NULL;
	}

	// If there is a hyperpreset list hanging around
	if (pMorpheusHyperpresetList != NULL)
	{
		// Free it
		GlobalFreePtr ((LPVOID)pMorpheusHyperpresetList);
		pMorpheusHyperpresetList = NULL;
	}

	// If there is a midimap list hanging around
	if (pMorpheusMidimapList != NULL)
	{
		// Free it
		GlobalFreePtr ((LPVOID)pMorpheusMidimapList);
		pMorpheusMidimapList = NULL;
	}

	// For each possible preset
	for (i=0 ; i<MAX_PRESETS ; i++)
	{
		// If there is already an buffer for the preset
		if (pMorpheusPresets[i] != NULL)
		{
			// Free it
			GlobalFreePtr ((LPVOID)pMorpheusPresets[i]);
			pMorpheusPresets[i] = NULL;
		}
	}

	// For each possible hyperpreset
	for (i=0 ; i<MAX_HYPERPRESETS ; i++)
	{
		// If there is already an buffer for the hyperpreset
		if (pMorpheusHyperPresets[i] != NULL)
		{
			// Free it
			GlobalFreePtr ((LPVOID)pMorpheusHyperPresets[i]);
			pMorpheusHyperPresets[i] = NULL;
        }
	}

	// For each possible midimap
	for (i=0 ; i<MAX_MIDIMAPS ; i++)
	{
		// If there is already an buffer for the midimap
		if (pMorpheusMidimaps[i] != NULL)
		{
			// Free it
			GlobalFreePtr ((LPVOID)pMorpheusMidimaps[i]);
			pMorpheusMidimaps[i] = NULL;
        }
	}
}

/*^L*/
/*****************************************************************************
 Function:		MidiRxFreeInternals
 Parameters:	None.	
 Returns:		None.
 Description:	Frees all allocated Morpheus RX data buffers.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 25/4/94 	| Created.											| PW
*****************************************************************************/

VOID MidiRxFreeInternals (VOID)
{
	// If there is already an allocated instrument list buffer - free it
	if (pMorpheusInstrumentList != NULL)
	{
		GlobalFreePtr ((LPVOID)pMorpheusInstrumentList);
		pMorpheusInstrumentList = NULL;
	}

	// If there is already an allocated filter list buffer - free it
	if (pMorpheusFilterList != NULL)
	{
		GlobalFreePtr ((LPVOID)pMorpheusFilterList);
		pMorpheusFilterList = NULL;
	}

	// If there is already an allocated effect list buffer - free it
	if (pMorpheusEffectList != NULL)
	{
		GlobalFreePtr ((LPVOID)pMorpheusEffectList);
		pMorpheusEffectList = NULL;
	}
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusGetConfigurationData
 Parameters:	None.	
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Reads the Morpheus configuration data.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 8/4/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusGetConfigurationData (VOID)
{
	HPBYTE	hpRxData;
	DWORD	dwRxLength;

	MidiStatus (ResourceString (IDS_MIDIRX_READING_CONFIG));

	// Get the data (hopefully)
	if (GetMorpheusData (hwndMain, CONFIGURATION_REQUEST, NO_PARAM, NO_PARAM, &hpRxData, &dwRxLength))
	{
		// Copy the data
		hmemcpy (&MorpheusConfigurationData, hpRxData, sizeof (CONFIGURATION_DATA_PARAMS));

		// Free the allocated RX buffer
		GlobalFreePtr (hpRxData);
	}
	else
	{
		// Error
        return (FALSE);
	}

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusGetVersionData
 Parameters:	None.	
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Reads the Morpheus version data.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 8/4/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusGetVersionData (VOID)
{
	HPBYTE	hpRxData;
	DWORD	dwRxLength;

	MidiStatus (ResourceString (IDS_MIDIRX_READING_VERSION));

	// Get the data (hopefully)
	if (GetMorpheusData (hwndMain, VERSION_REQUEST, NO_PARAM, NO_PARAM, &hpRxData, &dwRxLength))
	{
		// Copy the data
		hmemcpy (&MorpheusVersionData, hpRxData, sizeof (VERSION_PARAMS));

		// Free the allocated RX buffer
		GlobalFreePtr (hpRxData);
	}
	else
	{
		// Error
        return (FALSE);
	}

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusGetPreset
 Parameters:	int	idPreset			Preset number to read.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Reads the specified Morpheus preset into an allocate buffer.
				The pointer to the preset buffer is stored in
				pMorpheusPresets[idPreset].
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 24/4/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusGetPreset (int idPreset)
{
	HPBYTE	hpRxData;
	DWORD	dwRxLength;
	char	szStatusBuf[100];

	// Display the MIDI status
	wsprintf (&szStatusBuf[0], ResourceString (IDS_MIDIRX_READING_PRESET), idPreset);
	MidiStatus (&szStatusBuf[0]);

	// If there is already a preset buffer
	if (pMorpheusPresets[idPreset] != NULL)
	{
		// Free it
		GlobalFreePtr ((LPVOID)pMorpheusPresets[idPreset]);
	}

	// Get the data (hopefully)
	if (GetMorpheusData (hwndMain, PRESET_REQUEST, NO_PARAM, idPreset, &hpRxData, &dwRxLength))
	{
		// Store the preset buffer
		pMorpheusPresets[idPreset] = (PPRESET_PARAMS)hpRxData;
		MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));

		// Check the checksum is OK
		if (MidiChecksum (&pMorpheusPresets[idPreset]->Name[0], PRESET_CHECKSUM_LENGTH) !=
													pMorpheusPresets[idPreset]->CheckSum)
		{
			MyMessageBox (NULL, ResourceString (IDS_MIDIRX_CHECKSUM_ERROR), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			GlobalFreePtr ((LPVOID)pMorpheusPresets[idPreset]);
			pMorpheusPresets[idPreset] = (PPRESET_PARAMS)NULL;
			return (FALSE);
		}

		// OK
		return (TRUE);
	}

	// Error
	MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));
	return (FALSE);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusGetHyper
 Parameters:	int	idHyper				Hyperpreset number to read.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Reads the specified Morpheus hyperpreset into an allocated
				buffer. The pointer to the hyperpreset buffer is stored in
				pMorpheusHyperPresets[idHyper].
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 1/6/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusGetHyper (int idHyper)
{
	HPBYTE	hpRxData;
	DWORD	dwRxLength;
	char	szStatusBuf[100];

	// Display the MIDI status
	wsprintf (&szStatusBuf[0], ResourceString (IDS_MIDIRX_READING_HP), idHyper);
	MidiStatus (&szStatusBuf[0]);

	// If there is already a hyperpreset buffer
	if (pMorpheusHyperPresets[idHyper] != NULL)
	{
		// Free it
		GlobalFreePtr ((LPVOID)pMorpheusHyperPresets[idHyper]);
	}

	// Get the data (hopefully)
	if (GetMorpheusData (hwndMain, HYPERPRESET_REQUEST, NO_PARAM, idHyper, &hpRxData, &dwRxLength))
	{
		// Store the buffer
		pMorpheusHyperPresets[idHyper] = (PHYPERPRESET_PARAMS)hpRxData;
		MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));

		// Check the checksum is OK
		if (MidiChecksum (&pMorpheusHyperPresets[idHyper]->Name[0], HYPERPRESET_CHECKSUM_LENGTH) !=
															pMorpheusHyperPresets[idHyper]->CheckSum)
		{
			MyMessageBox (NULL, ResourceString (IDS_MIDIRX_CHECKSUM_ERROR), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			GlobalFreePtr ((LPVOID)pMorpheusHyperPresets[idHyper]);
			pMorpheusHyperPresets[idHyper] = (PHYPERPRESET_PARAMS)NULL;
			return (FALSE);
		}

		// OK
		return (TRUE);
	}

	// Error
	MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));
	return (FALSE);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusGetMM
 Parameters:	int	idMM				Midimap number to read.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Reads the specified Morpheus midimap into an allocated
				buffer. The pointer to the midimap buffer is stored in
				pMorpheusMidimaps[idMM].
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusGetMM (int idMM)
{
	HPBYTE	hpRxData;
	DWORD	dwRxLength;
	char	szStatusBuf[100];

	// Display the MIDI status
	wsprintf (&szStatusBuf[0], ResourceString (IDS_MIDIRX_READING_MM), idMM);
	MidiStatus (&szStatusBuf[0]);

	// If there is already a midimap buffer
	if (pMorpheusMidimaps[idMM] != NULL)
	{
		// Free it
		GlobalFreePtr ((LPVOID)pMorpheusMidimaps[idMM]);
	}

	// Get the data (hopefully)
	if (GetMorpheusData (hwndMain, MIDIMAP_REQUEST, NO_PARAM, idMM, &hpRxData, &dwRxLength))
	{
		// Store the buffer
		pMorpheusMidimaps[idMM] = (PMIDIMAP_PARAMS)hpRxData;
		MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));

		// Check the checksum is OK
		if (MidiChecksum (&pMorpheusMidimaps[idMM]->Name[0], MIDIMAP_CHECKSUM_LENGTH) != pMorpheusMidimaps[idMM]->CheckSum)
		{
			MyMessageBox (NULL, ResourceString (IDS_MIDIRX_CHECKSUM_ERROR), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			GlobalFreePtr ((LPVOID)pMorpheusMidimaps[idMM]);
			pMorpheusMidimaps[idMM] = (PMIDIMAP_PARAMS)NULL;
			return (FALSE);
		}

		// OK
		return (TRUE);
	}

	// Error
	MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));
	return (FALSE);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusGetInstrumentList
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Reads the Morpheus instrument list.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 24/4/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusGetInstrumentList (VOID)
{
	HPBYTE	hpRxData;
	DWORD	dwRxLength;

	MidiStatus (ResourceString (IDS_MIDIRX_READING_INSLIST));

	// Get the data (hopefully)
	if (!GetMorpheusData (hwndMain, INSTRUMENT_LIST_REQUEST, NO_PARAM, NO_PARAM, &hpRxData, &dwRxLength))
	{
		// Error
        return (FALSE);
	}

	// Store the allocated buffer pointer
	pMorpheusInstrumentList = (PINSTRUMENT_LIST_PARAMS)hpRxData;
	nMorpheusInstrumentListLength = (WORD)dwRxLength;
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusGetFilterList
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Reads the Morpheus instrument list.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 24/4/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusGetFilterList (VOID)
{
	HPBYTE	hpRxData;
	DWORD	dwRxLength;

	MidiStatus (ResourceString (IDS_MIDIRX_READING_FILTLIST));

	// Get the data (hopefully)
	if (!GetMorpheusData (hwndMain, FILTER_LIST_REQUEST, NO_PARAM, NO_PARAM, &hpRxData, &dwRxLength))
	{
		// Error
		return (FALSE);
	}

	// Store the allocated buffer pointer
	pMorpheusFilterList = (PFILTER_LIST_PARAMS)hpRxData;
	nMorpheusFilterListLength = (WORD)dwRxLength;
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusGetEffectList
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Reads the Morpheus effects list.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 24/4/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusGetEffectList (VOID)
{
	HPBYTE	hpRxData;
	DWORD	dwRxLength;

	MidiStatus (ResourceString (IDS_MIDIRX_READING_EFFLIST));

	// Get the data (hopefully)
	if (!GetMorpheusData (hwndMain, EFFECT_LIST_REQUEST, NO_PARAM, NO_PARAM, &hpRxData, &dwRxLength))
	{
		// Error
        return (FALSE);
	}

	// Store the allocated buffer pointer
	pMorpheusEffectList = (LPVOID)hpRxData;
	nMorpheusEffectListLength = (WORD)dwRxLength;

	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusGetPresetsList
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Reads the list of Morpheus presets.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 25/4/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusGetPresetsList (VOID)
{
	HPBYTE	hpRxData;
	DWORD	dwRxLength;

	// Update MIDI status
	MidiStatus (ResourceString (IDS_MIDIRX_READING_PRLIST));

	// Get the data (hopefully)
	if (!GetMorpheusData (hwndMain, PRESET_LIST_REQUEST, NO_PARAM, NO_PARAM, &hpRxData, &dwRxLength))
	{
		// Error
		return (FALSE);
	}

	// Store the allocated buffer pointer
	pMorpheusPresetList = (PPRESET_LIST_PARAMS)hpRxData;

	// Display the presets list (if the PRESETS are being displayed)
	DisplayPresetsList ();

	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusGetHyperpresetsList
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Reads the list of Morpheus hyperpresets.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 25/4/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusGetHyperpresetsList (VOID)
{
	HPBYTE	hpRxData;
	DWORD	dwRxLength;

	// Update MIDI status
	MidiStatus (ResourceString (IDS_MIDIRX_READING_HPLIST));

	// Get the data (hopefully)
	if (!GetMorpheusData (hwndMain, HYPERPRESET_LIST_REQUEST, NO_PARAM, NO_PARAM, &hpRxData, &dwRxLength))
	{
		// Error
		return (FALSE);
	}

	// Store the allocated buffer pointer
	pMorpheusHyperpresetList = (PHYPERPRESET_LIST_PARAMS)hpRxData;

	// Display the hyperpresets list (if the HYPERPRESETS are being displayed)
	DisplayHypersList ();

	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusGetMidimapsList
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Reads the list of Morpheus midimaps.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 25/4/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusGetMidimapsList (VOID)
{
	HPBYTE	hpRxData;
	DWORD	dwRxLength;

	// Update MIDI status
	MidiStatus (ResourceString (IDS_MIDIRX_READING_MMLIST));

	// Get the data (hopefully)
	if (!GetMorpheusData (hwndMain, MIDIMAP_LIST_REQUEST, NO_PARAM, NO_PARAM, &hpRxData, &dwRxLength))
	{
		// Error
		return (FALSE);
	}

	// Store the allocated buffer pointer
	pMorpheusMidimapList = (PMIDIMAP_LIST_PARAMS)hpRxData;

	// Display the midimap list (if the MIDIMAPs are being displayed)
	DisplayMMsList ();

	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusGetTuning
 Parameters:	TUNING_TABLE_PARAMS	*pTable		Buffer to read table into.
 Returns:		BOOL							TRUE if OK, else FALSE.
 Description:	Reads the user tuning table.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 17/6/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusGetTuning (TUNING_TABLE_PARAMS *pTable)
{
	HPBYTE	hpRxData;
	DWORD	dwRxLength;

	// Update MIDI status
	MidiStatus (ResourceString (IDS_MIDIRX_READING_UT));

	// Get the data (hopefully)
	if (!GetMorpheusData (hwndMain, TUNE_TABLE_REQUEST, 0, NO_PARAM, &hpRxData, &dwRxLength))
	{
		// Error
		MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));
		return (FALSE);
	}

	// Update MIDI status
	MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));

	// Copy the table
	hmemcpy (pTable, hpRxData, sizeof (TUNING_TABLE_PARAMS));

	// Free the allocated RX buffer
	GlobalFreePtr (hpRxData);

	// Display the table if the user is editing it
	DisplayTuning ();

	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusGetAllProgramMaps
 Parameters:	None.
 Returns:		BOOL							TRUE if OK, else FALSE.
 Description:	Reads all the program maps.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/6/94 	| Created.											| PW
*****************************************************************************/

static BOOL MorpheusGetAllProgramMaps (VOID)
{
	int		i;

	// For each program map
	for (i=0 ; i<NUM_PROGRAM_MAPS ; i++)
	{
		// If error reading this map
		if (!MorpheusGetProgramMap (i))
		{
			// Error
            return (FALSE);
        }
	}

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusGetProgramMap
 Parameters:	int		nMap			Program map number.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Reads a program map.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/6/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusGetProgramMap (int nMap)
{
	HPBYTE	hpRxData;
	DWORD	dwRxLength;
	char	szStatus[100];

	// Update MIDI status
	wsprintf (&szStatus[0], ResourceString (IDS_MIDIRX_READING_PM), nMap);
	MidiStatus (&szStatus[0]);

	// Get the data (hopefully)
	if (!GetMorpheusData (hwndMain, PROG_MAP_REQUEST, nMap, NO_PARAM, &hpRxData, &dwRxLength))
	{
		// Error
		MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));
		return (FALSE);
	}

	// Update MIDI status
	MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));

	// Copy the table into the global program map array
	hmemcpy (&MorpheusProgramMaps[nMap], hpRxData, sizeof (PROGRAM_MAP_PARAMS));

	// Free the allocated RX buffer
	GlobalFreePtr (hpRxData);

	// If this is program map on view
	if (nSelectedProgramMap == nMap)
	{
		// Display the map if the user is editing it
		DisplayProgramMap ();
	}

	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusGetMasterSettings
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Reads the master settings.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 12/6/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusGetMasterSettings (VOID)
{
	HPBYTE	hpRxData;
	DWORD	dwRxLength;

	// If there's already data loaded
	if (pMasterSettings != NULL)
	{
		// Free it
		GlobalFreePtr (pMasterSettings);
    }

	// Update MIDI status
	MidiStatus (ResourceString (IDS_MIDIRX_READING_MASTER));

	// Get the data (hopefully)
	if (!GetMorpheusData (hwndMain, MASTER_DATA_REQUEST, NO_PARAM, NO_PARAM, &hpRxData, &dwRxLength))
	{
		// Error
		MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));
		return (FALSE);
	}

	// Update MIDI status
	MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));

	// Store the allocated buffer pointer
	pMasterSettings = (LPBYTE)hpRxData;
	nMasterSettingsLength = (UINT)dwRxLength;

	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		InstrumentNumberToIndex
 Parameters:	int	idInstrument		Instrument number.
 Returns:		int						Internal instrument index (0...)
 Description:	Searches the instrument list for the specified number.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/4/94 	| Created.											| PW
*****************************************************************************/

int InstrumentNumberToIndex (int idInstrument)
{
	int		i, num, idList;

	// Do nothing if no list has been loaded
	if (pMorpheusInstrumentList == NULL)
	{
		return (0);
	}

	// How many instruments ?
	num = ReadSysexWord (&pMorpheusInstrumentList->wNumInstruments);

	// For each of those instruments
	for (i=0 ; i<num ; i++)
	{
		// What's the number of this one ?
		idList = ReadSysexWord (&pMorpheusInstrumentList->Info[i].wInstrumentNumber);

		// If it matches
		if (idList == idInstrument)
		{
			// Stop looking
            break;
		}
	}

	// Return the index (or max+1 if not found)
	return (i);
}

/*^L*/
/*****************************************************************************
 Function:		IndexToInstrumentNumber
 Parameters:	int	nIndex				Instrument index.
 Returns:		int						Morpheus instrument number.
 Description:	Returns the Morpheus instrument number corresponding to
				an internal instrument index.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 28/4/94 	| Created.											| PW
*****************************************************************************/

int IndexToInstrumentNumber (int nIndex)
{
	// Do nothing if no list has been loaded
	if (pMorpheusInstrumentList == NULL)
	{
		return (0);
	}

	// Just index into the loaded instrument list
	return (ReadSysexWord (&pMorpheusInstrumentList->Info[nIndex].wInstrumentNumber));
}

/*^L*/
/*****************************************************************************
 Function:		FilterNumberToIndex
 Parameters:	int	idFilter			Filter number.
 Returns:		int						Internal filter index (0...)
 Description:	Searches the filter list for the specified number.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 28/4/94 	| Created.											| PW
*****************************************************************************/

int FilterNumberToIndex (int idFilter)
{
	int		i, num, idList;

	// Do nothing if no list has been loaded
	if (pMorpheusFilterList == NULL)
	{
		return (0);
	}

	// How many filters ?
	num = ReadSysexWord (&pMorpheusFilterList->wNumFilters);

	// For each of those filters
	for (i=0 ; i<num ; i++)
	{
		// What's the number of this one ?
		idList = ReadSysexWord (&pMorpheusFilterList->Info[i].wFilterNumber);

		// If it matches
		if (idList == idFilter)
		{
			// Stop looking
			break;
		}
	}

	// Return the index (or max+1 if not found)
	return (i);
}

/*^L*/
/*****************************************************************************
 Function:		IndexToFilterNumber
 Parameters:	int	nIndex				Filter index.
 Returns:		int						Morpheus filter ID.
 Description:	Returns the Morpheus filter ID corresponding to an internal
				filter index.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 28/4/94 	| Created.											| PW
*****************************************************************************/

int IndexToFilterNumber (int nIndex)
{
	// Do nothing if no list has been loaded
	if (pMorpheusFilterList == NULL)
	{
		return (0);
	}

	// Just index into the loaded instrument list
	return (ReadSysexWord (&pMorpheusFilterList->Info[nIndex].wFilterNumber));
}

/*^L*/
/*****************************************************************************
 Function:		EffectNumberToIndex
 Parameters:	int	idFx				Which effect channel (FXA or FXB).
				int	idEffect			Effect number.
 Returns:		int						Internal effect index (0...)
 Description:	Searches the effect list (A or B) for the specified number.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94 	| Created.											| PW
*****************************************************************************/

int EffectNumberToIndex (int idFx, int idEffect)
{
	WORD				NumAEffects, NumBEffects, NumParams;
	EFFECT_AB_HDR		*pAB;
	EFFECT_DATA_HDR		*pED;
	EFFECT_PARAM		*pEP;
	WORD				e, p;
	LPBYTE				pb = ((LPBYTE)pMorpheusEffectList) + sizeof (EFFECT_LIST_HDR);

	/*
		A effects
	 */
	pAB = (EFFECT_AB_HDR *)pb;
	pb += sizeof (EFFECT_AB_HDR);
	NumAEffects = WNUM (pAB->wNumEffects);

	// For each A effect
	for (e=0 ; e<NumAEffects ; e++)
	{
		// Create a pointer the effect's data
		pED = (EFFECT_DATA_HDR *)pb;
		pb += sizeof (EFFECT_DATA_HDR);

		// If we are looking for this effect channel and this effect ID
		if ((idFx == FXA) && (idEffect == WNUM (pED->wID)))
		{
			// Then return the effect index
			return (e);
		}

		// Extract the number of parameters for this effect
		NumParams = WNUM (pED->wNumParams);

		// For each paramater
		for (p=0 ; p<NumParams ; p++)
		{
			// Create a pointer to the parameters then step past the parameter
			pEP = (EFFECT_PARAM *)pb;
			pb += sizeof (EFFECT_PARAM);
		}
	}

	// It we were looking for that channel but didn't find the effect ID
	if (idFx == FXA)
	{
		// Not found
		return (INDEX_NONE);
	}

	/*
		B effects
     */
	pAB = (EFFECT_AB_HDR *)pb;
	pb += sizeof (EFFECT_AB_HDR);
	NumBEffects = WNUM (pAB->wNumEffects);

	// For each B effect
	for (e=0 ; e<NumBEffects ; e++)
	{
		// Create a pointer the effect's data
		pED = (EFFECT_DATA_HDR *)pb;
		pb += sizeof (EFFECT_DATA_HDR);

		// If we are looking for this effect channel and this effect ID
		if ((idFx == FXB) && (idEffect == WNUM (pED->wID)))
		{
			// Then return the effect index
			return (e);
		}

		// Extract the number of parameters for this effect
		NumParams = WNUM (pED->wNumParams);

		// For each paramater
		for (p=0 ; p<NumParams ; p++)
		{
			// Create a pointer to the parameters then step past the parameter
			pEP = (EFFECT_PARAM *)pb;
			pb += sizeof (EFFECT_PARAM);
		}
	}

	// Not found
	return (INDEX_NONE);
}

/*^L*/
/*****************************************************************************
 Function:		IndexToEffectNumber
 Parameters:	int	idFx				Which effect channel (FXA or FXB).
				int	idIndex				Effect index.
 Returns:		int						Internal effect index (0...)
 Description:	Returns the effect number for a specified list index.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94 	| Created.											| PW
*****************************************************************************/

int IndexToEffectNumber (int idFx, int idIndex)
{
	WORD				NumAEffects, NumBEffects, NumParams;
	EFFECT_AB_HDR		*pAB;
	EFFECT_DATA_HDR		*pED;
	EFFECT_PARAM		*pEP;
	WORD				e, p;
	LPBYTE				pb = ((LPBYTE)pMorpheusEffectList) + sizeof (EFFECT_LIST_HDR);

	/*
		A effects
	 */
	pAB = (EFFECT_AB_HDR *)pb;
	pb += sizeof (EFFECT_AB_HDR);
	NumAEffects = WNUM (pAB->wNumEffects);

	// For each A effect
	for (e=0 ; e<NumAEffects ; e++)
	{
		// Create a pointer the effect's data
		pED = (EFFECT_DATA_HDR *)pb;
		pb += sizeof (EFFECT_DATA_HDR);

		// If we are looking for this effect channel and this effect index
		if ((idFx == FXA) && (idIndex == e))
		{
			// Then return the effect number
			return (WNUM (pED->wID));
		}

		// Extract the number of parameters for this effect
		NumParams = WNUM (pED->wNumParams);

		// For each paramater
		for (p=0 ; p<NumParams ; p++)
		{
			// Create a pointer to the parameters then step past the parameter
			pEP = (EFFECT_PARAM *)pb;
			pb += sizeof (EFFECT_PARAM);
		}
	}

	// It we were looking for that channel but didn't find the effect index
	if (idFx == FXA)
	{
		// Not found
		return (INDEX_NONE);
	}

	/*
		B effects
     */
	pAB = (EFFECT_AB_HDR *)pb;
	pb += sizeof (EFFECT_AB_HDR);
	NumBEffects = WNUM (pAB->wNumEffects);

	// For each B effect
	for (e=0 ; e<NumBEffects ; e++)
	{
		// Create a pointer the effect's data
		pED = (EFFECT_DATA_HDR *)pb;
		pb += sizeof (EFFECT_DATA_HDR);

		// If we are looking for this effect channel and this effect index
		if ((idFx == FXB) && (idIndex == e))
		{
			// Then return the effect number
			return (WNUM (pED->wID));
		}

		// Extract the number of parameters for this effect
		NumParams = WNUM (pED->wNumParams);

		// For each paramater
		for (p=0 ; p<NumParams ; p++)
		{
			// Create a pointer to the parameters then step past the parameter
			pEP = (EFFECT_PARAM *)pb;
			pb += sizeof (EFFECT_PARAM);
		}
	}

	// Not found
	return (INDEX_NONE);
}

/*^L*/
/*****************************************************************************
 Function:		GetEffectParamInfo
 Parameters:	int	idFx				Which effect channel (FXA or FXB).
				int	idIndex				Effect index.
				int	*pNumParams			(Output) number of parameters.
 Returns:		EFFECT_PARAM_INFO	*	Array of parameter info.
 Description:	Returns all the parameter info for a specified effect.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94 	| Created.											| PW
*****************************************************************************/

EFFECT_PARAM_INFO *GetEffectParamInfo (int idFx, int idIndex, int *pNumParams)
{
	WORD				NumAEffects, NumBEffects, NumParams;
	EFFECT_AB_HDR		*pAB;
	EFFECT_DATA_HDR		*pED;
	EFFECT_PARAM		*pEP;
	WORD				e, p;
	LPBYTE				pb = ((LPBYTE)pMorpheusEffectList) + sizeof (EFFECT_LIST_HDR);
	char				szName[20];

	/*
		A effects
	 */
	pAB = (EFFECT_AB_HDR *)pb;
	pb += sizeof (EFFECT_AB_HDR);
	NumAEffects = WNUM (pAB->wNumEffects);

	// For each A effect
	for (e=0 ; e<NumAEffects ; e++)
	{
		// Create a pointer the effect's data
		pED = (EFFECT_DATA_HDR *)pb;
		pb += sizeof (EFFECT_DATA_HDR);

		// Copy and zero-terminate the effect name
		_fmemcpy (&szName[0], &pED->szName[0], 12);
		szName[12] = '\0';

		// Extract the number of parameters for this effect
		NumParams = WNUM (pED->wNumParams);

		// For each paramater
		for (p=0 ; p<NumParams ; p++)
		{
			// Create a pointer to the parameters then step past the parameter
			pEP = (EFFECT_PARAM *)pb;
			pb += sizeof (EFFECT_PARAM);

			// Copy and zero-terminate the parameter name
			_fmemcpy (&EffectParamInfo[p].lpszName[0], &pEP->szName[0], 11);
			EffectParamInfo[p].lpszName[11] = '\0';

			// Copy the max, min, and default values
			EffectParamInfo[p].nMin	= WNUM (pEP->wMinValue);
			EffectParamInfo[p].nMax	= WNUM (pEP->wMaxValue);
			EffectParamInfo[p].nDef	= WNUM (pEP->wDefValue);

#ifdef PARAM_BUG
			/// Handle MORPHEUS BUG

			// If MAX < MIN 
			if (EffectParamInfo[p].nMax < EffectParamInfo[p].nMin)
			{
				// Treat as signed bytes
				EffectParamInfo[p].nMin = (int)(signed char)EffectParamInfo[p].nMin;
				EffectParamInfo[p].nMax = (int)(signed char)EffectParamInfo[p].nMax;
				EffectParamInfo[p].nDef = (int)(signed char)EffectParamInfo[p].nDef;
			}
#endif
		}

		// If we are looking for this effect channel and this effect index
		if ((idFx == FXA) && (idIndex == e))
		{
			// Then EffectParamInfo[] contains the parameter information
			*pNumParams = NumParams;		// Return the number of parameters
			return (&EffectParamInfo[0]);	// And their info
        }
	}

	/*
		B effects
	 */
	pAB = (EFFECT_AB_HDR *)pb;
	pb += sizeof (EFFECT_AB_HDR);
	NumBEffects = WNUM (pAB->wNumEffects);

	// For each B effect
	for (e=0 ; e<NumBEffects ; e++)
	{
		// Create a pointer the effect's data
		pED = (EFFECT_DATA_HDR *)pb;
		pb += sizeof (EFFECT_DATA_HDR);

		// Copy and zero-terminate the effect name
		_fmemcpy (&szName[0], &pED->szName[0], 12);
		szName[12] = '\0';

		// Extract the number of parameters for this effect
		NumParams = WNUM (pED->wNumParams);

		// For each paramater
		for (p=0 ; p<NumParams ; p++)
		{
			// Create a pointer to the parameters then step past the parameter
			pEP = (EFFECT_PARAM *)pb;
			pb += sizeof (EFFECT_PARAM);

			// Copy and zero-terminate the parameter name
			_fmemcpy (&EffectParamInfo[p].lpszName[0], &pEP->szName[0], 11);
			EffectParamInfo[p].lpszName[11] = '\0';

			// Copy the max, min, and default values
			EffectParamInfo[p].nMin	= WNUM (pEP->wMinValue);
			EffectParamInfo[p].nMax	= WNUM (pEP->wMaxValue);
			EffectParamInfo[p].nDef	= WNUM (pEP->wDefValue);

#ifdef PARAM_BUG
			/// Handle MORPHEUS BUG

			// If MAX < MIN 
			if (EffectParamInfo[p].nMax < EffectParamInfo[p].nMin)
			{
				// Treat as signed bytes
				EffectParamInfo[p].nMin = (int)(signed char)EffectParamInfo[p].nMin;
				EffectParamInfo[p].nMax = (int)(signed char)EffectParamInfo[p].nMax;
				EffectParamInfo[p].nDef = (int)(signed char)EffectParamInfo[p].nDef;
			}
#endif
		}

		// If we are looking for this effect channel and this effect index
		if ((idFx == FXB) && (idIndex == e))
		{
			// Then EffectParamInfo[] contains the parameter information
			*pNumParams = NumParams;		// Return the number of parameters
			return (&EffectParamInfo[0]);	// And their info
        }
	}

	// Not found
	return (NULL);
}

/*^L*/
/*****************************************************************************
 Function:		NumEffects
 Parameters:	int	idFx				Effect channel (FXA or FXB).	
 Returns:		int						Number of effects.
 Description:	Gets the number of effects for a particular channel/ 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94 	| Created.											| PW
*****************************************************************************/

int NumEffects (int idFx)
{
	WORD				NumAEffects, NumBEffects, NumParams;
	EFFECT_AB_HDR		*pAB;
	EFFECT_DATA_HDR		*pED;
	EFFECT_PARAM		*pEP;
	WORD				e, p;
	LPBYTE				pb = ((LPBYTE)pMorpheusEffectList) + sizeof (EFFECT_LIST_HDR);

	/*
		A effects
	 */
	pAB = (EFFECT_AB_HDR *)pb;
	pb += sizeof (EFFECT_AB_HDR);
	NumAEffects = WNUM (pAB->wNumEffects);

	// If this is the effects channel we are interested in
	if (idFx == FXA)
	{
		// Then return the number of effects
		return (NumAEffects);
	}

	// For each A effect
	for (e=0 ; e<NumAEffects ; e++)
	{
		// Create a pointer the effect's data
		pED = (EFFECT_DATA_HDR *)pb;
		pb += sizeof (EFFECT_DATA_HDR);

		// Extract the number of parameters for this effect
		NumParams = WNUM (pED->wNumParams);

		// For each paramater
		for (p=0 ; p<NumParams ; p++)
		{
			// Create a pointer to the parameters then step past the parameter
			pEP = (EFFECT_PARAM *)pb;
			pb += sizeof (EFFECT_PARAM);
		}
	}

	/*
		B effects
     */
	pAB = (EFFECT_AB_HDR *)pb;
	pb += sizeof (EFFECT_AB_HDR);
	NumBEffects = WNUM (pAB->wNumEffects);

	// Then return the number of effects
	return (NumBEffects);
}

/*^L*/
/*****************************************************************************
 Function:		EffectName
 Parameters:	int	idFx				Effect channel (FXA or FXB).
				int	idIndex				Effect index (0...)	
 Returns:		LPCSTR					Effect name.
 Description:	Gets the specified effect name. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94 	| Created.											| PW
*****************************************************************************/

LPCSTR EffectsName (int idFx, int idIndex)
{
	WORD				NumAEffects, NumBEffects, NumParams;
	EFFECT_AB_HDR		*pAB;
	EFFECT_DATA_HDR		*pED;
	EFFECT_PARAM		*pEP;
	WORD				e, p;
	LPBYTE				pb = ((LPBYTE)pMorpheusEffectList) + sizeof (EFFECT_LIST_HDR);
	static char			szName[20];

	/*
		A effects
	 */
	pAB = (EFFECT_AB_HDR *)pb;
	pb += sizeof (EFFECT_AB_HDR);
	NumAEffects = WNUM (pAB->wNumEffects);

	// For each A effect
	for (e=0 ; e<NumAEffects ; e++)
	{
		// Create a pointer the effect's data
		pED = (EFFECT_DATA_HDR *)pb;
		pb += sizeof (EFFECT_DATA_HDR);

		// If this is the channel and effect index we are interested in
		if ((idFx == FXA) && (idIndex == e))
		{
			// Copy and zero-terminate the effect name and return it
			_fmemcpy (&szName[0], &pED->szName[0], 12);
			szName[12] = '\0';
            return ((LPCSTR)&szName[0]);
		}

		// Extract the number of parameters for this effect
		NumParams = WNUM (pED->wNumParams);

		// For each paramater
		for (p=0 ; p<NumParams ; p++)
		{
			// Create a pointer to the parameters then step past the parameter
			pEP = (EFFECT_PARAM *)pb;
			pb += sizeof (EFFECT_PARAM);
		}
	}

	// It we were looking for that channel but didn't find the effect ID
	if (idFx == FXA)
	{
		// Not found
		return (NULL);
	}

	/*
		B effects
     */
	pAB = (EFFECT_AB_HDR *)pb;
	pb += sizeof (EFFECT_AB_HDR);
	NumBEffects = WNUM (pAB->wNumEffects);

	// For each B effect
	for (e=0 ; e<NumBEffects ; e++)
	{
		// Create a pointer the effect's data
		pED = (EFFECT_DATA_HDR *)pb;
		pb += sizeof (EFFECT_DATA_HDR);

		// If this is the channel and effect index we are interested in
		if ((idFx == FXB) && (idIndex == e))
		{
			// Copy and zero-terminate the effect name and return it
			_fmemcpy (&szName[0], &pED->szName[0], 12);
			szName[12] = '\0';
            return ((LPCSTR)&szName[0]);
		}

		// Extract the number of parameters for this effect
		NumParams = WNUM (pED->wNumParams);

		// For each paramater
		for (p=0 ; p<NumParams ; p++)
		{
			// Create a pointer to the parameters then step past the parameter
			pEP = (EFFECT_PARAM *)pb;
			pb += sizeof (EFFECT_PARAM);
		}
	}

	// Not found
	return (NULL);
}

/*^L*/
/*****************************************************************************
 Function:		ProgramName
 Parameters:	int		idProgram		Program number.
 Returns:		LPCSTR					Program name.
 Description:	Returns the specified program name. Note that PROGRAM may
				mean PRESET or HYPERPRESET; this function will work out
				which it is from the program number (see p.249 or user guide).
				The program name is returned in the form :-

				"BANK/NUMBER: Name"

				for example

				"2/100 Hyp:RealPno"

 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 7/6/94		| Created.											| PW
*****************************************************************************/

LPCSTR ProgramName (int idProgram)
{
	static char	szName[100];
	int			nBank, nOffset;

	// Handle BANK 0 (presets)
	if (idProgram < 128)
	{
		// Format the name
		nBank = 0;
		nOffset = 0;
		wsprintf (&szName[0], "%d/%d: %s", nBank, idProgram - nOffset,
					pMorpheusPresetList == NULL ? "" : (LPSTR)&pMorpheusPresetList->Info[idProgram].szName[0]);
	}
	// Handle BANK 1 (presets)
	else if (idProgram < 256)
	{
		// Format the name
		nBank = 1;
		nOffset = 128;
		wsprintf (&szName[0], "%d/%d: %s", nBank, idProgram - nOffset,
					pMorpheusPresetList == NULL ? "" : (LPSTR)&pMorpheusPresetList->Info[idProgram].szName[0]);
	}
	// Handle BANK 2 (hyperpresets)
	else if (idProgram < 384)
	{
		// Format the name
		nBank = 2;
		nOffset = 256;
		wsprintf (&szName[0], "%d/%d: %s", nBank, idProgram - nOffset,
					(LPSTR)&pMorpheusHyperpresetList->Info[idProgram - 256].szName[0]);
	}
	// Handle BANK 3 (presets)
	else if (idProgram < 512)
	{
		nBank = 3;
		nOffset = 384;

		// If this bank doesn't exist
		if ((idProgram - 128) > ((pMorpheusPresetList == NULL) ? 256 : WNUM (pMorpheusPresetList->wNumPresets)))
		{
			wsprintf (&szName[0], "%d/%d: %s", nBank, idProgram - nOffset, ResourceString (IDS_NO_DATA_CARD));
		}
		else
		{
			// Format the name
			wsprintf (&szName[0], "%d/%d: %s", nBank, idProgram - nOffset,
						pMorpheusPresetList == NULL ? "" : (LPSTR)&pMorpheusPresetList->Info[idProgram - 128].szName[0]);
		}
	}
	// Handle BANK 4 (hyperpresets)
	else if (idProgram < 640)
	{
		nBank = 4;
		nOffset = 512;

		// If this bank doesn't exist
		if ((idProgram - 384) > ((pMorpheusHyperpresetList == NULL) ? 128 : WNUM (pMorpheusHyperpresetList->wNumHyperpresets)))
		{
			wsprintf (&szName[0], "%d/%d: %s", nBank, idProgram - nOffset, ResourceString (IDS_NO_DATA_CARD));
		}
		else
        {
			// Format the name
			wsprintf (&szName[0], "%d/%d: %s", nBank, idProgram - nOffset,
				pMorpheusHyperpresetList == NULL ? "" : (LPSTR)&pMorpheusHyperpresetList->Info[idProgram - 384].szName[0]);
		}
	}

	// Return the name
	return ((LPCSTR)&szName[0]);
}

