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


/*
===============
G_DamageFeedback

Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player_state_t
damage values to that client for pain blends and kicks, and
global pain sound events for all clients.
===============
*/
void P_DamageFeedback( gentity_t *player ) {
	gclient_t	*client;
	float	count;
	vec3_t	angles;

	client = player->client;
	if ( client->ps.pm_type == PM_DEAD ) {
		return;
	}

	// total points of damage shot at the player this frame
	count = client->damage_blood + client->damage_armor;
	if ( count == 0 ) {
		return;		// didn't take any damage
	}

	if ( count > 255 ) {
		count = 255;
	}

	// send the information to the client

	// world damage (falling, slime, etc) uses a special code
	// to make the blend blob centered instead of positional
	if ( client->damage_fromWorld ) {
		client->ps.damagePitch = 255;
		client->ps.damageYaw = 255;

		client->damage_fromWorld = qfalse;
	} else {
		vectoangles( client->damage_from, angles );
		client->ps.damagePitch = angles[PITCH]/360.0 * 256;
		client->ps.damageYaw = angles[YAW]/360.0 * 256;
	}

	// play an apropriate pain sound
	if ( (level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE) ) {
		player->pain_debounce_time = level.time + 700;
		G_AddEvent( player, EV_PAIN, player->health );
		client->ps.damageEvent++;
	}


	client->ps.damageCount = count;

	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_knockback = 0;
}



/*
=============
P_WorldEffects

Check for lava / slime contents and drowning
=============
*/
void P_WorldEffects( gentity_t *ent ) {
	qboolean	envirosuit;
	int			waterlevel;

	envirosuit = ent->client->ps.powerups[PW_BATTLESUIT] > level.time;

	// BFP - No drowning
#if 0
	if ( ent->client->noclip ) {
		ent->client->airOutTime = level.time + 12000;	// don't need air
		return;
	}

	waterlevel = ent->waterlevel;


	//
	// check for drowning
	//
	if ( waterlevel == 3 ) {
		// envirosuit give air
		if ( envirosuit ) {
			ent->client->airOutTime = level.time + 10000;
		}

		// if out of air, start drowning
		if ( ent->client->airOutTime < level.time) {
			// drown!
			ent->client->airOutTime += 1000;
			if ( ent->health > 0 ) {
				// take more damage the longer underwater
				ent->damage += 2;
				if (ent->damage > 15)
					ent->damage = 15;

				// play a gurp sound instead of a normal pain sound
				if (ent->health <= ent->damage) {
					G_Sound(ent, CHAN_VOICE, G_SoundIndex("*drown.wav"));
				} else if (rand()&1) {
					G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/player/gurp1.wav"));
				} else {
					G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/player/gurp2.wav"));
				}

				// don't play a normal pain sound
				ent->pain_debounce_time = level.time + 200;

				G_Damage (ent, NULL, NULL, NULL, NULL, 
					ent->damage, DAMAGE_NO_ARMOR, MOD_WATER);
			}
		}
	} else {
		ent->client->airOutTime = level.time + 12000;
		ent->damage = 2;
	}
#endif

	//
	// check for sizzle damage (move to pmove?)
	//
	if (waterlevel && 
		(ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) ) {
		if (ent->health > 0
			&& ent->pain_debounce_time <= level.time	) {

			if ( envirosuit ) {
				G_AddEvent( ent, EV_POWERUP_BATTLESUIT, 0 );
			} else {
				if (ent->watertype & CONTENTS_LAVA) {
					G_Damage (ent, NULL, NULL, NULL, NULL, 
						30*waterlevel, 0, MOD_LAVA);
				}

				if (ent->watertype & CONTENTS_SLIME) {
					G_Damage (ent, NULL, NULL, NULL, NULL, 
						10*waterlevel, 0, MOD_SLIME);
				}
			}
		}
	}
}



/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound( gentity_t *ent ) {
	if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) ) {
		ent->client->ps.loopSound = level.snd_fry;
	} else {
		ent->client->ps.loopSound = 0;
	}
}



//==============================================================

/*
==============
ClientImpacts
==============
*/
void ClientImpacts( gentity_t *ent, pmove_t *pm ) {
	int		i, j;
	trace_t	trace;
	gentity_t	*other;

	memset( &trace, 0, sizeof( trace ) );
	for (i=0 ; i<pm->numtouch ; i++) {
		for (j=0 ; j<i ; j++) {
			if (pm->touchents[j] == pm->touchents[i] ) {
				break;
			}
		}
		if (j != i) {
			continue;	// duplicated
		}
		other = &g_entities[ pm->touchents[i] ];

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, other, &trace );
		}

		if ( !other->touch ) {
			continue;
		}

		other->touch( other, ent, &trace );
	}

}

