/*
===========================================================================

BFPR ENDMATCH

===========================================================================
*/

#include "cg_local.h"

#define	EMV_MAP_CELL_W			128
#define	EMV_MAP_CELL_H			96
#define	EMV_MAP_CELL_GAP		8
#define	EMV_MAP_GRID_COLS		4
#define	EMV_MAP_GRID_TOP		96
#define	EMV_MAP_GRID_LEFT		52

#define	EMV_MAP_REVEAL_SHADER_SIZE	320

// ASCII '0'..'9' - this codebase's keycodes.h isn't available here, but
// Q3's key event/polling API uses the ASCII value itself for digit keys
#define	EMV_KEY_0			'0'
#define	EMV_KEY_1			'1'
#define	EMV_KEY_9			'9'

// directional, enter and space keys
#define	EMV_KEY_UPARROW		132
#define	EMV_KEY_DOWNARROW	133
#define	EMV_KEY_LEFTARROW	134
#define	EMV_KEY_RIGHTARROW	135
#define	EMV_KEY_ENTER		13
#define	EMV_KEY_SPACE		32
#define	EMV_KEY_KP_ENTER	169

static vec4_t		transparentBackground = { 0.0f, 0.0f, 0.6f, 0.33f };
static vec4_t		cursorHighlight = { 1.0f, 1.0f, 1.0f, 0.25f }; // white, translucent - the arrow-key cursor, drawn behind whichever cell/line it's on
static char			emvCachedGametypeName[MAX_QPATH];

// edge-detection state for the arrow/confirm keys, so holding one down
// moves/confirms once per press instead of racing every drawn frame
static qboolean		emvConfirmWasDown;
static qboolean		emvArrowWasDown[4]; // indexed by EMV_ARROW_* below
#define	EMV_ARROW_UP		0
#define	EMV_ARROW_DOWN		1
#define	EMV_ARROW_LEFT		2
#define	EMV_ARROW_RIGHT		3

/*
==================
CG_EndMatchVoteOptions

Server command: "emvopts <phase> <count> <special0> <opt0> <special1> <opt1> ..."
==================
*/
void CG_EndMatchVoteOptions( void ) {
	int		i, phase, count;

	phase = atoi( CG_Argv( 1 ) );
	count = atoi( CG_Argv( 2 ) );

	if ( count < 0 ) {
		count = 0;
	}
	if ( count > MAX_ENDMATCH_MAP_CANDIDATES ) {
		count = MAX_ENDMATCH_MAP_CANDIDATES;
	}

	cgs.emvPhase = phase;
	cgs.emvOptionCount = count;
	cgs.emvWinnerIndex = -1;
	cgs.emvMyVoteIndex = -1;
	cgs.emvCursorIndex = -1;
	cgs.emvLocked = qfalse;

	for ( i = 0 ; i < count ; i++ ) {
		cgs.emvOptionSpecial[i] = atoi( CG_Argv( 3 + i * 2 ) );
		Q_strncpyz( cgs.emvOptions[i], CG_Argv( 4 + i * 2 ), sizeof( cgs.emvOptions[0] ) );
		cgs.emvOptionVotes[i] = 0;
		cgs.emvOptionLockedOut[i] = qfalse;
		cgs.emvMapShaders[i] = 0; // lazily (re)registered by the draw code
	}
}

/*
==================
CG_EndMatchVoteResult

Server command: "emvresult <phase> <winnerIndex>"
==================
*/
void CG_EndMatchVoteResult( void ) {
	int		phase = atoi( CG_Argv( 1 ) );
	int		winnerIndex;

	if ( phase != cgs.emvPhase ) {
		return;
	}

	winnerIndex = atoi( CG_Argv( 2 ) );

	if ( phase == EMV_GAMETYPE_VOTE
	&& winnerIndex >= 0 && winnerIndex < cgs.emvOptionCount ) {
		Q_strncpyz( emvCachedGametypeName, cgs.emvOptions[winnerIndex],
			sizeof( emvCachedGametypeName ) );
	}
	cgs.emvWinnerIndex = winnerIndex;
}

