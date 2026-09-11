/*
===========================================================================

BFPR ENDMATCH

===========================================================================
*/

// The intermission no longer waits for players to mark themselves ready.
// Instead it runs through a short sequence of phases:
//
//   EMV_SCOREBOARD    - fixed scoreboard display, 5000 ms
//   EMV_GAMETYPE_VOTE - vote for the next gametype, g_endmatch_timeout ms
//   EMV_MAP_VOTE      - vote for the next map, g_endmatch_timeout ms
//   EMV_MAP_REVEAL    - show the winning map, 2500 ms

#include "g_local.h"

static const char *gametypeVoteNames[] = {
	"Free For All",
	"Tournament",
	"Single Player",
	"Survival",
	"Monster",
	"Team Deathmatch",
	"Team Last Man Standing",
	"Capture the Flag"
};

#define	EMV_MAP_LIST_BUFFER_SIZE	32768

/*
==================
G_EndMatchGametypeName
==================
*/
static const char *G_EndMatchGametypeName( int gametype ) {
	if ( gametype < 0 || gametype >= GT_MAX_GAME_TYPE || gametype >= (int)ARRAY_LEN( gametypeVoteNames ) ) {
		return "Unknown";
	}
	return gametypeVoteNames[gametype];
}


/*
==================
G_ListServerMaps
==================
*/
static int G_ListServerMaps( char names[][MAX_QPATH], int maxNames ) {
	static char	listBuf[EMV_MAP_LIST_BUFFER_SIZE];
	int			total;
	int			i;
	const char	*ptr;
	char		trimmed[MAX_QPATH];
	int			len;

	total = trap_FS_GetFileList( "maps", ".bsp", listBuf, sizeof( listBuf ) );

	ptr = listBuf;
	for ( i = 0 ; i < total ; i++ ) {
		if ( i < maxNames ) {
			Q_strncpyz( trimmed, ptr, sizeof( trimmed ) );
			len = (int)strlen( trimmed );
			// strip the ".bsp" extension
			if ( len > 4 && !Q_stricmp( trimmed + len - 4, ".bsp" ) ) {
				trimmed[len - 4] = 0;
			}
			Q_strncpyz( names[i], trimmed, MAX_QPATH );
		}
		ptr += strlen( ptr ) + 1;
	}

	return total;
}


/*
==================
G_ParseGametypePool

Parses g_endmatch_gametype_options ("0 3 5") into level.emvGametypeCandidates, then
appends the "Don't care" special option
==================
*/
static void G_ParseGametypePool( void ) {
	char	buf[MAX_STRING_CHARS];
	char	*s, *tok;
	int		gt;

	level.emvGametypeCandidateCount = 0;

	Q_strncpyz( buf, g_endmatch_gametype_options.string, sizeof( buf ) );
	s = buf;

	// leave one slot free for the "Don't care" option appended below
	while ( *s && level.emvGametypeCandidateCount < MAX_ENDMATCH_GAMETYPE_CANDIDATES - 1 ) {
		while ( *s == ' ' || *s == '\t' ) {
			s++;
		}
		if ( !*s ) {
			break;
		}

		tok = s;
		while ( *s && *s != ' ' && *s != '\t' ) {
			s++;
		}
		if ( *s ) {
			*s = 0;
			s++;
		}

		gt = atoi( tok );
		if ( gt == GT_SINGLE_PLAYER || gt < GT_FFA || gt >= GT_MAX_GAME_TYPE ) {
			G_Printf( S_COLOR_YELLOW "WARNING: g_endmatch_gametype_options: invalid gametype '%s', skipping\n", tok );
			continue;
		}

		level.emvGametypeCandidates[level.emvGametypeCandidateCount] = gt;
		level.emvGametypeCandidateSpecial[level.emvGametypeCandidateCount] = EMV_OPT_NONE;
		level.emvGametypeCandidateCount++;
	}

	// fall back to the current gametype if the pool is empty/invalid,
	// so there's always at least one real option to vote on
	if ( !level.emvGametypeCandidateCount ) {
		level.emvGametypeCandidates[0] = g_gametype.integer;
		level.emvGametypeCandidateSpecial[0] = EMV_OPT_NONE;
		level.emvGametypeCandidateCount = 1;
	}

	// always offer "Don't care"
	level.emvGametypeCandidates[level.emvGametypeCandidateCount] = 0;
	level.emvGametypeCandidateSpecial[level.emvGametypeCandidateCount] = EMV_OPT_DONT_CARE;
	level.emvGametypeCandidateCount++;
}


