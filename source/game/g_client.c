/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
#include "g_local.h"

// g_client.c -- client functions that don't happen every frame

static vec3_t	playerMins = {-15, -15, -24};
static vec3_t	playerMaxs = {15, 15, 32};
#define	MAX_SPAWN_POINTS	128

static char	ban_reason[MAX_CVAR_VALUE_STRING];

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_deathmatch( gentity_t *ent ) {
	int		i;

	G_SpawnInt( "nobots", "0", &i);
	if ( i ) {
		ent->flags |= FL_NO_BOTS;
	}
	G_SpawnInt( "nohumans", "0", &i );
	if ( i ) {
		ent->flags |= FL_NO_HUMANS;
	}
}

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
equivelant to info_player_deathmatch
*/
void SP_info_player_start(gentity_t *ent) {
	ent->classname = "info_player_deathmatch";
	SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void SP_info_player_intermission( gentity_t *ent ) {

}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
SpotWouldTelefrag

================
*/
qboolean SpotWouldTelefrag( gentity_t *spot ) {
	int			i, num;
	int			touch[MAX_CLIENTS];
	gentity_t	*hit;
	vec3_t		mins, maxs;

	VectorAdd( spot->s.origin, playerMins, mins );
	VectorAdd( spot->s.origin, playerMaxs, maxs );
	num = G_EntitiesInBox( mins, maxs, touch, level.maxclients );

	for ( i = 0; i < num ; i++ ) {
		hit = &g_entities[touch[i]];
		//if ( hit->client && hit->client->ps.stats[STAT_HEALTH] > 0 ) {
		if ( hit->client ) {
			return qtrue;
		}

	}

	return qfalse;
}

/*
================
SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
================
*/
gentity_t *SelectNearestDeathmatchSpawnPoint( vec3_t from ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist, nearestDist;
	gentity_t	*nearestSpot;

	nearestDist = 999999;
	nearestSpot = NULL;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {

		VectorSubtract( spot->s.origin, from, delta );
		dist = VectorLength( delta );
		if ( dist < nearestDist ) {
			nearestDist = dist;
			nearestSpot = spot;
		}
	}

	return nearestSpot;
}


/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
gentity_t *SelectRandomDeathmatchSpawnPoint( void ) {
	gentity_t	*spot;
	int			count;
	int			selection;
	gentity_t	*spots[MAX_SPAWN_POINTS];

	count = 0;
	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
		if ( SpotWouldTelefrag( spot ) ) {
			continue;
		}
		spots[ count ] = spot;
		count++;
	}

	if ( !count ) {	// no spots that won't telefrag
		return G_Find( NULL, FOFS(classname), "info_player_deathmatch");
	}

	selection = rand() % count;
	return spots[ selection ];
}

/*
===========
SelectRandomFurthestSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
static gentity_t *SelectRandomFurthestSpawnPoint( const gentity_t *ent, vec3_t avoidPoint, vec3_t origin, vec3_t angles ) {
	gentity_t	*spot;
	vec3_t		delta;
	float		dist;
	float		list_dist[MAX_SPAWN_POINTS];
	gentity_t	*list_spot[MAX_SPAWN_POINTS];
	int			numSpots, i, j, n;
	int			selection;
	int			checkTelefrag;
	int			checkType;
	int			checkMask;
	qboolean	isBot;

	checkType = qtrue;
	checkTelefrag = qtrue;

	if ( ent )
		isBot = ((ent->r.svFlags & SVF_BOT) == SVF_BOT); 
	else
		isBot = qfalse;

	checkMask = 3;

__search:

	checkTelefrag = checkMask & 1;
	checkType = checkMask & 2;

	numSpots = 0;
	for ( n = 0 ; n < level.numSpawnSpots ; n++ ) {
		spot = level.spawnSpots[n];

		if ( spot->fteam != TEAM_FREE && level.numSpawnSpotsFFA > 0 )
			continue;

		if ( checkTelefrag && SpotWouldTelefrag( spot ) )
			continue;

		if ( checkType ) 
		{
			if ( (spot->flags & FL_NO_BOTS) && isBot )
				continue;
			if ( (spot->flags & FL_NO_HUMANS) && !isBot )
				continue;
		}

		VectorSubtract( spot->s.origin, avoidPoint, delta );
		dist = VectorLength( delta );

		for ( i = 0; i < numSpots; i++ )
		{
			if( dist > list_dist[i] )
			{
				if (numSpots >= MAX_SPAWN_POINTS)
					numSpots = MAX_SPAWN_POINTS - 1;

				for( j = numSpots; j > i; j-- )
				{
					list_dist[j] = list_dist[j-1];
					list_spot[j] = list_spot[j-1];
				}

				list_dist[i] = dist;
				list_spot[i] = spot;

				numSpots++;
				break;
			}
		}

		if(i >= numSpots && numSpots < MAX_SPAWN_POINTS)
		{
			list_dist[numSpots] = dist;
			list_spot[numSpots] = spot;
			numSpots++;
		}
	}

	if ( !numSpots ) {
		if ( checkMask <= 0 ) {
			G_Error( "Couldn't find a spawn point" );
			return NULL;
		}
		checkMask--;
		goto __search; // next attempt with different flags
	}

	// select a random spot from the spawn points furthest away
	selection = random() * (numSpots / 2);
	spot = list_spot[ selection ];

	VectorCopy( spot->s.angles, angles );
	VectorCopy( spot->s.origin, origin );
	origin[2] += SPAWN_HEIGHT;

	return spot;
}

/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectSpawnPoint( gentity_t *ent, vec3_t avoidPoint, vec3_t origin, vec3_t angles ) {
	return SelectRandomFurthestSpawnPoint( ent, avoidPoint, origin, angles );
}

/*
===========
SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
gentity_t *SelectInitialSpawnPoint( gentity_t *ent, vec3_t origin, vec3_t angles ) {
	gentity_t	*spot;
	int n;

	spot = NULL;

	for ( n = 0; n < level.numSpawnSpotsFFA; n++ ) {
		spot = level.spawnSpots[ n ];
		if ( spot->fteam != TEAM_FREE )
			continue;
		if ( spot->spawnflags & 1 )
			break;
		else
			spot = NULL;
	}

	if ( !spot || SpotWouldTelefrag( spot ) ) {
		return SelectSpawnPoint( ent, vec3_origin, origin, angles );
	}

	VectorCopy( spot->s.angles, angles );
	VectorCopy( spot->s.origin, origin );
	origin[2] += SPAWN_HEIGHT;

	return spot;
}

/*
===========
SelectSpectatorSpawnPoint

============
*/
gentity_t *SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles ) {
	FindIntermissionPoint();

	VectorCopy( level.intermission_origin, origin );
	VectorCopy( level.intermission_angle, angles );

	return level.spawnSpots[ SPAWN_SPOT_INTERMISSION ]; // was NULL
}

