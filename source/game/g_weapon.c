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
// g_weapon.c 
// perform the server side effects of a weapon firing

#include "g_local.h"

static	float	s_quadFactor;
static	vec3_t	forward, right, up;
static	vec3_t	muzzle;

/*
================
G_BounceProjectile
================
*/
void G_BounceProjectile( vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout ) {
	vec3_t v, newv;
	float dot;

	VectorSubtract( impact, start, v );
	dot = DotProduct( v, dir );
	VectorMA( v, -2*dot, dir, newv );

	VectorNormalize(newv);
	VectorMA(impact, 8192, newv, endout);
}


/*
======================================================================

GAUNTLET

======================================================================
*/

// BFP - TODO: That's just a test for gauntlet. 
// It might modify if weapon config is going to be implemented

// check for the hit-scan gauntlet, don't let the action
// go through as an attack unless it actually hits something
void Weapon_Gauntlet( gentity_t *ent ) {
	CheckKiShockwavePushAttack( ent );
}

// BFP - TODO: There's a ki attack that acts like a shockwave to push opponents, 
// gauntlet attack can be modified adding punch attack distance and pushing opponents

/*
===============
CheckKiShockwavePushAttack
===============
*/
qboolean CheckKiShockwavePushAttack( gentity_t *ent ) {
	trace_t		tr;
	vec3_t		end;
	// gentity_t	*tent;
	gentity_t	*traceEnt;
	// BFP - bfp_weapon.cfg: damage, range ...
	int			damage = 8;
	int			range = 500;

	// BFP - TODO: For weapon config, set this as hitscan attack type conditional
	// if ( Q_stricmp( ent->classname, "hitscan" ) ) {
	//	return qfalse;
	// }

	// BFP - TODO: For weapon config, apply the logic of weaponTime
	if ( ent->client->ps.weaponTime < 100 ) {
		return qfalse;
	}

	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, right, up);

	CalcMuzzlePoint ( ent, forward, right, up, muzzle );

	VectorMA (muzzle, range, forward, end);

	// BFP - Reflective
	ent->reflective = qtrue;

	trap_Trace (&tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT);
	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		return qfalse;
	}

	// BFP - Reflective
	G_Reflective( ent, muzzle, end );

	traceEnt = &g_entities[ tr.entityNum ];

	// BFP - No EV_MISSILE_HIT here
#if 0
	// send blood impact
	if ( traceEnt->takedamage && traceEnt->client ) {
		tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
		tent->s.otherEntityNum = traceEnt->s.number;
		tent->s.eventParm = DirToByte( tr.plane.normal );
		tent->s.weapon = ent->s.weapon;
	}
#endif

	if ( !traceEnt->takedamage) {
		return qfalse;
	}

	// BFP - No PW_QUAD damage calculation here
#if 0
	if (ent->client->ps.powerups[PW_QUAD] ) {
		G_AddEvent( ent, EV_POWERUP_QUAD, 0 );
		s_quadFactor = g_quadfactor.value;
	} else {
		s_quadFactor = 1;
	}

	damage = 50 * s_quadFactor;
#endif
	G_Damage( traceEnt, ent, ent, forward, tr.endpos,
		damage, 0, MOD_GAUNTLET );

	return qtrue;
}

/*
===================
GetEntityNearMeleeRadius
===================
*/
gentity_t *GetEntityNearMeleeRadius( vec3_t point, gentity_t *attacker, gentity_t *target ) { // BFP - Melee near radius detection
	// BFP - NOTE: Maybe there's an idea that could be detected for any entity than client,
	// imagine a player who wants to use melee against a rock and break it.
	// So, that would be MAX_GENTITIES and trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );
	// to trace entities that doesn't have ->client data and just ->takedamage and ->health
	// like func_breakable ones
	int			i, num, touch[MAX_CLIENTS];
	vec3_t		mins, maxs;
	gentity_t	*prevTarget = target;

	// if already exists, don't apply calculation detection
	if ( prevTarget->client ) {
		return prevTarget;
	}

	VectorAdd( point, attacker->r.mins, mins );
	VectorAdd( point, attacker->r.maxs, maxs );
	num = G_EntitiesInBox( mins, maxs, touch, level.maxclients );

	for ( i = 0; i < num; i++ ) {
		target = &g_entities[ touch[i] ];
		if ( target->client && target->client != attacker->client ) {
			return target;
		}
	}

	return prevTarget;
}