/*
==================
G_CountRealGametypeCandidates

How many *real* (non "Don't care") gametypes are in the pool. 
Used to decide whether the gametype vote phase should run at all
==================
*/
static int G_CountRealGametypeCandidates( void ) {
	char	buf[MAX_STRING_CHARS];
	char	*s, *tok;
	int		gt, count;

	count = 0;

	Q_strncpyz( buf, g_endmatch_gametype_options.string, sizeof( buf ) );
	s = buf;

	while ( *s && count < MAX_ENDMATCH_GAMETYPE_CANDIDATES - 1 ) {
		while ( *s == ' ' || *s == '\t' ) {
			s++;
		}
		if ( !*s ) {
			break;
		}

		tok = s;
		while ( *s && *s != ' ' && *s != '\t' ) {
			s++;
		}
		if ( *s ) {
			*s = 0;
			s++;
		}

		gt = atoi( tok );
		if ( gt == GT_SINGLE_PLAYER || gt < GT_FFA || gt >= GT_MAX_GAME_TYPE ) {
			continue;
		}
		count++;
	}

	return count ? count : 1; // the g_gametype.integer fallback counts as one real option
}


/*
==================
G_PickRandomMapCandidates

Fills level.emvMapCandidates with up to g_endmatch_map_count unique,
randomly chosen real maps (from whatever .bsp files exist under maps/),
then appends the "Don't care" special option
==================
*/
static void G_PickRandomMapCandidates( void ) {
	static char	allMaps[64][MAX_QPATH];
	int			totalOnDisk, poolCount, wanted;
	int			available[64];
	int			i, pick, tmp, availableCount;

	level.emvMapCandidateCount = 0;

	totalOnDisk = G_ListServerMaps( allMaps, (int)ARRAY_LEN( allMaps ) );
	poolCount = totalOnDisk;
	if ( poolCount > (int)ARRAY_LEN( allMaps ) ) {
		poolCount = (int)ARRAY_LEN( allMaps );
	}

	// leave one slot free for the "Don't care" option appended below
	wanted = g_endmatch_map_count.integer;
	if ( wanted < 1 ) {
		wanted = 1;
	}
	if ( wanted > MAX_ENDMATCH_MAP_CANDIDATES - 1 ) {
		wanted = MAX_ENDMATCH_MAP_CANDIDATES - 1;
	}
	if ( wanted > poolCount ) {
		wanted = poolCount;
	}

	availableCount = poolCount;
	for ( i = 0 ; i < availableCount ; i++ ) {
		available[i] = i;
	}

	for ( i = 0 ; i < wanted ; i++ ) {
		pick = i + ( rand() % ( availableCount - i ) );

		tmp = available[i];
		available[i] = available[pick];
		available[pick] = tmp;

		Q_strncpyz( level.emvMapCandidates[i], allMaps[available[i]], sizeof( level.emvMapCandidates[0] ) );
		level.emvMapCandidateSpecial[i] = EMV_OPT_NONE;
	}

	level.emvMapCandidateCount = wanted;

	// always offer "Don't care"
	level.emvMapCandidates[level.emvMapCandidateCount][0] = 0;
	level.emvMapCandidateSpecial[level.emvMapCandidateCount] = EMV_OPT_DONT_CARE;
	level.emvMapCandidateCount++;
}


/*
==================
G_CountRealMapCandidates

How many .bsp files exist under maps/. Used to decide whether the map
vote phase should run at all (a single-map server always just restarts)
==================
*/
static int G_CountRealMapCandidates( void ) {
	static char	allMaps[64][MAX_QPATH];
	return G_ListServerMaps( allMaps, (int)ARRAY_LEN( allMaps ) );
}


