/*****************************************************************************
 File:			funcgen.c
 Description:	Handling for the function generators.
				It would be nice if there was more common FG code, but it's
				a bit difficult due to the slight differences between the
				PRESET and HYPERPRESETs. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 30/5/94	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

static LPCSTR pszFGShapes[] =
			{"Linear",	"Expo+1",	"Expo+2",	"Expo+3",	"Expo+4",	"Expo+5",	"Expo+6",	"Expo+7",
			 "Circ1.4",	"Circ1.6",	"Circ1.8",	"Circ1.16",	"Squeeze",	"FastLn1",	"FastLn2",	"FastLn3",
			 "MedLn1",	"MedLn2",	"SlwRmp1",	"SlwRmp2",	"Bloom",	"Bloom2",	"Cr1.16R",	"Cir1.8R",
			 "Cir1.6R",	"Cir1.4R",	"SlwCrv1",	"SlwCrv2",  "DelayDC",  "DCdelay",	"Curve2X",	"Curv2XB",
			 "Curv2XC",	"ZigZag1",	"ZigZag2",	"ZigZag3",	"Chaos03",	"Chaos06",	"Chaos12",	"Chaos16",
			 "Chaos25",	"Chaos33",	"Chaos37",	"Chaos50",	"Chaos66",	"Chaos75",	"Chaos95",	"Chaos99",
			 "LinShf1",	"LnShfl2",	"RandomA",	"RandomB",	"RandomC",	"RandomD",	"RandomE",	"RandomF",
			 "RandomG",	"RandomH",	"RandomI",	"RandomJ",	"RandomK",	"RandomL",	"RandomZ"};
static LPCSTR pszFGConditions[] =
			{"Never",				"AlwaysEnd",			"NoteOnEnd",			"NoteOnImm",
			 "NoteOffEnd",			"NoteOffImm",			"LFO1End",				"LFO2End",
			 "Footswitch1End",		"Footswitch1Imm",		"Footswitch2End",		"Footswitch2Imm",
			 "Footswitch3End",		"Footswitch3Imm",		"VelocityEnd",			"KeyEnd"};

/*^L*/
/*****************************************************************************
 Function:		DisplayFuncGenList
 Parameters:	HWND		hWnd		Listbox window handle.
				PFG_PARAMS	pFgParams	Function generator parameters.
 Returns:		None.
 Description:	Formats a function generator and fills a listbox with it. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 30/5/94		| Created.											| PW
*****************************************************************************/

VOID DisplayFuncGenList (HWND hWnd, PFG_PARAMS pFgParams)
{
	UINT			i;
	char			szLineBuf[200];
	static	LPCSTR	pszLevelTypes[] = {"", "\x80", "\x81", "\x82"};
	LPCSTR			pLevelType;
	int				nLevel;

	// Clear the list
	SendMessage (hWnd, LB_RESETCONTENT, 0, 0);

	// For each segment (of 8) 
	for (i=0 ; i<8 ; i++)
	{
		// Decode the segment level
		nLevel = WNUM (pFgParams->Segment[i].Level);
		pLevelType = pszLevelTypes[(nLevel >> 8) & 3];
		nLevel = (int)(signed char)nLevel;

		// Format the whole line in one go
		wsprintf (
					&szLineBuf[0], "%d\t%s%d\t%d\t%s\t%s\t%d\t%d",
					i + 1,
					pLevelType, nLevel,
					WNUM (pFgParams->Segment[i].Time),
					pszFGShapes[WNUM (pFgParams->Segment[i].Shape)],
					pszFGConditions[WNUM (pFgParams->Segment[i].CondJump)],
					WNUM (pFgParams->Segment[i].CondValue),
					WNUM (pFgParams->Segment[i].DestSegment) + 1
				);

		// Put it in the listbox
		SendMessage (hWnd, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)&szLineBuf[0]);
	}
}

/*^L*/
/*****************************************************************************
 Function:		FGShapeName
 Parameters:	int		n				Shape index.
 Returns:		LPCSTR					Name for that shape.
 Description:	Returns the name for an FG segment shape. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 30/5/94		| Created.											| PW
*****************************************************************************/

LPCSTR FGShapeName (int n)
{
	return (pszFGShapes[n]);
}

/*^L*/
/*****************************************************************************
 Function:		FGConditionName
 Parameters:	int		n				Condition index.
 Returns:		LPCSTR					Name for that condition index.
 Description:	Returns the condition name for an FG segment condition. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 30/5/94		| Created.											| PW
*****************************************************************************/

LPCSTR FGConditionName (int n)
{
	return (pszFGConditions[n]);
}

