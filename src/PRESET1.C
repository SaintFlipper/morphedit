/*****************************************************************************
 File:			preset1.c
 Description:	Module 1 for preset handling. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 16/3/94	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define	PATCH_WINDOW_WIDTH				600
#define	PATCH_WINDOW_HEIGHT				400

#define	MAX_MOD_SOURCES					4
#define	MAX_WIRES_PER_CONNECTION		2

// Preset object definitions
#define	OBJ_NONE						-1
#define OBJ_INSTRUMENT1					0
#define OBJ_INSTRUMENT2					1
#define	OBJ_PORTAMENTO1					2
#define	OBJ_PORTAMENTO2					3
#define	OBJ_TONE1						4
#define	OBJ_TONE2						5
#define OBJ_FILTER1						6
#define	OBJ_FILTER2						7
#define	OBJ_DCA1						8
#define	OBJ_DCA2						9
#define	OBJ_AHDSR1						10
#define	OBJ_AHDSR2						11
#define	OBJ_XFADE						12
#define	OBJ_PAN1						13
#define	OBJ_PAN2						14
#define OBJ_KEYBOARD					15
#define	OBJ_PITCH_WHEEL					16
#define	OBJ_MIDI_CONTROLLER				17
#define	OBJ_FREE_RUN_FG					18
#define	OBJ_LFO1						19
#define	OBJ_LFO2						20
#define	OBJ_AUX_ENV						21
#define	OBJ_FG1							22
#define	OBJ_FG2							23

/****************************************************************************/
/*								Local types									*/
/****************************************************************************/

typedef struct
{
	LPCSTR		lpBitmapName;				// Bitmap resource name
	HBITMAP		hBitmap;					// Bitmap handle
	int			nXpos;						// X position relative to preset edit window
	int			nYpos;						// Y position relative to preset edit window
	int			nWidth;						// Bitmap width in pixels
	int			nHeight;					// Bitmap height in pixels
	struct
	{
		UINT	nNumSources;				// Number of patchcord sources
		POINT	SourcePos[MAX_MOD_SOURCES];	// Patchcord source positions (relative to top-left)
		UINT	nNumDests;					// Number of patchcord destinations
		POINT	DestPos[MAX_MOD_SOURCES];	// Patchcord destination positions (relative to top-left)
	} Connections;
	POINT		AudioInPos;					// Audio input connection position
	POINT		AudioOutPos;				// Audio output connection position
} PRESET_OBJECT;

// Patch connection (audio or modulation)
typedef struct
{
	int		Output;			// Patch source (nb. for audio this is the OBJECT, not the type)
	int		Input;			// Patch destination
} PATCH;

// Patchcord source
typedef struct
{
	int		Object;			// Source object 
	int		Connector;		// Connector index within object structure
} PATCH_SOURCE;

// Patchcord destinations
typedef struct
{
	int		Type;			// Real-time (RT) OR Note-on (NO) OR Both (RTNO)
	int		nNumWires;		// Number of wires (because some go to both primary & secondary)
	struct
	{
		int	Object;			// Destination object 
		int	Connector;		// Connector index within object structure
	} Wire[MAX_WIRES_PER_CONNECTION];
} PATCH_DESTINATION;

// Line type
typedef enum
{
	BAND,
	SNAP
} LINE_TYPE;

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

