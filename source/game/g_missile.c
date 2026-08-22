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
================
G_SplitProjectile_Fire
================
*/
static void G_SplitProjectile_Fire( gentity_t *ent, vec3_t start, vec3_t dir, int explosionSpawn ) { // BFP - rdmissile: split projectile now!
	gentity_t		*m;
	// BFP - explosionSpawn sets to split the projectiles, then if that happens,
	// goes to the weaponNum of this and fires there x number of splitted projectiles
	// explosionSpawn is a weaponNum of the attack to set
	bfpWeapon_t		*wpCfg;

	wpCfg = BG_FindBFPBFPWeapon( explosionSpawn );
	if ( !wpCfg ) {
		wpCfg = BG_SetDefaultBFPWeapon();
	}
	if ( !wpCfg ) {
		return;
	}

	if ( wpCfg->attackType == ATK_MISSILE ) {
		m = G_BFPFireProjectileWeapon( ent, start, dir, wpCfg );
		m->splitKiBall = qtrue; // handle splitted ki ball, otherwise crashes (in DLL/SO)
		// that part doesn't have -10 in the down minimum, because needs to be like a sphere collision
		VectorSet( m->r.mins, -wpCfg->radius, -wpCfg->radius, -wpCfg->radius );
		VectorSet( m->r.maxs, wpCfg->radius, wpCfg->radius, wpCfg->radius );
	
		//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
	}
}

/*
================
G_HandleRDMissile
================
*/
static void G_HandleRDMissile( gentity_t *ent, gclient_t *client ) { // BFP - rdmissile: splitting projectile, when pressing the attack key again, splits by the number of balls depending on the ki attack charge points had
	vec3_t	dir, angles;
	int		i, projectiles_to_spawn = 3;
	int		splitKiChargePoints; // adjust ki charge points to split

	if ( !ent->bfpWeapon ) {
		return;
	}

	splitKiChargePoints = ent->kiChargePoints - ent->bfpWeapon->minCharge;
	if ( ent->bfpWeapon->maxCharge > ent->bfpWeapon->minCharge && ent->kiChargePoints >= ent->bfpWeapon->maxCharge ) {
		--splitKiChargePoints;
	}

	// BFP - NOTE: That makes us to understand how the projectile is being split by x projectiles
	/* 
	int		yawAdjustments[6] = {
		// if charge attack is 2:
		// horizontal:
		// ←   →
		//   ↓
		-270, -360, -90,
		// if charge attack is 3:
		// horizontal:
		//   ↑
		// ←   →
		//   ↓
		-180,
		0, 0
	};
	int		pitchAdjustments[6] = {
		0, 0, 0, 0,
		// if charge attack is 4:
		// horizontal:
		//   ↑
		// ←   →
		//   ↓
		// vertical: ↑
		-90,
		// if charge attack is 5 and above:
		// horizontal:
		//   ↑
		// ←   →
		//   ↓
		// vertical: ↑ and ↓
		90
	};
	*/

	// determine the number of projectiles to spawn based on the ki attack charge points
	switch( splitKiChargePoints ) {
	case 0:
		projectiles_to_spawn = 3;
		break;
	case 1:
		projectiles_to_spawn = 4;
		break;
	case 2:
		projectiles_to_spawn = 5;
		break;
	case 3:
	case 4:
	case 5:
		projectiles_to_spawn = 6;
		break;
	default:
		return;
	}

	{
		// this is for the new spawning projectiles, 
		// so, the owner is protected from projectile collisions and 
		// only can be damaged by explosion
		vec3_t		forward, right, up;
		gentity_t *owner = NULL;
		float		radius = ent->r.maxs[0];
		if ( ent->r.ownerNum < MAX_CLIENTS && ent->r.ownerNum >= 0 ) {
			owner = &g_entities[ent->r.ownerNum];
		}
		
		if ( !owner || !owner->client ) {
			owner = &g_entities[client->ps.clientNum];
		}

		VectorCopy( ent->s.angles, angles );
		AngleVectors( angles, forward, right, up );
		// correct up vector
		up[0] = up[1] = 0;
		up[2] = 1;

		for ( i = 0; i < projectiles_to_spawn; ++i ) {
			vec3_t	origin;
			trace_t	tr;
			VectorCopy( ent->r.currentOrigin, origin );

			// a trace to correct the projectile that goes up
			VectorCopy( origin, tr.endpos );
			tr.endpos[2] -= 128;
			trap_Trace( &tr, origin, ent->r.mins, ent->r.maxs, tr.endpos, ent->r.ownerNum, MASK_SOLID );

			switch ( i ) {
			case 0: // forward
				VectorCopy( forward, dir );
				break;
			case 1: // left
				VectorNegate( right, dir );
				break;
			case 2: // right
				VectorCopy( right, dir );
				break;
			case 3: // backward
				VectorNegate( forward, dir );
				break;
			case 4: // up
				if ( tr.fraction < 1.0f && tr.entityNum != ENTITYNUM_NONE ) {
					origin[2] = tr.endpos[2] + radius + 1;
				}
				VectorCopy( up, dir );
				break;
			case 5: // down
				VectorNegate( up, dir );
			}
			VectorNormalize( dir );
			G_SplitProjectile_Fire( owner, origin, dir, ent->bfpWeapon->explosionSpawn );
		}
	}
}

/*
================
G_RDMissile
================
*/
void G_RDMissile( gentity_t *ent, gclient_t *client ) { // BFP - rdmissile (Splitting ki ball)
	G_HandleRDMissile( ent, client );
	ent->splitKiBall = qtrue;
	G_FreeEntity( ent );
}