/*
==================
G_SendEndMatchVoteOptions

Sends the option list for the phase that is about to start (or, when
called for a late-joining client, for the phase currently in progress).
Format: "emvopts <phase> <count> <special0> <opt0> <special1> <opt1> ..."
special is an EMV_OPT_* value; opt is the display name.
target is -1 to broadcast to everyone, or a specific clientNum
==================
*/
static void G_SendEndMatchVoteOptions( int target ) {
	char	msg[MAX_STRING_CHARS];
	int		i;

	if ( level.emvPhase == EMV_GAMETYPE_VOTE ) {
		Com_sprintf( msg, sizeof( msg ), "emvopts %i %i", EMV_GAMETYPE_VOTE, level.emvGametypeCandidateCount );
		for ( i = 0 ; i < level.emvGametypeCandidateCount ; i++ ) {
			if ( level.emvGametypeCandidateSpecial[i] == EMV_OPT_DONT_CARE ) {
				Q_strcat( msg, sizeof( msg ), va( " %i \"Don't care\"", EMV_OPT_DONT_CARE ) );
			} else {
				Q_strcat( msg, sizeof( msg ), va( " %i \"%s\"", EMV_OPT_NONE, G_EndMatchGametypeName( level.emvGametypeCandidates[i] ) ) );
			}
		}
		trap_SendServerCommand( target, msg );
	} else if ( level.emvPhase == EMV_MAP_VOTE ) {
		Com_sprintf( msg, sizeof( msg ), "emvopts %i %i", EMV_MAP_VOTE, level.emvMapCandidateCount );
		for ( i = 0 ; i < level.emvMapCandidateCount ; i++ ) {
			if ( level.emvMapCandidateSpecial[i] == EMV_OPT_DONT_CARE ) {
				Q_strcat( msg, sizeof( msg ), va( " %i \"Don't care\"", EMV_OPT_DONT_CARE ) );
			} else {
				Q_strcat( msg, sizeof( msg ), va( " %i \"%s\"", EMV_OPT_NONE, level.emvMapCandidates[i] ) );
			}
		}
		trap_SendServerCommand( target, msg );
	}
}


/*
==================
G_SendEndMatchVoteResult

Sends which option won the phase that just ended.
Format: "emvresult <phase> <winnerIndex>"
winnerIndex is -1 when the phase resolved to EMV_OPT_RESTART without
ever offering a vote (single-map server).
target is -1 to broadcast to everyone, or a specific clientNum
==================
*/
static void G_SendEndMatchVoteResult( int target, int phase, int winnerIndex ) {
	trap_SendServerCommand( target, va( "emvresult %i %i", phase, winnerIndex ) );
}


/*
==================
G_SendEndMatchState

Sends the current phase/timing, lock state, and live vote counts.
Format: "emvstate <phase> <phaseStartTime> <phaseEndTime> <locked> <count> <v0> <locked0> <v1> <locked1> ..."
target is -1 to broadcast to everyone, or a specific clientNum
==================
*/
static void G_SendEndMatchState( int target ) {
	char		msg[MAX_STRING_CHARS];
	const int	*votes;
	const qboolean	*lockedOut;
	int			i, count;

	Com_sprintf( msg, sizeof( msg ), "emvstate %i %i %i %i", level.emvPhase, level.emvPhaseStartTime, level.emvPhaseEndTime, level.emvLocked );

	if ( level.emvPhase == EMV_GAMETYPE_VOTE ) {
		votes = level.emvGametypeVotes;
		lockedOut = level.emvGametypeLockedOut;
		count = level.emvGametypeCandidateCount;
	} else if ( level.emvPhase == EMV_MAP_VOTE ) {
		votes = level.emvMapVotes;
		lockedOut = level.emvMapLockedOut;
		count = level.emvMapCandidateCount;
	} else {
		votes = NULL;
		lockedOut = NULL;
		count = 0;
	}

	Q_strcat( msg, sizeof( msg ), va( " %i", count ) );
	for ( i = 0 ; i < count ; i++ ) {
		Q_strcat( msg, sizeof( msg ), va( " %i %i", votes[i], lockedOut[i] ) );
	}

	trap_SendServerCommand( target, msg );
}


/*
==================
G_BroadcastEndMatchState

Convenience wrapper for the common "notify everyone" case.
==================
*/
static void G_BroadcastEndMatchState( void ) {
	G_SendEndMatchState( -1 );
}


/*
==================
G_EndMatchVoteActive
==================
*/
qboolean G_EndMatchVoteActive( void ) {
	return ( level.emvPhase != EMV_INACTIVE );
}