PRESET_OBJECT PresetObject[] =
{
	{"BMP_INSTRUMENT1", 	(HBITMAP)NULL,	5, 5, 0, 0,
		{
			0, {0, 0},
			3, {{10, 50}, {25, 50}, {40, 50}},
		},
		{0, 0}, {50, 25}
	},
	{"BMP_INSTRUMENT2", 	(HBITMAP)NULL,	5, 125, 0, 0,
		{
			0, {0, 0},
			3, {{10, 50}, {25, 50}, {40, 50}},
		},
		{0, 0}, {50, 25}
	},
	{"BMP_PORTAMENTO",		(HBITMAP)NULL,	5, 65, 0, 0,
		{
			1, {25, 0},
			1, {25, 50},
		},
		{0, 0}, {50, 25}
	},
	{"BMP_PORTAMENTO",		(HBITMAP)NULL,	5, 185, 0, 0,
		{
			1, {25, 0},
			1, {25, 50},
		},
		{0, 0}, {50, 25}
	},
	{"BMP_TONE",			(HBITMAP)NULL,	125, 5, 0, 0,
		{
			0, {0, 0},
			1, {25, 50},
		},
		{0, 25}, {50, 25}
	},
	{"BMP_TONE",			(HBITMAP)NULL,	125, 125, 0, 0,
		{
			0, {0, 0},
			1, {25, 50},
		},
		{0, 25}, {50, 25}
	},
	{"BMP_ZPLANE_FILTER",	(HBITMAP)NULL,	245, 5, 0, 0,
		{
			0, {0, 0},
			4, {{10, 50}, {20, 50}, {30, 50}, {40, 50}},
		},
		{0, 25}, {50, 25}
	},
	{"BMP_ZPLANE_FILTER",	(HBITMAP)NULL,	245, 125, 0, 0,
		{
			0, {0, 0},
			4, {{10, 50}, {20, 50}, {30, 50}, {40, 50}},
		},
		{0, 25}, {50, 25}
	},
	{"BMP_DCA",				(HBITMAP)NULL,	365, 5, 0, 0,
		{
			0, {0, 0},
			3, {{10, 50}, {25, 50}, {40, 50}},
		},
		{0, 25}, {50, 25}
	},
	{"BMP_DCA",				(HBITMAP)NULL,	365, 125, 0, 0,
		{
			0, {0, 0},
			3, {{10, 50}, {25, 50}, {40, 50}},
		},
		{0, 25}, {50, 25}
	},
	{"BMP_VOLUME_AHDSR",	(HBITMAP)NULL,	365, 65, 0, 0,
		{
			1, {25, 0},
			3, {{10, 50}, {25, 50}, {40, 50}},
		},
		{0, 0}, {50, 25}
	},
	{"BMP_VOLUME_AHDSR",	(HBITMAP)NULL,	365, 185, 0, 0,
		{
			1, {25, 0},
			3, {{10, 50}, {25, 50}, {40, 50}},
		},
		{0, 0}, {50, 25}
	},
	{"BMP_XFADE",			(HBITMAP)NULL,	425, 65, 0, 0,
		{
			1, {25, 0},
			1, {25, 25},
		},
		{0, 0}, {50, 25}
	},
	{"BMP_PAN",				(HBITMAP)NULL,	485, 5, 0, 0,
		{
			0, {0, 0},
			1, {25, 50},
		},
		{0, 25}, {50, 25}
	},
	{"BMP_PAN",				(HBITMAP)NULL,	485, 125, 0, 0,
		{
			0, {0, 0},
			1, {25, 50},
		},
		{0, 25}, {50, 25}
	},
	{"BMP_KEYBOARD",		(HBITMAP)NULL,	5, 305, 0, 0,
		{
			4, {{10, 0}, {20, 0}, {30, 0}, {40, 0}},
			0, {0, 0},
		},
		{0, 0}, {50, 25}
	},
	{"BMP_PITCH_WHEEL",		(HBITMAP)NULL,	65, 305, 0, 0,
		{
			1, {25, 0},
			0, {0, 0},
		},
		{0, 0}, {50, 25}
	},
	{"BMP_MIDI_CONTROLLER",	(HBITMAP)NULL,	125, 305, 0, 0,
		{
			4, {{10, 0}, {20, 0}, {30, 0}, {40, 0}},
			0, {0, 0},
		},
		{0, 0}, {50, 25}
	},
	{"BMP_FREE_RUN_FG",		(HBITMAP)NULL,	185, 305, 0, 0,
		{
			1, {25, 0},
			0, {0, 0},
		},
		{0, 0}, {50, 25}
	},
	{"BMP_LFO1",			(HBITMAP)NULL,	245, 305, 0, 0,
		{
			1, {25, 0},
			2, {{15, 50}, {35, 50}},
		},
		{0, 0}, {50, 25}
	},
	{"BMP_LFO2",			(HBITMAP)NULL,	305, 305, 0, 0,
		{
			1, {25, 0},
			2, {{15, 50}, {35, 50}},
		},
		{0, 0}, {50, 25}
	},
	{"BMP_AUX_ENV",			(HBITMAP)NULL,	365, 305, 0, 0,
		{
			1, {25, 0},
			4, {{10, 50}, {20, 50}, {30, 50}, {40, 50}},
		},
		{0, 0}, {50, 25}
	},
	{"BMP_FG1",				(HBITMAP)NULL,	425, 305, 0, 0,
		{
			1, {25, 0},
			1, {25, 50},
		},
		{0, 0}, {50, 25}
	},
	{"BMP_FG2",				(HBITMAP)NULL,	485, 305, 0, 0,
		{
			1, {25, 0},
			1, {25, 50},
		},
		{0, 0}, {50, 25}
	},
};

static PATCH_SOURCE PatchNoteOnSource[] =
{
	{OBJ_KEYBOARD,			0},							// NOS_KEY
	{OBJ_KEYBOARD,			1},							// NOS_VELOCITY
	{OBJ_PITCH_WHEEL,		0},							// NOS_PITCHWHEEL
	{OBJ_MIDI_CONTROLLER,	0},							// NOS_CONTROL_A
	{OBJ_MIDI_CONTROLLER,	1},							// NOS_CONTROL_B
	{OBJ_MIDI_CONTROLLER,	2},							// NOS_CONTROL_C
	{OBJ_MIDI_CONTROLLER,	3},							// NOS_CONTROL_D
	{OBJ_KEYBOARD,			2},							// NOS_MONO_PRESSURE
	{OBJ_FREE_RUN_FG,		0},							// NOS_FREE_RUN_FG
};