/*
=======================================================================

BODYQUE

=======================================================================
*/

/*
===============
InitBodyQue
===============
*/
void InitBodyQue (void) {
	int		i;
	gentity_t	*ent;

	level.bodyQueIndex = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++) {
		ent = G_Spawn();
		ent->classname = "bodyque";
		ent->neverFree = qtrue;
		level.bodyQue[i] = ent;
	}
}

/*
=============
BodySink

After sitting around for five seconds, fall into the ground and dissapear
=============
*/
void BodySink( gentity_t *ent ) {
	if ( level.time - ent->timestamp > 6500 ) {
		// the body ques are never actually freed, they are just unlinked
		trap_UnlinkEntity( ent );
		ent->physicsObject = qfalse;
		return;	
	}
	ent->nextthink = level.time + 100;
	ent->s.pos.trBase[2] -= 1;
}

/*
=============
CopyToBodyQue

A player is respawning, so make an entity that looks
just like the existing corpse to leave behind.
=============
*/
void CopyToBodyQue( gentity_t *ent ) {
	gentity_t		*body;
	int			contents;

	trap_UnlinkEntity (ent);

	// don't leave a corpse if already gibbed
	if ( ent->s.eType == ET_INVISIBLE && ent->health <= GIB_HEALTH ) {
		return;
	}

	// if client is in a nodrop area, don't leave the body
	contents = trap_PointContents( ent->s.origin, -1 );
	if ( contents & CONTENTS_NODROP ) {
		return;
	}

	// grab a body que and cycle to the next one
	body = level.bodyQue[ level.bodyQueIndex ];
	level.bodyQueIndex = (level.bodyQueIndex + 1) % BODY_QUEUE_SIZE;

	trap_UnlinkEntity (body);

	body->s = ent->s;
	body->s.eFlags = EF_DEAD;		// clear EF_TALK, etc
	body->s.powerups = 0;	// clear powerups
	body->s.loopSound = 0;	// clear lava burning
	body->s.number = body - g_entities;
	body->timestamp = level.time;
	body->physicsObject = qtrue;
	body->physicsBounce = 0;		// don't bounce
	if ( body->s.groundEntityNum == ENTITYNUM_NONE ) {
		body->s.pos.trType = TR_GRAVITY;
		body->s.pos.trTime = level.time;
		VectorCopy( ent->client->ps.velocity, body->s.pos.trDelta );
	} else {
		body->s.pos.trType = TR_STATIONARY;
	}
	body->s.event = 0;

	// change the animation to the last-frame only, so the sequence
	// doesn't repeat anew for the body
	switch ( body->s.legsAnim & ~ANIM_TOGGLEBIT ) {
	case BOTH_DEATH1:
	case BOTH_DEAD1:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD1;
		break;
	case BOTH_DEATH2:
	case BOTH_DEAD2:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD2;
		break;
	case BOTH_DEATH3:
	case BOTH_DEAD3:
	default:
		body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD3;
		break;
	}

	body->r.svFlags = ent->r.svFlags;
	VectorCopy (ent->r.mins, body->r.mins);
	VectorCopy (ent->r.maxs, body->r.maxs);
	VectorCopy (ent->r.absmin, body->r.absmin);
	VectorCopy (ent->r.absmax, body->r.absmax);

	body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
	body->r.contents = CONTENTS_CORPSE;
	body->r.ownerNum = ent->s.number;

	body->nextthink = level.time + 5000;
	body->think = BodySink;

	body->die = body_die;

	// BFP - Attention: DLL & SO are prone to crash here, so setting body->takedamage as qfalse avoids that
	body->takedamage = qfalse;
#if 0
	// don't take more damage if already gibbed
	if ( ent->health <= GIB_HEALTH ) {
		body->takedamage = qfalse;
	} else {
		body->takedamage = qtrue;
	}
#endif


	VectorCopy ( body->s.pos.trBase, body->r.currentOrigin );
	trap_LinkEntity (body);
}

//======================================================================


/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle( gentity_t *ent, vec3_t angle ) {
	int			i;

	// set the delta angle
	for (i=0 ; i<3 ; i++) {
		int		cmdAngle;

		cmdAngle = ANGLE2SHORT(angle[i]);
		ent->client->ps.delta_angles[i] = cmdAngle - ent->client->pers.cmd.angles[i];
	}
	VectorCopy( angle, ent->s.angles );
	VectorCopy (ent->s.angles, ent->client->ps.viewangles);
}

/*
================
respawn
================
*/
void respawn( gentity_t *ent ) {
	gentity_t	*tent;

	CopyToBodyQue (ent);
	ClientSpawn(ent);

	// add a teleportation effect
	tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
	tent->s.clientNum = ent->s.clientNum;
}

/*
================
TeamCount

Returns number of players on a team
================
*/
team_t TeamCount( int ignoreClientNum, int team ) {
	int		i;
	int		count = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( i == ignoreClientNum ) {
			continue;
		}
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == team ) {
			count++;
		}
	}

	return count;
}

/*
================
TeamLeader

Returns the client number of the team leader
================
*/
int TeamLeader( int team ) {
	int		i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == team ) {
			if ( level.clients[i].sess.teamLeader )
				return i;
		}
	}

	return -1;
}


