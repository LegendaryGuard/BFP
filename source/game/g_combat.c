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
// g_combat.c

#include "g_local.h"


/*
============
ScorePlum
============
*/
void ScorePlum( gentity_t *ent, vec3_t origin, int score ) {
	gentity_t *plum;

	plum = G_TempEntity( origin, EV_SCOREPLUM );
	// only send this temp entity to a single client
	plum->r.svFlags |= SVF_SINGLECLIENT;
	plum->r.singleClient = ent->s.number;
	//
	plum->s.otherEntityNum = ent->s.number;
	plum->s.time = score;
}

/*
============
AddScore

Adds score to both the client and his team
============
*/
void AddScore( gentity_t *ent, vec3_t origin, int score ) {
	if ( !ent->client ) {
		return;
	}
	// no scoring during pre-match warmup
	if ( level.warmupTime ) {
		return;
	}
	// show score plum
	ScorePlum(ent, origin, score);
	//
	ent->client->ps.persistant[PERS_SCORE] += score;
	if ( g_gametype.integer == GT_TEAM
	|| g_gametype.integer == GT_TLMS ) // BFP - Team Last Man Standing
		level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += score;
	CalculateRanks();
}

/*
=================
TossClientItems

Toss the weapon and powerups for the killed player
=================
*/
void TossClientItems( gentity_t *self ) {
	gitem_t		*item;
	float		angle;
	int			i;
	gentity_t	*drop;

	// BFP - No weapon drop
#if 0
	int			weapon;
	
	// drop the weapon if not a gauntlet or machinegun
	weapon = self->s.weapon;

	// make a special check to see if they are changing to a new
	// weapon that isn't the mg or gauntlet.  Without this, a client
	// can pick up a weapon, be killed, and not drop the weapon because
	// their weapon change hasn't completed yet and they are still holding the MG.
	if ( weapon == WP_MACHINEGUN || weapon == WP_GRAPPLING_HOOK ) {
		if ( self->client->ps.weaponstate == WEAPON_DROPPING ) {
			weapon = self->client->pers.cmd.weapon;
		}
		if ( !( self->client->ps.stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) {
			weapon = WP_NONE;
		}
	}

	if ( weapon > WP_MACHINEGUN && weapon != WP_GRAPPLING_HOOK && 
		self->client->ps.ammo[ weapon ] ) {
		// find the item type for this weapon
		item = BG_FindItemForWeapon( weapon );

		// spawn the item
		Drop_Item( self, item, 0 );
	}
#endif

	// drop all the powerups if not in teamplay
	if ( g_gametype.integer != GT_TEAM ) {
		angle = 45;
		for ( i = 1 ; i < PW_NUM_POWERUPS ; i++ ) {
			if ( self->client->ps.powerups[ i ] > level.time ) {
				item = BG_FindItemForPowerup( i );
				if ( !item ) {
					continue;
				}
				drop = Drop_Item( self, item, angle );
				// decide how many seconds it has left
				drop->count = ( self->client->ps.powerups[ i ] - level.time ) / 1000;
				if ( drop->count < 1 ) {
					drop->count = 1;
				}
				angle += 45;
			}
		}
	}
}

/*
==================
LookAtKiller
==================
*/
void LookAtKiller( gentity_t *self, gentity_t *inflictor, gentity_t *attacker ) {
	vec3_t		dir;
	float		killerYaw = self->s.angles[YAW];

	if ( attacker && attacker != self ) {
		VectorSubtract (attacker->s.pos.trBase, self->s.pos.trBase, dir);
	} else if ( inflictor && inflictor != self ) {
		VectorSubtract (inflictor->s.pos.trBase, self->s.pos.trBase, dir);
	} else {
		if ( killerYaw > 255 ) {
			self->client->ps.damageYaw = 255;
			self->client->ps.damagePitch = (int)( killerYaw - 255 ); 
		} else {
			self->client->ps.damageYaw = (int)killerYaw;
			self->client->ps.damagePitch = 0;
		}
		return;
	}

	killerYaw = vectoyaw( dir );
	if ( killerYaw > 255 ) {
		self->client->ps.damageYaw = 255;
		self->client->ps.damagePitch = (int)( killerYaw - 255 ); 
	} else {
		self->client->ps.damageYaw = (int)killerYaw;
		self->client->ps.damagePitch = 0;
	}
}

/*
==================
GibEntity
==================
*/
void GibEntity( gentity_t *self, int killer ) {
	G_AddEvent( self, EV_GIB_PLAYER, killer );
	self->takedamage = qfalse;
	self->s.eType = ET_INVISIBLE;
	self->r.contents = 0;
}

/*
==================
body_die
==================
*/
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	if ( self->health > GIB_HEALTH ) {
		return;
	}
	if ( !g_blood.integer ) {
		self->health = GIB_HEALTH+1;
		return;
	}

	GibEntity( self, 0 );
}


// these are just for logging, the client prints its own messages
char	*modNames[MOD_MAX] = {
	// BFP - Means of death are declared in bg_meansofdeath.h file
#define MOD_STRINGS
	#include "bg_meansofdeath.h"
#undef MOD_STRINGS
	"NULL"	// avoid -Wpedantic warnings
};