/*
==================
CG_EndMatchVoteState

Server command: "emvstate <phase> <phaseStartTime> <phaseEndTime> <locked> <count> <v0> <locked0> <v1> <locked1> ..."
==================
*/
void CG_EndMatchVoteState( void ) {
	int		i, phase, count;

	phase = atoi( CG_Argv( 1 ) );

	cgs.emvPhase = phase;
	cgs.emvPhaseStartTime = atoi( CG_Argv( 2 ) );
	cgs.emvPhaseEndTime = atoi( CG_Argv( 3 ) );
	cgs.emvLocked = atoi( CG_Argv( 4 ) );

	count = atoi( CG_Argv( 5 ) );
	if ( count < 0 ) {
		count = 0;
	}
	if ( count > cgs.emvOptionCount ) {
		count = cgs.emvOptionCount;
	}

	// clear cached gametype
	if ( phase == EMV_SCOREBOARD ) {
		emvCachedGametypeName[0] = 0;
	}

	for ( i = 0 ; i < count ; i++ ) {
		cgs.emvOptionVotes[i] = atoi( CG_Argv( 6 + i * 2 ) );
		cgs.emvOptionLockedOut[i] = atoi( CG_Argv( 7 + i * 2 ) );
	}
}

/*
==================
CG_EndMatchCastVote

Sends the player's choice to the server. Voting again for a different
option moves the player's vote there; the server handles the actual tally change
==================
*/
void CG_EndMatchCastVote( int index ) {
	if ( cgs.emvPhase != EMV_GAMETYPE_VOTE && cgs.emvPhase != EMV_MAP_VOTE ) {
		return;
	}
	if ( index < 0 || index >= cgs.emvOptionCount ) {
		return;
	}
	if ( cgs.emvOptionLockedOut[index] ) {
		return;
	}
	if ( cgs.emvMyVoteIndex == index ) {
		return; // already voted for this one
	}

	cgs.emvMyVoteIndex = index;
	trap_SendClientCommand( va( "endmatchvote %i", index ) );
}


/*
==================
CG_EndMatchTimeLeft
==================
*/
static int CG_EndMatchTimeLeft( void ) {
	int		left = cgs.emvPhaseEndTime - cg.time;
	if ( left < 0 ) {
		left = 0;
	}
	return left;
}


/*
==================
CG_EndMatchFindDontCare
==================
*/
static int CG_EndMatchFindDontCare( void ) {
	int		i;
	for ( i = 0 ; i < cgs.emvOptionCount ; i++ ) {
		if ( cgs.emvOptionSpecial[i] == EMV_OPT_DONT_CARE ) {
			return i;
		}
	}
	return -1;
}


/*
==================
CG_EndMatchMapGridLayout
==================
*/
static int CG_EndMatchMapGridLayout( int rowOfIndex[MAX_ENDMATCH_MAP_CANDIDATES], int colOfIndex[MAX_ENDMATCH_MAP_CANDIDATES],
									int firstIndexOfRow[MAX_ENDMATCH_MAP_CANDIDATES], int itemsInRow[MAX_ENDMATCH_MAP_CANDIDATES] ) {
	int		i, q, r, mapCells, rows, cols, row, cellIndex;

	mapCells = 0;
	for ( i = 0 ; i < cgs.emvOptionCount ; i++ ) {
		rowOfIndex[i] = -1;
		colOfIndex[i] = -1;
		if ( cgs.emvOptionSpecial[i] != EMV_OPT_DONT_CARE ) {
			mapCells++;
		}
	}

	cols = EMV_MAP_GRID_COLS;
	if ( mapCells > 16 && cols < 5 ) {
		cols = 5;
	}

	if ( mapCells <= cols ) {
		rows = 1;
	} else {
		rows = ( mapCells + cols - 1 ) / cols;
	}
	if ( rows < 1 ) {
		rows = 1;
	}

	q = mapCells / rows;
	r = mapCells % rows;

	i = 0;
	for ( row = 0 ; row < rows ; row++ ) {
		itemsInRow[row] = ( row < r ) ? ( q + 1 ) : q;
		firstIndexOfRow[row] = i;

		for ( cellIndex = 0 ; cellIndex < itemsInRow[row] ; cellIndex++ ) {
			while ( i < cgs.emvOptionCount && cgs.emvOptionSpecial[i] == EMV_OPT_DONT_CARE ) {
				i++;
			}
			if ( i >= cgs.emvOptionCount ) {
				break;
			}
			rowOfIndex[i] = row;
			colOfIndex[i] = cellIndex;
			i++;
		}
	}

	return rows;
}


/*
==================
CG_EndMatchNextGametypeIndex

Returns the next cursor position in the gametype list, moving forward
(dir=+1) or backward (dir=-1) from 'from', wrapping around the ends
==================
*/
static int CG_EndMatchNextGametypeIndex( int from, int dir ) {
	if ( cgs.emvOptionCount <= 0 ) {
		return from;
	}
	if ( from < 0 ) {
		return 0;
	}
	return ( from + dir + cgs.emvOptionCount ) % cgs.emvOptionCount;
}