/*
===================
G_BreakRDMissile
===================
*/
qboolean G_BreakRDMissile( gentity_t* ent ) { // BFP - Break and split the ki ball
	if ( ent && ent->bfpWeapon && ent->bfpWeapon->attackType == ATK_RDMISSILE && !ent->splitKiBall ) {
		G_RDMissile( ent, ent->parent->client );
		return qtrue;
	}
	return qfalse;
}

/*
================
G_BounceMissile

================
*/
void G_BounceMissile( gentity_t *ent, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

	// BFP - Replaced to bounces instead using EF_BOUNCE_HALF eFlag
	if ( ent && ent->bfpWeapon && ent->bounces ) {
		if ( ent && !ent->client && ent->bfpWeapon->bounceFriction > 0 ) {
			VectorScale( ent->s.pos.trDelta, ent->bfpWeapon->bounceFriction, ent->s.pos.trDelta );
		}

		// BFP - noZBounce: fix the vertical component to a deterministic bounce height
		// but... what the hell BFP dev was thinking for this behavior here? That isn't a float... 
		if ( ent->bfpWeapon->noZBounce && trace->plane.normal[2] > 0.2 ) {
			ent->s.pos.trDelta[2] = 325.0f;
			if ( ent->bfpWeapon->missileGravity > 0 ) {
				ent->s.pos.trDelta[2] = 1000;
			}
		}

		// check for stop
		if ( trace->plane.normal[2] > 0.2 && VectorLength( ent->s.pos.trDelta ) < 40 ) {
			G_SetOrigin( ent, trace->endpos );
			return;
		}
	}

	VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;
}


/*
================
G_ExplodeMissile

Explode a missile without an impact
================
*/
void G_ExplodeMissile( gentity_t *ent ) {
	vec3_t		dir;
	vec3_t		origin;

	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
	SnapVector( origin );
	G_SetOrigin( ent, origin );

	// we don't have a valid direction, so just point straight up
	dir[0] = dir[1] = 0;
	dir[2] = 1;

	// BFP - Put again to keep the ki charge points
	ent->s.generic1 = ent->kiChargePoints;

	ent->s.eType = ET_GENERAL;
	G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( dir ) );

	ent->freeAfterEvent = qtrue;

	// splash damage
	if ( ent->splashDamage ) {
		if( G_RadiusDamage( ent, ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent
			, ent->splashMethodOfDeath ) ) {
			g_entities[ent->r.ownerNum].client->accuracy_hits++;
		}
	}

	trap_LinkEntity( ent );
}


/*
================
G_DetonateMissile

Detonate a missile
================
*/
void G_DetonateMissile( gentity_t *ent ) {	// BFP - Detonate a missile, for nextthink
	gentity_t *tempEnt = G_TempEntity( ent->r.currentOrigin, EV_MISSILE_DETONATE );
	tempEnt->s.otherEntityNum  = ent->s.number;
	tempEnt->s.generic1 = ent->kiChargePoints;
	tempEnt->s.clientNum = ent->parent->client->ps.clientNum;
	tempEnt->s.weapon = ent->s.weapon;

	// BFP - That's for ki charge points, make it lesser than 0
	ent->s.generic1 = 0;

	// BFP - Splitting ki ball
	if ( ent->parent && ent->parent->client
	&& ent->bfpWeapon && ent->bfpWeapon->attackType == ATK_RDMISSILE
	&& ent->parent->client->ps.weapon == ent->s.weapon
	&& ent->s.weapon == ent->parent->s.weapon && !ent->splitKiBall ) {
		ent->parent->client->ps.weaponstate = WEAPON_READY;
		G_RDMissile( ent, ent->parent->client );
	}

	// splash damage
	if ( ent->splashDamage ) {
		if ( G_RadiusDamage( ent, ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent
			, ent->splashMethodOfDeath ) ) {
			g_entities[ent->r.ownerNum].client->accuracy_hits++;
		}
	}
}


/*
================
G_CollideDetonationCheck

Checks the projectile collision radius and detonation point
================
*/
static void G_CollideDetonationCheck( gentity_t *ent, trace_t *trace ) { // BFP - Detonation check
	vec3_t impactPoint;
	// BFP - NOTE: This setup solves the issue of real impact crack mark for collision radius, but original BFP didn't that (uses -1)
	// but that doesn't fix the issue, because the dynamic light won't be shown if it isn't -1.
	// So, let's leave as -1 by default
	float distToPlane = -1; // DotProduct( trace->endpos, trace->plane.normal ) - trace->plane.dist;

	// if there's a mover or a corpse going to be gibbed, change the impact direction, otherwise the explosion will appear very down or under the target
	gentity_t *target = &g_entities[ trace->entityNum ];
	if ( target
	&& ( ( target->client && target->client->ps.pm_type == PM_DEAD )
	|| target->s.eType == ET_MOVER ) ) {
		distToPlane = 1;
	}

	VectorMA( trace->endpos, -distToPlane, trace->plane.normal, impactPoint );

	if ( trace->contents & MASK_PLAYERSOLID ) {
		gentity_t *te = G_TempEntity( impactPoint, EV_MISSILE_MISS );
		te->s.eventParm = DirToByte( trace->plane.normal );
		te->s.generic1 = ent->s.generic1;
		te->s.clientNum = ent->s.clientNum;
		te->s.weapon = ent->s.weapon;
	} else {
		G_AddEvent( ent, EV_MISSILE_DETONATE, DirToByte( trace->plane.normal ) );
	}
}