/*
==================
CheckAlmostCapture
==================
*/
void CheckAlmostCapture( gentity_t *self, gentity_t *attacker ) {
	gentity_t	*ent;
	vec3_t		dir;
	char		*classname;

	// if this player was carrying a flag
	if ( self->client->ps.powerups[PW_REDFLAG] ||
		self->client->ps.powerups[PW_BLUEFLAG] ) {
		// get the goal flag this player should have been going for
		if ( g_gametype.integer == GT_CTF ) {
			if ( self->client->sess.sessionTeam == TEAM_BLUE ) {
				classname = "team_CTF_blueflag";
			}
			else {
				classname = "team_CTF_redflag";
			}
		}
		else {
			if ( self->client->sess.sessionTeam == TEAM_BLUE ) {
				classname = "team_CTF_redflag";
			}
			else {
				classname = "team_CTF_blueflag";
			}
		}
		ent = NULL;
		do
		{
			ent = G_Find(ent, FOFS(classname), classname);
		} while (ent && (ent->flags & FL_DROPPED_ITEM));
		// if we found the destination flag and it's not picked up
		if (ent && !(ent->r.svFlags & SVF_NOCLIENT) ) {
			// if the player was *very* close
			VectorSubtract( self->client->ps.origin, ent->s.origin, dir );
			if ( VectorLength(dir) < 200 ) {
				self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				if ( attacker->client ) {
					attacker->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				}
			}
		}
	}
}

/*
======================
GainPowerlevelKiHealth
======================
*/
static void GainPowerlevelKiHealth( gentity_t *self, gentity_t *attacker ) { // BFP - Gain powerlevel, ki and health
	float currentKiPercentage;
	qboolean alreadyTier1 = qfalse;
	qboolean alreadyTier2 = qfalse;
	qboolean alreadyTier3 = qfalse;
	qboolean alreadyTransformed = qfalse;
	// BFP - Monster gamemode handling the maximum ki calculation
	int monsterKi = 1;

	if ( attacker->client->ps.persistant[PERS_POWERLEVEL] > 99 && attacker->client->ps.persistant[PERS_POWERLEVEL] < 250 ) {
		alreadyTier1 = qtrue;
	}
	if ( attacker->client->ps.persistant[PERS_POWERLEVEL] > 249 && attacker->client->ps.persistant[PERS_POWERLEVEL] < 500 ) {
		alreadyTier2 = qtrue;
	}
	if ( attacker->client->ps.persistant[PERS_POWERLEVEL] > 499 && attacker->client->ps.persistant[PERS_POWERLEVEL] < 1000 ) {
		alreadyTier3 = qtrue;
	}
	if ( attacker->client->ps.persistant[PERS_POWERLEVEL] >= 1000 ) {
		alreadyTransformed = qtrue;
	}

	// BFP - Attacker gains powerlevel from the opponent
	// Formula: attackerPowerlevel += 1 + ( opponentPowerlevel * g_plKillBonusPct.value )
	attacker->client->ps.persistant[PERS_POWERLEVEL] += 1 + ( self->client->ps.persistant[PERS_POWERLEVEL] * g_plKillBonusPct.value );
	if ( !alreadyTier1 && attacker->client->ps.persistant[PERS_POWERLEVEL] > 99 && attacker->client->ps.persistant[PERS_POWERLEVEL] < 250 ) {
		attacker->client->ps.eFlags |= EF_AURA_TIER_UP;
		attacker->client->tierUnlockedTime = level.time + 2000;
		BG_AddPredictableEventToPlayerstate( EV_TIER_1, 0, &attacker->client->ps, -1 );
	} else if ( !alreadyTier2 && attacker->client->ps.persistant[PERS_POWERLEVEL] > 249 && attacker->client->ps.persistant[PERS_POWERLEVEL] < 500 ) {
		attacker->client->ps.eFlags |= EF_AURA_TIER_UP;
		attacker->client->tierUnlockedTime = level.time + 2000;
		BG_AddPredictableEventToPlayerstate( EV_TIER_2, 0, &attacker->client->ps, -1 );
	} else if ( !alreadyTier3 && attacker->client->ps.persistant[PERS_POWERLEVEL] > 499 && attacker->client->ps.persistant[PERS_POWERLEVEL] < 1000 ) {
		attacker->client->ps.eFlags |= EF_AURA_TIER_UP;
		attacker->client->tierUnlockedTime = level.time + 2000;
		BG_AddPredictableEventToPlayerstate( EV_TIER_3, 0, &attacker->client->ps, -1 );
	} else if ( !alreadyTransformed && attacker->client->ps.persistant[PERS_POWERLEVEL] > 999 ) {
		attacker->client->ps.eFlags |= EF_AURA_TIER_UP;
		attacker->client->ps.pm_flags |= PMF_ULTIMATE_TIER;
		attacker->client->tierUnlockedTime = level.time + 5000;
		BG_AddPredictableEventToPlayerstate( EV_TIER_4, 0, &attacker->client->ps, -1 );
	}
	if ( attacker->client->ps.persistant[PERS_POWERLEVEL] > 1000 ) { // if higher, clamp to 1000
		attacker->client->ps.persistant[PERS_POWERLEVEL] = 1000;
	}

	// BFP - Send powerlevel info to cgame reusing frame from entityState_t struct
	attacker->s.frame = attacker->client->ps.persistant[PERS_POWERLEVEL];

	// BFP - Add more maximum ki
	currentKiPercentage = ( (float)attacker->client->ps.ammo[WP_KI] / (float)attacker->client->ps.stats[STAT_MAX_KI] );
	// BFP - Monster gamemode, handle attacker ki monster calculation
	if ( attacker->client->ps.eFlags & EF_MONSTER ) {
		monsterKi = 2;
	}
	// BFP - NOTE: What the heck? Did BFP dev make this multiplying 9.00825 with powerlevel? Strange approximation...
	attacker->client->ps.stats[STAT_MAX_KI] = monsterKi * ( 999 + ( 9.00825 * attacker->client->ps.persistant[PERS_POWERLEVEL] ) );

	// BFP - Avoid exceeding maximum ki
	if ( attacker->client->ps.stats[STAT_MAX_KI] > ( 10000 * monsterKi ) ) {
		attacker->client->ps.stats[STAT_MAX_KI] = 10000 * monsterKi;
	}

	// BFP - Add and balance ki
	if ( attacker->client->ps.persistant[PERS_POWERLEVEL] < 1000 ) {
		if ( currentKiPercentage < 1.0f ) {
			currentKiPercentage *= (float)attacker->client->ps.stats[STAT_MAX_KI];
			attacker->client->ps.ammo[WP_KI] = currentKiPercentage;
		} else {
			attacker->client->ps.ammo[WP_KI] = attacker->client->ps.stats[STAT_MAX_KI];
		}
		if ( attacker->client->ps.ammo[WP_KI] > attacker->client->ps.stats[STAT_MAX_KI] ) {
			attacker->client->ps.ammo[WP_KI] = attacker->client->ps.stats[STAT_MAX_KI];
		}
	}

	// BFP - Add max health and balance health
	if ( attacker->client->ps.stats[STAT_MAX_HEALTH] < 1000 ) {
		float currentHealthPercentage = ( (float)attacker->client->ps.stats[STAT_HEALTH] / (float)attacker->client->ps.stats[STAT_MAX_HEALTH] ) * 10.0f;
		
		attacker->client->ps.stats[STAT_MAX_HEALTH] = 1 + attacker->client->ps.persistant[PERS_POWERLEVEL];
		if ( attacker->client->ps.stats[STAT_MAX_HEALTH] > 1000 ) {
			attacker->client->ps.stats[STAT_MAX_HEALTH] = 1000;
		}

		// keep and balance health
		if ( currentHealthPercentage < 10 ) {
			attacker->health = attacker->client->ps.stats[STAT_HEALTH] = attacker->client->ps.stats[STAT_HEALTH] + (short)currentHealthPercentage;
		} else {
			attacker->health = attacker->client->ps.stats[STAT_HEALTH] = attacker->client->ps.stats[STAT_MAX_HEALTH];
		}
		if ( attacker->client->ps.stats[STAT_HEALTH] > attacker->client->ps.stats[STAT_MAX_HEALTH] ) {
			attacker->health = attacker->client->ps.stats[STAT_HEALTH] = attacker->client->ps.stats[STAT_MAX_HEALTH];
		}
	}

	if ( attacker->client->ps.ammo[WP_KI] > attacker->client->ps.stats[STAT_MAX_KI] ) {
		attacker->client->ps.ammo[WP_KI] = attacker->client->ps.stats[STAT_MAX_KI];
	}

	// BFP - When unlocking a tier, give the player maximum health and ki
	if ( attacker->client->ps.eFlags & EF_AURA_TIER_UP ) {
		attacker->health = attacker->client->ps.stats[STAT_HEALTH] = attacker->client->ps.stats[STAT_MAX_HEALTH];
		attacker->client->ps.ammo[WP_KI] = attacker->client->ps.stats[STAT_MAX_KI];
	}
}

