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
// cg_event.c -- handle entity events at snapshot or playerstate transitions

#include "cg_local.h"


/*
===================
CG_PlaceString

Also called by scoreboard drawing
===================
*/
const char	*CG_PlaceString( int rank ) {
	static char	str[64];
	char	*s, *t;

	if ( rank & RANK_TIED_FLAG ) {
		rank &= ~RANK_TIED_FLAG;
		t = "Tied for ";
	} else {
		t = "";
	}

	if ( rank == 1 ) {
		s = S_COLOR_BLUE "1st" S_COLOR_WHITE;		// draw in blue
	} else if ( rank == 2 ) {
		s = S_COLOR_RED "2nd" S_COLOR_WHITE;		// draw in red
	} else if ( rank == 3 ) {
		s = S_COLOR_YELLOW "3rd" S_COLOR_WHITE;		// draw in yellow
	} else if ( rank == 11 ) {
		s = "11th";
	} else if ( rank == 12 ) {
		s = "12th";
	} else if ( rank == 13 ) {
		s = "13th";
	} else if ( rank % 10 == 1 ) {
		s = va("%ist", rank);
	} else if ( rank % 10 == 2 ) {
		s = va("%ind", rank);
	} else if ( rank % 10 == 3 ) {
		s = va("%ird", rank);
	} else {
		s = va("%ith", rank);
	}

	Com_sprintf( str, sizeof( str ), "%s%s", t, s );
	return str;
}