/*
==================
G_SyncEndMatchVoteToClient
==================
*/
void G_SyncEndMatchVoteToClient( int clientNum ) {
	if ( level.emvPhase == EMV_INACTIVE ) {
		return;
	}

	if ( level.emvPhase == EMV_GAMETYPE_VOTE || level.emvPhase == EMV_MAP_VOTE ) {
		G_SendEndMatchVoteOptions( clientNum );
	} else if ( level.emvPhase == EMV_MAP_REVEAL ) {
		// the vote that led to this reveal already happened before this
		// client connected - replay its result too, using whichever
		// phase actually produced the map reveal we're currently in
		if ( level.emvWinnerIsRestart ) {
			G_SendEndMatchVoteResult( clientNum, EMV_MAP_VOTE, -1 );
		} else {
			// the client has no candidate list for a phase that's
			// already over, but CG_EndMatchVoteResult only needs the
			// winner's name, so send a one-option "list" carrying just
			// that map, with index 0 as the winner
			trap_SendServerCommand( clientNum, va( "emvopts %i 1 %i \"%s\"", EMV_MAP_VOTE, EMV_OPT_NONE, level.emvWinnerMap ) );
			G_SendEndMatchVoteResult( clientNum, EMV_MAP_VOTE, 0 );
		}
	}

	G_SendEndMatchState( clientNum );
}


/*
==================
G_ApplyEndMatchLockout
==================
*/
static void G_ApplyEndMatchLockout( void ) {
	const int	*votes;
	qboolean	*lockedOut;
	const int	*special;
	int			i, j, tmp;
	int			count, realCount, bestReal, topCount, keepCount, toFill, candCount;
	int			candidates[MAX_ENDMATCH_MAP_CANDIDATES];
	qboolean	keep[MAX_ENDMATCH_MAP_CANDIDATES];

	switch ( level.emvPhase ) {
	case EMV_GAMETYPE_VOTE:
		votes = level.emvGametypeVotes;
		lockedOut = level.emvGametypeLockedOut;
		special = level.emvGametypeCandidateSpecial;
		count = level.emvGametypeCandidateCount;
		break;
	case EMV_MAP_VOTE:
		votes = level.emvMapVotes;
		lockedOut = level.emvMapLockedOut;
		special = level.emvMapCandidateSpecial;
		count = level.emvMapCandidateCount;
		break;
	default:
		return;
	}

	// start from all-locked and unlock a small set as we go
	for ( i = 0 ; i < count ; i++ ) {
		lockedOut[i] = ( special[i] == EMV_OPT_NONE );
		keep[i] = qfalse;
	}

	// count real options and the highest real vote tally
	realCount = 0;
	bestReal = 0;
	for ( i = 0 ; i < count ; i++ ) {
		if ( special[i] != EMV_OPT_NONE ) {
			continue;
		}
		realCount++;
		if ( votes[i] > bestReal ) {
			bestReal = votes[i];
		}
	}

	if ( realCount == 0 ) {
		// shouldn't happen - there's always at least one real option
		level.emvLocked = qtrue;
		return;
	}

	// aim for at least 3 options available (or all of them if there are
	// fewer than 3 real options in the first place)
	keepCount = 3;
	if ( keepCount > realCount ) {
		keepCount = realCount;
	}

	// options tied for the lead always stay in
	topCount = 0;
	if ( bestReal > 0 ) {
		for ( i = 0 ; i < count ; i++ ) {
			if ( special[i] != EMV_OPT_NONE ) {
				continue;
			}
			if ( votes[i] == bestReal ) {
				keep[i] = qtrue;
				topCount++;
			}
		}
	}

	// top up with random real options until keepCount is reached
	if ( topCount < keepCount ) {
		candCount = 0;
		for ( i = 0 ; i < count ; i++ ) {
			if ( special[i] == EMV_OPT_NONE && !keep[i] ) {
				candidates[candCount++] = i;
			}
		}

		toFill = keepCount - topCount;
		for ( i = 0 ; i < toFill && i < candCount ; i++ ) {
			j = i + rand() % ( candCount - i );
			tmp = candidates[i];
			candidates[i] = candidates[j];
			candidates[j] = tmp;
			keep[candidates[i]] = qtrue;
		}
	}

	// unlock the kept real options
	for ( i = 0 ; i < count ; i++ ) {
		if ( special[i] == EMV_OPT_NONE && keep[i] ) {
			lockedOut[i] = qfalse;
		}
	}

	level.emvLocked = qtrue;
}


