/*****************************************************************************
 File:			connect.c
 Description:	Functions for connecting objects with lines. 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 21/3/94	| Created.											| PW
*****************************************************************************/

#include "morpheus.h"

/****************************************************************************/
/*								Local defines								*/
/****************************************************************************/

#define	GRID_SPACING				60

#define	NUM_CONNECTION_TYPES		8
#define	MAX_POINTS_IN_CONNECTION	6
#define	P1							1
#define	P1_PLUS						2
#define	P1_MINUS					3
#define	P2							4
#define	P2_PLUS						5
#define	P2_MINUS					6

/****************************************************************************/
/*								Local data									*/
/****************************************************************************/

// All possible connection types
static struct
{
	UINT	nNumPoints;							// Number of points forming the connection path
	POINT	Point[MAX_POINTS_IN_CONNECTION];	// Points forming the connection
} ConnectionPath[NUM_CONNECTION_TYPES] =
{
	{6, {{P1, P1}, {P1, P1_MINUS}, {P2_MINUS, P1_MINUS}, {P2_MINUS, P2_PLUS}, {P2, P2_PLUS}, {P2, P2}}},
	{4, {{P1, P1}, {P1, P2_PLUS}, {P2, P2_PLUS}, {P2, P2}}},
	{2,	{{P1, P1}, {P2, P2}}},
	{4, {{P1, P1}, {P1, P2_PLUS}, {P2, P2_PLUS}, {P2, P2}}},
	{6, {{P1, P1}, {P1, P1_MINUS}, {P2_PLUS, P1_MINUS}, {P2_PLUS, P2_PLUS}, {P2, P2_PLUS}, {P2, P2}}},
	{6, {{P1, P1}, {P1, P1_MINUS}, {P2_PLUS, P1_MINUS}, {P2_PLUS, P2_PLUS}, {P2, P2_PLUS}, {P2, P2}}},
	{6, {{P1, P1}, {P1, P1_MINUS}, {P2_MINUS, P1_MINUS}, {P2_MINUS, P2_PLUS}, {P2, P2_PLUS}, {P2, P2}}},
	{6, {{P1, P1}, {P1, P1_MINUS}, {P2_MINUS, P1_MINUS}, {P2_MINUS, P2_PLUS}, {P2, P2_PLUS}, {P2, P2}}},
};

/****************************************************************************/
/*								Local functions								*/
/****************************************************************************/

static int FindConnectionType (int x1, int y1, int x2, int y2);
static VOID GetPoint (int iXtype, int iYtype, int *pX, int *pY, int x1, int y1, int x2, int y2);

/*^L*/
/*****************************************************************************
 Function:		ConnectTopToBottom
 Parameters:	HDC			hDC				Device context to draw in
				HPEN		hPen			Pen to draw the line with
				int			xt				X of first object's bottom
				int			yt				Y of first object's bottom
				int			xb				X of second object's bottom
				int			yb				Y of second object's bottom
 Returns:		None.
 Description:	Draws a series of lines that connect the top of the first
				object to the bottom of the second.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 21/3/94 	| Created.											| PW
*****************************************************************************/

VOID ConnectTopToBottom (HDC hDC, HPEN hPen, int x1, int y1, int x2, int y2)
{
	HPEN	hOldPen;
	int		nType;
	UINT	i, nNumPoints;
	int		x, y;

	// Select the pen
	hOldPen = SelectObject (hDC, hPen);

	// Find which direction the connection is
	nType = FindConnectionType (x1, y1, x2, y2);

	// Set the starting point
	GetPoint (ConnectionPath[nType].Point[0].x, ConnectionPath[nType].Point[0].y,
				&x, &y, x1, y1, x2, y2);
	MoveTo (hDC, x, y);

	// For every point
	nNumPoints = ConnectionPath[nType].nNumPoints;
	for (i=1 ; i<nNumPoints ; i++)
	{
    	// Find that point
		GetPoint (ConnectionPath[nType].Point[i].x, ConnectionPath[nType].Point[i].y,
				&x, &y, x1, y1, x2, y2);

		// Draw a line to it
		LineTo (hDC, x, y);
	}

    // Re-select the previous pen
	SelectObject (hDC, hOldPen);
}