/*
=============
CG_Obituary
=============
*/
static void CG_Obituary( entityState_t *ent ) {
	int			mod;
	int			target, attacker;
	char		*message;
	char		*message2;
	const char	*targetInfo;
	const char	*attackerInfo;
	char		targetName[32];
	char		attackerName[32];
	// BFP - No gender variable for MOD messages
	// gender_t	gender;

	target = ent->otherEntityNum;
	attacker = ent->otherEntityNum2;
	mod = ent->eventParm;

	if ( target < 0 || target >= MAX_CLIENTS ) {
		CG_Error( "CG_Obituary: target out of range" );
	}

	if ( attacker < 0 || attacker >= MAX_CLIENTS ) {
		attacker = ENTITYNUM_WORLD;
		attackerInfo = NULL;
	} else {
		attackerInfo = CG_ConfigString( CS_PLAYERS + attacker );
	}

	targetInfo = CG_ConfigString( CS_PLAYERS + target );
	if ( !targetInfo ) {
		return;
	}
	Q_strncpyz( targetName, Info_ValueForKey( targetInfo, "n" ), sizeof(targetName) - 2);
	strcat( targetName, S_COLOR_WHITE );

	message2 = "";

	// check for single client messages

	switch( mod ) {
	case MOD_SUICIDE:
		message = "suicides";
		break;
	case MOD_FALLING:
		message = "cratered";
		break;
	case MOD_CRUSH:
		message = "was squished";
		break;
	case MOD_WATER:
		message = "sank like a rock";
		break;
	case MOD_SLIME:
		message = "melted";
		break;
	case MOD_LAVA:
		message = "does a back flip into the lava";
		break;
	case MOD_TARGET_LASER:
		message = "saw the light";
		break;
	case MOD_TRIGGER_HURT:
		message = "was in the wrong place";
		break;
	// BFP - Player being kicked by using an illegal model
	case MOD_ILLEGAL_PLAYER_MODEL:
		message = "got kicked into spectator mode for using an illegal model";
		break;
	default:
		message = NULL;
		break;
	}

	if (attacker == target) {
		// BFP - No gender MOD messages
#if 0
		gender = ci->gender;
		switch (mod) {
		case MOD_GRENADE_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "tripped on her own grenade";
			else if ( gender == GENDER_NEUTER )
				message = "tripped on its own grenade";
			else
				message = "tripped on his own grenade";
			break;
		case MOD_ROCKET_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "blew herself up";
			else if ( gender == GENDER_NEUTER )
				message = "blew itself up";
			else
				message = "blew himself up";
			break;
		case MOD_PLASMA_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "melted herself";
			else if ( gender == GENDER_NEUTER )
				message = "melted itself";
			else
				message = "melted himself";
			break;
		case MOD_BFG_SPLASH:
			message = "should have used a smaller gun";
			break;
		default:
			if ( gender == GENDER_FEMALE )
				message = "killed herself";
			else if ( gender == GENDER_NEUTER )
				message = "killed itself";
			else
				message = "killed himself";
			break;
		}
#endif
		if ( attacker == cg.snap->ps.clientNum ) { // BFP - Add 1 lifedeath in the history
			trap_Cvar_Set( "cg_lifedeaths", va( "%i", (int)( cg_lifedeaths.integer + 1 ) ) );
		}
	}

	if (message) {
		CG_Printf( "%s %s.\n", targetName, message);
		return;
	}

	// check for kill messages from the current clientNum
	if ( attacker == cg.snap->ps.clientNum && attacker != target ) {
		char	*s;

		if ( cgs.gametype < GT_TEAM ) { // BFP - Before Q3: "You fragged %s\n%s place with %i"
			s = va("You sent %s to the next dimension!\n%s place with %i", targetName, 
				CG_PlaceString( cg.snap->ps.persistant[PERS_RANK] + 1 ),
				cg.snap->ps.persistant[PERS_SCORE] );
		} else { // BFP - Before Q3: "You fragged %s"
			s = va("You sent %s to the next dimension!", targetName );
		}
		CG_CenterPrint( s, SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		// print the text message as well
		// BFP - Add 1 lifekill in the history
		trap_Cvar_Set( "cg_lifekills", va( "%i", (int)( cg_lifekills.integer + 1 ) ) );
	}

	// check for double client messages
	if ( !attackerInfo ) {
		attacker = ENTITYNUM_WORLD;
		strcpy( attackerName, "noname" );
	} else {
		Q_strncpyz( attackerName, Info_ValueForKey( attackerInfo, "n" ), sizeof(attackerName) - 2);
		strcat( attackerName, S_COLOR_WHITE );
		// check for kill messages about the current clientNum
		if ( target == cg.snap->ps.clientNum ) {
			Q_strncpyz( cg.killerName, attackerName, sizeof( cg.killerName ) );
		}
	}

	// BFP - Don't admit that the attacker is the same as the target
	if ( attacker != ENTITYNUM_WORLD && attacker != target ) {
		switch (mod) {
		// BFP - No other MOD messages
#if 0
		case MOD_GRAPPLE:
			message = "was caught by";
			break;
		case MOD_GAUNTLET:
			message = "was pummeled by";
			break;
		case MOD_MACHINEGUN:
			message = "was machinegunned by";
			break;
		case MOD_SHOTGUN:
			message = "was gunned down by";
			break;
		case MOD_GRENADE:
			message = "ate";
			message2 = "'s grenade";
			break;
		case MOD_GRENADE_SPLASH:
			message = "was shredded by";
			message2 = "'s shrapnel";
			break;
		case MOD_ROCKET:
			message = "ate";
			message2 = "'s rocket";
			break;
		case MOD_ROCKET_SPLASH:
			message = "almost dodged";
			message2 = "'s rocket";
			break;
		case MOD_PLASMA:
			message = "was melted by";
			message2 = "'s plasmagun";
			break;
		case MOD_PLASMA_SPLASH:
			message = "was melted by";
			message2 = "'s plasmagun";
			break;
		case MOD_RAILGUN:
			message = "was railed by";
			break;
		case MOD_LIGHTNING:
			message = "was electrocuted by";
			break;
		case MOD_BFG:
		case MOD_BFG_SPLASH:
			message = "was blasted by";
			message2 = "'s BFG";
			break;
		case MOD_TELEFRAG:
			message = "tried to invade";
			message2 = "'s personal space";
			break;
#endif
		case MOD_MELEE: // BFP - Melee
			message = "was beaten up by";
			break;
		default: // BFP - Kill with ki attack and telefrag message
			message = "was sent to the next dimension by";
			break;
		}

		if ( target == cg.snap->ps.clientNum ) { // BFP - Add 1 lifedeath in the history
			trap_Cvar_Set( "cg_lifedeaths", va( "%i", (int)( cg_lifedeaths.integer + 1 ) ) );
		}
		if (message) {
			CG_Printf( "%s %s %s%s\n", 
				targetName, message, attackerName, message2);
			return;
		}
	}

	// we don't know what it was
	CG_Printf( "%s died.\n", targetName );
	if ( target == cg.snap->ps.clientNum ) { // BFP - Add 1 lifedeath in the history
		trap_Cvar_Set( "cg_lifedeaths", va( "%i", (int)( cg_lifedeaths.integer + 1 ) ) );
	}
	// BFP - Reset ki trails to avoid viewing other trails
	CG_ResetTrail( KI_TRAIL, target, vec3_origin );
}

//==========================================================================

/*
===============
CG_UseItem
===============
*/
static void CG_UseItem( centity_t *cent ) {
	clientInfo_t *ci;
	int			itemNum, clientNum;
	gitem_t		*item;
	entityState_t *es;

	es = &cent->currentState;
	
	itemNum = (es->event & ~EV_EVENT_BITS) - EV_USE_ITEM0;
	if ( itemNum < 0 || itemNum > HI_NUM_HOLDABLE ) {
		itemNum = 0;
	}

	// print a message if the local player
	if ( es->number == cg.snap->ps.clientNum ) {
		if ( !itemNum ) {
			CG_CenterPrint( "No item to use", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		} else {
			item = BG_FindItemForHoldable( itemNum );
			CG_CenterPrint( va("Use %s", item->pickup_name), SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		}
	}

	switch ( itemNum ) {
	default:
	case HI_NONE:
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.useNothingSound );
		break;

	case HI_TELEPORTER:
		break;

	case HI_MEDKIT:
		clientNum = cent->currentState.clientNum;
		if ( clientNum >= 0 && clientNum < MAX_CLIENTS ) {
			ci = &cgs.clientinfo[ clientNum ];
			ci->medkitUsageTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.medkitSound );
		break;
	}

}

/*
================
CG_ItemPickup

A new item was picked up this frame
================
*/
static void CG_ItemPickup( int itemNum ) {
	cg.itemPickup = itemNum;
	cg.itemPickupTime = cg.time;
	// cg.itemPickupBlendTime = cg.time; // BFP - BFP doesn't use the item pickup effect for the crosshair, now it's reused for when some opponent is being hit
	// see if it should be the grabbed weapon
	// BFP - No weapon item
	/*
	if ( bg_itemlist[itemNum].giType == IT_WEAPON ) {
		// select it immediately
		if ( cg_autoswitch.integer && bg_itemlist[itemNum].giTag != WP_MACHINEGUN ) {
			cg.weaponSelectTime = cg.time;
			cg.weaponSelect = bg_itemlist[itemNum].giTag;
		}
	}
	*/

}


/*
================
CG_PainEvent

Also called by playerstate transition
================
*/
void CG_PainEvent( centity_t *cent, int health ) {
	char	*snd;

	// don't do more than two pain sounds a second
	if ( cg.time - cent->pe.painTime < 500 ) {
		return;
	}

	if ( health < 25 ) {
		snd = "*pain25_1.wav";
	} else if ( health < 50 ) {
		snd = "*pain50_1.wav";
	} else if ( health < 75 ) {
		snd = "*pain75_1.wav";
	} else {
		snd = "*pain100_1.wav";
	}
	trap_S_StartSound( NULL, cent->currentState.number, CHAN_VOICE, 
		CG_CustomSound( cent->currentState.number, snd ) );

	// save pain time for programitic twitch animation
	cent->pe.painTime = cg.time;
	cent->pe.painDirection ^= 1;
}



/*
==============
CG_EntityEvent

An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/
void CG_EntityEvent( centity_t *cent, vec3_t position, int entityNum ) {
	entityState_t	*es;
	entity_event_t	event;
	vec3_t			dir;
	const char		*s;
	int				clientNum;
	clientInfo_t	*ci;
	centity_t		*ce;
	bfpAttackSkinConfig_t	*skinAtkCfg;

	es = &cent->currentState;
	event = es->event & ~EV_EVENT_BITS;

	if ( (unsigned) event >= EV_MAX ) {
		CG_Error( "Unknown event: %i", event );
		return;
	}

	if ( cg_debugEvents.integer ) {
		CG_Printf( "ent:%3i  event:%3i %s\n", es->number, event, eventnames[ event ] );
	}

	if ( !event ) {
		// DEBUGNAME("ZEROEVENT");
		return;
	}

	clientNum = es->clientNum;
	if ( (unsigned) clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}
	ci = &cgs.clientinfo[ clientNum ];
	skinAtkCfg = &ci->skinConfig.attacks[es->weapon];
	// BFP - HIGHLY MODIFIED, every event is sorted for original BFP networking

	switch ( event ) {
	case EV_NONE:					// 0
	case EV_UNUSED_INDEX1:			// 1
	case EV_UNUSED_INDEX2:			// 2
	case EV_UNUSED_INDEX3:			// 3
	case EV_UNUSED_INDEX4:			// 4
	case EV_UNUSED_INDEX5:			// 5
	case EV_UNUSED_INDEX6:			// 6
	case EV_UNUSED_INDEX7:			// 7
	case EV_UNUSED_INDEX8:			// 8
	case EV_UNUSED_INDEX9:			// 9
		break;

	// BFP - Melee
	case EV_MELEE_READY:			// 10
		break;
	case EV_MELEE:					// 11
	{
		int rndMeleeSnd = rand() % 5;
		switch ( rndMeleeSnd ) {
			case 0: {
				trap_S_StartSound (NULL, es->number, CHAN_BODY, CG_CustomSound( es->number, "sound/bfp/melee_hit1.wav" ) );
				break;
			}
			case 1: {
				trap_S_StartSound (NULL, es->number, CHAN_BODY, CG_CustomSound( es->number, "sound/bfp/melee_hit2.wav" ) );
				break;
			}
			case 2: {
				trap_S_StartSound (NULL, es->number, CHAN_BODY, CG_CustomSound( es->number, "sound/bfp/melee_hit3.wav" ) );
				break;
			}
			case 3: {
				trap_S_StartSound (NULL, es->number, CHAN_BODY, CG_CustomSound( es->number, "sound/bfp/melee_hit4.wav" ) );
				break;
			}
			default: {
				trap_S_StartSound (NULL, es->number, CHAN_BODY, CG_CustomSound( es->number, "sound/bfp/melee_hit5.wav" ) );
			}
		}
		break;
	}

	case EV_UNUSED_INDEX12:			// 12
		break;
		
	// BFP - Tier up events
	case EV_TIER_RESET:				// 13
		break;
	case EV_TIER_0:					// 14
	case EV_TIER_1:					// 15
	case EV_TIER_2:					// 16
	case EV_TIER_3:					// 17
	case EV_TIER_4:					// 18
		trap_S_StartSound ( NULL, es->otherEntityNum, CHAN_BODY, cgs.media.tierUpSound );
		if ( event == EV_TIER_4 && es->otherEntityNum == cg.snap->ps.clientNum ) {
			trap_SendConsoleCommand( "transformorbit\n" );
		}
		break;

	// BFP - Short-Range Teleport (Zanzoken)
	case EV_ZANZOKEN_IN:			// 19
		trap_S_StartSound (NULL, es->number, CHAN_BODY, CG_CustomSound( es->number, "sound/bfp/srteleport.wav" ) );
	case EV_ZANZOKEN_OUT:			// 20
		break;
	
	// BFP - Ki boost
	case EV_KI_BOOST:				// 21
		break;

	// BFP - A normal jump sound is played when enables the flight
	case EV_ENABLE_FLIGHT:			// 22
		trap_S_StartSound (NULL, es->number, CHAN_BODY, CG_CustomSound( es->number, "sound/bfp/jump1.wav" ) );
		break;

	//
	// movement generated events
	//
	case EV_FOOTSTEP:				// 23
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ ci->footsteps ][rand()&3] );
		}
		break;

	case EV_FOOTSTEP_METAL:			// 24
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_METAL ][rand()&3] );
		}
		break;

	case EV_FOOTSPLASH:				// 25
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;

	case EV_FOOTWADE:				// 26
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;

	case EV_SWIM:					// 27
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;

	case EV_STEP_4:					// 28
	case EV_STEP_8:					// 29
	case EV_STEP_12:				// 30
									// 31
	case EV_STEP_16:		// smooth out step up transitions
	{
		float	oldStep;
		int		delta;
		int		step;

		if ( clientNum != cg.predictedPlayerState.clientNum ) {
			break;
		}
		// if we are interpolating, we don't need to smooth steps
		if ( cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW) ||
			cg_nopredict.integer || cg_synchronousClients.integer ) {
			break;
		}
		// check for stepping up before a previous step is completed
		delta = cg.time - cg.stepTime;
		if (delta < STEP_TIME) {
			oldStep = cg.stepChange * (STEP_TIME - delta) / STEP_TIME;
		} else {
			oldStep = 0;
		}

		// add this amount
		step = 4 * (event - EV_STEP_4 + 1 );
		cg.stepChange = oldStep + step;
		if ( cg.stepChange > MAX_STEP_CHANGE ) {
			cg.stepChange = MAX_STEP_CHANGE;
		}
		cg.stepTime = cg.time;
		break;
	}

	case EV_FALL_SHORT:				// 32
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.landSound );
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -8;
			cg.landTime = cg.time;
		}
		break;

	case EV_FALL_MEDIUM:			// 33
		// BFP - Use normal land sound instead
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.landSound );
		// use normal pain sound
		// trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*pain100_1.wav" ) );
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -16;
			cg.landTime = cg.time;
		}
		break;

	case EV_FALL_FAR:				// 34
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*fall1.wav" ) );
		cent->pe.painTime = cg.time;	// don't play a pain sound right after this
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -24;
			cg.landTime = cg.time;
		}
		break;

	case EV_JUMP_PAD:				// 35
//		CG_Printf( "EV_JUMP_PAD w/effect #%i\n", es->eventParm );
// BFP - No smoke puff effect when using a jump pad
#if 0
		{
			localEntity_t	*smoke;
			vec3_t			up = {0, 0, 1};


			smoke = CG_SmokePuff( cent->lerpOrigin, up, 
						  32, 
						  1, 1, 1, 0.33f,
						  1000, 
						  cg.time, 0,
						  LEF_PUFF_DONT_SCALE, 
						  cgs.media.smokePuffShader );
		}
#endif

		// boing sound at origin, jump sound on player
		trap_S_StartSound ( cent->lerpOrigin, -1, CHAN_VOICE, cgs.media.jumpPadSound );
		// BFP - Q3 jump sound removed and no break after this case, so continue to BFP jump sound
		break;

	case EV_JUMP:					// 36
	case EV_JUMP_2:					// 37
		// BFP - Use the second jump sound when using ki boost only when it isn't flying
		if ( ( es->eFlags & EF_AURA ) && !( cent->currentState.eFlags & EF_FLIGHT ) ) {
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "sound/bfp/jump2.wav" ) ); // BFP - Ki boost jump sound
		} else {
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "sound/bfp/jump1.wav" ) ); // BFP - Normal jump sound
		}
		break;


	case EV_WATER_TOUCH:			// 38
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrInSound );
		break;

	case EV_WATER_LEAVE:			// 39
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrOutSound );
		break;

	case EV_WATER_UNDER:			// 40
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrUnSound );
		// BFP - Bubble and splash particles when entering under water
		{
			vec3_t end = {0, 0, 1};
			vec3_t splashOrigin;
			float bubbleSize = 2;
			float bubbleRange = 20;
			float debrisSize = 3;
			float velocity = 150;
			float accel = 250;

			VectorCopy( cent->lerpOrigin, splashOrigin );
			splashOrigin[2] += 20; // place a bit above

			// that would be the range
			splashOrigin[0] += (crandom() * 5);
			splashOrigin[1] += (crandom() * 5);

			// BFP - Monster gamemode, player monster particle size and positions
			if ( cgs.gametype == GT_MONSTER
			&& ( cent->currentState.eFlags & EF_MONSTER ) ) {
				bubbleSize = 8;
				bubbleRange = 100;
				debrisSize = 8;
				velocity = 400;
				accel = 700;
			}

			// Splash!
			// BFP - NOTE: These are not debris :P
			CG_ParticleWaterSplash( cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, splashOrigin, end, debrisSize, velocity, accel );
			CG_ParticleWaterSplash( cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, splashOrigin, end, debrisSize, velocity, accel );
			CG_ParticleWaterSplash( cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, splashOrigin, end, debrisSize, velocity, accel );
			CG_ParticleWaterSplash( cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, splashOrigin, end, debrisSize, velocity, accel );
			CG_ParticleWaterSplash( cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, splashOrigin, end, debrisSize, velocity, accel );

			splashOrigin[2] -= 25; // place a bit below
			// BFP - Monster gamemode, player monster particle size and positions
			if ( cgs.gametype == GT_MONSTER
			&& ( cent->currentState.eFlags & EF_MONSTER ) ) {
				splashOrigin[2] -= 100; // place a bit below
			}

			// Blub, blub, blub...
			CG_ParticleBubble( cent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, splashOrigin, end, 700, bubbleRange, bubbleSize );
			CG_ParticleBubble( cent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, splashOrigin, end, 700, bubbleRange, bubbleSize );
			CG_ParticleBubble( cent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, splashOrigin, end, 700, bubbleRange, bubbleSize );
			CG_ParticleBubble( cent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, splashOrigin, end, 700, bubbleRange, bubbleSize );
			CG_ParticleBubble( cent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, splashOrigin, end, 700, bubbleRange, bubbleSize );
		}
		break;

	case EV_WATER_CLEAR:			// 41
		// BFP - No water clear sound
		// trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*gasp.wav" ) );
		break;

	case EV_ITEM_PICKUP:			// 42
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}

			if ( entityNum >= 0 ) {
				// our predicted entity
				ce = cg_entities + entityNum;
				if ( ce->delaySpawn > cg.time && ce->delaySpawnPlayed ) {
					break; // delay item pickup
				}
			} else {
				ce = NULL;
			}

			item = &bg_itemlist[ index ];

			// powerups and team items will have a separate global sound, this one
			// will be played at prediction time
			if ( item->giType == IT_POWERUP || item->giType == IT_TEAM) {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.n_healthSound );
			} else if (item->giType == IT_PERSISTANT_POWERUP) {
			} else {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	trap_S_RegisterSound( item->pickup_sound, qfalse ) );
			}

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index );
			}

			if ( ce ) {
				ce->delaySpawnPlayed = qtrue;
			}
		}
		break;

	case EV_GLOBAL_ITEM_PICKUP:		// 43
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}

			if ( entityNum >= 0 ) {
				// our predicted entity
				ce = cg_entities + entityNum;
				if ( ce->delaySpawn > cg.time && ce->delaySpawnPlayed ) {
					break;
				}
			} else {
				ce = NULL;
			}

			item = &bg_itemlist[ index ];
			// powerup pickups are global
			if( item->pickup_sound ) {
				trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, trap_S_RegisterSound( item->pickup_sound, qfalse ) );
			}

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index );
			}

			if ( ce ) {
				ce->delaySpawnPlayed = qtrue;
			}
		}
		break;

	//
	// weapon events
	//
	case EV_NOAMMO:					// 44
//		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.noAmmoSound );
		if ( es->number == cg.snap->ps.clientNum ) {
			CG_OutOfAmmoChange();
		}
		break;

	case EV_CHANGE_WEAPON:			// 45
		// BFP - Don't play select sound to other players and spectators, only the player itself
		if ( es->number == cg.snap->ps.clientNum && !( cg.snap->ps.pm_flags & PMF_FOLLOW ) ) {
			trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.selectSound );
		}
		break;

	case EV_FIRE_WEAPON:			// 46
		CG_FireWeapon( cent );
		break;

	case EV_USE_ITEM0:				// 47
	case EV_USE_ITEM1:				// 48
	case EV_USE_ITEM2:				// 49
	case EV_USE_ITEM3:				// 50
	case EV_USE_ITEM4:				// 51
	case EV_USE_ITEM5:				// 52
	case EV_USE_ITEM6:				// 53
	case EV_USE_ITEM7:				// 54
	case EV_USE_ITEM8:				// 55
	case EV_USE_ITEM9:				// 56
	case EV_USE_ITEM10:				// 57
	case EV_USE_ITEM11:				// 58
	case EV_USE_ITEM12:				// 59
	case EV_USE_ITEM13:				// 60
	case EV_USE_ITEM14:				// 61
	case EV_USE_ITEM15:				// 62
		CG_UseItem( cent );
		break;

	//=================================================================

	//
	// other events
	//
	case EV_ITEM_RESPAWN:			// 63
		cent->miscTime = cg.time;	// scale up from this
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
		break;

	case EV_ITEM_POP:				// 64
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
		break;

	case EV_PLAYER_TELEPORT_IN:		// 65
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleInSound );
		CG_SpawnEffect( position, cg.snap->ps.clientNum == clientNum && cg_thirdPerson.integer == 0 );
		break;

	case EV_PLAYER_TELEPORT_OUT:	// 66
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleOutSound );
		CG_SpawnEffect(  position, qfalse );
		break;

	case EV_GRENADE_BOUNCE:			// 67
#if 0	/* BFP - No grenade bounce sound */
		if ( rand() & 1 ) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.hgrenb1aSound );
		} else {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.hgrenb2aSound );
		}