static PATCH_SOURCE PatchRealTimeSource[] =
{
	{OBJ_PITCH_WHEEL,		0},							// RTS_PITCHWHEEL
	{OBJ_MIDI_CONTROLLER,	0},							// RTS_CONTROL_A
	{OBJ_MIDI_CONTROLLER,	1},							// RTS_CONTROL_B
	{OBJ_MIDI_CONTROLLER,	2},							// RTS_CONTROL_C
	{OBJ_MIDI_CONTROLLER,	3},							// RTS_CONTROL_D
	{OBJ_KEYBOARD,			2},							// RTS_MONO_PRESSURE
	{OBJ_KEYBOARD,			3},							// RTS_POLY_PRESSURE
	{OBJ_LFO1,				0},							// RTS_LFO1
	{OBJ_LFO2,				0},							// RTS_LFO2
	{OBJ_AUX_ENV,			0},							// RTS_AUX_ENV
	{OBJ_FG1,				0},							// RTS_FG1
	{OBJ_FG2,				0},							// RTS_FG2
	{OBJ_FREE_RUN_FG,		0},							// RTS_FREE_RUN_FG

	// Internal (fixed patches)
	{OBJ_PORTAMENTO1,		0},							// RTS_PORT_1
	{OBJ_PORTAMENTO2,		0},							// RTS_PORT_2
	{OBJ_AHDSR1,			0},							// RTS_AHDSR1
	{OBJ_AHDSR2,			0},							// RTS_AHDSR2
	{OBJ_XFADE,				0},							// RTS_XFADE
};