/*
==================================================
Survival_ForceToSpectateAndRespawnAnotherPlayer
==================================================
*/
static void Survival_ForceToSpectateAndRespawnAnotherPlayer( gentity_t *self ) { // BFP - Function for survival gamemode
	int i, oldestTime = level.time, oldestClient = -1;

	// force to spectate the dead player
	if ( level.numPlayingClients == 2 && level.numConnectedClients > 2 ) {
		SetTeam( self, "s" );
		self->client->sess.sessionTeam = TEAM_SPECTATOR;
	}

	for ( i = 0; i < level.maxclients; ++i ) {
		gclient_t *cl = &level.clients[i];
		if ( cl->pers.connected == CON_CONNECTED && cl->sess.sessionTeam == TEAM_SPECTATOR ) {
			if ( cl->pers.enterTime < oldestTime ) {
				oldestTime = cl->pers.enterTime;
				oldestClient = i;
			}
		}
	}

	if ( oldestClient != -1 && level.numConnectedClients > 2 ) {
		gentity_t *spectator = &g_entities[oldestClient];
		SetTeam( spectator, "f" );
		ClientSpawn( spectator );
	}
}

/*
====================
CheckSurvivalWarmup
====================
*/
static void CheckSurvivalWarmup( void ) { // BFP - Function for survival gamemode
	// if the warmup is changed at the console, restart it
	if ( g_warmup.modificationCount != level.warmupModificationCount ) {
		level.warmupModificationCount = g_warmup.modificationCount;
		level.warmupTime = -1;
	}

	// if all players have arrived, start the countdown
	if ( level.numPlayingClients == 2 ) {
		// fudge by -1 to account for extra delays
		level.warmupTime = level.time + ( g_warmup.integer - 1 ) * 1000;
		trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
	}
}

/*
==================
CheckSurvivalRules
==================
*/
static void CheckSurvivalRules( gentity_t *self, int meansOfDeath ) { // BFP - Survival rules
	int i;

	if ( g_gametype.integer != GT_SURVIVAL ) {
		return;
	}

	// don't make players not see the scoreboard when the match timelimit ends
	if ( g_timelimit.integer && !level.warmupTime 
	&& level.time - level.startTime >= g_timelimit.integer*60000 ) {
		return;
	}

	// don't make players blind to the scoreboard when the winner reached the fraglimit and the match ended
	for ( i = 0 ; i < level.maxclients; ++i ) {
		gclient_t	*cl;
		cl = level.clients + i;
		if ( cl->ps.persistant[PERS_SCORE] >= g_fraglimit.integer ) {
			return;
		}
	}

	// BFP - NOTE: This is the best conditional when changing a character model.
	// Originally on BFP, when a player changed the model, the warmup time was being restarted.
	// So, that could lead trolling cases to break the game balance.

	if ( level.warmupTime > 0 && level.numPlayingClients == 2 ) {
		// force to spectate the player who is touching something mortal
		if ( meansOfDeath == MOD_TRIGGER_HURT || meansOfDeath == MOD_CRUSH ) {
			// if the score points didn't decrease, decrease it for balance sake!
			ScorePlum( self, self->r.currentOrigin, -1 );
			--self->client->ps.persistant[PERS_SCORE];
			CalculateRanks();

			Survival_ForceToSpectateAndRespawnAnotherPlayer( self );
			
			CheckSurvivalWarmup();

			if ( level.numConnectedClients == 2 ) {
				respawn( self );
			}
			
			return;
		}
		// respawn if changing, trying to spectate or commiting a suicide
		else if ( meansOfDeath != MOD_TRIGGER_HURT && meansOfDeath != MOD_CRUSH ) {
			respawn( self );
			return;
		}
	}

	// if there are only 2 players in the game
	if ( level.warmupTime <= 0 
	&& level.numConnectedClients == 2 && level.numPlayingClients == 2 ) {
		// respawn quickly instead watching the scoreboard while being dead
		respawn( self );

		CheckSurvivalWarmup();

		return;
	}

	Survival_ForceToSpectateAndRespawnAnotherPlayer( self );

	CheckSurvivalWarmup();
}