/*
================
PickTeam

================
*/
team_t PickTeam( int ignoreClientNum ) {
	int		counts[TEAM_NUM_TEAMS];

	counts[TEAM_BLUE] = TeamCount( ignoreClientNum, TEAM_BLUE );
	counts[TEAM_RED] = TeamCount( ignoreClientNum, TEAM_RED );

	if ( counts[TEAM_BLUE] > counts[TEAM_RED] ) {
		return TEAM_RED;
	}
	if ( counts[TEAM_RED] > counts[TEAM_BLUE] ) {
		return TEAM_BLUE;
	}
	// equal team count, so join the team with the lowest score
	if ( level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED] ) {
		return TEAM_RED;
	}
	return TEAM_BLUE;
}

/*
===========
ForceClientSkin

Forces a client's skin (for teamplay)
===========
*/
/*
static void ForceClientSkin( gclient_t *client, char *model, const char *skin ) {
	char *p;

	if ((p = Q_strrchr(model, '/')) != 0) {
		*p = 0;
	}

	Q_strcat(model, MAX_QPATH, "/");
	Q_strcat(model, MAX_QPATH, skin);
}
*/

/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap_SetUserinfo
if desired.
============
*/
qboolean ClientUserinfoChanged( int clientNum ) {
	gentity_t *ent;
	team_t		team;
	// BFP - No handicap for health
	// int		health;
	char	*s;
	char	model[MAX_QPATH];
	// BFP - Check model in list
	char	modelCheck[MAX_QPATH];
	char	oldname[MAX_STRING_CHARS];
	gclient_t	*client;
	// BFP - No color1
	// char	c1[MAX_INFO_STRING];
	char	userinfo[MAX_INFO_STRING];

	// BFP - Model prefix load
	char newModelPrefix[MAX_QPATH];
	char *oldModelDash, *newModelDash;

	ent = g_entities + clientNum;
	client = ent->client;

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	// check for malformed or illegal info strings
	if ( !Info_Validate( userinfo ) ) {
		Q_strcpy( ban_reason, "bad userinfo" );
		if ( client && client->pers.connected != CON_DISCONNECTED )
			trap_DropClient( clientNum, ban_reason );
		return qfalse;
	}

	if ( client->pers.connected == CON_DISCONNECTED ) {
		// we just checked if connecting player can join server
		// so quit now as some important data like player team is still not set
		return qtrue;
	}

	// check for local client
	s = Info_ValueForKey( userinfo, "ip" );
	if ( !strcmp( s, "localhost" ) ) {
		client->pers.localClient = qtrue;
	}

	// check the item prediction
	s = Info_ValueForKey( userinfo, "cg_predictItems" );
	if ( !atoi( s ) ) {
		client->pers.predictItemPickup = qfalse;
	} else {
		client->pers.predictItemPickup = qtrue;
	}

	// set name
	Q_strncpyz ( oldname, client->pers.netname, sizeof( oldname ) );
	s = Info_ValueForKey (userinfo, "name");
	BG_CleanName( s, client->pers.netname, sizeof( client->pers.netname ), "UnnamedPlayer" );

	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
			Q_strncpyz( client->pers.netname, "scoreboard", sizeof(client->pers.netname) );
		}
	}

	if ( client->pers.connected == CON_CONNECTED ) {
		if ( strcmp( oldname, client->pers.netname ) ) {
			trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " renamed to %s\n\"", oldname, 
				client->pers.netname) );
		}
	}

	// BFP - No handicap
#if 0
	// set max health
	health = atoi( Info_ValueForKey( userinfo, "handicap" ) );
	client->pers.maxHealth = health;
	if ( client->pers.maxHealth < 1 || client->pers.maxHealth > 100 ) {
		client->pers.maxHealth = 100;
	}
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;
#endif

	client->ps.stats[STAT_MAX_HEALTH] = 1 + client->ps.persistant[PERS_POWERLEVEL];
	if ( client->ps.stats[STAT_MAX_HEALTH] > 1000 ) {
		client->ps.stats[STAT_MAX_HEALTH] = 1000;
	}

	// BFP - Monster gamemode, double max health for player monster
	if ( g_gametype.integer == GT_MONSTER 
	&& level.monsterClientNum == clientNum
	&& ( client->ps.eFlags & EF_MONSTER ) ) {
		client->ps.stats[STAT_MAX_HEALTH] *= 2;
		if ( client->ps.stats[STAT_MAX_HEALTH] > 2000 ) {
			client->ps.stats[STAT_MAX_HEALTH] = 2000;
		}
	}

	// set model
	// BFP - Resolve player model when loading by prefix
	G_ResolvePlayerModel( userinfo, model, model, sizeof( model ) );

	// BFP - BFP WEAPON CONFIG: Recompute the 5 attack slots -> weaponNum cache for this model
	BG_SetClientAttackWeaponNums( clientNum, model );

	// BFP - Kick/force to spectate the player who uses an illegal model which isn't available in the server
	Q_strncpyz( modelCheck, G_GetPlayerModelName( clientNum, userinfo ), sizeof( modelCheck ) );
	if ( !G_PlayerModelExistsOnServer( modelCheck )
	&& ( g_gametype.integer != GT_MONSTER
	|| ( g_gametype.integer == GT_MONSTER && g_monster.integer < 1 ) ) ) {
#if KICK_ILLEGAL_PLAYER_MODEL
		if ( client && client->pers.connected != CON_DISCONNECTED ) {
			trap_DropClient( clientNum, "was kicked" );
		}
#else
		gentity_t *tempEnt = G_TempEntity( ent->r.currentOrigin, EV_OBITUARY );
		tempEnt->s.eventParm = MOD_ILLEGAL_PLAYER_MODEL;
		tempEnt->r.svFlags = SVF_BROADCAST;
		client->sess.sessionTeam = TEAM_SPECTATOR;
		client->sess.spectatorState = SPECTATOR_FREE;
		client->sess.spectatorClient = 0;
		client->sess.teamLeader = qfalse;
		ClientBegin( clientNum );
#endif
		return qfalse;
	}

	// bots set their team a few frames later
	if (g_gametype.integer >= GT_TEAM && g_entities[clientNum].r.svFlags & SVF_BOT) {
		s = Info_ValueForKey( userinfo, "team" );
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
			team = TEAM_RED;
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
			team = TEAM_BLUE;
		} else {
			// pick the team with the least number of players
			team = PickTeam( clientNum );
		}
	}
	else {
		team = client->sess.sessionTeam;
	}

	// teamInfo
	s = Info_ValueForKey( userinfo, "teamoverlay" );
	if ( ! *s || atoi( s ) != 0 ) {
		client->pers.teamInfo = qtrue;
	} else {
		client->pers.teamInfo = qfalse;
	}

	// BFP - No color1