/*
============
G_TouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void	G_TouchTriggers( gentity_t *ent ) {
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	trace_t		trace;
	vec3_t		mins, maxs;
	static vec3_t	range = { 40, 40, 52 };

	if ( !ent->client ) {
		return;
	}

	// dead clients don't activate triggers!
	if ( ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	VectorSubtract( ent->client->ps.origin, range, mins );
	VectorAdd( ent->client->ps.origin, range, maxs );

	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	// can't use ent->absmin, because that has a one unit pad
	VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
	VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );

	for ( i=0 ; i<num ; i++ ) {
		hit = &g_entities[touch[i]];

		if ( !hit->touch && !ent->touch ) {
			continue;
		}
		if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) {
			continue;
		}

		// ignore most entities if a spectator
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			if ( hit->s.eType != ET_TELEPORT_TRIGGER &&
				// this is ugly but adding a new ET_? type will
				// most likely cause network incompatibilities
				hit->touch != Touch_DoorTrigger) {
				continue;
			}
		}

		// use seperate code for determining if an item is picked up
		// so you don't have to actually contact its bounding box
		if ( hit->s.eType == ET_ITEM ) {
			if ( !BG_PlayerTouchesItem( &ent->client->ps, &hit->s, level.time ) ) {
				continue;
			}
		} else {
			if ( !trap_EntityContact( mins, maxs, hit ) ) {
				continue;
			}
		}

		memset( &trace, 0, sizeof(trace) );

		if ( hit->touch ) {
			hit->touch (hit, ent, &trace);
		}

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, hit, &trace );
		}
	}

	// if we didn't touch a jump pad this pmove frame
	if ( ent->client->ps.jumppad_frame != ent->client->ps.pmove_framecount ) {
		ent->client->ps.jumppad_frame = 0;
		ent->client->ps.jumppad_ent = 0;
	}
}

/*
=================
SpectatorThink
=================
*/
void SpectatorThink( gentity_t *ent, usercmd_t *ucmd ) {
	pmove_t	pm;
	gclient_t	*client;

	client = ent->client;

	if ( client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		client->ps.pm_type = PM_SPECTATOR;
		client->ps.speed = 400;	// faster than normal

		// set up for pmove
		memset (&pm, 0, sizeof(pm));
		pm.ps = &client->ps;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;	// spectators can fly through bodies
		pm.trace = trap_Trace;
		pm.pointcontents = trap_PointContents;

		// perform a pmove
		Pmove (&pm);
		// save results of pmove
		VectorCopy( client->ps.origin, ent->s.origin );

		G_TouchTriggers( ent );
		trap_UnlinkEntity( ent );
	}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;

	// attack button cycles through spectators
	if ( ( client->buttons & BUTTON_ATTACK ) && ! ( client->oldbuttons & BUTTON_ATTACK ) ) {
		Cmd_FollowCycle_f( ent, 1 );
	}
}



/*
=================
ClientInactivityTimer

Returns qfalse if the client is dropped
=================
*/
qboolean ClientInactivityTimer( gclient_t *client ) {
	if ( ! g_inactivity.integer ) {
		// give everyone some time, so if the operator sets g_inactivity during
		// gameplay, everyone isn't kicked
		client->inactivityTime = level.time + 60 * 1000;
		client->inactivityWarning = qfalse;
	} else if ( client->pers.cmd.forwardmove || 
		client->pers.cmd.rightmove || 
		client->pers.cmd.upmove ||
		(client->pers.cmd.buttons & BUTTON_ATTACK) ) {
		client->inactivityTime = level.time + g_inactivity.integer * 1000;
		client->inactivityWarning = qfalse;
	} else if ( !client->pers.localClient ) {
		if ( level.time > client->inactivityTime ) {
			trap_DropClient( client - level.clients, "Dropped due to inactivity" );
			return qfalse;
		}
		if ( level.time > client->inactivityTime - 10000 && !client->inactivityWarning ) {
			client->inactivityWarning = qtrue;
			trap_SendServerCommand( client - level.clients, "cp \"Ten seconds until inactivity drop!\n\"" );
		}
	}
	return qtrue;
}

/*
==================
ClientTimerActions

Actions that happen once a second
==================
*/
void ClientTimerActions( gentity_t *ent, int msec ) {
	gclient_t	*client;

	client = ent->client;
	client->timeResidual += msec;

	while ( client->timeResidual >= 1000 ) {
		client->timeResidual -= 1000;

		// regenerate
		if ( client->ps.powerups[PW_REGEN] ) {
			if ( ent->health < client->ps.stats[STAT_MAX_HEALTH]) {
				ent->health += 15;
				if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] * 1.1 ) {
					ent->health = client->ps.stats[STAT_MAX_HEALTH] * 1.1;
				}
				G_AddEvent( ent, EV_POWERUP_REGEN, 0 );
			} else if ( ent->health < client->ps.stats[STAT_MAX_HEALTH] * 2) {
				ent->health += 5;
				if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] * 2 ) {
					ent->health = client->ps.stats[STAT_MAX_HEALTH] * 2;
				}
				G_AddEvent( ent, EV_POWERUP_REGEN, 0 );
			}
		} else {
			// count down health when over max
			if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] ) {
				ent->health--;
			}
		}

		// BFP - Ki up/down when flying/ki use
		if ( client->ps.powerups[PW_FLIGHT] > 0 ) {

			// BFP - TODO: Add cvar for flight cost

			if ( client->ps.stats[STAT_KI] > 0 ) {
				client->ps.stats[STAT_KI]--;
			
				if ( client->pers.cmd.buttons & BUTTON_KI_USE )
					client->ps.stats[STAT_KI]--;
			}
		} else {
			client->ps.stats[STAT_KI]++;
		}

		// BFP - if ki drops to 0, disable flight
		if ( client->ps.stats[STAT_KI] <= 0 ) {
			client->ps.stats[STAT_KI] = 0;
			client->ps.powerups[PW_FLIGHT] = 0;
			// Com_Printf( "ki amount: %d\n", client->ps.stats[STAT_KI] );
		}

		// count down armor when over max
		if ( client->ps.stats[STAT_ARMOR] > client->ps.stats[STAT_MAX_HEALTH] ) {
			client->ps.stats[STAT_ARMOR]--;
		}
	}
}

/*
====================
ClientIntermissionThink
====================
*/
void ClientIntermissionThink( gclient_t *client ) {
	client->ps.eFlags &= ~EF_TALK;
	client->ps.eFlags &= ~EF_FIRING;

	// the level will exit when everyone wants to or after timeouts

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = client->pers.cmd.buttons;
	if ( client->buttons & ( BUTTON_ATTACK | BUTTON_USE_HOLDABLE ) & ( client->oldbuttons ^ client->buttons ) ) {
		// this used to be an ^1 but once a player says ready, it should stick
		client->readyToExit = 1;
	}
}


