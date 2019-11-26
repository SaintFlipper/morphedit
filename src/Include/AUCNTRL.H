/*****************************************************************************
 File:			aucntrl.h
 Description:	The AUDIO CONTROLS control header.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 31/3/94	| Created.											| PW
*****************************************************************************/

#ifndef INC_AUCNTRL_H
#define INC_AUCNTRL_H

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

// Colours

#define BLUE	RGB(0, 0, 255)
#define RED 	RGB(255, 0, 0)
#define GREEN	RGB(0, 255, 0)
#define BLACK	RGB(0, 0, 0)
#define WHITE	RGB(255, 255, 255)

// Control window classes
#define	SLIDER_CLASS_NAME	"SliderControl"
#define	UPDOWN_CLASS_NAME	"UpdownControl"
#define	ENVELOPE_CLASS_NAME	"EnvelopeControl"
#define	PIN_CLASS_NAME		"PinControl"

// Dimensions of the controls
#define	SLIDER_CONTROL_WIDTH		29
#define	SLIDER_CONTROL_HEIGHT		100
#define	UPDOWN_CONTROL_WIDTH		16
#define	UPDOWN_CONTROL_HEIGHT		16

// Control messages (SLIDER)
#define	SL_SET_MIN          WM_USER + 0
#define	SL_SET_MAX			WM_USER + 1
#define	SL_SET_CURRENT		WM_USER + 2

// Control messages (UP-DOWN)
#define	UD_SET_MIN          WM_USER + 4
#define	UD_SET_MAX			WM_USER + 5
#define	UD_SET_CURRENT		WM_USER + 6
#define	UD_SET_NUM_DISPLAY	WM_USER + 7

// Control messages (ENVELOPE)
#define	ENV_SET_ATTACK		WM_USER + 8
#define	ENV_SET_HOLD		WM_USER + 9
#define	ENV_SET_DECAY		WM_USER + 10
#define	ENV_SET_SUSTAIN		WM_USER + 11
#define	ENV_SET_RELEASE		WM_USER + 12

// Control messages (PIN)
#define	PIN_SET_CURRENT		WM_USER + 13
#define	PIN_SET_FOCUS		WM_USER + 14
#define	PIN_KILL_FOCUS		WM_USER + 15

// Notification messages (SLIDER)
#define	SLN_NEW_VALUE		WM_USER + 100

// Notification messages (UP-DOWN)
#define	UDN_NEW_VALUE		WM_USER + 102

// Notification messages (PIN)
#define	PINN_SELECTED		WM_USER + 103

/****************************************************************************/
/*								Functions									*/
/****************************************************************************/

#if defined (WINDOWS32)
typedef VOID (CALLBACK* VALUE_DISPLAY_FUNCTION) (HWND hWnd, int nValue);
#else
typedef VOID CALLBACK (* VALUE_DISPLAY_FUNCTION) (HWND hWnd, int nValue);
#endif

/****************************************************************************/
/*								Functions									*/
/****************************************************************************/

#endif