/*
===============
CheckMeleeAttack
===============
*/
qboolean CheckMeleeAttack( gentity_t *attacker ) { // BFP - Melee
	trace_t		tr;
	vec3_t		end;
	gentity_t	*traceTarget;
	vec3_t		traceMins, traceMaxs;
	float		diveRange = g_meleeDiveRange.integer;

	// BFP - Monster gamemode, adjust dive range for player monster
	if ( attacker->client->ps.eFlags & EF_MONSTER ) {
		diveRange *= 2.5;
	}

	// set aiming directions
	AngleVectors( attacker->client->ps.viewangles, forward, NULL, NULL );
	CalcMuzzlePoint( attacker, forward, NULL, NULL, muzzle );
	VectorMA( muzzle, diveRange, forward, end );

	// BFP - Monster gamemode, use scaled bounding box for player monster
	if ( attacker->client->ps.eFlags & EF_MONSTER ) {
		// scale down the trace box for monsters to allow teleportation through tighter spaces
		// use 60% of monster size to avoid getting stuck
		VectorScale( attacker->r.mins, 0.6, traceMins );
		VectorScale( attacker->r.maxs, 0.6, traceMaxs );
	} else {
		VectorCopy( attacker->r.mins, traceMins );
		VectorCopy( attacker->r.maxs, traceMaxs );
	}

	// that part is where the target can be detected when the attacker is on air, if not, the trace will be different
	trap_Trace( &tr, muzzle, traceMins, traceMaxs, end, attacker->s.number, MASK_SHOT );
	if ( attacker->client->ps.groundEntityNum != ENTITYNUM_NONE ) {
		trap_Trace( &tr, muzzle, NULL, NULL, end, attacker->s.number, MASK_SHOT );
	}

	// when the attacker is very near from the target, continue attacking if that happens
	traceTarget = GetEntityNearMeleeRadius( muzzle, attacker, &g_entities[ tr.entityNum ] );

	// avoid entity null references
	if ( !traceTarget || traceTarget == NULL ) {
		attacker->client->ps.pm_flags &= ~PMF_MELEE;
		return qfalse;
	}

	// stop melee if there's no entity
	if ( !traceTarget->takedamage ) {
		attacker->client->ps.pm_flags &= ~PMF_MELEE;
		return qfalse;
	}

	// avoid if the entity isn't a player (e.g. a breakable map entity)
	if ( traceTarget->s.eType != ET_PLAYER ) {
		attacker->client->ps.pm_flags &= ~PMF_MELEE;
		return qfalse;
	}

	// the target's corpse is starting to sink, avoid interacting with a sinking corpse, nothing special happens
	if ( traceTarget->physicsObject ) {
		attacker->client->ps.pm_flags &= ~PMF_MELEE;
		return qfalse;
	}

	// BFP - NOTE: Apply g_friendlyFire for melee, originally in BFP, friendly fire wasn't never applied
	// stop melee if the entity is in the same team
	if ( OnSameTeam( traceTarget, attacker ) ) {
		attacker->client->ps.pm_flags &= ~PMF_MELEE;
		return qfalse;
	}

	// ENTITY DETECTED!
	if ( traceTarget->client ) {
		gentity_t	*target;
		vec3_t		direction;
		float		distance;
		// BFP - Melee range, it isn't known why, but it's the approximation
		float		rangeMultiplier = g_meleeRange.integer + 45;

		// BFP - Monster gamemode, adjust range multiplier for player monster
		if ( attacker->client->ps.eFlags & EF_MONSTER ) {
			rangeMultiplier *= 2.5;
		}

		VectorSubtract( traceTarget->client->ps.origin, attacker->client->ps.origin, direction );
		distance = VectorLength( direction );

		// distance from the attacker and the target, more range = attacker can attack at that distance without being teleported
		// the distance needs to be greater than 25, otherwise it won't respect the lengths around the target
		if ( distance >= rangeMultiplier && distance >= 25 ) {
			// trace only when the player is alive
			if ( traceTarget->client->ps.pm_type != PM_DEAD ) {
				trap_Trace( &tr, muzzle, traceMins, traceMaxs, end, attacker->s.number, MASK_PLAYERSOLID );
			}

			// if the target is very near to some brush (solid or surface with no impact) from the map
			// avoid the attacker teleporting there, otherwise gets stuck
			if ( tr.startsolid || tr.allsolid ) {
				attacker->client->ps.pm_flags &= ~PMF_MELEE;
				return qfalse;
			}

			// if the target position is being covered under something solid (e.g. a brush from the map), 
			// avoid the attacker teleporting there, otherwise gets stuck
			target = &g_entities[ tr.entityNum ];
			if ( target && target->client // avoids DLL/SO crash
			&& target->client->ps.pm_type != PM_DEAD && !target->takedamage ) {
				attacker->client->ps.pm_flags &= ~PMF_MELEE;
				return qfalse;
			}

			// try to trace when the target is dead, that's what BFP originally did, and teleport near the corpse
			if ( traceTarget->client->ps.pm_type == PM_DEAD ) {
				trap_Trace( &tr, muzzle, traceMins, traceMaxs, end, attacker->s.number, MASK_PLAYERSOLID );
			}

			// TELEPORT!
			if ( target && target->client // avoids DLL/SO crash
			&& attacker->client->ps.origin[2] == target->client->ps.origin[2] ) {
				tr.endpos[2] = target->client->ps.origin[2];
			}

			// BFP - Monster gamemode, for player monster, adjust teleport position to account for larger bbox
			if ( attacker->client->ps.eFlags & EF_MONSTER ) {
				vec3_t adjustedPos, pullBack;
				VectorCopy( tr.endpos, adjustedPos );
				
				// pull back slightly from target to ensure monster fits
				VectorSubtract( attacker->client->ps.origin, traceTarget->client->ps.origin, pullBack );
				VectorNormalize( pullBack );
				VectorMA( adjustedPos, 30.0f, pullBack, adjustedPos );

				// final check that monster fits at this position
				trap_Trace( &tr, adjustedPos, attacker->r.mins, attacker->r.maxs, adjustedPos, attacker->s.number, MASK_PLAYERSOLID );
				if ( !tr.startsolid && !tr.allsolid ) {
					VectorCopy( adjustedPos, attacker->client->ps.origin );
				} else { // can't fit, abort melee
					attacker->client->ps.pm_flags &= ~PMF_MELEE;
					return qfalse;
				}
			} else {
				VectorCopy( tr.endpos, attacker->client->ps.origin );
			}
		}

		// BFP - Don't deal damage on warmup
		if ( level.time < level.warmupTime ) {
			// act as if the player is attacking
			return qtrue;
		}

		// PUSH AND DEAL DAMAGE!
		if ( g_meleeDamage.integer > 0 ) {
			// consume 5% of ki when being defended and apply knockback
			if ( ( traceTarget->client->ps.pm_flags & PMF_BLOCK ) && traceTarget->client->blockKnockbackTime <= 0 ) {
				traceTarget->client->ps.ammo[WP_KI] -= traceTarget->client->ps.stats[STAT_MAX_KI] * 0.05;
				traceTarget->client->blockKnockbackTime = level.time + 250;
			}
			G_Damage ( traceTarget, attacker, attacker, direction, forward, 
				g_meleeDamage.integer, 0, MOD_MELEE );
			if ( g_meleeDamage.integer < 4 ) { // lose altitude while flying, when damage is lesser than 4
				traceTarget->client->ps.velocity[2] -= 300;
			}
			if ( g_meleeDamage.integer < 30 ) {
				traceTarget->client->ps.pm_time = 200;
			}
			traceTarget->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}
	}

	// if the attacker is using ki boost, stun the target! (g_hitStun enabled only)
	if ( g_hitStun.integer >= 1 
	&& ( attacker->client->ps.eFlags & EF_KI_BOOST ) 
	&& traceTarget->client->ps.stats[STAT_HITSTUN_TIME] <= 0
	&& !( traceTarget->client->ps.pm_flags & PMF_BLOCK ) 
	&& attacker->client->hitStunMeleeDelayTime <= 0 ) {
		// add 3 seconds to the hitstun when there's no delay
		traceTarget->client->ps.stats[STAT_HITSTUN_TIME] = 3000;
		attacker->client->hitStunMeleeDelayTime = level.time + 6000;
	}

	return qtrue;
}