/*
================
ClientEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
void ClientEvents( gentity_t *ent, int oldEventSequence ) {
	int		i, j;
	int		event;
	gclient_t *client;
	int		damage;
	vec3_t	dir;
	vec3_t	origin, angles;
//	qboolean	fired;
	gitem_t *item;
	gentity_t *drop;

	client = ent->client;

	if ( oldEventSequence < client->ps.eventSequence - MAX_PS_EVENTS ) {
		oldEventSequence = client->ps.eventSequence - MAX_PS_EVENTS;
	}
	for ( i = oldEventSequence ; i < client->ps.eventSequence ; i++ ) {
		event = client->ps.events[ i & (MAX_PS_EVENTS-1) ];

		switch ( event ) {
		case EV_FALL_MEDIUM:
		case EV_FALL_FAR:
			// BFP - There's no crash land damage when the players fell in the ground
#if 0
			if ( ent->s.eType != ET_PLAYER ) {
				break;		// not in the player model
			}
			if ( g_dmflags.integer & DF_NO_FALLING ) {
				break;
			}
			if ( event == EV_FALL_FAR ) {
				damage = 10;
			} else {
				damage = 5;
			}
			VectorSet (dir, 0, 0, 1);
			ent->pain_debounce_time = level.time + 200;	// no normal pain sound
			G_Damage (ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING);
#endif
			break;

		case EV_FIRE_WEAPON:
			FireWeapon( ent );
			break;

		case EV_USE_ITEM1:		// teleporter
			// drop flags in CTF
			item = NULL;
			j = 0;

			if ( ent->client->ps.powerups[ PW_REDFLAG ] ) {
				item = BG_FindItemForPowerup( PW_REDFLAG );
				j = PW_REDFLAG;
			} else if ( ent->client->ps.powerups[ PW_BLUEFLAG ] ) {
				item = BG_FindItemForPowerup( PW_BLUEFLAG );
				j = PW_BLUEFLAG;
			} else if ( ent->client->ps.powerups[ PW_NEUTRALFLAG ] ) {
				item = BG_FindItemForPowerup( PW_NEUTRALFLAG );
				j = PW_NEUTRALFLAG;
			}

			if ( item ) {
				drop = Drop_Item( ent, item, 0 );
				// decide how many seconds it has left
				drop->count = ( ent->client->ps.powerups[ j ] - level.time ) / 1000;
				if ( drop->count < 1 ) {
					drop->count = 1;
				}

				ent->client->ps.powerups[ j ] = 0;
			}

			SelectSpawnPoint( ent, ent->client->ps.origin, origin, angles );
			TeleportPlayer( ent, origin, angles );
			break;

		case EV_USE_ITEM2:		// medkit
			ent->health = ent->client->ps.stats[STAT_MAX_HEALTH] + 25;

			break;

		default:
			break;
		}
	}

}

void BotTestSolid(vec3_t origin);

/*
==============
SendPendingPredictableEvents
==============
*/
void SendPendingPredictableEvents( playerState_t *ps ) {
	gentity_t *t;
	int event, seq;
	int extEvent, number;

	// if there are still events pending
	if ( ps->entityEventSequence < ps->eventSequence ) {
		// create a temporary entity for this event which is sent to everyone
		// except the client who generated the event
		seq = ps->entityEventSequence & (MAX_PS_EVENTS-1);
		event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
		// set external event to zero before calling BG_PlayerStateToEntityState
		extEvent = ps->externalEvent;
		ps->externalEvent = 0;
		// create temporary entity for event
		t = G_TempEntity( ps->origin, event );
		number = t->s.number;
		BG_PlayerStateToEntityState( ps, &t->s, qtrue );
		t->s.number = number;
		t->s.eType = ET_EVENTS + event;
		t->s.eFlags |= EF_PLAYER_EVENT;
		t->s.otherEntityNum = ps->clientNum;
		// send to everyone except the client who generated the event
		t->r.svFlags |= SVF_NOTSINGLECLIENT;
		t->r.singleClient = ps->clientNum;
		// set back external event
		ps->externalEvent = extEvent;
	}
}

/*
=================
FlyingThink
=================
*/
void FlyingThink( gentity_t *ent, usercmd_t *ucmd ) { // BFP - Flight
	gclient_t	*client;

	client = ent->client;

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;

	// enableflight button cycles
	if ( ( client->buttons & BUTTON_ENABLEFLIGHT ) && ! ( client->oldbuttons & BUTTON_ENABLEFLIGHT ) ) {
		Cmd_BFP_Fly_f( ent );
	}
}

