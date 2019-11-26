/*****************************************************************************
 File:			morfile.h
 Description:	The MORPHEUS FILE internal header.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 1/6/94		| Created.											| PW
*****************************************************************************/

// Morpheus FILE HEADER strings

#define	FILE_VERSION				"0.001"
#define	MFILE_INTERNALS_HEADER		"Morpheus internals" FILE_VERSION
#define	MFILE_SOUNDS_HEADER			"Morpheus all sounds" FILE_VERSION
#define	MFILE_PRESET_HEADER			"Morpheus preset" FILE_VERSION
#define	MFILE_HYPERPRESET_HEADER	"Morpheus hyperpreset" FILE_VERSION
#define	MFILE_MIDIMAP_HEADER		"Morpheus midimap" FILE_VERSION
#define	MFILE_PROGRAM_MAP_HEADER	"Morpheus program map" FILE_VERSION
#define	MFILE_USER_TUNING_HEADER	"Morpheus user tuning" FILE_VERSION

// The fixed MORPHEUS INTERNALS FILE NAME

#define	INTERNALS_FILE_NAME			"morph.int"

// Utility functions

extern BOOL		WriteLengthAndData (HFILE hFile, WORD FAR *pLength, LPVOID lpData, LPWORD lpCsum);
extern LPVOID	ReadLengthAndData (HFILE hFile, WORD FAR *pLength, LPWORD lpCsum);
extern BOOL		FileChecksumOk (HFILE hFile);
//extern WORD		CalcChecksum (LPVOID lpData, WORD nLength);