/*
==================
CG_EndMatchNextMapIndex

Returns the map-grid cursor position reached by moving up/down/left/
right from 'from' (screen-space, matching what's drawn): up/down move by
row within the same column (falling back to the nearest existing column
on shorter rows), left/right move by column within the same row and
wrap to the next/previous row at the ends
==================
*/
static int CG_EndMatchNextMapIndex( int from, int dir ) {
	int		rowOfIndex[MAX_ENDMATCH_MAP_CANDIDATES], colOfIndex[MAX_ENDMATCH_MAP_CANDIDATES];
	int		firstIndexOfRow[MAX_ENDMATCH_MAP_CANDIDATES], itemsInRow[MAX_ENDMATCH_MAP_CANDIDATES];
	int		rows, dontCareIndex, row, col, target;

	if ( cgs.emvOptionCount <= 0 ) {
		return from;
	}

	rows = CG_EndMatchMapGridLayout( rowOfIndex, colOfIndex, firstIndexOfRow, itemsInRow );
	dontCareIndex = CG_EndMatchFindDontCare();

	if ( from < 0 ) {
		// nothing picked yet: first press just lands on option 1,
		// it doesn't also apply a move
		return 0;
	}

	if ( from == dontCareIndex ) {
		// sitting on "Don't care": up returns to the grid (its first
		// cell on the bottom row), left/right/down do nothing (it's a
		// lone cell with no neighbours in those directions)
		if ( dir == EMV_ARROW_UP && rows > 0 ) {
			return firstIndexOfRow[rows - 1];
		}
		return from;
	}

	row = rowOfIndex[from];
	col = colOfIndex[from];
	if ( row < 0 ) {
		return from; // shouldn't happen for a non-"Don't care" index
	}

	switch ( dir ) {
	case EMV_ARROW_LEFT:
		if ( col > 0 ) {
			target = firstIndexOfRow[row] + col - 1;
		} else if ( row > 0 ) {
			target = firstIndexOfRow[row - 1] + itemsInRow[row - 1] - 1;
		} else if ( dontCareIndex >= 0 ) {
			target = dontCareIndex;
		} else {
			target = firstIndexOfRow[rows - 1] + itemsInRow[rows - 1] - 1;
		}
		break;

	case EMV_ARROW_RIGHT:
		if ( col < itemsInRow[row] - 1 ) {
			target = firstIndexOfRow[row] + col + 1;
		} else if ( row < rows - 1 ) {
			target = firstIndexOfRow[row + 1];
		} else if ( dontCareIndex >= 0 ) {
			target = dontCareIndex;
		} else {
			target = firstIndexOfRow[0];
		}
		break;

	case EMV_ARROW_UP:
		if ( row > 0 ) {
			target = firstIndexOfRow[row - 1] + ( ( col < itemsInRow[row - 1] ) ? col : itemsInRow[row - 1] - 1 );
		} else if ( dontCareIndex >= 0 ) {
			target = dontCareIndex;
		} else {
			return from; // already on the top row, nowhere to wrap to
		}
		break;

	case EMV_ARROW_DOWN:
	default:
		if ( row < rows - 1 ) {
			target = firstIndexOfRow[row + 1] + ( ( col < itemsInRow[row + 1] ) ? col : itemsInRow[row + 1] - 1 );
		} else if ( dontCareIndex >= 0 ) {
			target = dontCareIndex;
		} else {
			return from; // already on the bottom row, nowhere to wrap to
		}
		break;
	}

	if ( target >= 0 && target < cgs.emvOptionCount ) {
		return target;
	}
	return from;
}


/*
==================
CG_EndMatchPollArrowKeys

Edge-triggered (fires once per press, not once per frame the key is held) arrow-key cursor movement
==================
*/
static void CG_EndMatchPollArrowKeys( void ) {
	static const int	arrowKeys[4] = { EMV_KEY_UPARROW, EMV_KEY_LEFTARROW, EMV_KEY_DOWNARROW, EMV_KEY_RIGHTARROW };
	static const int	arrowIds[4]  = { EMV_ARROW_UP,    EMV_ARROW_LEFT,    EMV_ARROW_DOWN,    EMV_ARROW_RIGHT };
	static const int	gametypeDir[4] = { -1, -1, +1, +1 }; // up/left = previous, down/right = next
	int		i;
	qboolean	down;

	for ( i = 0 ; i < 4 ; i++ ) {
		down = trap_Key_IsDown( arrowKeys[i] );
		if ( down && !emvArrowWasDown[arrowIds[i]] ) {
			if ( cgs.emvPhase == EMV_GAMETYPE_VOTE ) {
				cgs.emvCursorIndex = CG_EndMatchNextGametypeIndex( cgs.emvCursorIndex, gametypeDir[i] );
			} else { // EMV_MAP_VOTE
				cgs.emvCursorIndex = CG_EndMatchNextMapIndex( cgs.emvCursorIndex, arrowIds[i] );
			}
		}
		emvArrowWasDown[arrowIds[i]] = down;
	}
}