/*
=================
BlockHandling
=================
*/
static void BlockHandling( gclient_t *client, usercmd_t *ucmd ) { // BFP - Block, reflect ki attacks and reduce health damage
	// initialize the blocking and start the block length duration, specifically, ki boost and aura are disabled
	if ( !( client->ps.pm_flags & PMF_BLOCK )
	&& ( ucmd->buttons & BUTTON_BLOCK )
	&& client->blockTime <= 0
	&& client->blockDelayTime <= 0 ) {
		client->ps.pm_flags |= PMF_BLOCK;
		client->ps.powerups[PW_HASTE] = 0;
		client->ps.eFlags &= ~EF_AURA;
		ucmd->buttons &= ~BUTTON_KI_USE;
		client->blockTime = level.time + (g_blockLength.integer * 1000);
	}

	// BFP - Blocking status. Ki energy is being consumed and ki boost can't be used
	if ( ( client->ps.pm_flags & PMF_BLOCK ) 
	&& client->blockTime > 0 
	&& level.time < client->blockTime ) {
		// BFP - NOTE: Approximate calculation of ki consumption while blocking
		float blockCostPct = g_blockCostPct.integer * 0.1; // percentage of ki consumed per millisecond
		float bCost = g_blockCost.integer > 0 ? g_blockCost.integer : 1; // absolute value of ki consumed per millisecond
		float kiBlockConsume = bCost / (g_blockLength.integer * 1000.0);
		float totalBlockConsume;
		// random variable to make an approximate calculation of the ki consumption while using block
		float rndkiConsume = rand() % 2;

		// BFP - TODO: Implement random calculations correctly?
		if ( crandom() > 0.5 && crandom() < 0.8 ) {
			rndkiConsume = rand() % 1;
		} else if ( crandom() > 0.2 && crandom() < 0.5 ) {
			rndkiConsume = random() + 0.38;
		}

		// calculate total ki being consumed
		totalBlockConsume = kiBlockConsume * (g_blockLength.integer * 1000.0) * rndkiConsume;

		client->ps.stats[STAT_KI] -= totalBlockConsume;
	}

	// when the block length duration has been expired, then start the delay to avoid user 
	if ( ( client->ps.pm_flags & PMF_BLOCK ) 
	&& client->blockTime > 0 
	&& level.time >= client->blockTime ) {
		client->ps.pm_flags &= ~PMF_BLOCK;
		client->blockTime = 0;
		client->blockDelayTime = level.time + (g_blockDelay.integer * 1000);
	}

	// if the block length duration hasn't been expired yet and 
	// pressing ki charge (if the aura is lighting) or attack buttons, then stop blocking and start the delay
	if ( ( client->ps.pm_flags & PMF_BLOCK ) 
	&& ( ( client->ps.pm_flags & PMF_KI_CHARGE )
	|| ( ucmd->buttons & BUTTON_KI_CHARGE )
	|| ( ucmd->buttons & BUTTON_ATTACK )
	|| ( ucmd->buttons & BUTTON_MELEE ) ) ) {
		client->ps.pm_flags &= ~PMF_BLOCK;
		client->blockTime = 0;
		client->blockDelayTime = level.time + (g_blockDelay.integer * 1000);
	}

	// debug print block length and delay duration
#if 0
	Com_Printf( "BLOCK LENGTH: %d\n", client->blockTime );
	Com_Printf( "BLOCK DELAY: %d\n", client->blockDelayTime );
#endif

	// handle the delay and don't leave the user get away with it
	if ( !( client->ps.pm_flags & PMF_BLOCK )
	&& client->blockDelayTime > 0 
	&& level.time < client->blockDelayTime ) {
		client->blockTime = 0;
		ucmd->buttons &= ~BUTTON_BLOCK; // if the user holds the key, when that ends, then immediately enters to this status again
	}

	// reset block delay time if expired
	if ( !( client->ps.pm_flags & PMF_BLOCK )
	&& client->blockDelayTime > 0
	&& level.time >= client->blockDelayTime ) {
		client->blockDelayTime = 0;
	}
}

/*
=================
MeleeHandling
=================
*/
static void MeleeHandling( gentity_t *ent, usercmd_t *ucmd, pmove_t *pm ) { // BFP - Melee
	gclient_t	*client;

	client = ent->client;

	if ( !( ucmd->buttons & BUTTON_MELEE ) ) {
		client->ps.pm_flags &= ~PMF_MELEE;
	}

	if ( !( ucmd->buttons & BUTTON_TALK ) && ( ucmd->buttons & BUTTON_MELEE )
	&& !( client->ps.pm_flags & PMF_KI_CHARGE )
	&& client->ps.weaponTime <= 0 ) {
		pm->gauntletHit = CheckMeleeAttack( ent );
	}
}

/*
============
Zanzoken
============
*/
qboolean Zanzoken( gentity_t *ent, int range ) { // BFP - Short-Range Teleport (Zanzoken)
	trace_t	tr;
	vec3_t	right, up, start, direction;
	int		startRightRange = ( range < 0 ) ? -10 : 10;

	// set diagonal direction, included the up vector for upward detection
	AngleVectors( ent->client->ps.viewangles, NULL, right, up );

	// upward detection, avoid if the player is touching the surface above
	VectorMA( ent->client->ps.origin, 10, up, start );
	VectorMA( start, 100, up, direction );

	trap_Trace( &tr, start, ent->r.mins, ent->r.maxs, direction, ent->s.number, MASK_PLAYERSOLID );
	if ( tr.startsolid || tr.allsolid ) {
		return qfalse;
	}

	// if there's something solid diagonally, then avoid the teleportation
	VectorMA( ent->client->ps.origin, startRightRange, right, start );
	VectorMA( ent->client->ps.origin, range, right, direction );

	trap_Trace( &tr, start, ent->r.mins, ent->r.maxs, direction, ent->s.number, MASK_PLAYERSOLID );
	if ( tr.startsolid || tr.allsolid ) {
		return qfalse;
	}

	// TELEPORT!
	VectorCopy( tr.endpos, ent->client->ps.origin );

	// sound event
	G_AddEvent( ent, EV_ZANZOKEN, 0 );

	return qtrue;
}