/*
==========================
CheckMonsterGamemodeRules
==========================
*/
static void CheckMonsterGamemodeRules( gentity_t *self, gentity_t *attacker, int meansOfDeath ) { // BFP - Monster gamemode rules
	qboolean selfMonster = ( self && self->client && self->client->ps.clientNum == level.monsterClientNum );

	if ( g_gametype.integer != GT_MONSTER ) {
		return;
	}

	// if it's going to spectate
	if ( selfMonster && self->client->pers.teamState.state == TEAM_BEGIN ) {
		return;
	}

	// if changed the character, just respawn
	if ( selfMonster && meansOfDeath == MOD_UNKNOWN ) {
		respawn( self );
		return;
	}

	if ( selfMonster
	&& ( meansOfDeath == MOD_TRIGGER_HURT || meansOfDeath == MOD_CRUSH || meansOfDeath == MOD_FALLING
	|| meansOfDeath == MOD_LAVA || meansOfDeath == MOD_SLIME ) ) {
		trap_SendServerCommand( -1, va("print \"The monster died without a killer.\n\"") );
		respawn( self );
		return;
	}

	if ( attacker && attacker->client && selfMonster ) {
		if ( attacker == self ) {
			trap_SendServerCommand( -1, va("print \"The monster killed himself!\n\"") );
			respawn( attacker );
			return;
		}
		trap_SendServerCommand( -1, va("print \"%s killed the monster!\n\"", attacker->client->pers.netname) );
		level.monsterClientNum = attacker->client->ps.clientNum;
		self->client->ps.eFlags &= ~EF_MONSTER;
		attacker->client->ps.eFlags |= EF_MONSTER;

		respawn( attacker );
	}
}

/*
=================================
TeamLastManStanding_CheckDeadTeam
=================================
*/
static qboolean TeamLastManStanding_CheckDeadTeam( team_t team ) { // BFP - Function for Team Last Man Standing gamemode
	int i;

	for ( i = 0; i < level.numConnectedClients; ++i ) {
		gclient_t *cl = &level.clients[i];
		if ( cl->sess.sessionTeam == team && cl->ps.pm_type != PM_DEAD ) {
			return qfalse;
		}
	}
	return qtrue;
}

/*
=======================================
TeamLastManStanding_RespawnAllDeadTeams
=======================================
*/
static void TeamLastManStanding_RespawnAllDeadTeams( void ) { // BFP - Function for Team Last Man Standing gamemode
	int i;

	for ( i = 0; i < level.numConnectedClients; ++i ) {
		gentity_t *ent = &g_entities[i];
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR
		&& ent->client->forceToSpectate ) {
			ent->client->forceToSpectate = qfalse;
			if ( ent->client->selectedTeam == TEAM_RED ) {
				SetTeam( ent, "r" );
				ClientSpawn( ent );
			} else if ( ent->client->selectedTeam == TEAM_BLUE ) {
				SetTeam( ent, "b" );
				ClientSpawn( ent );
			}
			ent->client->sess.sessionTeam = ent->client->selectedTeam;
		}
	}
}

/*
==============================
CheckTeamLastManStandingWarmup
==============================
*/
static void CheckTeamLastManStandingWarmup( void ) { // BFP - Function for Team Last Man Standing gamemode
	if ( g_warmup.integer <= 0 ) {
		return;
	}

	// if the warmup is changed at the console, restart it
	if ( g_warmup.modificationCount != level.warmupModificationCount ) {
		level.warmupModificationCount = g_warmup.modificationCount;
		level.warmupTime = -1;
	}

	// fudge by -1 to account for extra delays
	level.warmupTime = level.time + ( g_warmup.integer - 1 ) * 1000;
	trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
}

