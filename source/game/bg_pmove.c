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
// bg_pmove.c -- both games player movement code
// takes a playerstate and a usercmd as input and returns a modifed playerstate

#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

pmove_t		*pm;
pml_t		pml;

// movement parameters
float	pm_stopspeed = 100.0f;
float	pm_duckScale = 0.25f;
float	pm_swimScale = 0.50f;

float	pm_accelerate = 10.0f;
float	pm_airaccelerate = 1.0f;
float	pm_wateraccelerate = 4.0f;
float	pm_flyaccelerate = 2.0f; // BFP - Add less flight acceleration, before 8.0f

float	pm_friction = 6.0f;
float	pm_waterfriction = 1.0f;
float	pm_flightfriction = 2.0f; // BFP - Add less flight friction, before 3.0f
float	pm_spectatorfriction = 5.0f;

int		c_pmove = 0;

// BFP - Macro for torso handling when using ki attacks, since the code looked repetitive, so this macro makes the code a bit shorter
/* BFP - TODO: When implementing ki attacks, look up about the properties of the ki attacks from cfg and correct animation changes if required
And tweak pmove_t struct, so we can handle that on g_active.c (like meleeHit), adding:
attackType ("beam", "hitscan", "missile", "rdmissile", "sbeam" or "forcefield"), // type of attack
randomWeaponTime (int, number of miliseconds), // random weapon time, maybe the max msec range of the random value
chargeAttack (int / qboolean), // charging yes or no
chargeAutoFire (int / qboolean), // even if it's charging the ki attack, fire
minCharge (int [0-6]), // min charge points
maxCharge (int [0-6]), // max charge points
loopAnim (int / qboolean), // Maybe it's: use PM_ContinueTorsoAnim, if not: PM_StartTorsoAnim
noAttackAnim (int / qboolean), // no animation strike yes or no
priority (int [0-2]), // if 2, it'll act like a overpowered forcefield, if 1 like a beam, if 0 nothing
movementPenalty (int, number of seconds) // enters WEAPON_STUN when the ki attack was being used
-----
The following sample testing torso ki attack animations are used with:
(ultimate_blast)	WP_BFG would be					"beam", chargeAttack 1, chargeAutoFire 0, loopingAnim 0, noAttackAnim 0, priority 1, movementPenalty 0
(finger_blast)		WP_MACHINEGUN would be			"hitscan", chargeAttack 0, chargeAutoFire 0, loopingAnim 1, noAttackAnim 0, priority 0, movementPenalty 0
(ki_blast)			WP_ROCKET_LAUNCHER would be		"missile", chargeAttack 0, chargeAutoFire 0, loopingAnim 0, noAttackAnim 0, priority 0, movementPenalty 0
(super_homing)		WP_GRENADE_LAUNCHER would be	"missile", chargeAttack 1, chargeAutoFire 0, loopingAnim 1, noAttackAnim 0, priority 0, movementPenalty 0
(finger_beam)		WP_RAILGUN would be				"hitscan", chargeAttack 0, chargeAutoFire 0, loopingAnim 0, noAttackAnim 0, priority 0, movementPenalty 0
(eyebeam)			WP_LIGHTNING would be			"hitscan", chargeAttack 0, chargeAutoFire 0, loopingAnim 0, noAttackAnim 0, priority 0, movementPenalty 0
(homing_special)	WP_PLASMAGUN would be			"rdmissile", chargeAttack 1, chargeAutoFire 0, loopingAnim 0, noAttackAnim 0, priority 0, movementPenalty 0
(aga)				WP_SHOTGUN would be				"forcefield", chargeAttack 1, chargeAutoFire 1, loopingAnim 1, noAttackAnim 0, priority 2, movementPenalty 2
(blinding_flash)	would be						"forcefield", chargeAttack 1, chargeAutoFire 0, loopingAnim 0, noAttackAnim 1, priority 0, movementPenalty 0

About "sbeam" attackType would be like a beam that, by holding down the attack key, 
you direct it wherever you want by moving the cursor. 
If you stop pressing the attack key, it explodes to the point where it arrived.
This attackType was originally left unfinished, 
so there's a bug: after colliding the beam into something solid and 
keep holding down the attack key, keeps muzzling and 
doesn't shoot anything while the ki is wasted out of control. 
*/
#define KI_ATTACK_TORSO_ANIM_HANDLING() \
	if ( ( pm->cmd.buttons & BUTTON_ATTACK ) && !( pm->ps->pm_flags & PMF_KI_ATTACK ) ) { \
		switch( pm->ps->weapon ) { \
		case WP_ROCKET_LAUNCHER: { PM_StartTorsoAnim( TORSO_ATTACK1_PREPARE ); break; } \
		case WP_GRENADE_LAUNCHER: { PM_ContinueTorsoAnim( TORSO_ATTACK2_PREPARE ); break; } \
		case WP_RAILGUN: { PM_StartTorsoAnim( TORSO_ATTACK3_PREPARE ); break; } \
		case WP_PLASMAGUN: { PM_ContinueTorsoAnim( TORSO_ATTACK3_PREPARE ); break; } \
		case WP_SHOTGUN: \
		case WP_BFG: { PM_ContinueTorsoAnim( TORSO_ATTACK4_PREPARE ); break; } \
		} \
	} else if ( pm->ps->pm_flags & PMF_KI_ATTACK ) { \
		pm->cmd.buttons &= ~BUTTON_GESTURE; \
		switch( pm->ps->weapon ) { \
		default: \
		case WP_MACHINEGUN: { PM_ContinueTorsoAnim( TORSO_ATTACK0_STRIKE ); break; } \
		case WP_ROCKET_LAUNCHER: { PM_ContinueTorsoAnim( TORSO_ATTACK1_STRIKE ); break; } \
		case WP_GRENADE_LAUNCHER: { PM_ContinueTorsoAnim( TORSO_ATTACK2_STRIKE ); break; } \
		case WP_PLASMAGUN: \
		case WP_RAILGUN: { PM_ContinueTorsoAnim( TORSO_ATTACK3_STRIKE ); break; } \
		case WP_SHOTGUN: \
		case WP_BFG: { PM_ContinueTorsoAnim( TORSO_ATTACK4_STRIKE ); break; } \
		} \
	}

// BFP - Macro for torso handling, since the code looked repetitive, so this macro makes the code a bit shorter
#define TORSOSTATUS_ANIM_HANDLING(other_torsostatus) \
	if ( pm->ps->pm_flags & PMF_BLOCK ) PM_ContinueTorsoAnim( TORSO_BLOCK ); \
	else if ( ( pm->cmd.buttons & BUTTON_MELEE ) && !( pm->ps->pm_flags & PMF_MELEE ) ) PM_ContinueTorsoAnim( TORSO_MELEE_READY ); \
	else if ( pm->ps->pm_flags & PMF_MELEE ) PM_ContinueTorsoAnim( TORSO_MELEE_STRIKE ); \
	else KI_ATTACK_TORSO_ANIM_HANDLING() \
	else PM_ContinueTorsoAnim( other_torsostatus )

// BFP - Macro for jump handling, since the code looked repetitive, so this macro makes the code a bit shorter
#define FORCEJUMP_ANIM_HANDLING() ( pm->cmd.forwardmove >= 0 ) ? PM_ForceLegsAnim( LEGS_JUMP ) : PM_ForceLegsAnim( LEGS_JUMPB )

// BFP - Macro for fly handling, since the code looked repetitive, so this macro makes the code a bit shorter
#define CONTINUEFLY_ANIM_HANDLING() \
	if ( pm->cmd.forwardmove > 0 ) { TORSOSTATUS_ANIM_HANDLING( TORSO_FLYA ); PM_ContinueLegsAnim( LEGS_FLYA ); } \
	else if ( pm->cmd.forwardmove < 0 ) { TORSOSTATUS_ANIM_HANDLING( TORSO_FLYB ); PM_ContinueLegsAnim( LEGS_FLYB ); } \
	else { TORSOSTATUS_ANIM_HANDLING( TORSO_STAND ); PM_ContinueLegsAnim( LEGS_FLYIDLE ); }

// BFP - Macro for melee strike handling, since the code looked repetitive, so this macro makes the code a bit shorter
#define CONTINUEMELEESTRIKE_LEGS_ANIM_HANDLING(condition) \
	/* Keep moving the legs when the player is attacking to the target through melee. If the condition variable isn't used leave using this value: 1 */ \
	if ( ( condition ) && ( pm->ps->pm_flags & PMF_MELEE ) \
	&& !( pm->ps->pm_flags & PMF_HITSTUN ) && !( pm->ps->pm_flags & PMF_KI_CHARGE ) ) { PM_ContinueLegsAnim( LEGS_MELEE_STRIKE ); }

// BFP - Macro for movement handling in the slopes and when being near to the ground, since the code looked repetitive, so this macro makes the code a bit shorter
#define SLOPES_NEARGROUND_ANIM_HANDLING(is_slope) \
	if (is_slope) { \
		if ( pm->ps->pm_flags & PMF_DUCKED ) { \
			PM_ContinueLegsAnim( LEGS_IDLECR ); \
			if ( pm->cmd.forwardmove < 0 \
			|| ( pm->cmd.forwardmove > 0 \
			|| ( pm->cmd.forwardmove == 0 && pm->cmd.rightmove ) ) ) { PM_ContinueLegsAnim( LEGS_WALKCR ); } \
			TORSOSTATUS_ANIM_HANDLING( TORSO_STAND ); \
			return; \
		} \
	} else { \
		/* If it's trying to crouch, then play the jump animation once */ \
		if ( pm->ps->pm_flags & PMF_DUCKED ) { \
			pm->ps->pm_flags |= PMF_NEARGROUND; \
			FORCEJUMP_ANIM_HANDLING(); \
			return; \
		} \
	} \
	/* If it's very near to the other entity and the melee strike is executed, continue playing the melee strike legs animation */ \
	if ( !pm->cmd.forwardmove && !pm->cmd.rightmove ) { PM_ContinueLegsAnim( LEGS_IDLE ); CONTINUEMELEESTRIKE_LEGS_ANIM_HANDLING( 1 ) return; } \
	if ( !( pm->cmd.buttons & BUTTON_WALKING ) ) { \
		if ( pm->cmd.forwardmove < 0 ) { PM_ContinueLegsAnim( LEGS_BACK ); TORSOSTATUS_ANIM_HANDLING( TORSO_STAND ); } \
		else if ( pm->cmd.forwardmove > 0 \
		|| ( pm->cmd.forwardmove == 0 && pm->cmd.rightmove ) ) { PM_ContinueLegsAnim( LEGS_RUN ); TORSOSTATUS_ANIM_HANDLING( TORSO_RUN ); } \
	} else { \
		if ( pm->cmd.forwardmove < 0 ) { PM_ContinueLegsAnim( LEGS_BACK ); } \
		else if ( pm->cmd.forwardmove > 0 \
		|| ( pm->cmd.forwardmove == 0 && pm->cmd.rightmove ) ) { PM_ContinueLegsAnim( LEGS_WALK ); } \
		TORSOSTATUS_ANIM_HANDLING( TORSO_STAND ); \
	} \
	CONTINUEMELEESTRIKE_LEGS_ANIM_HANDLING( 1 )

/*
===============
PM_AddEvent

===============
*/
void PM_AddEvent( int newEvent ) {
	BG_AddPredictableEventToPlayerstate( newEvent, 0, pm->ps );
}