/*
=================
ZanzokenHandling
=================
*/
#define ZANZOKEN_NUMBER_TIMES_ALLOWED	20
#define ZANZOKEN_ABUSE_DELAY			2000
static void ZanzokenHandling( gentity_t *ent, usercmd_t *ucmd ) { // BFP - Handling short-range teleport
	// zanzoken cannot be used with ki charging status and using explosion wave even while being stun after using explosion wave
	if ( !( ent->client->ps.pm_flags & PMF_KI_CHARGE ) 
	&& ( ent->client->ps.weaponstate != WEAPON_KIEXPLOSIONWAVE
		&& ent->client->ps.weaponstate != WEAPON_STUN ) ) {
		// restriction: stop abusing zanzoken technique all time
		if ( ent->client->zanzokenNumberTimesAllowed >= ZANZOKEN_NUMBER_TIMES_ALLOWED ) {
			ent->client->zanzokenNumberTimesAllowed = 0;
			ent->client->zanzokenDelay = level.time;
			return;
		}
		if ( ent->client->zanzokenDelay > 0
		&& level.time - ent->client->zanzokenDelay <= ZANZOKEN_ABUSE_DELAY ) {
			return;
		}

		if ( ucmd->rightmove && ent->client->zanzokenPressTime <= 0 ) {
			ent->client->zanzokenPressTime = level.time;
			ent->client->zanzokenNow = qfalse;
			// handle directions to avoid pressing the opposite
			if ( ucmd->rightmove > 0 ) {
				ent->client->zanzokenLeft = qfalse;
				ent->client->zanzokenRight = qtrue;
			} else {
				ent->client->zanzokenLeft = qtrue;
				ent->client->zanzokenRight = qfalse;
			}
		}

		// once pressed and having one moment to press again, zanzoken will be possible at these milliseconds
		if ( !ucmd->rightmove 
		&& level.time - ent->client->zanzokenPressTime > 100
		&& level.time - ent->client->zanzokenPressTime <= 250
		&& !ent->client->zanzokenNow ) {
			ent->client->zanzokenNow = qtrue;
			ent->client->zanzokenNumberTimesAllowed++;
		}

		if ( !ucmd->rightmove 
		&& level.time - ent->client->zanzokenPressTime > 250 ) {
			ent->client->zanzokenNumberTimesAllowed = 0;
			ent->client->zanzokenPressTime = 0;
			ent->client->zanzokenNow = qfalse;
			ent->client->zanzokenLeft = qfalse;
			ent->client->zanzokenRight = qfalse;
			return;
		}

		// BFP - TODO: Zanzoken ki consume looks relative to powerlevel and the maximum ki quantity 
		// (so, if it's 8160 as ki max quantity, then consumes 408)
		if ( ent->client->ps.stats[STAT_KI] > 408
		&& ucmd->rightmove
		&& ent->client->zanzokenNow ) {
			int	range = ( ucmd->rightmove > 0 ) ? 500 : -500;

			// handle the directions correctly
			if ( ucmd->rightmove > 0 && !ent->client->zanzokenRight ) {
				ent->client->zanzokenLeft = qfalse;
				ent->client->zanzokenRight = qfalse;
				return;
			}
			if ( ucmd->rightmove < 0 && !ent->client->zanzokenLeft ) {
				ent->client->zanzokenLeft = qfalse;
				ent->client->zanzokenRight = qfalse;
				return;
			}

			// put in 1 second delay before the player can 'zanzoken' out of stun
			if ( ( ent->client->ps.pm_flags & PMF_HITSTUN )
			&& ent->client->ps.pm_time > 2000 ) {
				ent->client->zanzokenPressTime = 0;
				ent->client->zanzokenNow = qfalse;
				ent->client->zanzokenLeft = qfalse;
				ent->client->zanzokenRight = qfalse;
				return;
			}

			if ( Zanzoken( ent, range ) ) {
				// block and stun statuses are removed when using zanzoken
				if ( ( ent->client->ps.pm_flags & PMF_HITSTUN )
				&& ent->client->ps.pm_time <= 2000 ) {
					ent->client->ps.pm_flags &= ~PMF_HITSTUN;
					ent->client->ps.pm_time = 0;
				}
				ent->client->ps.pm_flags &= ~PMF_BLOCK;
				ent->client->ps.stats[STAT_KI] -= 408;
				ent->client->zanzokenPressTime = 0;
				ent->client->zanzokenNow = qfalse;
				ent->client->zanzokenLeft = qfalse;
				ent->client->zanzokenRight = qfalse;
			}
		}
	}
}
#undef ZANZOKEN_NUMBER_TIMES_ALLOWED
#undef ZANZOKEN_ABUSE_DELAY

