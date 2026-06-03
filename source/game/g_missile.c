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

#define	MISSILE_PRESTEP_TIME	50

/*
================
G_DivideProjectile_Fire
================
*/
static void G_DivideProjectile_Fire( gentity_t *ent, vec3_t start, vec3_t dir ) {
	gentity_t	*m;

	// BFP - TODO: IMPORTANT, look (homing_special) and (homing_special_spawn), 
	// (homing_special) has explosionSpawn set to divide the projectiles, then if that happens,
	// goes to (homing_special_spawn) and fires there x number of divided projectiles

	m = fire_plasma( ent, start, dir );
	m->enabledivide = qtrue; // handle divided ki ball, otherwise crashes (in DLL/SO)

	// example of how would be the special spawn
	m->classname = "missile";
	m->homing = 0.9;
	m->homingRange = 800;

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}

/*
================
G_HandleRDMissile
================
*/
static void G_HandleRDMissile( gentity_t *ent, gclient_t *client ) { // BFP - WP_PLASMAGUN would be that dividing ball, when pressing the attack key again, divides by the number of balls depending on the ki attack charge points had
	vec3_t	dir, angles;
	int		i;
	int		chargePoints = client->kiChargePoints;
	int		projectiles_to_spawn = 0;
	// BFP - NOTE: That makes us to understand how the projectile is being divided by x projectiles
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

	// BFP - TODO: Apply minCharge and maxCharge from reading bfp_weapon.cfg 
	if ( chargePoints < 2 ) {
		client->ps.weaponstate = WEAPON_READY;
		client->ps.pm_flags &= ~PMF_KI_ATTACK;
		client->ps.stats[STAT_KI_ATTACK_CHARGE] = 0; // reset ki charge points
		client->ps.weaponTime = 0;
		return;
	}

	// determine the number of projectiles to spawn based on the ki attack charge points
	switch( chargePoints ) {
	case 2:
		projectiles_to_spawn = 3;
		break;
	case 3:
		projectiles_to_spawn = 4;
		break;
	case 4:
		projectiles_to_spawn = 5;
		break;
	case 5:
	case 6:
		projectiles_to_spawn = 6;
		break;
	default:
		projectiles_to_spawn = 0;
	}

	if ( projectiles_to_spawn == 0 ) {
		client->ps.weaponstate = WEAPON_READY;
		client->ps.pm_flags &= ~PMF_KI_ATTACK;
		client->ps.stats[STAT_KI_ATTACK_CHARGE] = 0; // reset ki charge points
		client->ps.weaponTime = 0;
		return;
	}

	{
		// this is for the new spawning projectiles, 
		// so, the owner is protected from projectile collisions and 
		// only can be damaged by explosion
		vec3_t		forward, right, up;
		gentity_t *owner = NULL;
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
				VectorCopy( up, dir );
				break;
			case 5: // down
				VectorNegate( up, dir );
			}
			VectorNormalize( dir );
			G_DivideProjectile_Fire( owner, ent->r.currentOrigin, dir );
		}
	}

	client->ps.weaponstate = WEAPON_READY;
	client->ps.pm_flags &= ~PMF_KI_ATTACK;
	client->ps.stats[STAT_KI_ATTACK_CHARGE] = 0; // reset ki charge points
	client->ps.weaponTime = 0;
}

