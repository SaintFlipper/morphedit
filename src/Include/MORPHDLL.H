/*****************************************************************************
 File:			morphdll.h
 Description:	The Morpheus MIDI callback DLL header. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 9/4/94		| Created.											| PW
*****************************************************************************/

#if defined (__WIN32__) || defined (_WIN32)
 #if !defined (WINDOWS32)
  #define	WINDOWS32
 #endif
#endif

/****************************************************************************/
/*								Defines										*/
/****************************************************************************/

// Macros

#if defined (WINDOWS32)

 #define	DllImport	__declspec( dllimport )
 #define	DllExport	__declspec( dllexport )
 #define	INTEXPORT	DllExport	int
 #define	INTIMPORT	DllImport	int
 #define	VOIDEXPORT	DllExport	VOID
 #define	VOIDIMPORT	DllImport	VOID
 #define	BOOLEXPORT	DllExport	BOOL
 #define	BOOLIMPORT	DllImport	BOOL

#else

 #define	INTEXPORT	int FAR PASCAL _export
 #define	INTIMPORT	int FAR PASCAL _export
 #define	VOIDEXPORT	VOID FAR PASCAL _export
 #define	VOIDIMPORT	VOID FAR PASCAL _export
 #define	BOOLEXPORT	BOOL FAR PASCAL _export
 #define	BOOLIMPORT	BOOL FAR PASCAL _export

#endif

/****************************************************************************/
/*								Typedefs									*/
/****************************************************************************/

/****************************************************************************/
/*								Functions									*/
/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Function to open MIDI devices
extern BOOLEXPORT morphdllOpenMidiDevices (int idInput, int idOutput);

// Function to set the notification window
extern VOIDEXPORT morphdllSetWindow (HWND hInput, HWND hOutput);

// Function to close MIDI devices
extern BOOLEXPORT morphdllCloseMidiDevices (VOID);

// Function to retrieve the open device handles
extern VOIDEXPORT morphdllGetHandles (LPHMIDIIN phMidiIn, LPHMIDIOUT phMidiOut);

#ifdef __cplusplus
}
#endif