/*
=================
KiAttackWeaponHandling
=================
*/
#define ATTACK_CHARGE_LIMIT 6	// BFP - Ki attack charge limit
// BFP - TODO: Use pmove_t variable to handle the ki attacks and send the info to bg_pmove.c
static void KiAttackWeaponHandling( gentity_t *ent, usercmd_t *ucmd, pmove_t *pm ) {
	gclient_t	*client;
	int			addTime;

	client = ent->client;

	// BFP - Hit stun, avoid shooting if the player is in this status
	if ( client->ps.pm_flags & PMF_HITSTUN ) {
		return;
	}

#define KI_CONSUME(addTime, kiconsume) \
	client->ps.stats[STAT_KI] -= kiconsume; \
	client->ps.weaponTime += addTime;

#define CHARGE_KI_ATTACK_STATE(minCharge, maxCharge, addTime, kiconsume) \
	if ( client->ps.stats[STAT_KI_ATTACK_CHARGE] < maxCharge ) { \
		++client->ps.stats[STAT_KI_ATTACK_CHARGE]; \
	} \
	if ( client->ps.stats[STAT_KI_ATTACK_CHARGE] >= minCharge ) { \
		client->ps.stats[STAT_READY_KI_ATTACK] = qtrue; \
	} \
	KI_CONSUME( addTime, kiconsume )

	// Weapon states, Q3 doesn't have this way
	switch( client->ps.weaponstate ) {
	case WEAPON_READY:
		client->ps.stats[STAT_READY_KI_ATTACK] = qfalse;
		if ( client->ps.weaponTime <= 0 ) {
			// check for fire
			if ( ucmd->buttons & BUTTON_ATTACK ) {

				// BFP - NOTE: These are just examples of ki charging and shooting,
				// - WP_GRENADE_LAUNCHER should be like WP_MACHINEGUN and WP_LIGHTNINGGUN to keep the continuous shooting animations
				//   WP_GRENADE_LAUNCHER is used as example of charge homing ball shot
				// - WP_SHOTGUN is used as example of ki explosion wave
				// - WP_PLASMAGUN is used as example of dividing ki ball
				// - WP_BFG is used as example of ki beam
				switch( client->ps.weapon ) {
				case WP_SHOTGUN: // add time to handle ki explosion wave animation
					client->ps.weaponTime += 700;
					break;
				case WP_LIGHTNING: // only play once this sound for ki attacks like eyebeam
					G_AddEvent( ent, EV_FIRE_WEAPON, 0 );
					break;
				case WP_GRENADE_LAUNCHER:
				case WP_PLASMAGUN:
				case WP_BFG:
				default: 
					break;
				}
			}
		}
		if ( !( ucmd->buttons & BUTTON_ATTACK ) ) {
			// BFP - When the ki attack is fully charged, enter beam firing state
			// or enter dividing ki ball firing state if it's a dividing ki ball

			// BFP - TODO: Apply minCharge and maxCharge from reading bfp_weapon.cfg 
			switch( client->ps.weapon ) {
			case WP_GRENADE_LAUNCHER:
				break;
			case WP_PLASMAGUN:
				break;
			case WP_BFG:
				break;
			}
		}
		break;
	case WEAPON_DROPPING:
	case WEAPON_RAISING:
		break;
	case WEAPON_CHARGING:
		// BFP - Shoot the projectile or beam already charged surpassing the minimum
		if ( !( ucmd->buttons & BUTTON_ATTACK ) ) {
			client->ps.stats[STAT_READY_KI_ATTACK] = qfalse;
			// no fully charged, skip...
			// BFP - TODO: Apply minCharge in that condition also
			if ( client->ps.stats[STAT_KI_ATTACK_CHARGE] < 2 ) {
				client->ps.weaponstate = WEAPON_READY;
				break;
			}
			// Fire and make a sound
			FireWeapon( ent );
			G_AddEvent( ent, EV_FIRE_WEAPON, 0 );

			// Handle the animation for the start of beam or ball shoot
			switch( client->ps.weapon ) {
			case WP_GRENADE_LAUNCHER: 
			case WP_BFG: 
				client->ps.weaponTime += 500;
			}
		}

		if ( client->ps.weaponTime <= 0 ) {
			// check for fire
			if ( ucmd->buttons & BUTTON_ATTACK ) {

				// BFP - NOTE: These are just examples of ki charging and shooting,
				// - WP_GRENADE_LAUNCHER should be like WP_MACHINEGUN and WP_LIGHTNING to keep the continuous shooting animations
				//   WP_GRENADE_LAUNCHER is used as example of charge homing ball shot
				// - WP_SHOTGUN is used as example of ki explosion wave
				// - WP_PLASMAGUN is used as example of dividing ki ball
				// - WP_BFG is used as example of ki beam

				// BFP - TODO: Also? Apply minCharge and maxCharge from reading bfp_weapon.cfg 
				switch( client->ps.weapon ) {
				case WP_GRENADE_LAUNCHER:
					CHARGE_KI_ATTACK_STATE( 2, 2, 700, 20 )
					break;
				case WP_PLASMAGUN:
					CHARGE_KI_ATTACK_STATE( 2, ATTACK_CHARGE_LIMIT, 1000, 120 )
					client->divideBallKiCharged = client->ps.stats[STAT_KI_ATTACK_CHARGE];
					break;
				case WP_BFG:
					CHARGE_KI_ATTACK_STATE( 2, ATTACK_CHARGE_LIMIT, 1000, 20 )
					break;
				default: 
					break;
				}
			}
		}
		break;
	case WEAPON_FIRING:
		if ( ucmd->buttons & BUTTON_ATTACK ) {
			if ( client->ps.weaponTime <= 0
			&& client->ps.weapon != WP_LIGHTNING ) {
				client->ps.pm_flags &= ~PMF_KI_ATTACK;
			}

#define KI_CONSUME_ADDTIME(weptime, kiconsume) \
	addTime = weptime; \
	client->ps.stats[STAT_KI] -= kiconsume;

			switch( client->ps.weapon ) {
			default:
			case WP_MACHINEGUN:
				FireWeapon( ent );
				G_AddEvent( ent, EV_FIRE_WEAPON, 0 );
				KI_CONSUME_ADDTIME( 100, 10 )
				break;
			case WP_ROCKET_LAUNCHER:
				FireWeapon( ent );
				G_AddEvent( ent, EV_FIRE_WEAPON, 0 );
				KI_CONSUME_ADDTIME( 800, 50 )
				break;
			case WP_LIGHTNING:
				if ( client->ps.weaponTime <= 0 ) {
					FireWeapon( ent );
					KI_CONSUME_ADDTIME( 50, 70 )
				}
				break;
			case WP_RAILGUN:
				FireWeapon( ent );
				G_AddEvent( ent, EV_FIRE_WEAPON, 0 );
				KI_CONSUME_ADDTIME( 1500, 150 )
				break;
			}

			client->ps.weaponTime += addTime;
		}
		break;
	// BFP - NOTE: The beam is triggering until pressing the attack key again after holded, using ki charge or blocking
	// Pressing attack key again or changing weapon, the beam is exploded before the impact
	case WEAPON_BEAMFIRING:
		break;
	// BFP - NOTE: That happens when the player uses a quick ki explosion themself or a homing ki ball is being triggered
	case WEAPON_EXPLODING_KIBALLFIRING:
		break;
	// BFP - NOTE: The dividing ki ball is triggering until pressing the attack key again after holded or changing weapon
	case WEAPON_DIVIDINGKIBALLFIRING:
		if ( ucmd->buttons & BUTTON_ATTACK ) {
			KI_CONSUME( 0, 120 )
		}
		break;
	// BFP - NOTE: That ki explosion wave is triggering until stop pressing the attack key or changing weapon,
	// also when stopped enters in WEAPON_STUN state in 1 sec
	case WEAPON_KIEXPLOSIONWAVE:
		if ( client->ps.weaponTime <= 0 ) {
			KI_CONSUME( 200, 20 )
			if ( client->ps.stats[STAT_KI_ATTACK_CHARGE] < ATTACK_CHARGE_LIMIT ) {
				++client->ps.stats[STAT_KI_ATTACK_CHARGE];
			}
		}

		// fall even whether the player is flying
		//client->ps.velocity[2] -= client->ps.gravity * pml.frametime;
		break;
	// BFP - NOTE: This stun state makes the player can't move in 1 sec, it's different from "hit stun"
	case WEAPON_STUN:
		break;
	}
}
#undef KI_CONSUME
#undef CHARGE_KI_ATTACK_STATE
#undef KI_CONSUME_ADDTIME

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
void ClientThink_real( gentity_t *ent ) {
	gclient_t	*client;
	pmove_t		pm;
	int			oldEventSequence;
	int			msec;
	usercmd_t	*ucmd;

	client = ent->client;

	// don't think if the client is not yet connected (and thus not yet spawned in)
	if (client->pers.connected != CON_CONNECTED) {
		return;
	}
	// mark the time, so the connection sprite can be removed
	ucmd = &ent->client->pers.cmd;

	// sanity check the command time to prevent speedup cheating
	if ( ucmd->serverTime > level.time + 200 ) {
		ucmd->serverTime = level.time + 200;
//		G_Printf("serverTime <<<<<\n" );
	}
	if ( ucmd->serverTime < level.time - 1000 ) {
		ucmd->serverTime = level.time - 1000;
//		G_Printf("serverTime >>>>>\n" );
	} 

	msec = ucmd->serverTime - client->ps.commandTime;
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if ( msec < 1 && client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		return;
	}
	if ( msec > 200 ) {
		msec = 200;
	}

	if ( pmove_msec.integer < 8 ) {
		trap_Cvar_Set("pmove_msec", "8");
	}
	else if (pmove_msec.integer > 33) {
		trap_Cvar_Set("pmove_msec", "33");
	}

	if ( pmove_fixed.integer || client->pers.pmoveFixed ) {
		ucmd->serverTime = ((ucmd->serverTime + pmove_msec.integer-1) / pmove_msec.integer) * pmove_msec.integer;
		//if (ucmd->serverTime - client->ps.commandTime <= 0)
		//	return;
	}

	//
	// check for exiting intermission
	//
	if ( level.intermissiontime ) {
		ClientIntermissionThink( client );
		return;
	}

	// spectators don't do much
	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
			return;
		}
		SpectatorThink( ent, ucmd );
		return;
	}

	// check for inactivity timer, but never drop the local client of a non-dedicated server
	if ( !ClientInactivityTimer( client ) ) {
		return;
	}

	// clear the rewards if time
	if ( level.time > client->rewardTime ) {
		client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP );
	}

	if ( client->noclip ) {
		client->ps.pm_type = PM_NOCLIP;
	} else if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
		client->ps.pm_type = PM_DEAD;
	} else {
		client->ps.pm_type = PM_NORMAL;
	}

	client->ps.gravity = g_gravity.value;

	// set speed
	client->ps.speed = g_speed.value;