static PATCH_DESTINATION PatchDest[] =
{
	// Morpheus patch destinations 
	{RTNO,	1,	{{OBJ_NONE, 0},			{OBJ_NONE, 0}}},					// D_OFF
	{RTNO,	2,	{{OBJ_INSTRUMENT1, 1},	{OBJ_INSTRUMENT2, 1}}},				// D_PITCH
	{RTNO,	1,	{{OBJ_INSTRUMENT1, 1},	{OBJ_NONE, 0}}},					// D_PITCHP
	{RTNO,	1,	{{OBJ_INSTRUMENT2, 1},	{OBJ_NONE, 0}}},					// D_PITCHS
	{RTNO,	2,	{{OBJ_DCA1, 1},			{OBJ_DCA2, 1}}},					// D_VOLUME
	{RTNO,	1,	{{OBJ_DCA1, 1},			{OBJ_NONE, 0}}},					// D_VOLUMEP
	{RTNO,	1,	{{OBJ_DCA2, 1},			{OBJ_NONE, 0}}},					// D_VOLUMES
	{RTNO,	2,	{{OBJ_AHDSR1, 0},		{OBJ_AHDSR2, 0}}},					// D_ATTACK
	{RTNO,	1,	{{OBJ_AHDSR1, 0},		{OBJ_NONE, 0}}},					// D_ATTACKP
	{RTNO,	1,	{{OBJ_AHDSR2, 0},		{OBJ_NONE, 0}}},					// D_ATTACKS
	{RTNO,	2,	{{OBJ_AHDSR1, 1},		{OBJ_AHDSR2, 0}}},					// D_DECAY
	{RTNO,	1,	{{OBJ_AHDSR1, 1},		{OBJ_NONE, 0}}},					// D_DECAYP
	{RTNO,	1,	{{OBJ_AHDSR1, 1},		{OBJ_NONE, 0}}},					// D_DECAYS
	{RTNO,	2,	{{OBJ_AHDSR1, 2},		{OBJ_AHDSR2, 0}}},					// D_RELEASE
	{RTNO,	1,	{{OBJ_AHDSR1, 2},		{OBJ_NONE, 0}}},					// D_RELEASEP
	{RTNO,	1,	{{OBJ_AHDSR1, 2},		{OBJ_NONE, 0}}},					// D_RELEASES
	{RTNO,	1,	{{OBJ_XFADE, 0},		{OBJ_NONE, 0}}},					// D_XFADE
	{RTNO,	1,	{{OBJ_LFO1, 0},			{OBJ_NONE, 0}}},					// D_LFO1_AMOUNT
	{RTNO,	1,	{{OBJ_LFO1, 1},			{OBJ_NONE, 0}}},					// D_LFO1_RATE
	{RTNO,	1,	{{OBJ_LFO2, 0},			{OBJ_NONE, 0}}},					// D_LFO2_AMOUNT
	{RTNO,	1,	{{OBJ_LFO2, 1},			{OBJ_NONE, 0}}},					// D_LFO2_RATE
	{RTNO,	1,	{{OBJ_AUX_ENV, 0},		{OBJ_NONE, 0}}},					// D_AUX_AMOUNT
	{RTNO,	1,	{{OBJ_AUX_ENV, 1},		{OBJ_NONE, 0}}},					// D_AUX_ATTACK
	{RTNO,	1,	{{OBJ_AUX_ENV, 2},		{OBJ_NONE, 0}}},					// D_AUX_DECAY
	{RTNO,	1,	{{OBJ_AUX_ENV, 3},		{OBJ_NONE, 0}}},					// D_AUX_RELEASE
	{NO,	2,	{{OBJ_INSTRUMENT1, 2},	{OBJ_INSTRUMENT2, 2}}},				// D_START
	{NO,	1,	{{OBJ_INSTRUMENT1, 2},	{OBJ_NONE, 0}}},					// D_STARTP
	{NO,	1,	{{OBJ_INSTRUMENT2, 2},	{OBJ_NONE, 0}}},					// D_STARTS
	{RTNO,	2,	{{OBJ_PAN1, 0},			{OBJ_PAN2, 0}}},					// D_PAN
	{RTNO,	1,	{{OBJ_PAN1, 0},			{OBJ_NONE, 0}}},					// D_PANP
	{RTNO,	1,	{{OBJ_PAN2, 0},			{OBJ_NONE, 0}}},					// D_PANS
	{NO,	2,	{{OBJ_TONE1, 0},		{OBJ_TONE2, 0}}},					// D_TONE
	{NO,	1,	{{OBJ_TONE1, 0},		{OBJ_NONE, 0}}},					// D_TONEP
	{NO,	1,	{{OBJ_TONE2, 0},		{OBJ_NONE, 0}}},					// D_TONES
	{RTNO,	2,	{{OBJ_FILTER1, 1},		{OBJ_FILTER2, 1}}},					// D_MORPH
	{RTNO,	1,	{{OBJ_FILTER1, 1},		{OBJ_NONE, 0}}},					// D_MORPH_P
	{RTNO,	1,	{{OBJ_FILTER2, 1},		{OBJ_NONE, 0}}},					// D_MORPH_S
	{NO,	2,	{{OBJ_FILTER1, 3},		{OBJ_FILTER2, 3}}},					// D_TRANS2
	{NO,	1,	{{OBJ_FILTER1, 3},		{OBJ_NONE, 0}}},					// D_TRANS2_P
	{NO,	1,	{{OBJ_FILTER2, 3},		{OBJ_NONE, 0}}},					// D_TRANS2_S
	{RTNO,	2,	{{OBJ_PORTAMENTO1, 0},	{OBJ_PORTAMENTO2, 0}}},				// D_PORT_RATE
	{RTNO,	1,	{{OBJ_PORTAMENTO1, 0},	{OBJ_NONE, 0}}},					// D_PORT_RATE_P
	{RTNO,	1,	{{OBJ_PORTAMENTO2, 0},	{OBJ_NONE, 0}}},					// D_PORT_RATE_S
	{RTNO,	1,	{{OBJ_FG1, 0},			{OBJ_NONE, 0}}},					// D_FG1_AMOUNT
	{RTNO,	1,	{{OBJ_FG2, 0},			{OBJ_NONE, 0}}},					// D_FG2_AMOUNT
	{NO,	2,	{{OBJ_FILTER1, 0},		{OBJ_FILTER2, 0}}},					// D_FILTER_LEVEL
	{NO,	1,	{{OBJ_FILTER1, 0},		{OBJ_NONE, 0}}},					// D_FILTER_LEVEL_P
	{NO,	1,	{{OBJ_FILTER2, 0},		{OBJ_NONE, 0}}},					// D_FILTER_LEVEL_S
	{NO,	2,	{{OBJ_FILTER1, 2},		{OBJ_FILTER2, 2}}},					// D_FREQ_TRACK
	{NO,	1,	{{OBJ_FILTER1, 2},		{OBJ_NONE, 0}}},					// D_FREQ_TRACK_P
	{NO,	1,	{{OBJ_FILTER2, 2},		{OBJ_NONE, 0}}},					// D_FREQ_TRACK_S

	// Internal patch destinations (for fixed routings)
	{RT,	1,	{{OBJ_INSTRUMENT1, 0},	{OBJ_NONE, 0}}},					// D_INSTR1_PORT
	{RT,	1,	{{OBJ_INSTRUMENT2, 0},	{OBJ_NONE, 0}}},					// D_INSTR2_PORT
	{RT,	1,	{{OBJ_DCA1, 0},			{OBJ_NONE, 0}}},					// D_DCA1_VOL
	{RT,	1,	{{OBJ_DCA2, 0},			{OBJ_NONE, 0}}},					// D_DCA2_VOL
	{RT,	2,	{{OBJ_DCA1, 2},			{OBJ_DCA2, 2}}},					// D_DCA_XFADE
};

/*
	List of fixed audio connections
 */
static PATCH FixedAudioConnections[] =
{
	{OBJ_INSTRUMENT1,	OBJ_TONE1},
	{OBJ_TONE1,			OBJ_FILTER1},
	{OBJ_FILTER1,		OBJ_DCA1},
	{OBJ_DCA1,			OBJ_PAN1},
	{OBJ_INSTRUMENT2,	OBJ_TONE2},
	{OBJ_TONE2,			OBJ_FILTER2},
	{OBJ_FILTER2,		OBJ_DCA2},
	{OBJ_DCA2,			OBJ_PAN2},
};