#if 0
	// colors
	strcpy(c1, Info_ValueForKey( userinfo, "color1" ));
#endif

	// send over a subset of the userinfo keys so other clients can
	// print scoreboards, display models, and play custom sounds
	if ( ent->r.svFlags & SVF_BOT ) {
		s = va("n\\%s\\t\\%i\\model\\%s\\w\\%i\\l\\%i\\skill\\%s",
			client->pers.netname, team, model, 
			client->sess.wins, client->sess.losses,
			Info_ValueForKey( userinfo, "skill" ) );
	} else {
		s = va("n\\%s\\t\\%i\\model\\%s\\w\\%i\\l\\%i",
			client->pers.netname, client->sess.sessionTeam, model, 
			client->sess.wins, client->sess.losses );
	}

	trap_SetConfigstring( CS_PLAYERS+clientNum, s );

	// BFP - Model prefix handling
	{
		// extract model prefixes safely
		oldModelDash = strchr(ent->oldModel, '-');
		if ( oldModelDash ) {
			Q_strncpyz( ent->oldModelPrefix, ent->oldModel, oldModelDash - ent->oldModel + 1 );
		} else {
			Q_strncpyz( ent->oldModelPrefix, ent->oldModel, sizeof( ent->oldModelPrefix ) );
		}

		newModelDash = strchr(model, '-');
		if ( newModelDash ) {
			Q_strncpyz( newModelPrefix, model, newModelDash - model + 1 );
		} else {
			Q_strncpyz( newModelPrefix, model, sizeof( newModelPrefix ) );
		}

		// compare model prefixes
		if ( Q_stricmp( ent->oldModelPrefix, newModelPrefix )
		&& ent->client->sess.sessionTeam != TEAM_SPECTATOR ) { // only when the player is playing
			// prefixes differ, kill the player
			ent->flags &= ~FL_GODMODE;
			ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
			player_die( ent, ent, NULL, 100000, MOD_UNKNOWN );
		}

		// save the new model as the old model for the next time this function runs
		Q_strncpyz( ent->oldModel, model, sizeof( ent->oldModel ) );
	}

	// BFP - Send powerlevel info to cgame reusing frame from entityState_t struct
	ent->s.frame = client->ps.persistant[PERS_POWERLEVEL];

	// this is not the userinfo, more like the configstring actually
	G_LogPrintf( "ClientUserinfoChanged: %i %s\n", clientNum, s );

	return qtrue;
}

/*
===========
ClientGetAveragePowerlevel

Get the average powerlevel when there are another players with 
different powerlevels.
============
*/
static int ClientGetAveragePowerlevel( void ) { // BFP - Average powerlevel
	int		i = 0;
	int		totalPowerLevel = 0;
	int		activeClients = 0;

	while ( i < level.numConnectedClients ) {
		gentity_t *ent = &g_entities[level.sortedClients[i]];

		if ( ent->inuse && ent->client ) {
			totalPowerLevel += ent->client->ps.persistant[PERS_POWERLEVEL];
			++activeClients;
		}

		// BFP - Survival
		if ( g_gametype.integer == GT_SURVIVAL 
		&& ent->inuse && ent->client && ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
			// get the same powerlevel of the standing one for the sake of balance
			// avoid division by zero
			return ( activeClients > 0 ) ? ent->client->ps.persistant[PERS_POWERLEVEL] : 0;
		}
		++i;
	}

	// avoid division by zero
	return ( activeClients > 0 ) ? ( totalPowerLevel / activeClients ) : 0;
}

/*
==================
ClientBecomeMonster

Sets the player monster status and properties.
==================
*/
static void ClientBecomeMonster( gentity_t *ent ) { // BFP - Monster gamemode function to set the player monster status
	// double health and max health
	ent->client->ps.stats[STAT_MAX_HEALTH] *= 2;
    if ( ent->client->ps.stats[STAT_MAX_HEALTH] > 2000 ) {
        ent->client->ps.stats[STAT_MAX_HEALTH] = 2000;
    }
	ent->health = ent->client->ps.stats[STAT_HEALTH] = ent->client->ps.stats[STAT_MAX_HEALTH];

	// double ki and max ki
	ent->client->ps.stats[STAT_MAX_KI] *= 2;
	ent->client->ps.stats[STAT_KI] = ent->client->ps.stats[STAT_MAX_KI];

	ent->client->ps.eFlags |= EF_MONSTER;

	trap_SendServerCommand( -1, va("print \"%s is the monster\n\"", ent->client->pers.netname) );
}

/*
===========
ClientCheckMonsterGone

Check if the player monster is gone and set the monster 
to the most waited player.
============
*/
void ClientCheckMonsterGone( gentity_t *ent ) { // BFP - Monster gamemode function check
	if ( g_gametype.integer == GT_MONSTER 
	&& ( ent->client->ps.eFlags & EF_MONSTER )
	&& ent->client->ps.clientNum == level.monsterClientNum ) {
		qboolean becameMonster = qfalse;
		int i;

		ent->client->ps.eFlags &= ~EF_MONSTER;

		for ( i = 0 ; i < level.maxclients ; ++i ) {
			if ( g_entities[i].client->pers.connected == CON_CONNECTED
			&& g_entities[i].client->sess.sessionTeam != TEAM_SPECTATOR
			&& ent->client->ps.clientNum != g_entities[i].client->ps.clientNum ) {
				level.monsterClientNum = g_entities[i].client->ps.clientNum;
				g_entities[i].client->ps.eFlags |= EF_MONSTER;
				becameMonster = qtrue;
				respawn( &g_entities[i] );
				break;
			}
		}
		// a check to detect if no one is here, so reset the value, otherwise whoever joins won't become a monster
		if ( !becameMonster ) {
			level.monsterClientNum = -1;
		}
	}
}