/*
===============
PM_AddTouchEnt
===============
*/
void PM_AddTouchEnt( int entityNum ) {
	int		i;

	if ( entityNum == ENTITYNUM_WORLD ) {
		return;
	}
	if ( pm->numtouch == MAXTOUCH ) {
		return;
	}

	// see if it is already added
	for ( i = 0 ; i < pm->numtouch ; i++ ) {
		if ( pm->touchents[ i ] == entityNum ) {
			return;
		}
	}

	// add it
	pm->touchents[pm->numtouch] = entityNum;
	pm->numtouch++;
}

/*
===================
PM_StartTorsoAnim
===================
*/
static void PM_StartTorsoAnim( int anim ) {
	if ( pm->ps->pm_type >= PM_DEAD ) {
		return;
	}
	pm->ps->torsoAnim = ( ( pm->ps->torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )
		| anim;
}
static void PM_StartLegsAnim( int anim ) {
	if ( pm->ps->pm_type >= PM_DEAD ) {
		return;
	}
	if ( pm->ps->legsTimer > 0 ) {
		return;		// a high priority animation is running
	}
	pm->ps->legsAnim = ( ( pm->ps->legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )
		| anim;
}

static void PM_ContinueLegsAnim( int anim ) {
	if ( ( pm->ps->legsAnim & ~ANIM_TOGGLEBIT ) == anim ) {
		return;
	}
	if ( pm->ps->legsTimer > 0 ) {
		return;		// a high priority animation is running
	}
	PM_StartLegsAnim( anim );
}

static void PM_ContinueTorsoAnim( int anim ) {
	if ( ( pm->ps->torsoAnim & ~ANIM_TOGGLEBIT ) == anim ) {
		return;
	}
	if ( pm->ps->torsoTimer > 0 ) {
		return;		// a high priority animation is running
	}
	PM_StartTorsoAnim( anim );
}

static void PM_ForceLegsAnim( int anim ) {
	pm->ps->legsTimer = 0;
	PM_StartLegsAnim( anim );
}


/*
==================
PM_ClipVelocity

Slide off of the impacting surface
==================
*/
void PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce ) {
	float	backoff;
	float	change;
	int		i;
	
	backoff = DotProduct (in, normal);
	
	if ( backoff < 0 ) {
		backoff *= overbounce;
	} else {
		backoff /= overbounce;
	}

	for ( i=0 ; i<3 ; i++ ) {
		change = normal[i]*backoff;
		out[i] = in[i] - change;
	}
}


/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
static void PM_Friction( void ) {
	vec3_t	vec;
	float	*vel;
	float	speed, newspeed, control;
	float	drop;
	
	vel = pm->ps->velocity;
	
	VectorCopy( vel, vec );
	if ( pml.walking ) {
		vec[2] = 0;	// ignore slope movement
	}

	speed = VectorLength(vec);
	if (speed < 1) {
		vel[0] = 0;
		vel[1] = 0;		// allow sinking underwater
		// FIXME: still have z friction underwater?
		return;
	}

	drop = 0;

	// apply ground friction
	if ( pm->waterlevel <= 1 ) {
		if ( pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK) ) {

			// BFP - No handling PMF_TIME_KNOCKBACK
			// if getting knocked back, no friction
			// if ( ! (pm->ps->pm_flags & PMF_TIME_KNOCKBACK) ) {
				control = speed < pm_stopspeed ? pm_stopspeed : speed;
				drop += control*pm_friction*pml.frametime;
			// }
		}
	}

	// apply water friction even if just wading
	if ( pm->waterlevel ) {
		drop += speed*pm_waterfriction*pm->waterlevel*pml.frametime;
	}

	// apply flying friction
	// BFP - Flight
	if ( pm->ps->powerups[PW_FLIGHT] > 0 ) {
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control*pm_flightfriction*pml.frametime;
	}

	if ( pm->ps->pm_type == PM_SPECTATOR) {
		drop += speed*pm_spectatorfriction*pml.frametime;
	}

	// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0) {
		newspeed = 0;
	}
	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}


/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
static void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel ) {
#if 1
	// q2 style
	int			i;
	float		addspeed, accelspeed, currentspeed;

	currentspeed = DotProduct (pm->ps->velocity, wishdir);
	addspeed = wishspeed - currentspeed;
	if (addspeed <= 0) {
		return;
	}
	accelspeed = accel*pml.frametime*wishspeed;
	if (accelspeed > addspeed) {
		accelspeed = addspeed;
	}
	
	for (i=0 ; i<3 ; i++) {
		pm->ps->velocity[i] += accelspeed*wishdir[i];	
	}
#else
	// proper way (avoids strafe jump maxspeed bug), but feels bad
	vec3_t		wishVelocity;
	vec3_t		pushDir;
	float		pushLen;
	float		canPush;

	VectorScale( wishdir, wishspeed, wishVelocity );
	VectorSubtract( wishVelocity, pm->ps->velocity, pushDir );
	pushLen = VectorNormalize( pushDir );

	canPush = accel*pml.frametime*wishspeed;
	if (canPush > pushLen) {
		canPush = pushLen;
	}

	VectorMA( pm->ps->velocity, canPush, pushDir, pm->ps->velocity );
#endif
}


/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
static float PM_CmdScale( usercmd_t *cmd ) {
	int		max;
	float	total;
	float	scale;

	max = abs( cmd->forwardmove );
	if ( abs( cmd->rightmove ) > max ) {
		max = abs( cmd->rightmove );
	}
	if ( abs( cmd->upmove ) > max ) {
		max = abs( cmd->upmove );
	}
	if ( !max ) {
		return 0;
	}

	total = sqrt( cmd->forwardmove * cmd->forwardmove
		+ cmd->rightmove * cmd->rightmove + cmd->upmove * cmd->upmove );
	scale = (float)pm->ps->speed * max / ( 127.0 * total );

	return scale;
}


/*
================
PM_SetMovementDir

Determine the rotation of the legs reletive
to the facing dir
================
*/
static void PM_SetMovementDir( void ) {
	if ( pm->cmd.forwardmove || pm->cmd.rightmove ) {
		if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 0;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 1;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove == 0 ) {
			pm->ps->movementDir = 2;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 3;
		} else if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 4;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 5;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove == 0 ) {
			pm->ps->movementDir = 6;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 7;
		}
	} else {
		// if they aren't actively going directly sideways,
		// change the animation to the diagonal so they
		// don't stop too crooked
		if ( pm->ps->movementDir == 2 ) {
			pm->ps->movementDir = 1;
		} else if ( pm->ps->movementDir == 6 ) {
			pm->ps->movementDir = 7;
		} 
	}
}


/*
=============
PM_CheckJump
=============
*/
static qboolean PM_CheckJump( void ) {
	if ( pm->ps->pm_flags & PMF_RESPAWNED ) {
		return qfalse;		// don't allow jump until all buttons are up
	}

	if ( pm->cmd.upmove < 10 ) {
		// not holding jump
		return qfalse;
	}

	// must wait for jump to be released
	if ( pm->ps->pm_flags & PMF_JUMP_HELD ) {
		// clear upmove so cmdscale doesn't lower running speed
		pm->cmd.upmove = 0;
		return qfalse;
	}

	pml.groundPlane = qfalse;		// jumping away
	pml.walking = qfalse;
	pm->ps->pm_flags |= PMF_JUMP_HELD;
	pm->ps->pm_flags |= PMF_NEARGROUND; // BFP - Handle PMF_NEARGROUND, avoid checking if there's ground at that point

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pm->ps->velocity[2] = JUMP_VELOCITY;
	// BFP - Double jump velocity when using ki boost
	if ( pm->ps->powerups[PW_HASTE] > 0 ) {
		pm->ps->velocity[2] *= 2;
	}

	PM_AddEvent( EV_JUMP );

	// BFP - No PMF_BACKWARDS_JUMP handling (code removed)
	FORCEJUMP_ANIM_HANDLING();

	TORSOSTATUS_ANIM_HANDLING( TORSO_STAND );

	return qtrue;
}

/*
=============
PM_CheckWaterJump
=============
*/
static qboolean	PM_CheckWaterJump( void ) {
	// BFP - Apply for backwards too, Q3 doesn't have that
	vec3_t	spot, backwardSpot;
	int		cont;
	vec3_t	flatforward, flatbackward;
#define WATER_JUMP_HORIZONTAL_VELOCITY		200
#define WATER_JUMP_VERTICAL_VELOCITY		250

	if (pm->ps->pm_time) {
		return qfalse;
	}

	// check for water jump
	if ( pm->waterlevel != 2 ) {
		return qfalse;
	}

	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;
	VectorNormalize (flatforward);

	// backward direction
	flatbackward[0] = -pml.forward[0];
	flatbackward[1] = -pml.forward[1];
	flatbackward[2] = 0;
	VectorNormalize( flatbackward );

	// check forward
	VectorMA ( pm->ps->origin, 30, flatforward, spot );
	spot[2] += 4;
	cont = pm->pointcontents( spot, pm->ps->clientNum );
	if ( cont & CONTENTS_SOLID ) {
		spot[2] += 16;
		cont = pm->pointcontents( spot, pm->ps->clientNum );
		if ( !cont ) {
			VectorScale( pml.forward, WATER_JUMP_HORIZONTAL_VELOCITY, pm->ps->velocity );
			pm->ps->velocity[2] = WATER_JUMP_VERTICAL_VELOCITY;
			return qtrue;
		}
	}

	// check backward
	VectorMA( pm->ps->origin, 30, flatbackward, spot );
	spot[2] += 4;
	cont = pm->pointcontents( spot, pm->ps->clientNum );
	if ( cont & CONTENTS_SOLID ) {
		spot[2] += 16;
		cont = pm->pointcontents( spot, pm->ps->clientNum );
		if ( !cont ) {
			VectorScale( pml.forward, -WATER_JUMP_HORIZONTAL_VELOCITY, pm->ps->velocity );
			pm->ps->velocity[2] = WATER_JUMP_VERTICAL_VELOCITY;
			return qtrue;
		}
	}

	return qfalse;
}
#undef WATER_JUMP_HORIZONTAL_VELOCITY
#undef WATER_JUMP_VERTICAL_VELOCITY

//============================================================================


/*
===================
PM_WaterJumpMove

Flying out of the water
===================
*/
static void PM_WaterJumpMove( void ) {
	// waterjump has no control, but falls

	PM_StepSlideMove( qtrue );

	pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
	// BFP - No handling PMF_ALL_TIMES
#if 0
	if (pm->ps->velocity[2] < 0) {
		// cancel as soon as we are falling down again
		pm->ps->pm_flags &= ~PMF_ALL_TIMES;
		pm->ps->pm_time = 0;
	}
#endif
}