/*
	List of fixed real-time patches
 */
static PATCH FixedRealTimePatches[] =
{
	{RTS_PORT_1,		D_INSTR1_PORT},
	{RTS_PORT_2,		D_INSTR2_PORT},
	{RTS_AHDSR1,		D_DCA1_VOL},
	{RTS_AHDSR2,		D_DCA2_VOL},
	{RTS_XFADE,			D_DCA_XFADE},
};

static HPEN		hAudioPatchPen;			// Used for (hardwired) audio patchcords
static HPEN		hFixedPatchPen;			// Used for hardwired patchcords
static HPEN		hNoteOnPatchPen[NUM_NOTE_ON_PATCHES];
static HPEN		hRealTimePatchPen[NUM_REAL_TIME_PATCHES];
static COLORREF	PenColours[NUM_REAL_TIME_PATCHES] =
{
	RGB (255, 0, 0),
	RGB (0, 255, 0),
	RGB (0, 0, 255),
	RGB (128, 0, 0),
	RGB (0, 128, 0),
	RGB (0, 0, 128),
	RGB (255, 255, 0),
	RGB (0, 128, 128),
	RGB (128, 128, 0),
	RGB (0, 255, 255),
};

/****************************************************************************/
/*								Global data									*/
/****************************************************************************/

// Currently displayed patch
int nDisplayedPatch = NO_PARAM;

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

static VOID PresetPatchesInit (VOID);
static VOID DrawPresetPatchesWindow (PPRESET_PARAMS pPreset, HDC hDC);

static VOID DrawObjects (HDC hDC);
static VOID DrawFixedAudioConnections (HDC hDC);
static VOID DrawFixedPatches (HDC hDC);
static VOID DrawNoteOnPatches (PPRESET_PARAMS pPreset, HDC hDC);
static VOID DrawRealTimePatches (PPRESET_PARAMS pPreset, HDC hDC);

static VOID DrawNoteOnPatch (HDC hDC, HPEN hPen, int Source, int Dest, LINE_TYPE Ltype);
static VOID DrawRealTimePatch (HDC hDC, HPEN hPen, int Source, int Dest, LINE_TYPE Ltype);

/*^L*/
/*****************************************************************************
 Function:		DrawPatches
 Parameters:	int		nPreset			Preset to draw.
 Returns:		None.
 Description:	Draws the specified preset's patches.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/5/94 	| Created.											| PW
*****************************************************************************/

VOID DrawPatches (int nPreset)
{
	// Store the preset number
	nDisplayedPatch = nPreset;

	// Force a redraw
	InvalidateRect (hwndPresetPatches, NULL, TRUE);
}

/*^L*/
/*****************************************************************************
 Function:		PresetPatchesWindowProc
 Parameters:	HWND	hWnd
				UINT	nMsg
				WPARAM	wParam
				LPARAM	lParam
 Returns:		LRESULT					Window proc return value.
 Description:	The preset patchcords window procedure.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 10/4/94 	| Created.											| PW
*****************************************************************************/