/*
=============
ClientSetAttack
=============
*/
void ClientSetAttack( gclient_t *client, int slot, bfpWeaponDef_t *def ) { // BFP - Set attack
	switch ( def->attackType ) {
	case ATK_BEAM:
		client->ps.ammo[ slot ] = AMMOF_ATK_BEAM;
		break;
	case ATK_SBEAM:
		client->ps.ammo[ slot ] = AMMOF_ATK_SBEAM;
		break;
	case ATK_FORCEFIELD:
		client->ps.ammo[ slot ] = AMMOF_ATK_FORCEFIELD;
		break;
	default:
		client->ps.ammo[ slot ] = AMMOF_ACTIVE;
		break;
	}
	if ( def->chargeAttack ) {
		client->ps.ammo[ slot ] |= AMMOF_CHARGEATTACK;
	}
	if ( def->chargeAutoFire ) {
		client->ps.ammo[ slot ] |= AMMOF_CHARGEAUTOFIRE;
	}
	if ( def->loopingAnim ) {
		client->ps.ammo[ slot ] |= AMMOF_LOOPINGANIM;
	}
	if ( def->noAttackAnim ) {
		client->ps.ammo[ slot ] |= AMMOF_NOATTACKANIM;
	}
	// BFP - Debug ammo states for weapons
#if 0
	Com_Printf( "client->ps.ammo[ %d ] & AMMOF_ATK_BEAM: %d\n", slot, client->ps.ammo[ slot ] & AMMOF_ATK_BEAM );
	Com_Printf( "client->ps.ammo[ %d ] & AMMOF_ATK_SBEAM: %d\n", slot, client->ps.ammo[ slot ] & AMMOF_ATK_SBEAM );
	Com_Printf( "client->ps.ammo[ %d ] & AMMOF_ATK_FORCEFIELD: %d\n", slot, client->ps.ammo[ slot ] & AMMOF_ATK_FORCEFIELD );
	Com_Printf( "client->ps.ammo[ %d ] & AMMOF_CHARGEATTACK: %d\n", slot, client->ps.ammo[ slot ] & AMMOF_CHARGEATTACK );
	Com_Printf( "client->ps.ammo[ %d ] & AMMOF_CHARGEAUTOFIRE: %d\n", slot, client->ps.ammo[ slot ] & AMMOF_CHARGEAUTOFIRE );
	Com_Printf( "client->ps.ammo[ %d ] & AMMOF_LOOPINGANIM: %d\n", slot, client->ps.ammo[ slot ] & AMMOF_LOOPINGANIM );
	Com_Printf( "client->ps.ammo[ %d ] & AMMOF_NOATTACKANIM: %d\n", slot, client->ps.ammo[ slot ] & AMMOF_NOATTACKANIM );
#endif
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and tournement
restarts.
============
*/
const char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
	char		*value;
//	char		*areabits;
	gclient_t	*client;
	char		userinfo[MAX_INFO_STRING];
	gentity_t	*ent;
	qboolean	isAdmin = qfalse;

	if ( clientNum >= level.maxclients ) {
		return "Bad connection slot.";
	}

	ent = &g_entities[ clientNum ];
	ent->client = level.clients + clientNum;

	if ( firstTime ) {
		// cleanup previous data manually
		// because client may silently (re)connect without ClientDisconnect in case of crash for example
		if ( level.clients[ clientNum ].pers.connected != CON_DISCONNECTED )
			ClientDisconnect( clientNum );

		// remove old entity from the world
		trap_UnlinkEntity( ent );
		ent->r.contents = 0;
		ent->s.eType = ET_INVISIBLE;
		ent->s.eFlags = 0;
		ent->s.modelindex = 0;
		ent->s.clientNum = clientNum;
		ent->s.number = clientNum;
		ent->takedamage = qfalse;
	}

	ent->r.svFlags &= ~SVF_BOT;
	ent->inuse = qfalse;

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

 	// IP filtering
 	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=500
 	// recommanding PB based IP / GUID banning, the builtin system is pretty limited
 	// check to see if they are on the banned IP list
	value = Info_ValueForKey( userinfo, "ip" );

	if ( !strcmp( value, "localhost" ) && !isBot )
		isAdmin = qtrue;

	if ( !isAdmin && G_FilterPacket( value ) ) {
		return "You are banned from this server.";
	}

	// we don't check password for bots and local client
	// NOTE: local client <-> "ip" "localhost"
	// this means this client is not running in our current process
	if ( !isBot && !isAdmin ) {
		// check for a password
		if ( g_password.string[0] && Q_stricmp( g_password.string, "none" ) ) {
			value = Info_ValueForKey( userinfo, "password" );
			if ( strcmp( g_password.string, value ) )
				return "Invalid password";
		}
	}

	// they can connect
	ent->client = level.clients + clientNum;
	client = ent->client;

	// BFP - Monster gamemode
	if ( level.monsterClientNum == -1 ) {
		// first player becomes the monster
		level.monsterClientNum = clientNum;
	}

//	areabits = client->areabits;

	memset( client, 0, sizeof(*client) );
	
	// BFP - Monster gamemode
	if ( clientNum == level.monsterClientNum ) {
		client->ps.eFlags |= EF_MONSTER;
	}

	if ( !ClientUserinfoChanged( clientNum ) ) {
		return ban_reason;
	}

	// BFP - Survival
	if ( g_gametype.integer == GT_SURVIVAL ) {
		// make sure the player isn't playing and must be spectator
		// when the match is restarted or the map is changed
		if ( !firstTime && client->sess.sessionTeam != TEAM_SPECTATOR ) {
			client->sess.sessionTeam = TEAM_SPECTATOR;
			// output "joined to ..." message
			BroadcastTeamChange( client, -1 );
		}

		// output "joined to ..." message
		if ( level.newSession ) {
			BroadcastTeamChange( client, -1 );
		}

		// make sure the player score isn't negative when being connected at that moment
		if ( client->ps.persistant[PERS_SCORE] < 0 ) {
			client->ps.persistant[PERS_SCORE] = 0;
		}
	}

	// BFP - Team Last Man Standing
	if ( g_gametype.integer == GT_TLMS ) {
		client->forceToSpectate = qfalse;
	}

	// read or initialize the session data
	if ( firstTime || level.newSession ) {
		value = Info_ValueForKey( userinfo, "team" );
		G_InitSessionData( client, value, isBot );
		G_WriteClientSessionData( client );
	}

	G_ReadClientSessionData( client );

	if( isBot ) {
		if( !G_BotConnect( clientNum, !firstTime ) ) {
			return "BotConnectfailed";
		}
		ent->r.svFlags |= SVF_BOT;
		client->sess.spectatorClient = clientNum;
	}
	ent->inuse = qtrue;

	// get and distribute relevant paramters
	G_LogPrintf( "ClientConnect: %i\n", clientNum );

	client->pers.connected = CON_CONNECTING;

	ClientUserinfoChanged( clientNum );

	// don't do the "xxx connected" messages if they were caried over from previous level
	if ( firstTime ) {
		trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " connected\n\"", client->pers.netname) );
	}

	// count current clients and rank for scoreboard
	CalculateRanks();

	// for statistics