/*
==================
CG_EndMatchPollConfirmKey

Edge-triggered Enter/Space/keypad-Enter: confirms the cursor position as
this client's vote. Does nothing if the cursor hasn't been moved yet
(no accidental vote from a stray Enter press before ever touching the
arrow keys).
==================
*/
static void CG_EndMatchPollConfirmKey( void ) {
	qboolean	down;

	down = trap_Key_IsDown( EMV_KEY_ENTER ) || trap_Key_IsDown( EMV_KEY_SPACE ) || trap_Key_IsDown( EMV_KEY_KP_ENTER );
	if ( down && !emvConfirmWasDown ) {
		if ( cgs.emvCursorIndex >= 0 ) {
			CG_EndMatchCastVote( cgs.emvCursorIndex );
		}
	}
	emvConfirmWasDown = down;
}


/*
==================
CG_EndMatchPollInput

Polls (rather than event-hooks) the number keys, arrow keys, the
confirm keys (Enter/Space), and whatever key is bound to "+scores",
once per drawn frame. Number keys (and "0" for "Don't care") still vote
immediately, same as before - the arrow-key cursor is an additional,
optional way to pick and confirm a vote, not a replacement for them.
==================
*/
static void CG_EndMatchPollInput( void ) {
	int		key, dontCareIndex;

	if ( cgs.emvPhase != EMV_GAMETYPE_VOTE && cgs.emvPhase != EMV_MAP_VOTE ) {
		return;
	}

	CG_EndMatchPollArrowKeys();
	CG_EndMatchPollConfirmKey();

	// "0" is a shortcut for the abstain option. 
	// Useful when the option list is long 
	// (e.g. 9 map candidates + "Don't care" = index 9, which keys 1 ... 9 can't reach)
	dontCareIndex = CG_EndMatchFindDontCare();
	if ( dontCareIndex >= 0 && trap_Key_IsDown( EMV_KEY_0 ) ) {
		cgs.emvCursorIndex = dontCareIndex;
		CG_EndMatchCastVote( dontCareIndex );
		return;
	}

	for ( key = EMV_KEY_1 ; key <= EMV_KEY_9 ; key++ ) {
		if ( trap_Key_IsDown( key ) ) {
			cgs.emvCursorIndex = key - EMV_KEY_1;
			CG_EndMatchCastVote( key - EMV_KEY_1 );
			break; // one vote per frame is plenty, and avoids double-casts from holding two keys
		}
	}
}


/*
==================
CG_EndMatchScoreboardKeyHeld

True while the key bound to "+scores" is held down - used to temporarily
show the normal scoreboard instead of the vote overlay
==================
*/
static qboolean CG_EndMatchScoreboardKeyHeld( void ) {
	int		key = trap_Key_GetKey( "+scores" );
	if ( key == -1 ) {
		return qfalse;
	}
	return trap_Key_IsDown( key );
}