/*
==================
G_PickWinner

Returns the index with the most votes among the candidates that are not
locked out, breaking ties randomly. If a "Don't care" option wins (or
there's a tie involving it), the winner is instead picked among the
real, non-"Don't care" options with the most votes; if none of those
have any votes either, picks uniformly at random among all real,
non-locked-out options
==================
*/
static int G_PickWinner( const int *votes, const int *special, const qboolean *lockedOut, int count ) {
	int		tied[MAX_ENDMATCH_MAP_CANDIDATES];
	int		realTied[MAX_ENDMATCH_MAP_CANDIDATES];
	int		i, bestCount, tiedCount, realTiedCount, bestRealCount;
	qboolean	dontCareInvolved;

	if ( count <= 0 ) {
		return 0;
	}

	// pass 1: find the overall leader(s) among options still in play
	bestCount = -1;
	for ( i = 0 ; i < count ; i++ ) {
		if ( lockedOut[i] ) {
			continue;
		}
		if ( votes[i] > bestCount ) {
			bestCount = votes[i];
		}
	}

	tiedCount = 0;
	for ( i = 0 ; i < count ; i++ ) {
		if ( lockedOut[i] ) {
			continue;
		}
		if ( votes[i] == bestCount ) {
			tied[tiedCount++] = i;
		}
	}

	// if none of the leaders is "Don't care", we're done
	dontCareInvolved = qfalse;
	for ( i = 0 ; i < tiedCount ; i++ ) {
		if ( special[tied[i]] == EMV_OPT_DONT_CARE ) {
			dontCareInvolved = qtrue;
			break;
		}
	}
	if ( !dontCareInvolved ) {
		return tied[rand() % tiedCount];
	}

	// "Don't care" is winning (alone or tied): resolve among the real
	// options instead, regardless of lock state - a locked-out real
	// option should still be reachable if the alternative is nothing
	// but an abstention
	bestRealCount = -1;
	for ( i = 0 ; i < count ; i++ ) {
		if ( special[i] != EMV_OPT_NONE ) {
			continue;
		}
		if ( votes[i] > bestRealCount ) {
			bestRealCount = votes[i];
		}
	}

	realTiedCount = 0;
	for ( i = 0 ; i < count ; i++ ) {
		if ( special[i] != EMV_OPT_NONE ) {
			continue;
		}
		if ( votes[i] == bestRealCount ) {
			realTied[realTiedCount++] = i;
		}
	}

	if ( realTiedCount ) {
		return realTied[rand() % realTiedCount];
	}

	// nobody voted for any real option at all: pick uniformly at random
	// among all real options, regardless of lock state
	realTiedCount = 0;
	for ( i = 0 ; i < count ; i++ ) {
		if ( special[i] == EMV_OPT_NONE ) {
			realTied[realTiedCount++] = i;
		}
	}
	if ( !realTiedCount ) {
		return 0; // shouldn't happen - there's always at least one real option
	}
	return realTied[rand() % realTiedCount];
}


/*
==================
G_EndMatchEnterPhase

Advances to a new phase, resetting per-phase vote state and notifying clients
==================
*/
static void G_EndMatchEnterPhase( int phase ) {
	int		i, duration;

	level.emvPhase = phase;
	level.emvPhaseStartTime = level.time;
	level.emvLocked = qfalse;

	switch ( phase ) {
	default:
	case EMV_SCOREBOARD:
		duration = 5000;
		break;
	case EMV_GAMETYPE_VOTE:
	case EMV_MAP_VOTE:
		duration = g_endmatch_timeout.integer * 1000;
		break;
	case EMV_MAP_REVEAL:
		duration = 2500;
		break;
	}
	if ( duration < 0 ) {
		duration = 0;
	}
	level.emvPhaseEndTime = level.time + duration;

	for ( i = 0 ; i < MAX_CLIENTS ; i++ ) {
		level.emvClientVote[i] = -1;
	}

	if ( phase == EMV_GAMETYPE_VOTE ) {
		G_ParseGametypePool();
		Com_Memset( level.emvGametypeVotes, 0, sizeof( level.emvGametypeVotes ) );
		Com_Memset( level.emvGametypeLockedOut, 0, sizeof( level.emvGametypeLockedOut ) );
		G_SendEndMatchVoteOptions( -1 );
	} else if ( phase == EMV_MAP_VOTE ) {
		G_PickRandomMapCandidates();
		Com_Memset( level.emvMapVotes, 0, sizeof( level.emvMapVotes ) );
		Com_Memset( level.emvMapLockedOut, 0, sizeof( level.emvMapLockedOut ) );
		G_SendEndMatchVoteOptions( -1 );
	}

	G_BroadcastEndMatchState();
}


