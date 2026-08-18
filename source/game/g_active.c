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
	int			waterlevel;
	// BFP - No battlesuit powerup
	//qboolean	envirosuit = ent->client->ps.powerups[PW_BATTLESUIT] > level.time;

	waterlevel = ent->waterlevel;

	// BFP - No drowning
#if 0
	if ( ent->client->noclip ) {
		ent->client->airOutTime = level.time + 12000;	// don't need air
		return;
	}

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

			// BFP - No battlesuit powerup
#if 0
			if ( envirosuit ) {
				G_AddEvent( ent, EV_POWERUP_BATTLESUIT, 0 );
			} else 
#endif
			{
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
	// BFP - Disabled
	//static vec3_t	range = { 40, 40, 52 };

	if ( !ent->client ) {
		return;
	}

	// dead clients don't activate triggers!
	if ( ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	// BFP - Disabled, to touch correctly without a defined range 
	// and num declaration moved down mins and maxs. 
	// Player monster cannot touch and that causes troubles by stucking from there
	//VectorSubtract( ent->client->ps.origin, range, mins );
	//VectorAdd( ent->client->ps.origin, range, maxs );

	// can't use ent->absmin, because that has a one unit pad
	VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
	VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );
	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

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
			// BFP - Most players and player monster can pick up items with its bounding box
			if ( mins[0] > hit->r.absmax[0] || maxs[0] < hit->r.absmin[0]
			|| mins[1] > hit->r.absmax[1] || maxs[1] < hit->r.absmin[1]
			|| mins[2] > hit->r.absmax[2] || maxs[2] < hit->r.absmin[2] ) {
				continue;
			}
			// BFP - Disabled to make bounding box detection work
#if 0
			if ( !BG_PlayerTouchesItem( &ent->client->ps, &hit->s, level.time ) ) {
				continue;
			}
#endif
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
	// BFP - Flight cost total variable
	float		flightCostTotal = g_flightCost.value + g_flightCostPct.value * ( ent->client->ps.stats[STAT_MAX_KI] * 0.01 );
	// BFP - Random factor for ki charge last digits
	float		rndKiCharge = 0.8f + crandom() * 0.2f;

	client = ent->client;
	client->timeResidual += msec;

	// BFP - Charge ki
	if ( client->kiCharging && ( client->ps.eFlags & EF_AURA )
	&& client->ps.stats[STAT_HITSTUN_TIME] <= 0
	&& client->ps.stats[STAT_KI] < client->ps.stats[STAT_MAX_KI] ) {
		float kiChargeTotal = ( g_kiCharge.value * 0.01 ) + g_kiChargePct.value * ( client->ps.stats[STAT_MAX_KI] * 0.0001 );
		client->ps.stats[STAT_KI] += kiChargeTotal * rndKiCharge;
	}

	// BFP - Block ki consume
	if ( ( client->ps.pm_flags & PMF_BLOCK )
	&& client->ps.stats[STAT_KI] > 0
	&& client->blockKnockbackTime <= 0
	&& random() < 0.75 ) { // a weird random thingy (¬_¬') tried to get the similar result
		// BFP - NOTE: On original BFP, this is handled into another way, so, the formula remains unknown, it tried the best
		float blockCostTotal = ( g_blockCost.value * 0.01 ) + ( g_blockCostPct.value * 0.01 ) * ( client->ps.stats[STAT_MAX_KI] * 0.0001 );
		if ( blockCostTotal < 1 ) {
			blockCostTotal = 1;
		}
		client->ps.stats[STAT_KI] -= blockCostTotal;
	}

	while ( client->timeResidual >= 1000 ) {
		client->timeResidual -= 1000;

		// BFP - No regen powerup
#if 0
		// regenerate
		if ( client->ps.powerups[PW_REGEN] ) {
			if ( ent->health < client->ps.stats[STAT_MAX_HEALTH] ) {
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
		} else 
#endif
		{
			// count down health when over max
			if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] ) {
				ent->health--;
			}
		}

		// BFP - Decrease ki when flying
		if ( ( client->ps.eFlags & EF_FLIGHT )
		&& client->ps.stats[STAT_KI] > 0
		&& !client->kiCharging ) { // don't decrease when charging
			if ( g_flightCostPct.value > 0 && client->ps.persistant[PERS_POWERLEVEL] < 1000 ) { // reduce a bit if the percentage cost is more than 0 and has less powerlevel
				--flightCostTotal;
			}
			client->ps.stats[STAT_KI] -= flightCostTotal;
		}

		// BFP - Regenerate ki
		if ( ( ent->flags & FL_HITSTUN_KI_EMPTY )
		|| ( !( client->pers.cmd.buttons & BUTTON_KI_USE ) && !( client->ps.eFlags & EF_KI_BOOST )
		&& !( client->kiCharging && ( client->ps.eFlags & EF_AURA ) ) // don't increase when charging
		&& !( client->kiCharging && ( client->ps.eFlags & EF_AURA ) && client->ps.stats[STAT_HITSTUN_TIME] > 0 ) // don't increase when trying to charge when stunned
		&& !( ( client->pers.cmd.buttons & BUTTON_ATTACK ) && client->ps.stats[STAT_HITSTUN_TIME] > 0 ) ) ) { // don't increase when trying to attack when stunned
			client->ps.stats[STAT_KI] += g_kiRegen.value + ( g_kiRegenPct.value * client->ps.stats[STAT_MAX_KI] * 0.01 );
			if ( ent->flags & FL_HITSTUN_KI_EMPTY ) {
				client->ps.stats[STAT_HITSTUN_TIME] = 0;
				ent->flags &= ~FL_HITSTUN_KI_EMPTY;
				if ( ( client->pers.cmd.buttons & BUTTON_ATTACK ) && client->ps.stats[STAT_KI] <= 0 ) {
					client->ps.stats[STAT_HITSTUN_TIME] = 1000;
					ent->flags |= FL_HITSTUN_KI_EMPTY;
				}
			}
		}

		// count down armor when over max
		if ( client->ps.stats[STAT_ARMOR] > client->ps.stats[STAT_MAX_HEALTH] ) {
			client->ps.stats[STAT_ARMOR]--;
		}
	}

	// BFP - Set maximum ki
	if ( client->ps.stats[STAT_KI] > client->ps.stats[STAT_MAX_KI] ) {
		client->ps.stats[STAT_KI] = client->ps.stats[STAT_MAX_KI];
	}

	// BFP - If ki drops to 0, disable flight
	if ( client->ps.stats[STAT_KI] <= 0 ) {
		client->ps.stats[STAT_KI] = 0;
		client->ps.eFlags &= ~EF_FLIGHT;
	}

	// BFP - When the player doesn't have more ki, gets a hit stun
	if ( ( client->ps.stats[STAT_KI] <= 0 )
		|| ( ( client->ps.eFlags & EF_FLIGHT ) && client->ps.stats[STAT_KI] < flightCostTotal && client->ps.stats[STAT_HITSTUN_TIME] <= 0 )
		|| ( ( client->pers.cmd.buttons & BUTTON_ATTACK ) && client->ps.stats[STAT_HITSTUN_TIME] > 0 ) ) {
		if ( ( client->ps.eFlags & EF_FLIGHT ) && client->ps.stats[STAT_KI] < flightCostTotal && client->ps.stats[STAT_HITSTUN_TIME] <= 0 ) {
			client->ps.stats[STAT_KI] /= flightCostTotal;
		}
		if ( client->ps.stats[STAT_KI] <= 0 ) {
			client->ps.stats[STAT_HITSTUN_TIME] = 1000;
			ent->flags |= FL_HITSTUN_KI_EMPTY;
		}
	}

	// BFP - Ki boost consumption
	if ( ( ( client->pers.cmd.buttons & BUTTON_KI_USE ) || ( client->ps.eFlags & EF_KI_BOOST ) )
	&& client->ps.stats[STAT_KI] > 0
	&& client->ps.stats[STAT_HITSTUN_TIME] <= 0
	&& !( client->ps.pm_flags & PMF_BLOCK )
	&& !( ( client->ps.ammo[client->ps.weapon] & AMMOF_ATK_FORCEFIELD ) && client->ps.weaponstate == WEAPON_ACTIVE )
	&& client->ps.weaponstate != WEAPON_STUN ) {
		// BFP - NOTE: On original BFP, this is handled into another way, so, the formula remains unknown, it tried the best
		float boostCostTotal = ( g_boostCost.value * 0.001 ) + ( g_boostCostPct.value * 0.1 ) * client->ps.stats[STAT_MAX_KI] * 0.0001;
		// use msec to adjust the consumption
		client->kiResidual += boostCostTotal * msec;
		if ( client->kiResidual >= 1.0f ) {
			int drop = (int)client->kiResidual;
			client->ps.stats[STAT_KI] -= drop;
			client->kiResidual -= drop;
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

	client->ps.commandTime = client->pers.cmd.serverTime;
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
			if ( ent->s.eType != ET_PLAYER ) {
				break;		// not in the player model
			}
			if ( g_dmflags.integer & DF_NO_FALLING ) {
				break;
			}
			// BFP - When the player is falling with stunned status
			// if powerlevel is very low, weaker at falling (player has less max health)
			// but when the powerlevel is getting higher, the less damage it will have (player has more max health)
			if ( client->ps.stats[STAT_HITSTUN_TIME] > 0 ) {
				damage = 5; // medium fall
				if ( event == EV_FALL_FAR ) {
					damage = 10;
				}
				ent->pain_debounce_time = level.time + 200;	// no normal pain sound
				G_Damage ( ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING );
			}
			break;

		case EV_FIRE_WEAPON:
		{
			FireWeapon( ent );
			break;
		}

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
			ent->health = client->ps.stats[STAT_MAX_HEALTH]; // BFP - Before Q3: + 25

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
BlockHandling
=================
*/
static void BlockHandling( gclient_t *client, usercmd_t *ucmd ) { // BFP - Block, reflect ki attacks and reduce health damage
	// if the block length duration hasn't been expired yet and 
	// pressing ki charge (if the aura is lighting) or attack buttons, then stop blocking and start the delay
	if ( ( client->ps.pm_flags & PMF_BLOCK ) 
	&& ( client->kiCharging
	|| ( ucmd->buttons & BUTTON_KI_CHARGE )
	|| ( ucmd->buttons & BUTTON_ATTACK )
	|| ( ucmd->buttons & BUTTON_MELEE ) ) ) {
		client->ps.pm_flags &= ~PMF_BLOCK;
		client->blockTime = 0;
		client->blockDelayTime = level.time + (g_blockDelay.integer * 1000);
	}

	// initialize the blocking and start the block length duration, specifically, ki boost and aura are disabled
	if ( !( client->ps.pm_flags & PMF_BLOCK )
	&& ( ucmd->buttons & BUTTON_BLOCK )
	&& !( ucmd->buttons & BUTTON_KI_CHARGE )
	&& client->blockTime <= 0
	&& client->blockDelayTime <= 0 ) {
		client->ps.pm_flags |= PMF_BLOCK;
		if ( ucmd->buttons & BUTTON_KI_USE ) {
			client->ps.eFlags &= ~EF_KI_BOOST;
			client->ps.eFlags &= ~EF_AURA;
		}
		client->blockTime = level.time + (g_blockLength.integer * 1000);
	}

	// when the block length duration has been expired, then start the delay to avoid user 
	if ( ( client->ps.pm_flags & PMF_BLOCK ) 
	&& client->blockTime > 0 
	&& level.time >= client->blockTime ) {
		client->ps.pm_flags &= ~PMF_BLOCK;
		client->blockTime = 0;
		client->blockDelayTime = level.time + (g_blockDelay.integer * 1000);
	}

	// knockback when reflecting from attacks
	if ( ( client->ps.pm_flags & PMF_BLOCK ) 
	&& client->blockTime > 0 
	&& level.time >= client->blockKnockbackTime ) {
		client->blockKnockbackTime = 0;
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

	if ( client->ps.stats[STAT_HITSTUN_TIME] > 0 ) {
		return;
	}

	if ( !( ucmd->buttons & BUTTON_MELEE ) ) {
		client->ps.pm_flags &= ~PMF_MELEE;
	}

	if ( !( ucmd->buttons & BUTTON_TALK ) && ( ucmd->buttons & BUTTON_MELEE )
	&& !client->kiCharging
	&& client->ps.weaponTime <= 0 ) {
		pm->meleeHit = CheckMeleeAttack( ent );
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

	if ( !ent || !ent->client ) {
		return qfalse;
	}

	// set diagonal direction, included the up vector for upward detection
	AngleVectors( ent->client->ps.viewangles, NULL, right, up );

	// upward detection, avoid if the player is touching the surface above
	VectorMA( ent->client->ps.origin, 25, up, start );
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
	tr.endpos[2] += 16; // place the position a bit up
	VectorCopy( tr.endpos, ent->client->ps.origin );

	// sound event
	BG_AddPredictableEventToPlayerstate( EV_ZANZOKEN_IN, 0, &ent->client->ps, -1 );

	return qtrue;
}

/*
=================
ZanzokenHandling
=================
*/
static void ZanzokenHandling( gentity_t *ent, usercmd_t *ucmd ) { // BFP - Handling short-range teleport
	const int	ZANZOKEN_NUMBER_TIMES_ALLOWED = 10, 
				ZANZOKEN_ABUSE_DELAY = 2000,
				MAX_ZANZOKEN_PRESS_TIME = 240,
				ZANZOKEN_COOLDOWN = 70;
	gclient_t	*client = ent->client;
	int			currentTime = level.time; // use level.time which is not affected by timescale

	if ( client->ps.weaponstate == WEAPON_ACTIVE
	|| client->ps.weaponstate == WEAPON_BEAMSTRUGGLE
	|| client->ps.weaponstate == WEAPON_STUN ) {
		return;
	}

	// check if zanzoken is on cooldown
	if ( client->zanzokenLastUsed > 0
	&& currentTime - client->zanzokenLastUsed < ZANZOKEN_COOLDOWN ) {
		return;
	}

	if ( level.time < client->ultimateTierUnlockedTime ) {
		return;
	}

	// zanzoken cannot be used with ki charging status
	if ( client->kiCharging ) {
		return;
	}

	// restriction: stop abusing zanzoken technique all time
	if ( client->zanzokenNumberTimesAllowed >= ZANZOKEN_NUMBER_TIMES_ALLOWED ) {
		client->zanzokenNumberTimesAllowed = 0;
		client->zanzokenDelay = currentTime;
		return;
	}

	if ( client->zanzokenDelay > 0 && currentTime - client->zanzokenDelay <= ZANZOKEN_ABUSE_DELAY ) {
		return;
	}

	if ( ucmd->rightmove && client->zanzokenPressTime <= 0 ) {
		client->zanzokenPressTime = currentTime;
		client->zanzokenNow = qfalse;
		// handle directions to avoid pressing the opposite
		if ( ucmd->rightmove > 0 ) {
			client->zanzokenLeft = qfalse;
			client->zanzokenRight = qtrue;
		} else {
			client->zanzokenLeft = qtrue;
			client->zanzokenRight = qfalse;
		}
	}

	// once pressed and having one moment to press again, zanzoken will be possible at these milliseconds
	if ( !ucmd->rightmove && client->zanzokenPressTime > 0 ) {
		int elapsed = currentTime - client->zanzokenPressTime;
		
		if ( elapsed > 50 && elapsed <= MAX_ZANZOKEN_PRESS_TIME && !client->zanzokenNow ) {
			client->zanzokenNow = qtrue;
			client->zanzokenNumberTimesAllowed++;
		}

		if ( elapsed > MAX_ZANZOKEN_PRESS_TIME ) {
			client->zanzokenNumberTimesAllowed = 0;
			client->zanzokenPressTime = 0;
			client->zanzokenNow = qfalse;
			client->zanzokenLeft = qfalse;
			client->zanzokenRight = qfalse;
			return;
		}
	}

	if ( client->ps.stats[STAT_KI] > ( client->ps.stats[STAT_MAX_KI] * 0.05 )
	&& ucmd->rightmove && client->zanzokenNow ) {
		int range = ( ucmd->rightmove > 0 ) ? 500 : -500;

		// handle the directions correctly
		if ( ( ucmd->rightmove > 0 && !client->zanzokenRight )
		|| ( ucmd->rightmove < 0 && !client->zanzokenLeft ) ) {
			client->zanzokenLeft = qfalse;
			client->zanzokenRight = qfalse;
			return;
		}

		// put in 0.1 msec delay before the player can 'zanzoken' out of stun
		if ( client->ps.stats[STAT_HITSTUN_TIME] > 2900 ) {
			client->zanzokenPressTime = 0;
			client->zanzokenNow = qfalse;
			client->zanzokenLeft = qfalse;
			client->zanzokenRight = qfalse;
			return;
		}

		if ( Zanzoken( ent, range ) ) {
			// block and stun statuses are removed when using zanzoken
			if ( client->ps.stats[STAT_HITSTUN_TIME] <= 3000 ) {
				client->ps.stats[STAT_HITSTUN_TIME] = 0;
			}
			client->ps.pm_flags &= ~PMF_BLOCK;
			// consumes 5% of ki
			client->ps.stats[STAT_KI] -= ( client->ps.stats[STAT_MAX_KI] * 0.05 );
			client->zanzokenPressTime = 0;
			client->zanzokenNow = qfalse;
			client->zanzokenLeft = qfalse;
			client->zanzokenRight = qfalse;
			client->zanzokenLastUsed = currentTime; // Set cooldown
		}
	}
}


/*
===========
Client_KiConsumption
===========
*/
static void Client_KiConsumption( gclient_t *client, int addTime, int kiConsume ) { // BFP - Ki consumption when using ki attacks
	if ( client->ps.stats[STAT_KI] >= 0 ) { // avoid consuming more ki than available
		client->ps.stats[STAT_KI] -= kiConsume;
		if ( client->ps.stats[STAT_KI] < 0 ) {
			client->ps.stats[STAT_KI] = 0;
		}
		client->ps.weaponTime += addTime;
	} else { // not enough ki
		client->ps.eFlags &= ~EF_FIRING;
	}
}


/*
===========
Client_KiCost
===========
*/
static float Client_KiCost( gclient_t *client, bfpWeaponCfgDef_t *wpCfg ) { // BFP - kiCost, kiCostAsPct and kiPct
	float	kiCost = ( wpCfg->kiCost > 0 ) ? wpCfg->kiCost : 0;
	float	kiPct = ( wpCfg->kiPct > 1 ) ? 1 : wpCfg->kiPct;
	if ( wpCfg->kiCostAsPct && kiPct > 0 ) {
		kiCost = client->ps.stats[STAT_MAX_KI] * kiPct;
	}
	return kiCost;
}


/*
============
Client_ChargeKiAttackState
============
*/
static void Client_ChargeKiAttackState( gclient_t *client, bfpWeaponCfgDef_t *wpCfg, int minCharge, int maxCharge, int addTime, int kiConsume ) { // BFP - Charge ki attack state
	Client_KiConsumption( client, addTime, kiConsume );
	if ( client->ps.stats[STAT_KI] < kiConsume ) {
		return;
	}
	if ( !wpCfg->chargeAttack && !wpCfg->chargeAutoFire ) {
		return;
	}
	if ( ( wpCfg->chargeAutoFire
	|| maxCharge < client->ps.generic1
	|| ( maxCharge > 0 && client->ps.generic1 < maxCharge )
	|| ( maxCharge <= 0 && client->ps.generic1 < minCharge ) ) // if minCharge is superior to maxCharge, e.g. minCharge 3, maxCharge 0
	&& client->ps.generic1 < ATTACK_CHARGE_LIMIT ) {
		++client->ps.generic1;
	}
}

/*
============
Client_RandomWeaponTime
============
*/
static int Client_RandomWeaponTime( bfpWeaponCfgDef_t *wpCfg ) { // BFP - randomWeaponTime calculation
	int		weaponTime = wpCfg->weaponTime;
	float	randomWeaponTime = 0;
	if ( wpCfg->randomWeaponTime > 0 ) {
		randomWeaponTime = random() * wpCfg->randomWeaponTime;
	}
	weaponTime += randomWeaponTime;
	return weaponTime;
}

/*
=============
Client_MovementPenaltyStun
=============
*/
static qboolean Client_MovementPenaltyStun( gclient_t *client, bfpWeaponCfgDef_t *wpCfg, usercmd_t *ucmd ) { // BFP - movementPenalty stun
	if ( wpCfg->movementPenalty <= 0 ) {
		return qfalse;
	}

	if ( wpCfg->chargeAutoFire && wpCfg->minCharge >= 0
	&& client->ps.generic1 < wpCfg->minCharge ) {
		return qfalse;
	}

	// ki boost/aura can't be used while under movementPenalty
	ucmd->buttons &= ~( BUTTON_KI_USE | BUTTON_KI_CHARGE );
	client->pers.cmd.buttons &= ~( BUTTON_KI_USE | BUTTON_KI_CHARGE );
	client->ps.eFlags &= ~( EF_AURA | EF_KI_BOOST );

	if ( !( ucmd->buttons & BUTTON_ATTACK )
	|| ( ucmd->buttons & BUTTON_MELEE )
	|| client->ps.weapon != ucmd->weapon ) { // avoid when changing weapon
		client->ps.weaponTime = wpCfg->movementPenalty;
		client->ps.eFlags &= ~EF_FIRING;
		client->ps.weaponstate = WEAPON_STUN;
		return qtrue;
	}
	return qfalse;
}

/*
=============
Client_Weapon
=============
*/
static void Client_Weapon( gentity_t *ent, usercmd_t *ucmd, pmove_t *pm ) { // BFP - Client weapon handling
	gclient_t		*client = ent->client;
	bfpWeaponCfgDef_t	*wpCfg;
	// BFP - Ki cost
	float		kiCost;
	// BFP - Weapon time
	int			weaponTime;

	// BFP - Hit stun and ultimate tier, avoid shooting if the player is in this status
	if ( client->ps.stats[STAT_HITSTUN_TIME] > 0 || ( level.time < client->ultimateTierUnlockedTime ) ) {
		return;
	}

	// don't allow attack until all buttons are up
	if ( client->ps.pm_flags & PMF_RESPAWNED ) {
		return;
	}

	// ignore if spectator
	if ( client->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
		return;
	}

	// check for dead player
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	// BFP - Don't allow attack when recharging ki
	if ( client->kiCharging ) {
		ucmd->buttons &= ~BUTTON_ATTACK;
		return;
	}

	// BFP - Melee only, players can't fire weapons at all
	if ( g_meleeOnly.integer > 0 ) {
		return;
	}

	wpCfg = BG_GetClientWeaponDefForSlot( client->ps.clientNum, client->ps.weapon );

	// BFP - Monster gamemode, player monster with g_monster 1 uses its own weapon
	if ( ( client->ps.eFlags & EF_MONSTER ) && g_monster.integer > 0 ) {
		wpCfg = BG_SetMonsterDefaultWeaponDef();
	}

	if ( !wpCfg ) { // safe fallback
		wpCfg = BG_SetDefaultWeaponDef();
	}

	if ( !wpCfg ) { // don't continue
		return;
	}

	// if it isn't unlocked, or it has no active ammo locked by powerlevel, force to the first valid weapon selection
	if ( ucmd->weapon < BFP_NUM_WEAPONS ) {
		if ( !( client->ps.stats[STAT_WEAPONS] & ( 1 << ucmd->weapon ) )
		|| client->ps.ammo[ucmd->weapon] == 0 ) {
			ucmd->weapon = client->ps.weapon;
		}
	}

	kiCost = Client_KiCost( client, wpCfg );
	weaponTime = Client_RandomWeaponTime( wpCfg );

	// BFP - Debug extracted weapon from bfp_weapon.cfg 
#if 0
	Com_Printf( "Client_Weapon - WEAPONDEF: client %d, slot %d -> (attackName %s, chargeAttack %d, chargeAutoFire %d, kiCost %d, kiPct %f, weaponTime %d, randomWeaponTime %d)\n",
		client->ps.clientNum, client->ps.weapon, 
		wpCfg ? wpCfg->attackName : "NULL", 
		wpCfg->chargeAttack, 
		wpCfg->chargeAutoFire, 
		wpCfg->kiCost, 
		wpCfg->kiPct, 
		wpCfg->weaponTime, 
		wpCfg->randomWeaponTime );
#endif

	// BFP - Melee, avoid shooting if the player is in this status
	if ( ucmd->buttons & BUTTON_MELEE ) {
		// cancel attack
		if ( ( client->ps.eFlags & EF_FIRING )
		&& ( ( wpCfg->attackType == ATK_FORCEFIELD && wpCfg->chargeAutoFire ) 
			|| wpCfg->movementPenalty > 0 )
		&& client->ps.weaponstate != WEAPON_STUN ) {
			client->ps.eFlags &= ~EF_FIRING;
			// apply movementPenalty
			if ( !Client_MovementPenaltyStun( client, wpCfg, ucmd ) ) {
				client->ps.weaponstate = WEAPON_READY;
				client->ps.weaponTime = 0;
			}
			client->ps.generic1 = 0;
			return;
		}
		else if ( client->ps.weaponstate != WEAPON_STUN ) {
			// avoid playing change weapon sound continuously while changing weapon by pressing melee button
			if ( client->ps.weapon != ucmd->weapon ) {
				return;
			}
			// only use when there's no splitting ki ball until it has been splitted or collided, 
			// unless if the player wanna change the weapon from this state
			if ( client->ps.weaponstate != WEAPON_ACTIVE ) {
				client->ps.weaponstate = WEAPON_READY;
				client->ps.generic1 = 0;
			}
			// Melee fight handling
			if ( pm->meleeHit && client->ps.weaponTime <= 0 ) {
				int rndSnd = rand() % 6;
				client->ps.weaponTime += 300;
				client->ps.pm_flags |= PMF_MELEE;
				// melee sound event is randomly executed
				if ( rndSnd > 3 ) {
					BG_AddPredictableEventToPlayerstate( EV_MELEE, 0, &ent->client->ps, -1 );
				}
			}
			return;
		}
	}

	// BFP - Weapon states, Q3 doesn't have this way
	switch( client->ps.weaponstate ) {
	case WEAPON_READY:
		if ( !( ucmd->buttons & BUTTON_ATTACK ) ) {
			// movementPenalty
			if ( !wpCfg->chargeAttack && !wpCfg->chargeAutoFire && wpCfg->movementPenalty > 0
			&& ( client->ps.eFlags & EF_FIRING ) ) {
				if ( !( ucmd->buttons & BUTTON_ATTACK )
				|| ( ucmd->buttons & BUTTON_MELEE )
				|| client->ps.weapon != ucmd->weapon ) {
					if ( wpCfg->movementPenalty > client->ps.weaponTime ) {
						client->ps.weaponTime = wpCfg->movementPenalty - client->ps.weaponTime;
					} else {
						client->ps.weaponTime = wpCfg->movementPenalty;
					}
					client->ps.weaponstate = WEAPON_STUN;
				}
			}
			client->ps.eFlags &= ~EF_FIRING;
			client->ps.generic1 = 0;
		} else {
			if ( client->ps.weaponTime <= 0 ) {
				if ( wpCfg->chargeAttack || wpCfg->chargeAutoFire ) {
					client->ps.stats[STAT_KI] -= kiCost;
				}
				// BFP - sbeam attack type
				if ( wpCfg->attackType == ATK_SBEAM ) {
					client->ps.stats[STAT_KI] -= kiCost;
					client->ps.weaponTime += weaponTime;
					client->ps.weaponstate = WEAPON_ACTIVE;
					// fire and make a sound
					BG_AddPredictableEventToPlayerstate( EV_FIRE_WEAPON, 0, &ent->client->ps, -1 );
					break;
				}
				if ( wpCfg->chargeAttack || wpCfg->chargeAutoFire ) {
					client->ps.weaponTime += weaponTime;
				}
				client->ps.weaponstate = WEAPON_FIRING;
			}
		}
		break;
	case WEAPON_DROPPING:
	case WEAPON_RAISING:
		break;
	case WEAPON_FIRING:
		// don't allow ki charging while charging the attack, skip if BUTTON_KI_USE is also held
		if ( ( client->kiCharging && !( ucmd->buttons & BUTTON_KI_USE ) )
		|| ( ( ucmd->buttons & BUTTON_KI_CHARGE ) && !( ucmd->buttons & BUTTON_KI_USE ) ) ) {
			break;
		}

		// BFP - Charging states here
		if ( wpCfg->chargeAttack && !wpCfg->chargeAutoFire ) {
			if ( wpCfg->attackType == ATK_FORCEFIELD ) {
				client->ps.eFlags |= EF_FIRING;
			}
			if ( !( ucmd->buttons & BUTTON_ATTACK ) ) {
				// BFP - When the ki attack is fully charged, enter beam firing state
				// or enter splitting ki ball firing state if it's a splitting ki ball

				// no fully charged, skip...
				if ( client->ps.generic1 < wpCfg->minCharge ) {
					client->ps.weaponstate = WEAPON_RAISING;
					break;
				}

				// handle the animation for the start of beam or ball shoot
				switch( wpCfg->attackType ) {
				case ATK_MISSILE:
					client->ps.weaponstate = WEAPON_READY;
					client->ps.weaponTime = 800; // 0.8 sec to keep the strike animation
					// movementPenalty
					Client_MovementPenaltyStun( client, wpCfg, ucmd );
					break;
				case ATK_RDMISSILE:
				case ATK_BEAM:
					client->ps.weaponstate = WEAPON_ACTIVE;
					client->ps.weaponTime = weaponTime;
					// movementPenalty
					if ( wpCfg->movementPenalty > 0 ) {
						client->ps.weaponTime = wpCfg->movementPenalty;
					}
					break;
				case ATK_FORCEFIELD:
					client->ps.eFlags |= EF_FIRING;
					client->ps.weaponstate = WEAPON_ACTIVE;
					client->ps.weaponTime = weaponTime;
				}

				// fire and make a sound
				BG_AddPredictableEventToPlayerstate( EV_FIRE_WEAPON, 0, &ent->client->ps, -1 );
			}
		}

		// chargeAutoFire: keep firing while attack key is holding
		if ( wpCfg->chargeAutoFire
		&& client->ps.weaponTime <= 0 ) {
			if ( !( ucmd->buttons & BUTTON_ATTACK )
			|| ( ucmd->buttons & BUTTON_MELEE )
			|| client->ps.weapon != ucmd->weapon ) { // avoid when changing weapon
				client->ps.weaponstate = WEAPON_READY;
				client->ps.eFlags &= ~EF_FIRING;
				break;
			}
			if ( ucmd->buttons & BUTTON_ATTACK ) {
				client->ps.eFlags |= EF_FIRING;
				BG_AddPredictableEventToPlayerstate( EV_FIRE_WEAPON, 0, &ent->client->ps, -1 );
			}
			if ( wpCfg->attackType == ATK_BEAM
			|| wpCfg->attackType == ATK_FORCEFIELD
			|| wpCfg->attackType == ATK_RDMISSILE ) {
				client->ps.weaponstate = WEAPON_ACTIVE;
			}
		}
		if ( wpCfg->chargeAutoFire && wpCfg->movementPenalty > 0 ) {
			if ( !( ucmd->buttons & BUTTON_ATTACK )
			|| ( ucmd->buttons & BUTTON_MELEE )
			|| client->ps.weapon != ucmd->weapon ) {
				if ( wpCfg->movementPenalty > client->ps.weaponTime ) {
					client->ps.weaponTime = wpCfg->movementPenalty - client->ps.weaponTime;
				} else {
					client->ps.weaponTime = wpCfg->movementPenalty;
				}
				if ( wpCfg->chargeAutoFire && wpCfg->minCharge >= 0
				&& client->ps.generic1 < wpCfg->minCharge ) {
					client->ps.weaponTime = 0;
					client->ps.weaponstate = WEAPON_READY;
					client->ps.eFlags &= ~EF_FIRING;
					break;
				}
				client->ps.eFlags &= ~EF_FIRING;
				client->ps.weaponstate = WEAPON_STUN;
				break;
			}
		}

		// check for fire
		if ( client->ps.weaponTime <= 0 && ( ucmd->buttons & BUTTON_ATTACK )
		&& ( wpCfg->chargeAttack || wpCfg->chargeAutoFire ) ) {
			Client_ChargeKiAttackState( client, wpCfg, wpCfg->minCharge, wpCfg->maxCharge, weaponTime, kiCost );
		}

		if ( !wpCfg->chargeAttack && !wpCfg->chargeAutoFire
		&& client->ps.weaponTime <= 0 ) {
			if ( wpCfg->attackType == ATK_HITSCAN ) {
				if ( ucmd->buttons & BUTTON_ATTACK ) {
					Client_KiConsumption( client, weaponTime, kiCost );
					if ( client->ps.stats[STAT_KI] >= kiCost ) {
						client->ps.eFlags |= EF_FIRING;
						BG_AddPredictableEventToPlayerstate( EV_FIRE_WEAPON, 0, &ent->client->ps, -1 );
					}
					client->ps.weaponstate = WEAPON_READY;
				} else {
					client->ps.weaponstate = WEAPON_READY;
				}
				break;
			}

			if ( client->ps.stats[STAT_KI] >= kiCost ) {
				client->ps.eFlags |= EF_FIRING;
				BG_AddPredictableEventToPlayerstate( EV_FIRE_WEAPON, 0, &ent->client->ps, -1 );
			}
			Client_KiConsumption( client, weaponTime, kiCost );
			if ( wpCfg->attackType == ATK_BEAM ) {
				client->ps.weaponstate = WEAPON_ACTIVE;
			}
			// movementPenalty
			if ( wpCfg->attackType == ATK_MISSILE || wpCfg->attackType == ATK_RDMISSILE ) {
				if ( !Client_MovementPenaltyStun( client, wpCfg, ucmd ) ) {
					client->ps.weaponstate = WEAPON_READY;
				}
			}
		}
		if ( !wpCfg->chargeAttack && !wpCfg->chargeAutoFire && wpCfg->movementPenalty > 0 ) {
			if ( !( ucmd->buttons & BUTTON_ATTACK )
			|| ( ucmd->buttons & BUTTON_MELEE )
			|| client->ps.weapon != ucmd->weapon ) {
				if ( wpCfg->movementPenalty > client->ps.weaponTime ) {
					client->ps.weaponTime = wpCfg->movementPenalty - client->ps.weaponTime;
				} else {
					client->ps.weaponTime = wpCfg->movementPenalty;
				}
				client->ps.eFlags &= ~EF_FIRING;
				client->ps.weaponstate = WEAPON_STUN;
				break;
			}
		}
		break;
	// BFP - NOTE: The beam is triggering until pressing the attack key again after holded, using ki charge or blocking
	// Pressing attack key again or changing weapon, the beam is exploded before the impact
	case WEAPON_ACTIVE:
	// BFP - NOTE: Lock movement during beam struggle
	case WEAPON_BEAMSTRUGGLE:
		//client->ps.eFlags |= EF_FIRING; // keep playing firing sound

		// chargeAutoFire: keep firing while attack key is holding
		if ( wpCfg->attackType == ATK_BEAM  && wpCfg->chargeAutoFire ) {
			if ( ( ucmd->buttons & BUTTON_ATTACK )
			&& client->ps.weaponTime <= 0 ) {
				// BG_AddPredictableEventToPlayerstate( EV_FIRE_WEAPON, 0, &ent->client->ps, -1 );
				Client_ChargeKiAttackState( client, wpCfg, wpCfg->minCharge, wpCfg->maxCharge, weaponTime, kiCost );
			}
			if ( !( ucmd->buttons & BUTTON_ATTACK )
			|| ( client->kiCharging && ( client->ps.eFlags & EF_AURA ) )
			|| ( client->ps.pm_flags & PMF_BLOCK ) ) {
				// movementPenalty
				if ( !Client_MovementPenaltyStun( client, wpCfg, ucmd ) ) {
					client->ps.weaponstate = WEAPON_READY;
				}
			}
			break;
		}

		if ( ( wpCfg->chargeAttack || wpCfg->chargeAutoFire ) 
		&& wpCfg->attackType == ATK_FORCEFIELD ) {
			if ( ( ucmd->buttons & BUTTON_ATTACK )
			&& client->ps.weaponTime <= 0 ) {
				BG_AddPredictableEventToPlayerstate( EV_FIRE_WEAPON, 0, &ent->client->ps, -1 );
				Client_ChargeKiAttackState( client, wpCfg, wpCfg->minCharge, wpCfg->maxCharge, weaponTime, kiCost );
			}
			if ( wpCfg->chargeAttack && !wpCfg->chargeAutoFire && !( ucmd->buttons & BUTTON_ATTACK ) ) {
				// movementPenalty
				if ( !Client_MovementPenaltyStun( client, wpCfg, ucmd ) ) { // no weaponTime delay with chargeAttack
					client->ps.weaponTime = 0;
					client->ps.weaponstate = WEAPON_READY;
				}
			}
			if ( wpCfg->chargeAutoFire && !( ucmd->buttons & BUTTON_ATTACK ) ) {
				// movementPenalty
				if ( !Client_MovementPenaltyStun( client, wpCfg, ucmd ) ) {
					client->ps.weaponstate = WEAPON_READY;
					client->ps.eFlags &= ~EF_FIRING;
				}
			}
			break;
		}

		if ( wpCfg->attackType == ATK_RDMISSILE && wpCfg->chargeAutoFire ) {
			if ( ( ucmd->buttons & BUTTON_ATTACK )
			&& client->ps.weaponTime <= 0 ) {
				BG_AddPredictableEventToPlayerstate( EV_FIRE_WEAPON, 0, &ent->client->ps, -1 );
				Client_ChargeKiAttackState( client, wpCfg, wpCfg->minCharge, wpCfg->maxCharge, weaponTime, kiCost );
			}

			// movementPenalty
			if ( !Client_MovementPenaltyStun( client, wpCfg, ucmd ) ) {
				if ( !( ucmd->buttons & BUTTON_ATTACK )
				|| ( ucmd->buttons & BUTTON_MELEE )
				|| client->ps.weapon != ucmd->weapon ) { // avoid when changing weapon
					client->ps.weaponstate = WEAPON_RAISING;
				}
			}
			break;
		}
		client->ps.generic1 = 0;

		// BFP - sbeam attack type
		if ( wpCfg->attackType == ATK_SBEAM ) {
			if ( client->ps.weaponTime <= 0 ) {
				client->ps.stats[STAT_KI] -= kiCost;
				client->ps.weaponTime += weaponTime;
			}
			if ( !( ucmd->buttons & BUTTON_ATTACK )
			|| ( client->kiCharging && ( client->ps.eFlags & EF_AURA ) )
			|| ( ucmd->buttons & BUTTON_MELEE ) ) {
				// movementPenalty
				if ( wpCfg->movementPenalty > 0 ) {
					if ( wpCfg->movementPenalty > client->ps.weaponTime ) {
						client->ps.weaponTime = wpCfg->movementPenalty - client->ps.weaponTime;
					} else {
						client->ps.weaponTime = wpCfg->movementPenalty;
					}
					client->ps.weaponstate = WEAPON_STUN;
				} else {
					client->ps.weaponstate = WEAPON_READY;
				}
			}
			// movementPenalty
			Client_MovementPenaltyStun( client, wpCfg, ucmd );
			break;
		}

		if ( wpCfg->attackType == ATK_BEAM ) {
			// movementPenalty
			if ( wpCfg->movementPenalty > 0
			&& ( client->ps.eFlags & EF_FIRING ) 
			&& ( ( ucmd->buttons & BUTTON_ATTACK ) || ( ucmd->buttons & BUTTON_MELEE )
			|| client->ps.weapon != ucmd->weapon ) ) {
				if ( wpCfg->movementPenalty > client->ps.weaponTime ) {
					client->ps.weaponTime = wpCfg->movementPenalty - client->ps.weaponTime;
				} else {
					client->ps.weaponTime = wpCfg->movementPenalty;
				}
				client->ps.weaponstate = WEAPON_STUN;
				break;
			}
			if ( ( ucmd->buttons & BUTTON_ATTACK )
			|| ( client->kiCharging && ( client->ps.eFlags & EF_AURA ) )
			|| ( client->ps.pm_flags & PMF_BLOCK ) ) {
				client->ps.weaponstate = WEAPON_READY;
				// movementPenalty
				Client_MovementPenaltyStun( client, wpCfg, ucmd );
			}
		}
	}
	
	// debug print about weapon states, EF_FIRING and weapon time
#if 0
	switch( client->ps.weaponstate ) {
	case WEAPON_READY: Com_Printf( "client->ps.weaponstate = WEAPON_READY\n" ); break;
	case WEAPON_RAISING: Com_Printf( "client->ps.weaponstate = WEAPON_RAISING\n" ); break;
	case WEAPON_DROPPING: Com_Printf( "client->ps.weaponstate = WEAPON_DROPPING\n" ); break;
	case WEAPON_FIRING: Com_Printf( "client->ps.weaponstate = WEAPON_FIRING\n" ); break;
	case WEAPON_ACTIVE: Com_Printf( "client->ps.weaponstate = WEAPON_ACTIVE\n" ); break;
	case WEAPON_STUN: Com_Printf( "client->ps.weaponstate = WEAPON_STUN\n" ); break;
	}
	Com_Printf( "client->ps.eFlags & EF_FIRING: %d\n", client->ps.eFlags & EF_FIRING );
	Com_Printf( "weaponTime: %d\n", client->ps.weaponTime );
#endif
}

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
	ucmd = &client->pers.cmd;

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
		// BFP - No impressive, gauntlet, defend, assist and cap medals
		client->ps.eFlags &= ~(EF_AWARD_EXCELLENT /*| EF_AWARD_IMPRESSIVE | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP*/ );
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

	// set up for pmove
	oldEventSequence = client->ps.eventSequence;

	memset (&pm, 0, sizeof(pm));

	if ( client->ps.pm_type != PM_DEAD && client->ps.pm_type != PM_SPECTATOR ) {
		// BFP - Short-Range Teleport (Zanzoken)
		ZanzokenHandling( ent, ucmd );

		// BFP - Hit stun melee delay time
		if ( client->hitStunMeleeDelayTime > 0 
		&& level.time >= client->hitStunMeleeDelayTime ) {
			client->hitStunMeleeDelayTime = 0;
		}

		// BFP - g_chargeDelay cvar for ki charge animation and appearing the aura after this time
		if ( !( ucmd->buttons & BUTTON_KI_USE ) 
		&& ( ucmd->buttons & BUTTON_KI_CHARGE ) 
		&& !client->kiCharging ) {
			client->ps.pm_time = ( g_chargeDelay.integer > 0 ) ? g_chargeDelay.integer : 0;
		}

		// BFP - Ki use has 2 options: "kiusetoggle" to toggle and "+button8" when key is being hold
		if ( client->ps.stats[STAT_HITSTUN_TIME] <= 0
		&& !( client->ps.pm_flags & PMF_BLOCK )
		&& ( ( ucmd->buttons & BUTTON_KI_USE ) // BFP - Using Ki
			|| ( client->ps.eFlags & EF_KI_BOOST ) ) // BFP - When "kiusetoggle" is binded, enables/disables
		&& client->ps.weaponstate != WEAPON_STUN ) {
			client->ps.eFlags |= EF_KI_BOOST; // Handle ki boost status
			client->ps.eFlags |= EF_AURA;
		} else {
			if ( !( client->ps.pm_flags & PMF_BLOCK ) // BFP - Handle block status from this conditional
			&& !( ucmd->buttons & BUTTON_KI_CHARGE ) ) { // BFP - If it's charging while it was using ki boost, don't remove the aura!
				client->ps.eFlags &= ~EF_AURA;
				client->ps.eFlags &= ~EF_KI_BOOST; // Handle ki boost status
			}
		}

		// BFP - Block, reflect ki attacks and reduce health damage
		BlockHandling( client, ucmd );

		// BFP - Melee handling
		MeleeHandling( ent, ucmd, &pm );

		// BFP - Client weapon handling
		Client_Weapon( ent, ucmd, &pm );

		// BFP - Ki Charge
		if ( ( ucmd->buttons & BUTTON_KI_CHARGE ) && client->kiCharging
		&& client->ps.pm_time > 0 ) { // still delayed
			client->ps.eFlags &= ~EF_AURA;
		}
		if ( client->kiCharging
		&& client->ps.pm_time <= 0 ) { // charge ki!
			client->ps.eFlags |= EF_AURA;
		}
	}

	// BFP - No flight
	pm.noFlight = qfalse;
	if ( g_noFlight.integer > 0 ) {
		pm.noFlight = qtrue;
	}

	// BFP - Melee only
	pm.meleeOnly = qfalse;
	if ( g_meleeOnly.integer > 0 ) {
		pm.meleeOnly = qtrue;
	}

	// BFP - Ultimate tier unlocked timer
	pm.ultimateTierUnlockedTime = client->ultimateTierUnlockedTime;

	// BFP - Ki charge state
	pm.kiCharging = client->kiCharging;

	// BFP - No force gesture anim
#if 0
	if ( ent->flags & FL_FORCE_GESTURE ) {
		ent->flags &= ~FL_FORCE_GESTURE;
		client->pers.cmd.buttons |= BUTTON_GESTURE;
	}
#endif

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

	// BFP - Ki charge state
	client->kiCharging = pm.kiCharging;

	// save results of pmove
	if ( client->ps.eventSequence != oldEventSequence ) {
		ent->eventTime = level.time;
	}
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate( &client->ps, &ent->s, client->ps.commandTime, qtrue );
	}
	else {
		BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
	}
	SendPendingPredictableEvents( &client->ps );

	// BFP - Beam fire hold handling
	if ( !( client->ps.eFlags & EF_FIRING ) ) {
		client->fireHeld = qfalse;		// for grapple
	}

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
	if ( !client->noclip ) {
		G_TouchTriggers( ent );
	}

	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	VectorCopy( client->ps.origin, ent->r.currentOrigin );

	//test for solid areas in the AAS file
	BotTestAAS(ent->r.currentOrigin);

	// touch other objects
	ClientImpacts( ent, &pm );

	// save results of triggers and client events
	if (client->ps.eventSequence != oldEventSequence) {
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
	gclient_t	*cl, *client;

	client = ent->client;

	// if we are doing a chase cam or a remote view, grab the latest info
	if ( client->sess.spectatorState == SPECTATOR_FOLLOW ) {
		int		clientNum = client->sess.spectatorClient;

		// team follow1 and team follow2 go to whatever clients are playing
		if ( clientNum == -1 ) {
			clientNum = level.follow1;
		} else if ( clientNum == -2 ) {
			clientNum = level.follow2;
		}
		if ( (unsigned)clientNum < MAX_CLIENTS ) {
			cl = &level.clients[ clientNum ];
			if ( cl->pers.connected == CON_CONNECTED && cl->sess.sessionTeam != TEAM_SPECTATOR ) {
				int	flags = (cl->ps.eFlags & ~(EF_VOTED | EF_TEAMVOTED)) | (client->ps.eFlags & (EF_VOTED | EF_TEAMVOTED));
				client->ps = cl->ps;
				client->ps.pm_flags |= PMF_FOLLOW;
				client->ps.eFlags = flags;
				return;
			} else {
				// drop them to free spectators unless they are dedicated camera followers
				if ( client->sess.spectatorClient >= 0 ) {
					client->sess.spectatorState = SPECTATOR_FREE;
					ClientBegin( client - level.clients );
				}
			}
		}
	}

	// BFP - PMF_SCOREBOARD is unused
#if 0
	if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
		client->ps.pm_flags |= PMF_SCOREBOARD;
	} else {
		client->ps.pm_flags &= ~PMF_SCOREBOARD;
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

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		SpectatorClientEndFrame( ent );
		return;
	}

	// turn off any expired powerups
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
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


