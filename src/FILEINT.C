/*****************************************************************************
 File:			files.c
 Description:	Functions for loading and saving Morpheus INTERNALS files. 
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
 Function:		SaveInternals
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Saves the current (loaded into memory) internal information.
				Internal information is :-
				a)	Configuration (no. of presets etc.)
				b)	Instrument list.
				c)	Filter list.
                d)	Effects list.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 8/5/94		| Created.											| PW
*****************************************************************************/

BOOL SaveInternals (VOID)
{
	HFILE		hFile;
	OFSTRUCT	os;
	WORD		wSum = 0;

	// Create the file (fixed name)
	memset (&os, 0, sizeof (OFSTRUCT));
	os.cBytes = sizeof (OFSTRUCT);
	if ((hFile = OpenFile (INTERNALS_FILE_NAME, &os, OF_CREATE)) == HFILE_ERROR)
	{
		// Couldn't create the file
		return (FALSE);
	}

	// Write the file header
	if (_lwrite (hFile, MFILE_INTERNALS_HEADER, sizeof (MFILE_INTERNALS_HEADER)) != sizeof (MFILE_INTERNALS_HEADER))
	{
		// Error
		_lclose (hFile);
		remove (INTERNALS_FILE_NAME);
		return (FALSE);
	}
	// Add to the checksum
	wSum += CalcChecksum (MFILE_INTERNALS_HEADER, sizeof (MFILE_INTERNALS_HEADER));

	// Write the VERSION structure
	if (_lwrite (hFile, (LPVOID)&MorpheusVersionData, sizeof (VERSION_PARAMS)) != sizeof (VERSION_PARAMS))
	{
		// Error
		_lclose (hFile);
		remove (INTERNALS_FILE_NAME);
		return (FALSE);
	}
	// Add to the checksum
	wSum += CalcChecksum (&MorpheusVersionData, sizeof (VERSION_PARAMS));

	// Write the CONFIGURATION structure
	if (_lwrite (hFile, (LPVOID)&MorpheusConfigurationData, sizeof (CONFIGURATION_DATA_PARAMS)) != sizeof (CONFIGURATION_DATA_PARAMS))
	{
		// Error
		_lclose (hFile);
		remove (INTERNALS_FILE_NAME);
		return (FALSE);
	}
	// Add to the checksum
	wSum += CalcChecksum (&MorpheusConfigurationData, sizeof (CONFIGURATION_DATA_PARAMS));

	// Write the INSTRUMENT LIST length & structure
	if (!WriteLengthAndData (hFile, &nMorpheusInstrumentListLength, pMorpheusInstrumentList, &wSum))
	{
		// Error
		_lclose (hFile);
		remove (INTERNALS_FILE_NAME);
		return (FALSE);
	}

	// Write the FILTER LIST length & structure
	if (!WriteLengthAndData (hFile, &nMorpheusFilterListLength, pMorpheusFilterList, &wSum))
	{
		// Error
		_lclose (hFile);
		remove (INTERNALS_FILE_NAME);
		return (FALSE);
	}

	// Write the EFFECT LIST length & structure
	if (!WriteLengthAndData (hFile, &nMorpheusEffectListLength, pMorpheusEffectList, &wSum))
	{
		// Error
		_lclose (hFile);
		remove (INTERNALS_FILE_NAME);
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
 Function:		LoadInternals
 Parameters:	None.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Loads the Morpheus internal information from disk.
				Internal information is :-
				a)	Configuration (no. of presets etc.)
				b)	Instrument list.
				c)	Filter list.
				d)	Effects list.

				If this function fails, it could mean :-
				1)	Out of memory.
				2)	File not found.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 8/5/94		| Created.											| PW
*****************************************************************************/

