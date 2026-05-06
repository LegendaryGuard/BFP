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

	m = fire_plasma( ent, start, dir );
	m->enabledivide = qtrue; // handle divided ki ball, otherwise crashes (in DLL/SO)

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}

/*
================
G_HandleDivideKiBall
================
*/
static void G_HandleDivideKiBall( gentity_t *ent, gclient_t *client ) { // BFP - WP_PLASMAGUN would be that dividing ball, when pressing the attack key again, divides by the number of balls depending on the ki attack charge points had
	vec3_t	dir, angles;
	int		i;
	int		chargePoints = client->kiChargePoints;
	int		projectiles_to_spawn = 0;
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

	// BFP - TODO: Apply minCharge and maxCharge from reading bfp_weapon.cfg 
	if ( chargePoints < 2 ) {
		client->ps.weaponstate = WEAPON_READY;
		client->ps.pm_flags &= ~PMF_KI_ATTACK;
		client->ps.stats[STAT_KI_ATTACK_CHARGE] = 0; // reset ki charge points
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
		return;
	}

	{
		// this is for the new spawning projectiles, 
		// so, the owner is protected from projectile collisions and 
		// only can be damaged by explosion
		gentity_t *owner = NULL;
		if ( ent->r.ownerNum < MAX_CLIENTS && ent->r.ownerNum >= 0 ) {
			owner = &g_entities[ent->r.ownerNum];
		}
		
		if ( !owner || !owner->client ) {
			owner = &g_entities[client->ps.clientNum];
		}

		for ( i = 0; i < projectiles_to_spawn; ++i ) {
			VectorCopy( ent->s.angles, angles );

			angles[YAW] += yawAdjustments[i];
			angles[PITCH] += pitchAdjustments[i];

			AngleVectors( angles, dir, NULL, NULL );
			G_DivideProjectile_Fire( owner, ent->r.currentOrigin, dir );
		}
	}

	client->ps.weaponstate = WEAPON_READY;
	client->ps.pm_flags &= ~PMF_KI_ATTACK;
	client->ps.stats[STAT_KI_ATTACK_CHARGE] = 0; // reset ki charge points
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

		SnapVectorTowards( v, ent->s.pos.trBase );	// save net bandwidth
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
	if ( !Q_stricmp( ent->classname, "beam" ) ) {
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
	gclient_t	*client = g_entities[ ent->r.ownerNum ].client;

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
		G_HandleDivideKiBall( ent, client );
		ent->enabledivide = qtrue;
		G_FreeEntity( ent );
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
			G_FreeEntity( ent );
			return;
		}
		G_MissileImpact( ent, &tr );
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

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "plasma";
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

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 2000, bolt->s.pos.trDelta );
	//SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

	VectorCopy (start, bolt->r.currentOrigin);

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
	VectorSet( bolt->r.mins, -radius, -radius, -radius );
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
	beam->splashRadius = 120;
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

	// BFP - Collision radius
	VectorSet( beam->r.mins, -radius, -radius, -radius );
	VectorSet( beam->r.maxs, radius, radius, radius );

	return beam;
}