/*
==============================
CheckTeamLastManStandingRules
==============================
*/
static void CheckTeamLastManStandingRules( gentity_t *self, gentity_t *attacker, int meansOfDeath ) { // BFP - Team Last Man Standing rules
	qboolean selfClient = ( self && self->client );
	qboolean attackerClient = ( attacker && attacker->client );

	// BFP - NOTE: Force to spectate the current dead players in the match, 
	// the dead player spectators can't join, the playing players can't switch teams and going to spectate.
	// Once, the match gets all dead players (these who are marked and forced to spectate), 
	// respawns all dead players, the game finishes until hits the fraglimit/timelimit.
	// The players who were beginning to connect and are news in the match, they can join if they wish, 
	// but after there's no turn back (that applies the explanation from before).
	// Originally on BFP, the dead players can join in the match, so that's invalid and it's against the rules.

	if ( g_gametype.integer != GT_TLMS ) {
		return;
	}

	// force to spectate the player who is touching something mortal during the warmup
	if ( level.warmupTime > 0
	&& selfClient
	&& ( meansOfDeath == MOD_TRIGGER_HURT || meansOfDeath == MOD_CRUSH ) ) {
		// if the score points didn't decrease, decrease it for balance sake!
		ScorePlum( self, self->r.currentOrigin, -1 );
		--self->client->ps.persistant[PERS_SCORE];
		CalculateRanks();
	}

	// check dead teams
	if ( TeamLastManStanding_CheckDeadTeam( TEAM_RED ) ) {
		CheckTeamLastManStandingWarmup();
		TeamLastManStanding_RespawnAllDeadTeams();
		// respawn the 2 last players in the match: the victim and the attacker
		if ( selfClient ) {
			respawn( self );
		}
		if ( attackerClient ) {
			respawn( attacker );
		}

		return;
	}

	if ( TeamLastManStanding_CheckDeadTeam( TEAM_BLUE ) ) {
		CheckTeamLastManStandingWarmup();
		TeamLastManStanding_RespawnAllDeadTeams();
		// respawn the 2 last players in the match: the victim and the attacker
		if ( selfClient ) {
			respawn( self );
		}
		if ( attackerClient ) {
			respawn( attacker );
		}

		return;
	}

	if ( selfClient ) {
		self->client->forceToSpectate = qtrue;
		// keep the selected team, otherwise won't spawn correctly
		self->client->selectedTeam = self->client->sess.sessionTeam;
		SetTeam( self, "s" );
		self->client->sess.sessionTeam = TEAM_SPECTATOR;
	}
}

/*
==================
player_die
==================
*/
void player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	gentity_t	*ent;
	int			anim;
	int			contents;
	int			killer;
	int			i;
	char		*killerName, *obit;

	if ( self->client->ps.pm_type == PM_DEAD ) {
		return;
	}

	if ( level.intermissiontime ) {
		return;
	}

	// BFP - For compilation safety from shared objects (.so) and dll
	if ( attacker != NULL ) {
		// check for an almost capture
		CheckAlmostCapture( self, attacker );
	}

	// BFP - BFP Beam handling
	if (self->client && self->client->hook) {
		// Weapon_HookFree(self->client->hook);
		Weapon_BFPBeamFree( self->client->hook );
	}

	// BFP - Monster gamemode, if the player monster fell, respawn and don't score
	if ( self && self->client && self->client->ps.clientNum == level.monsterClientNum
	&& ( self->client->ps.eFlags & EF_MONSTER )
	&& meansOfDeath == MOD_TRIGGER_HURT ) {
		respawn( self );
		return;
	}

	self->client->ps.pm_type = PM_DEAD;

	if ( attacker ) {
		killer = attacker->s.number;
		if ( attacker->client ) {
			killerName = attacker->client->pers.netname;
		} else {
			killerName = "<non-client>";
		}
	} else {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( killer < 0 || killer >= MAX_CLIENTS ) {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( meansOfDeath < 0 || meansOfDeath >= sizeof( modNames ) / sizeof( modNames[0] ) ) {
		obit = "<bad obituary>";
	} else {
		obit = modNames[ meansOfDeath ];
	}

	G_LogPrintf("Kill: %i %i %i: %s killed %s by %s\n", 
		killer, self->s.number, meansOfDeath, killerName, 
		self->client->pers.netname, obit );

	// broadcast the death event to everyone
	ent = G_TempEntity( self->r.currentOrigin, EV_OBITUARY );
	ent->s.eventParm = meansOfDeath;
	ent->s.otherEntityNum = self->s.number;
	ent->s.otherEntityNum2 = killer;
	ent->r.svFlags = SVF_BROADCAST;	// send to everyone

	self->enemy = attacker;

	self->client->ps.persistant[PERS_KILLED]++;

	if (attacker && attacker->client) {
		attacker->client->lastkilled_client = self->s.number;

		if ( attacker == self || OnSameTeam (self, attacker ) ) {
			AddScore( attacker, self->r.currentOrigin, -1 );
		} else {
			AddScore( attacker, self->r.currentOrigin, 1 );

			if( meansOfDeath == MOD_GAUNTLET ) {
				
				// BFP - No gauntlet counter
				// play humiliation on player
				// attacker->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;

				// add the sprite over the player's head
				// BFP - No impressive, gauntlet, defend, assist and cap medals
				attacker->client->ps.eFlags &= ~(EF_AWARD_EXCELLENT /*| EF_AWARD_IMPRESSIVE | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP*/ );
				// attacker->client->ps.eFlags |= EF_AWARD_GAUNTLET;
				attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;

				// also play humiliation on target
				self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_GAUNTLETREWARD;
			}

			// check for two kills in a short amount of time
			// if this is close enough to the last kill, give a reward sound
			if ( level.time - attacker->client->lastKillTime < CARNAGE_REWARD_TIME ) {
				// play excellent on player
				attacker->client->ps.persistant[PERS_EXCELLENT_COUNT]++;

				// add the sprite over the player's head
				// BFP - No impressive, gauntlet, defend, assist and cap medals
				attacker->client->ps.eFlags &= ~(EF_AWARD_EXCELLENT /*| EF_AWARD_IMPRESSIVE | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP*/ );
				attacker->client->ps.eFlags |= EF_AWARD_EXCELLENT;
				attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;
			}
			attacker->client->lastKillTime = level.time;

		}
	} else {
		AddScore( self, self->r.currentOrigin, -1 );
	}

	// BFP - Gain powerlevel, ki and health
	if ( attacker && attacker->client 
	&& attacker != self && !OnSameTeam (self, attacker ) ) {
		GainPowerlevelKiHealth( self, attacker );
	}

	// BFP - For compilation safety from shared objects (.so) and dll
	if ( attacker != NULL ) {
		// Add team bonuses
		Team_FragBonuses(self, inflictor, attacker);
	}

	// if I committed suicide, the flag does not fall, it returns.
	if (meansOfDeath == MOD_SUICIDE) {
		if ( self->client->ps.powerups[PW_REDFLAG] ) {		// only happens in standard CTF
			Team_ReturnFlag( TEAM_RED );
			self->client->ps.powerups[PW_REDFLAG] = 0;
		}
		else if ( self->client->ps.powerups[PW_BLUEFLAG] ) {	// only happens in standard CTF
			Team_ReturnFlag( TEAM_BLUE );
			self->client->ps.powerups[PW_BLUEFLAG] = 0;
		}
	}

	// if client is in a nodrop area, don't drop anything (but return CTF flags!)
	contents = trap_PointContents( self->r.currentOrigin, -1 );
	if ( !( contents & CONTENTS_NODROP )) {
		TossClientItems( self );
	}
	else {
		if ( self->client->ps.powerups[PW_REDFLAG] ) {		// only happens in standard CTF
			Team_ReturnFlag( TEAM_RED );
		}
		else if ( self->client->ps.powerups[PW_BLUEFLAG] ) {	// only happens in standard CTF
			Team_ReturnFlag( TEAM_BLUE );
		}
	}

	Cmd_Score_f( self );		// show scores
	// send updated scores to any clients that are following this one,
	// or they would get stale scoreboards
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		gclient_t	*client;

		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
			continue;
		}
		if ( client->sess.spectatorClient == self->s.number ) {
			Cmd_Score_f( g_entities + i );
		}
	}

	self->takedamage = qtrue;	// can still be gibbed

	self->s.weapon = WP_NONE;
	self->s.powerups = 0;
	self->r.contents = CONTENTS_CORPSE;

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;
	LookAtKiller (self, inflictor, attacker);

	VectorCopy( self->s.angles, self->client->ps.viewangles );

	self->s.loopSound = 0;

	self->r.maxs[2] = -8;

	// don't allow respawn until the death anim is done
	// g_forcerespawn may force spawning at some later time
	self->client->respawnTime = level.time + 1700;

	// remove powerups
	memset( self->client->ps.powerups, 0, sizeof(self->client->ps.powerups) );

	// never gib in a nodrop
	// BFP - No gibs when being attacked and entering to death phase, just stay as corpse