LRESULT CALLBACK _export PresetPatchesWindowProc (HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	UINT	i;
	HMENU	hMenu;

	switch (nMsg)
	{
		// Creation
		case WM_CREATE:
		{
			// Do the necessary initialisation for the preset patches window
			MY_CREATE_PROC;
			PresetPatchesInit ();

			// Make myself exactly the right size
			MoveWindow (hWnd, 0, 0, PATCH_WINDOW_WIDTH, PATCH_WINDOW_HEIGHT, FALSE);
			SetMainScroll ();

			// Remove lost of things from the system menu
			hMenu = GetSystemMenu (hWnd, FALSE);
			DeleteMenu (hMenu, SC_SIZE, MF_BYCOMMAND);
			DeleteMenu (hMenu, SC_MAXIMIZE, MF_BYCOMMAND);
			DeleteMenu (hMenu, SC_CLOSE, MF_BYCOMMAND);

			return (0);
		}

		// Trying to move the window
		case WM_WINDOWPOSCHANGING:
		{
			LPWINDOWPOS	lpWindowPos;

			lpWindowPos = (LPWINDOWPOS)lParam;

			// Don't let it go off the left of the parent window
			if (lpWindowPos->x < 0)
			{
				lpWindowPos->x = 0;
			}

			// Don't let it go off the top of the parent window
			if (lpWindowPos->y < 0)
			{
				lpWindowPos->y = 0;
			}

			return (TRUE);
		}


		// Want to re-size
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO FAR	*mmi = (MINMAXINFO FAR *)lParam;

			// Don't let it vary in size at all
			mmi->ptMaxSize.x		= PATCH_WINDOW_WIDTH;
			mmi->ptMaxSize.y		= PATCH_WINDOW_HEIGHT;
			mmi->ptMaxPosition.x	= 0;
			mmi->ptMaxPosition.y	= 0;
			mmi->ptMinTrackSize.x	= PATCH_WINDOW_WIDTH;
			mmi->ptMinTrackSize.y	= PATCH_WINDOW_HEIGHT;
			mmi->ptMaxTrackSize.x	= PATCH_WINDOW_WIDTH;
			mmi->ptMaxTrackSize.y	= PATCH_WINDOW_HEIGHT;
            return (0);
		}

		case WM_LBUTTONDOWN:
		{
			SetWindowPos (hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			break;
        }

		// Repaint
		case WM_PAINT:
		{
			HDC			hDC;
			PAINTSTRUCT	PaintStruct;
            RECT		UpdateRect;

			if (GetUpdateRect (hWnd, &UpdateRect, TRUE))
            {
				hDC = BeginPaint (hWnd, &PaintStruct);
				DrawPresetPatchesWindow (nDisplayedPatch == NO_PARAM ?
					(PPRESET_PARAMS)NULL : pMorpheusPresets[nDisplayedPatch], hDC);
				EndPaint (hWnd, &PaintStruct);
				return (0);
			}
			break;
		}

		// Close
		case WM_CLOSE:
        {
			break;
		}

		// Destructopm
		case WM_DESTROY:
		{
			// Unload the bitmaps
			for (i=0 ; i<ARRAY_SIZE(PresetObject) ; i++)
			{
				DeleteObject (PresetObject[i].hBitmap);
			}

			// Delete the pens
			DeleteObject (hAudioPatchPen);
			DeleteObject (hFixedPatchPen);
			for (i=0 ; i<NUM_REAL_TIME_PATCHES ; i++)
			{
				DeleteObject (hRealTimePatchPen[i]);
			}
			for (i=0 ; i<NUM_NOTE_ON_PATCHES ; i++)
			{
				DeleteObject (hNoteOnPatchPen[i]);
			}

        	return (0);
        }

		default:
		{
			break;
		}
	}

	// Default processing
	return (MY_DEFAULT_WINDOW_PROC (hWnd, nMsg, wParam, lParam));
}

/*^L*/
/*****************************************************************************
 Function:		PresetPatchesInit
 Parameters:	None.
 Returns:		None.
 Description:	Performs all necessary preset window initialisation.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 24/3/94 	| Created.											| PW
*****************************************************************************/

static VOID PresetPatchesInit (VOID)
{
	UINT	i;
	BITMAP	bm;

	/*
		Load the bitmaps for drawing the preset objects.
	 */

	// For each object
	for (i=0 ; i<ARRAY_SIZE(PresetObject) ; i++)
	{
		// Load its bitmap
		PresetObject[i].hBitmap = LoadBitmap (hInst, PresetObject[i].lpBitmapName);

		// Get its dimensions
		GetObject (PresetObject[i].hBitmap, sizeof (BITMAP), (LPSTR)&bm);
		PresetObject[i].nWidth = bm.bmWidth;
		PresetObject[i].nHeight = bm.bmHeight;
	}

	/*
		Create the pens for drawing connections.
	 */

	// Use a WHITE pen for fixed audio links
	hAudioPatchPen	= CreatePen (PS_SOLID, 2, RGB (255, 255, 255));

	// Use a BLACK pen for fixed real-time patchcords
	hFixedPatchPen	= CreatePen (PS_SOLID, 2, RGB (0, 0, 0));

	// Use solid coloured pens for the real-time patchcords
	for (i=0 ; i<NUM_REAL_TIME_PATCHES ; i++)
	{
		hRealTimePatchPen[i] = CreatePen (PS_SOLID, 1, PenColours[i]);
	}

	// Use dotted coloured pens for the real-time patchcords
	for (i=0 ; i<NUM_NOTE_ON_PATCHES ; i++)
	{
		hNoteOnPatchPen[i] = CreatePen (PS_DASH, 1, PenColours[i]);
	}
}

/*^L*/
/*****************************************************************************
 Function:		DrawPresetPatchesWindow
 Parameters:	PPRESET_PARAMS	pPreset	Preset to display.
				HDC				hDC		Display context to put it in.
 Returns:		None.
 Description:	Displays the specified preset in the specified window.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 24/3/94 	| Created.											| PW
*****************************************************************************/
 
static VOID DrawPresetPatchesWindow (PPRESET_PARAMS pPreset, HDC hDC)
{
	// Draw the objects
	DrawObjects (hDC);

	// Draw fixed audio connections
	DrawFixedAudioConnections (hDC);

	// Draw fixed patches
	DrawFixedPatches (hDC);

	// Draw patches
	if (pPreset != (PPRESET_PARAMS)NULL)
	{
		DrawNoteOnPatches (pPreset, hDC);
		DrawRealTimePatches (pPreset, hDC);
	}
}

/*^L*/
/*****************************************************************************
 Function:		DrawObjects
 Parameters:	HDC			hDC			Display context to put it in.
 Returns:		None.
 Description:	Draws all preset objects.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 25/3/94 	| Created.											| PW
*****************************************************************************/

