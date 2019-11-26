/*****************************************************************************
 File:			sysex.c
 Description:	The SYSEX send / receive module.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 18/3/94	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define	DIALOG_OK						0
#define	DIALOG_ERROR					1
#define	DIALOG_CANCELLED				2
#define	DIALOG_TIMEOUT					3

#define	SYSEX_RESPONSE_TIMEOUT_ID		1
#define	SYSEX_RESPONSE_TIMEOUT_VALUE	(10 * 1000)
#define	NUM_INPUT_SYSEX_BUFFERS			4
#define	SYSEX_INPUT_BUFFER_SIZE			32768L
#define	MAX_SYSEX_REQUEST_LENGTH		8

/****************************************************************************/
/*								Local types									*/
/****************************************************************************/

typedef enum {REQUEST, DUMP} SYSEX_TYPE;

typedef struct
{
	LPBYTE		lpData;					// Sysex data to be transmitted
	UINT		nLength;				// Length of Sysex data to transmit
	SYSEX_TYPE	Type;					// Dump or request ?
} SYSEX_PARAMS, FAR *LPSYSEX_PARAMS;

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

static HPBYTE	*phpRxDataBuffer;		// Pointer to buffer with received SYSEX data int
static DWORD	*pdwRxDataLength;		// Length of received SYSEX data
static HMIDIIN	hMidiInput;				// Input device handle
static HMIDIOUT	hMidiOutput;			// Output device handle

static UINT		nNumRxes, nNumRxesrequired;

/****************************************************************************/
/*								Global data									*/
/****************************************************************************/

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

static VOID			GetMorpheusSysexDump (BYTE bCommandId);
static LPMIDIHDR	MakeMidiOutputBuffer (DWORD dwLength);
static VOID			FreeMidiOutputBuffer (LPMIDIHDR lpMidiHdr);
static LPMIDIHDR	MakeMidiInputBuffer (DWORD dwLength);
static VOID			FreeMidiInputBuffer (LPMIDIHDR lpMidiHdr);
static BOOL			ProcessReceivedSysExBuffer (LPMIDIHDR lpMidiHdr, BOOL *fEnd);

/*^L*/
/*****************************************************************************
 Function:		GetMorpheusData
 Parameters:	HWND	hwndParent		Parent window.
				BYTE	bType			Morpheus data type to get.
				int		iByteParam		Optional BYTE parameter.
				int		iWordParam		Optional WORD request parameter.
				HPBYTE	*phpData		(Output) Data from Morpheus.
				DWORD	*pdwLength		(Output) Length of data from Morpheus.
 Returns:		BOOL					TRUE if OK, else FALSE
 Description:	Gets a specified data type from the Morpheus.
				The type specified is actually the Morpheus COMMAND ID.
				The optional parameter is used for certain data types where
				a numberic parameter is included (eg. PRESET REQUEST).
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 18/3/94 	| Created.											| PW
*****************************************************************************/