/*
================
G_BFPBeamImpact
================
*/
static void G_BFPBeamImpact( gentity_t *ent, gentity_t *other, trace_t *trace ) { // BFP - Beam impact
	gentity_t *nent;
	vec3_t v;

	nent = G_Spawn();
	nent->s.clientNum = ent->s.clientNum;
	ent->s.generic1 = ent->kiChargePoints;
	nent->s.generic1 = ent->kiChargePoints;
	nent->s.weapon = ent->s.weapon;
	nent->s.generic1 = ent->kiChargePoints;
	if ( other->takedamage && other->client ) {
		nent->s.otherEntityNum = other->s.number;
		G_AddEvent( nent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ) );

		ent->enemy = other;

		v[0] = other->r.currentOrigin[0] + ( other->r.mins[0] + other->r.maxs[0] ) * 0.5;
		v[1] = other->r.currentOrigin[1] + ( other->r.mins[1] + other->r.maxs[1] ) * 0.5;
		v[2] = other->r.currentOrigin[2] + ( other->r.mins[2] + other->r.maxs[2] ) * 0.5;
	} else {
		VectorCopy( trace->endpos, v );
		// BFP - Detonation check
		G_CollideDetonationCheck( nent, trace );
		ent->enemy = NULL;
	}

	SnapVectorTowards( v, ent->s.pos.trBase );	// save net bandwidth

	nent->freeAfterEvent = qtrue;
	// change over to a normal entity right at the point of impact
	nent->s.eType = ET_GENERAL;

	G_SetOrigin( ent, v );
	G_SetOrigin( nent, v );

	if ( ent->splashDamage ) {
		G_RadiusDamage( ent, v, ent->parent, ent->splashDamage, ent->splashRadius, 
			other, ent->splashMethodOfDeath );
	}

	trap_LinkEntity( ent );
	trap_LinkEntity( nent );
	// BFP - Free trails too
	Weapon_BFPBeamFree( ent );
}


/*
================
G_Homing
================
*/
static void G_Homing( gentity_t *ent ) { // BFP - Homing
	if ( ent && ent->bfpWeapon && ent->homing > 0 && ent->homingRange > 0 ) {
		gentity_t	*target = NULL, *rad = NULL;
		vec3_t		dir, raddir, start, end;
		trace_t		tr;

		// BFP - Don't apply gravity on this part, otherwise causes stucking or out of bounds glitches
		if ( ent->bfpWeapon->missileGravity > 0 ) {
			// prevents the projectile from getting stuck
			BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->r.currentOrigin );
		}

		while ( ( rad = FindRadius(rad, ent->r.currentOrigin, ent->bfpWeapon->homingRange) ) != NULL ) {
			if ( !IsValidTargetRadius( ent, rad ) ) {
				continue;
			}

			// tracing...
			VectorCopy( ent->r.currentOrigin, start );
			VectorAdd( rad->r.absmin, rad->r.absmax, end );
			VectorScale( end, 0.5, end );
			trap_Trace( &tr, start, NULL, NULL, end, ent->s.number, MASK_SHOT );
			if ( tr.fraction < 1.0 && tr.entityNum != rad->s.number ) { // target no visible
				continue;
			}

			// TARGET DETECTED!
			VectorSubtract( rad->r.currentOrigin, ent->r.currentOrigin, raddir );
			raddir[2] += 16;
			if ( target == NULL ) {
				target = rad;
				VectorCopy( raddir, dir );
			}
		}

		if ( target != NULL ) {
			float		homingAcceleration = ( ent->bfpWeapon->homingAcceleration <= 0 ) ? 1 : ent->bfpWeapon->homingAcceleration;
			VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
			VectorNormalize( dir );
			VectorScale( dir, ent->bfpWeapon->homing, dir );
			VectorAdd( dir, ent->r.currentAngles, dir );
			VectorNormalize( dir );
			VectorCopy( dir, ent->r.currentAngles );
			VectorScale( dir, ent->bfpWeapon->missileSpeed * homingAcceleration, ent->s.pos.trDelta );
			ent->s.pos.trTime = level.time;
		}
	}
}


/*
================
G_PiercingDamage
================
*/
static void G_PiercingDamage( gentity_t *ent, gentity_t *target, int damage ) { // BFP - Piercing helper function
	vec3_t	velocity;

	// stops homing
	ent->homing = 0;
	ent->homingRange = 0;

	BG_EvaluateTrajectoryDelta( &ent->s.pos, level.time, velocity );

	++ent->piercingTouch;
	G_Damage( target, ent, ent->parent, velocity,
		ent->s.origin, damage, 0, ent->methodOfDeath );
	{
		gentity_t *effect = G_TempEntity( target->r.currentOrigin, EV_SPARK );
		effect->s.otherEntityNum  = ent->s.number;
		effect->s.otherEntityNum2 = target->s.number;
	}
	ent->piercingTime = level.time + 1000;
	ent->piercingHitTime = level.time + 200;
}


/*
================
G_PiercingFade
================
*/
static void G_PiercingFade( gentity_t *ent ) { // BFP - Piercing helper function
	if ( ent->nextthink > level.time + 1000 ) {
		ent->nextthink = level.time + 1000;
	}
	ent->piercingFade = qtrue;

	// stops homing
	ent->homing = 0;
	ent->homingRange = 0;
}