//	client->areabits = areabits;
//	if ( !client->areabits )
//		client->areabits = G_Alloc( (trap_AAS_PointReachabilityAreaIndex( NULL ) + 7) / 8 );

	return NULL;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void ClientBegin( int clientNum ) {
	gentity_t	*ent;
	gclient_t	*client;
	gentity_t	*tent;
	int			flags;
	// BFP - Survival, save scores if the players are spectating
	int			savedScore;

	ent = g_entities + clientNum;

	client = level.clients + clientNum;

	if ( ent->r.linked ) {
		trap_UnlinkEntity( ent );
	}
	G_InitGentity( ent );
	ent->touch = 0;
	ent->pain = 0;
	ent->client = client;

	client->pers.connected = CON_CONNECTED;
	client->pers.enterTime = level.time;
	client->pers.teamState.state = TEAM_BEGIN;
	// BFP - Survival, save scores if the players are spectating
	savedScore = client->ps.persistant[PERS_SCORE];

	// save eflags around this, because changing teams will
	// cause this to happen with a valid entity, and we
	// want to make sure the teleport bit is set right
	// so the viewpoint doesn't interpolate through the
	// world to the new position
	flags = client->ps.eFlags;
	memset( &client->ps, 0, sizeof( client->ps ) );
	client->ps.eFlags = flags;

	// BFP - Monster gamemode
	if ( level.monsterClientNum == -1 ) {
		// first player becomes the monster
		level.monsterClientNum = clientNum;
	}
	if ( clientNum == level.monsterClientNum ) {
		client->ps.eFlags |= EF_MONSTER;
	}

	// BFP - Survival, save scores if the players are spectating
	if ( g_gametype.integer == GT_SURVIVAL ) {
		client->ps.persistant[PERS_SCORE] = savedScore;
	}

	// locate ent at a spawn point
	ClientSpawn( ent );

	// BFP - Kick the player who uses an illegal player model which isn't in the server
#if KICK_ILLEGAL_PLAYER_MODEL
	{
		char	userinfo[MAX_INFO_STRING];
		char	model[MAX_QPATH];

		trap_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );
		Q_strncpyz( model, G_GetPlayerModelName( clientNum, userinfo ), sizeof( model ) );

		if ( !G_PlayerModelExistsOnServer( model )
		&& ( g_gametype.integer != GT_MONSTER
		|| ( g_gametype.integer == GT_MONSTER && g_monster.integer < 1 ) ) ) {
			if ( client && client->pers.connected != CON_DISCONNECTED ) {
				trap_DropClient( clientNum, "was kicked" );
			}
			return;
		}
	}
#endif

	if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
		// send event
		tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = ent->s.clientNum;

		if ( g_gametype.integer != GT_TOURNAMENT 
		&& g_gametype.integer != GT_SURVIVAL ) { // BFP - Survival
			trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " entered the game\n\"", client->pers.netname) );
		}
	}
	G_LogPrintf( "ClientBegin: %i\n", clientNum );

	// count current clients and rank for scoreboard
	CalculateRanks();
}

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
void ClientSpawn(gentity_t *ent) {
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	clientPersistant_t	saved;
	clientSession_t		savedSess;
	int		persistant[MAX_PERSISTANT];
	gentity_t	*spawnPoint;
	int		flags;
	int		savedPing;
	// BFP - Team Last Man Standing, save force to spectate and selected team
	qboolean	savedForcedToSpectate;
	team_t	savedSelectedTeam;
//	char	*savedAreaBits;
	int		accuracy_hits, accuracy_shots;
	int		eventSequence;
	char	userinfo[MAX_INFO_STRING];

	index = ent - g_entities;
	client = ent->client;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
		spawnPoint = SelectSpectatorSpawnPoint ( 
						spawn_origin, spawn_angles);
	} else if (g_gametype.integer >= GT_CTF ) {
		// all base oriented team games use the CTF spawn points
		spawnPoint = SelectCTFSpawnPoint ( 
						ent,
						client->sess.sessionTeam, 
						client->pers.teamState.state, 
						spawn_origin, spawn_angles);
	} else {
		do {
			// BFP - Monster gamemode, the player monster spawns at the spectator spawn point, kinda curious (¬_¬)
			if ( g_gametype.integer == GT_MONSTER
			&& ( client->ps.eFlags & EF_MONSTER ) ) {
				spawnPoint = SelectSpectatorSpawnPoint ( spawn_origin, spawn_angles );
				break;
			}
			// the first spawn should be at a good looking spot
			if ( !client->pers.initialSpawn && client->pers.localClient ) {
				client->pers.initialSpawn = qtrue;
				spawnPoint = SelectInitialSpawnPoint( ent, spawn_origin, spawn_angles );
			} else {
				// don't spawn near existing origin if possible
				spawnPoint = SelectSpawnPoint ( 
					ent,
					client->ps.origin, 
					spawn_origin, spawn_angles);
			}

			// Tim needs to prevent bots from spawning at the initial point
			// on q3dm0...
			if ( ( spawnPoint->flags & FL_NO_BOTS ) && ( ent->r.svFlags & SVF_BOT ) ) {
				continue;	// try again
			}
			// just to be symetric, we have a nohumans option...
			if ( ( spawnPoint->flags & FL_NO_HUMANS ) && !( ent->r.svFlags & SVF_BOT ) ) {
				continue;	// try again
			}

			break;

		} while ( 1 );
	}
	client->pers.teamState.state = TEAM_ACTIVE;

	// toggle the teleport bit so the client knows to not lerp
	// and never clear the voted flag
	flags = ent->client->ps.eFlags & (EF_TELEPORT_BIT | EF_VOTED | EF_TEAMVOTED);
	flags ^= EF_TELEPORT_BIT;

	// clear everything but the persistant data

	saved = client->pers;
	savedSess = client->sess;
	// BFP - Team Last Man Standing, save force to spectate and selected team
	savedForcedToSpectate = client->forceToSpectate;
	savedSelectedTeam = client->selectedTeam;

	savedPing = client->ps.ping;
