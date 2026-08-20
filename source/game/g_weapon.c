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
	// BFP - Melee dive range
	float		diveRange = ( g_meleeRange.integer > g_meleeDiveRange.integer ) ? g_meleeRange.integer : g_meleeDiveRange.integer;

	if ( !attacker || !attacker->client ) {
		return qfalse;
	}

	if ( diveRange < 0 ) {
		diveRange = 0;
	}
	// it isn't known why, but it's the approximation
	diveRange += 15;

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
		// BFP - Melee range
		float		rangeMultiplier = g_meleeRange.integer;
		if ( rangeMultiplier < 0 ) {
			rangeMultiplier = 0;
		}
		// it isn't known why, but it's the approximation
		rangeMultiplier += 45;

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
				traceTarget->client->ps.stats[STAT_KI] -= traceTarget->client->ps.stats[STAT_MAX_KI] * 0.05;
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

	if ( alternatingXOffset > 0 && ent->parent ) {
		float	side = ( ent->parent->alternatingOffsetSide ) ? alternatingXOffset : -alternatingXOffset;
		VectorMA( muzzle, side, right, muzzle );
		ent->parent->alternatingOffsetSide = !ent->parent->alternatingOffsetSide;
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
================
G_BFPFireProjectileWeapon

Fires missile, rdmissile, beam, sbeam or forcefield attack
================
*/
gentity_t *G_BFPFireProjectileWeapon( gentity_t *self, vec3_t start, vec3_t dir, bfpWeapon_t *wpCfg ) { // BFP - Fire BFP weapon
	gentity_t	*proj;
	float		r = wpCfg->radius;

	if ( !self || !self->client ) {
		return NULL;
	}

	// if forcefield is still being used, skip
	if ( wpCfg->attackType == ATK_FORCEFIELD ) {
		if ( self->client->hook ) {
			if ( !self->client->hook->inuse ) {
				self->client->hook = NULL;
			} else {
				return NULL;
			}
		}
	}

	proj = G_Spawn();

	// weird classname set here, there are words like "missile" and "homing0" too,
	// although it doesn't know what logic was handled here
	proj->classname = "bfpbeam";
	proj->s.eType = ET_MISSILE;
	proj->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	proj->r.ownerNum = self->s.number;
	proj->parent = self;
	proj->clipmask = MASK_SHOT;
	proj->target_ent = NULL;
	proj->s.weapon = self->s.weapon;
	proj->s.clientNum = self->client->ps.clientNum;

	proj->damage = wpCfg->damage;
	proj->splashDamage = wpCfg->splashDamage;
	proj->splashRadius = wpCfg->explosionRadius;
	proj->methodOfDeath = MOD_KI_ATTACK;
	proj->splashMethodOfDeath = MOD_KI_ATTACK;

	// ki charge
	proj->kiChargePoints = self->client->ps.generic1;
	proj->s.generic1 = self->client->ps.generic1;

	// homing
	proj->homing = wpCfg->homing;
	proj->homingRange = wpCfg->homingRange;

	// set!
	proj->bfpWeapon = wpCfg;

	if ( wpCfg->attackType == ATK_MISSILE || wpCfg->attackType == ATK_RDMISSILE ) {
		ApplyMuzzleOffsets( proj, wpCfg->randXOffset, wpCfg->randYOffset, wpCfg->alternatingXOffset );
		ApplyConeOfFire( wpCfg->coneOfFireX, wpCfg->coneOfFireY );
	}

	if ( wpCfg->chargeAttack || wpCfg->chargeAutoFire ) {
		G_ChargeDamageScaling( proj, r );
	}

	proj->bounces = wpCfg->bounces;

	// trajectory
	if ( wpCfg->missileGravity > 0 ) {
		proj->s.pos.trType = TR_GRAVITY;
	} else {
		proj->s.pos.trType = TR_LINEAR;
	}
	proj->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;
	if ( wpCfg->missileAcceleration > 0 ) { // start from the very first frame if there's acceleration
		proj->s.pos.trTime = level.time;
	}
	VectorCopy( start, proj->s.pos.trBase );
	VectorScale( dir, wpCfg->missileSpeed, proj->s.pos.trDelta );
	SnapVector( proj->s.pos.trDelta );
	VectorCopy( start, proj->r.currentOrigin );

	// collision radius
	VectorSet( proj->r.mins, -r, -r, -r );
	if ( wpCfg->attackType == ATK_MISSILE || wpCfg->attackType == ATK_RDMISSILE ) {
		// set the upward minimum at that height, so the projectile can move through that gap
		VectorSet( proj->r.mins, -r, -r, -10 );
	}
	if ( wpCfg->attackType == ATK_SBEAM ) {
		// put a bit above
		VectorSet( proj->r.mins, -r, -r, -r + 5 );
	}
	VectorSet( proj->r.maxs, r, r, r );

	// for beams
	proj->deltaTime = level.time;
	proj->distance = 0;

	// think adjustments
	switch ( wpCfg->attackType ) {
	case ATK_BEAM:
	case ATK_SBEAM:
		proj->think = Weapon_BFPBeamFree;
		proj->nextthink = level.time + 15000;
		break;
	case ATK_FORCEFIELD:
		proj->think = Weapon_Forcefield_Think;
		proj->nextthink = level.time + 200;
		proj->s.pos.trType = TR_STATIONARY;
		proj->s.pos.trTime = level.time;
		VectorClear( proj->s.pos.trDelta );
		self->client->hook = proj;
		break;
	case ATK_RDMISSILE:
		proj->think = G_DetonateMissile;
		proj->nextthink = level.time + 10000;
		// explosionSpawn has a weaponNum of the projectile to be spawned after splitting
		break;
	default:
		proj->think = G_ExplodeMissile;
		proj->nextthink = level.time + 10000;
		if ( wpCfg->piercing ) {
			proj->think = G_FreeEntity;
		}
	}

	if ( wpCfg->missileDuration > 0 ) {
		proj->nextthink = level.time + wpCfg->missileDuration;
	}

	// rdmissile split
	proj->splitKiBall = qfalse;

	trap_LinkEntity( proj );

	return proj;
}


/*
=================
Weapon_RailTrail_Fire
=================
*/
void Weapon_RailTrail_Fire( gentity_t *ent ) { // BFP - Rail trail fire
	vec3_t		end, mins = {0, 0, 0}, maxs = {0, 0, 0};
	trace_t		trace;
	gentity_t	*tent = NULL, *traceEnt = NULL;
	int			hits;
	int			splashRadius = ent->bfpWeapon->explosionRadius;

	// BFP - Range
	VectorMA( muzzle, ent->bfpWeapon->range, forward, end );

	// trace only against the solids, so the railgun will go through people
	hits = 0;

	// set radius to the hitscan bounding box
	if ( ent->bfpWeapon->radius > 0 ) {
		VectorSet( mins, -ent->bfpWeapon->radius, -ent->bfpWeapon->radius, -ent->bfpWeapon->radius );
		VectorSet( maxs, ent->bfpWeapon->radius, ent->bfpWeapon->radius, ent->bfpWeapon->radius );
	}

	trap_Trace( &trace, muzzle, mins, maxs, end, ent->s.number, 
			( ent->bfpWeapon->piercing ) ? CONTENTS_BODY : MASK_SHOT );
	if ( trace.entityNum < ENTITYNUM_MAX_NORMAL ) {
		traceEnt = &g_entities[ trace.entityNum ];
		if ( traceEnt->takedamage ) {
			if ( LogAccuracyHit( traceEnt, ent ) ) {
				hits++;
			}
			G_Damage( traceEnt, ent, ent->parent, forward, trace.endpos, ent->bfpWeapon->damage, 0, MOD_KI_ATTACK );
		}
	}

	// the final trace endpos will be the terminal point of the rail trail

	// snap the endpos to integers to save net bandwidth, but nudged towards the line
	SnapVectorTowards( trace.endpos, muzzle );

	// no explosion at end if SURF_NOIMPACT, but still make the trail
	if ( !( trace.surfaceFlags & SURF_NOIMPACT ) ) {
		// rail trail events are also treated as a missile
		tent = G_TempEntity( trace.endpos, EV_MISSILE_MISS );
		tent->s.eventParm = DirToByte( trace.plane.normal ); // sends dir vector variable to the event
		tent->s.clientNum = ent->parent->client->ps.clientNum;
		tent->s.generic1 = ent->s.generic1;
		tent->s.weapon = ent->s.weapon;
	}

	// BFP - Reflective
	// BFP - NOTE: Why isn't it applying? It should do it, maybe collision radius detection?
	// In the original BFP, that doesn't apply at all, but it'd be cool to try
	// G_Reflective( ent, qtrue, muzzle );

	// send railgun beam effect
	tent = G_TempEntity( trace.endpos, EV_RAILTRAIL );

	// set player number for custom colors on the railtrail
	tent->s.clientNum = ent->parent->client->ps.clientNum;

	// set ki charge points
	tent->s.generic1 = ent->s.generic1;

	VectorCopy( muzzle, tent->s.origin2 );
	// move origin a bit to come closer to the drawn gun muzzle
	VectorMA( tent->s.origin2, 4, right, tent->s.origin2 );
	VectorMA( tent->s.origin2, -1, up, tent->s.origin2 );

	// splash damage
	if ( ( !traceEnt || !traceEnt->takedamage )
	&& splashRadius > 0 && ent->bfpWeapon->splashDamage > 0
	&& G_RadiusDamage( ent, trace.endpos, ent->parent, ent->bfpWeapon->splashDamage, splashRadius, 0, MOD_KI_ATTACK ) ) {
		hits++;
	}

	// give the shooter a reward sound if they have made two railgun hits in a row
	if ( hits == 0 ) {
		// complete miss
		ent->parent->client->accurateCount = 0;
	} else {
		ent->parent->client->accurateCount += hits;
		if ( ent->parent->client->accurateCount >= 2 ) {
			ent->parent->client->accurateCount -= 2;
			ent->parent->client->ps.eFlags &= ~EF_AWARD_EXCELLENT;
			ent->parent->client->rewardTime = level.time + REWARD_SPRITE_TIME;
		}
		ent->parent->client->accuracy_hits++;
	}
}


/*
================
G_BFPFireHitscanWeapon

Hitscan firing
================
*/
static void G_BFPFireHitscanWeapon( gentity_t *self, bfpWeapon_t *wpCfg ) { // BFP - Fire BFP hitscan attack type weapon
	trace_t		tr;
	vec3_t		end;
	vec3_t		mins = {0, 0, 0}, maxs = {0, 0, 0};
	int			weaponTime;
	gentity_t	*traceEnt;
	gentity_t	*ent = G_Spawn();

	if ( !self || !self->client ) {
		return;
	}

	ent->classname = "bfpbeam";
	ent->r.ownerNum = self->s.number;
	ent->parent = self;
	ent->s.weapon = self->s.weapon;
	ent->s.clientNum = self->client->ps.clientNum;
	ent->damage = wpCfg->damage;
	ent->splashDamage = wpCfg->splashDamage;
	ent->splashRadius = wpCfg->explosionRadius;
	ent->methodOfDeath = MOD_KI_ATTACK;
	ent->splashMethodOfDeath = MOD_KI_ATTACK;

	// ki charge
	ent->kiChargePoints = self->client->ps.generic1;
	ent->s.generic1 = self->client->ps.generic1;

	// set!
	ent->bfpWeapon = wpCfg;

	// apply weaponTime calculation to handle event effects
	weaponTime = wpCfg->weaponTime + wpCfg->randomWeaponTime;

	// rail trail, behaves like a rail gun
	if ( wpCfg->railTrail ) {
		Weapon_RailTrail_Fire( ent );
		return;
	}

	// set aiming directions
	AngleVectors( self->client->ps.viewangles, forward, right, up );
	CalcMuzzlePoint( self, forward, right, up, muzzle );

	ApplyMuzzleOffsets( ent, wpCfg->randXOffset, wpCfg->randYOffset, wpCfg->alternatingXOffset );
	ApplyConeOfFire( wpCfg->coneOfFireX, wpCfg->coneOfFireY );

	VectorMA( muzzle, wpCfg->range, forward, end );

	// set radius to the hitscan bounding box
	if ( wpCfg->radius > 0 ) {
		VectorSet( mins, -wpCfg->radius, -wpCfg->radius, -wpCfg->radius );
		VectorSet( maxs, wpCfg->radius, wpCfg->radius, wpCfg->radius );
	}

	// BFP - Reflective
	G_Reflective( ent, qtrue, muzzle );

	trap_Trace( &tr, muzzle, mins, maxs, end, self->s.number, 
			( wpCfg->piercing ) ? CONTENTS_BODY : MASK_SHOT );
	if ( wpCfg->radius > 0 && ( tr.startsolid || tr.allsolid || tr.entityNum == ENTITYNUM_NONE ) ) {
		vec3_t		boxMins, boxMaxs;
		int			entityList[MAX_GENTITIES];
		int			numEntities, i;
		qboolean	hitAny = qfalse;

		VectorAdd( muzzle, mins, boxMins );
		VectorAdd( muzzle, maxs, boxMaxs );

		numEntities = G_EntitiesInBox( boxMins, boxMaxs, entityList, MAX_GENTITIES );
		for ( i = 0; i < numEntities; i++ ) {
			gentity_t	*other = &g_entities[ entityList[i] ];
			vec3_t		targetOrigin;
			trace_t		visTrace;
			if ( !other->client || other == self ) {
				continue;
			}
			if ( !other->takedamage ) {
				continue;
			}
			if ( other->client->ps.stats[STAT_HEALTH] <= 0 ) {
				continue;
			}
			if ( OnSameTeam( other, self ) && !g_friendlyFire.integer ) {
				continue;
			}

			VectorCopy( other->r.currentOrigin, targetOrigin );
			targetOrigin[2] += ( other->r.mins[2] + other->r.maxs[2] ) * 0.5;

			trap_Trace( &visTrace, muzzle, NULL, NULL, targetOrigin, self->s.number, MASK_SHOT );
			if ( visTrace.fraction < 1.0 && visTrace.entityNum != other->s.number ) {
				continue;
			}

			G_Damage( other, ent, self, forward, visTrace.endpos, wpCfg->damage, 0, MOD_KI_ATTACK );

			if ( LogAccuracyHit( other, self ) ) {
				self->client->accuracy_hits++;
				hitAny = qtrue;
			}
		}

		if ( hitAny ) {
			gentity_t	*tent = G_TempEntity( muzzle, EV_MISSILE_HIT );
			tent->s.clientNum = self->client->ps.clientNum;
			tent->s.generic1 = ent->s.generic1;
			tent->s.otherEntityNum = self->s.number;
			tent->s.eventParm = 0;
			tent->s.weapon = self->s.weapon;
		}
		return;
	}
	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	traceEnt = &g_entities[ tr.entityNum ];
	if ( traceEnt && traceEnt->takedamage && traceEnt->client ) {
		gentity_t	*tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
		tent->s.clientNum = self->client->ps.clientNum;
		tent->s.generic1 = ent->s.generic1;
		tent->s.otherEntityNum = traceEnt->s.number;
		tent->s.eventParm = DirToByte( tr.plane.normal );
		tent->s.weapon = self->s.weapon;
		if( LogAccuracyHit( traceEnt, self ) ) {
			self->client->accuracy_hits++;
		}
	}

	// BFP - That random handles avoiding the particle spam with lesser weaponTime
	if ( ( weaponTime >= 100 || random() < 0.3 ) && !traceEnt->takedamage ) {
		gentity_t	*tent = G_TempEntity( tr.endpos, EV_MISSILE_MISS );
		tent->s.clientNum = self->client->ps.clientNum;
		tent->s.generic1 = ent->s.generic1;
		tent->s.eventParm = DirToByte( tr.plane.normal );
		tent->s.weapon = self->s.weapon; // BFP - Sends weapon info to the event
	}

	if ( traceEnt && traceEnt->takedamage ) {
		G_Damage( traceEnt, ent, self, forward, tr.endpos, wpCfg->damage, 0, MOD_KI_ATTACK );
		return;
	}

	// splash damage
	if ( wpCfg->splashDamage > 0 && wpCfg->explosionRadius > 0 ) {
		G_RadiusDamage( ent, tr.endpos, self, wpCfg->splashDamage, wpCfg->explosionRadius, 0, MOD_KI_ATTACK );
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


// BFP - Forcefield
/*
======================================================================

FORCEFIELD

======================================================================
*/

static void Forcefield_Free( gentity_t *self ) { // BFP - Forcefield free
	if ( self && self->parent && self->parent->client ) {
		self->parent->client->hook = NULL;
		G_FreeEntity( self );
	}
}

void Weapon_Forcefield_Think ( gentity_t *ent ) { // BFP - Forcefield
	gentity_t	*rad = NULL;
	int			damage = ( ent->splashDamage ) ? ent->splashDamage : ent->damage;
	qboolean	chargeAutoFire = ent->bfpWeapon->chargeAutoFire;
	// use weaponTime delay as attack time
	int			weaponTime = ent->bfpWeapon->weaponTime + ent->bfpWeapon->randomWeaponTime;

	if ( !ent->parent || !ent->parent->client || !ent->parent->client->hook
	|| ent->parent->client->ps.pm_type == PM_DEAD
	|| ent->parent->client->pers.connected == CON_DISCONNECTED ) {
		Forcefield_Free( ent );
		return;
	}

	if ( chargeAutoFire && ent->parent->client->ps.weaponstate != WEAPON_FIRING
	&& ent->parent->client->ps.weaponstate != WEAPON_ACTIVE ) {
		Forcefield_Free( ent );
		return;
	}
	
	VectorCopy( ent->parent->r.currentOrigin, ent->r.currentOrigin );
	VectorCopy( ent->parent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;

	// corrects force field origin
	BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->r.currentOrigin );

	while ( ( rad = FindRadius(rad, ent->r.currentOrigin, ent->bfpWeapon->radius) ) != NULL ) {
		if ( IsValidTargetRadius( ent, rad ) ) {
			G_Damage( rad, ent, ent->parent, forward, rad->r.currentOrigin, 
				damage, 0, ent->methodOfDeath );
		}
	}

	// no chargeAutoFire: removes hook pointer at that instant
	if ( !chargeAutoFire ) {
		Forcefield_Free( ent );
		return;
	}
	ent->nextthink = level.time + weaponTime;
}

// BFP - BFP Beam attack
/*
======================================================================

BFP BEAM

======================================================================
*/

void Weapon_BFPBeamFree ( gentity_t *ent ) // BFP - BFP Beam free
{
	if ( ent && ent->parent && ent->parent->client ) {
		ent->parent->client->hook = NULL;
		// BFP - movementPenalty
		if ( ent->bfpWeapon && ent->bfpWeapon->movementPenalty > 0 ) {
			if ( ent->bfpWeapon->movementPenalty > ent->parent->client->ps.weaponTime ) {
				ent->parent->client->ps.weaponTime = ent->bfpWeapon->movementPenalty - ent->parent->client->ps.weaponTime;
			} else {
				ent->parent->client->ps.weaponTime = ent->bfpWeapon->movementPenalty;
			}
			ent->parent->client->ps.weaponstate = WEAPON_STUN;
		} else {
			ent->parent->client->ps.weaponstate = WEAPON_READY;
		}
		ent->parent->client->ps.generic1 = 0; // BFP - Reset ki charge points
		ent->parent->client->fireHeld = qfalse;
	}
	G_FreeEntity( ent );
}

static void Weapon_BFPBeam_SetDistance( gentity_t *beam, float distance, vec3_t beamViewPos ) { // BFP - Set BFP beam distance
	vec3_t		newPos;
	float		correctBeamDist;

	// avoid null exception
	if ( !beam || !beam->parent || !beam->parent->client ) {
		if ( beam ) {
			Weapon_BFPBeamFree( beam );
		}
		return;
	}

	AngleVectors( beam->parent->client->ps.viewangles, forward, NULL, NULL );
	CalcMuzzlePoint( beam->parent, forward, NULL, NULL, muzzle );

	VectorMA( muzzle, distance, forward, newPos );

	VectorCopy( newPos, beam->r.currentOrigin );
	VectorCopy( newPos, beam->s.pos.trBase );
	VectorScale( forward, beam->bfpWeapon->missileSpeed, newPos );
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
	float		struggleRadius = ( ent->splashRadius > ent->bfpWeapon->radius ) ? ent->splashRadius : ent->bfpWeapon->radius;

	// avoid null exception
	if ( !ent || !ent->parent || !ent->parent->client ) {
		return qfalse;
	}

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

			// BFP - Priority (only for non-beam weapons)
			if ( rad->bfpWeapon->attackType != ATK_BEAM ) {
				if ( ent->bfpWeapon->priority > rad->bfpWeapon->priority || rad->bfpWeapon->priority <= 0 ) {
					// BFP - If it's a splitting ki ball, break and split!
					if ( !G_BreakRDMissile( rad ) ) {
						rad->parent->client->ps.weaponstate = WEAPON_READY;
						G_MissileImpact( rad, &trace );
					}
					continue;
				} else if ( ent->bfpWeapon->priority < rad->bfpWeapon->priority ) {
					G_MissileImpact( rad, &trace );
					return qtrue;
				}
			}

			if ( rad->bfpWeapon->attackType == ATK_BEAM ) {
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

	// avoid null exception
	if ( !target->parent || !target->parent->client ) {
		if ( target ) {
			Weapon_BFPBeamFree( target );
		}
		return qfalse;
	}

	if ( !ent->parent || !ent->parent->client || !target->parent || !target->parent->client ) {
		return qfalse;
	}

	// you're in a struggle, don't move!
	ent->parent->client->ps.weaponstate = WEAPON_BEAMSTRUGGLE;
	target->parent->client->ps.weaponstate = WEAPON_BEAMSTRUGGLE;

	// calculate power using calculated damage
	powerEnt = (float)ent->damage;
	powerTarget = (float)target->damage;
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
		gentity_t	*effect = G_TempEntity( targetViewPos, EV_SPARK );
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

	// avoid null exception
	if ( !ent || !ent->parent || !ent->parent->client
	|| !ent->bfpWeapon || ent->bfpWeapon->attackType != ATK_BEAM ) {
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
	distance = ent->distance + ent->bfpWeapon->missileSpeed * deltaTime;
	ent->distance = distance;

	VectorMA( muzzle, distance, forward, ent->s.pos.trBase );
	distance = VectorLength( dir );
	if ( !Weapon_BFPBeamStruggle( ent, ownerViewPos, distance ) ) {
		VectorScale( forward, ent->bfpWeapon->missileSpeed, vel );
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

static qboolean Weapon_SBeamRadius( gentity_t *ent ) { // BFP - sbeam (Super Beam?) radius
	gentity_t	*rad = NULL;
	float		radius = ( ent->splashRadius > ent->bfpWeapon->radius ) ? ent->splashRadius : ent->bfpWeapon->radius;

	if ( !ent || !ent->parent || !ent->parent->client ) {
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

			// BFP - Priority (only for non-beam weapons)
			if ( rad->bfpWeapon && rad->bfpWeapon->attackType != ATK_SBEAM ) {
				if ( ent->bfpWeapon->priority > rad->bfpWeapon->priority ) {
					// BFP - If it's a splitting ki ball, break and split!
					if ( !G_BreakRDMissile( rad ) ) {
						rad->parent->client->ps.weaponstate = WEAPON_READY;
						G_MissileImpact( rad, &trace );
					}
					continue;
				} else if ( ent->bfpWeapon->priority < rad->bfpWeapon->priority ) {
					G_MissileImpact( rad, &trace );
					return qtrue;
				}
			}

			// if one of them has more damage power, the other breaks
			if ( rad->bfpWeapon && rad->bfpWeapon->attackType == ATK_SBEAM ) {
				if ( ent->damage > rad->damage ) {
					G_MissileImpact( ent, &trace );
					continue;
				} else if ( ent->damage < rad->damage ) {
					G_MissileImpact( rad, &trace );
					return qtrue;
				}
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

	if ( !ent || !ent->parent || !ent->parent->client ) {
		return;
	}

	if ( !ent->bfpWeapon || ent->bfpWeapon->attackType != ATK_SBEAM ) {
		return;
	}

	VectorCopy( ent->parent->client->ps.origin, ownerViewPos );
	ownerViewPos[2] += ent->parent->client->ps.viewheight;
	AngleVectors( ent->parent->client->ps.viewangles, forward, NULL, NULL );

	CalcMuzzlePoint( ent->parent, forward, NULL, NULL, muzzle );

	VectorScale( forward, ent->bfpWeapon->missileSpeed, vel );
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
	bfpWeapon_t	*wpCfg;
	int		i, shots;

	if ( !ent || !ent->client ) {
		return;
	}

	wpCfg = BG_GetClientBFPWeaponForSlot( ent->client->ps.clientNum, ent->s.weapon );
	if ( !wpCfg ) {
		wpCfg = BG_SetDefaultBFPWeapon();
	}
	// BFP - Monster gamemode, player monster with g_monster 1 uses its own weapon
	if ( ( ent->client->ps.eFlags & EF_MONSTER ) && g_monster.integer > 0 ) {
		wpCfg = BG_SetMonsterDefaultBFPWeapon();
	}

	if ( !wpCfg ) {
		return;
	}
	shots = ( wpCfg->multishot > 0 ) ? wpCfg->multishot : 1;

	// BFP - No quad powerup damage calculation
#if 0
	if (ent->client->ps.powerups[PW_QUAD] ) {
		s_quadFactor = g_quadfactor.value;
	} else 
#endif
	{
		s_quadFactor = 1;
	}

	// track shots taken for accuracy tracking.  Grapple is not a weapon and gauntet is just not tracked
	if ( ( wpCfg->attackType != ATK_BEAM && wpCfg->attackType != ATK_SBEAM && wpCfg->attackType != ATK_HITSCAN )
	|| ( wpCfg->attackType == ATK_HITSCAN && wpCfg->railTrail ) ) {
		ent->client->accuracy_shots++;
	}

	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, right, up);
	CalcMuzzlePointOrigin ( ent, ent->client->oldOrigin, forward, right, up, muzzle );

	switch ( wpCfg->attackType ) {
	case ATK_MISSILE:
	case ATK_RDMISSILE:
		for ( i = 0; i < shots; i++ ) {
			G_BFPFireProjectileWeapon( ent, muzzle, forward, wpCfg );
		}
		break;
	case ATK_BEAM:
	case ATK_SBEAM:
	case ATK_FORCEFIELD:
		G_BFPFireProjectileWeapon( ent, muzzle, forward, wpCfg );
		break;
	case ATK_HITSCAN:
		G_BFPFireHitscanWeapon( ent, wpCfg );
		break;
	default:
		break;
	}
}