/*
================
G_Piercing
================
*/
static void G_Piercing( gentity_t *ent, trace_t *trace ) { // BFP - Piercing
	if ( ent && ent->bfpWeapon && ent->bfpWeapon->piercing ) {
		gentity_t	*rad = NULL, *other = &g_entities[trace->entityNum];
		int			damage = ( ent->splashDamage ) ? ent->splashDamage : ent->damage;
		const int	MAX_PIERCING_HITS = 4;
		float		radius = ent->r.maxs[0];

		// corrects the projectile from colliding
		BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->r.currentOrigin );

		if ( ent->piercingFade ) {
			return;
		}

		// disappear in about 1 second after hitting something solid (no living entities) or going out of boundaries
		if ( ( ( trace->fraction != 1 && trace->entityNum != ENTITYNUM_NONE )
		|| ( trace->surfaceFlags & SURF_NOIMPACT ) )
		&& !other->client
		&& !ent->piercingFade && !ent->piercingTime ) {
			G_PiercingFade( ent );
			return;
		}

		// disappear in about 1 second after receiving the maximum number of hits
		if ( ent->piercingTouch >= MAX_PIERCING_HITS ) {
			G_PiercingFade( ent );
			return;
		}

		// can still be gibbed
		if ( other && other->client
		&& ( other->health <= 0 || other->client->ps.pm_type == PM_DEAD )
		&& other->client->pers.connected != CON_DISCONNECTED
		&& other->client->sess.sessionTeam != TEAM_SPECTATOR
		&& !OnSameTeam( other, ent->parent ) ) {
			G_PiercingDamage( ent, other, ent->damage );
			VectorCopy( ent->r.currentOrigin, ent->piercingOrigin );
		}

		while ( ( rad = FindRadius(rad, ent->r.currentOrigin, radius) ) != NULL ) {
			// BFP - If it's a splitting ki ball, break and split!
			if ( rad && rad->s.eType == ET_MISSILE
			&& G_BreakRDMissile( rad ) ) {
				continue;
			}

			if ( rad && rad->s.eType == ET_MISSILE && rad->bfpWeapon && !rad->bfpWeapon->piercing ) { // pierce that projectile, let it explode
				G_MissileImpact( rad, trace );
				continue;
			}

			if ( !IsValidTargetRadius( ent, rad ) ) {
				continue;
			}

			// around 200 msec of piercing hit
			if ( ent->piercingHitTime && level.time >= ent->piercingHitTime ) {
				ent->piercingHitTime = 0;
			}
			if ( ent->piercingHitTime && level.time < ent->piercingHitTime ) {
				continue;
			}

			// around 1 second of piercing
			if ( ent->piercingTime && level.time >= ent->piercingTime ) {
				ent->piercingTime = 0;
			}
			if ( ent->piercingTime && level.time < ent->piercingTime ) {
				continue;
			}

			// avoid damaging someone alive behind
			// if the first didn't die and received damage with splash origin
			if ( rad && rad->client && !ent->piercingHitTime ) {
				ent->target_ent = rad;
				if ( other && other->client && other->takedamage
				&& rad->client == other->client ) {
					ent->target_ent = other;
				}
				if ( ent->target_ent ) {
					G_PiercingDamage( ent, ent->target_ent, damage );
				}
				VectorCopy( ent->r.currentOrigin, ent->piercingOrigin );
				if ( ent->target_ent
				&& ( ent->target_ent->health <= 0 || ent->target_ent->client->ps.pm_type == PM_DEAD ) ) {
					ent->target_ent = NULL;
					ent->piercingHitTime = 0;
					ent->piercingTime = 0;
				}
				continue;
			}
		}

		// piercing radius
		rad = NULL;
		while ( ( rad = FindRadius(rad, ent->piercingOrigin, radius) ) != NULL ) {
			if ( ent->target_ent && rad == ent->target_ent
			&& ent->piercingHitTime && level.time < ent->piercingHitTime ) {
				continue;
			}

			if ( !IsValidTargetRadius( ent, rad ) ) {
				continue;
			}

			if ( !ent->target_ent
			|| ent->target_ent->health <= 0 || ent->target_ent->client->ps.pm_type == PM_DEAD ) {
				ent->target_ent = rad;
				ent->piercingHitTime = 0;
				ent->piercingTime = 0;
			}

			if ( rad != ent->target_ent ) {
				continue;
			}

			G_PiercingDamage( ent, ent->target_ent, damage );

			if ( ent->target_ent->health <= 0 || ent->target_ent->client->ps.pm_type == PM_DEAD ) {
				// trace along the trajectory the projectile already travelled to find
				// the next living enemy that was crossed after the one that just died
				vec3_t		dir, scanOrigin, end;
				trace_t		tr;
				float		scanned;
				int			passent;
				const float	SCAN_DIST = 9999999;
				gentity_t	*next = NULL, *dead = ent->target_ent;

				VectorCopy( ent->s.pos.trDelta, dir );
				VectorNormalize( dir );
				// start the scan from where the dead target was standing, not from
				// the projectile's current position, so we pick up enemies the projectile
				// has already physically crossed along its path
				VectorCopy( ent->piercingOrigin, scanOrigin );
				passent = ent->r.ownerNum;
				scanned = 0.0f;

				while ( scanned < SCAN_DIST ) {
					float		segLen;
					gentity_t	*candidate;

					VectorMA( scanOrigin, SCAN_DIST - scanned, dir, end );
					trap_Trace( &tr, scanOrigin, NULL, NULL, end, passent, MASK_PLAYERSOLID );
					segLen = tr.fraction * ( SCAN_DIST - scanned );

					if ( tr.startsolid || tr.allsolid ) {
						scanned += 1.0f;
						VectorMA( scanOrigin, 1.0f, dir, scanOrigin );
						continue;
					}
					if ( tr.entityNum == ENTITYNUM_NONE ) {
						break;
					}

					// step past something solid and keep scanning
					if ( tr.entityNum != ENTITYNUM_NONE ) {
						scanned += segLen + 1.0f;
						VectorMA( tr.endpos, 1.0f, dir, scanOrigin );
						passent = ent->r.ownerNum;
						continue;
					}

					candidate = &g_entities[ tr.entityNum ];
					if ( !candidate->client ) {
						scanned += segLen + 1.0f;
						VectorMA( tr.endpos, 1.0f, dir, scanOrigin );
						passent = tr.entityNum;
						continue;
					}

					// skip the dead target and anyone invalid
					if ( candidate == dead
					|| candidate == ent->parent || candidate == ent
					|| candidate->r.ownerNum == ent->r.ownerNum
					|| !candidate->takedamage
					|| candidate->health <= 0 || candidate->client->ps.pm_type == PM_DEAD
					|| candidate->client->pers.connected == CON_DISCONNECTED
					|| candidate->client->sess.sessionTeam == TEAM_SPECTATOR
					|| OnSameTeam( candidate, ent->parent ) ) {
						scanned += segLen + 1.0f;
						VectorMA( tr.endpos, 1.0f, dir, scanOrigin );
						passent = tr.entityNum;
						continue;
					}
					next = candidate;
					break;
				}

				ent->target_ent = NULL;
				ent->piercingHitTime = 0;
				ent->piercingTime = 0;
				// snap piercing origin to the next target so the radius loop finds them
				if ( next ) {
					VectorCopy( next->r.currentOrigin, ent->piercingOrigin );
				} else {
					VectorCopy( ent->r.currentOrigin, ent->piercingOrigin );
				}
				rad = NULL;
			}
		}
	}
}