// BFP - no hook
#if 0
	// Let go of the hook if we aren't firing
	if ( client->ps.weapon == WP_GRAPPLING_HOOK &&
		client->hook && !( ucmd->buttons & BUTTON_ATTACK ) ) {
		Weapon_HookFree(client->hook);
	}
#endif

	// set up for pmove
	oldEventSequence = client->ps.eventSequence;

	memset (&pm, 0, sizeof(pm));

	if ( client->ps.pm_type != PM_DEAD && client->ps.pm_type != PM_SPECTATOR ) {
		
		// BFP - Short-Range Teleport (Zanzoken)
		ZanzokenHandling( ent, ucmd );

		// BFP - Ki attack handling (as weapon handling)
		KiAttackWeaponHandling( ent, ucmd, &pm );

		// BFP - Hit stun melee delay time
		if ( client->hitStunMeleeDelayTime > 0 
		&& level.time >= client->hitStunMeleeDelayTime ) {
			client->hitStunMeleeDelayTime = 0;
		}

		// BFP - Ki use has 2 options: "kiusetoggle" to toggle and "+button8" when key is being hold
		if ( !( client->ps.pm_flags & PMF_HITSTUN )
		&& !( client->ps.pm_flags & PMF_BLOCK )
		&& ( ( ucmd->buttons & BUTTON_KI_USE ) // BFP - Using Ki
			|| client->ps.powerups[PW_HASTE] > 0 ) // BFP - When "kiusetoggle" is binded, enables/disables
		&& ( client->ps.weaponstate != WEAPON_KIEXPLOSIONWAVE
			&& client->ps.weaponstate != WEAPON_STUN ) ) {
			if ( client->ps.powerups[PW_FLIGHT] <= 0 ) {
				client->ps.speed *= 2.5;
			}
			client->ps.powerups[PW_HASTE] = 1; // Handle ki boost status
			client->ps.eFlags |= EF_AURA;
		} else {
			if ( !( ucmd->buttons & BUTTON_KI_CHARGE ) ) { // BFP - If it's charging while it was using ki boost, don't remove the aura!
				client->ps.eFlags &= ~EF_AURA;
				client->ps.powerups[PW_HASTE] = 0; // Handle ki boost status
			}

			// BFP - g_chargeDelay cvar for ki charge animation and appearing the aura after this time
			if ( ( ucmd->buttons & BUTTON_KI_CHARGE )  
			&& !( client->ps.pm_flags & PMF_KI_CHARGE ) ) {
				client->ps.pm_time = ( g_chargeDelay.integer > 0 ) ? g_chargeDelay.integer : 0;
			}
		}

		// BFP - Block, reflect ki attacks and reduce health damage
		BlockHandling( client, ucmd );

		// BFP - Melee handling
		MeleeHandling( ent, ucmd, &pm );

		// BFP - Ki Charge
		if ( ( ucmd->buttons & BUTTON_KI_CHARGE ) && client->ps.pm_time <= 0 
		&& ( client->ps.pm_flags & PMF_KI_CHARGE ) ) {
			client->ps.eFlags |= EF_AURA;
		}

		if ( client->ps.powerups[PW_FLIGHT] > 0 ) { // BFP - Flight speed
			client->ps.speed *= 2;
		}
		// BFP - TODO: When charging a ki attack like beam wave, consult FlyingThink and SpectatorThink if that's the case

		// BFP - Enable flight
		FlyingThink( ent, ucmd ); // prevents client-server side issues when there's other client in-game
	}

	if ( ent->flags & FL_FORCE_GESTURE ) {
		ent->flags &= ~FL_FORCE_GESTURE;
		ent->client->pers.cmd.buttons |= BUTTON_GESTURE;
	}

	pm.ps = &client->ps;
	pm.cmd = *ucmd;
	if ( pm.ps->pm_type == PM_DEAD ) {
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
	}
	else if ( ent->r.svFlags & SVF_BOT ) {
		pm.tracemask = MASK_PLAYERSOLID | CONTENTS_BOTCLIP;
	}
	else {
		pm.tracemask = MASK_PLAYERSOLID;
	}
	pm.trace = trap_Trace;
	pm.pointcontents = trap_PointContents;
	pm.debugLevel = g_debugMove.integer;
	pm.noFootsteps = ( g_dmflags.integer & DF_NO_FOOTSTEPS ) > 0;

	pm.pmove_fixed = pmove_fixed.integer | client->pers.pmoveFixed;
	pm.pmove_msec = pmove_msec.integer;

	VectorCopy( client->ps.origin, client->oldOrigin );

	Pmove (&pm);

	// save results of pmove
	if ( ent->client->ps.eventSequence != oldEventSequence ) {
		ent->eventTime = level.time;
	}
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
	}
	else {
		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
	}
	SendPendingPredictableEvents( &ent->client->ps );