/*
===================
PM_WaterMove

===================
*/
static void PM_WaterMove( void ) {
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;
	float	scale;
	float	vel;

	// BFP - Avoid adding friction in the water while charging and flying
	if ( ( ( pm->ps->pm_flags & PMF_KI_CHARGE )
	|| ( pm->cmd.buttons & BUTTON_KI_CHARGE ) ) 
	&& pm->ps->powerups[PW_FLIGHT] > 0 ) {
		return;
	}

	if ( PM_CheckWaterJump() ) {
		PM_WaterJumpMove();
		return;
	}
#if 0
	// jump = head for surface
	if ( pm->cmd.upmove >= 10 ) {
		if (pm->ps->velocity[2] > -300) {
			if ( pm->watertype == CONTENTS_WATER ) {
				pm->ps->velocity[2] = 100;
			} else if (pm->watertype == CONTENTS_SLIME) {
				pm->ps->velocity[2] = 80;
			} else {
				pm->ps->velocity[2] = 50;
			}
		}
	}
#endif
	PM_Friction ();

	scale = PM_CmdScale( &pm->cmd );
	//
	// user intentions
	//
	if ( !scale ) {
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = -60;		// sink towards bottom
	} else {
		for (i=0 ; i<3 ; i++)
			wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;

		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	if ( wishspeed > pm->ps->speed * pm_swimScale ) {
		wishspeed = pm->ps->speed * pm_swimScale;
	}

	PM_Accelerate (wishdir, wishspeed, pm_wateraccelerate);

	// make sure we can go up slopes easily under water
	if ( pml.groundPlane && DotProduct( pm->ps->velocity, pml.groundTrace.plane.normal ) < 0 ) {
		vel = VectorLength(pm->ps->velocity);
		// slide along the ground plane
		PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal, 
			pm->ps->velocity, OVERCLIP );

		VectorNormalize(pm->ps->velocity);
		VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
	}

	// BFP - Water animation handling, uses flying animation in that case
	CONTINUEFLY_ANIM_HANDLING()

	// BFP - Melee strike legs animation
	CONTINUEMELEESTRIKE_LEGS_ANIM_HANDLING( 1 )

	PM_SlideMove( qfalse );
}

/*
===================
PM_FlyMove

Only with the flight powerup
===================
*/
static void PM_FlyMove( void ) {
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;
	float	scale;

	// normal slowdown
	PM_Friction ();

	scale = PM_CmdScale( &pm->cmd );
	//
	// user intentions
	//
	if ( !scale ) {
		VectorClear( wishvel );
	} else {
		for ( i = 0; i < 3; i++ ) {
			wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove + scale * pml.up[i]*pm->cmd.upmove;
		}
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	if ( !( pm->ps->pm_flags & PMF_BLOCK ) // BFP - Don't increase the speed when blocking
	&& ( pm->ps->powerups[PW_HASTE] > 0 || ( pm->cmd.buttons & BUTTON_KI_USE ) ) ) {
		wishspeed *= scale;
	}

	PM_Accelerate (wishdir, wishspeed, pm_flyaccelerate);

	PM_StepSlideMove( qfalse );
}


/*
===================
PM_AirMove

===================
*/
static void PM_AirMove( void ) {
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;
	usercmd_t	cmd;

	PM_Friction();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	cmd = pm->cmd;
	scale = PM_CmdScale( &cmd );

	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;
	VectorNormalize (pml.forward);
	VectorNormalize (pml.right);

	for ( i = 0 ; i < 2 ; i++ ) {
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	}
	wishvel[2] = 0;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	// not on ground, so little effect on velocity
	PM_Accelerate (wishdir, wishspeed, pm_airaccelerate);

	// we may have a ground plane that is very steep, even
	// though we don't have a groundentity
	// slide along the steep plane
	if ( pml.groundPlane ) {
		PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal, 
			pm->ps->velocity, OVERCLIP );
	}

#if 0
	//ZOID:  If we are on the grapple, try stair-stepping
	//this allows a player to use the grapple to pull himself
	//over a ledge
	if (pm->ps->pm_flags & PMF_GRAPPLE_PULL)
		PM_StepSlideMove ( qtrue );
	else
		PM_SlideMove ( qtrue );
#endif
	TORSOSTATUS_ANIM_HANDLING( TORSO_STAND );

	PM_StepSlideMove ( qtrue );
}

// BFP - no hook
#if 0
/*
===================
PM_GrappleMove

===================
*/
static void PM_GrappleMove( void ) {
	vec3_t vel, v;
	float vlen;

	VectorScale(pml.forward, -16, v);
	VectorAdd(pm->ps->grapplePoint, v, v);
	VectorSubtract(v, pm->ps->origin, vel);
	vlen = VectorLength(vel);
	VectorNormalize( vel );

	if (vlen <= 100)
		VectorScale(vel, 10 * vlen, vel);
	else
		VectorScale(vel, 800, vel);

	VectorCopy(vel, pm->ps->velocity);

	pml.groundPlane = qfalse;
}
#endif

/*
===================
PM_WalkMove

===================
*/
static void PM_WalkMove( void ) {
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;
	usercmd_t	cmd;
	float		accelerate;
	float		vel;

	if ( pm->waterlevel > 2 && DotProduct( pml.forward, pml.groundTrace.plane.normal ) > 0 ) {
		// begin swimming
		PM_WaterMove();
		return;
	}


	if ( PM_CheckJump () ) {
		// jumped away
		if ( pm->waterlevel > 1 ) {
			PM_WaterMove();
		} else {
			PM_AirMove();
		}
		return;
	}

	PM_Friction ();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	cmd = pm->cmd;
	scale = PM_CmdScale( &cmd );

	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;

	// project the forward and right directions onto the ground plane
	PM_ClipVelocity (pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP );
	PM_ClipVelocity (pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP );
	//
	VectorNormalize (pml.forward);
	VectorNormalize (pml.right);

	for ( i = 0 ; i < 3 ; i++ ) {
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	}
	// when going up or down slopes the wish velocity should Not be zero
//	wishvel[2] = 0;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	// clamp the speed lower if ducking
	if ( pm->ps->pm_flags & PMF_DUCKED ) {
		if ( wishspeed > pm->ps->speed * pm_duckScale ) {
			wishspeed = pm->ps->speed * pm_duckScale;
		}
	}

	// clamp the speed lower if wading or walking on the bottom
	if ( pm->waterlevel ) {
		float	waterScale;

		waterScale = pm->waterlevel / 3.0;
		waterScale = 1.0 - ( 1.0 - pm_swimScale ) * waterScale;
		if ( wishspeed > pm->ps->speed * waterScale ) {
			wishspeed = pm->ps->speed * waterScale;
		}
	}

	// when a player gets hit, they temporarily lose
	// full control, which allows them to be moved a bit
	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) ) { // BFP - No handling PMF_TIME_KNOCKBACK, before: || pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) {
		accelerate = pm_airaccelerate;
	} else {
		accelerate = pm_accelerate;
	}

	PM_Accelerate (wishdir, wishspeed, accelerate);

	//Com_Printf("velocity = %1.1f %1.1f %1.1f\n", pm->ps->velocity[0], pm->ps->velocity[1], pm->ps->velocity[2]);
	//Com_Printf("velocity1 = %1.1f\n", VectorLength(pm->ps->velocity));

	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) ) { // BFP - No handling PMF_TIME_KNOCKBACK, before: || pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) {
		pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
	} else {
		// don't reset the z velocity for slopes
//		pm->ps->velocity[2] = 0;
	}

	vel = VectorLength(pm->ps->velocity);

	// slide along the ground plane
	PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal, 
		pm->ps->velocity, OVERCLIP );

	// don't decrease velocity when going up or down a slope
	VectorNormalize(pm->ps->velocity);
	VectorScale(pm->ps->velocity, vel, pm->ps->velocity);

	// don't do anything if standing still
	if (!pm->ps->velocity[0] && !pm->ps->velocity[1]) {
		return;
	}

	PM_StepSlideMove( qfalse );

	//Com_Printf("velocity2 = %1.1f\n", VectorLength(pm->ps->velocity));

}


/*
==============
PM_DeadMove
==============
*/
static void PM_DeadMove( void ) {
	float	forward;

	if ( !pml.walking ) {
		return;
	}

	// extra friction

	forward = VectorLength (pm->ps->velocity);
	forward -= 20;
	if ( forward <= 0 ) {
		VectorClear (pm->ps->velocity);
	} else {
		VectorNormalize (pm->ps->velocity);
		VectorScale (pm->ps->velocity, forward, pm->ps->velocity);
	}
}


/*
===============
PM_NoclipMove
===============
*/
static void PM_NoclipMove( void ) {
	float	speed, drop, friction, control, newspeed;
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;

	pm->ps->viewheight = DEFAULT_VIEWHEIGHT;

	// friction

	speed = VectorLength (pm->ps->velocity);
	if (speed < 1)
	{
		VectorCopy (vec3_origin, pm->ps->velocity);
	}
	else
	{
		drop = 0;

		friction = pm_friction*1.5;	// extra friction
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control*friction*pml.frametime;

		// scale the velocity
		newspeed = speed - drop;
		if (newspeed < 0)
			newspeed = 0;
		newspeed /= speed;

		VectorScale (pm->ps->velocity, newspeed, pm->ps->velocity);
	}

	// accelerate
	scale = PM_CmdScale( &pm->cmd );

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;
	
	for (i=0 ; i<3 ; i++)
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	wishvel[2] += pm->cmd.upmove;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	PM_Accelerate( wishdir, wishspeed, pm_accelerate );

	// move
	VectorMA (pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin);
}

//============================================================================

/*
================
PM_FootstepForSurface

Returns an event number apropriate for the groundsurface
================
*/
static int PM_FootstepForSurface( void ) {
	if ( pml.groundTrace.surfaceFlags & SURF_NOSTEPS ) {
		return 0;
	}
	if ( pml.groundTrace.surfaceFlags & SURF_METALSTEPS ) {
		return EV_FOOTSTEP_METAL;
	}
	return EV_FOOTSTEP;
}


/*
=================
PM_CrashLand

Check for hard landings that generate sound events
=================
*/
static void PM_CrashLand( void ) {
	float		delta;
	float		dist;
	float		vel, acc;
	float		t;
	float		a, b, c, den;

	// decide which landing animation to use
	// BFP - Non-existant animations
#if 0
	if ( pm->ps->pm_flags & PMF_BACKWARDS_JUMP ) {
		PM_ForceLegsAnim( LEGS_LANDB );
	} else {
		PM_ForceLegsAnim( LEGS_LAND );
	}
#endif

	// BFP - No timer land on the legs
	// pm->ps->legsTimer = TIMER_LAND; 

	// calculate the exact velocity on landing
	dist = pm->ps->origin[2] - pml.previous_origin[2];
	vel = pml.previous_velocity[2];
	acc = -pm->ps->gravity;

	a = acc / 2;
	b = vel;
	c = -dist;

	den =  b * b - 4 * a * c;
	if ( den < 0 ) {
		return;
	}
	t = (-b - sqrt( den ) ) / ( 2 * a );

	delta = vel + t * acc;
	delta = delta*delta * 0.0001;

	// ducking while falling doubles damage
	if ( pm->ps->pm_flags & PMF_DUCKED ) {
		delta *= 2;
	}

	// never take falling damage if completely underwater
	if ( pm->waterlevel == 3 ) {
		return;
	}

	// reduce falling damage if there is standing water
	if ( pm->waterlevel == 2 ) {
		delta *= 0.25;
	}
	if ( pm->waterlevel == 1 ) {
		delta *= 0.5;
	}

	if ( delta < 1 ) {
		return;
	}
	// create a local entity event to play the sound

	// SURF_NODAMAGE is used for bounce pads where you don't ever
	// want to take damage or play a crunch sound
	if ( !(pml.groundTrace.surfaceFlags & SURF_NODAMAGE) )  {
		if ( delta > 180 ) { // BFP - Before Q3 default value (60), the far fall in BFP is deeper
			PM_AddEvent( EV_FALL_FAR );
		} else if ( delta > 40 ) {
			// this is a pain grunt, so don't play it if dead
			if ( pm->ps->stats[STAT_HEALTH] > 0 ) {
				PM_AddEvent( EV_FALL_MEDIUM );
			}
		} else if ( delta > 7 ) {
			PM_AddEvent( EV_FALL_SHORT );
		} else {
			PM_AddEvent( PM_FootstepForSurface() );
		}
	}

	// start footstep cycle over
	pm->ps->bobCycle = 0;
}

