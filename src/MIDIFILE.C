/*****************************************************************************
 File:			midifile.c
 Description:	Functions for writing MIDI files. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 1/7/94		| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

// Standard MIDI file header
static BYTE	bSMFHeader[14] =
{
	'M', 'T', 'h', 'd',			// Standard file signature
	0, 0, 0, 6,					// Header data length = 6
	0, 0,						// File format 0 (single track)
	0, 1,						// 1 track
	0, 96,						// 96 ppqn
};

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

static BOOL WriteSysexFile (HFILE hFile, LPCSTR lpszTitle, LPBYTE lpData, UINT nLength);
static UINT VariableLength (DWORD dwValue, PBYTE pBuf);

/*^L*/
/*****************************************************************************
 Function:		ExportPreset
 Parameters:	int		idPreset		Preset number.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Exports a single Morpheus preset as a MIDI file.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 1/7/94		| Created.											| PW
*****************************************************************************/

BOOL ExportPreset (int idPreset)
{
	HFILE		hFile;
	OFSTRUCT	os;
	char		szTitle[200];
	static char	szFileName[MAX_PATH];

	// If we aren't connected to the Morpheus
	if (!fConnectedToMorpheus)
	{
		// If it's not in memory
		if (pMorpheusPresets[idPreset] == NULL)
		{
			// Then we can't continue
			MyMessageBox (hwndMain, ResourceString (IDS_SAVE_MIDI_PR_NOT_CON),
				ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			return (FALSE);
		}
	}

	// Ask the user for a file name
	if (!MyGetSaveFileName (hwndMain,
							ResourceString (IDS_SAVE_MIDI_PR_FBOX_TITLE),
							MIDI_FILES_FILE_MASK,
//							ResourceString (IDS_SAVE_MIDI_PR_FBOX_MASK),
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

	// If it's not in memory
	if (pMorpheusPresets[idPreset] == NULL)
	{
		// Fetch it from the Morpheus
		if (!MorpheusGetPreset (idPreset))
		{
			// Error fetching preset from Morpheus
			MyMessageBox (hwndMain, ResourceString (IDS_SAVE_PR_ERROR_FETCHING), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			_lclose (hFile);
			remove (&szFileName[0]);
			return (FALSE);
		}
	}

	// Fill in the preset number
	WriteSysexWord (&pMorpheusPresets[idPreset]->wNumber, idPreset);

	// Fill in the preset checksum
	pMorpheusPresets[idPreset]->CheckSum =
		MidiChecksum (&pMorpheusPresets[idPreset]->Name[0], PRESET_CHECKSUM_LENGTH);

	// Create a suitable MIDI file title
	wsprintf (&szTitle[0], ResourceString (IDS_SAVE_MIDI_PR_FILE_MASK),
				idPreset, (LPCSTR)&pMorpheusPresetList->Info[idPreset].szName[0]);

	// Save it
	if (!WriteSysexFile (hFile, &szTitle[0], (LPBYTE)pMorpheusPresets[idPreset], sizeof (PRESET_PARAMS)))
	{
		// Error saving this preset
		MyMessageBox (hwndMain, ResourceString (IDS_SAVE_PR_ERROR_DISK), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		remove (&szFileName[0]);
		return (FALSE);
	}

	// Close the file
	_lclose (hFile);

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		ExportHyper
 Parameters:	int		idHyper		Hyperpreset number.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Exports a single Morpheus hyperpreset as a MIDI file.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 1/7/94		| Created.											| PW
*****************************************************************************/

BOOL ExportHyper (int idHyper)
{
	HFILE		hFile;
	OFSTRUCT	os;
	char		szTitle[200];
	static char	szFileName[MAX_PATH];

	// If we aren't connected to the Morpheus
	if (!fConnectedToMorpheus)
	{
		// If it's not in memory
		if (pMorpheusHyperPresets[idHyper] == NULL)
		{
			// Then we can't continue
			MyMessageBox (hwndMain, ResourceString (IDS_SAVE_MIDI_HP_NOT_CON),
				ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			return (FALSE);
		}
	}

	// Ask the user for a file name
	if (!MyGetSaveFileName (hwndMain,
							ResourceString (IDS_SAVE_MIDI_HP_FBOX_TITLE),
//							ResourceString (IDS_SAVE_MIDI_HP_FBOX_MASK),
							MIDI_FILES_FILE_MASK,
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

	// If it's not in memory
	if (pMorpheusHyperPresets[idHyper] == NULL)
	{
		// Fetch it from the Morpheus
		if (!MorpheusGetHyper (idHyper))
		{
			// Error fetching preset from Morpheus
			MyMessageBox (hwndMain, ResourceString (IDS_SAVE_HP_ERROR_FETCHING), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
			_lclose (hFile);
			remove (&szFileName[0]);
			return (FALSE);
		}
	}

	// Fill in the hyperpreset number
	WriteSysexWord (&pMorpheusHyperPresets[idHyper]->wNumber, idHyper);

	// Fill in the preset checksum
	pMorpheusHyperPresets[idHyper]->CheckSum =
		MidiChecksum (&pMorpheusHyperPresets[idHyper]->Name[0], HYPERPRESET_CHECKSUM_LENGTH);

	// Create a suitable MIDI file title
	wsprintf (&szTitle[0], ResourceString (IDS_SAVE_MIDI_HP_FILE_MASK),
				idHyper, (LPCSTR)&pMorpheusHyperpresetList->Info[idHyper].szName[0]);

	// Save it
	if (!WriteSysexFile (hFile, &szTitle[0], (LPBYTE)pMorpheusHyperPresets[idHyper], sizeof (HYPERPRESET_PARAMS)))
	{
		// Error saving this preset
		MyMessageBox (hwndMain, ResourceString (IDS_SAVE_HP_ERROR_DISK), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		_lclose (hFile);
		remove (&szFileName[0]);
		return (FALSE);
	}

	// Close the file
	_lclose (hFile);

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		WriteSysexFile
 Parameters:	HFILE	hFile			File to write to.
				LPCSTR	lpszTitle		Title to store in file.
				LPBYTE	lpData			Sysex data to write.
				UINT	nLength			Length of sysex message.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Writes a sysex message to the specified file.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 1/7/94		| Created.											| PW
*****************************************************************************/

static BOOL WriteSysexFile (HFILE hFile, LPCSTR lpszTitle, LPBYTE lpData, UINT nLength)
{
	static BYTE	bTrackHeader[8] = {'M', 'T', 'r', 'k', 0, 0, 0, 0};
	BYTE		bSysexLength[4];
	UINT		nSysexLengthLength;
	DWORD		dwTrackLength;
	DWORD		dwTrackPos;
	UINT		nTrackNameLength, nTrackNameLengthLength;
	BYTE		bTitleLength[4];

	// Write the MIDI file header
	if (_lwrite (hFile, &bSMFHeader[0], sizeof (bSMFHeader)) != sizeof (bSMFHeader))
	{
		return (FALSE);
	}

	// Write out the track header
	_lwrite (hFile, &bTrackHeader[0], sizeof (bTrackHeader));

	// Save the current file position
	dwTrackPos = _llseek (hFile, 0, SEEK_CUR);

	// Get the title length and turn it into variable length format
	nTrackNameLength = lstrlen (&lpszTitle[0]) + 1;
	nTrackNameLengthLength = VariableLength ((DWORD)nTrackNameLength, (PBYTE)&bTitleLength[0]);

	// Turn the sysex length into variable format
	nSysexLengthLength = VariableLength ((DWORD)nLength - 1, (PBYTE)&bSysexLength[0]);

	// Write the title (delta time + header + variable length + title)
	_lwrite (hFile, "\x00", 1);
	_lwrite (hFile, "\xFF\x03", 2);
	_lwrite (hFile, &bTitleLength[0], nTrackNameLengthLength);
	_lwrite (hFile, &lpszTitle[0], nTrackNameLength);

	// Write the SYSEX packet (delta time + 1st byte + variable length + SYSEX body)
	_lwrite (hFile, "\x00", 1);
	_lwrite (hFile, &lpData[0], 1);
	_lwrite (hFile, &bSysexLength[0], nSysexLengthLength);
	_lwrite (hFile, &lpData[1], nLength - 1);

	// Write the end-of-track-marker (delta time + marker)
	_lwrite (hFile, "\x7F", 1);
	_lwrite (hFile, "\xFF\x2F\x00", 3);

	// Calculate the track length (current file offset from saved start of track offset)
	dwTrackLength = _llseek (hFile, 0, SEEK_CUR) - dwTrackPos;

	// Create the track header
	bTrackHeader[4] = (BYTE)(dwTrackLength >> 24);
	bTrackHeader[5] = (BYTE)(dwTrackLength >> 16);
	bTrackHeader[6] = (BYTE)(dwTrackLength >>  8);
	bTrackHeader[7] = (BYTE)(dwTrackLength >>  0);

	// Seek back to the track header + 4 (where the track length is written)
	_llseek (hFile, dwTrackPos - 4, SEEK_SET);

	// Write the track length
	_lwrite (hFile, &bTrackHeader[4], 4);

#ifdef NEVER
	// Stick a control-Z on the end
	_llseek (hFile, 0, SEEK_END);
	_lwrite (hFile, "\x1A", 1);
#endif

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		VariableLength
 Parameters:	DWORD	dwValue			Input value.
				PBYTE	pBuf			Output byte array.
 Returns:		UINT					Length of output array.
 Description:	Converts a DWORD into variable length MIDI file format.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 1/7/94		| Created.											| PW
*****************************************************************************/

static UINT VariableLength (DWORD dwValue, PBYTE pBuf)
{
	DWORD	dw;
	int		i, nNumBytes;


	// Find the highest bit that's 1 in the value
	dw = dwValue;
	for (i=32 ; i>0 ; i--)
	{
		if (dw & 0x80000000L)	// If the top bit is set
			break;				// Found it
		dw <<= 1;				// Else shift left and keep trying
	}

	// How many 7 bit bytes are needed to hold the value then ?
	nNumBytes = (i + 6) / 7;

	// For each byte required
	dw = dwValue;
	for (i=0 ; i<nNumBytes ; i++)
	{
		// Write the current LS 7 bits
		pBuf[nNumBytes - i - 1] = ((BYTE)dw) | 0x80;

		// Shift the input value down 7 more bits
		dw >>= 7;
	}

	// Clear the 8th bit in the last byte
	pBuf[nNumBytes - 1] &= 0x7F;

	return (nNumBytes);
}
