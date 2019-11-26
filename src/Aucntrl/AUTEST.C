
#include <stdlib.h>
#include <windows.h>

#include <aucntrl.h>
#include "autest.h"

HINSTANCE	hInst;

BOOL CALLBACK JunkDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
VOID CALLBACK _export DisplayValue (HWND hWnd, int nValue);

int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	DLGPROC		DlgProc;
	HINSTANCE	hDll;

	hDll = LoadLibrary ("aucntrl.dll");

	DlgProc = (DLGPROC)MakeProcInstance (JunkDialogProc, hInstance);
	DialogBox (hInstance, "DLG_JUNK", NULL, DlgProc);
	FreeProcInstance ((FARPROC)DlgProc);

	FreeLibrary (hDll);

	return (0);
}

BOOL CALLBACK JunkDialogProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg)
	{
		case WM_INITDIALOG:
		{
//			ReformatDialog (hWnd, RDF_CENTRE | RDF_THINFONT);

			SendDlgItemMessage (hWnd, IDC_A1, SL_SET_MIN, 0, 0);
			SendDlgItemMessage (hWnd, IDC_A1, SL_SET_MAX, 99, 0);
			SendDlgItemMessage (hWnd, IDC_A1, SL_SET_CURRENT, 20, 0);

			SendDlgItemMessage (hWnd, IDC_H1, SL_SET_MIN, 0, 0);
			SendDlgItemMessage (hWnd, IDC_H1, SL_SET_MAX, 99, 0);
			SendDlgItemMessage (hWnd, IDC_H1, SL_SET_CURRENT, 20, 0);

			SendDlgItemMessage (hWnd, IDC_D1, SL_SET_MIN, 0, 0);
			SendDlgItemMessage (hWnd, IDC_D1, SL_SET_MAX, 99, 0);
			SendDlgItemMessage (hWnd, IDC_D1, SL_SET_CURRENT, 20, 0);

			SendDlgItemMessage (hWnd, IDC_S1, SL_SET_MIN, 0, 0);
			SendDlgItemMessage (hWnd, IDC_S1, SL_SET_MAX, 99, 0);
			SendDlgItemMessage (hWnd, IDC_S1, SL_SET_CURRENT, 20, 0);

			SendDlgItemMessage (hWnd, IDC_R1, SL_SET_MIN, 0, 0);
			SendDlgItemMessage (hWnd, IDC_R1, SL_SET_MAX, 99, 0);
			SendDlgItemMessage (hWnd, IDC_R1, SL_SET_CURRENT, 20, 0);

			SendDlgItemMessage (hWnd, IDC_PIN1_SLIDER, SL_SET_MIN, -127, 0);
			SendDlgItemMessage (hWnd, IDC_PIN1_SLIDER, SL_SET_MAX, 127, 0);
			SendDlgItemMessage (hWnd, IDC_PIN1_SLIDER, SL_SET_CURRENT, 0, 0);

			return (TRUE);
		}

		case WM_COMMAND:
		{
			switch (wParam)
			{
				case IDOK:
				{
					EndDialog (hWnd, 0);
					return (TRUE);
				}

				case IDC_HIDE1:
				{
					static BOOL	fVisible = TRUE;
					ShowWindow (GetDlgItem (hWnd, IDC_A1), fVisible ? SW_HIDE : SW_SHOW);
					fVisible = fVisible ? FALSE : TRUE;
					return (TRUE);
                }

				case IDC_HIDE2:
				{
					static BOOL	fVisible = TRUE;
					ShowWindow (GetDlgItem (hWnd, IDC_H1), fVisible ? SW_HIDE : SW_SHOW);
					fVisible = fVisible ? FALSE : TRUE;
					return (TRUE);
                }
				case IDC_HIDE3:
				{
					static BOOL	fVisible = TRUE;
					ShowWindow (GetDlgItem (hWnd, IDC_D1), fVisible ? SW_HIDE : SW_SHOW);
					fVisible = fVisible ? FALSE : TRUE;
					return (TRUE);
                }
				case IDC_HIDE4:
				{
					static BOOL	fVisible = TRUE;
					ShowWindow (GetDlgItem (hWnd, IDC_D1), fVisible ? SW_HIDE : SW_SHOW);
					fVisible = fVisible ? FALSE : TRUE;
					return (TRUE);
                }
				case IDC_HIDE5:
				{
					static BOOL	fVisible = TRUE;
					ShowWindow (GetDlgItem (hWnd, IDC_R1), fVisible ? SW_HIDE : SW_SHOW);
					fVisible = fVisible ? FALSE : TRUE;
					return (TRUE);
                }

				default:
				{
					break;
                }
			}

            break;
		}

		case SLN_NEW_VALUE:
		{
			switch (wParam)
			{
				case IDC_A1:
				{
					SendDlgItemMessage (hWnd, IDC_ENV1, ENV_SET_ATTACK, (WPARAM)(int)lParam, 0);
					return (TRUE);
				}
				case IDC_H1:
				{
					SendDlgItemMessage (hWnd, IDC_ENV1, ENV_SET_HOLD, (WPARAM)(int)lParam, 0);
					return (TRUE);
				}
				case IDC_D1:
				{
					SendDlgItemMessage (hWnd, IDC_ENV1, ENV_SET_DECAY, (WPARAM)(int)lParam, 0);
					return (TRUE);
				}
				case IDC_S1:
				{
					SendDlgItemMessage (hWnd, IDC_ENV1, ENV_SET_SUSTAIN, (WPARAM)(int)lParam, 0);
					return (TRUE);
				}
				case IDC_R1:
				{
					SendDlgItemMessage (hWnd, IDC_ENV1, ENV_SET_RELEASE, (WPARAM)(int)lParam, 0);
					return (TRUE);
				}
				case IDC_PIN1_SLIDER:
				{
					SendDlgItemMessage (hWnd, IDC_PIN1, PIN_SET_CURRENT, (WPARAM)(int)lParam, 0);
					return (TRUE);
				}
				default:
				{
					break;
                }
			}
			break;
		}

		default:
		{
			break;
        }
	}

	return (FALSE);
}

VOID CALLBACK _export DisplayValue (HWND hWnd, int nValue)
{
	char	szNumBuf[30];

	itoa (nValue, &szNumBuf[0], 10);
	SendMessage (hWnd, WM_SETTEXT, 0, (LPARAM)(LPCSTR)&szNumBuf[0]);
}