//	savedAreaBits = client->areabits;
	accuracy_hits = client->accuracy_hits;
	accuracy_shots = client->accuracy_shots;
	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		persistant[i] = client->ps.persistant[i];
	}
	eventSequence = client->ps.eventSequence;

	Com_Memset (client, 0, sizeof(*client));

	client->pers = saved;
	client->sess = savedSess;
	// BFP - Team Last Man Standing, keep force to spectate and selected team
	client->forceToSpectate = savedForcedToSpectate;
	client->selectedTeam = savedSelectedTeam;

	client->ps.ping = savedPing;
//	client->areabits = savedAreaBits;
	client->accuracy_hits = accuracy_hits;
	client->accuracy_shots = accuracy_shots;
	client->lastkilled_client = -1;

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		client->ps.persistant[i] = persistant[i];
	}
	client->ps.eventSequence = eventSequence;
	// increment the spawncount so the client will detect the respawn
	client->ps.persistant[PERS_SPAWN_COUNT]++;
	client->ps.persistant[PERS_TEAM] = client->sess.sessionTeam;

	// BFP - No drowning
	// client->airOutTime = level.time + 12000;

	trap_GetUserinfo( index, userinfo, sizeof(userinfo) );
	// BFP - No handicap
#if 0
	// set max health
	client->pers.maxHealth = atoi( Info_ValueForKey( userinfo, "handicap" ) );
	if ( client->pers.maxHealth < 1 || client->pers.maxHealth > 100 ) {
		client->pers.maxHealth = 100;
	}
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;
#endif

	client->ps.eFlags = flags;

	// clear entity values
	ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->client = &level.clients[index];
	ent->takedamage = qtrue;
	ent->inuse = qtrue;
	ent->classname = "player";
	ent->r.contents = CONTENTS_BODY;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags = 0;
	
	VectorCopy (playerMins, ent->r.mins);
	VectorCopy (playerMaxs, ent->r.maxs);

	client->ps.clientNum = index;

	// BFP - TODO: list of 5 skills

	// BFP - Powerlevel start
	if ( client->ps.persistant[PERS_POWERLEVEL] < g_basePL.integer ) {
		client->ps.persistant[PERS_POWERLEVEL] = g_basePL.integer;
	} else {
		if ( level.numConnectedClients > 1 ) {
			client->ps.persistant[PERS_POWERLEVEL] = ClientGetAveragePowerlevel();
		}
	}
	if ( client->ps.persistant[PERS_POWERLEVEL] > 1000 || g_basePL.integer > 998 ) {
		client->ps.persistant[PERS_POWERLEVEL] = 1000;
	}

	// BFP - Max spawn powerlevel, only when g_maxSpawnPL is higher than 0
	if ( g_maxSpawnPL.integer > 0 && client->ps.persistant[PERS_POWERLEVEL] > g_maxSpawnPL.integer
	&& g_basePL.integer < 999 ) {
		client->ps.persistant[PERS_POWERLEVEL] = g_maxSpawnPL.integer;
	}

	// BFP - Team Last Man Standing, the powerlevel always starts at the maximum and reset spectating reason
	if ( g_gametype.integer == GT_TLMS ) {
		client->ps.persistant[PERS_POWERLEVEL] = 1000;
	}
	if ( client->forceToSpectate <= 0 ) {
		client->forceToSpectate = qfalse;
	}

	// BFP - Send powerlevel info to cgame reusing frame from entityState_t struct
	ent->s.frame = client->ps.persistant[PERS_POWERLEVEL];
	
	// BFP - Max health start
	client->ps.stats[STAT_MAX_HEALTH] = 1 + client->ps.persistant[PERS_POWERLEVEL];
	if ( client->ps.stats[STAT_MAX_HEALTH] > 1000 ) {
		client->ps.stats[STAT_MAX_HEALTH] = 1000;
	}

	// BFP - Ki start
	client->ps.stats[STAT_KI] = 999;
	// BFP - NOTE: What the heck? Did BFP dev make this multiplying 9.00825 with powerlevel? Strange approximation...
	client->ps.stats[STAT_KI] = client->ps.stats[STAT_KI] + ( 9.00825 * client->ps.persistant[PERS_POWERLEVEL] );
	client->ps.stats[STAT_MAX_KI] = client->ps.stats[STAT_KI];

	if ( client->ps.stats[STAT_MAX_KI] > 10000 ) {
		client->ps.stats[STAT_MAX_KI] = client->ps.stats[STAT_KI] = 10000;
	}

	// BFP - Monster gamemode
	if ( g_gametype.integer == GT_MONSTER && g_monster.integer > 0
	&& client->ps.clientNum == level.monsterClientNum ) {
		bfpWeaponDef_t	*def = BG_SetMonsterDefaultWeaponDef();
		client->ps.stats[STAT_WEAPONS] = ( 1 << WP_NONE );
		client->ps.ammo[WP_NONE] = 1;
		if ( def ) {
			ClientSetAttack( client, WP_NONE, def );
		}
	} else {
		int	slot;

		client->ps.stats[STAT_WEAPONS] = ( 1 << WP_NONE ) | ( 1 << WP_GAUNTLET ) | ( 1 << WP_MACHINEGUN ) | ( 1 << WP_SHOTGUN ) | ( 1 << WP_GRENADE_LAUNCHER );
		for ( slot = 0; slot < BFP_NUM_WEAPONS; slot++ ) {
			bfpWeaponDef_t	*def = BG_GetClientWeaponDefForSlot( client->ps.clientNum, slot );
			if ( !def ) {
				def = BG_SetDefaultWeaponDef();
			}
			if ( def ) {
				ClientSetAttack( client, slot, def );
			}
		}
	}

	// health will count down towards max_health
	ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH]; // BFP - Before Q3: + 25

	// BFP - Monster gamemode
	if ( g_gametype.integer == GT_MONSTER && client->ps.clientNum == level.monsterClientNum
	&& ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		ClientBecomeMonster( ent );
	}

	G_SetOrigin( ent, spawn_origin );
	VectorCopy( spawn_origin, client->ps.origin );

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps.pm_flags |= PMF_RESPAWNED;
	client->ps.viewheight = DEFAULT_VIEWHEIGHT;
	client->ps.gravity = g_gravity.integer;
	client->ps.speed = g_speed.integer;

	trap_GetUsercmd( client - level.clients, &client->pers.cmd );
	SetClientViewAngle( ent, spawn_angles );

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		// BFP - Monster gamemode, if this guy spectated, respawn the other guy who can become a monster
		ClientCheckMonsterGone( ent );
	} else {
		G_KillBox( ent );
		trap_LinkEntity (ent);

		// force the base weapon up
		client->ps.weapon = WP_NONE;
		client->ps.weaponstate = WEAPON_READY;

	}

	// don't allow full run speed for a bit
	client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	client->ps.pm_time = 100;

	client->respawnTime = level.time;
	client->inactivityTime = level.time + g_inactivity.integer * 1000;
	client->latched_buttons = 0;

	// set default animations
	client->ps.torsoAnim = TORSO_STAND;
	client->ps.legsAnim = LEGS_IDLE;

	if ( level.intermissiontime ) {
		MoveClientToIntermission( ent );
	} else {
		// fire the targets of the spawn point
		G_UseTargets( spawnPoint, ent );

		// select the highest weapon number available, after any
		// spawn given items have fired
		client->ps.weapon = WP_NONE;	// BFP - First attack to be selected freely
		// BFP - Make the first attack selected instead
#if 0
		for ( i = BFP_NUM_WEAPONS - 1 ; i > 0 ; i-- ) {
			if ( client->ps.stats[STAT_WEAPONS] & ( 1 << i ) ) {
				client->ps.weapon = i;
				break;
			}
		}
#endif
	}

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps.commandTime = level.time - 100;
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink( ent-g_entities );

	// positively link the client, even if the command times are weird
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
		VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
		trap_LinkEntity( ent );
	}

	// run the presend to set anything else
	ClientEndFrame( ent );

	// clear entity state values
	BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
}