/*^L*/
/*****************************************************************************
 Function:		FindConnectionType
 Parameters:	int			x1
				int			y1
				int			x2
				int			y2
 Returns:		UINT					Connection direction (type).
 Description:	Finds which connection type this is. The connection type
				is the connection direction, with type 0 being 0 left to right,
				type 1 in the 1st quadrant, etc. (clockwise). 
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/3/94 	| Created.											| PW
*****************************************************************************/

static int FindConnectionType (int x1, int y1, int x2, int y2)
{
	if (y2 == y1 && x2 > x1)
	{
		return (0);						// 0 degrees
	}
	else if (y2 < y1 && x2 > x1)
	{
		return (1);						// First quadrant
	}
	else if (y2 < y1 && x2 == x1)
	{
		return (2);						// 90 degrees
	}
	else if (y2 < y1 && x2 < x1)
	{
		return (3);						// Second quadrant
	}
	else if (y2 == y1 && x2 < x1)
	{
		return (4);						// 180 degrees
	}
	else if (y2 > y1 && x2 < x1)
	{
		return (5);						// Third quadrant
	}
	else if (y2 > y1 && x2 == x1)
	{
		return (6);						// 270 degrees
	}
	else if (y2 > y1 && x2 > x1)
	{
		return (7);						// Fourth quadrant
	}

	// Error
	assert (FALSE);
	return (-1);
}

/*^L*/
/*****************************************************************************
 Function:		GetPoint
 Parameters:	int	iXtype              X relative
				int	iytype				Y relative
				int	*pX					Output X
				int	*pY					Output Y
				int	x1
				int	y1
				int	x2
				int	y2
 Returns:		None.
 Description:	Calculates a point position from the placement relative to
				point 1 and point.
 History:
 Date		| Description										| Name
 -----------+---------------------------------------------------+------
 22/3/94 	| Created.											| PW
*****************************************************************************/

static VOID GetPoint (int iXtype, int iYtype, int *pX, int *pY, int x1, int y1, int x2, int y2)
{
	switch (iXtype)
	{
		case P1:
		{
			*pX = x1;
            break;
		}
		case P1_PLUS:
		{
			*pX = ((x1 + GRID_SPACING) / GRID_SPACING) * GRID_SPACING;
			break;
		}
		case P1_MINUS:
		{
			*pX = (x1 - 1) - ((x1 - 1) % GRID_SPACING);
			break;
		}
		case P2:
		{
			*pX = x2;
			break;
		}
		case P2_PLUS:
		{
			*pX = ((x2 + GRID_SPACING) / GRID_SPACING) * GRID_SPACING;
			break;
		}
		case P2_MINUS:
		{
			*pX = (x2 - 1) - ((x2 - 1) % GRID_SPACING);
			break;
		}
		default:
		{
			assert (FALSE);
        }
	}

	switch (iYtype)
	{
		case P1:
		{
			*pY = y1;
            break;
		}
		case P1_PLUS:
		{
			*pY = ((y1 + GRID_SPACING) / GRID_SPACING) * GRID_SPACING;
			break;
		}
		case P1_MINUS:
		{
			*pY = (y1 - 1) - ((y1 - 1) % GRID_SPACING);
			break;
		}
		case P2:
		{
			*pY = y2;
			break;
		}
		case P2_PLUS:
		{
			*pY = ((y2 + GRID_SPACING) / GRID_SPACING) * GRID_SPACING;
			break;
		}
		case P2_MINUS:
		{
			*pY = (y2 - 1) - ((y2 - 1) % GRID_SPACING);
			break;
		}
		default:
		{
			assert (FALSE);
        }
	}
}