#endif
		break;

	case EV_GENERAL_SOUND:			// 68
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, s ) );
		}
		break;
									// 69
	case EV_GLOBAL_SOUND:	// play from the player's head so it never diminishes
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, CG_CustomSound( es->number, s ) );
		}
		break;
									// 70
	case EV_GLOBAL_TEAM_SOUND:	// play from the player's head so it never diminishes
		{
			switch( es->eventParm ) {
				case GTS_RED_CAPTURE: // CTF: red team captured the blue flag, 1FCTF: red team captured the neutral flag
					if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED )
						CG_AddBufferedSound( cgs.media.captureYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.captureOpponentSound );
					break;
				case GTS_BLUE_CAPTURE: // CTF: blue team captured the red flag, 1FCTF: blue team captured the neutral flag
					if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
						CG_AddBufferedSound( cgs.media.captureYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.captureOpponentSound );
					break;
				case GTS_RED_RETURN: // CTF: blue flag returned, 1FCTF: never used
					if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED )
						CG_AddBufferedSound( cgs.media.returnYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.returnOpponentSound );
					//
					CG_AddBufferedSound( cgs.media.blueFlagReturnedSound );
					break;
				case GTS_BLUE_RETURN: // CTF red flag returned, 1FCTF: neutral flag returned
					if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
						CG_AddBufferedSound( cgs.media.returnYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.returnOpponentSound );
					//
					CG_AddBufferedSound( cgs.media.redFlagReturnedSound );
					break;

				case GTS_RED_TAKEN: // CTF: red team took blue flag, 1FCTF: blue team took the neutral flag
					// if this player picked up the flag then a sound is played in CG_CheckLocalSounds
					if ( cg.snap->ps.powerups[PW_BLUEFLAG] ) {
					}
					else {
						if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE) {
						 	CG_AddBufferedSound( cgs.media.enemyTookYourFlagSound );
						}
						else if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED) {
 							CG_AddBufferedSound( cgs.media.yourTeamTookEnemyFlagSound );
						}
					}
					break;
				case GTS_BLUE_TAKEN: // CTF: blue team took the red flag, 1FCTF red team took the neutral flag
					// if this player picked up the flag then a sound is played in CG_CheckLocalSounds
					if ( cg.snap->ps.powerups[PW_REDFLAG] ) {
					} else {
						if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED) {
							CG_AddBufferedSound( cgs.media.enemyTookYourFlagSound );
						}
						else if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE) {
							CG_AddBufferedSound( cgs.media.yourTeamTookEnemyFlagSound );
						}
					}
					break;

				case GTS_REDTEAM_SCORED:
					CG_AddBufferedSound(cgs.media.redScoredSound);
					break;
				case GTS_BLUETEAM_SCORED:
					CG_AddBufferedSound(cgs.media.blueScoredSound);
					break;
				case GTS_REDTEAM_TOOK_LEAD:
					CG_AddBufferedSound(cgs.media.redLeadsSound);
					break;
				case GTS_BLUETEAM_TOOK_LEAD:
					CG_AddBufferedSound(cgs.media.blueLeadsSound);
					break;
				case GTS_TEAMS_ARE_TIED:
					CG_AddBufferedSound( cgs.media.teamsTiedSound );
					break;
				default:
					break;
			}
			break;
		}

	case EV_BULLET_HIT_FLESH:		// 71
		break;

	case EV_BULLET_HIT_WALL:		// 72
		break;

	//
	// missile impacts
	//
	case EV_MISSILE_HIT:			// 73
		ByteToDir( es->eventParm, dir );
		CG_MissileHitPlayer( es->weapon, position, dir, es->otherEntityNum, skinAtkCfg, cent );
		CG_ResetTrail( BEAM_TRAIL, es->number, es->origin ); // BFP - Reset beam trail
		break;

	case EV_MISSILE_MISS:			// 74
		ByteToDir( es->eventParm, dir );
		CG_MissileHitWall( es->weapon, es->otherEntityNum, position, dir, IMPACTSOUND_DEFAULT, skinAtkCfg, cent );
		// BFP - Debris particles explosion
		CG_DebrisExplosion( position, dir, skinAtkCfg );
		// BFP - Spark particles explosion
		CG_SparksExplosion( position, dir, skinAtkCfg );
		CG_ResetTrail( BEAM_TRAIL, es->number, es->origin ); // BFP - Reset beam trail
		break;

	case EV_MISSILE_MISS_METAL:		// 75
		ByteToDir( es->eventParm, dir );
		CG_MissileHitWall( es->weapon, es->otherEntityNum, position, dir, IMPACTSOUND_METAL, skinAtkCfg, cent );
		// BFP - Debris particles explosion
		CG_DebrisExplosion( position, dir, skinAtkCfg );
		// BFP - Spark particles explosion
		CG_SparksExplosion( position, dir, skinAtkCfg );
		CG_ResetTrail( BEAM_TRAIL, es->number, es->origin ); // BFP - Reset beam trail
		break;

	// BFP - EV_MISSILE_DETONATE - used on ki grenade bounces and beams, 
	// that happens when projectiles/beams reaches their lifetime limit or are stopped by the player actions
	// no debris and sparks particles here
	case EV_MISSILE_DETONATE:		// 76
		{
			vec3_t	dirDetonate = {0, 0, 1}; // place the explosion position and size correctly
			CG_MissileHitWall( es->weapon, es->otherEntityNum, position, dirDetonate, IMPACTSOUND_DEFAULT, skinAtkCfg, cent );
			CG_ResetTrail( BEAM_TRAIL, es->number, es->origin ); // BFP - Reset beam trail
		}
		break;

	case EV_RAILTRAIL:				// 77
		// if the end was on a nomark surface, don't make an explosion
		CG_RailTrail( ci, es->origin2, es->pos.trBase );
		ByteToDir( es->eventParm, dir );
		break;

	case EV_SHOTGUN:				// 78
		// BFP - No shotgun fire, just force field test
		// CG_ShotgunFire( es );
		break;
	
	case EV_UNUSED_INDEX79:			// 79
		break;

	case EV_PAIN:					// 80
		// local player sounds are triggered in CG_CheckLocalSounds,
		// so ignore events on the player
		if ( cent->currentState.number != cg.snap->ps.clientNum ) {
			CG_PainEvent( cent, es->eventParm );
		}
		break;

	case EV_DEATH1:					// 81
	case EV_DEATH2:					// 82
	case EV_DEATH3:					// 83
		trap_S_StartSound( NULL, es->number, CHAN_VOICE, 
				CG_CustomSound( es->number, va("*death%i.wav", event - EV_DEATH1 + 1) ) );
		break;

	case EV_OBITUARY:				// 84
		CG_Obituary( es );
		break;

	//
	// powerup events
	//
	case EV_POWERUP_QUAD:			// 85
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_QUAD;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.quadSound );
		break;

	case EV_POWERUP_BATTLESUIT:		// 86
	// BFP - No battlesuit powerup