// BFP - no hook
#if 0
	if ( !( ent->client->ps.eFlags & EF_FIRING ) ) {
		client->fireHeld = qfalse;		// for grapple
	}
#endif

	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );

	VectorCopy (pm.mins, ent->r.mins);
	VectorCopy (pm.maxs, ent->r.maxs);

	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;

	// execute client events
	ClientEvents( ent, oldEventSequence );

	// link entity now, after any personal teleporters have been used
	trap_LinkEntity (ent);
	if ( !ent->client->noclip ) {
		G_TouchTriggers( ent );
	}

	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );

	//test for solid areas in the AAS file
	BotTestAAS(ent->r.currentOrigin);

	// touch other objects
	ClientImpacts( ent, &pm );

	// save results of triggers and client events
	if (ent->client->ps.eventSequence != oldEventSequence) {
		ent->eventTime = level.time;
	}

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// check for respawning
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
		// wait for the attack button to be pressed
		if ( level.time > client->respawnTime ) {
			// forcerespawn is to prevent users from waiting out powerups
			if ( g_forcerespawn.integer > 0 && 
				( level.time - client->respawnTime ) > g_forcerespawn.integer * 1000 ) {
				respawn( ent );
				return;
			}
		
			// pressing attack or use is the normal respawn method
			if ( ucmd->buttons & ( BUTTON_ATTACK | BUTTON_USE_HOLDABLE ) ) {
				respawn( ent );
			}
		}
		return;
	}

	// perform once-a-second actions
	ClientTimerActions( ent, msec );
}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/
void ClientThink( int clientNum ) {
	gentity_t *ent;

	ent = g_entities + clientNum;
	trap_GetUsercmd( clientNum, &ent->client->pers.cmd );

	// mark the time we got info, so we can display the
	// phone jack if they don't get any for a while
	ent->client->lastCmdTime = level.time;

	if ( !(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer ) {
		ClientThink_real( ent );
	}
}


void G_RunClient( gentity_t *ent ) {
	if ( !(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer ) {
		return;
	}
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink_real( ent );
}


/*
==================
SpectatorClientEndFrame

==================
*/
void SpectatorClientEndFrame( gentity_t *ent ) {
	gclient_t	*cl;

	// if we are doing a chase cam or a remote view, grab the latest info
	if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
		int		clientNum, flags;

		clientNum = ent->client->sess.spectatorClient;

		// team follow1 and team follow2 go to whatever clients are playing
		if ( clientNum == -1 ) {
			clientNum = level.follow1;
		} else if ( clientNum == -2 ) {
			clientNum = level.follow2;
		}
		if ( clientNum >= 0 ) {
			cl = &level.clients[ clientNum ];
			if ( cl->pers.connected == CON_CONNECTED && cl->sess.sessionTeam != TEAM_SPECTATOR ) {
				flags = (cl->ps.eFlags & ~(EF_VOTED | EF_TEAMVOTED)) | (ent->client->ps.eFlags & (EF_VOTED | EF_TEAMVOTED));
				ent->client->ps = cl->ps;
				ent->client->ps.pm_flags |= PMF_FOLLOW;
				ent->client->ps.eFlags = flags;
				return;
			} else {
				// drop them to free spectators unless they are dedicated camera followers
				if ( ent->client->sess.spectatorClient >= 0 ) {
					ent->client->sess.spectatorState = SPECTATOR_FREE;
					ClientBegin( ent->client - level.clients );
				}
			}
		}
	}

	// BFP - PMF_SCOREBOARD is unused
#if 0
	if ( ent->client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
		ent->client->ps.pm_flags |= PMF_SCOREBOARD;
	} else {
		ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
	}
#endif
}

/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEdFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame( gentity_t *ent ) {
	int			i;
	clientPersistant_t	*pers;

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		SpectatorClientEndFrame( ent );
		return;
	}

	pers = &ent->client->pers;

	// turn off any expired powerups
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		// BFP - Flight and haste are skipped, these are treated for player status
		if ( i == PW_FLIGHT || i == PW_HASTE ) continue;
		if ( ent->client->ps.powerups[ i ] < level.time ) {
			ent->client->ps.powerups[ i ] = 0;
		}
	}

	// save network bandwidth
#if 0
	if ( !g_synchronousClients->integer && ent->client->ps.pm_type == PM_NORMAL ) {
		// FIXME: this must change eventually for non-sync demo recording
		VectorClear( ent->client->ps.viewangles );
	}
#endif

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if ( level.intermissiontime ) {
		return;
	}

	// burn from lava, etc
	P_WorldEffects (ent);

	// apply all the damage taken this frame
	P_DamageFeedback (ent);

	// add the EF_CONNECTION flag if we haven't gotten commands recently
	if ( level.time - ent->client->lastCmdTime > 1000 ) {
		ent->s.eFlags |= EF_CONNECTION;
	} else {
		ent->s.eFlags &= ~EF_CONNECTION;
	}

	ent->client->ps.stats[STAT_HEALTH] = ent->health;	// FIXME: get rid of ent->health...

	G_SetClientSound (ent);

	// set the latest infor
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
	}
	else {
		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
	}
	SendPendingPredictableEvents( &ent->client->ps );

	// set the bit for the reachability area the client is currently in
//	i = trap_AAS_PointReachabilityAreaIndex( ent->client->ps.origin );
//	ent->client->areabits[i >> 3] |= 1 << (i & 7);
}


