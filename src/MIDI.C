/*****************************************************************************
 File:			midi.c
 Description:	The main MIDI module.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 8/4/94		| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

/*^L*/
/*****************************************************************************
 Function:		MidiStatus
 Parameters:	LPSTR	lpString		Status string.
 Returns:		None.
 Description:	Displays the specified MIDI status.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 9/4/94 	| Created.											| PW
*****************************************************************************/

VOID MidiStatus (LPCSTR lpString)
{
	char	szStatusString[100];

	wsprintf (&szStatusString[0], ResourceString (IDS_MIDI_STATUS_MASK), lpString);
	DisplayStatusText (0, &szStatusString[0]);
}

/*^L*/
/*****************************************************************************
 Function:		DisplayMidiError
 Parameters:	WORD	wErrorCode		Error code.
 Returns:		None.
 Description:	Displays an MMSYSTEM/MIDI error message.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 16/3/94 	| Created.											| PW
*****************************************************************************/

VOID DisplayMidiError (WORD wErrorCode)
{
	char	szLocalString[200];
	LPCSTR	pszErrorString;

	switch (wErrorCode)
	{
		case MMSYSERR_BADDEVICEID:
		{
			pszErrorString = ResourceString (IDS_MIDI_ERR_BAD_DEVICE_ID);
			break;
		}
		case MMSYSERR_ALLOCATED:
		{
			pszErrorString = ResourceString (IDS_MIDI_ERR_ALREADY_USED);
			break;
		}
		case MMSYSERR_NOMEM:
		{
			pszErrorString = ResourceString (IDS_MIDI_ERR_OUT_OF_MEMORY);
			break;
		}
		case MIDIERR_NOMAP:
		{
			pszErrorString = ResourceString (IDS_MIDI_ERR_NO_MIDI_MAP);
			break;
		}
		case MIDIERR_NODEVICE:
		{
			pszErrorString = ResourceString (IDS_MIDI_ERR_NO_DEV_PORT);
			break;
		}
		default:
		{
			wsprintf (&szLocalString[0], ResourceString (IDS_MIDI_ERR_UNKNOWN_MASK), wErrorCode);
			pszErrorString = &szLocalString[0];
			break;
		}
	}

	MyMessageBox (NULL, pszErrorString, ResourceString (IDS_MIDI_ERR_MBOX_TITLE), MB_ICONEXCLAMATION | MB_OK);
}

/*^L*/
/*****************************************************************************
 Function:		MidiChecksum
 Parameters:	LPVOID	lpData			Start of data to be checksummed.
				UINT	nLength			Length of data to checksum.
 Returns:		BYTE					MIDI checksum.
 Description:	Calculates the MIDI checksum for the specified data.
				The Morpheus MIDI checksum seems to be the bytewise sum
				of the data (ie. excluding SysEx header) without the top bit.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/5/94 	| Created.											| PW
*****************************************************************************/

BYTE MidiChecksum (LPVOID lpData, UINT nLength)
{
	LPBYTE	lpbData = (LPBYTE)lpData;
	BYTE	bsum = 0;
	UINT	i;

	for (i=0 ; i<nLength ; i++)
	{
		bsum += *lpbData++;
	}
	return (bsum & 0x7F);
}