/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap_DropClient(), which will call this and do
server system housekeeping.
============
*/
void ClientDisconnect( int clientNum ) {
	gentity_t	*ent;
	gentity_t	*tent;
	int			i;

	// cleanup if we are kicking a bot that
	// hasn't spawned yet
	G_RemoveQueuedBotBegin( clientNum );

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;
	}

	// BFP - Avoid null exception when firing a weapon like this
	if ( ent->client->hook ) {
        Weapon_BFPBeamFree( ent->client->hook );
    }

	// stop any following clients
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].sess.sessionTeam == TEAM_SPECTATOR
			&& level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW
			&& level.clients[i].sess.spectatorClient == clientNum ) {
			StopFollowing( &g_entities[i] );
		}
	}

	// send effect if they were completely connected
	if ( ent->client->pers.connected == CON_CONNECTED 
		&& ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = ent->s.clientNum;

		// They don't get to take powerups with them!
		// Especially important for stuff like CTF flags
		TossClientItems( ent );
	}

	G_LogPrintf( "ClientDisconnect: %i\n", clientNum );

	// if we are playing in tourney mode and losing, give a win to the other player
	if ( ( g_gametype.integer == GT_TOURNAMENT
		|| g_gametype.integer == GT_SURVIVAL ) // BFP - Survival
		&& !level.intermissiontime
		&& !level.warmupTime && level.sortedClients[1] == clientNum ) {
		level.clients[ level.sortedClients[0] ].sess.wins++;
		ClientUserinfoChanged( level.sortedClients[0] );
	}

	// BFP - Monster gamemode, if this guy disconnected, respawn the other guy who can become a monster
	ClientCheckMonsterGone( ent );

	trap_UnlinkEntity (ent);
	ent->s.modelindex = 0;
	ent->inuse = qfalse;
	ent->classname = "disconnected";
	ent->client->pers.connected = CON_DISCONNECTED;
	ent->client->ps.persistant[PERS_TEAM] = TEAM_FREE;
	ent->client->sess.sessionTeam = TEAM_FREE;

	trap_SetConfigstring( CS_PLAYERS + clientNum, "");

	CalculateRanks();

	if ( ent->r.svFlags & SVF_BOT ) {
		BotAIShutdownClient( clientNum, qfalse );
	}
}