BOOL LoadInternals (VOID)
{
	HFILE		hFile;
	OFSTRUCT	os;
	WORD		wSum = 0;
	char		szHeader[sizeof (MFILE_INTERNALS_HEADER)];

	// Open the file (fixed name)
	memset (&os, 0, sizeof (OFSTRUCT));
	os.cBytes = sizeof (OFSTRUCT);
	if ((hFile = OpenFile (INTERNALS_FILE_NAME, &os, OF_READ)) == HFILE_ERROR)
	{
		// Couldn't open the file
		return (FALSE);
	}

	// Read the file header
	if (_lread (hFile, &szHeader[0], sizeof (MFILE_INTERNALS_HEADER)) != sizeof (MFILE_INTERNALS_HEADER))
	{
		// Error
		_lclose (hFile);
		return (FALSE);
	}

	// Did the header say the right thing ?
	if (memcmp (&szHeader[0], MFILE_INTERNALS_HEADER, sizeof (MFILE_INTERNALS_HEADER)))
	{
		// Error
		_lclose (hFile);
		return (FALSE);
	}

	// Is the file checksum OK ?
	if (!FileChecksumOk (hFile))
	{
		// Error
		_lclose (hFile);
		return (FALSE);
	}

	// Read the VERSION structure
	if (_lread (hFile, &MorpheusVersionData, sizeof (VERSION_PARAMS)) != sizeof (VERSION_PARAMS))
	{
		// Error
		_lclose (hFile);
		return (FALSE);
	}

	// Read the CONFIGURATION structure
	if (_lread (hFile, &MorpheusConfigurationData, sizeof (CONFIGURATION_DATA_PARAMS)) !=
														sizeof (CONFIGURATION_DATA_PARAMS))
	{
		// Error
		_lclose (hFile);
		return (FALSE);
	}

	// Read the INSTRUMENT LIST length
	if ((pMorpheusInstrumentList = ReadLengthAndData (hFile, &nMorpheusInstrumentListLength, &wSum)) == NULL)
	{
		// Error
		_lclose (hFile);
		return (FALSE);
	}

	// Read the FILTER LIST length
	if ((pMorpheusFilterList = ReadLengthAndData (hFile, &nMorpheusFilterListLength, &wSum)) == NULL)
	{
		// Error
		_lclose (hFile);
		return (FALSE);
	}

	// Read the EFFECT LIST length
	if ((pMorpheusEffectList = ReadLengthAndData (hFile, &nMorpheusEffectListLength, &wSum)) == NULL)
	{
		// Error
		_lclose (hFile);
		return (FALSE);
	}

	// Close the file
	_lclose (hFile);

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		CalcChecksum
 Parameters:	LPVOID	lpData			Data to checksum.
				WORD	nLength			Length of data.
 Returns:		WORD					Bytewise sum of the data.
 Description:	Calculates the checksum of the data.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 8/5/94		| Created.											| PW
*****************************************************************************/

WORD CalcChecksum (LPVOID lpData, WORD nLength)
{
	UINT	i;
	WORD	wSum = 0;
	LPBYTE	lpbData = (LPBYTE)lpData;

	for (i=0 ; i<nLength ; i++)
	{
		wSum += lpbData[i];
	}
	return (wSum);
}

/*^L*/
/*****************************************************************************
 Function:		FileChecksumOk
 Parameters:	HFILE	hFile			The file.
 Returns:		BOOL					TRUE if file checksum if OK, else FALSE.
 Description:	Sees whether the file checksum is OK or not.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 8/5/94		| Created.											| PW
*****************************************************************************/

BOOL FileChecksumOk (HFILE hFile)
{
	BYTE	bBuf[128];
	LONG	lCurpos, lFileLength, lRemains;
	WORD	wSum = 0, wReadSum;
	UINT	nToRead;

	// Save the current file position
	lCurpos = _llseek (hFile, 0, SEEK_CUR);

	// How long's the file
	lFileLength = _llseek (hFile, 0, SEEK_END);

	// Go back the file start
	_llseek (hFile, 0, SEEK_SET);

	// While not at the end of the file (minus the potential checksum)
	lRemains = lFileLength - sizeof (WORD);
	while (lRemains > 0)
	{
		// How much to read
		nToRead = (lRemains > sizeof (bBuf)) ? sizeof (bBuf) : (UINT)lRemains; 

		// Read a block of data
		_lread (hFile, &bBuf[0], nToRead);

		// Add this buffer's checksum onto the cumulative total
		wSum += CalcChecksum (&bBuf[0], (WORD)nToRead);

		// Step the current offset on
		lRemains -= (LONG)nToRead;
	}

	// Read the file checksum (last word in the file)
	_lread (hFile, &wReadSum, sizeof (WORD));

	// Reset the file position to what it used to be
	_llseek (hFile, lCurpos, SEEK_SET);

	// And is it right ?
	return ((wReadSum == wSum) ? TRUE : FALSE);
}

/*^L*/
/*****************************************************************************
 Function:		WriteLengthAndData
 Parameters:	HFILE		hFile		File to write to.
				UINT FAR	*pLength	Address of data length.
				LPVOID		lpData		Data.
				LPWORD		lpCsum		Existing checksum to add to.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Writes a LENGTH UINT followed by some DATA, and adds the
				length & data checksum to the specified checksum.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 8/5/94		| Created.											| PW
*****************************************************************************/

BOOL WriteLengthAndData (HFILE hFile, WORD FAR *pLength, LPVOID lpData, LPWORD lpCsum)
{
	// Write the LENGTH
	if (_lwrite (hFile, (LPVOID)pLength, sizeof (WORD)) != sizeof (WORD))
	{
		// Error writing the length
		return (FALSE);
	}

	// Write the DATA
	if (_lwrite (hFile, lpData, *pLength) != *pLength)
	{
		// Error writing the data
		return (FALSE);
	}

	// Add to the checksum
	*lpCsum += CalcChecksum (pLength, sizeof (WORD));
	*lpCsum += CalcChecksum (lpData, *pLength);

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		ReadLengthAndData
 Parameters:	HFILE		hFile		File to read from.
				UINT FAR	*pLength	Address of data length.
				LPWORD		lpCsum		Existing checksum to add to.
 Returns:		LPVOID					Address of allocated data buffer or
										NULL if failed.
 Description:	Read a LENGTH UINT followed by some DATA, and adds the
				length & data checksum to the specified checksum.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 8/5/94		| Created.											| PW
*****************************************************************************/

LPVOID ReadLengthAndData (HFILE hFile, WORD FAR *pLength, LPWORD lpCsum)
{
	LPBYTE	lpBuf;

	// Read the LENGTH
	if (_lread (hFile, pLength, sizeof (WORD)) != sizeof (WORD))
	{
		// Error reading the length
		return (NULL);
	}

	// Allocate a buffer for the read data
	if ((lpBuf = GlobalAllocPtr (GMEM_FIXED, *pLength)) == NULL)
	{
		// Out of memory
		return (NULL);
	}

	// Read the DATA
	if (_lread (hFile, lpBuf, *pLength) != *pLength)
	{
		// Error reading the data
		GlobalFreePtr (lpBuf);
		return (NULL);
	}

	// Add to the checksum
	*lpCsum += CalcChecksum (pLength, sizeof (WORD));
	*lpCsum += CalcChecksum (lpBuf, *pLength);

	// OK
	return ((LPVOID)lpBuf);
}