static VOID DrawObjects (HDC hDC)
{
	UINT	i;

	// For each object
	for (i=0 ; i<ARRAY_SIZE(PresetObject) ; i++)
	{
		// Display the bitmap
		DrawBitmap (hDC, PresetObject[i].hBitmap, PresetObject[i].nXpos, PresetObject[i].nYpos);
	}
}

/*^L*/
/*****************************************************************************
 Function:		DrawFixedAudioConnections
 Parameters:	HDC			hDC			Display context to put it in.
 Returns:		None.
 Description:	Draws all hardwired audio connections.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 25/3/94 	| Created.											| PW
*****************************************************************************/

static VOID DrawFixedAudioConnections (HDC hDC)
{
	UINT	i;
	HPEN	hOldPen;
    int		o1, o2;

	// Select the audio connection pen
	hOldPen = SelectObject (hDC, hAudioPatchPen);

	// For each audio connection
	for (i=0 ; i<ARRAY_SIZE(FixedAudioConnections) ; i++)
	{
		// Go to the audio output
		o1 = FixedAudioConnections[i].Output;
		MoveTo (hDC,
				PresetObject[o1].nXpos + PresetObject[o1].AudioOutPos.x,
				PresetObject[o1].nYpos + PresetObject[o1].AudioOutPos.y);

		// Draw to the audio input
		o2 = FixedAudioConnections[i].Input;
		LineTo (hDC,
				PresetObject[o2].nXpos + PresetObject[o2].AudioInPos.x,
				PresetObject[o2].nYpos + PresetObject[o2].AudioInPos.y);
	}

	// Select the old pen
	SelectObject (hDC, hOldPen);
}

/*^L*/
/*****************************************************************************
 Function:		DrawNoteOnPatches
 Parameters:	PPRESET_PARAMS	pPreset	Preset being displayed.
				HDC				hDC		Display context to put it in.
 Returns:		None.
 Description:	Draws all note-on audio connections.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/5/94 	| Created.											| PW
*****************************************************************************/

static VOID DrawNoteOnPatches (PPRESET_PARAMS pPreset, HDC hDC)
{
	UINT	i;

	// For each note-on patch
	for (i=0 ; i<NUM_NOTE_ON_PATCHES ; i++)
	{
		// Draw the patchcord
		DrawNoteOnPatch (hDC, hNoteOnPatchPen[i], pPreset->NoteOnModulation[i].Source,
							pPreset->NoteOnModulation[i].Destination, SNAP);
	}
}

/*^L*/
/*****************************************************************************
 Function:		DrawRealTimePatches
 Parameters:	PPRESET_PARAMS	pPreset	Preset being displayed.
				HDC				hDC		Display context to put it in.
 Returns:		None.
 Description:	Draws all real-time audio connections.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 6/5/94 	| Created.											| PW
*****************************************************************************/

static VOID DrawRealTimePatches (PPRESET_PARAMS pPreset, HDC hDC)
{
	UINT	i;

	// For each real-time patch
	for (i=0 ; i<NUM_REAL_TIME_PATCHES ; i++)
	{
		// Draw the patchcord
		DrawRealTimePatch (hDC, hRealTimePatchPen[i], pPreset->RealTimeModulation[i].Source,
							pPreset->RealTimeModulation[i].Destination, SNAP);
	}
}

/*^L*/
/*****************************************************************************
 Function:		DrawFixedPatches
 Parameters:	HDC			hDC			Display context to put it in.
 Returns:		None.
 Description:	Draws all hardwired patches.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 25/3/94 	| Created.											| PW
*****************************************************************************/

static VOID DrawFixedPatches (HDC hDC)
{
	UINT	i;

	// For each fixed patch
	for (i=0 ; i<ARRAY_SIZE(FixedRealTimePatches) ; i++)
	{
		// Draw the patchcord
		DrawRealTimePatch (hDC, hFixedPatchPen, FixedRealTimePatches[i].Output,
							FixedRealTimePatches[i].Input, SNAP);
	}
}

/*^L*/
/*****************************************************************************
 Function:		DrawNoteOnPatch
 Parameters:	HDC			hDC			Display context to put it in.
				HPEN		hPen		Pen to draw with.
				int			Source		Note-on patch source index.
				int			Dest		Patchcord destination index.
				LINE_TYPE	Ltype		Line type (rubber band or snap to grid).
 Returns:		None.
 Description:	Draws a single note-on patchcord.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/3/94 	| Created.											| PW
*****************************************************************************/