/*
==================
G_BeginEndMatchVote
==================
*/
void G_BeginEndMatchVote( void ) {
	if ( g_endmatch_map_count.integer <= 0 ) {
		level.emvPhase = EMV_INACTIVE;
		return;
	}

	if ( g_gametype.integer == GT_SINGLE_PLAYER ) {
		// single player uses the podium sequence instead
		level.emvPhase = EMV_INACTIVE;
		return;
	}

	level.emvWinnerGametype = g_gametype.integer;
	level.emvWinnerIsRestart = qfalse;
	level.emvWinnerMap[0] = 0;

	G_EndMatchEnterPhase( EMV_SCOREBOARD );
}


/*
==================
G_EndMatchUnanimousVote
==================
*/
static qboolean G_EndMatchUnanimousVote( void ) {
	const int	*votes, *special;
	int			count, i, voters, realVoted = 0;

	switch ( level.emvPhase ) {
	case EMV_GAMETYPE_VOTE:
		votes = level.emvGametypeVotes;
		special = level.emvGametypeCandidateSpecial;
		count = level.emvGametypeCandidateCount;
		break;
	case EMV_MAP_VOTE:
		votes = level.emvMapVotes;
		special = level.emvMapCandidateSpecial;
		count = level.emvMapCandidateCount;
		break;
	default:
		return qfalse;
	}

	// count only humans
	voters = 0;
	for ( i = 0; i < MAX_CLIENTS; i++ ) {
		gentity_t	*ent = &g_entities[i];

		if ( !ent->client ) {
			continue;
		}
		if ( ent->client->pers.connected != CON_CONNECTED ) {
			continue; // slot in use but not fully connected yet
		}
		if ( ent->r.svFlags & SVF_BOT ) {
			continue; // bots don't vote here
		}

		voters++;
		if ( level.emvClientVote[i] == -1 ) {
			return qfalse; // still waiting on this player
		}
	}

	if ( voters == 0 ) {
		return qfalse; // no human playing, nothing to short-circuit
	}

	// everyone voted: count how many distinct real options received a vote
	for ( i = 0 ; i < count ; i++ ) {
		if ( votes[i] <= 0 ) {
			continue;
		}
		if ( special[i] != EMV_OPT_NONE ) {
			continue; // skip "Don't care"
		}
		realVoted++;
		if ( realVoted > 1 ) {
			return qfalse; // two real options have votes, split
		}
	}

	return qtrue; // 0 or 1 real option(s) got a vote - unanimous
}