/*
==================
CG_DrawEndMatchOptionList

Draws a simple "N. option (votes)" vertical list for the gametype vote
phase (the map vote phase uses the thumbnail grid instead).
The player's own current pick is marked with a leading "> "
==================
*/
static void CG_DrawEndMatchOptionList( int y, const char *title ) {
	int		i, panelH, panelW, seconds, w;
	int		topY, endY;
	char	line[MAX_QPATH + 32];
	vec4_t	color;
	const char	*name;
	const int	TITLE_H = BIGCHAR_HEIGHT + 24;
	const int	OPTION_H = PROP_HEIGHT + 4;
	const int	PRESS_H = PROP_HEIGHT + 4;
	const int	SECONDS_H = BIGCHAR_HEIGHT + 4;
	const int	PAD_TOP = 18;
	const int	PAD_BOTTOM = 18;

	topY = y - PAD_TOP;
	panelW = 320;
	for ( i = 0 ; i < cgs.emvOptionCount ; i++ ) {
		name = ( cgs.emvOptionSpecial[i] == EMV_OPT_DONT_CARE ) ? "Don't care" : cgs.emvOptions[i];
		Com_sprintf( line, sizeof( line ), "%s%i. %s  (%i)", ( cgs.emvMyVoteIndex == i ) ? "> " : "", i + 1, name, cgs.emvOptionVotes[i] );
		w = 36 * BIGCHAR_WIDTH + 34;
		if ( w > panelW ) {
			panelW = w;
		}
	}
	w = CG_DrawStrlen( title ) * BIGCHAR_WIDTH + 24;
	if ( w > panelW ) {
		panelW = w;
	}
	if ( panelW > 620 ) {
		panelW = 620; // never wider than the virtual screen
	}

	endY = y;
	endY += TITLE_H;
	endY += cgs.emvOptionCount * OPTION_H;
	if ( cgs.emvPhase == EMV_GAMETYPE_VOTE ) {
		endY += 8;
		endY += PRESS_H;
		endY += SECONDS_H;
	}
	endY += PAD_BOTTOM;
	panelH = endY - topY;

	CG_FillRect( 320 - panelW / 2, topY, panelW, panelH, transparentBackground );

	// title
	UI_DrawProportionalString( 320, y, title, UI_CENTER|UI_BIGFONT|UI_DROPSHADOW, colorWhite );
	y += TITLE_H;

	// options
	for ( i = 0 ; i < cgs.emvOptionCount ; i++ ) {
		if ( cgs.emvOptionLockedOut[i] ) {
			Vector4Copy( colorWhite, color );
			color[3] = 0.5f;
		} else if ( cgs.emvWinnerIndex == i ) {
			Vector4Copy( colorYellow, color );
		} else if ( cgs.emvMyVoteIndex == i ) {
			Vector4Copy( colorGreen, color );
		} else {
			Vector4Copy( colorWhite, color );
		}

		if ( cgs.emvCursorIndex == i ) {
			// arrow-key highlight, drawn behind the text
			CG_FillRect( 320 - panelW / 2 + 6, y - 2, panelW - 12, OPTION_H, cursorHighlight );
		}

		name = ( cgs.emvOptionSpecial[i] == EMV_OPT_DONT_CARE ) ? "Don't care" : cgs.emvOptions[i];
		Com_sprintf( line, sizeof( line ), "%s%i. %s  (%i)", ( cgs.emvMyVoteIndex == i ) ? "> " : "", i + 1, name, cgs.emvOptionVotes[i] );
		UI_DrawProportionalString( 320, y, line, UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, color );
		y += OPTION_H;
	}

	if ( cgs.emvPhase == EMV_GAMETYPE_VOTE ) {
		y += 8;
		UI_DrawProportionalString( 320, y, "Press a number, or move + Enter, to vote",
			UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, colorYellow );
		y += PRESS_H;

		seconds = ( CG_EndMatchTimeLeft() + 999 ) * 0.001;
		if ( seconds > 0 ) {
			char	*s = va( "%i seconds left", seconds );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
			CG_DrawBigStringColor( 320 - w / 2, y, s, colorGreen );
		}
	}
}


/*
==================
CG_EndMatchShortMapName

Copies a map name into out, truncating to the limit of characters and
appending ".." if the source was longer
==================
*/
static void CG_EndMatchShortMapName( const char *in, char *out, int outSize, int maxChars ) {
	int		len;

	if ( !in || !out || outSize <= 0 ) {
		return;
	}
	if ( maxChars < 4 ) {
		maxChars = 4;
	}

	len = (int)strlen( in );
	if ( len <= maxChars ) {
		Q_strncpyz( out, in, outSize );
		return;
	}

	if ( outSize < maxChars + 3 ) {
		Q_strncpyz( out, in, outSize );
		return;
	}

	Com_Memcpy( out, in, maxChars );
	out[maxChars]     = '.';
	out[maxChars + 1] = '.';
	out[maxChars + 2] = 0;
}