/*
================
G_RDMissile
================
*/
void G_RDMissile( gentity_t *ent, gclient_t *client ) { // BFP - rdmissile (Divide ki ball)
	G_HandleRDMissile( ent, client );
	ent->enabledivide = qtrue;
	G_FreeEntity( ent );
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

	if ( ent->s.eFlags & EF_BOUNCE_HALF ) {
		VectorScale( ent->s.pos.trDelta, 0.65, ent->s.pos.trDelta );
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
G_CollideDetonationCheck

Checks the projectile collision radius and detonation point
================
*/
static void G_CollideDetonationCheck( gentity_t *ent, trace_t *trace ) { // BFP - Detonation check
	vec3_t impactPoint;
	// BFP - NOTE: This setup solves the issue of real impact crack mark for collision radius, but original BFP didn't that (uses -1)
	float distToPlane = DotProduct( trace->endpos, trace->plane.normal ) - trace->plane.dist;

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
	} else {
		G_AddEvent( ent, EV_MISSILE_DETONATE, 0 );
	}
}

/*
================
G_BFPBeamImpact
================
*/
static void G_BFPBeamImpact( gentity_t *ent, gentity_t *other, trace_t *trace ) {
	gentity_t *nent;
	vec3_t v;

	nent = G_Spawn();
	if ( other->takedamage && other->client ) {

		G_AddEvent( nent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ) );
		nent->s.otherEntityNum = other->s.number;

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
	// if ( ent->s.weapon == WP_GRAPPLING_HOOK ) {
		Weapon_BFPBeamFree( ent );
}


/*
================
G_Homing
================
*/
static void G_Homing( gentity_t *ent ) { // BFP - Homing
	gentity_t	*target = NULL, *rad = NULL;
	vec3_t		dir, raddir;

	// prevents the projectile from getting stuck
	BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->r.currentOrigin );

	while ( ( rad = FindRadius(rad, ent->r.currentOrigin, ent->homingRange) ) != NULL ) {
		if ( IsValidTargetRadius( ent, rad ) ) {
			VectorSubtract( rad->r.currentOrigin, ent->r.currentOrigin, raddir );
			raddir[2] += 16;
			if ( target == NULL ) {
				target = rad;
				VectorCopy( raddir, dir );
			}
		}
	}

	if ( target != NULL ) {
		float		homingAcceleration = ( ent->homingAcceleration <= 0 ) ? 1 : ent->homingAcceleration;
		VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
		VectorNormalize( dir );
		VectorScale( dir, ent->homing, dir );
		VectorAdd( dir, ent->r.currentAngles, dir );
		VectorNormalize( dir );
		VectorCopy( dir, ent->r.currentAngles );
		VectorScale( dir, ent->speed * homingAcceleration, ent->s.pos.trDelta );
		ent->s.pos.trTime = level.time;
	}
}