/*
===============
ApplyMuzzleOffsets
===============
*/
static void ApplyMuzzleOffsets( gentity_t *ent, float randXOffset, float randYOffset, float alternatingXOffset ) { // BFP - Muzzle offsets
	// randXOffset, randYOffset and alternatingXOffset
	if ( randXOffset > 0 ) {
		VectorMA( muzzle, crandom() * randXOffset, right, muzzle );
	}
	if ( randYOffset > 0 ) {
		VectorMA( muzzle, crandom() * randYOffset, up, muzzle );
	}

	if ( alternatingXOffset > 0 ) {
		float	side = ( ent->client->alternatingOffsetSide ) ? alternatingXOffset : -alternatingXOffset;
		VectorMA( muzzle, side, right, muzzle );
		ent->client->alternatingOffsetSide = !ent->client->alternatingOffsetSide;
	}
}

/*
===============
ApplyConeOfFire
===============
*/
static void ApplyConeOfFire( float coneOfFireX, float coneOfFireY ) { // BFP - Cone of fire
	if ( coneOfFireX <= 0 && coneOfFireY <= 0 ) {
		return;
	}
	if ( coneOfFireX > 0 ) {
		VectorMA( forward, crandom() * coneOfFireX * 0.001f, right, forward );
	}
	if ( coneOfFireY > 0 ) {
		VectorMA( forward, crandom() * coneOfFireY * 0.001f, up, forward );
	}
	if ( coneOfFireX > 0 || coneOfFireY > 0 ) {
		VectorNormalize( forward );
	}
}

/*
======================================================================

MACHINEGUN

======================================================================
*/

/*
======================
SnapVectorTowards

Round a vector to integers for more efficient network
transmission, but make sure that it rounds towards a given point
rather than blindly truncating.  This prevents it from truncating 
into a wall.
======================
*/
void SnapVectorTowards( vec3_t v, vec3_t to ) {
	int		i;

	for ( i = 0 ; i < 3 ; i++ ) {
		if ( to[i] <= v[i] ) {
			v[i] = (int)v[i];
		} else {
			v[i] = (int)v[i] + 1;
		}
	}
}

#define MACHINEGUN_SPREAD	200
#define	MACHINEGUN_DAMAGE	7
#define	MACHINEGUN_TEAM_DAMAGE	5		// wimpier MG in teamplay

void Bullet_Fire (gentity_t *ent, float spread, int damage ) {
	trace_t		tr;
	vec3_t		end;
	float		r;
	float		u;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	int			passent;

	// BFP - bfp_weapon.cfg: randYOffset, randXOffset, range
	float		randYOffset = 35;
	float		randXOffset = 35;
	float		range = 1500;

	damage *= s_quadFactor;

	r = crandom() * randXOffset;
	u = crandom() * randYOffset;
	VectorMA (muzzle, range, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);

	passent = ent->s.number;

	trap_Trace (&tr, muzzle, NULL, NULL, end, passent, MASK_SHOT);
	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	// snap the endpos to integers, but nudged towards the line
	SnapVectorTowards( tr.endpos, muzzle );

	// send bullet impact
	if ( traceEnt->takedamage && traceEnt->client ) {
		tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT ); // BFP - Before Q3: EV_BULLET_HIT_FLESH
		tent->s.eventParm = DirToByte( tr.plane.normal ); // BFP - Before Q3: traceEnt->s.number
		if( LogAccuracyHit( traceEnt, ent ) ) {
			ent->client->accuracy_hits++;
		}
	} else {
		tent = G_TempEntity( tr.endpos, EV_MISSILE_MISS ); // BFP - Before Q3: EV_BULLET_HIT_WALL
		tent->s.eventParm = DirToByte( tr.plane.normal );
	}
	tent->s.weapon = ent->s.weapon; // BFP - Sends weapon info to the event
	tent->s.otherEntityNum = ent->s.number;

	if ( traceEnt->takedamage ) {
			G_Damage( traceEnt, ent, ent, forward, tr.endpos,
				damage, 0, MOD_MACHINEGUN);
	}
}


