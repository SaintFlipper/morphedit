//---------------------------------------------------------------------------
//
//  Module:   mdidlg.h
//
//  Purpose:
//     This is the header for the MDI dialog functions.
//
//  Development Team:
//     Bryan A. Woodruff
//
//  Comments:
//     NOTE:  DLGICON is defined as 100 - this should be set according
//            to the resource defintion of a "dialog box" icon.
//
//  History:   Date       Author      Comment
//              4/ 5/92   BryanW      Extracted for sample.
//
//---------------------------------------------------------------------------
//
//  Written by Microsoft Product Support Services, Windows Developer Support.
//  Copyright (c) 1991 Microsoft Corporation.  All Rights Reserved.
//
//---------------------------------------------------------------------------

// constant definitions

#define GWL_MDIDIALOGPROC        0
#define GWW_MDIDIALOGFONT        4
#define GWW_MDIDIALOGEXTRABYTES  6

#define MAXLEN_MENUNAME          64
#define MAXLEN_CLASSNAME         64
#define MAXLEN_CAPTIONTEXT       132
#define MAXLEN_TYPEFACE          64

/*** this string is used to compute the average character width ***/

#define WIDTHSTRING "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

// data structures & data types

#if (WINVER < 0x30a)
typedef unsigned int     UINT ;
typedef const char FAR*  LPCSTR ;
typedef UINT             WPARAM ;
typedef LONG             LPARAM ;
typedef LONG             LRESULT ;
typedef FARPROC          DLGPROC ;
typedef HANDLE           HINSTANCE ;
#endif

typedef struct tagMDIDIALOGINFO
{
   int      dtItemCount ;
   DLGPROC  lpDialogProc ;
   HFONT    hFont ;
   LPARAM   lParam ;
   LPSTR    lpDlgItems ;
   WORD     wUnitsX, wUnitsY ;

} MDIDIALOGINFO, NEAR *NPMDIDIALOGINFO, FAR *LPMDIDIALOGINFO ;

// macros

#ifdef _WIN32
#define GETHINST(hWnd)    ((HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE))
#else
#define GETHINST( hWnd )    ((HINSTANCE) GetWindowWord( hWnd, GWW_HINSTANCE ))
#endif

#define DLGTOCLIENTX( x, units )   MulDiv( x, units, 4 )
#define DLGTOCLIENTY( y, units )   MulDiv( y, units, 8 )

// function declarations

BOOL NEAR CreateMDIDialogChildren( HWND, LPSTR, int, HFONT, WORD, WORD ) ;
BOOL FAR PASCAL RegisterMDIDialog( HINSTANCE ) ;
HWND FAR PASCAL CreateMDIDialogParam( HINSTANCE, LPCSTR, HWND, DLGPROC,
                                      LPARAM ) ;
LRESULT FAR PASCAL MDIDialogProc( HWND, UINT, WPARAM, LPARAM ) ;


//---------------------------------------------------------------------------
//  End of File: mdidlg.h
//---------------------------------------------------------------------------