/*
================
G_Reflective
================
*/
void G_Reflective( gentity_t *ent, qboolean useViewAngles, const vec3_t start ) { // BFP - Reflective
	if ( ent && ent->bfpWeapon && ent->bfpWeapon->reflective ) {
		gentity_t	*rad = NULL;
		vec3_t		forward;
		float		range = ent->bfpWeapon->range;
		int			i;

		// BFP - For hitscan weapons: reflect toward where the player aims
		if ( useViewAngles && ent->parent && ent->parent->client ) {
			AngleVectors( ent->parent->client->ps.viewangles, forward, NULL, NULL );
		}

		for ( i = 0; i < level.num_entities; ++i ) {
			rad = &g_entities[i];
			if ( !rad->inuse || rad->s.eType != ET_MISSILE ) {
				continue;
			}
			if ( rad->parent == ent || rad->r.ownerNum == ent->s.number ) {
				continue;
			}
			if ( rad->freeAfterEvent || rad->nextthink <= level.time ) {
				continue;
			}
			if ( rad->bfpWeapon && ( rad->bfpWeapon->attackType == ATK_BEAM || rad->bfpWeapon->attackType == ATK_SBEAM ) ) { // cannot defend from beam & sbeam attack types
				continue;
			}
			if ( rad->bfpWeapon && rad->bfpWeapon->piercing ) { // cannot defend from piercing attacks
				continue;
			}

			BG_EvaluateTrajectory( &rad->s.pos, level.time, rad->r.currentOrigin );

			if ( Distance( rad->r.currentOrigin, start ) > range ) {
				continue;
			}

			// BFP - Projectile weapons: reflect using the missile's own travel direction
			if ( !useViewAngles ) {
				VectorNormalize2( rad->s.pos.trDelta, forward );
			}

			if ( rad->bfpWeapon ) {
				VectorScale( forward, rad->bfpWeapon->missileSpeed, rad->s.pos.trDelta );
			}
			VectorCopy( rad->r.currentOrigin, rad->s.pos.trBase );
			rad->s.pos.trTime = level.time;

			rad->r.ownerNum = ent->parent->s.number;
			rad->parent = ent->parent;
			// don't use this, otherwise, it'll convert in your weapon skin :P
			// rad->s.clientNum = ent->parent->client->ps.clientNum;

			rad->homingRange = 0;
			rad->homing = 0;
		}
	}
}


/*
================
G_MissileGravity
================
*/
static void G_MissileGravity( gentity_t *ent, trace_t *trace ) { // BFP - Missile gravity
	if ( ent && ent->bfpWeapon && ent->bfpWeapon->missileGravity > 0 ) {
		ent->s.pos.trDelta[2] -= ent->bfpWeapon->missileGravity;
		VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
		ent->s.pos.trTime = level.time;
	}
}


/*
=====================
G_MissileAcceleration
=====================
*/
static void G_MissileAcceleration( gentity_t *ent ) { // BFP - Missile acceleration
	if ( ent && ent->bfpWeapon && ent->bfpWeapon->missileAcceleration > 0 ) {
		VectorScale( ent->s.pos.trDelta, ent->bfpWeapon->missileAcceleration, ent->s.pos.trDelta );
		VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
		ent->s.pos.trTime = level.time;
	}
}

/*
================
G_MissileArch
================
*/
static void G_MissileArch( gentity_t *ent ) { // BFPR - Missile arch
	if ( ent && ent->bfpWeapon && ent->bfpWeapon->missileArch ) {
		vec3_t	curPos, nextPos, dir;
		float	t0, t1, dt, speed;
		int		nextTime;

		dt = FRAMETIME * 0.001f; // seconds per server frame, same units as TR_LINEAR
		t0 = (float)( level.time - ent->archStartTime ) / (float) ent->archDuration;
		nextTime = level.time + FRAMETIME;
		t1 = (float)( nextTime - ent->archStartTime ) / (float) ent->archDuration;

		// evaluate the quadratic Bézier at "now"
		BG_LerpQuadraticSpline( ent->archTargetPoint, ent->archControlPoint, ent->archStartPoint, t0, curPos );
		BG_LerpQuadraticSpline( ent->archTargetPoint, ent->archControlPoint, ent->archStartPoint, t1, nextPos );

		// derive an instantaneous velocity from the curve's local slope
		VectorSubtract( nextPos, curPos, dir );
		speed = VectorLength( dir );
		if ( speed > 0.001f ) {
			VectorScale( dir, 1.0f / dt, ent->s.pos.trDelta ); // units/sec
		}

		VectorCopy( curPos, ent->r.currentOrigin );
		VectorCopy( curPos, ent->s.pos.trBase );
		ent->s.pos.trTime = level.time;
	}
}

