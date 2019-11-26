/*****************************************************************************
 File:			animate.h
 Description:	The animation control include file
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 29/9/94	| Created.											| PW
*****************************************************************************/

// Class name

#define	ANIMATION_CONTROL				"AnimationControl"

// Maximum number of bitmaps in the animation

#define	MAX_BITMAPS						100

// Control messages

#define	ANIM_ADD_BITMAP					(WM_USER + 0x100)
#define	ANIM_SET_INTERVAL				(WM_USER + 0x101)
#define	ANIM_START_ANIMATION			(WM_USER + 0x102)
#define	ANIM_PAUSE_ANIMATION			(WM_USER + 0x103)
#define	ANIM_RESET						(WM_USER + 0x104)

// Functions

extern BOOL AnimateControlInit (HINSTANCE hInst);
extern VOID AnimateControlTerm (HINSTANCE hInst);