#if 0
	if ((self->health <= GIB_HEALTH && !(contents & CONTENTS_NODROP) && g_blood.integer) || meansOfDeath == MOD_SUICIDE) {
		// gib death
		GibEntity( self, killer );
	} else
#endif
	{
		// normal death
		static int i;

		switch ( i ) {
		case 0:
			anim = BOTH_DEATH1;
			break;
		case 1:
			anim = BOTH_DEATH2;
			break;
		case 2:
		default:
			anim = BOTH_DEATH3;
			break;
		}

		// for the no-blood option, we need to prevent the health
		// from going to gib level
		if ( self->health <= GIB_HEALTH ) {
			self->health = GIB_HEALTH+1;
		}

		self->client->ps.legsAnim = 
			( ( self->client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
		self->client->ps.torsoAnim = 
			( ( self->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

		G_AddEvent( self, EV_DEATH1 + i, killer );

		// the body can still be gibbed
		self->die = body_die;

		// globally cycle through the different death animations
		i = ( i + 1 ) % 3;

	}

	trap_LinkEntity (self);

	// BFP - Survival rules
	CheckSurvivalRules( self, meansOfDeath );

	// BFP - Monster gamemode rules
	CheckMonsterGamemodeRules( self, attacker, meansOfDeath );

	// BFP - Team Last Man Standing rules
	CheckTeamLastManStandingRules( self, attacker, meansOfDeath );
}


/*
================
CheckArmor
================
*/
int CheckArmor (gentity_t *ent, int damage, int dflags)
{
	gclient_t	*client;
	int			save;
	int			count;

	if (!damage)
		return 0;

	client = ent->client;

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	// armor
	count = client->ps.stats[STAT_ARMOR];
	save = ceil( damage * ARMOR_PROTECTION );
	if (save >= count)
		save = count;

	if (!save)
		return 0;

	client->ps.stats[STAT_ARMOR] -= save;

	return save;
}

/*
================
RaySphereIntersections
================
*/
int RaySphereIntersections( vec3_t origin, float radius, vec3_t point, vec3_t dir, vec3_t intersections[2] ) {
	float b, c, d, t;

	//	| origin - (point + t * dir) | = radius
	//	a = dir[0]^2 + dir[1]^2 + dir[2]^2;
	//	b = 2 * (dir[0] * (point[0] - origin[0]) + dir[1] * (point[1] - origin[1]) + dir[2] * (point[2] - origin[2]));
	//	c = (point[0] - origin[0])^2 + (point[1] - origin[1])^2 + (point[2] - origin[2])^2 - radius^2;

	// normalize dir so a = 1
	VectorNormalize(dir);
	b = 2 * (dir[0] * (point[0] - origin[0]) + dir[1] * (point[1] - origin[1]) + dir[2] * (point[2] - origin[2]));
	c = (point[0] - origin[0]) * (point[0] - origin[0]) +
		(point[1] - origin[1]) * (point[1] - origin[1]) +
		(point[2] - origin[2]) * (point[2] - origin[2]) -
		radius * radius;

	d = b * b - 4 * c;
	if (d > 0) {
		t = (- b + sqrt(d)) / 2;
		VectorMA(point, t, dir, intersections[0]);
		t = (- b - sqrt(d)) / 2;
		VectorMA(point, t, dir, intersections[1]);
		return 2;
	}
	else if (d == 0) {
		t = (- b ) / 2;
		VectorMA(point, t, dir, intersections[0]);
		return 1;
	}
	return 0;
}

/*
============
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback
point		point at which the damage is being inflicted, used for headshots
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

inflictor, attacker, dir, and point can be NULL for environmental effects

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/

void G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			   vec3_t dir, vec3_t point, int damage, int dflags, int mod ) {
	gclient_t	*client;
	int			take;
	int			asave;
	int			knockback;
	int			max;
	// BFP - Melee knockback
	int			meleeKnockback = 0;

	// BFP - Ultimate tier status is invulnerable!
	if ( targ && targ->client // BFP - NOTE: Avoid DLL/SO crashing when impacting a door or any map entity (ET_MOVER), this is important for implementations like that!
	&& ( targ->client->ps.pm_flags & PMF_ULTIMATE_TIER ) ) {
		return;
	}

	if ( !targ || ( targ && !targ->takedamage ) ) {
		return;
	}

	// BFP - Don't deal damage on warmup
	if ( level.time < level.warmupTime ) {
		return;
	}

	// the intermission has allready been qualified for, so don't
	// allow any extra scoring
	if ( level.intermissionQueued ) {
		return;
	}
	if ( !inflictor ) {
		inflictor = &g_entities[ENTITYNUM_WORLD];
	}
	if ( !attacker ) {
		attacker = &g_entities[ENTITYNUM_WORLD];
	}

	// shootable doors / buttons don't actually have any health
	if ( targ->s.eType == ET_MOVER ) {
		if ( targ->use && targ->moverState == MOVER_POS1 ) {
			targ->use( targ, inflictor, attacker );
		}
		// BFP - For breakable map entities
		if ( targ->takedamage && targ->health > 0 ) {
			if ( damage < 1 ) {
				damage = 1;
			}
			take = damage;
			targ->health = targ->health - take;
		}
		return;
	}

	// BFP - Melee knockback
	if ( mod == MOD_MELEE ) {
		float	meleeFactor = 15.0f;
		if ( damage <= 10 ) {
			meleeFactor = 7.5f;
		} else if ( damage <= 20 ) {
			meleeFactor = 7.5f + ( damage - 10 ) * 0.45f;
		} else if ( damage <= 50 ) {
			meleeFactor = 12.0f + ( damage - 20 ) * 0.1f;
		}
		meleeKnockback = damage * meleeFactor;
	}

	// reduce damage by the attacker's handicap value
	// unless they are rocket jumping
	if ( attacker->client && attacker != targ ) {
		// BFP - Apply attacker powerlevel calculation, no maximum health calculation
		max = attacker->client->ps.persistant[PERS_POWERLEVEL] + 1; // BFP - before Q3: max = attacker->client->ps.stats[STAT_MAX_HEALTH];
		damage = damage * max * 0.01; // BFP - before Q3: damage = damage * max / 100;
	}

	client = targ->client;

	if ( client ) {
		if ( client->noclip ) {
			return;
		}
	}

	if ( !dir ) {
		dflags |= DAMAGE_NO_KNOCKBACK;
	} else {
		VectorNormalize(dir);

		// BFP - Lose altitude while flying/floating underwater
		if ( targ->client && !( ( targ->client->ps.pm_flags & PMF_JUMP_HELD )
		|| targ->client->ps.groundEntityNum != ENTITYNUM_NONE )
		// BFP - Don't apply for rocket jumping
		&& dir[2] <= 0.5f ) {
			dir[2] = -1;
		}
	}

	knockback = damage;
	if ( knockback > 200 ) {
		knockback = 200;
	}
	// BFP - Add enough knockback to push the targets while receiving explosion/projectile impacts
	if ( mod != MOD_MELEE ) {
		knockback = 200;
		// BFP - Rocket jumping
		if ( dir && dir[2] > 0.5f ) {
			knockback = 50;
		}
	} else { // BFP - Melee knockback
		knockback = meleeKnockback;
	}
	if ( knockback > 2000 ) { // BFP - Melee knockback cannot be more than 2000
		knockback = 2000;
	}
	if ( targ->flags & FL_NO_KNOCKBACK ) {
		knockback = 0;
	}
	if ( dflags & DAMAGE_NO_KNOCKBACK ) {
		knockback = 0;
	}

	// BFP - Blinds the opponent and it can be blinded again after 4 seconds (look inside cg_draw.c in CG_DrawBlindEffect for more details)
	if ( mod == MOD_MACHINEGUN // BFP - TODO: That's just a test. Add something to the weapon: 'blinding' for properties of the ki attacks from cfg
	&& ( !targ->blindedTime || level.time - targ->blindedTime >= 2000 )
	&& targ->client && targ->client->ps.pm_type != PM_DEAD ) {
		targ->blindedTime = level.time;
		BG_AddPredictableEventToPlayerstate( EV_BLINDING, 0, &targ->client->ps, -1 );
	}

	// figure momentum add, even if the damage won't be taken
	if ( knockback && targ->client ) {
		vec3_t	kvel;
		float	mass;

		mass = 200;

		VectorScale (dir, g_knockback.value * (float)knockback / mass, kvel);
		// BFP - Rocket jumping
		if ( ( ( targ->client->ps.pm_flags & PMF_JUMP_HELD )
		|| targ->waterlevel > 1 )
		&& dir && dir[2] > 0.5f ) {
			// increase vertical impulse by 25%
			kvel[2] *= 1.25f;
			if ( targ->waterlevel > 1 ) { // if underwater double the impulse and a little push
				VectorScale( kvel, 2, kvel );
				kvel[0] *= 1.5f;
				kvel[1] *= 1.5f;
			}
		}
		VectorAdd (targ->client->ps.velocity, kvel, targ->client->ps.velocity);

		// set the timer so that the other client can't cancel
		// out the movement immediately
		if ( !targ->client->ps.pm_time ) {
			int		t;

			t = knockback * 2;
			if ( t < 50 ) {
				t = 50;
			}
			if ( t > 200 ) {
				t = 200;
			}
			targ->client->ps.pm_time = t;
			targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}
	}

	// check for completely getting out of the damage
	if ( !(dflags & DAMAGE_NO_PROTECTION) ) {

		// if TF_NO_FRIENDLY_FIRE is set, don't do damage to the target
		// if the attacker was on the same team
		if ( targ != attacker && OnSameTeam (targ, attacker)  ) {
			if ( !g_friendlyFire.integer ) {
				return;
			}
		}

		// check for godmode
		if ( targ->flags & FL_GODMODE ) {
			return;
		}
	}

	// battlesuit protects from all radius damage (but takes knockback)
	// and protects 50% against all damage
	if ( client && client->ps.powerups[PW_BATTLESUIT] ) {
		G_AddEvent( targ, EV_POWERUP_BATTLESUIT, 0 );
		if ( ( dflags & DAMAGE_RADIUS ) || ( mod == MOD_FALLING ) ) {
			return;
		}
		damage *= 0.5;
	}

	// add to the attacker's hit counter (if the target isn't a general entity like a prox mine)
	if ( attacker->client && targ != attacker && targ->health > 0
			&& targ->s.eType != ET_MISSILE
			&& targ->s.eType != ET_GENERAL
	&& !( client->ps.pm_flags & PMF_BLOCK ) // BFP - When blocking, don't receive any hit
 	&& damage > 0 ) { // BFP - Don't apply hits persistance if the damage is lesser than 1
		if ( OnSameTeam( targ, attacker ) ) {
			attacker->client->ps.persistant[PERS_HITS]--;
		} else {
			attacker->client->ps.persistant[PERS_HITS]++;
		}
		attacker->client->ps.persistant[PERS_ATTACKEE_ARMOR] = (targ->health<<8)|(client->ps.stats[STAT_ARMOR]);
	}

	// always give half damage if hurting self
	// calculated after knockback, so rocket jumping works
	if ( targ == attacker) {
		damage *= 0.5;
	}

	// BFP - No damage < 1 conditional
#if 0
	if ( damage < 1 ) {
		damage = 1;
	}
#endif
	take = damage;

	// save some from armor
	asave = CheckArmor (targ, take, dflags);
	take -= asave;

	 // BFP - When blocking, don't receive damage
	if ( client && ( client->ps.pm_flags & PMF_BLOCK ) ) {
		take = asave = 0;
	}

	if ( g_debugDamage.integer ) {
		G_Printf( "%i: client:%i health:%i damage:%i armor:%i\n", level.time, targ->s.number,
			targ->health, take, asave );
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if ( client
	&& !( client->ps.pm_flags & PMF_BLOCK ) ) { // BFP - When blocking, don't receive screams of pain
		if ( attacker ) {
			client->ps.persistant[PERS_ATTACKER] = attacker->s.number;
		} else {
			client->ps.persistant[PERS_ATTACKER] = ENTITYNUM_WORLD;
		}
		client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;
		if ( dir ) {
			VectorCopy ( dir, client->damage_from );
			client->damage_fromWorld = qfalse;
		} else {
			VectorCopy ( targ->r.currentOrigin, client->damage_from );
			client->damage_fromWorld = qtrue;
		}
	}

	// See if it's the player hurting the emeny flag carrier
	if( g_gametype.integer == GT_CTF) {
		Team_CheckHurtCarrier(targ, attacker);
	}

	if (targ->client) {
		// set the last client who damaged the target
		targ->client->lasthurt_client = attacker->s.number;
		targ->client->lasthurt_mod = mod;
	}

	// do the damage
	if (take) {
		targ->health = targ->health - take;
		if ( targ->client ) {
			targ->client->ps.stats[STAT_HEALTH] = targ->health;
		}
			
		if ( targ->health <= 0 ) {
			if ( client )
				targ->flags |= FL_NO_KNOCKBACK;

			if (targ->health < -999)
				targ->health = -999;

			targ->enemy = attacker;
			targ->die (targ, inflictor, attacker, take, mod);
			return;
		} else if ( targ->pain ) {
			targ->pain (targ, attacker, take);
		}
	}

}


/*
============
CanDamage

Returns qtrue if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean CanDamage (gentity_t *targ, vec3_t origin) {
	vec3_t	dest;
	trace_t	tr;
	vec3_t	midpoint;

	// use the midpoint of the bounds instead of the origin, because
	// bmodels may have their origin is 0,0,0
	VectorAdd (targ->r.absmin, targ->r.absmax, midpoint);
	VectorScale (midpoint, 0.5, midpoint);

	VectorCopy (midpoint, dest);
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0 || tr.entityNum == targ->s.number)
		return qtrue;

	// this should probably check in the plane of projection, 
	// rather than in world coordinate, and also include Z
	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] -= 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] += 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] -= 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;


	return qfalse;
}


/*
============
G_RadiusDamage
============
*/
qboolean G_RadiusDamage ( gentity_t *self, vec3_t origin, gentity_t *attacker, float damage, float radius,
					 gentity_t *ignore, int mod) {
	float		points, dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;
	qboolean	hitClient = qfalse;

	if ( radius < 1 ) {
		radius = 1;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( origin[i] < ent->r.absmin[i] ) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if ( origin[i] > ent->r.absmax[i] ) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

		points = damage * ( 1.0 - dist / radius );

		if( CanDamage (ent, origin) ) {
			if( LogAccuracyHit( ent, attacker ) ) {
				hitClient = qtrue;
			}
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			G_Damage (ent, self, attacker, dir, origin, (int)points, DAMAGE_RADIUS, mod);
		}
	}

	return hitClient;
}