/*
=====================
G_ChargeDamageScaling
=====================
*/
void G_ChargeDamageScaling( gentity_t *ent, float radius ) { // BFP - Charge damage scaling
	int		chargeLevel, totalCharge;
	float	r, rdown, er;

	if ( !ent->bfpWeapon ) {
		return;
	}

	totalCharge = ent->kiChargePoints - ent->bfpWeapon->minCharge;
	chargeLevel = 1 + totalCharge; // add one point to adjust
	if ( chargeLevel < 0 ) {
		chargeLevel = 0;
	}
	if ( chargeLevel > totalCharge ) { // don't surpass the total
		chargeLevel = totalCharge;
	}

	r = radius + chargeLevel * ent->bfpWeapon->chargeRadiusMult;
	er = ent->bfpWeapon->explosionRadius + chargeLevel * ent->bfpWeapon->chargeExpRadiusMult;

	if ( ent->kiChargePoints <= 0 ) {
		return;
	}

	// direct damage
	ent->damage += chargeLevel * ent->bfpWeapon->chargeDamageMult;
	if ( ent->bfpWeapon->maxDamage > 0 && ent->damage > ent->bfpWeapon->maxDamage ) {
		ent->damage = ent->bfpWeapon->maxDamage;
	}

	// splash damage
	ent->splashDamage += chargeLevel * ent->bfpWeapon->chargeDamageMult;
	if ( ent->bfpWeapon->maxDamage > 0 && ent->splashDamage > ent->bfpWeapon->maxDamage ) {
		ent->splashDamage = ent->bfpWeapon->maxDamage;
	}

	// collision radius (hitbox)
	if ( ent->bfpWeapon->maxRadius > 0 && r > ent->bfpWeapon->maxRadius ) {
		r = ent->bfpWeapon->maxRadius;
	}
	radius = r;
	rdown = r;
	if ( ent->bfpWeapon->attackType != ATK_BEAM && ent->bfpWeapon->attackType != ATK_SBEAM ) {
		rdown = 10;
	}
	VectorSet( ent->r.mins, -r, -r, -rdown );
	VectorSet( ent->r.maxs, r, r, r );

	// explosionRadius (splashRadius)
	if ( ent->bfpWeapon->maxExpRadius > 0 && er > ent->bfpWeapon->maxExpRadius ) {
		er = ent->bfpWeapon->maxExpRadius;
	}
	ent->splashRadius = (int)er;
}


/*
===========
G_Priority
===========
*/
static void G_Priority( gentity_t *ent, trace_t *trace ) { // BFP - Priority
	int			entityList[MAX_GENTITIES];
	int			numEntities, i;
	vec3_t		mins, maxs;
	gentity_t	*other;

	if ( !ent || !ent->bfpWeapon ) {
		return;
	}

	VectorAdd( ent->r.mins, ent->r.currentOrigin, mins );
	VectorAdd( ent->r.maxs, ent->r.currentOrigin, maxs );

	numEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
	for ( i = 0; i < numEntities; ++i ) {
		other = &g_entities[entityList[i]];

		if ( !other || !other->inuse || other == ent ) {
			continue;
		}
		if ( other->s.eType != ET_MISSILE ) { // projectiles only
			continue;
		}
		if ( other->parent == ent->parent ) {
			continue;
		}

		if ( other->bfpWeapon && ( other->bfpWeapon->attackType == ATK_BEAM
		|| other->bfpWeapon->attackType == ATK_SBEAM
		|| other->bfpWeapon->attackType == ATK_FORCEFIELD ) ) {
			continue;
		}

		if ( other->bfpWeapon && ent->bfpWeapon->priority > other->bfpWeapon->priority ) {
			if ( other->parent && other->parent->client
			&& other->bfpWeapon->attackType == ATK_RDMISSILE && !other->splitKiBall ) {
				other->parent->client->ps.weaponstate = WEAPON_READY;
				G_RDMissile( other, other->parent->client );
			} else {
				G_MissileImpact( other, trace );
			}
		}

		if ( other->bfpWeapon && ent->bfpWeapon->priority == other->bfpWeapon->priority ) {
			if ( other->parent && other->parent->client
			&& other->bfpWeapon->attackType == ATK_RDMISSILE && !other->splitKiBall ) {
				other->parent->client->ps.weaponstate = WEAPON_READY;
				G_RDMissile( other, other->parent->client );
			} else {
				G_MissileImpact( other, trace );
			}
			if ( ent->parent && ent->parent->client
			&& ent->bfpWeapon->attackType == ATK_RDMISSILE && !ent->splitKiBall ) {
				ent->parent->client->ps.weaponstate = WEAPON_READY;
				G_RDMissile( ent, ent->parent->client );
			} else {
				G_MissileImpact( ent, trace );
			}
			return;
		}
	}
}


