/*****************************************************************************
 File:			msgbox.c
 Description:	My message box functions. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 1/6/94		| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local types									*/
/****************************************************************************/

typedef struct
{
	UINT	fuStyle;			// Message box style
	LPCSTR	lpszText;			// Message box text
	LPCSTR	lpszTitle;			// Message box title
} MESSAGE_BOX_PARAMS, FAR *PMESSAGE_BOX_PARAMS;

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

DIALOG_PROC MessageBoxDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
static VOID UnTabControl (HWND hWnd, int idControl);

/*^L*/
/*****************************************************************************
 Function:		MyResMessageBox
 Parameters:	HWND	hwndParent		Parent window handle.
				int		idText			Resource ID for text.
				int		idTitle			Resource ID for title.
				UINT	fuStyle			Message box style.
 Returns:		int						0 if out of memory.
 Description:	Displays a message box.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 1/6/94 	| Created.											| PW
*****************************************************************************/

int MyResMessageBox (HWND hwndParent, int idText, int idTitle, UINT fuStyle)
{
	char	szText[256], szTitle[256];

	// Load the resource strings
	LoadString (hInst, idText, (LPSTR)&szText[0], sizeof (szText));
	LoadString (hInst, idTitle, (LPSTR)&szTitle[0], sizeof (szTitle));

	// Call my message box
	return (MyMessageBox (hwndParent, &szText[0], &szTitle[0], fuStyle));
}

/*^L*/
/*****************************************************************************
 Function:		MyMessageBox
 Parameters:	HWND	hwndParent		Parent window handle.
				LPCSTR	lpszText		Message box text.
				LPCSTR	lpszTitle		Message box title.
				UINT	fuStyle			Message box style.
 Returns:		int						0 if out of memory.
 Description:	Displays a message box.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 1/6/94 	| Created.											| PW
*****************************************************************************/

int MyMessageBox (HWND hwndParent, LPCSTR lpszText, LPCSTR lpszTitle, UINT fuStyle)
{
	DLGPROC				lpDialogProc;
	int					nRetVal;
	MESSAGE_BOX_PARAMS	Params;

	// Set up the message box parameters
	Params.fuStyle		= fuStyle;
	Params.lpszText		= lpszText;
	Params.lpszTitle	= lpszTitle;

	// Do the dialog
	lpDialogProc = MakeProcInstance (MessageBoxDialogProc, hInst);
	nRetVal = DialogBoxParam (hInst, "DLG_MESSAGE_BOX", hwndParent, lpDialogProc, (LPARAM)(LPVOID)&Params);
	FreeProcInstance (lpDialogProc);
	return (nRetVal);
}

/*^L*/
/*****************************************************************************
 Function:		MessageBoxDialogProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		BOOL					TRUE if processed, else FALSE.
 Description:	The message box dialog procedure.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 1/6/94 	| Created.											| PW
*****************************************************************************/