/*
==================
CG_DrawEndMatchMapGrid

Draws the map vote as a thumbnail grid, same cell geometry as the
"Start Server" menu's map picker (128x96 cells, 8px gap, 4 columns).
"Don't care" option gets a plain filled cell instead of a levelshot
==================
*/
static void CG_DrawEndMatchMapGrid( void ) {
	int			i, x, y, cellIndex, seconds;
	int			mapCells, rows, rowStride, labelWidth;
	int			cols, q, r, row, itemsInRow, rowWidth, rowStartX;
	int			bgX, bgTop, bgH;
	int			gridTop, dontCareH, headerExtra, dontCareIndex = -1;
	int			cellW, cellH, cellGap;
	int			imgW, imgH, imgX;
	int			nameChars, chrome, availH, rowStrideMax;
	qhandle_t	shader;
	vec4_t		dim = { 1.0f, 1.0f, 1.0f, 1.0f };
	char		label[MAX_QPATH + 8];
	char		shortMapName[MAX_QPATH];
	vec4_t		labelColor;
	const int	MAX_BACKGROUND_WIDTH = 600;

	mapCells = 0;
	for ( i = 0 ; i < cgs.emvOptionCount ; i++ ) {
		if ( cgs.emvOptionSpecial[i] == EMV_OPT_DONT_CARE ) {
			dontCareIndex = i;
		} else {
			mapCells++;
		}
	}

	// columns
	cols = EMV_MAP_GRID_COLS;
	if ( mapCells > 16 && cols < 5 ) {
		cols = 5;
	}

	// rows
	if ( mapCells <= cols ) {
		rows = 1;
	} else {
		rows = ( mapCells + cols - 1 ) / cols;
	}
	if ( rows < 1 ) {
		rows = 1;
	}

	q = mapCells / rows;
	r = mapCells % rows;

	// cell width: fit the panel without overflowing
	cellGap = EMV_MAP_CELL_GAP;
	{
		const int	PANEL_MAX_W = 620;
		const int	MIN_CELL_W  = 84;
		int	rowContentMax = PANEL_MAX_W - 52;
		int	maxCellW = ( rowContentMax - ( cols - 1 ) * cellGap ) / cols;
		if ( maxCellW > EMV_MAP_CELL_W ) {
			maxCellW = EMV_MAP_CELL_W;
		}
		if ( maxCellW < MIN_CELL_W ) {
			maxCellW = MIN_CELL_W;
		}
		cellW = maxCellW;
	}

	// cell height: fit available vertical space after the chrome
	dontCareH   = ( dontCareIndex >= 0 ) ? ( SMALLCHAR_HEIGHT + 4 ) : 0;
	headerExtra = emvCachedGametypeName[0] ? ( BIGCHAR_HEIGHT + 8 ) : 0;

	chrome = 24									// top pad
		+ ( BIGCHAR_HEIGHT + 8 )				// "Vote for a map"
		+ headerExtra							// cached gametype name
		+ dontCareH								// "Don't care" line
		+ ( PROP_HEIGHT + BIGCHAR_HEIGHT + 8 )	// Press + seconds
		+ 12;									// bottom pad

	availH = SCREEN_HEIGHT - 16 - chrome;
	if ( availH < 100 ) {
		availH = 100;
	}

	rowStrideMax = availH / rows;
	// rowStride = cellH + cellGap + SMALLCHAR_HEIGHT + 4
	cellH = rowStrideMax - cellGap - SMALLCHAR_HEIGHT - 4;
	if ( cellH > EMV_MAP_CELL_H ) {
		cellH = EMV_MAP_CELL_H;
	}
	if ( cellH < 24 ) {
		cellH = 24; // hard floor so thumbnails stay readable
	}

	rowStride = cellH + cellGap + SMALLCHAR_HEIGHT + 4;

	// grid top: keep the panel inside the screen
	gridTop = EMV_MAP_GRID_TOP;
	{
		const int	TOP_MARGIN = 4;
		const int	BOTTOM_MARGIN = 8;
		int	contentBelow = rows * rowStride + dontCareH
				+ 4 + PROP_HEIGHT + BIGCHAR_HEIGHT + 12;
		int	maxTop = SCREEN_HEIGHT - BOTTOM_MARGIN - contentBelow;
		int	minTop = PROP_HEIGHT + 24 + headerExtra + TOP_MARGIN;

		if ( gridTop > maxTop ) {
			gridTop = maxTop;
		}
		if ( gridTop < minTop ) {
			gridTop = minTop;
		}
	}

	bgX = 320 - MAX_BACKGROUND_WIDTH / 2;
	bgTop = gridTop - PROP_HEIGHT - 24 - headerExtra;
	bgH = ( gridTop - bgTop ) + rows * rowStride
		+ dontCareH + 4 + PROP_HEIGHT + BIGCHAR_HEIGHT + 12;
	CG_FillRect( bgX, bgTop, MAX_BACKGROUND_WIDTH, bgH, transparentBackground );

	UI_DrawProportionalString( 320, gridTop - BIGCHAR_HEIGHT - 16 - headerExtra, "Vote for a map",
		UI_CENTER|UI_BIGFONT|UI_DROPSHADOW, colorWhite );

	if ( emvCachedGametypeName[0] ) {
		UI_DrawProportionalString( 320, gridTop - BIGCHAR_HEIGHT - 12, emvCachedGametypeName, 
			UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
	}

	// per-cell labels: fit inside the cell width
	nameChars = cellW / SMALLCHAR_WIDTH - 7;
	if ( nameChars < 4 )  nameChars = 4;
	if ( nameChars > 12 ) nameChars = 12;

	i = 0;
	for ( row = 0 ; row < rows ; row++ ) {
		itemsInRow = ( row < r ) ? ( q + 1 ) : q;
		if ( itemsInRow <= 0 ) {
			continue;
		}

		rowWidth  = itemsInRow * cellW + ( itemsInRow - 1 ) * cellGap;
		rowStartX = 320 - rowWidth / 2;
		y = gridTop + row * rowStride;

		for ( cellIndex = 0 ; cellIndex < itemsInRow ; cellIndex++ ) {
			while ( i < cgs.emvOptionCount && cgs.emvOptionSpecial[i] == EMV_OPT_DONT_CARE ) {
				i++;
			}
			if ( i >= cgs.emvOptionCount ) {
				break;
			}

			x = rowStartX + cellIndex * ( cellW + cellGap );

			if ( cgs.emvCursorIndex == i ) {
				trap_R_SetColor( NULL );
				CG_FillRect( x, y, cellW, cellH + 23, cursorHighlight );
			}

			dim[3] = cgs.emvOptionLockedOut[i] ? 0.5f : 1.0f;
			trap_R_SetColor( dim );

			if ( !cgs.emvMapShaders[i] ) {
				shader = trap_R_RegisterShaderNoMip( va( "levelshots/%s.tga", cgs.emvOptions[i] ) );
				if ( !shader ) {
					shader = trap_R_RegisterShaderNoMip( "menu/art/unknownmap" );
				}
				cgs.emvMapShaders[i] = shader;
			}
			imgH = cellH;
			imgW = ( cellH * 4 ) / 3;
			if ( imgW > cellW ) {
				imgW = cellW;
				imgH = ( cellW * 3 ) / 4;
			}
			imgX = x + ( cellW - imgW ) / 2;

			CG_DrawPic( imgX, y, imgW, imgH, cgs.emvMapShaders[i] );

			if ( cgs.emvMyVoteIndex == i ) {
				CG_DrawRect( imgX, y, imgW, imgH, 2, colorGreen );
			} else if ( cgs.emvWinnerIndex == i ) {
				CG_DrawRect( imgX, y, imgW, imgH, 2, colorYellow );
			}

			trap_R_SetColor( NULL );

			Vector4Copy( colorWhite, labelColor );
			if ( cgs.emvOptionLockedOut[i] ) {
				labelColor[3] = 0.5f;
			} else if ( cgs.emvWinnerIndex == i ) {
				Vector4Copy( colorYellow, labelColor );
			} else if ( cgs.emvMyVoteIndex == i ) {
				Vector4Copy( colorGreen, labelColor );
			}

			CG_EndMatchShortMapName( cgs.emvOptions[i], shortMapName, sizeof( shortMapName ), nameChars );
			Com_sprintf( label, sizeof( label ), "%i.%s(%i)", i + 1, shortMapName, cgs.emvOptionVotes[i] );
			labelWidth = CG_DrawStrlen( label ) * SMALLCHAR_WIDTH;
			CG_DrawSmallStringColor( x + cellW / 2 - labelWidth / 2, y + cellH + 4, label, labelColor );

			i++;
		}
	}

	y = gridTop + rows * rowStride;

	if ( dontCareIndex >= 0 ) {
		i = dontCareIndex;
		if ( cgs.emvOptionLockedOut[i] ) {
			Vector4Copy( colorWhite, labelColor );
			labelColor[3] = 0.5f;
		} else if ( cgs.emvWinnerIndex == i ) {
			Vector4Copy( colorYellow, labelColor );
		} else if ( cgs.emvMyVoteIndex == i ) {
			Vector4Copy( colorGreen, labelColor );
		} else {
			Vector4Copy( colorWhite, labelColor );
		}

		Com_sprintf( label, sizeof( label ), "%s%i. Don't care (%i)",
			( cgs.emvMyVoteIndex == i ) ? "> " : "", i + 1, cgs.emvOptionVotes[i] );
		labelWidth = CG_DrawStrlen( label ) * SMALLCHAR_WIDTH;

		if ( cgs.emvCursorIndex == i ) {
			CG_FillRect( 320 - labelWidth / 2 - 6, y - 2, labelWidth + 12, SMALLCHAR_HEIGHT + 4, cursorHighlight );
		}

		CG_DrawSmallStringColor( 320 - labelWidth / 2, y, label, labelColor );
		y += SMALLCHAR_HEIGHT + 4;
	}

	if ( cgs.emvPhase == EMV_MAP_VOTE ) {
		int		w;
		char	*s;

		y += 4;
		UI_DrawProportionalString( 320, y, "Press a number, or move + Enter, to vote",
			UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, colorYellow );
		y += PROP_HEIGHT;

		seconds = ( CG_EndMatchTimeLeft() + 999 ) * 0.001;
		if ( seconds > 0 ) {
			s = va( "%i seconds left", seconds );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
			CG_DrawBigStringColor( 320 - w / 2, y, s, colorGreen );
		}
	}
}


/*
==================
CG_DrawEndMatchMapReveal

Shows the winning map's levelshot enlarged in the center of the screen,
scaling up from a tiny size with an ease-out-back pop
==================
*/
static void CG_DrawEndMatchMapReveal( void ) {
	qhandle_t	levelshot;
	const char	*mapName;
	int			x, y, finalY;
	float		t, size, elapsed, alpha;
	vec4_t		color;
	const float	REVEAL_DURATION = 900.0f; // ms, tweak to taste

	// if the map phase resolved to a restart without a vote (single-map server), 
	// shows a simple "Restarting..." message instead
	if ( cgs.emvWinnerIndex < 0 || cgs.emvWinnerIndex >= cgs.emvOptionCount ) {
		UI_DrawProportionalString( 320, SCREEN_HEIGHT / 2 - PROP_HEIGHT, "Restarting map...",
			UI_CENTER|UI_BIGFONT|UI_DROPSHADOW, colorYellow );
		return;
	}
	mapName = cgs.emvOptions[cgs.emvWinnerIndex];

	levelshot = trap_R_RegisterShaderNoMip( va( "levelshots/%s.tga", mapName ) );
	if ( !levelshot ) {
		levelshot = trap_R_RegisterShaderNoMip( "menu/art/unknownmap" );
	}

	finalY = ( SCREEN_HEIGHT - EMV_MAP_REVEAL_SHADER_SIZE ) / 2 - 20;

	// scale + fade-in animation
	elapsed = (float)( cg.time - cgs.emvPhaseStartTime );
	if ( elapsed < 0.0f ) {
		elapsed = 0.0f;
	}
	t = elapsed / REVEAL_DURATION;
	if ( t > 1.0f ) {
		t = 1.0f;
	}

	// ease-out cubic
	{
		float	u = 1.0f - t;
		t = 1.0f - u * u * u;
	}

	size = EMV_MAP_REVEAL_SHADER_SIZE * t;
	if ( size < 1.0f ) {
		size = 1.0f; // avoid a degenerate (0x0) quad at t == 0
	}

	// alpha fade-in: same curve as the scale, so both finish together
	alpha = t;
	if ( alpha < 0.05f ) {
		alpha = 0.05f;
	}

	x = ( SCREEN_WIDTH  - size ) / 2;
	y = ( SCREEN_HEIGHT - size ) / 2 - 20;

	color[0] = 1.0f;
	color[1] = 1.0f;
	color[2] = 1.0f;
	color[3] = alpha;
	trap_R_SetColor( color );
	CG_DrawPic( x, y, size, size, levelshot );
	trap_R_SetColor( NULL );

	// labels anchored to the final position, not the animated one, so they don't jitter;
	// fade them in with the same curve so they don't pop in ahead of the shader
	{
		vec4_t	labelGreen, labelWhite;

		Vector4Copy( colorGreen, labelGreen );
		labelGreen[3] = alpha;
		Vector4Copy( colorWhite, labelWhite );
		labelWhite[3] = alpha;

		UI_DrawProportionalString( 320, finalY + EMV_MAP_REVEAL_SHADER_SIZE + 10, mapName,
			UI_CENTER|UI_BIGFONT|UI_DROPSHADOW, labelGreen );
	}
}


/*
==================
CG_DrawEndMatchVote
==================
*/
void CG_DrawEndMatchVote( void ) {
	CG_EndMatchPollInput();

	switch ( cgs.emvPhase ) {
	case EMV_GAMETYPE_VOTE:
		if ( CG_EndMatchScoreboardKeyHeld() ) {
			return; // caller draws the normal scoreboard for us
		}
		CG_DrawEndMatchOptionList( 56, "Decide the gametype" );
		break;

	case EMV_MAP_VOTE:
		if ( CG_EndMatchScoreboardKeyHeld() ) {
			return; // caller draws the normal scoreboard for us
		}
		CG_DrawEndMatchMapGrid();
		break;

	case EMV_MAP_REVEAL:
		CG_DrawEndMatchMapGrid(); // keep map vote selection
		CG_DrawEndMatchMapReveal();
		break;

	case EMV_SCOREBOARD:
	case EMV_INACTIVE:
	default: // nothing extra to draw - the scoreboard alone is enough
		break;
	}
}


/*
==================
CG_EndMatchVoteShowsScoreboard
==================
*/
qboolean CG_EndMatchVoteShowsScoreboard( void ) {
	if ( cgs.emvPhase == EMV_MAP_REVEAL ) {
		return qfalse;
	}
	if ( cgs.emvPhase != EMV_GAMETYPE_VOTE && cgs.emvPhase != EMV_MAP_VOTE ) {
		return qtrue;
	}
	return CG_EndMatchScoreboardKeyHeld();
}