/*
======================================================================

BFG

======================================================================
*/

void BFG_Fire ( gentity_t *ent ) {
	gentity_t	*m;

	// BFP - Homing disk testing
	m = fire_disk( ent, muzzle, forward ); // fire_bfg (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}


// BFP - Forcefield
/*
======================================================================

FORCEFIELD

======================================================================
*/

void Forcefield_Fire ( gentity_t *ent ) { // BFP - Forcefield fire
	if ( !ent->client->hook ) {
		fire_forcefield( ent, muzzle );
	}
}

static void Forcefield_Free( gentity_t *self ) { // BFP - Forcefield free
	self->parent->client->hook = NULL;
	G_FreeEntity( self );
}

void Weapon_Forcefield_Think ( gentity_t *ent ) { // BFP - Forcefield
	gentity_t	*rad = NULL;
	int			damage = ( ent->splashDamage ) ? ent->splashDamage : ent->damage;

	if ( !ent->parent || !ent->parent->client || !ent->parent->client->hook
	|| ent->parent->client->ps.pm_type == PM_DEAD
	|| ent->parent->client->pers.connected == CON_DISCONNECTED ) {
		Forcefield_Free( ent );
		return;
	}

	if ( ent->parent->client->ps.weaponstate != WEAPON_FIRING
	&& ent->parent->client->ps.weaponstate != WEAPON_KIEXPLOSIONWAVE
	&& ent->parent->client->ps.weaponstate != WEAPON_EXPLODING_KIBALLFIRING ) {
		Forcefield_Free( ent );
		return;
	}
	
	VectorCopy( ent->parent->r.currentOrigin, ent->r.currentOrigin );
	VectorCopy( ent->parent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;

	// corrects force field origin
	BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->r.currentOrigin );

	while ( ( rad = FindRadius(rad, ent->r.currentOrigin, ent->radius) ) != NULL ) {
		if ( IsValidTargetRadius( ent, rad ) ) {
			G_Damage( rad, ent, ent->parent, forward, rad->r.currentOrigin, 
				damage, 0, ent->methodOfDeath );
		}
	}

	// no chargeAttack: removes hook pointer at that instant
	if ( ent->parent->client->ps.weaponstate == WEAPON_EXPLODING_KIBALLFIRING ) {
		Forcefield_Free( ent );
		return;
	}
	ent->nextthink = level.time + 200;
}

/*
======================================================================

SHOTGUN

======================================================================
*/

// DEFAULT_SHOTGUN_SPREAD and DEFAULT_SHOTGUN_COUNT	are in bg_public.h, because
// client predicts same spreads
#define	DEFAULT_SHOTGUN_DAMAGE	10

qboolean ShotgunPellet( vec3_t start, vec3_t end, gentity_t *ent ) {
	trace_t		tr;
	int			damage;
	gentity_t	*traceEnt;

	trap_Trace (&tr, start, NULL, NULL, end, ent->s.number, MASK_SHOT);
	traceEnt = &g_entities[ tr.entityNum ];

	// send bullet impact
	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		return qfalse;
	}

	if ( traceEnt->takedamage ) {
		damage = DEFAULT_SHOTGUN_DAMAGE * s_quadFactor;

		G_Damage( traceEnt, ent, ent, forward, tr.endpos,
			damage, 0, MOD_SHOTGUN );
		if( LogAccuracyHit( traceEnt, ent ) ) {
			return qtrue;
		}
	}
	return qfalse;
}

