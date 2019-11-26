/*****************************************************************************
 File:			auc_i.h
 Description:	The audio control internal header.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/4/94		| Created.											| PW
*****************************************************************************/

/****************************************************************************/
/*								Defines										*/
/****************************************************************************/

#if defined (WINDOWS32)
 #define	WINDOW_PROC					LRESULT WINAPI
 #define	DIALOG_PROC					BOOL WINAPI
 #define	NCODE						HIWORD (wParam)
 #define	GETWNDID(hWnd)				GetWindowLong (hWnd, GWL_ID)
 #define	SETWNDID(hWnd,id)			SetWindowLong (hWnd, GWL_ID, id)
#else
 #define	WINDOW_PROC					LRESULT FAR PASCAL _export
 #define	DIALOG_PROC					BOOL FAR PASCAL _export
 #define	NCODE						HIWORD (lParam)
 #define	GETWNDID(hWnd)				GetWindowWord (hWnd, GWW_ID)
 #define	SETWNDID(hWnd,id)			SetWindowWord (hWnd, GWW_ID, id)
#endif

#if !defined (SHORT)
 #define	SHORT						short
#endif

/****************************************************************************/
/*								Functions									*/
/****************************************************************************/

extern BOOL SliderControlInit (HINSTANCE hInst);
extern VOID SliderControlTerm (HINSTANCE hInst);

extern BOOL UpdownControlInit (HINSTANCE hInst);
extern VOID UpdownControlTerm (HINSTANCE hInst);

extern BOOL EnvelopeControlInit (HINSTANCE hInst);
extern VOID EnvelopeControlTerm (HINSTANCE hInst);

extern BOOL PatchPinControlInit (HINSTANCE hInst);
extern VOID PatchPinControlTerm (HINSTANCE hInst);