#if 0
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_BATTLESUIT;
			cg.powerupTime = cg.time;
		}
#endif
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.protectSound );
		break;
	
	case EV_UNUSED_INDEX87:			// 87
		break;

	case EV_GIB_PLAYER:				// 88
		trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
		CG_GibPlayer( cent->lerpOrigin );
		break;

	case EV_UNUSED_INDEX89:			// 89
	case EV_UNUSED_INDEX90:			// 90
		break;

	case EV_SCOREPLUM:				// 91
		CG_ScorePlum( cent->currentState.otherEntityNum, cent->lerpOrigin, cent->currentState.time );
		break;

	case EV_TAUNT:					// 92
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*taunt.wav" ) );
		break;

	case EV_UNUSED_INDEX93:			// 93
	case EV_UNUSED_INDEX94:			// 94
	case EV_UNUSED_INDEX95:			// 95
		break;

	// BFP - Blind
	case EV_BLINDING:				// 96
		if ( es->number == cg.snap->ps.clientNum 
		&& ( !cg.blindLastAttackTime || cg.time - cg.blindLastAttackTime > 4000 ) ) {
			cg.blind = qtrue;
			cg.blindStartTime = cg.time;
			cg.blindLastAttackTime = cg.time;
		}
		break;

	case EV_DEBUG_LINE:				// 97
		CG_Beam( cent );
		break;

	case EV_STOPLOOPINGSOUND:		// 98
		trap_S_StopLoopingSound( es->number );
		es->loopSound = 0;
		break;

	// BFP - Spark and beam struggle effect
	case EV_SPARK:					// 99
		ByteToDir( es->eventParm, dir );
		// BFP - Beam struggle effect
		CG_BeamStruggleEffect( position, dir );
		break;

	default:
		CG_Error( "Unknown event: %i", event );
		break;
	}
}


/*
==============
CG_CheckEvents

==============
*/
void CG_CheckEvents( centity_t *cent ) {
	// check for event-only entities
	if ( cent->currentState.eType > ET_EVENTS ) {
		if ( cent->previousEvent ) {
			return;	// already fired
		}
		// if this is a player event set the entity number of the client entity number
		if ( cent->currentState.eFlags & EF_PLAYER_EVENT ) {
			cent->currentState.number = cent->currentState.otherEntityNum;
		}

		cent->previousEvent = 1;

		cent->currentState.event = cent->currentState.eType - ET_EVENTS;
	} else {
		// check for events riding with another entity
		if ( cent->currentState.event == cent->previousEvent ) {
			return;
		}
		cent->previousEvent = cent->currentState.event;
		if ( ( cent->currentState.event & ~EV_EVENT_BITS ) == 0 ) {
			return;
		}
	}

	// calculate the position at exactly the frame time
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, cent->lerpOrigin );
	CG_SetEntitySoundPosition( cent );

	CG_EntityEvent( cent, cent->lerpOrigin, -1 );
}