/*
==================
G_RunEndMatchVote
==================
*/
void G_RunEndMatchVote( void ) {
	int			winnerIndex;
	qboolean	inVotePhase;

	if ( level.emvPhase == EMV_INACTIVE ) {
		return;
	}

	inVotePhase = ( level.emvPhase == EMV_GAMETYPE_VOTE || level.emvPhase == EMV_MAP_VOTE );

	// check unanimous votes
	if ( inVotePhase && !level.emvLocked && G_EndMatchUnanimousVote() ) {
		level.emvPhaseEndTime = level.time;
		G_BroadcastEndMatchState();
	}

	// apply the vote lockout once, milliseconds into a voting phase, even if it's unanimous
	if ( inVotePhase && !level.emvLocked && level.time < level.emvPhaseEndTime ) {
		int	lockWindowMs = g_endmatch_lock_time.integer * 1000;
		int	timeLeft = level.emvPhaseEndTime - level.time;

		if ( lockWindowMs > 0 && timeLeft <= lockWindowMs ) {
			G_ApplyEndMatchLockout();
			G_BroadcastEndMatchState();
		}
	}

	if ( level.time < level.emvPhaseEndTime ) {
		return;
	}

	switch ( level.emvPhase ) {
	case EMV_SCOREBOARD:
		if ( G_CountRealGametypeCandidates() <= 1 ) {
			// only one real gametype available - skip the vote silently
			G_EndMatchEnterPhase( EMV_MAP_VOTE );
		} else {
			G_EndMatchEnterPhase( EMV_GAMETYPE_VOTE );
		}
		break;

	case EMV_GAMETYPE_VOTE:
		winnerIndex = G_PickWinner( level.emvGametypeVotes, level.emvGametypeCandidateSpecial, level.emvGametypeLockedOut, level.emvGametypeCandidateCount );
		level.emvWinnerGametype = level.emvGametypeCandidates[winnerIndex];
		G_SendEndMatchVoteResult( -1, EMV_GAMETYPE_VOTE, winnerIndex );

		if ( G_CountRealMapCandidates() <= 1 ) {
			// single-map server - nothing to vote on, just restart
			level.emvWinnerIsRestart = qtrue;
			G_SendEndMatchVoteResult( -1, EMV_MAP_VOTE, -1 );
			G_EndMatchEnterPhase( EMV_MAP_REVEAL );
		} else {
			G_EndMatchEnterPhase( EMV_MAP_VOTE );
		}
		break;

	case EMV_MAP_VOTE:
		winnerIndex = G_PickWinner( level.emvMapVotes, level.emvMapCandidateSpecial, level.emvMapLockedOut, level.emvMapCandidateCount );
		Q_strncpyz( level.emvWinnerMap, level.emvMapCandidates[winnerIndex], sizeof( level.emvWinnerMap ) );
		G_SendEndMatchVoteResult( -1, EMV_MAP_VOTE, winnerIndex );
		G_EndMatchEnterPhase( EMV_MAP_REVEAL );
		break;

	case EMV_MAP_REVEAL:
		// apply the winning gametype/map
		if ( level.emvWinnerGametype != g_gametype.integer ) {
			trap_Cvar_Set( "g_gametype", va( "%i", level.emvWinnerGametype ) );
		}
		if ( !level.emvWinnerIsRestart && level.emvWinnerMap[0] ) {
			trap_Cvar_Set( "nextmap", va( "map \"%s\"", level.emvWinnerMap ) );
		}
		// restarts the current map
		level.emvPhase = EMV_INACTIVE;
		G_BroadcastEndMatchState();

		ExitLevel();
		break;

	default:
		level.emvPhase = EMV_INACTIVE;
		break;
	}
}


/*
==================
Cmd_EndMatchVote_f

Client command: "endmatchvote <index>"
==================
*/
void Cmd_EndMatchVote_f( gentity_t *ent ) {
	char	arg[16];
	int		index, clientNum, count;
	int		*votes;
	qboolean	*lockedOut;

	if ( !ent->client ) {
		return;
	}
	clientNum = ent - g_entities;

	if ( level.emvPhase != EMV_GAMETYPE_VOTE && level.emvPhase != EMV_MAP_VOTE ) {
		trap_SendServerCommand( clientNum, "print \"No end-of-match vote in progress.\n\"" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	index = atoi( arg );

	if ( level.emvPhase == EMV_GAMETYPE_VOTE ) {
		votes = level.emvGametypeVotes;
		lockedOut = level.emvGametypeLockedOut;
		count = level.emvGametypeCandidateCount;
	} else {
		votes = level.emvMapVotes;
		lockedOut = level.emvMapLockedOut;
		count = level.emvMapCandidateCount;
	}

	if ( index < 0 || index >= count || lockedOut[index] ) {
		trap_SendServerCommand( clientNum, "print \"Invalid vote option.\n\"" );
		return;
	}

	if ( level.emvClientVote[clientNum] == index ) {
		// already voted for this exact option, nothing to do
		return;
	}

	// move the vote: remove it from whatever they picked before (if
	// anything), then add it to the new pick
	if ( level.emvClientVote[clientNum] != -1 ) {
		votes[level.emvClientVote[clientNum]]--;
	}
	votes[index]++;
	level.emvClientVote[clientNum] = index;

	// trap_SendServerCommand( clientNum, "print \"Vote cast.\n\"" );

	// let everyone see live vote counts on the vote UI
	G_BroadcastEndMatchState();
}