static VOID DrawNoteOnPatch (HDC hDC, HPEN hPen, int Source, int Dest, LINE_TYPE Ltype)
{
	int		os, od, cs, cd, xs, ys, xd, yd;
	int		i, nNumWires;
	HPEN	hOldPen;

	// Do nothing if destination is OFF
	if (Dest == D_OFF)
	{
    	return;
	}

	// Get the object and connector index of the source
	os = PatchNoteOnSource[Source].Object;
	cs = PatchNoteOnSource[Source].Connector;
	 
	// Get the (x,y) of the source connector
	xs = PresetObject[os].nXpos + PresetObject[os].Connections.SourcePos[cs].x;
	ys = PresetObject[os].nYpos + PresetObject[os].Connections.SourcePos[cs].y;

	// For each wire to be drawn (some patches go to 2 destinations)
	nNumWires = PatchDest[Dest].nNumWires;
	for (i=0 ; i<nNumWires ; i++)
	{
		// Get the object and connector index of the wire destination
		od = PatchDest[Dest].Wire[i].Object;
		cd = PatchDest[Dest].Wire[i].Connector;

		// Get the (x,y) of the wire destination
		xd = PresetObject[od].nXpos + PresetObject[od].Connections.DestPos[cd].x;
		yd = PresetObject[od].nYpos + PresetObject[od].Connections.DestPos[cd].y;

		// Connect the top of the source to the bottom of the destination
		if (Ltype == SNAP)
		{
			ConnectTopToBottom (hDC, hPen, xs, ys, xd, yd);
		}
		else
		{
			hOldPen = SelectObject (hDC, hPen);
			MoveToEx (hDC, xs, ys, NULL);
			LineTo (hDC, xd, yd);
			SelectObject (hDC, hOldPen);
		}
	}
}

/*^L*/
/*****************************************************************************
 Function:		DrawRealTimePatch
 Parameters:	HDC			hDC			Display context to put it in.
				HPEN		hPen		Pen to draw with.
				int			Source		Real-time patch source index.
				int			Dest		Patchcord destination index.
				LINE_TYPE	Ltype		Line type (rubber band or snap to grid).
 Returns:		None.
 Description:	Draws a single real-time patchcord.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 26/3/94 	| Created.											| PW
*****************************************************************************/

static VOID DrawRealTimePatch (HDC hDC, HPEN hPen, int Source, int Dest, LINE_TYPE Ltype)
{
	int		os, od, cs, cd, xs, ys, xd, yd;
	int		i, nNumWires;
	HPEN	hOldPen;

	// Do nothing if destination is OFF
	if (Dest == D_OFF)
	{
    	return;
	}

	// Get the object and connector index of the source
	os = PatchRealTimeSource[Source].Object;
	cs = PatchRealTimeSource[Source].Connector;
	 
	// Get the (x,y) of the source connector
	xs = PresetObject[os].nXpos + PresetObject[os].Connections.SourcePos[cs].x;
	ys = PresetObject[os].nYpos + PresetObject[os].Connections.SourcePos[cs].y;

	// For each wire to be drawn (some patches go to 2 destinations)
	nNumWires = PatchDest[Dest].nNumWires;
	for (i=0 ; i<nNumWires ; i++)
	{
		// Get the object and connector index of the wire destination
		od = PatchDest[Dest].Wire[i].Object;
		cd = PatchDest[Dest].Wire[i].Connector;

		// Get the (x,y) of the wire destination
		xd = PresetObject[od].nXpos + PresetObject[od].Connections.DestPos[cd].x;
		yd = PresetObject[od].nYpos + PresetObject[od].Connections.DestPos[cd].y;

		// Connect the top of the source to the bottom of the destination
		if (Ltype == SNAP)
		{
			ConnectTopToBottom (hDC, hPen, xs, ys, xd, yd);
		}
		else
		{
			hOldPen = SelectObject (hDC, hPen);
			MoveToEx (hDC, xs, ys, NULL);
			LineTo (hDC, xd, yd);
			SelectObject (hDC, hOldPen);
		}
	}
}

/**/
/***************************************************************************
Function:		DrawBitmap
Parameters:		HDC		hDC				Device context to draw into.
				HBITMAP	hBitmap			Loaded bitmap handle.
				int		x				X position to display bitmap at.
				int		y				Y position to display bitmap at.
Return value:	None.
Description:	Displays a bitmap.
History:

Date		| Description										| Name
------------+---------------------------------------------------+-----
24/4/93		| Created.											| PW
***************************************************************************/

VOID DrawBitmap (HDC hDC, HBITMAP hBitmap, int x, int y)
{
	BITMAP	bm;
	HDC		hdcMem;
	POINT	ptSize, ptOrg;

	hdcMem = CreateCompatibleDC (hDC);
	SelectObject (hdcMem, hBitmap);
	SetMapMode (hdcMem, GetMapMode (hDC));

	GetObject (hBitmap, sizeof (BITMAP), (LPSTR)&bm);
	ptSize.x = bm.bmWidth;
	ptSize.y = bm.bmHeight;
	DPtoLP (hDC, &ptSize, 1);

	ptOrg.x = 0;
	ptOrg.y = 0;
	DPtoLP (hdcMem, &ptOrg, 1);

	BitBlt (hDC, x, y, ptSize.x, ptSize.y, hdcMem, ptOrg.x, ptOrg.y, SRCCOPY);

	DeleteDC (hdcMem);
}