/*
=============
PM_CheckStuck
=============
*/
static void PM_CheckStuck(void) {
	// BFP - NOTE: Curiously and originally, BFP uses this function to animate when the player is stuck, 
	// that can be tested when the player is pretty near to the other player
	// or being stuck in the same origin as the other player, specially outside water.
	// It has been implemented when melee animations were being used
	trace_t trace;

	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);
	if (trace.allsolid) {
		//int shit = qtrue;

		// BFP - Handle the animations when being stuck! (Only outside water)
		if ( pm->waterlevel < 1 ) {
			if ( pm->cmd.forwardmove < 0 ) {
				PM_ContinueLegsAnim( LEGS_JUMPB );
			} else {
				PM_ContinueLegsAnim( LEGS_JUMP );
			}
		}
		// BFP - Melee strike legs animation
		CONTINUEMELEESTRIKE_LEGS_ANIM_HANDLING( 1 )
	}
}

/*
=============
PM_CorrectAllSolid
=============
*/
static int PM_CorrectAllSolid( trace_t *trace ) {
	int			i, j, k;
	vec3_t		point;

	if ( pm->debugLevel ) {
		Com_Printf("%i:allsolid\n", c_pmove);
	}

	// jitter around
	for (i = -1; i <= 1; i++) {
		for (j = -1; j <= 1; j++) {
			for (k = -1; k <= 1; k++) {
				VectorCopy(pm->ps->origin, point);
				point[0] += (float) i;
				point[1] += (float) j;
				point[2] += (float) k;
				pm->trace (trace, point, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
				if ( !trace->allsolid ) {
					point[0] = pm->ps->origin[0];
					point[1] = pm->ps->origin[1];
					point[2] = pm->ps->origin[2] - 0.25;

					pm->trace (trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
					pml.groundTrace = *trace;
					return qtrue;
				}
			}
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;

	return qfalse;
}


/*
=============
PM_GroundTraceMissed

The ground trace didn't hit a surface, so we are in freefall
=============
*/
static void PM_GroundTraceMissed( void ) {
	trace_t		trace;
	vec3_t		point;

	if ( pm->ps->groundEntityNum != ENTITYNUM_NONE ) {
		// we just transitioned into freefall
		if ( pm->debugLevel ) {
			Com_Printf("%i:lift\n", c_pmove);
		}

		// if they aren't in a jumping animation and the ground is a ways away, force into it
		// if we didn't do the trace, the player would be backflipping down staircases
		VectorCopy( pm->ps->origin, point );
		point[2] -= 64;

		pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
		if ( trace.fraction == 1.0 ) {
			// BFP - No PMF_BACKWARDS_JUMP handling (code removed)
			FORCEJUMP_ANIM_HANDLING();
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
}


/*
=============
PM_GroundTrace
=============
*/
static void PM_GroundTrace( void ) {
	vec3_t		point;
	trace_t		trace;

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] - 0.25;

	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
	pml.groundTrace = trace;

	// BFP - No ground trace handling in the water
	if ( pm->waterlevel > 1 ) {
		return;
	}

	// do something corrective if the trace starts in a solid...
	if ( trace.allsolid ) {
		if ( !PM_CorrectAllSolid(&trace) )
			return;
	}

	// if the trace didn't hit anything, we are in free fall
	if ( trace.fraction == 1.0 ) {
		PM_GroundTraceMissed();
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}

	// check if getting thrown off the ground
	if ( pm->ps->velocity[2] > 0 && DotProduct( pm->ps->velocity, trace.plane.normal ) > 10 ) {
		if ( pm->debugLevel ) {
			Com_Printf("%i:kickoff\n", c_pmove);
		}

		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}

	// BFP - Make sure to handle the PMF flags when the player isn't flying
	if ( pm->ps->powerups[PW_FLIGHT] <= 0 ) {
		pm->ps->pm_flags |= PMF_FALLING;
		pm->ps->pm_flags &= ~PMF_NEARGROUND;
	}

	// BFP - If the player is in the ground, then jump!
	// And make sure to handle the PMF flag when the player isn't flying and falling
	if ( pm->ps->powerups[PW_FLIGHT] > 0
	&& ( pm->ps->pm_flags & PMF_FALLING ) 
	&& !( pm->ps->pm_flags & PMF_NEARGROUND ) ) {
		if ( pml.groundTrace.contents & MASK_PLAYERSOLID ) {
			// do a smooth jump animation like BFP does
			if ( !( pm->cmd.buttons & BUTTON_KI_CHARGE )
			&& pm->ps->weaponstate != WEAPON_KIEXPLOSIONWAVE
			&& pm->ps->weaponstate != WEAPON_STUN ) {
				pm->ps->pm_time = 550;
				pm->ps->velocity[2] = JUMP_VELOCITY;
				if ( !( pm->ps->pm_flags & PMF_KI_ATTACK )
				&& !( pm->ps->pm_flags & PMF_MELEE ) ) {
					if ( pm->cmd.forwardmove > 0 ) {
						TORSOSTATUS_ANIM_HANDLING( TORSO_FLYA );
					} else if ( pm->cmd.forwardmove < 0 ) {
						TORSOSTATUS_ANIM_HANDLING( TORSO_FLYB );
					} else {
						TORSOSTATUS_ANIM_HANDLING( TORSO_STAND );
					}
				}
				PM_ForceLegsAnim( LEGS_JUMP );
			}
			pml.groundPlane = qfalse;		// jumping away
			pml.walking = qfalse;
		}
		pm->ps->pm_flags &= ~PMF_FALLING;	
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		return;
	}

	// slopes that are too steep will not be considered onground
	if ( trace.plane.normal[2] < MIN_WALK_NORMAL ) {
		if ( pm->debugLevel ) {
			Com_Printf("%i:steep\n", c_pmove);
		}
		// FIXME: if they can't slide down the slope, let them
		// walk (sharp crevices)
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qtrue;
		pml.walking = qfalse;

		// BFP - If flying, prevent from doing a jumping action on slopes
		if ( pm->ps->powerups[PW_FLIGHT] > 0 ) {
			return;
		}

		// BFP - Handle if the player is trying to jump and/or do another movements
		// when stepping the steep slopes
		if ( PM_CheckJump () ) {
			// jumped away
			if ( pm->waterlevel > 1 ) {
				PM_WaterMove();
			} else {
				PM_AirMove();
			}
			return;
		}

		// BFP - Handling the PMF flags when that happens
		pm->ps->pm_flags &= ~PMF_NEARGROUND;

		SLOPES_NEARGROUND_ANIM_HANDLING( 1 )
		return;
	}

	// BFP - NOTE: Originally, BFP doesn't stop "groundtracing" until here when the player is flying
	// BFP - If flying, prevent from doing a jumping action on flat ground
	if ( pm->ps->powerups[PW_FLIGHT] > 0 ) {
		// BFP - To stick to the movers if the player is near to them
		pm->ps->groundEntityNum = trace.entityNum;
		PM_AddTouchEnt( trace.entityNum );
		pm->ps->pm_flags |= PMF_NEARGROUND;
		return;
	}

	pml.groundPlane = qtrue;
	pml.walking = qtrue;

	// BFP - No handling PMF_TIME_WATERJUMP
#if 0
	// hitting solid ground will end a waterjump
	if (pm->ps->pm_flags & PMF_TIME_WATERJUMP)
	{
		pm->ps->pm_flags &= ~PMF_TIME_WATERJUMP; // BFP: before: ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND);
		pm->ps->pm_time = 0;
	}
#endif

	// BFP - Handle when the player isn't flying
	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE 
	&& pm->ps->powerups[PW_FLIGHT] <= 0 ) {
		// just hit the ground
		if ( pm->debugLevel ) {
			Com_Printf("%i:Land\n", c_pmove);
		}
		
		PM_CrashLand();

		// BFP - Handling the PM flag when stepping the ground
		pm->ps->pm_flags &= ~PMF_NEARGROUND;

		// BFP - PMF_TIME_LAND doesn't exist and it doesn't have any handle checks
#if 0
		// don't do landing time if we were just going down a slope
		if ( pml.previous_velocity[2] < -200 ) {
			// don't allow another jump for a little while
			pm->ps->pm_flags |= PMF_TIME_LAND;
			pm->ps->pm_time = 250;
		}
#endif
	}

	pm->ps->groundEntityNum = trace.entityNum;

	// don't reset the z velocity for slopes
	// pm->ps->velocity[2] = 0;

	// BFP - Avoid jumping unintentionally when that happens
	if ( trace.plane.normal[2] == 1.0 ) {
		pm->ps->velocity[2] = 0;
	}

	PM_AddTouchEnt( trace.entityNum );
}


/*
=============
PM_SetWaterLevel	FIXME: avoid this twice?  certainly if not moving
=============
*/
static void PM_SetWaterLevel( void ) {
	vec3_t		point;
	int			cont;
	int			sample1;
	int			sample2;

	//
	// get waterlevel, accounting for ducking
	//
	pm->waterlevel = 0;
	pm->watertype = 0;

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] + MINS_Z + 1;	
	cont = pm->pointcontents( point, pm->ps->clientNum );

	if ( cont & MASK_WATER ) {
		sample2 = pm->ps->viewheight - MINS_Z;
		sample1 = sample2 / 2;

		pm->watertype = cont;
		pm->waterlevel = 1;
		point[2] = pm->ps->origin[2] + MINS_Z + sample1;
		cont = pm->pointcontents (point, pm->ps->clientNum );
		if ( cont & MASK_WATER ) {
			pm->waterlevel = 2;
			point[2] = pm->ps->origin[2] + MINS_Z + sample2;
			cont = pm->pointcontents (point, pm->ps->clientNum );
			if ( cont & MASK_WATER ){
				pm->waterlevel = 3;
			}
		}
	}

}

/*
==============
PM_CheckDuck

Sets mins, maxs, and pm->ps->viewheight
==============
*/
static void PM_CheckDuck (void)
{
	trace_t	trace;

	if ( pm->ps->powerups[PW_INVULNERABILITY] ) {
		if ( pm->ps->pm_flags & PMF_INVULEXPAND ) {
			// invulnerability sphere has a 42 units radius
			VectorSet( pm->mins, -42, -42, -42 );
			VectorSet( pm->maxs, 42, 42, 42 );
		}
		else {
			VectorSet( pm->mins, -15, -15, MINS_Z );
			VectorSet( pm->maxs, 15, 15, 16 );
		}
		pm->ps->pm_flags |= PMF_DUCKED;
		pm->ps->viewheight = CROUCH_VIEWHEIGHT;
		return;
	}
	pm->ps->pm_flags &= ~PMF_INVULEXPAND;

	pm->mins[0] = -15;
	pm->mins[1] = -15;

	pm->maxs[0] = 15;
	pm->maxs[1] = 15;

	pm->mins[2] = MINS_Z;

	if (pm->ps->pm_type == PM_DEAD)
	{
		pm->maxs[2] = -8;
		pm->ps->viewheight = DEAD_VIEWHEIGHT;
		return;
	}

	if (pm->cmd.upmove < 0)
	{	// duck
		pm->ps->pm_flags |= PMF_DUCKED;
	}
	else
	{	// stand up if possible
		if (pm->ps->pm_flags & PMF_DUCKED)
		{
			// try to stand up
			pm->maxs[2] = 32;
			pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask );
			if (!trace.allsolid)
				pm->ps->pm_flags &= ~PMF_DUCKED;
		}
	}

	if (pm->ps->pm_flags & PMF_DUCKED)
	{
		pm->maxs[2] = 16;
		pm->ps->viewheight = CROUCH_VIEWHEIGHT;
	}
	else
	{
		pm->maxs[2] = 32;
		pm->ps->viewheight = DEFAULT_VIEWHEIGHT;
	}
}



//===================================================================


/*
===============
PM_Footsteps
===============
*/
static void PM_Footsteps( void ) {
	float		bobmove;
	int			old;
	qboolean	footstep;

	// BFP - Hit stun
	if ( pm->ps->pm_flags & PMF_HITSTUN ) {
		return;
	}

	// BFP - Ki explosion wave state
	if ( pm->ps->weaponstate == WEAPON_KIEXPLOSIONWAVE ) {
		PM_ContinueLegsAnim( LEGS_IDLE );
		return;
	}

	// BFP - Ki explosion wave stun state
	if ( pm->ps->weaponstate == WEAPON_STUN ) {
		PM_ContinueTorsoAnim( TORSO_STUN );
		PM_ContinueLegsAnim( LEGS_IDLE );
		return;
	}

	// BFP - Avoid when flying (for melee strike animation, that's applied)
	if ( pm->ps->powerups[PW_FLIGHT] > 0 ) {
		// BFP - Melee strike legs animation, don't apply if it's playing the starting jump animation in the flight status
		CONTINUEMELEESTRIKE_LEGS_ANIM_HANDLING( pm->ps->pm_time <= 0 )
		return;
	}

	// BFP - Avoid when charging
	if ( pm->ps->pm_flags & PMF_KI_CHARGE ) {
		return;
	}

	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
	pm->xyspeed = sqrt( pm->ps->velocity[0] * pm->ps->velocity[0]
		+  pm->ps->velocity[1] * pm->ps->velocity[1] );

	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
		// BFP - PM_CheckStuck has been moved here, Q3 and the rest of mods hadn't used this
		PM_CheckStuck();

		// BFP - Underwater animation handling, uses flying animation in that case
		// also keep the torso
		if ( pm->waterlevel > 2 ) {
			CONTINUEFLY_ANIM_HANDLING()
		} else {
			// BFP - Keep the torso when using a ki attack even after charged, avoid when melee is being used
			if ( !( pm->cmd.buttons & BUTTON_MELEE ) ) {
				KI_ATTACK_TORSO_ANIM_HANDLING()
			}
		}

		return;
	}

	// if not trying to move
	if ( !pm->cmd.forwardmove && !pm->cmd.rightmove ) {
		if (  pm->xyspeed < 5 ) {
			pm->ps->bobCycle = 0;	// start at beginning of cycle again
			if ( pm->ps->pm_flags & PMF_DUCKED ) {
				PM_ContinueLegsAnim( LEGS_IDLECR );
			} else if ( !( pm->ps->pm_flags & PMF_KI_CHARGE ) ) {
				PM_ContinueLegsAnim( LEGS_IDLE );
			}
		} else if ( !( pm->ps->pm_flags & PMF_DUCKED ) ) { // BFP - Handle the legs while it isn't doing nothing
			PM_ContinueLegsAnim( LEGS_IDLE );
		}
		// BFP - Melee strike legs animation
		CONTINUEMELEESTRIKE_LEGS_ANIM_HANDLING( 1 )
		return;
	}
	

	footstep = qfalse;

	if ( pm->ps->pm_flags & PMF_DUCKED ) {
		bobmove = 0.5;	// ducked characters bob much faster
		// BFP - Replaced PMF_BACKWARDS_RUN handling
		if ( pml.groundTrace.contents & MASK_PLAYERSOLID ) {
			if ( pm->cmd.forwardmove < 0 ) {
				PM_ContinueLegsAnim( LEGS_WALKCR ); // BFP - before LEGS_BACKCR
			} else if ( pm->cmd.forwardmove > 0 || ( pm->cmd.forwardmove == 0 && pm->cmd.rightmove ) ) {
				PM_ContinueLegsAnim( LEGS_WALKCR );
			}
			TORSOSTATUS_ANIM_HANDLING( TORSO_STAND ); // BFP - Keep the torso
		}
		// ducked characters never play footsteps
	/*
	} else 	if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
		if ( !( pm->cmd.buttons & BUTTON_WALKING ) ) {
			bobmove = 0.4;	// faster speeds bob faster
			footstep = qtrue;
		} else {
			bobmove = 0.3;
		}
		PM_ContinueLegsAnim( LEGS_BACK );
	*/
	} else {
		if ( !( pm->cmd.buttons & BUTTON_WALKING ) && ( pml.groundTrace.contents & MASK_PLAYERSOLID ) ) {
			bobmove = 0.4f;	// faster speeds bob faster
			// BFP - Replaced PMF_BACKWARDS_RUN handling
			if ( pm->cmd.forwardmove < 0 ) {
				PM_ContinueLegsAnim( LEGS_BACK );
				TORSOSTATUS_ANIM_HANDLING( TORSO_STAND ); // BFP - Keep the torso
			} else if ( pm->cmd.forwardmove > 0 || ( pm->cmd.forwardmove == 0 && pm->cmd.rightmove ) ) {
				PM_ContinueLegsAnim( LEGS_RUN );
				TORSOSTATUS_ANIM_HANDLING( TORSO_RUN ); // BFP - Keep the torso
			}
			footstep = qtrue;
		} else if ( pml.groundTrace.contents & MASK_PLAYERSOLID ) {
			bobmove = 0.3f;	// walking bobs slow
			// BFP - Replaced PMF_BACKWARDS_RUN handling
			if ( pm->cmd.forwardmove < 0 ) {
				PM_ContinueLegsAnim( LEGS_BACK );
			} else if ( pm->cmd.forwardmove > 0 || ( pm->cmd.forwardmove == 0 && pm->cmd.rightmove ) ) {
				PM_ContinueLegsAnim( LEGS_WALK );
			}
			TORSOSTATUS_ANIM_HANDLING( TORSO_STAND ); // BFP - Keep the torso
		}
	}

	// BFP - Melee strike legs animation
	CONTINUEMELEESTRIKE_LEGS_ANIM_HANDLING( 1 )

	// check for footstep / splash sounds
	old = pm->ps->bobCycle;
	pm->ps->bobCycle = (int)( old + bobmove * pml.msec ) & 255;

	// if we just crossed a cycle boundary, play an apropriate footstep event
	if ( ( ( old + 64 ) ^ ( pm->ps->bobCycle + 64 ) ) & 128 ) {
		if ( pm->waterlevel == 0 ) {
			// on ground will only play sounds if running
			if ( footstep && !pm->noFootsteps ) {
				PM_AddEvent( PM_FootstepForSurface() );
			}
		} else if ( pm->waterlevel == 1 ) {
			// splashing
			PM_AddEvent( EV_FOOTSPLASH );
		} else if ( pm->waterlevel == 2 ) {
			// wading / swimming at surface
			PM_AddEvent( EV_SWIM );
		} else if ( pm->waterlevel == 3 ) {
			// no sound when completely underwater

		}
	}
}

/*
==============
PM_WaterEvents

Generate sound events for entering and leaving water
==============
*/
static void PM_WaterEvents( void ) {		// FIXME?
	//
	// if just entered a water volume, play a sound
	//
	if (!pml.previous_waterlevel && pm->waterlevel) {
		PM_AddEvent( EV_WATER_TOUCH );
	}

	//
	// if just completely exited a water volume, play a sound
	//
	if (pml.previous_waterlevel && !pm->waterlevel) {
		PM_AddEvent( EV_WATER_LEAVE );
		// BFP - Handle jumping animation when getting out of the water
		if ( pm->ps->powerups[PW_FLIGHT] <= 0 ) {
			FORCEJUMP_ANIM_HANDLING();
		}
	}

	//
	// check for head just going under water
	//
	if (pml.previous_waterlevel != 3 && pm->waterlevel == 3) {
		PM_AddEvent( EV_WATER_UNDER );
	}

	//
	// check for head just coming out of water
	//
	if (pml.previous_waterlevel == 3 && pm->waterlevel != 3) {
		PM_AddEvent( EV_WATER_CLEAR );
	}
}


/*
===============
PM_BeginWeaponChange
===============
*/
static void PM_BeginWeaponChange( int weapon ) {
	if ( weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS ) {
		return;
	}

	if ( !( pm->ps->stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) {
		return;
	}
	
	if ( pm->ps->weaponstate == WEAPON_DROPPING ) {
		return;
	}

	PM_AddEvent( EV_CHANGE_WEAPON );
	pm->ps->weaponstate = WEAPON_DROPPING;
	// BFP - Don't add weaponTime when changing ki attack
	// pm->ps->weaponTime += 200;
	// BFP - Non-existant animation
	// PM_StartTorsoAnim( TORSO_DROP );
}


/*
===============
PM_FinishWeaponChange
===============
*/
static void PM_FinishWeaponChange( void ) {
	int		weapon;

	weapon = pm->cmd.weapon;
	if ( weapon < WP_NONE || weapon >= WP_NUM_WEAPONS ) {
		weapon = WP_NONE;
	}

	if ( !( pm->ps->stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) {
		weapon = WP_NONE;
	}

	pm->ps->weapon = weapon;
	pm->ps->weaponstate = WEAPON_RAISING;
	// BFP - Don't add weaponTime when changing ki attack
	// pm->ps->weaponTime += 250;
	// BFP - Non-existant animation
	// PM_StartTorsoAnim( TORSO_RAISE );
}


/*
==============
PM_TorsoAnimation

==============
*/
static void PM_TorsoAnimation( void ) {
	// BFP - NOTE: That function could be called as "PM_NearGround", 
	// here is tracing something similar to PM_GroundTraceMissed
	trace_t		trace;
	vec3_t		point;

	// BFP - No ground trace handling in the water
	if ( pm->waterlevel > 1 ) {
		return;
	}

	VectorCopy( pm->ps->origin, point );
	point[2] -= 128; // BFP - Put more down, obviously it was 64, but BFP does that

	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
	pml.groundTrace = trace;

	// BFP - Falling distantly from the ground
	if ( trace.fraction == 1.0 && !( pm->ps->pm_flags & PMF_NEARGROUND )
	&& pm->ps->powerups[PW_FLIGHT] <= 0 ) {
		pm->ps->pm_flags |= PMF_NEARGROUND;
		FORCEJUMP_ANIM_HANDLING();
		TORSOSTATUS_ANIM_HANDLING( TORSO_STAND );
	}

	// If idling, keep the torso
	if ( !pm->cmd.forwardmove && !pm->cmd.rightmove ) {
		TORSOSTATUS_ANIM_HANDLING( TORSO_STAND );
	}

	// Handle the player movement animation when stopping to fly and falling near to the ground
	// that happens when PMF_FALLING flag isn't handled correctly
	if ( ( pml.groundTrace.contents & MASK_PLAYERSOLID )
	&& pm->ps->powerups[PW_FLIGHT] <= 0
	&& !( pm->ps->pm_flags & PMF_FALLING )
	&& ( pm->ps->pm_flags & PMF_NEARGROUND ) ) {
		pm->ps->pm_flags |= PMF_FALLING;
		pm->ps->pm_flags &= ~PMF_NEARGROUND;
	}

	// BFP - That happens when the player is landing nearly
	if ( !( pm->ps->pm_flags & PMF_NEARGROUND )
	&& pm->ps->powerups[PW_FLIGHT] <= 0
	&& pm->ps->groundEntityNum == ENTITYNUM_NONE // hasn't touched the ground yet
	&& ( pml.groundTrace.contents & MASK_PLAYERSOLID ) ) {
		SLOPES_NEARGROUND_ANIM_HANDLING( 0 )
	}

	// BFP - Melee strike legs animation, don't apply if it isn't touching the ground
	CONTINUEMELEESTRIKE_LEGS_ANIM_HANDLING( pm->ps->groundEntityNum != ENTITYNUM_NONE )

#if 0
	if ( pm->ps->weaponstate == WEAPON_READY ) {
		if ( pm->ps->weapon == WP_GAUNTLET ) {
			PM_ContinueTorsoAnim( TORSO_STAND ); // BFP - before TORSO_STAND2
		} else {
			PM_ContinueTorsoAnim( TORSO_STAND );
		}
		return;
	}
#endif
}

/*
==============
PM_FlightAnimation
==============
*/
static void PM_FlightAnimation( void ) { // BFP - Flight

	if ( pm->ps->powerups[PW_FLIGHT] > 0 && pm->ps->pm_time <= 0 ) {

		// make sure to handle the PMF flag
		pm->ps->pm_flags &= ~PMF_FALLING;

		CONTINUEFLY_ANIM_HANDLING()
		return;
	}

	// Handle the player movement animation if trying to change quickly the direction of forward or backward
	if ( !( pml.groundTrace.contents & MASK_PLAYERSOLID )
	&& !( pm->ps->pm_flags & PMF_FALLING )
	&& pm->ps->powerups[PW_FLIGHT] <= 0 ) {

		// stops entering again here and don't change the animation to backwards/forward
		pm->ps->pm_flags |= PMF_FALLING;
		pm->ps->pm_flags &= ~PMF_NEARGROUND;

		if ( pm->cmd.forwardmove < 0 ) { // when failing backwards after flying
			PM_StartLegsAnim( LEGS_JUMPB );
		} else {
			PM_StartLegsAnim( LEGS_JUMP );
		}
		TORSOSTATUS_ANIM_HANDLING( TORSO_STAND );
	}
}

/*
==============
PM_KiChargeAnimation
==============
*/
static void PM_KiChargeAnimation( void ) { // BFP - Ki Charge

	// stop charging if it's using ki boost
	if ( ( pm->cmd.buttons & BUTTON_KI_USE ) && ( pm->cmd.buttons & BUTTON_KI_CHARGE ) ) {
		// handle the button to avoid toggling the animations forward and backwards while using ki boost
		pm->cmd.buttons &= ~BUTTON_KI_CHARGE;
	}

	if ( ( pm->cmd.buttons & BUTTON_KI_USE ) && ( pm->ps->pm_flags & PMF_KI_CHARGE ) ) {
		pm->ps->pm_flags &= ~PMF_KI_CHARGE;
		pm->ps->pm_time = 0;
		// do jump animation if it's falling
		if ( !( pml.groundTrace.contents & MASK_PLAYERSOLID )
			&& pm->ps->powerups[PW_FLIGHT] <= 0
			&& ( pm->ps->pm_flags & PMF_FALLING )
			&& pm->waterlevel <= 1 ) { // Don't force inside the water
			pm->ps->pm_flags &= ~PMF_FALLING; // Handle PMF_FALLING when falling
			FORCEJUMP_ANIM_HANDLING();
			PM_ContinueTorsoAnim( TORSO_STAND ); // Keep the torso
		}
		return;
	}

	if ( ( pm->ps->pm_flags & PMF_KI_CHARGE ) && !( pm->cmd.buttons & BUTTON_KI_CHARGE ) ) {
		pm->ps->eFlags &= ~EF_AURA; // Make sure the aura is off, otherwise the ki use proceeds
		pm->ps->pm_flags &= ~PMF_KI_CHARGE;
		pm->ps->pm_flags &= ~PMF_NEARGROUND; // Make sure to handle the PMF flag
		PM_ContinueLegsAnim( LEGS_IDLE ); // Keep the legs when being near to the ground at that height
		// do jump animation if it's falling
		if ( !( pml.groundTrace.contents & MASK_PLAYERSOLID )
			&& ( pm->ps->pm_flags & PMF_FALLING )
			&& pm->waterlevel <= 1 ) { // Don't force inside the water
			FORCEJUMP_ANIM_HANDLING();
			PM_ContinueTorsoAnim( TORSO_STAND ); // Keep the torso
		}
	}

	if ( pm->cmd.buttons & BUTTON_KI_CHARGE ) {
		pm->ps->powerups[PW_HASTE] = 0;
		pm->ps->pm_flags |= PMF_KI_CHARGE;
		PM_ContinueTorsoAnim( TORSO_CHARGE );
		PM_ContinueLegsAnim( LEGS_CHARGE );
	}

	// handle the button to avoid toggling ki boost when already used "kiusetoggle" key bind
	if ( ( pm->cmd.buttons & BUTTON_KI_USE ) && ( pm->ps->powerups[PW_HASTE] > 0 ) ) {
		pm->ps->powerups[PW_HASTE] = 0;
	}
}

/*
==============
PM_HitStunAnimation
==============
*/
static void PM_HitStunAnimation( void ) { // BFP - Hit stun

	if ( pm->ps->pm_flags & PMF_HITSTUN ) {
		PM_StartTorsoAnim( TORSO_STUN );
		PM_StartLegsAnim( LEGS_IDLECR );
	}

	// When the player doesn't have more ki, play hit stun animation
	if ( pm->ps->stats[STAT_KI] <= 0 && !( pm->ps->pm_flags & PMF_HITSTUN )
		|| ( pm->ps->powerups[PW_FLIGHT] > 0 && pm->ps->stats[STAT_KI] <= 24 ) // BFP - TODO: Apply some timer if used any ki, if flying and has less ki, then hit stun (also BFP does that)
		|| ( ( pm->cmd.buttons & BUTTON_ATTACK ) && ( pm->ps->pm_flags & PMF_HITSTUN ) ) ) {
		pm->ps->pm_time = 1000;
		pm->ps->pm_flags |= PMF_HITSTUN;
	}

	if ( ( pm->ps->pm_flags & PMF_HITSTUN ) && pm->ps->pm_time <= 0 ) {
		pm->ps->pm_flags &= ~PMF_HITSTUN;
		// do jump animation if it's falling
		if ( !( pml.groundTrace.contents & MASK_PLAYERSOLID )
			&& ( pm->ps->pm_flags & PMF_FALLING ) ) {
			FORCEJUMP_ANIM_HANDLING();
			PM_ContinueTorsoAnim( TORSO_STAND ); // Keep the torso
		}
	}
}

/*
==============
PM_Melee
==============
*/
static void PM_Melee( void ) { // BFP - Melee
	// Don't allow pressing ki attack and block buttons when melee is being used
	if ( ( pm->ps->pm_flags & PMF_MELEE ) 
	|| ( pm->cmd.buttons & BUTTON_MELEE ) ) {
		pm->cmd.buttons &= ~( BUTTON_ATTACK | BUTTON_BLOCK );
	}
}

/*
==============
PM_KiExplosionWave

Handle ki explosion wave during and at the end of its use
==============
*/
static void PM_KiExplosionWave( void ) { // BFP - Ki explosion wave handling
	// ki explosion wave state
	if ( pm->ps->weaponstate == WEAPON_KIEXPLOSIONWAVE ) {
		// don't move, also these keys cannot be used; blocking, ki charge and ki boost statuses are removed
		pm->cmd.forwardmove = pm->cmd.rightmove = pm->cmd.upmove = 0;
		pm->cmd.buttons &= ~( BUTTON_KI_USE | BUTTON_KI_CHARGE | BUTTON_BLOCK );
		pm->ps->pm_flags &= ~( PMF_KI_CHARGE | PMF_BLOCK );
		pm->ps->powerups[PW_HASTE] = 0;
		return;
	}

	// ki explosion wave stun state
	if ( pm->ps->weaponstate == WEAPON_STUN ) {
		// don't move, also these keys cannot be used; melee, attacking, blocking, ki charge and ki boost statuses are removed
		pm->cmd.forwardmove = pm->cmd.rightmove = pm->cmd.upmove = 0;
		pm->cmd.buttons &= ~( BUTTON_ATTACK | BUTTON_KI_CHARGE | BUTTON_KI_USE | BUTTON_BLOCK | BUTTON_MELEE );
		pm->ps->pm_flags &= ~( PMF_KI_ATTACK | PMF_KI_CHARGE | PMF_BLOCK | PMF_MELEE );
		pm->ps->powerups[PW_HASTE] = 0;
		return;
	}
}


/*
==============
PM_Weapon

Generates weapon events and modifes the weapon counter
==============
*/
static void PM_Weapon( void ) {
	int		addTime;

	// BFP - Hit stun, avoid shooting if the player is in this status
	if ( pm->ps->pm_flags & PMF_HITSTUN ) {
		pm->ps->stats[STAT_READY_KI_ATTACK] = qfalse;
		pm->ps->stats[STAT_KI_ATTACK_CHARGE] = 0;
		pm->ps->weaponTime = 0;
		return;
	}

	// don't allow attack until all buttons are up
	if ( pm->ps->pm_flags & PMF_RESPAWNED ) {
		return;
	}

	// ignore if spectator
	if ( pm->ps->persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
		return;
	}

	// check for dead player
	if ( pm->ps->stats[STAT_HEALTH] <= 0 ) {
		pm->ps->weapon = WP_NONE;
		return;
	}

	// check for item using
	if ( pm->cmd.buttons & BUTTON_USE_HOLDABLE ) {
		if ( ! ( pm->ps->pm_flags & PMF_USE_ITEM_HELD ) ) {
			if ( bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag == HI_MEDKIT
				&& pm->ps->stats[STAT_HEALTH] >= (pm->ps->stats[STAT_MAX_HEALTH] + 25) ) {
				// don't use medkit if at max health
			} else {
				pm->ps->pm_flags |= PMF_USE_ITEM_HELD;
				PM_AddEvent( EV_USE_ITEM0 + bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag );
				pm->ps->stats[STAT_HOLDABLE_ITEM] = 0;
			}
			return;
		}
	} else {
		pm->ps->pm_flags &= ~PMF_USE_ITEM_HELD;
	}

	// make weapon function
	if ( pm->ps->weaponTime > 0 ) {
		pm->ps->weaponTime -= pml.msec;
	}

	// check for weapon change
	// can't change if weapon is firing, but can change
	// again if lowering or raising
	if ( pm->ps->weaponstate != WEAPON_BEAMFIRING // BFP - Avoid if the beam is still firing
	&& pm->ps->weaponstate != WEAPON_STUN // BFP - Avoid when being stunned
	&& ( pm->ps->weaponTime <= 0 || pm->ps->weaponstate != WEAPON_FIRING ) ) {
		if ( pm->ps->weapon != pm->cmd.weapon ) {
			PM_BeginWeaponChange( pm->cmd.weapon );
		}
	}

	// change weapon if time
	if ( pm->ps->weaponstate == WEAPON_DROPPING 
	&& pm->ps->weaponTime <= 0 ) { // BFP - Handling ki attack animations when these already shoot
		PM_FinishWeaponChange();
		return;
	}

	// BFP - Melee, avoid shooting if the player is in this status
	if ( pm->cmd.buttons & BUTTON_MELEE ) {
		// only use when there's no dividing ki ball until it has been divided or collided, 
		// unless if the player wanna change the weapon from this state
		if ( pm->ps->weaponstate != WEAPON_DIVIDINGKIBALLFIRING ) {
			pm->ps->weaponstate = WEAPON_READY;
			pm->ps->stats[STAT_KI_ATTACK_CHARGE] = 0;
		}
		// Melee fight handling
		if ( pm->meleeHit && pm->ps->weaponTime <= 0 ) {
			pm->ps->weaponTime += 300;
			pm->ps->pm_flags |= PMF_MELEE;
		}
		return;
	}

	// BFP - Weapon states, Q3 doesn't have this way
	switch( pm->ps->weaponstate ) {
	case WEAPON_READY:
		if ( pm->ps->weaponTime <= 0 ) {
			pm->ps->weaponTime = 0;
			pm->ps->pm_flags &= ~PMF_KI_ATTACK;
			// check for fire
			if ( pm->cmd.buttons & BUTTON_ATTACK ) {

				// BFP - NOTE: These are just examples of ki charging and shooting,
				// - WP_GRENADE_LAUNCHER should be like WP_MACHINEGUN and WP_LIGHTNINGGUN to keep the continuous shooting animations
				//   WP_GRENADE_LAUNCHER is used as example of charge homing ball shot
				// - WP_SHOTGUN is used as example of ki explosion wave
				// - WP_PLASMAGUN is used as example of dividing ki ball
				// - WP_BFG is used as example of ki beam
				switch( pm->ps->weapon ) {
				case WP_GRENADE_LAUNCHER:
				case WP_PLASMAGUN:
				case WP_BFG:
					pm->ps->weaponstate = WEAPON_CHARGING;
					break;
				case WP_SHOTGUN:
					pm->ps->weaponstate = WEAPON_KIEXPLOSIONWAVE;
					break;
				default: 
					pm->ps->weaponstate = WEAPON_FIRING;
					break;
				}
			}
		}
		if ( !( pm->cmd.buttons & BUTTON_ATTACK ) ) {
			pm->ps->stats[STAT_KI_ATTACK_CHARGE] = 0;
		}

		break;
	case WEAPON_DROPPING:
	case WEAPON_RAISING:
		if ( pm->ps->weaponTime <= 0 ) {
			pm->ps->weaponTime = 0;
			pm->ps->stats[STAT_KI_ATTACK_CHARGE] = 0;
			pm->ps->weaponstate = WEAPON_READY;
		}
		break;
	case WEAPON_CHARGING:
		if ( pm->ps->weaponTime <= 0 ) {
			break;
		}

		if ( !( pm->cmd.buttons & BUTTON_ATTACK ) ) {
			// BFP - When the ki attack is fully charged, enter beam firing state
			// or enter dividing ki ball firing state if it's a dividing ki ball

#define FIRE_CHARGED_STATE(wepstate) \
	pm->ps->pm_flags |= PMF_KI_ATTACK; \
	pm->ps->weaponstate = wepstate;

			switch( pm->ps->weapon ) {
			case WP_GRENADE_LAUNCHER:
				FIRE_CHARGED_STATE( WEAPON_EXPLODING_KIBALLFIRING )
				break;
			case WP_PLASMAGUN:
				FIRE_CHARGED_STATE( WEAPON_DIVIDINGKIBALLFIRING )
				break;
			case WP_BFG:
				FIRE_CHARGED_STATE( WEAPON_BEAMFIRING )
			}
			pm->ps->stats[STAT_KI_ATTACK_CHARGE] = 0;
		}
		break;
	case WEAPON_FIRING:
		if ( !( pm->cmd.buttons & BUTTON_ATTACK ) ) {
			pm->ps->weaponstate = WEAPON_READY;
			break;
		}

		// fire weapon
		switch( pm->ps->weapon ) {
		case WP_MACHINEGUN:
		case WP_ROCKET_LAUNCHER:
		case WP_RAILGUN:
			pm->ps->weaponstate = WEAPON_READY;
			pm->ps->pm_flags |= PMF_KI_ATTACK;
			//PM_AddEvent( EV_FIRE_WEAPON );
			break;
		case WP_LIGHTNING:
			pm->ps->pm_flags |= PMF_KI_ATTACK;
		}
		break;
	// BFP - NOTE: The beam is triggering until pressing the attack key again after holded, using ki charge or blocking
	// Pressing attack key again or changing weapon, the beam is exploded before the impact
	case WEAPON_BEAMFIRING:
		if ( ( pm->cmd.buttons & BUTTON_ATTACK )
		|| ( pm->ps->pm_flags & PMF_KI_CHARGE )
		|| ( pm->ps->pm_flags & PMF_BLOCK )
		|| !( pm->ps->pm_flags & PMF_KI_ATTACK ) ) {
			pm->ps->pm_flags &= ~PMF_KI_ATTACK;
			pm->ps->weaponstate = WEAPON_READY;
		}
		break;
	// BFP - NOTE: That happens when the player uses a quick ki explosion themself or a homing ki ball is being triggered
	case WEAPON_EXPLODING_KIBALLFIRING:
		if ( pm->ps->weaponTime <= 0 ) {
			pm->ps->pm_flags &= ~PMF_KI_ATTACK;
			pm->ps->weaponstate = WEAPON_READY;
			pm->ps->weaponTime = 0;
		}
		break;
	// BFP - NOTE: The dividing ki ball is triggering until pressing the attack key again after holded or changing weapon
	case WEAPON_DIVIDINGKIBALLFIRING:
		if ( pm->cmd.buttons & BUTTON_ATTACK ) {
			pm->ps->pm_flags &= ~PMF_KI_ATTACK;
			pm->ps->weaponstate = WEAPON_READY;
			pm->ps->weaponTime += 100;
		}
		break;
	// BFP - NOTE: That ki explosion wave is triggering until stop pressing the attack key or changing weapon,
	// also when stopped enters in WEAPON_STUN state in 1 sec
	case WEAPON_KIEXPLOSIONWAVE:
		if ( pm->ps->weaponTime <= 0 ) {
			if ( pm->ps->stats[STAT_KI_ATTACK_CHARGE] >= 1 ) {
				pm->ps->pm_flags |= PMF_KI_ATTACK;
				PM_AddEvent( EV_FIRE_WEAPON );
			}
		}
		if ( !( pm->cmd.buttons & BUTTON_ATTACK )
		|| ( pm->cmd.buttons & BUTTON_MELEE ) ) {
			pm->ps->weaponTime = 1000;
			pm->ps->weaponstate = WEAPON_STUN;
		}

		// fall even whether the player is flying
		pm->ps->velocity[2] -= pm->ps->gravity * 2 * pml.frametime;
		break;
	// BFP - NOTE: This stun state makes the player can't move in 1 sec, it's different from "hit stun"
	case WEAPON_STUN:
		if ( pm->ps->weaponTime <= 0 ) {
			pm->ps->weaponTime = 0;
			pm->ps->weaponstate = WEAPON_READY;
		}
		pm->ps->stats[STAT_KI_ATTACK_CHARGE] = 0;

		// fall even whether the player is flying
		pm->ps->velocity[2] -= pm->ps->gravity * 2 * pml.frametime;
	}
}
#undef FIRE_CHARGED_STATE

/*
================
PM_Animate
================
*/

static void PM_Animate( void ) {
	if ( pm->cmd.buttons & BUTTON_GESTURE ) {
		if ( pm->ps->torsoTimer == 0 ) {
			PM_StartTorsoAnim( TORSO_GESTURE );
			pm->ps->torsoTimer = TIMER_GESTURE;
			PM_AddEvent( EV_TAUNT );
		}
	}
}


/*
================
PM_DropTimers
================
*/
static void PM_DropTimers( void ) {
	// drop misc timing counter
	if ( pm->ps->pm_time ) {
		if ( pml.msec >= pm->ps->pm_time ) {
			// BFP - No handling PMF_ALL_TIMES
			// pm->ps->pm_flags &= ~PMF_ALL_TIMES;
			pm->ps->pm_time = 0;
		} else {
			pm->ps->pm_time -= pml.msec;
		}
	}

	// drop animation counter
	if ( pm->ps->legsTimer > 0 ) {
		pm->ps->legsTimer -= pml.msec;
		if ( pm->ps->legsTimer < 0 ) {
			pm->ps->legsTimer = 0;
		}
	}

	if ( pm->ps->torsoTimer > 0 ) {
		pm->ps->torsoTimer -= pml.msec;
		if ( pm->ps->torsoTimer < 0 ) {
			pm->ps->torsoTimer = 0;
		}
	}
}

/*
================
PM_UpdateViewAngles

This can be used as another entry point when only the viewangles
are being updated isntead of a full move
================
*/
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd ) {
	short		temp;
	int		i;

	if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPINTERMISSION) {
		return;		// no view changes at all
	}

	if ( ps->pm_type != PM_SPECTATOR && ps->stats[STAT_HEALTH] <= 0 ) {
		return;		// no view changes at all
	}

	// circularly clamp the angles with deltas
	for (i=0 ; i<3 ; i++) {
		temp = cmd->angles[i] + ps->delta_angles[i];
		if ( i == PITCH && ps->powerups[PW_FLIGHT] <= 0 ) { // BFP - Avoid that when flying
			// don't let the player look up or down more than 90 degrees
			if ( temp > 16000 ) {
				ps->delta_angles[i] = 16000 - cmd->angles[i];
				temp = 16000;
			} else if ( temp < -16000 ) {
				ps->delta_angles[i] = -16000 - cmd->angles[i];
				temp = -16000;
			}
		}
		ps->viewangles[i] = SHORT2ANGLE(temp);
	}
}

/*
================
PM_EnableFlight

Enables/disables flight
================
*/
static qboolean PM_EnableFlight( void ) { // BFP - Flight

	// BFP - Hit stun, avoid enabling flight if the player is in this status
	if ( pm->ps->pm_flags & PMF_HITSTUN ) {
		return qfalse;
	}

	if ( pm->ps->powerups[PW_FLIGHT] <= 0 ) {
		return qfalse;
	}

	// Handle the PMF flag if it's already flying
	if ( pm->ps->powerups[PW_FLIGHT] > 0 && !( pm->ps->pm_flags & PMF_FALLING ) ) {
		return qtrue;
	}

	// do not proceed to the jump event while enables the flight in the charging status
	if ( ( pm->ps->pm_flags & PMF_KI_CHARGE ) && pm->ps->powerups[PW_FLIGHT] > 0 ) {
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		return qfalse;
	}

	return qtrue;
}

/*
================
PM_KiCharge

Charges ki
================
*/
static void PM_KiCharge( void ) { // BFP - Ki Charge

	// BFP - Hit stun, avoid charging if the player is in this status
	if ( pm->ps->pm_flags & PMF_HITSTUN ) {
		return;
	}

	// BFP - Ki explosion wave and stun after using it, avoid charging also
	if ( pm->ps->weaponstate == WEAPON_KIEXPLOSIONWAVE
	|| pm->ps->weaponstate == WEAPON_STUN ) {
		return;
	}

	pm->cmd.forwardmove = pm->cmd.rightmove = pm->cmd.upmove = 0;

	if ( pm->cmd.buttons & ( BUTTON_ATTACK | BUTTON_KI_USE | BUTTON_MELEE | BUTTON_BLOCK | BUTTON_ENABLEFLIGHT ) ) {
		pm->cmd.buttons &= ~( BUTTON_ATTACK | BUTTON_KI_USE | BUTTON_MELEE | BUTTON_BLOCK | BUTTON_ENABLEFLIGHT );
	}

	if ( pm->ps->powerups[PW_FLIGHT] <= 0 ) {
		// Don't move from the position when falling
		if ( !( pml.groundTrace.contents & MASK_PLAYERSOLID ) 
		&& pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
			pm->ps->velocity[0] = 0;
			pm->ps->velocity[1] = 0;
		}
		pm->ps->pm_flags |= PMF_FALLING; // Handle PMF_FALLING flag
	}

	if ( ( pm->ps->pm_flags & PMF_KI_CHARGE ) && pm->ps->pm_time <= 0 ) {
		pm->ps->stats[STAT_KI]++;
	}
}

/*
================
PM_HitStun

Receives hit stun
================
*/
static void PM_HitStun( void ) { // BFP - Hit stun

	if ( pm->cmd.buttons & ( BUTTON_MELEE | BUTTON_KI_USE | BUTTON_BLOCK | BUTTON_ENABLEFLIGHT ) ) {
		pm->cmd.buttons &= ~( BUTTON_MELEE | BUTTON_KI_USE | BUTTON_BLOCK | BUTTON_ENABLEFLIGHT );
	}

	pm->ps->powerups[PW_FLIGHT] = 0;
	pm->ps->powerups[PW_HASTE] = 0;
	pm->ps->pm_flags &= ~PMF_KI_ATTACK;
	// don't display shot effects on the stunned status
	pm->ps->eFlags &= ~EF_FIRING;
	pm->ps->eFlags &= ~EF_AURA;
	pm->ps->weaponstate = WEAPON_READY;

	// BFP - NOTE: BFP doesn't handle nothing the button directions when there's hit stun
	pm->cmd.upmove = 0;
}

/*
================
PmoveSingle

================
*/
void trap_SnapVector( float *v );

void PmoveSingle (pmove_t *pmove) {
	pm = pmove;

	// this counter lets us debug movement problems with a journal
	// by setting a conditional breakpoint fot the previous frame
	c_pmove++;

	// clear results
	pm->numtouch = 0;
	pm->watertype = 0;
	pm->waterlevel = 0;

	if ( pm->ps->stats[STAT_HEALTH] <= 0 ) {
		pm->tracemask &= ~CONTENTS_BODY;	// corpses can fly through bodies
	}

	// make sure walking button is clear if they are running, to avoid
	// proxy no-footsteps cheats
	if ( abs( pm->cmd.forwardmove ) > 64 || abs( pm->cmd.rightmove ) > 64 ) {
		pm->cmd.buttons &= ~BUTTON_WALKING;
	}

	// set the talk balloon flag
	if ( pm->cmd.buttons & BUTTON_TALK ) {
		pm->ps->eFlags |= EF_TALK;
	} else {
		pm->ps->eFlags &= ~EF_TALK;
	}

	// BFP - Handling the PMF flag when stepping the ground
	if ( pm->ps->pm_flags & PMF_RESPAWNED ) {
		pm->ps->pm_flags |= PMF_FALLING;
	}

	// set the firing flag for continuous beam weapons
	if ( !(pm->ps->pm_flags & PMF_RESPAWNED) && pm->ps->pm_type != PM_INTERMISSION
		&& ( pm->cmd.buttons & BUTTON_ATTACK ) && pm->ps->ammo[ pm->ps->weapon ] ) {
		pm->ps->eFlags |= EF_FIRING;
	} else {
		pm->ps->eFlags &= ~EF_FIRING;
	}

	// clear the respawned flag if attack and use are cleared
	if ( pm->ps->stats[STAT_HEALTH] > 0 && 
		!( pm->cmd.buttons & (BUTTON_ATTACK | BUTTON_USE_HOLDABLE) ) ) {
		pm->ps->pm_flags &= ~PMF_RESPAWNED;
	}

	// if talk button is down, dissallow all other input
	// this is to prevent any possible intercept proxy from
	// adding fake talk balloons
	if ( pmove->cmd.buttons & BUTTON_TALK ) {
		// keep the talk button set tho for when the cmd.serverTime > 66 msec
		// and the same cmd is used multiple times in Pmove
		pmove->cmd.buttons = BUTTON_TALK;
		pmove->cmd.forwardmove = 0;
		pmove->cmd.rightmove = 0;
		pmove->cmd.upmove = 0;
	}

	// clear all pmove local vars
	memset (&pml, 0, sizeof(pml));

	// determine the time
	pml.msec = pmove->cmd.serverTime - pm->ps->commandTime;
	if ( pml.msec < 1 ) {
		pml.msec = 1;
	} else if ( pml.msec > 200 ) {
		pml.msec = 200;
	}
	pm->ps->commandTime = pmove->cmd.serverTime;

	// save old org in case we get stuck
	VectorCopy (pm->ps->origin, pml.previous_origin);

	// save old velocity for crashlanding
	VectorCopy (pm->ps->velocity, pml.previous_velocity);

	pml.frametime = pml.msec * 0.001;

	// update the viewangles
	PM_UpdateViewAngles( pm->ps, &pm->cmd );

	AngleVectors (pm->ps->viewangles, pml.forward, pml.right, pml.up);

	if ( pm->cmd.upmove < 10 ) {
		// not holding jump
		pm->ps->pm_flags &= ~PMF_JUMP_HELD;
	}

	// BFP - No handling PMF_BACKWARDS_RUN
#if 0
	// decide if backpedaling animations should be used
	if ( pm->cmd.forwardmove < 0 ) {
		pm->ps->pm_flags |= PMF_BACKWARDS_RUN;
	} else if ( pm->cmd.forwardmove > 0 || ( pm->cmd.forwardmove == 0 && pm->cmd.rightmove ) ) {
		pm->ps->pm_flags &= ~PMF_BACKWARDS_RUN;
	}
#endif

	if ( pm->ps->pm_type >= PM_DEAD ) {

		// BFP - If player is dead, disable the following statuses
		pm->ps->powerups[PW_FLIGHT] = 0;
		pm->ps->powerups[PW_HASTE] = 0;
		pm->ps->eFlags &= ~EF_AURA;

// BFP - NOTE: disabled for notes, don't allow pressing these buttons
#if 0
		pm->cmd.buttons &= ~BUTTON_KI_CHARGE;
		pm->cmd.buttons &= ~BUTTON_KI_USE;
#endif

		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
		pm->cmd.upmove = 0;
	}

	// BFP - Ki Charge
	if ( ( pmove->cmd.buttons & BUTTON_KI_CHARGE ) 
		&& !( pmove->cmd.buttons & BUTTON_KI_USE )
		&& pm->ps->pm_type != PM_DEAD
		&& pm->ps->pm_type != PM_SPECTATOR ) {
		PM_KiCharge();
	}

	// BFP - Hit stun
	if ( ( pm->ps->pm_flags & PMF_HITSTUN ) 
		&& pm->ps->pm_type != PM_DEAD
		&& pm->ps->pm_type != PM_SPECTATOR ) {
		PM_HitStun();
	}

	if ( pm->ps->pm_type == PM_SPECTATOR ) {
		PM_CheckDuck ();
		PM_FlyMove ();
		PM_DropTimers ();
		return;
	}

	if ( pm->ps->pm_type == PM_NOCLIP ) {
		PM_NoclipMove ();
		PM_DropTimers ();
		return;
	}

	if (pm->ps->pm_type == PM_FREEZE) {
		return;		// no movement at all
	}

	if ( pm->ps->pm_type == PM_INTERMISSION || pm->ps->pm_type == PM_SPINTERMISSION) {
		return;		// no movement at all
	}

	// set watertype, and waterlevel
	PM_SetWaterLevel();
	pml.previous_waterlevel = pmove->waterlevel;

	// set mins, maxs, and viewheight
	PM_CheckDuck ();

	// set groundentity
	PM_GroundTrace();

	if ( pm->ps->pm_type == PM_DEAD ) {
		PM_DeadMove ();
	}

	// BFP - Melee
	PM_Melee();

	// BFP - Ki explosion wave handling
	PM_KiExplosionWave();

	PM_DropTimers();

	// BFP - Flight
	if ( PM_EnableFlight() ) {
		// flight powerup doesn't allow jump and has different friction
		PM_FlyMove();
	}
// BFP - no hook
#if 0
	else if (pm->ps->pm_flags & PMF_GRAPPLE_PULL) {
		PM_GrappleMove();
		// We can wiggle a bit
		PM_AirMove();
	}
#endif
	// BFP - No handling PMF_TIME_WATERJUMP
#if 0
	else if (pm->ps->pm_flags & PMF_TIME_WATERJUMP) {
		PM_WaterJumpMove();
	}
#endif
	else if ( pm->waterlevel > 1 ) {
		// swimming
		PM_WaterMove();
	} else if ( pml.walking ) {
		// walking on ground
		PM_WalkMove();
	} else {
		// airborne
		PM_AirMove();
	}

	PM_Animate();

	// set groundentity, watertype, and waterlevel
	PM_GroundTrace();
	PM_SetWaterLevel();

	// weapons
	PM_Weapon();

	// torso animation
	PM_TorsoAnimation();

	// BFP - Flight animation
	PM_FlightAnimation();

	// BFP - Ki Charge animation
	PM_KiChargeAnimation();

	// BFP - Hit stun animation
	PM_HitStunAnimation();

	// footstep events / legs animations
	PM_Footsteps();

	// entering / leaving water splashes
	PM_WaterEvents();

	// snap some parts of playerstate to save network bandwidth
	trap_SnapVector( pm->ps->velocity );
}


/*
================
Pmove

Can be called by either the server or the client
================
*/
void Pmove (pmove_t *pmove) {
	int			finalTime;

	finalTime = pmove->cmd.serverTime;

	if ( finalTime < pmove->ps->commandTime ) {
		return;	// should not happen
	}

	if ( finalTime > pmove->ps->commandTime + 1000 ) {
		pmove->ps->commandTime = finalTime - 1000;
	}

	pmove->ps->pmove_framecount = (pmove->ps->pmove_framecount+1) & ((1<<PS_PMOVEFRAMECOUNTBITS)-1);

	// chop the move up if it is too long, to prevent framerate
	// dependent behavior
	while ( pmove->ps->commandTime != finalTime ) {
		int		msec;

		msec = finalTime - pmove->ps->commandTime;

		if ( pmove->pmove_fixed ) {
			if ( msec > pmove->pmove_msec ) {
				msec = pmove->pmove_msec;
			}
		}
		else {
			if ( msec > 66 ) {
				msec = 66;
			}
		}
		pmove->cmd.serverTime = pmove->ps->commandTime + msec;
		PmoveSingle( pmove );

		if ( pmove->ps->pm_flags & PMF_JUMP_HELD ) {
			pmove->cmd.upmove = 20;
		}
	}
}

// BFP - Undefine the macros
#undef KI_ATTACK_TORSO_ANIM_HANDLING
#undef TORSOSTATUS_ANIM_HANDLING
#undef FORCEJUMP_ANIM_HANDLING
#undef CONTINUEFLY_ANIM_HANDLING
#undef CONTINUEMELEESTRIKE_LEGS_ANIM_HANDLING
#undef SLOPES_NEARGROUND_ANIM_HANDLING