/*
================
G_PiercingDamage
================
*/
static void G_PiercingDamage( gentity_t *ent, gentity_t *target, int damage ) { // BFP - Piercing helper function
	// stops homing
	ent->homing = 0;
	ent->homingRange = 0;

	++ent->piercingTouch;
	G_Damage( target, ent, ent->parent, NULL,
		ent->s.origin, damage, 0, ent->methodOfDeath );
	{
		gentity_t *effect = G_TempEntity( target->r.currentOrigin, EV_BEAM_STRUGGLE );
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
static void G_Piercing( gentity_t *ent, trace_t trace ) { // BFP - Piercing
	gentity_t	*rad = NULL, *other = &g_entities[trace.entityNum];
	int			damage = ( ent->splashDamage ) ? ent->splashDamage : ent->damage;
	const int	MAX_PIERCING_HITS = 4;

	// corrects the projectile from colliding
	BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->r.currentOrigin );

	if ( ent->piercingFade ) {
		return;
	}

	// disappear in about 1 second after hitting something solid (no living entities) or going out of boundaries
	if ( ( ( trace.fraction != 1 && trace.entityNum != ENTITYNUM_NONE )
	|| ( trace.surfaceFlags & SURF_NOIMPACT ) )
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

	while ( ( rad = FindRadius(rad, ent->r.currentOrigin, ent->splashRadius) ) != NULL ) {
		// BFP - If it's a dividing ki ball, break and divide!
		if ( rad && rad->s.eType == ET_MISSILE
		&& !Q_stricmp( rad->classname, "rdmissile" ) && !rad->enabledivide
		&& !rad->piercing ) {
			G_RDMissile( rad, rad->parent->client );
			continue;
		}

		if ( rad && rad->s.eType == ET_MISSILE && !rad->piercing ) { // pierce that projectile, let it explode
			G_MissileImpact( rad, &trace );
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
			G_PiercingDamage( ent, ent->target_ent, damage );
			VectorCopy( ent->r.currentOrigin, ent->piercingOrigin );
			if ( ent->target_ent->health <= 0 || ent->target_ent->client->ps.pm_type == PM_DEAD ) {
				ent->target_ent = NULL;
				ent->piercingHitTime = 0;
				ent->piercingTime = 0;
			}
			continue;
		}
	}

	// piercing radius
	rad = NULL;
	while ( ( rad = FindRadius(rad, ent->piercingOrigin, ent->splashRadius) ) != NULL ) {
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


/*
================
G_Reflective
================
*/
void G_Reflective( gentity_t *ent, const vec3_t start, const vec3_t end ) {
	gentity_t	*rad = NULL;
	vec3_t		forward;
	// BFP - TODO: For weapon config: range, ...
	float		range = 500;
	int			i;

	if ( !ent || !ent->client ) {
		return;
	}

	AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );

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
		if ( !Q_stricmp( rad->classname, "beam" ) || !Q_stricmp( rad->classname, "sbeam" ) ) { // cannot defend from beam & sbeam attack types
			continue;
		}
		if ( rad->piercing ) { // cannot defend from piercing attacks
			continue;
		}

		BG_EvaluateTrajectory( &rad->s.pos, level.time, rad->r.currentOrigin );

		if ( Distance( rad->r.currentOrigin, start ) > range ) {
			continue;
		}

		VectorScale( forward, rad->speed, rad->s.pos.trDelta );
		VectorCopy( rad->r.currentOrigin, rad->s.pos.trBase );
		rad->s.pos.trTime = level.time;

		rad->r.ownerNum = ent->s.number;
		rad->parent = ent;

		rad->homingRange = 0;
		rad->homing = 0;
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

	// check for bounce
	if ( !other->takedamage &&
		( ent->s.eFlags & ( EF_BOUNCE | EF_BOUNCE_HALF ) ) ) {
		G_BounceMissile( ent, trace );
		G_AddEvent( ent, EV_GRENADE_BOUNCE, 0 );
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
					other->client->ps.ammo[WP_KI] -= other->client->ps.stats[STAT_MAX_KI] * 0.1;
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
	if ( !Q_stricmp( ent->classname, "beam" ) || !Q_stricmp( ent->classname, "sbeam" ) ) {
		G_BFPBeamImpact( ent, other, trace );
		return;
	}

// BFP - no hook
#if 0
	if (!strcmp(ent->classname, "hook")) {
		gentity_t *nent;
		vec3_t v;

		nent = G_Spawn();
		if ( other->takedamage && other->client ) {

			G_AddEvent( nent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ) );
			nent->s.otherEntityNum = other->s.number;

			ent->enemy = other;

			v[0] = other->r.currentOrigin[0] + (other->r.mins[0] + other->r.maxs[0]) * 0.5;
			v[1] = other->r.currentOrigin[1] + (other->r.mins[1] + other->r.maxs[1]) * 0.5;
			v[2] = other->r.currentOrigin[2] + (other->r.mins[2] + other->r.maxs[2]) * 0.5;

			SnapVectorTowards( v, ent->s.pos.trBase );	// save net bandwidth
		} else {
			VectorCopy(trace->endpos, v);
			G_AddEvent( nent, EV_MISSILE_MISS, DirToByte( trace->plane.normal ) );
			ent->enemy = NULL;
		}

		SnapVectorTowards( v, ent->s.pos.trBase );	// save net bandwidth

		nent->freeAfterEvent = qtrue;
		// change over to a normal entity right at the point of impact
		nent->s.eType = ET_GENERAL;
		ent->s.eType = ET_GRAPPLE;

		G_SetOrigin( ent, v );
		G_SetOrigin( nent, v );

		ent->think = Weapon_HookThink;
		ent->nextthink = level.time + FRAMETIME;

		ent->parent->client->ps.pm_flags |= PMF_GRAPPLE_PULL;
		VectorCopy( ent->r.currentOrigin, ent->parent->client->ps.grapplePoint);

		trap_LinkEntity( ent );
		trap_LinkEntity( nent );

		return;
	}
#endif

	// is it cheaper in bandwidth to just remove this ent and create a new
	// one, rather than changing the missile into the explosion?

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

	trap_LinkEntity( ent );

	// BFP - When the player stopped shooting the charged beam/projectile by pressing the attack key
	// These are sample weapons used as examples for BFP: 

	// WP_GRAPPLING_HOOK would be the beam
	if ( client 
	&& ( client->ps.weaponstate != WEAPON_BEAMFIRING
	&& client->ps.weaponstate != WEAPON_BEAMSTRUGGLE )
	&& ent->s.weapon == WP_GRAPPLING_HOOK ) {
		G_MissileImpact( ent, &tr );
	}

	// WP_PLASMAGUN would be that dividing ball, when pressing the attack key again, divides by the number of balls depending on the ki attack charge points had
	if ( client 
	&& !( client->ps.pm_flags & PMF_KI_ATTACK ) 
	&& client->ps.weapon == WP_PLASMAGUN
	&& ent->s.weapon == WP_PLASMAGUN
	&& !ent->enabledivide ) {
		client->ps.weaponstate = WEAPON_DIVIDINGKIBALLFIRING;
		client->ps.pm_flags |= PMF_KI_ATTACK;
	}

	if ( client 
	&& ( client->pers.cmd.buttons & BUTTON_ATTACK )
	&& client->ps.weapon == WP_PLASMAGUN
	&& ent->s.weapon == WP_PLASMAGUN
	&& !ent->enabledivide ) {
		G_RDMissile( ent, client );
		return;
	}

	// BFP - Piercing weapons
	if ( ent->piercing ) {
		G_Piercing( ent, tr );
	}

	// BFP - Homing weapons
	if ( ent->homingRange ) {
		G_Homing( ent );
	}

	if ( client 
	&& client->ps.weapon == WP_PLASMAGUN
	&& client->ps.weaponstate == WEAPON_DIVIDINGKIBALLFIRING
	&& ent->s.weapon == WP_PLASMAGUN ) {
		client->ps.pm_flags |= PMF_KI_ATTACK;
	}

	// BFP - Beam handling
	if ( client 
	&& ent->s.weapon == WP_GRAPPLING_HOOK ) {
		Weapon_BFPBeamRun( ent );
		Weapon_SBeam_Run( ent );

		if ( client->ps.pm_type == PM_DEAD 				// if just died, then stop
		|| client->pers.connected == CON_DISCONNECTED 	// if disconnected, stop
		|| ( tr.surfaceFlags & SURF_NOIMPACT )
		|| tr.fraction != 1 ) {
			Weapon_BFPBeamFree( ent );
			return;
		}
	}

	if ( tr.fraction != 1 ) {

		// BFP - When the charged projectile touches into something, disable ki attack PMF flag
		if ( client 
		&& ( client->ps.weaponstate == WEAPON_BEAMFIRING
		|| client->ps.weaponstate == WEAPON_BEAMSTRUGGLE
		|| client->ps.weaponstate == WEAPON_DIVIDINGKIBALLFIRING ) ) {
			client->ps.pm_flags &= ~PMF_KI_ATTACK;
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
			if ( !ent->piercing ) {
				G_FreeEntity( ent );
			}
			return;
		}

		// BFP - Dividing ball
		if ( client 
		&& ent->s.weapon == WP_PLASMAGUN
		&& !ent->enabledivide ) {
			G_RDMissile( ent, client );
			return;
		}

		// BFP - Don't explode on piercing weapons
		if ( !ent->piercing ) {	
			G_MissileImpact( ent, &tr );
		}
		if ( ent->s.eType != ET_MISSILE ) {
			return;		// exploded
		}
	}
	// check think function after bouncing
	G_RunThink( ent );
}


//=============================================================================

/*
=================
fire_plasma

=================
*/
gentity_t *fire_plasma (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;
	// BFP - Projectile radius
	float		radius = 30;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "rdmissile";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_PLASMAGUN;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 20;
	bolt->splashDamage = 15;
	bolt->splashRadius = 20;
	bolt->methodOfDeath = MOD_PLASMA;
	bolt->splashMethodOfDeath = MOD_PLASMA_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	bolt->enabledivide = qfalse; // BFP - For dividing ki ball

	// BFP - Speed similar to BFP ki blast attack (missileSpeed)
	bolt->speed = 700;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, bolt->speed, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	// BFP - Collision radius
	VectorSet( bolt->r.mins, -radius, -radius, -10 );
	VectorSet( bolt->r.maxs, radius, radius, radius );

	return bolt;
}	

//=============================================================================


/*
=================
fire_grenade
=================
*/
gentity_t *fire_grenade (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "grenade";
	bolt->nextthink = level.time + 2500;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_GRENADE_LAUNCHER;
	bolt->s.eFlags = EF_BOUNCE_HALF;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 100;
	bolt->splashDamage = 100;
	bolt->splashRadius = 150;
	bolt->methodOfDeath = MOD_GRENADE;
	bolt->splashMethodOfDeath = MOD_GRENADE_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 700, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}

//=============================================================================


/*
=================
fire_bfg
=================
*/
gentity_t *fire_bfg (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "bfg";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_BFG;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 100;
	bolt->splashDamage = 100;
	bolt->splashRadius = 120;
	bolt->methodOfDeath = MOD_BFG;
	bolt->splashMethodOfDeath = MOD_BFG_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 2000, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	return bolt;
}

//=============================================================================


/*
=================
fire_rocket
=================
*/
gentity_t *fire_rocket (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*bolt;
	// BFP - Projectile radius
	float		radius = 20;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "rocket";
	bolt->nextthink = level.time + 15000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_ROCKET_LAUNCHER;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 100;
	bolt->splashDamage = 100;
	bolt->splashRadius = 120;
	bolt->methodOfDeath = MOD_ROCKET;
	bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	// BFP - Speed similar to BFP ki blast attack (missileSpeed)
	bolt->speed = 6000;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, bolt->speed, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	// BFP - Collision radius
	VectorSet( bolt->r.mins, -radius, -radius, -10 );
	VectorSet( bolt->r.maxs, radius, radius, radius );

	return bolt;
}
// BFP - no hook
#if 0
/*
=================
fire_grapple
=================
*/
gentity_t *fire_grapple (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*hook;

	VectorNormalize (dir);

	hook = G_Spawn();
	hook->classname = "hook";
	hook->nextthink = level.time + 10000;
	hook->think = Weapon_HookFree;
	hook->s.eType = ET_MISSILE;
	hook->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	hook->s.weapon = WP_GRAPPLING_HOOK;
	hook->r.ownerNum = self->s.number;
	hook->methodOfDeath = MOD_GRAPPLE;
	hook->clipmask = MASK_SHOT;
	hook->parent = self;
	hook->target_ent = NULL;

	hook->s.pos.trType = TR_LINEAR;
	hook->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	hook->s.otherEntityNum = self->s.number; // use to match beam in client
	VectorCopy( start, hook->s.pos.trBase );
	VectorScale( dir, 800, hook->s.pos.trDelta );
	SnapVector( hook->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, hook->r.currentOrigin);

	self->client->hook = hook;

	return hook;
}
#endif

/*
=================
fire_bfpbeam
=================
*/
gentity_t *fire_bfpbeam (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*beam;
	// BFP - Projectile radius
	float		radius = 30;

	VectorNormalize( dir );

	beam = G_Spawn();
	beam->classname = "beam";
	beam->nextthink = level.time + 20000;
	beam->think = Weapon_BFPBeamFree;
	beam->s.eType = ET_MISSILE;
	beam->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	beam->s.weapon = WP_GRAPPLING_HOOK;
	beam->r.ownerNum = self->s.number;
	beam->methodOfDeath = MOD_KI_ATTACK;
	beam->clipmask = MASK_SHOT;
	beam->parent = self;
	beam->target_ent = NULL;

	beam->damage = 80;
	beam->splashDamage = 80;
	beam->splashRadius = 350;
	beam->splashMethodOfDeath = MOD_KI_ATTACK;

	// BFP - Speed similar to BFP lightning blast/heaven's wrath attack (missileSpeed)
	beam->speed = 2000;

	beam->s.pos.trType = TR_LINEAR;
	beam->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	beam->s.otherEntityNum = self->s.number; // use to match beam in client
	VectorCopy( start, beam->s.pos.trBase );
	VectorScale( dir, beam->speed, beam->s.pos.trDelta );	// speed
	SnapVector( beam->s.pos.trDelta );			// save net bandwidth
	VectorCopy( start, beam->r.currentOrigin );

	// BFP - Set beam delta time and distance to avoid timescale < 1 issues
	beam->deltaTime = level.time;
	beam->distance = 0;

	// BFP - TODO: For weapon config, if uses chargeAttack, maxRadius, chargeRadiusMult, calculate the radius here
#if 0
	if ( beam->chargeAttack && self->client ) {
		float	r = cfg->radius + ( self->client->kiChargePoints - cfg->minCharge ) * cfg->chargeRadiusMult;
		if ( r > cfg->maxRadius && cfg->maxRadius > 0 ) {
			r = cfg->maxRadius;
		}
		radius = r;
	}
#endif

	// BFP - Collision radius
	VectorSet( beam->r.mins, -radius, -radius, -radius );
	VectorSet( beam->r.maxs, radius, radius, radius );

	return beam;
}

// BFP - Just a testing disk proyectile
/*
=================
fire_disk
=================
*/
gentity_t *fire_disk (gentity_t *self, vec3_t start, vec3_t dir) {
	gentity_t	*disk;
	// BFP - Projectile radius
	float		radius = 75;

	VectorNormalize ( dir );

	disk = G_Spawn();
	disk->classname = "missile";
	disk->nextthink = level.time + 10000;
	disk->think = G_FreeEntity;
	disk->s.eType = ET_MISSILE;
	disk->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	disk->s.weapon = WP_BFG;
	disk->r.ownerNum = self->s.number;
	disk->parent = self;
	disk->damage = 20;
	disk->splashDamage = 20;
	disk->splashRadius = 120;
	disk->methodOfDeath = MOD_KI_ATTACK;
	disk->splashMethodOfDeath = MOD_KI_ATTACK;
	disk->clipmask = MASK_SHOT;
	disk->target_ent = NULL;

	disk->speed = 1000;

	disk->piercing = 1;
	disk->homing = 0.5;
	disk->homingRange = 2000;
	disk->homingAcceleration = 0;

	disk->s.pos.trType = TR_LINEAR;
	disk->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, disk->s.pos.trBase );
	VectorScale( dir, disk->speed, disk->s.pos.trDelta );
	SnapVector( disk->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, disk->r.currentOrigin);

	// BFP - Collision radius
	VectorSet( disk->r.mins, -radius, -radius, -10 );
	VectorSet( disk->r.maxs, radius, radius, radius );

	return disk;
}

// BFP - Just a testing forcefield
/*
=================
fire_forcefield
=================
*/
gentity_t *fire_forcefield (gentity_t *self, vec3_t start)
{
	// BFP - TODO: bfp_weapon.cfg: radius, ...
	gentity_t   *field;

	field = G_Spawn();
	field->classname = "forcefield";
	field->nextthink = level.time + 200;
	field->think = Weapon_Forcefield_Think;
	field->s.eType = ET_MISSILE;
	field->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	field->s.weapon = WP_SHOTGUN;
	field->r.ownerNum = self->s.number;
	field->parent = self;
	field->clipmask = MASK_SHOT;
	field->target_ent = NULL;

	field->damage = 20;
	field->splashDamage = 0;
	field->splashRadius = 0;
	field->methodOfDeath = MOD_KI_ATTACK;
	field->splashMethodOfDeath = MOD_KI_ATTACK;

	field->radius = 900;
	field->blinding = qfalse;

	field->s.pos.trType = TR_STATIONARY;
	field->s.pos.trTime = level.time;
	VectorCopy( start, field->s.pos.trBase );
	VectorClear( field->s.pos.trDelta );

	VectorCopy( start, field->r.currentOrigin );

	self->client->hook = field;

	return field;
}

// BFP - Just a testing sbeam
/*
=================
fire_sbeam
=================
*/
gentity_t *fire_sbeam (gentity_t *self, vec3_t start, vec3_t dir)
{
	// BFP - TODO: bfp_weapon.cfg: radius, ...
	gentity_t   *sbeam;

	sbeam = G_Spawn();
	sbeam->classname = "sbeam";
	sbeam->nextthink = level.time + 20000;
	sbeam->think = G_FreeEntity;
	sbeam->s.eType = ET_MISSILE;
	sbeam->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	sbeam->s.weapon = WP_GRAPPLING_HOOK;
	sbeam->r.ownerNum = self->s.number;
	sbeam->parent = self;
	sbeam->clipmask = MASK_SHOT;
	sbeam->target_ent = NULL;

	sbeam->damage = 15;
	sbeam->splashDamage = 15;
	sbeam->splashRadius = 200;
	sbeam->methodOfDeath = MOD_KI_ATTACK;
	sbeam->splashMethodOfDeath = MOD_KI_ATTACK;

	sbeam->radius = 50;

	// BFP - Speed similar to BFP mouth beam (missileSpeed)
	sbeam->speed = 1000;

	sbeam->s.pos.trType = TR_LINEAR;
	sbeam->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	sbeam->s.otherEntityNum = self->s.number; // use to match sbeam in client
	VectorCopy( start, sbeam->s.pos.trBase );
	VectorScale( dir, sbeam->speed, sbeam->s.pos.trDelta );	// speed
	SnapVector( sbeam->s.pos.trDelta );			// save net bandwidth
	VectorCopy( start, sbeam->r.currentOrigin );

	// BFP - Collision radius
	VectorSet( sbeam->r.mins, -sbeam->radius, -sbeam->radius, -sbeam->radius );
	VectorSet( sbeam->r.maxs, sbeam->radius, sbeam->radius, sbeam->radius );

	return sbeam;
}
