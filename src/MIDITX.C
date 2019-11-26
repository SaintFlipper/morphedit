/*****************************************************************************
 File:			miditx.c
 Description:	The high-level MIDI transmit module.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/5/94		| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/


/*^L*/
/*****************************************************************************
 Function:		MorpheusSelectPreset
 Parameters:	HWND	hwndParent		Parent window.
				int		nPreset			Internal preset number.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Selects the specified Morpheus preset number.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 7/5/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusSelectPreset (HWND hwndParent, int nPreset)
{
	int		nBankOffset;

	// If we are bank 0 or 1
	if (nPreset < 256)
	{
		// No bank offset
		nBankOffset = 0;
	}
	else
	{
		// We must be in BANK 3 => offset = 384
		nBankOffset = 384;

		// Make sure that the preset number is < 128
		nPreset %= 128;
	}

	// Use SET PRESET parameter change
	return (MorpheusSetParameter (hwndParent, MPARAM_CURRENT_PROGRAM, nBankOffset + nPreset));
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusSelectHyper
 Parameters:	HWND	hwndParent		Parent window.
				int		nHyper			Internal hyperpreset number.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Selects the specified Morpheus hyperpreset number.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 1/6/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusSelectHyper (HWND hwndParent, int nHyper)
{
	int		nBankOffset;

	// If we are BANK 2
	if (nHyper < 128)
	{
		// No bank offset
		nBankOffset = 256;
	}
	else
	{
		// We must be in BANK 4 => offset = 512
		nBankOffset = 512;

		// Make sure that the hyperpreset number is < 128
		nHyper %= 128;
	}

	// Use SET PRESET parameter change
	return (MorpheusSetParameter (hwndParent, MPARAM_CURRENT_PROGRAM, nBankOffset + nHyper));
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusSelectMM
 Parameters:	HWND	hwndParent		Parent window.
				int		nMM				Internal midimap number.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Selects the specified Morpheus midimap number.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusSelectMM (HWND hwndParent, int nMM)
{
	// Use SET PRESET parameter change
	return (MorpheusSetParameter (hwndParent, MPARAM_CURRENT_MIDIMAP, nMM));
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusSetParameter
 Parameters:	HWND	hwndParent		Parent window.
				int		nParam			Parameter number to set.
				int		nValue			Value to set it to. 	
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Sets the specified parameter to the specified value.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 7/5/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusSetParameter (HWND hwndParent, int nParam, int nValue)
{
	BYTE	bData[SYSEX_HEADER_LEN + 5];
	UINT	i = 0;

	// Fill in the SysEx data
	bData[i++]		= SYSEX_START;
	bData[i++]		= EMU_MFG_ID;
	bData[i++]		= MORPHEUS_PRODUCT_ID;
	bData[i++]		= bMidiDeviceID;
	bData[i++]		= PARAMETER_DATA;
	WriteSysexWord (&bData[i], nParam);
	i += 2;
	WriteSysexWord (&bData[i], nValue);
	i += 2;
	bData[i++]		= SYSEX_END;

	// Send the packet
	return (SendMorpheusData (hwndParent, &bData[0], i));
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusSendPreset
 Parameters:	HWND	hwndParent		Parent window.
				int		idPreset		Preset number to send.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Sends the specified preset back to the Morpheus.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/5/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusSendPreset (HWND hwndParent, int idPreset)
{
	char	szStatusBuf[100];
	BOOL	fOK;

	// If the preset is between 128 and 255
	if ((idPreset >=128) && (idPreset < 256))
	{
		// Then it's in ROM, so there's no point trying to send it
		return (TRUE);
	}

	// Display the MIDI status
	wsprintf (&szStatusBuf[0], ResourceString (IDS_MIDITX_SENDING_PR), idPreset);
	MidiStatus (&szStatusBuf[0]);

	// If the preset isn't loaded
	if (pMorpheusPresets[idPreset] == NULL)
	{
		// Bad things
		MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));
		return (FALSE);
	}

	// Fill in the preset number
	WriteSysexWord (&pMorpheusPresets[idPreset]->wNumber, idPreset);

	// Fill in the preset checksum
	pMorpheusPresets[idPreset]->CheckSum =
		MidiChecksum (&pMorpheusPresets[idPreset]->Name[0], PRESET_CHECKSUM_LENGTH);

	// Send the preset
	fOK = SendMorpheusData (hwndParent, (LPBYTE)pMorpheusPresets[idPreset], sizeof (PRESET_PARAMS));

	// Display the MIDI status
	MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));

	// Done
    return (fOK);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusSendHyper
 Parameters:	HWND	hwndParent		Parent window.
				int		idHyper			Hyperpreset number to send.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Sends the specified hyperpreset back to the Morpheus.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 1/6/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusSendHyper (HWND hwndParent, int idHyper)
{
	char	szStatusBuf[100];
	BOOL	fOK;

	// Display the MIDI status
	wsprintf (&szStatusBuf[0], ResourceString (IDS_MIDITX_SENDING_HP), idHyper);
	MidiStatus (&szStatusBuf[0]);

	// If the hyperpreset isn't loaded
	if (pMorpheusHyperPresets[idHyper] == NULL)
	{
		// Bad things
		MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));
		return (FALSE);
	}

	// Fill in the hyperpreset number
	WriteSysexWord (&pMorpheusHyperPresets[idHyper]->wNumber, idHyper);

	// Fill in the hyperpreset checksum
	pMorpheusHyperPresets[idHyper]->CheckSum =
		MidiChecksum (&pMorpheusHyperPresets[idHyper]->Name[0], HYPERPRESET_CHECKSUM_LENGTH);

	// Send the hyperpreset
	fOK = SendMorpheusData (hwndParent, (LPBYTE)pMorpheusHyperPresets[idHyper], sizeof (HYPERPRESET_PARAMS));

	// Display the MIDI status
	MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));

	// Done
    return (fOK);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusSendMM
 Parameters:	HWND	hwndParent		Parent window.
				int		idMM			Midimap number to send.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Sends the specified midimap back to the Morpheus.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/6/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusSendMM (HWND hwndParent, int idMM)
{
	char	szStatusBuf[100];
	BOOL	fOK;

	// Display the MIDI status
	wsprintf (&szStatusBuf[0], ResourceString (IDS_MIDITX_SENDING_MM), idMM);
	MidiStatus (&szStatusBuf[0]);

	// If the midimap isn't loaded
	if (pMorpheusMidimaps[idMM] == NULL)
	{
		// Bad things
		return (FALSE);
	}

	// Fill in the midimap number
	WriteSysexWord (&pMorpheusMidimaps[idMM]->wNumber, idMM);

	// Fill in the hyperpreset checksum
	pMorpheusMidimaps[idMM]->CheckSum =
		MidiChecksum (&pMorpheusMidimaps[idMM]->Name[0], MIDIMAP_CHECKSUM_LENGTH);

	// Send the hyperpreset
	fOK = SendMorpheusData (hwndParent, (LPBYTE)pMorpheusMidimaps[idMM], sizeof (MIDIMAP_PARAMS));

	// Display the MIDI status
	MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));

	// Done
	return (fOK);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusSendTuning
 Parameters:	HWND	hwndParent				Parent window.
				TUNING_TABLE_PARAMS	*pTable		Tuning table to send.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Sends the specified tuning table back to the Morpheus.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 17/6/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusSendTuning (HWND hwndParent, TUNING_TABLE_PARAMS *pTable)
{
	BOOL	fOK;

	// Display the MIDI status
	MidiStatus (ResourceString (IDS_MIDITX_SENDING_UT));

	// Make sure the header and tail of the table contains sysex data
	pTable->bHeader[0]	= SYSEX_START;
	pTable->bHeader[1]	= EMU_MFG_ID;
	pTable->bHeader[2]	= MORPHEUS_PRODUCT_ID;
	pTable->bHeader[3]	= bMidiDeviceID;
	pTable->bHeader[4]	= TUNE_TABLE_DATA;
	pTable->Number		= 0;
	pTable->EndOfSysEx	= SYSEX_END;

	// Send the midimap
	fOK = SendMorpheusData (hwndParent, (LPBYTE)pTable, sizeof (TUNING_TABLE_PARAMS));

	// Display the MIDI status
	MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));

	// Done
	return (fOK);
}

/*^L*/
/*****************************************************************************
 Function:		MorpheusSendProgramMap
 Parameters:	HWND	hwndParent		Parent window.
				int		nMap			Program map number.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Sends the specified program map back to the Morpheus.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/6/94 	| Created.											| PW
*****************************************************************************/