DIALOG_PROC MessageBoxDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	PMESSAGE_BOX_PARAMS	pParams;
	UINT				fuStyle;
	LOGFONT				lf;
	static HBITMAP		hBitmap;
	static HFONT		hTextFont;

	switch (nMsg)
	{
		// Initialisation
		case WM_INITDIALOG:
		{
			// Create the text box font
			memset (&lf, 0, sizeof (lf));
			lf.lfHeight			= 18;
			lf.lfWeight			= FW_NORMAL;
			lf.lfCharSet		= ANSI_CHARSET;
			lf.lfOutPrecision	= OUT_TT_PRECIS;
			lf.lfClipPrecision	= CLIP_LH_ANGLES | CLIP_DEFAULT_PRECIS;
			lf.lfQuality		= PROOF_QUALITY;
			lf.lfPitchAndFamily	= VARIABLE_PITCH | FF_DONTCARE;
			lf.lfEscapement		= 0;
			strcpy (&lf.lfFaceName[0], "Arial");
			if ((hTextFont = CreateFontIndirect (&lf)) == (HFONT)NULL)
			{
				return (FALSE);
			}

			// Reformat myself
			ReformatDialog (hWnd, RDF_CENTRE | RDF_THINFONT);

			// Fetch the parameters
			pParams = (PMESSAGE_BOX_PARAMS)lParam;
			fuStyle = pParams->fuStyle;

			// Set my title
			SetWindowText (hWnd, pParams->lpszTitle);

			// Set the message box text
			SendDlgItemMessage (hWnd, IDC_MESSAGE_TEXT, WM_SETFONT, (WPARAM)hTextFont, FALSE);
			SetDlgItemText (hWnd, IDC_MESSAGE_TEXT, pParams->lpszText);

			// Decide which bitmap to display
			if ((fuStyle & 0xF0) == MB_ICONEXCLAMATION)
			{
            	hBitmap = LoadBitmap (hInst, "BMP_MB_EXCLAMATION");
			}
			else if (((fuStyle & 0xF0) == MB_ICONHAND) || ((fuStyle & 0xF0) == MB_ICONSTOP))
			{
            	hBitmap = LoadBitmap (hInst, "BMP_MB_STOP");
			}
			else if ((fuStyle & 0xF0) == MB_ICONQUESTION)
			{
				hBitmap = LoadBitmap (hInst, "BMP_MB_QUESTION");
            }
			else
			{
				hBitmap = LoadBitmap (hInst, "BMP_MB_INFO");
			}

			// Decide where to put the initial focus
			if (fuStyle & MB_DEFBUTTON2)
			{
				SetFocus (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON2));
			}
			else if (fuStyle & MB_DEFBUTTON2)
			{
				SetFocus (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON3));
			}
			else
			{
				SetFocus (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON1));
			}

			// Decide which buttons to allow
			if ((fuStyle & 7) == MB_ABORTRETRYIGNORE)
			{
				SetWindowText (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON1), "&Abort");
				SetWindowText (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON2), "&Retry");
				SetWindowText (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON3), "&Ignore");

				SETWNDID (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON1), IDABORT);
				SETWNDID (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON2), IDRETRY);
				SETWNDID (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON3), IDIGNORE);
			}
			else if ((fuStyle & 7) == MB_OKCANCEL)
			{
				SetWindowText (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON1), "&OK");
				SetWindowText (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON2), "&Cancel");
				ShowWindow (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON3), SW_HIDE);
				UnTabControl (hWnd, IDC_MESSAGE_BUTTON3);
				SETWNDID (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON1), IDOK);
				SETWNDID (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON2), IDCANCEL);
			}
			else if ((fuStyle & 7) == MB_RETRYCANCEL)
			{
				SetWindowText (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON1), "&Retry");
				SetWindowText (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON2), "&Cancel");
				ShowWindow (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON3), SW_HIDE);
				UnTabControl (hWnd, IDC_MESSAGE_BUTTON3);
				SETWNDID (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON1), IDRETRY);
				SETWNDID (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON2), IDCANCEL);
			}
			else if ((fuStyle & 7) == MB_YESNO)
			{
				SetWindowText (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON1), "&Yes");
				SetWindowText (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON2), "&No");
				ShowWindow (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON3), SW_HIDE);
				UnTabControl (hWnd, IDC_MESSAGE_BUTTON3);
				SETWNDID (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON1), IDYES);
				SETWNDID (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON2), IDNO);
			}
			else if ((fuStyle & 7) == MB_YESNOCANCEL)
			{
				SetWindowText (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON1), "&Yes");
				SetWindowText (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON2), "&No");
				SetWindowText (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON2), "&Cancel");
				SETWNDID (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON1), IDYES);
				SETWNDID (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON2), IDNO);
				SETWNDID (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON3), IDCANCEL);
			}
			else
			{
				SetWindowText (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON1), "&OK");
				ShowWindow (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON2), SW_HIDE);
				UnTabControl (hWnd, IDC_MESSAGE_BUTTON2);
				ShowWindow (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON3), SW_HIDE);
				UnTabControl (hWnd, IDC_MESSAGE_BUTTON3);
				SETWNDID (GetDlgItem (hWnd, IDC_MESSAGE_BUTTON1), IDOK);
			}

			// Force the user drawn button to be exactly 64 x 64 pixels
			SetWindowPos (GetDlgItem (hWnd, IDC_MESSAGE_BITMAP), HWND_TOP, 0, 0, 64, 64, SWP_NOMOVE | SWP_NOZORDER);

			return (FALSE);
		}

		// A control
		case WM_COMMAND:
		{
			// Return the control ID
			EndDialog (hWnd, LOWORD (wParam));
			return (TRUE); 
		}

		// Draw the user button
		case WM_DRAWITEM:
		{
			// If we are being told to draw it
			if (((DRAWITEMSTRUCT FAR *)lParam)->itemAction == ODA_DRAWENTIRE)
			{
				// Draw the bitmap
				DrawBitmap (((DRAWITEMSTRUCT FAR *)lParam)->hDC, hBitmap, 0, 0);
			}
			return (TRUE);
		}

		// Destruction of the window
		case WM_DESTROY:
		{
			// Get rid of the bitmap
			DeleteObject (hBitmap);

			// Get rid of the text box font
			DeleteObject (hTextFont);

			return (TRUE);
		}

		// Control about to be drawn
		case WM_CTLCOLOR:
		{
			Ctl3dCtlColorEx (nMsg, wParam, lParam);
			break;
		}

		// Unknown messages
		default:
		{
			break;
		}
	}

	// Not processed
	return (FALSE);
}

static VOID UnTabControl (HWND hWnd, int idControl)
{
	LONG	l;
	HWND	hwndControl = GetDlgItem (hWnd, idControl);

	l = GetWindowLong (hwndControl, GWL_STYLE);
	l &= ~WS_TABSTOP;
	SetWindowLong (hwndControl, GWL_STYLE, l);
}