// this should match CG_ShotgunPattern
void ShotgunPattern( vec3_t origin, vec3_t origin2, int seed, gentity_t *ent ) {
	int			i;
	float		r, u;
	vec3_t		end;
	vec3_t		forward, right, up;
	// int			oldScore;
	qboolean	hitClient = qfalse;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( origin2, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	// oldScore = ent->client->ps.persistant[PERS_SCORE];

	// generate the "random" spread pattern
	for ( i = 0 ; i < DEFAULT_SHOTGUN_COUNT ; i++ ) {
		r = Q_crandom( &seed ) * DEFAULT_SHOTGUN_SPREAD * 16;
		u = Q_crandom( &seed ) * DEFAULT_SHOTGUN_SPREAD * 16;
		VectorMA( origin, 8192 * 16, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);
		if( ShotgunPellet( origin, end, ent ) && !hitClient ) {
			hitClient = qtrue;
			ent->client->accuracy_hits++;
		}
	}
}


void weapon_supershotgun_fire (gentity_t *ent) {
	gentity_t		*tent;

	// send shotgun blast
	tent = G_TempEntity( muzzle, EV_SHOTGUN );
	VectorScale( forward, 4096, tent->s.origin2 );
	SnapVector( tent->s.origin2 );
	tent->s.eventParm = rand() & 255;		// seed for spread pattern
	tent->s.otherEntityNum = ent->s.number;

	ShotgunPattern( tent->s.pos.trBase, tent->s.origin2, tent->s.eventParm, ent );
}


/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

void weapon_grenadelauncher_fire (gentity_t *ent) {
	gentity_t	*m;

	// extra vertical velocity
	//forward[2] += 0.2f;
	//VectorNormalize( forward );

	ApplyMuzzleOffsets( ent, 7, 25, 15 ); // BFP - Muzzle offsets
	ApplyConeOfFire( 40, 50 ); // BFP - Cone of fire
	m = fire_grenade (ent, muzzle, forward);

	// BFP - If the projectile muzzle offset landed within solid or out of bounds geometry,
	// then remove
	{
		trace_t	tr;
		trap_Trace( &tr, m->s.pos.trBase, m->r.mins, m->r.maxs, muzzle, m->s.number, MASK_SOLID );
		if ( tr.fraction < 1.0f || ( tr.surfaceFlags & SURF_NOIMPACT ) ) {
			G_FreeEntity( m );
		}
	}

	//m->damage *= s_quadFactor;
	//m->splashDamage *= s_quadFactor;

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}

/*
======================================================================

ROCKET

======================================================================
*/

void Weapon_RocketLauncher_Fire (gentity_t *ent) {
	gentity_t	*m;

	m = fire_rocket (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}


/*
======================================================================

PLASMA GUN

======================================================================
*/

void Weapon_Plasmagun_Fire (gentity_t *ent) {
	gentity_t	*m;

	m = fire_plasma (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}

/*
======================================================================

RAILGUN

======================================================================
*/


/*
=================
weapon_railgun_fire
=================
*/
void weapon_railgun_fire (gentity_t *ent) {
	vec3_t		end;
	trace_t		trace;
	gentity_t	*tent = NULL;
	gentity_t	*traceEnt = NULL;
	int			damage;
	int			hits;
	int			passent;
	// BFP - For splash damage
	int			splashRadius = 120;

	damage = 100 * s_quadFactor;

	VectorMA (muzzle, 8192, forward, end);

	// trace only against the solids, so the railgun will go through people
	hits = 0;
	passent = ent->s.number;
	trap_Trace( &trace, muzzle, NULL, NULL, end, passent, MASK_SHOT );
	if ( trace.entityNum < ENTITYNUM_MAX_NORMAL ) {
		traceEnt = &g_entities[ trace.entityNum ];
		if ( traceEnt && traceEnt->takedamage ) {
			// BFP - Railgun events are also treated as a missile
			tent = G_TempEntity( trace.endpos, EV_MISSILE_HIT );
		}
	}

	// the final trace endpos will be the terminal point of the rail trail

	// snap the endpos to integers to save net bandwidth, but nudged towards the line
	SnapVectorTowards( trace.endpos, muzzle );

	// BFP - Moved EV_MISSILE_MISS there
	// no explosion at end if SURF_NOIMPACT, but still make the trail
	if ( !( trace.surfaceFlags & SURF_NOIMPACT ) ) {
		// BFP - Railgun events are also treated as a missile
		tent = G_TempEntity( trace.endpos, EV_MISSILE_MISS );
		tent->s.eventParm = DirToByte( trace.plane.normal ); // BFP - Sends dir vector variable to the event
	}

	// send railgun beam effect
	tent = G_TempEntity( trace.endpos, EV_RAILTRAIL );

	// set player number for custom colors on the railtrail
	tent->s.clientNum = ent->s.clientNum;

	VectorCopy( muzzle, tent->s.origin2 );
	// move origin a bit to come closer to the drawn gun muzzle
	VectorMA( tent->s.origin2, 4, right, tent->s.origin2 );
	VectorMA( tent->s.origin2, -1, up, tent->s.origin2 );

	// BFP - Finger beam splash damage
	if ( G_RadiusDamage( ent, trace.endpos, ent, damage, splashRadius, 0, MOD_RAILGUN ) ) {
		hits++;
	}

	// give the shooter a reward sound if they have made two railgun hits in a row
	if ( hits == 0 ) {
		// complete miss
		ent->client->accurateCount = 0;
	} else {
		// check for "impressive" reward sound
		ent->client->accurateCount += hits;
		if ( ent->client->accurateCount >= 2 ) {
			ent->client->accurateCount -= 2;
			// BFP - No impressive counter
			// ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
			// add the sprite over the player's head
			// BFP - No impressive, gauntlet, defend, assist and cap medals
			ent->client->ps.eFlags &= ~(EF_AWARD_EXCELLENT /*| EF_AWARD_IMPRESSIVE | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP*/ );
			// ent->client->ps.eFlags |= EF_AWARD_IMPRESSIVE;
			ent->client->rewardTime = level.time + REWARD_SPRITE_TIME;
		}
		ent->client->accuracy_hits++;
	}

}

// BFP - no hook
#if 0
/*
======================================================================

GRAPPLING HOOK

======================================================================
*/

void Weapon_GrapplingHook_Fire (gentity_t *ent)
{
	if (!ent->client->fireHeld && !ent->client->hook)
		fire_grapple (ent, muzzle, forward);

	ent->client->fireHeld = qtrue;
}

void Weapon_HookFree (gentity_t *ent)
{
	ent->parent->client->hook = NULL;
	ent->parent->client->ps.pm_flags &= ~PMF_GRAPPLE_PULL;
	G_FreeEntity( ent );
}

void Weapon_HookThink (gentity_t *ent)
{
	if (ent->enemy) {
		vec3_t v, oldorigin;

		VectorCopy(ent->r.currentOrigin, oldorigin);
		v[0] = ent->enemy->r.currentOrigin[0] + (ent->enemy->r.mins[0] + ent->enemy->r.maxs[0]) * 0.5;
		v[1] = ent->enemy->r.currentOrigin[1] + (ent->enemy->r.mins[1] + ent->enemy->r.maxs[1]) * 0.5;
		v[2] = ent->enemy->r.currentOrigin[2] + (ent->enemy->r.mins[2] + ent->enemy->r.maxs[2]) * 0.5;
		SnapVectorTowards( v, oldorigin );	// save net bandwidth

		G_SetOrigin( ent, v );
	}

	VectorCopy( ent->r.currentOrigin, ent->parent->client->ps.grapplePoint);
}
#endif

// BFP - BFP Beam attack
/*
======================================================================

BFP BEAM

======================================================================
*/

void Weapon_BFPBeam_Fire ( gentity_t *ent ) // BFP - BFP Beam fire
{
	if ( !ent->client->fireHeld && !ent->client->hook )
		fire_bfpbeam ( ent, muzzle, forward );

	ent->client->fireHeld = qtrue;
}

void Weapon_BFPBeamFree ( gentity_t *ent ) // BFP - BFP Beam free
{
	if ( ent && ent->parent && ent->parent->client ) {
		ent->parent->client->hook = NULL;
		ent->parent->client->ps.weaponstate = WEAPON_READY;
		ent->parent->client->ps.stats[STAT_KI_ATTACK_CHARGE] = 0; // BFP - Reset ki charge points
		ent->parent->client->fireHeld = qfalse;
	}
	G_FreeEntity( ent );
}

static void Weapon_BFPBeam_SetDistance( gentity_t *beam, float distance, vec3_t beamViewPos ) { // BFP - Set BFP beam distance
	vec3_t		newPos;
	float		correctBeamDist;

	if ( !beam->parent->client ) {
		return;
	}

	AngleVectors( beam->parent->client->ps.viewangles, forward, NULL, NULL );
	CalcMuzzlePoint( beam->parent, forward, NULL, NULL, muzzle );

	VectorMA( muzzle, distance, forward, newPos );

	VectorCopy( newPos, beam->r.currentOrigin );
	VectorCopy( newPos, beam->s.pos.trBase );
	VectorScale( forward, beam->speed, newPos );
	VectorCopy( newPos, beam->s.pos.trDelta );
	beam->distance = distance;

	correctBeamDist = Distance( beam->r.currentOrigin, beamViewPos );
	if ( correctBeamDist != distance ) {
		VectorClear( beam->s.pos.trDelta );
	}
	beam->s.pos.trTime = level.time;
}

static qboolean Weapon_BFPBeamStruggle( gentity_t *ent, vec3_t ownerViewPos, float ownerDist ) { // BFP - BFP Beam struggle
	gentity_t	*target = NULL;
	gentity_t	*rad = NULL;
	vec3_t		raddir, targetViewPos;
	float		distTarget, powerEnt, powerTarget;
	const float	BEAM_PUSH_STEP = 200.0f;
	float		struggleRadius = ( ent->splashRadius > ent->radius ) ? ent->splashRadius : ent->radius;

	while ( ( rad = FindRadius(rad, ent->r.currentOrigin, struggleRadius) ) != NULL ) {
		if ( rad && ( rad->r.ownerNum == ent->r.ownerNum 
		|| rad->s.eType != ET_MISSILE ) ) { // projectiles only
			continue;
		}
		if ( rad && rad != ent ) {
			// BFP - If there's a piercing projectile, break it
			trace_t		trace;
			trap_Trace( &trace, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, 
						ent->r.ownerNum, ent->clipmask );
			if ( ent->piercing && !rad->piercing ) {
				G_MissileImpact( rad, &trace );
				continue;
			}
			if ( rad->piercing && !ent->piercing ) {
				G_MissileImpact( ent, &trace );
				G_FreeEntity( ent );
				return qtrue;
			}

			// BFP - If it's a splitting ki ball, break and split!
			if ( !Q_stricmp( rad->classname, "rdmissile" ) && !rad->splitKiBall ) {
				G_RDMissile( rad, rad->parent->client );
				continue;
			}

			// BFP - If it isn't a beam, only other weapon classname like missile, break it
			if ( Q_stricmp( rad->classname, "beam" ) ) {
				G_MissileImpact( rad, &trace );
				continue;
			}

			if ( !Q_stricmp( rad->classname, "beam" ) ) {
				VectorSubtract( rad->r.currentOrigin, ent->r.currentOrigin, raddir );
				distTarget = VectorLength( raddir );
				if ( target == NULL ) {
					target = rad;
				}
			}
		}
	}

	if ( !target ) {
		return qfalse;
	}

	if ( !ent->parent || !ent->parent->client || !target->parent || !target->parent->client ) {
		return qfalse;
	}

	// you're in a struggle, don't move!
	ent->parent->client->ps.weaponstate = WEAPON_BEAMSTRUGGLE;
	target->parent->client->ps.weaponstate = WEAPON_BEAMSTRUGGLE;

	// calculate power using ki charge points
	powerEnt = ent->kiChargePoints;
	powerTarget = target->kiChargePoints;
	if ( ( ent->parent->client->ps.eFlags & EF_KI_BOOST )
	|| ( ent->parent->client->pers.cmd.buttons & BUTTON_KI_USE ) ) {
		powerEnt *= 2;
	}
	if ( ( target->parent->client->ps.eFlags & EF_KI_BOOST )
	|| ( target->parent->client->pers.cmd.buttons & BUTTON_KI_USE ) ) {
		powerTarget *= 2;
	}

	// get beam target distance
	VectorCopy( target->parent->client->ps.origin, targetViewPos );
	targetViewPos[2] += target->parent->client->ps.viewheight;
	VectorSubtract( target->r.currentOrigin, targetViewPos, raddir );
	distTarget = VectorLength( raddir );

	if ( powerEnt > powerTarget ) {
		// target beam loses distance
		Weapon_BFPBeam_SetDistance( target, distTarget - BEAM_PUSH_STEP, targetViewPos );
	} else if ( powerTarget > powerEnt ) {
		// owner beam loses distance
		Weapon_BFPBeam_SetDistance( ent, ownerDist - BEAM_PUSH_STEP, ownerViewPos );
	} else {
		// both beams lose distance
		Weapon_BFPBeam_SetDistance( ent, ownerDist - BEAM_PUSH_STEP, ownerViewPos );
		Weapon_BFPBeam_SetDistance( target, distTarget - BEAM_PUSH_STEP, targetViewPos );
	}

	// visual effect
	VectorAdd( ent->r.currentOrigin, target->r.currentOrigin, targetViewPos );
	VectorScale( targetViewPos, 0.5f, targetViewPos );
	{
		gentity_t	*effect = G_TempEntity( targetViewPos, EV_BEAM_STRUGGLE );
		effect->s.otherEntityNum = ent->s.number;
		effect->s.otherEntityNum2 = target->s.number;
	}
	return qtrue;
}

static qboolean Weapon_BFPBeamTrace ( gentity_t *ent, vec3_t origin ) { // BFP - Beam trace
	trace_t		trace;

	trap_Trace( &trace, origin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, 
				ent->r.ownerNum, MASK_SHOT | MASK_SOLID );

	if ( trace.surfaceFlags & SURF_NOIMPACT ) {
		Weapon_BFPBeamFree( ent );
		return qtrue;
	}

	if ( trace.fraction < 1.0 ) {
		VectorCopy( trace.endpos, ent->r.currentOrigin );
		G_MissileImpact( ent, &trace );
		return qtrue;
	}
	return qfalse;
}

void Weapon_BFPBeamRun ( gentity_t *ent ) // BFP - BFP Beam run
{
	vec3_t		ownerViewPos, vel, dir;
	float		distance, deltaTime;

	if ( Q_stricmp( ent->classname, "beam" ) ) {
		return;
	}

	VectorCopy( ent->parent->client->ps.origin, ownerViewPos );
	ownerViewPos[2] += ent->parent->client->ps.viewheight;
	AngleVectors( ent->parent->client->ps.viewangles, forward, NULL, NULL );

	CalcMuzzlePoint( ent->parent, forward, NULL, NULL, muzzle );

	VectorSubtract( ent->r.currentOrigin, ownerViewPos, dir );

	// to fix timescale < 1 issues
	deltaTime = ( level.time - ent->deltaTime ) * 0.001f;
	ent->deltaTime = level.time;
	distance = ent->distance + ent->speed * deltaTime;
	ent->distance = distance;

	VectorMA( muzzle, distance, forward, ent->s.pos.trBase );
	distance = VectorLength( dir );
	if ( !Weapon_BFPBeamStruggle( ent, ownerViewPos, distance ) ) {
		VectorScale( forward, ent->speed, vel );
		VectorCopy( vel, ent->s.pos.trDelta );
	}
	if ( !ent->inuse ) { // if already disappeared
		return;
	}
	ent->s.pos.trTime = level.time;		// smooth it

	BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->r.currentOrigin );

	if ( Weapon_BFPBeamTrace( ent, ownerViewPos ) ) {
		return;
	}
	Weapon_BFPBeamTrace( ent, ent->r.currentOrigin );
}

// BFP - sbeam (Super Beam?)
/*
======================================================================

SBEAM

======================================================================
*/

void Weapon_SBeam_Fire ( gentity_t *ent ) // BFP - sbeam (Super Beam?) fire
{
	if ( !ent->client->fireHeld && !ent->client->hook )
		fire_sbeam ( ent, muzzle, forward );

	ent->client->fireHeld = qtrue;
}

static qboolean Weapon_SBeamRadius( gentity_t *ent ) { // BFP - sbeam (Super Beam?) radius
	gentity_t	*rad = NULL;
	float		radius = ( ent->splashRadius > ent->radius ) ? ent->splashRadius : ent->radius;

	if ( !ent->parent || !ent->parent->client ) {
		return qtrue;
	}

	while ( ( rad = FindRadius(rad, ent->r.currentOrigin, radius) ) != NULL ) {
		if ( rad && ( rad->r.ownerNum == ent->r.ownerNum
		|| rad->s.eType != ET_MISSILE ) ) { // projectiles only
			continue;
		}
		if ( rad && rad != ent ) {
			// BFP - If there's a piercing projectile, break it
			trace_t		trace;
			trap_Trace( &trace, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, 
						ent->r.ownerNum, ent->clipmask );
			if ( ent->piercing && !rad->piercing ) {
				G_MissileImpact( rad, &trace );
				continue;
			}
			if ( rad->piercing && !ent->piercing ) {
				G_MissileImpact( ent, &trace );
				return qtrue;
			}

			// BFP - If it's a splitting ki ball, break and split!
			if ( !Q_stricmp( rad->classname, "rdmissile" ) && !rad->splitKiBall ) {
				G_RDMissile( rad, rad->parent->client );
				continue;
			}

			// BFP - If it isn't a sbeam nor a beam, only other weapon classname like missile, break it
			if ( Q_stricmp( rad->classname, "sbeam" ) && Q_stricmp( rad->classname, "beam" ) ) {
				G_MissileImpact( rad, &trace );
				continue;
			}

			if ( !Q_stricmp( rad->classname, "sbeam" ) ) {
				G_MissileImpact( rad, &trace );
				G_MissileImpact( ent, &trace );
				return qtrue;
			}
		}
	}
	return qfalse;
}

void Weapon_SBeam_Run ( gentity_t *ent ) // BFP - sbeam (Super Beam?) run
{
	vec3_t		ownerViewPos, vel;

	if ( Q_stricmp( ent->classname, "sbeam" ) ) {
		return;
	}

	VectorCopy( ent->parent->client->ps.origin, ownerViewPos );
	ownerViewPos[2] += ent->parent->client->ps.viewheight;
	AngleVectors( ent->parent->client->ps.viewangles, forward, NULL, NULL );

	CalcMuzzlePoint( ent->parent, forward, NULL, NULL, muzzle );

	VectorScale( forward, ent->speed, vel );
	VectorCopy( vel, ent->s.pos.trDelta );
	VectorCopy( ownerViewPos, ent->s.pos.trBase );

	BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->r.currentOrigin );

	if ( Weapon_BFPBeamTrace( ent, ownerViewPos ) ) {
		return;
	}
	if ( Weapon_SBeamRadius( ent ) ) {
		return;
	}
	Weapon_BFPBeamTrace( ent, ent->r.currentOrigin );
}

/*
======================================================================

LIGHTNING GUN

======================================================================
*/

void Weapon_LightningFire( gentity_t *ent ) {
	trace_t		tr;
	vec3_t		end;
	gentity_t	*traceEnt, *tent;
	int			damage, passent;
	// BFP - bfp_weapon.cfg: range
	float		range = LIGHTNING_RANGE;

	damage = 8 * s_quadFactor;

	passent = ent->s.number;
	VectorMA( muzzle, range, forward, end );

	trap_Trace( &tr, muzzle, NULL, NULL, end, passent, MASK_SHOT );

	if ( tr.surfaceFlags & SURF_NOIMPACT /*tr.entityNum == ENTITYNUM_NONE*/ ) {
		return;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	if ( traceEnt->takedamage) {
		G_Damage( traceEnt, ent, ent, forward, tr.endpos,
			damage, 0, MOD_LIGHTNING);
	}

	if ( traceEnt->takedamage && traceEnt->client ) {
		tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
		tent->s.otherEntityNum = traceEnt->s.number;
		tent->s.eventParm = DirToByte( tr.plane.normal );
		tent->s.weapon = ent->s.weapon;
		if( LogAccuracyHit( traceEnt, ent ) ) {
			ent->client->accuracy_hits++;
		}
		return;
	}

	// BFP - That random handles avoiding the particle spam with lightning gun type
	if ( random() < 0.3 ) {
		tent = G_TempEntity( tr.endpos, EV_MISSILE_MISS );
		tent->s.eventParm = DirToByte( tr.plane.normal );
		tent->s.weapon = ent->s.weapon; // BFP - Sends weapon info to the event
	}
}

//======================================================================


/*
===============
LogAccuracyHit
===============
*/
qboolean LogAccuracyHit( gentity_t *target, gentity_t *attacker ) {
	if( !target->takedamage ) {
		return qfalse;
	}

	if ( target == attacker ) {
		return qfalse;
	}

	if( !target->client ) {
		return qfalse;
	}

	if( !attacker->client ) {
		return qfalse;
	}

	if( target->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return qfalse;
	}

	if ( OnSameTeam( target, attacker ) ) {
		return qfalse;
	}

	return qtrue;
}


/*
===============
CalcMuzzlePoint

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePoint ( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) {
	VectorCopy( ent->s.pos.trBase, muzzlePoint );
	muzzlePoint[2] += ent->client->ps.viewheight;
	VectorMA( muzzlePoint, 14, forward, muzzlePoint );
	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector( muzzlePoint );
}

/*
===============
CalcMuzzlePointOrigin

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePointOrigin ( gentity_t *ent, vec3_t origin, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) {
	VectorCopy( ent->s.pos.trBase, muzzlePoint );
	muzzlePoint[2] += ent->client->ps.viewheight;
	VectorMA( muzzlePoint, 14, forward, muzzlePoint );
	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector( muzzlePoint );
}



/*
===============
FireWeapon
===============
*/
void FireWeapon( gentity_t *ent ) {
	if (ent->client->ps.powerups[PW_QUAD] ) {
		s_quadFactor = g_quadfactor.value;
	} else {
		s_quadFactor = 1;
	}

	// track shots taken for accuracy tracking.  Grapple is not a weapon and gauntet is just not tracked
	if( ent->s.weapon != WP_GRAPPLING_HOOK && ent->s.weapon != WP_GAUNTLET ) {
		ent->client->accuracy_shots++;
	}

	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, right, up);

	CalcMuzzlePointOrigin ( ent, ent->client->oldOrigin, forward, right, up, muzzle );

	// fire the specific weapon
	switch( ent->s.weapon ) {
	case WP_GAUNTLET:
		Weapon_Gauntlet( ent );
		break;
	case WP_LIGHTNING:
		Weapon_LightningFire( ent );
		break;
// BFP - Using that as forcefield
	case WP_SHOTGUN:
		Forcefield_Fire( ent );
		break;
	case WP_MACHINEGUN:
		if ( g_gametype.integer != GT_TEAM ) {
			Bullet_Fire( ent, MACHINEGUN_SPREAD, MACHINEGUN_DAMAGE );
		} else {
			Bullet_Fire( ent, MACHINEGUN_SPREAD, MACHINEGUN_TEAM_DAMAGE );
		}
		break;
	case WP_GRENADE_LAUNCHER:
		weapon_grenadelauncher_fire( ent );
		break;
	case WP_ROCKET_LAUNCHER:
		Weapon_RocketLauncher_Fire( ent );
		break;
	case WP_PLASMAGUN:
		Weapon_Plasmagun_Fire( ent );
		break;
	case WP_RAILGUN:
		weapon_railgun_fire( ent );
		break;
	case WP_BFG:
		BFG_Fire( ent );
		break;
// BFP - Using that as BFP Beam
	case WP_GRAPPLING_HOOK:
		//Weapon_SBeam_Fire( ent );
		Weapon_BFPBeam_Fire( ent );
		break;
	default:
// FIXME		G_Error( "Bad ent->s.weapon" );
		break;
	}
}