BOOL MorpheusSendProgramMap (HWND hwndParent, int nMap)
{
	BOOL	fOK;
	char	szStatus[100];

	// Display the MIDI status
	wsprintf (&szStatus[0], ResourceString (IDS_MIDITX_SENDING_PM), nMap);
	MidiStatus (&szStatus[0]);

	// Make sure the header and tail of the table contains sysex data
	MorpheusProgramMaps[nMap].bHeader[0]	= SYSEX_START;
	MorpheusProgramMaps[nMap].bHeader[1]	= EMU_MFG_ID;
	MorpheusProgramMaps[nMap].bHeader[2]	= MORPHEUS_PRODUCT_ID;
	MorpheusProgramMaps[nMap].bHeader[3]	= bMidiDeviceID;
	MorpheusProgramMaps[nMap].bHeader[4]	= PROG_MAP_DATA;
	MorpheusProgramMaps[nMap].Number		= nMap;
	MorpheusProgramMaps[nMap].EndOfSysEx	= SYSEX_END;

	// Send the midimap
	fOK = SendMorpheusData (hwndParent, (LPBYTE)&MorpheusProgramMaps[nMap], sizeof (PROGRAM_MAP_PARAMS));

	// Display the MIDI status
	MidiStatus (ResourceString (IDS_MIDI_STATUS_IDLE));

	// Done
	return (fOK);
}