BOOL GetMorpheusData (HWND hwndParent, BYTE bType, int iByteParam, int iWordParam, HPBYTE *phpData, DWORD *pdwLength)
{
	SYSEX_PARAMS	RequestParams;
	UINT			i = 0;
	MSG				Msg;
	static HWND		hwndSysex;
	BYTE			bData[MAX_SYSEX_REQUEST_LENGTH];
//	HWND			hwndOldFocus;

	// Store the buffer parameters
	phpRxDataBuffer = phpData;
	pdwRxDataLength = pdwLength;
	*phpData = NULL;
	*pdwLength = 0;

	// Fill in the request data
	bData[i++]		= SYSEX_START;
	bData[i++]		= EMU_MFG_ID;
	bData[i++]		= MORPHEUS_PRODUCT_ID;
	bData[i++]		= bMidiDeviceID;
	bData[i++]		= bType;
	if (iByteParam != NO_PARAM)
	{
		bData[i++]	= (iByteParam & 0x7F);
	}
	if (iWordParam != NO_PARAM)
	{
		WriteSysexWord (&bData[i], iWordParam);
		i += 2;
	}
	bData[i++]		= SYSEX_END;

	// Fill in the request structure
	RequestParams.nLength	= i;
	RequestParams.lpData	= &bData[0];
	RequestParams.Type		= REQUEST;

	/// PJAW
	nNumRxes = 0;
	nNumRxesrequired = (bType == MASTER_DATA_REQUEST) ? 2 : 1;
	/// PJAW

	// Create the SYSEX window
	if ((hwndSysex = CreateWindow (
					MORPHEUS_SYSEX_WINDOW_CLASS,
					"Sysex",
					WS_CHILD,
					CW_USEDEFAULT, CW_USEDEFAULT,
					CW_USEDEFAULT, CW_USEDEFAULT,
					hwndParent,
					NULL,
					hInst,
					(LPVOID)&RequestParams)) == (HWND)NULL)
	{
		MyMessageBox (NULL, ResourceString (IDS_SYSEX_ERR_CREAT_WND), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Local message processing loop
//	hwndOldFocus = SetFocus (hwndSysex);
	while (GetMessage (&Msg, NULL, 0, 0))	// Until WM_QUIT message
	{
		// Handle according to message type
		switch (Msg.message)
		{
			// Controls are dangerous while we're yielding
			case WM_COMMAND:
			case WM_SYSCOMMAND:
			case WM_INITMENU:
			case WM_INITMENUPOPUP:
			case WM_MENUSELECT:
			{
				break;
			}

			// Dispatch everything else
			default:
			{
				// Translate keys to WM_CHAR
				TranslateMessage (&Msg);

				// If ESC has been hit
				if ((Msg.message == WM_CHAR) && (Msg.wParam == VK_ESCAPE))
				{
					// Mark as hit
					fAbortLongOperation = TRUE;
				}

				// Dispatch
				DispatchMessage (&Msg);

				break;
			}
		}
	}

	// Destroy the SYSEX window
	DestroyWindow (hwndSysex);
	hwndSysex = (HWND)NULL;
//	SetFocus (hwndOldFocus);

	return (Msg.wParam == DIALOG_OK ? TRUE : FALSE);
}

/*^L*/
/*****************************************************************************
 Function:		SendMorpheusData
 Parameters:	HWND	hwndParent		Parent window.
				LPBYTE	lpData			SysEx dump data.
				UINT	nLength			Length of SysEx data to send.
 Returns:		BOOL					TRUE if OK, else FALSE
 Description:	Sends the specified sysex data buffer to the Morpheus.
				The buffer must be a fully formatted valid SysEx dump, or
				who knows what the Morpheus will do.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 7/5/94 	| Created.											| PW
*****************************************************************************/

BOOL SendMorpheusData (HWND hwndParent, LPBYTE lpData, UINT nLength)
{
	SYSEX_PARAMS	RequestParams;
	MSG				Msg;
	static HWND		hwndSysex;

	// Fill in the request structure
	RequestParams.nLength	= nLength;
	RequestParams.lpData	= lpData;
	RequestParams.Type		= DUMP;

	// Create the SYSEX window
	if ((hwndSysex = CreateWindow (
					MORPHEUS_SYSEX_WINDOW_CLASS,
					"Sysex",
					WS_CHILD,
					CW_USEDEFAULT, CW_USEDEFAULT,
					CW_USEDEFAULT, CW_USEDEFAULT,
					hwndParent,
					NULL,
					hInst,
					(LPVOID)&RequestParams)) == (HWND)NULL)
	{
		MyMessageBox (NULL, ResourceString (IDS_SYSEX_ERR_CREAT_WND), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}

	// Yielding
	fYielding = TRUE;

	// Local message processing loop
	while (GetMessage (&Msg, NULL, 0, 0))	// Until WM_QUIT message
	{
		// Handle according to message type
		switch (Msg.message)
		{
			// Controls are dangerous while we're yielding
			case WM_COMMAND:
			case WM_SYSCOMMAND:
			case WM_INITMENU:
			case WM_INITMENUPOPUP:
			case WM_MENUSELECT:
			{
				break;
			}

			// Dispatch everything else
			default:
			{
				// Translate keys to WM_CHAR
				TranslateMessage (&Msg);

				// If ESC has been hit
				if ((Msg.message == WM_CHAR) && (Msg.wParam == VK_ESCAPE))
				{
					// Mark as hit
					fAbortLongOperation = TRUE;
				}

				// Dispatch
				DispatchMessage (&Msg);

				break;
			}
		}

	}

	DestroyWindow (hwndSysex);
	hwndSysex = (HWND)NULL;

	// Not yielding
	fYielding = FALSE;

	return (Msg.wParam == DIALOG_OK ? TRUE : FALSE);
}

/*^L*/
/*****************************************************************************
 Function:		SysexDumpWindowProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if message processed, else FALSE.
 Description:	The modeless dialog for a SYSEX MIDI request.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 16/3/94 	| Created.											| PW
*****************************************************************************/

#ifdef USE_SYSEX_DIALOG
DIALOG_PROC SysexDumpWindowProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
#else
WINDOW_PROC  SysexDumpWindowProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
#endif
{
	LPSYSEX_PARAMS				pRequestParams;
	static BOOL					fEndOfSysEx;
	static UINT					nNumInputBuffersInQueue;
	LPMIDIHDR					lpOutputBuffer;
	LPMIDIHDR					lpInputBuffer[NUM_INPUT_SYSEX_BUFFERS];
	LPMIDIHDR					lpRxedHdr;
	UINT						i, n;
	static int					idStatus;
	static UINT					idTimer;
	static SYSEX_TYPE			TransactionType;

	switch (nMsg)
	{
#ifdef USE_SYSEX_DIALOG
		case WM_INITDIALOG:
#else
		// Creation
		case WM_CREATE:
#endif
		{
#ifdef USE_SYSEX_DIALOG
			ShowWindow (hWnd, SW_HIDE);
#endif

			// Redirect MIDI messages to me
			morphdllSetWindow (hWnd, hWnd);

			// Find out what the MIDI device handles are
			morphdllGetHandles (&hMidiInput, &hMidiOutput);

#ifdef USE_SYSEX_DIALOG
			pRequestParams = (LPSYSEX_PARAMS)lParam;
#else
			// Get the specified Morpheus SYSEX request definition
			pRequestParams = (LPSYSEX_PARAMS)((LPCREATESTRUCT)lParam)->lpCreateParams;
#endif

			// Store the transaction type (DUMP or REQUEST)
			TransactionType = pRequestParams->Type;

			// Create an output buffer
			if ((lpOutputBuffer = MakeMidiOutputBuffer (pRequestParams->nLength)) == NULL)
			{
				MyMessageBox (hWnd, ResourceString (IDS_OUT_OF_MEMORY), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);
#ifdef USE_SYSEX_DIALOG
				EndDialog (hWnd, DIALOG_ERROR);
#else
				PostMessage (hWnd, WM_QUIT, DIALOG_ERROR, 0);
#endif
				return (TRUE);
			}

			// Fill it with the request SYSEX message
			hmemcpy (&lpOutputBuffer->lpData[0], &pRequestParams->lpData[0], pRequestParams->nLength);

			// If this is a request
			if (TransactionType == REQUEST)
            {
				// Create some input buffers
				for (i=0 ; i<NUM_INPUT_SYSEX_BUFFERS ; i++)
				{
					lpInputBuffer[i] = MakeMidiInputBuffer (SYSEX_INPUT_BUFFER_SIZE);
					if (lpInputBuffer[i] == NULL)
					{
						if (i != 0)
						{
							for (n=0 ; n<i ; n++)
							{
								FreeMidiInputBuffer (lpInputBuffer[n]);
							}
							MyMessageBox (hWnd, ResourceString (IDS_OUT_OF_MEMORY), ResourceString (IDS_ERROR_TITLE) , MB_ICONEXCLAMATION | MB_OK);

							// Return failure
							morphdllSetWindow ((HWND)NULL, (HWND)NULL);
#ifdef USE_SYSEX_DIALOG
							EndDialog (hWnd, DIALOG_ERROR);
#else
							PostMessage (hWnd, WM_QUIT, DIALOG_ERROR, 0);
#endif
							return (TRUE);
						}
					}
				}

				// Put the input buffers on the queue
				for (i=0 ; i<NUM_INPUT_SYSEX_BUFFERS ; i++)
				{
					midiInAddBuffer (hMidiInput, lpInputBuffer[i], sizeof (MIDIHDR));
				}
				nNumInputBuffersInQueue = NUM_INPUT_SYSEX_BUFFERS;

				// Indicate that SYSEX input is going
				fEndOfSysEx = FALSE;

				// Start input
				midiInStart (hMidiInput);

				// Start a response timeout
				idTimer = SetTimer (hWnd, SYSEX_RESPONSE_TIMEOUT_ID, SYSEX_RESPONSE_TIMEOUT_VALUE, NULL);
			}

			// Send the request off
			if (midiOutLongMsg (hMidiOutput, lpOutputBuffer, sizeof (MIDIHDR)))
			{
				MyMessageBox (hWnd, ResourceString (IDS_SYSEX_ERR_SENDING), ResourceString (IDS_ERROR_TITLE), MB_ICONEXCLAMATION | MB_OK);

				// Return failure
				morphdllSetWindow ((HWND)NULL, (HWND)NULL);
#ifdef USE_SYSEX_DIALOG
				EndDialog (hWnd, DIALOG_ERROR);
#else
				PostMessage (hWnd, WM_QUIT, DIALOG_ERROR, 0);
#endif
				return (TRUE);
			}

			// Start off with an OK status
			idStatus = DIALOG_OK;

			return (TRUE);
		}

		// MIDI input device opened
		case MM_MIM_OPEN:
		{
			return (TRUE);
		}

		// MIDI output device opened
		case MM_MOM_OPEN:
		{
			return (TRUE);
		}

		// Sysex message sent
		case MM_MOM_DONE:
		{
			// Free the output buffer
			FreeMidiOutputBuffer ((LPMIDIHDR)lParam);

			// If this was just a TRANSMIT transaction
			if (TransactionType == DUMP)
			{
				// Return SUCCESS
				morphdllSetWindow ((HWND)NULL, (HWND)NULL);
#ifdef USE_SYSEX_DIALOG
				EndDialog (hWnd, idStatus);
#else
				PostMessage (hWnd, WM_QUIT, idStatus, 0);
#endif
			}

			break;
		}

		// MIDI input data
		case MM_MIM_DATA:
		{
			break;
		}

		// MIDI input error
		case MM_MIM_ERROR:
		{
			break;
		}

		// MIDI input SYSEX data
		case MM_MIM_LONGDATA:
		{
			// Create a pointer to the filled SYSEX buffer
			lpRxedHdr = (LPMIDIHDR)lParam;

			// If we have already received the whole SYSEX message
			if (fEndOfSysEx)
			{
				// Free this buffer as it must be one released by a RESET
				FreeMidiInputBuffer (lpRxedHdr);

				// If there are no buffers now in circulation
				if (--nNumInputBuffersInQueue == 0)
				{
					// Excellent - everything's done ...

					// Stop the timer
					KillTimer (hWnd, SYSEX_RESPONSE_TIMEOUT_ID);

					// If everything went OK
					if (idStatus == DIALOG_OK)
					{
						// Return SUCCESS
						morphdllSetWindow ((HWND)NULL, (HWND)NULL);
#ifdef USE_SYSEX_DIALOG
						EndDialog (hWnd, idStatus);
#else
						PostMessage (hWnd, WM_QUIT, idStatus, 0);
#endif
					}
                    else
					// Else here was a timeout, or the user cancelled
					{
						// Free the received buffer pointer
						if (*phpRxDataBuffer != NULL)
						{
							GlobalFreePtr (*phpRxDataBuffer);
						}

						// Return FAILURE
						morphdllSetWindow ((HWND)NULL, (HWND)NULL);
#ifdef USE_SYSEX_DIALOG
						EndDialog (hWnd, idStatus);
#else
						PostMessage (hWnd, WM_QUIT, idStatus, 0);
#endif
					}
				}
			}
			else
            {
				// Process the received buffer
				if (!ProcessReceivedSysExBuffer (lpRxedHdr, &fEndOfSysEx))
				{
					// Out of memory error processing the data - pretend SYSEX is complete
					fEndOfSysEx = TRUE;
				}

				// If that was the end of the SYSEX data
				if (fEndOfSysEx)
				{
					// Stop recording and force pending input to be freed
					midiInReset (hMidiInput);

					// Free that buffer
					FreeMidiInputBuffer (lpRxedHdr);

					// Decrement the count of buffers in circulation
					nNumInputBuffersInQueue--;
				}
				else
				{
					// Recirculate the buffer
					midiInUnprepareHeader (hMidiInput, lpRxedHdr, sizeof (MIDIHDR));
					midiInPrepareHeader (hMidiInput, lpRxedHdr, sizeof (MIDIHDR));
					midiInAddBuffer (hMidiInput, lpRxedHdr, sizeof (MIDIHDR));
				}
			}

			break;
		}

		// MIDI input SYSEX error
		case MM_MIM_LONGERROR:
		{
			// Stop the timer
			KillTimer (hWnd, SYSEX_RESPONSE_TIMEOUT_ID);

			// Set the current state to ERROR
			idStatus = DIALOG_ERROR;

			// Pretend the SYSEX message end has been seen
			fEndOfSysEx = TRUE;

			// Stop recording and force pending input to be freed
			midiInReset (hMidiInput);

			break;
		}

		// Timeout
		case WM_TIMER:
		{
			// Stop the timer
			KillTimer (hWnd, SYSEX_RESPONSE_TIMEOUT_ID);

			// Set the current state to TIMEOUT
			idStatus = DIALOG_TIMEOUT;

			// Pretend the SYSEX message end has been seen
			fEndOfSysEx = TRUE;

			// Stop recording and force pending input to be freed
			midiInReset (hMidiInput);

			return (TRUE);
		}

		// Control
		case WM_COMMAND:
		{
			switch (wParam)
			{
				// User selected CANCEL
				case IDCANCEL:
				{
					// Set the current state to CANCELLED
					idStatus = DIALOG_CANCELLED;

					// Pretend the SYSEX message end has been seen
					fEndOfSysEx = TRUE;

					// Stop recording and force pending input to be freed
					midiInReset (hMidiInput);

					return (TRUE);
				}

				default:
				{
					return (FALSE);
				}
			}
		}

		// Default
		default:
		{
			break;
		}
	}

	// Default processing
#ifdef USE_SYSEX_DIALOG
	return (FALSE);
#else
	return (DefWindowProc (hWnd, nMsg, wParam, lParam));
#endif
}

/*^L*/
/*****************************************************************************
 Function:		MakeMidiOutputBuffer
 Parameters:    DWORD	dwLength		Length of buffer required.
 Returns:		LPMIDIHDR				Pointer to MIDIHDR created.
 Description:	Allocates a MIDIHDR and data buffer for SYSEX data.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 17/3/94 	| Created.											| PW
*****************************************************************************/

static LPMIDIHDR MakeMidiOutputBuffer (DWORD dwLength)
{
	LPMIDIHDR	lpMidiHdr;
	LPSTR		lpData;

	// Allocate the header
	if ((lpMidiHdr = (LPMIDIHDR)GlobalAllocPtr (GMEM_MOVEABLE | GMEM_SHARE, sizeof (MIDIHDR))) == NULL)
	{
		return (NULL);
	}

	// Allocate the data
	if ((lpData = (LPSTR)GlobalAllocPtr (GMEM_MOVEABLE | GMEM_SHARE, dwLength)) == NULL)
	{
		GlobalFreePtr (lpMidiHdr);
		return (NULL);
	}

	// Fill in the header
	_fmemset (lpMidiHdr, 0, sizeof (MIDIHDR));
	lpMidiHdr->lpData			= lpData;
	lpMidiHdr->dwBufferLength	= dwLength;

	// Prepare it
	if (midiOutPrepareHeader (hMidiOutput, lpMidiHdr, sizeof (MIDIHDR)))
	{
		// Couldn't allocate the header
		GlobalFreePtr (lpData);
		GlobalFreePtr (lpMidiHdr);
		return (NULL);
	}

	// Return the MIDIHDR
	return (lpMidiHdr);
}

/*^L*/
/*****************************************************************************
 Function:		FreeMidiOutputBuffer
 Parameters:    LPMIDIHDR	lpMidiHdr   Midi header.
 Returns:		None.
 Description:	Frees a MIDIHDR and its data.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 17/3/94 	| Created.											| PW
*****************************************************************************/

static VOID FreeMidiOutputBuffer (LPMIDIHDR lpMidiHdr)
{
	// Unprepare it
	midiOutUnprepareHeader (hMidiOutput, lpMidiHdr, sizeof (MIDIHDR));

	// Free the data buffer then the header
	GlobalFreePtr (lpMidiHdr->lpData);
	GlobalFreePtr (lpMidiHdr);
}

/*^L*/
/*****************************************************************************
 Function:		MakeMidiInputBuffer
 Parameters:    DWORD	dwLength		Length of buffer required.
 Returns:		LPMIDIHDR				Pointer to MIDIHDR created.
 Description:	Allocates a MIDIHDR and data buffer for SYSEX data.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 17/3/94 	| Created.											| PW
*****************************************************************************/

static LPMIDIHDR MakeMidiInputBuffer (DWORD dwLength)
{
	LPMIDIHDR	lpMidiHdr;
	LPSTR		lpData;

	// Allocate the header
	if ((lpMidiHdr = (LPMIDIHDR)GlobalAllocPtr (GMEM_MOVEABLE | GMEM_SHARE, sizeof (MIDIHDR))) == NULL)
	{
		// Couldn't allocate the header
		return (NULL);
	}

	// Allocate the data
	if ((lpData = GlobalAllocPtr (GMEM_MOVEABLE | GMEM_SHARE, dwLength)) == NULL)
	{
		// Couldn't allocate the header
		GlobalFreePtr (lpMidiHdr);
		return (NULL);
	}

	// Fill in the header
	_fmemset (lpMidiHdr, 0, sizeof (MIDIHDR));
	lpMidiHdr->lpData			= lpData;
	lpMidiHdr->dwBufferLength	= dwLength;

	// Prepare it
	if (midiInPrepareHeader (hMidiInput, lpMidiHdr, sizeof (MIDIHDR)))
	{
		// Couldn't allocate the header
		GlobalFreePtr (lpData);
		GlobalFreePtr (lpMidiHdr);
		return (NULL);
	}

	// Return the MIDIHDR
	return (lpMidiHdr);
}

/*^L*/
/*****************************************************************************
 Function:		FreeMidiInputBuffer
 Parameters:    LPMIDIHDR	lpMidiHdr   Midi header.
 Returns:		None.
 Description:	Frees a MIDIHDR and its data.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 17/3/94 	| Created.											| PW
*****************************************************************************/

static VOID FreeMidiInputBuffer (LPMIDIHDR lpMidiHdr)
{
	// Unprepare it
	midiInUnprepareHeader (hMidiInput, lpMidiHdr, sizeof (MIDIHDR));

	// Free the data buffer then the header
	GlobalFreePtr (lpMidiHdr->lpData);
	GlobalFreePtr (lpMidiHdr);
}

/*^L*/
/*****************************************************************************
 Function:		ProcessReceivedSysExBuffer
 Parameters:	LPMIDIHDR	lpMidiHdr	Pointer to received MIDIHDR.
				BOOL		*fEnd		Set to TRUE if end of SYSEX received.
 Returns:		BOOL					TRUE if OK, else FALSE.
 Description:	Processes a received SYSEX message buffer. The buffer may
				just be part of the whole SYSEX message. The function has to
				look at the last byte in the buffer to see if it's the end of
				the SYSEX message, and if so perhaps take special action.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 19/3/94 	| Created.											| PW
*****************************************************************************/

static BOOL ProcessReceivedSysExBuffer (LPMIDIHDR lpMidiHdr, BOOL *fEnd)
{
	static HPBYTE	hpWholeSysExBuffer = NULL;
	static DWORD	dwWholeSysExLength = 0;

	// If we haven't current got a buffer to store the received SYSEX data in
	if (hpWholeSysExBuffer == NULL)
	{
		// Allocate one
		if ((hpWholeSysExBuffer = (HPBYTE)GlobalAllocPtr (GMEM_MOVEABLE, lpMidiHdr->dwBytesRecorded)) == NULL)
		{
			// Out of memory
			hpWholeSysExBuffer = NULL;
			dwWholeSysExLength = 0;
			return (FALSE);
		}
	}
	else
	// We've already got some SYSEX data
	{
		// Re-allocate the cumulative buffer to allow space for the received data
		if ((hpWholeSysExBuffer = GlobalReAllocPtr (hpWholeSysExBuffer,
									dwWholeSysExLength + lpMidiHdr->dwBytesRecorded,
									GMEM_MOVEABLE)) == NULL)
		{
			// Out of memory
			hpWholeSysExBuffer = NULL;
			dwWholeSysExLength = 0;
			return (FALSE);
		}

	}

	// Copy the received data into it
	hmemcpy (&hpWholeSysExBuffer[dwWholeSysExLength], lpMidiHdr->lpData, lpMidiHdr->dwBytesRecorded);

	// Add to the length received so far
	dwWholeSysExLength += lpMidiHdr->dwBytesRecorded;

	// If the final byte in the SYSEX buffer is the end flag
	if (hpWholeSysExBuffer[dwWholeSysExLength - 1] == SYSEX_END)
	{
		/// PJAW
		if (++nNumRxes == nNumRxesrequired)
        {
		/// PJAW

		// Set the output flag indicating we at the end of the SYSEX message
		*fEnd = TRUE;

		// Write the address of the allocated SYSEX buffer and its length out
		*phpRxDataBuffer = hpWholeSysExBuffer;
		*pdwRxDataLength = dwWholeSysExLength;

		// Reset the local buffer pointer and length
		hpWholeSysExBuffer = NULL;
		dwWholeSysExLength = 0;

		/// PJAW
		}
		/// PJAW

	}

	// OK
	return (TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		WriteSysexWord
 Parameters:	LPVOID	lpAddr			Address to write the word to.
				int		iValue			Value to write.
 Returns:		None.
 Description:	Writes a signed integer value to a Sysex buffer, as defined in
				the Morpheus Sysex description.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 19/3/94 	| Created.											| PW
*****************************************************************************/

VOID WriteSysexWord (LPVOID lpAddr, int iValue)
{
	WORD	w;

	w = (iValue < 0) ? (WORD)(iValue + 16384) : (WORD)iValue;
	((LPBYTE)lpAddr)[0] = (w % 128);
	((LPBYTE)lpAddr)[1] = (w / 128);
}

/*^L*/
/*****************************************************************************
 Function:		ReadSysexWord
 Parameters:	LPVOID	lpAddr			Address to write the word to.
 Returns:		int						14 bit signed integer from lpAddr.
 Description:	Reads a signed integer value from a Sysex buffer, as defined in
				the Morpheus Sysex description.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 19/3/94 	| Created.											| PW
*****************************************************************************/

int ReadSysexWord (LPVOID lpAddr)
{
	WORD	raw;

	raw = 128 * ((LPBYTE)lpAddr)[1] + ((LPBYTE)lpAddr)[0];
	return ((raw > 8192) ? (int)raw - 16384 : (int)raw);
}