/*
================
G_MissileImpact
================
*/
void G_MissileImpact( gentity_t *ent, trace_t *trace ) {
	gentity_t		*other;
	qboolean		hitClient = qfalse;

	// BFP - Don't explode if going to impact outside map boundaries
	if ( trace->surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	other = &g_entities[trace->entityNum];

	// BFP - That's for ki charge points, make it lesser than 0
	ent->s.generic1 = 0;

	// check for bounce
	if ( !other->takedamage &&
	// BFP - Replaced to bounces instead using EF_BOUNCE and EF_BOUNCE_HALF eFlags
		ent->bounces ) {
		// BFP - When the projectile is stuck, just explode
		vec3_t	oldOrigin;
		trace_t	checkTrace;
		VectorCopy( ent->r.currentOrigin, oldOrigin );

		G_BounceMissile( ent, trace );

		trap_Trace( &checkTrace, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin,
				ent->r.ownerNum, ent->clipmask );

		if ( checkTrace.startsolid || checkTrace.allsolid ) {
			VectorCopy( oldOrigin, ent->r.currentOrigin );
			G_ExplodeMissile( ent );
			return;
		}
		// BFP - No grenade bounce sound
		// G_AddEvent( ent, EV_GRENADE_BOUNCE, 0 );
		return;
	}

	// impact damage
	if (other->takedamage) {
		// FIXME: wrong damage direction?
		if ( ent->damage ) {
			vec3_t	velocity;

			// BFP - When blocking... Deflect the projectile!
			if ( other->client && ( other->client->ps.pm_flags & PMF_BLOCK ) ) {
				// PUSH!
				BG_EvaluateTrajectoryDelta( &ent->s.pos, level.time, velocity );
				// BFP - No vector length check
#if 0
				if ( VectorLength( velocity ) == 0 ) {
					velocity[2] = 1;
				}
#endif

				// BFP - Consume 10% of ki when deflecting the projectile and apply knockback
				if ( other->client->blockKnockbackTime <= 0 ) {
					other->client->ps.stats[STAT_KI] -= other->client->ps.stats[STAT_MAX_KI] * 0.1;
					other->client->blockKnockbackTime = level.time + 250;
				}

				G_Damage (other, ent, &g_entities[ent->r.ownerNum], velocity,
					ent->s.origin, ent->damage, 
					0, ent->methodOfDeath);
				
				// DEFLECT!
				G_BounceMissile( ent, trace );
				return;
			}

			if( LogAccuracyHit( other, &g_entities[ent->r.ownerNum] ) ) {
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
				hitClient = qtrue;
			}
			BG_EvaluateTrajectoryDelta( &ent->s.pos, level.time, velocity );
			// BFP - No vector length check
#if 0
			if ( VectorLength( velocity ) == 0 ) {
				velocity[2] = 1;	// stepped on a grenade
			}
#endif

			G_Damage (other, ent, &g_entities[ent->r.ownerNum], velocity,
				ent->s.origin, ent->damage, 
				0, ent->methodOfDeath);
		}
	}

	// BFP - Changed "hook" to "beam" classname
	if ( ent->bfpWeapon
	&& ( ent->bfpWeapon->attackType == ATK_BEAM || ent->bfpWeapon->attackType == ATK_SBEAM ) ) {
		G_BFPBeamImpact( ent, other, trace );
		return;
	}

	// is it cheaper in bandwidth to just remove this ent and create a new
	// one, rather than changing the missile into the explosion?

	ent->s.generic1 = ent->kiChargePoints;
	if ( other->takedamage && other->client ) {
		G_AddEvent( ent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ) );
		ent->s.otherEntityNum = other->s.number;
	} else if( trace->surfaceFlags & SURF_METALSTEPS ) {
		G_AddEvent( ent, EV_MISSILE_MISS_METAL, DirToByte( trace->plane.normal ) );
	} else {
		// BFP - Detonation check
		G_CollideDetonationCheck( ent, trace );
	}

	ent->freeAfterEvent = qtrue;

	// change over to a normal entity right at the point of impact
	ent->s.eType = ET_GENERAL;

	SnapVectorTowards( trace->endpos, ent->s.pos.trBase );	// save net bandwidth

	G_SetOrigin( ent, trace->endpos );

	// splash damage (doesn't apply to person directly hit)
	if ( ent->splashDamage ) {
		if( G_RadiusDamage( ent, trace->endpos, ent->parent, ent->splashDamage, ent->splashRadius, 
			other, ent->splashMethodOfDeath ) ) {
			if( !hitClient ) {
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
			}
		}
	}

	trap_LinkEntity( ent );
}