/*^L*/
/*****************************************************************************
 Function:		SendChannelProgram
 Parameters:	HWND	hWnd			Parent window.
				int		nChan			MIDI channel.
                int		nProgram		Program number.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Selects a MIDI program.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 15/6/94 	| Created.											| PW
*****************************************************************************/

BOOL SendChannelProgram (HWND hWnd, int nChan, int nProgram)
{
	BYTE	bMidiData[7];

	// Fill in a bank / program change
	bMidiData[0] = 0xB0 | (BYTE)(nChan & 0x0F);
	bMidiData[1] = 0x00;
	bMidiData[2] = 0x00;
	bMidiData[3] = 0x20;
	bMidiData[4] = (BYTE)(nProgram / 128);
	bMidiData[5] = 0xC0 | (BYTE)(nChan & 0x0F);
	bMidiData[6] = (BYTE)(nProgram & 0x7F);

	// Send the data
	return (SendMorpheusData (hWnd, &bMidiData[0], 7));
}

/*^L*/
/*****************************************************************************
 Function:		SendChannelVolume
 Parameters:	HWND	hWnd			Parent window.
				int		nChan			MIDI channel.
                int		nVolume			Channel volume.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Sets a channel volume.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 15/6/94 	| Created.											| PW
*****************************************************************************/

BOOL SendChannelVolume (HWND hWnd, int nChan, int nVolume)
{
	HMIDIIN		hMidiInput;				// Input device handle
	HMIDIOUT	hMidiOutput;			// Output device handle
	BYTE		bMidi[4];

	// Find out what the MIDI device handles are
	morphdllGetHandles (&hMidiInput, &hMidiOutput);

	// Send a MIDI volume controller
	bMidi[0] = 0xB0 | (BYTE)(nChan & 0x0F);
	bMidi[1] = 0x07;
	bMidi[2] = (BYTE)(nVolume & 0x7F);
	bMidi[3] = 0;
	midiOutShortMsg (hMidiOutput, *(DWORD *)&bMidi[0]);

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		SendChannelPan
 Parameters:	HWND	hWnd			Parent window.
				int		nChan			MIDI channel.
                int		nPan			Channel pan.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Sets a channel volume.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 15/6/94 	| Created.											| PW
*****************************************************************************/

BOOL SendChannelPan (HWND hWnd, int nChan, int nPan)
{
	HMIDIIN		hMidiInput;				// Input device handle
	HMIDIOUT	hMidiOutput;			// Output device handle
	BYTE		bMidi[4];

	// Find out what the MIDI device handles are
	morphdllGetHandles (&hMidiInput, &hMidiOutput);

	// Send a MIDI pan controller
	bMidi[0] = 0xB0 | (BYTE)(nChan & 0x0F);
	bMidi[1] = 0x0A;
	bMidi[2] = (BYTE)(nPan & 0x7F);
	bMidi[3] = 0;
	midiOutShortMsg (hMidiOutput, *(DWORD *)&bMidi[0]);

	// OK
	return (TRUE);
}