/*
================
G_RunMissile
================
*/
void G_RunMissile( gentity_t *ent ) {
	vec3_t		origin;
	trace_t		tr;
	int			passent;
	// BFP - Shortcut variable to make sure who is using this projectile
	gclient_t	*client = ent->parent->client;

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

	// if this missile bounced off an invulnerability sphere
	if ( ent->target_ent ) {
		passent = ent->target_ent->s.number;
	}
	else {
		// ignore interactions with the missile owner
		passent = ent->r.ownerNum;
	}
	// trace a line from the previous position to the current position
	trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask );

	if ( tr.startsolid || tr.allsolid ) {
		// make sure the tr.entityNum is set to the entity we're stuck in
		trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, passent, ent->clipmask );
		tr.fraction = 0;
	}
	else {
		VectorCopy( tr.endpos, ent->r.currentOrigin );
	}

	// BFP - Ignore sinking corpses: their collision bbox lags behind their actual sinking position
	if ( tr.fraction != 1 
	&& g_entities[tr.entityNum].s.eType == ET_PLAYER 
	&& g_entities[tr.entityNum].physicsObject ) {
		trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, tr.entityNum, ent->clipmask );
		VectorCopy( tr.endpos, ent->r.currentOrigin );
	}

	trap_LinkEntity( ent );

	if ( !ent->bfpWeapon ) {
		return;
	}

	// BFP - Forcefield
	if ( ent->bfpWeapon->attackType == ATK_FORCEFIELD ) {
		if ( ent->parent && ent->parent->client ) {
			VectorCopy( ent->parent->r.currentOrigin, ent->r.currentOrigin );
			VectorCopy( ent->parent->r.currentOrigin, ent->s.pos.trBase );
			ent->s.pos.trTime = level.time;
		}
		G_RunThink( ent );
		return;
	}

	// BFP - Hitscan
	if ( ent->bfpWeapon->attackType == ATK_HITSCAN ) {
		G_RunThink( ent );
		return;
	}

	// rdmissile splitting ki ball, when pressing the attack key again, splits by the number of balls depending on the ki attack charge points had
	if ( client 
	&& client->ps.weaponstate != WEAPON_FIRING 
	&& ent->bfpWeapon && ent->bfpWeapon->attackType == ATK_RDMISSILE
	&& ent->s.weapon == ent->parent->s.weapon && !ent->splitKiBall ) {
		client->ps.weaponTime = 0;
	}

	// BFP - Avoid null bfpWeapon pointer exception
	if ( !ent->bfpWeapon || ent->bfpWeapon == NULL ) {
		return;
	}

	// BFP - Priority
	G_Priority( ent, &tr );

	// BFP - Reflective
	G_Reflective( ent, qfalse, ent->r.currentOrigin );

	// BFPR - Missile arch
	G_MissileArch( ent );

	// BFP - Missile gravity
	G_MissileGravity( ent, &tr );

	// BFP - Missile acceleration
	G_MissileAcceleration( ent );

	if ( client 
	&& ( client->pers.cmd.buttons & BUTTON_ATTACK )
	&& ent->bfpWeapon && ent->bfpWeapon->attackType == ATK_RDMISSILE
	&& !ent->splitKiBall ) {
		G_RDMissile( ent, client );
		client->ps.weaponstate = WEAPON_READY;
		return;
	}

	// BFP - Piercing weapons
	G_Piercing( ent, &tr );

	// BFP - Homing weapons
	G_Homing( ent );

	// BFP - Beam handling
	if ( client 
	&& ent->bfpWeapon && ( ent->bfpWeapon->attackType == ATK_BEAM || ent->bfpWeapon->attackType == ATK_SBEAM )
	&& ent->s.weapon == ent->parent->s.weapon ) {
		if ( client->ps.weaponstate != WEAPON_ACTIVE
		&& client->ps.weaponstate != WEAPON_BEAMSTRUGGLE ) {
			G_MissileImpact( ent, &tr );
			client->ps.weaponTime = 0;
		}
		if ( tr.fraction != 1 && !( tr.surfaceFlags & SURF_NOIMPACT ) ) {
			G_MissileImpact( ent, &tr );
			client->ps.weaponTime = 0;
		} else {
			Weapon_BFPBeamRun( ent );
			Weapon_SBeam_Run( ent );
		}

		if ( client->ps.pm_type == PM_DEAD 				// if just died, then stop
		|| client->pers.connected == CON_DISCONNECTED 	// if disconnected, stop
		|| ( tr.surfaceFlags & SURF_NOIMPACT )
		|| tr.fraction != 1 ) {
			Weapon_BFPBeamFree( ent );
			return;
		}
	}

	if ( tr.fraction != 1 ) {

		// BFP - When the charged projectile touches into something, return to ready state
		if ( client 
		&& ( client->ps.weaponstate == WEAPON_ACTIVE
		|| client->ps.weaponstate == WEAPON_BEAMSTRUGGLE )
		// avoid exploding if there's another projectile running on
		&& ent->bfpWeapon
		&& ( ent->bfpWeapon->attackType == ATK_BEAM
		|| ent->bfpWeapon->attackType == ATK_SBEAM
		|| ent->bfpWeapon->attackType == ATK_RDMISSILE )
		&& ent->s.weapon != ent->parent->s.weapon ) {
			// BFP - movementPenalty
			if ( ent->bfpWeapon->movementPenalty > 0 ) {
				if ( ent->bfpWeapon->movementPenalty > client->ps.weaponTime ) {
					client->ps.weaponTime = ent->bfpWeapon->movementPenalty - client->ps.weaponTime;
				} else {
					client->ps.weaponTime = ent->bfpWeapon->movementPenalty;
				}
				client->ps.weaponstate = WEAPON_STUN;
			} else {
				client->ps.weaponstate = WEAPON_READY;
			}
		}

		// BFP - Splitting ki ball
		if ( client 
		&& ent->bfpWeapon && ent->bfpWeapon->attackType == ATK_RDMISSILE
		&& ent->s.weapon == ent->parent->s.weapon
		&& !ent->splitKiBall ) {
			client->ps.weaponstate = WEAPON_READY;
		}

		// never explode or bounce on sky
		if ( tr.surfaceFlags & SURF_NOIMPACT ) {
// BFP - no hook
#if 0
			// If grapple, reset owner
			if (ent->parent && ent->parent->client && ent->parent->client->hook == ent) {
				ent->parent->client->hook = NULL;
			}
#endif

			// BFP - Don't disappear instantly on piercing weapons
			if ( ent->bfpWeapon && !ent->bfpWeapon->piercing ) {
				G_FreeEntity( ent );
			}
			return;
		}

		// BFP - Splitting ki ball
		if ( client 
		&& ent->bfpWeapon && ent->bfpWeapon->attackType == ATK_RDMISSILE && !ent->splitKiBall ) {
			G_RDMissile( ent, client );
			return;
		}

		// BFP - Don't explode on piercing weapons
		if ( ent->bfpWeapon && !ent->bfpWeapon->piercing ) {	
			G_MissileImpact( ent, &tr );
		}
		if ( ent->s.eType != ET_MISSILE ) {
			return;		// exploded
		}
	}
	// check think function after bouncing
	G_RunThink( ent );
}
