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
// cg_effects.c -- these functions generate localentities, usually as a result
// of event processing

#include "cg_local.h"


/*
==================
CG_BubbleTrail

Bullets shot underwater
==================
*/
void CG_BubbleTrail( vec3_t start, vec3_t end, float spacing ) {
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			i;

	if ( cg_noProjectileTrail.integer ) {
		return;
	}

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	// advance a random amount first
	i = rand() % (int)spacing;
	VectorMA( move, i, vec, move );

	VectorScale (vec, spacing, vec);

	for ( ; i < len; i += spacing ) {
		localEntity_t	*le;
		refEntity_t		*re;

		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + 1000 + random() * 250;
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );

		re = &le->refEntity;
		re->shaderTime = cg.time / 1000.0f;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = 3;
		re->customShader = cgs.media.waterBubbleShader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;

		le->color[3] = 1.0;

		le->pos.trType = TR_LINEAR;
		le->pos.trTime = cg.time;
		VectorCopy( move, le->pos.trBase );
		le->pos.trDelta[0] = crandom()*5;
		le->pos.trDelta[1] = crandom()*5;
		le->pos.trDelta[2] = crandom()*5 + 6;

		VectorAdd (move, vec, move);
	}
}

/*
=====================
CG_SmokePuff

Adds a smoke puff or blood trail localEntity.
=====================
*/
localEntity_t *CG_SmokePuff( const vec3_t p, const vec3_t vel, 
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,
				   qhandle_t hShader ) {
	static int	seed = 0x92;
	localEntity_t	*le;
	refEntity_t		*re;
//	int fadeInTime = startTime + duration / 2;

	le = CG_AllocLocalEntity();
	le->leFlags = leFlags;
	le->radius = radius;

	re = &le->refEntity;
	re->rotation = Q_random( &seed ) * 360;
	re->radius = radius;
	re->shaderTime = startTime / 1000.0f;

	le->leType = LE_MOVE_SCALE_FADE;
	le->startTime = startTime;
	le->fadeInTime = fadeInTime;
	le->endTime = startTime + duration;
	if ( fadeInTime > startTime ) {
		le->lifeRate = 1.0 / ( le->endTime - le->fadeInTime );
	}
	else {
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );
	}
	le->color[0] = r;
	le->color[1] = g; 
	le->color[2] = b;
	le->color[3] = a;


	le->pos.trType = TR_LINEAR;
	le->pos.trTime = startTime;
	VectorCopy( vel, le->pos.trDelta );
	VectorCopy( p, le->pos.trBase );

	VectorCopy( p, re->origin );
	re->customShader = hShader;

	
	re->shaderRGBA[0] = le->color[0] * 0xff;
	re->shaderRGBA[1] = le->color[1] * 0xff;
	re->shaderRGBA[2] = le->color[2] * 0xff;
	re->shaderRGBA[3] = 0xff;
	

	re->reType = RT_SPRITE;
	re->radius = le->radius;

	return le;
}

/*
==================
CG_SpawnEffect

Player teleporting in or out
==================
*/
void CG_SpawnEffect( vec3_t org ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + 500;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

	re->customShader = cgs.media.teleportEffectShader;
	re->hModel = cgs.media.teleportEffectModel;
	AxisClear( re->axis );

	VectorCopy( org, re->origin );
	re->origin[2] -= 24;
}

/*
==================
CG_ScorePlum
==================
*/
void CG_ScorePlum( int client, vec3_t org, int score ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			angles;
	static vec3_t lastPos;

	// only visualize for the client that scored
	if (client != cg.predictedPlayerState.clientNum || cg_scorePlum.integer == 0) {
		return;
	}

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_SCOREPLUM;
	le->startTime = cg.time;
	le->endTime = cg.time + 4000;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	
	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;
	le->radius = score;
	
	VectorCopy( org, le->pos.trBase );
	if (org[2] >= lastPos[2] - 20 && org[2] <= lastPos[2] + 20) {
		le->pos.trBase[2] -= 20;
	}

	//CG_Printf( "Plum origin %i %i %i -- %i\n", (int)org[0], (int)org[1], (int)org[2], (int)Distance(org, lastPos));
	VectorCopy(org, lastPos);


	re = &le->refEntity;

	re->reType = RT_SPRITE;
	re->radius = 16;

	VectorClear(angles);
	AnglesToAxis( angles, re->axis );
}


/*
====================
CG_MakeExplosion
====================
*/
localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir, 
								qhandle_t hModel, qhandle_t shader,
								int msec, qboolean isSprite ) {
	float			ang;
	localEntity_t	*ex;
	int				offset;
	vec3_t			tmpVec, newOrigin;

	if ( msec <= 0 ) {
		CG_Error( "CG_MakeExplosion: msec = %i", msec );
	}

	// skew the time a bit so they aren't all in sync
	offset = rand() & 63;

	ex = CG_AllocLocalEntity();
	if ( isSprite ) {
		ex->leType = LE_SPRITE_EXPLOSION;

		// randomly rotate sprite orientation
		ex->refEntity.rotation = rand() % 360;
		VectorScale( dir, 16, tmpVec );
		VectorAdd( tmpVec, origin, newOrigin );
	} else {
		ex->leType = LE_EXPLOSION;
		VectorCopy( origin, newOrigin );

		// set axis with random rotate
		if ( !dir ) {
			AxisClear( ex->refEntity.axis );
		} else {
			ang = rand() % 360;
			VectorCopy( dir, ex->refEntity.axis[0] );
			RotateAroundDirection( ex->refEntity.axis, ang );
		}
	}

	ex->startTime = cg.time - offset;
	ex->endTime = ex->startTime + msec;

	// bias the time so all shader effects start correctly
	ex->refEntity.shaderTime = ex->startTime / 1000.0f;

	ex->refEntity.hModel = hModel;
	ex->refEntity.customShader = shader;

	// set origin
	VectorCopy( newOrigin, ex->refEntity.origin );
	VectorCopy( newOrigin, ex->refEntity.oldorigin );

	ex->color[0] = ex->color[1] = ex->color[2] = 1.0;

	return ex;
}

/*
====================
CG_SpawnExplosionModel
====================
*/
localEntity_t *CG_SpawnExplosionModel( vec3_t origin, vec3_t dir, leType_t type, qhandle_t hModel, qhandle_t shader, float duration ) { // BFP - Explosion sphere, shell and ring effect
	localEntity_t	*le;
	vec3_t			newOrigin;

	le = CG_AllocLocalEntity();

	le->leType = type;
	
	le->refEntity.hModel = hModel;
	le->refEntity.customShader = shader;

	VectorCopy( origin, newOrigin );

	if ( !dir ) {
		AxisClear( le->refEntity.axis );
	} else {
		VectorCopy( dir, le->refEntity.axis[0] );
		RotateAroundDirection( le->refEntity.axis, 0 );
	}

	VectorCopy( origin, le->refEntity.origin );
	VectorCopy( newOrigin, le->refEntity.oldorigin );

	le->refEntity.reType = RT_MODEL;

	le->color[0] = le->color[1] = le->color[2] = 1.0;

	le->startTime = cg.time - (rand() & 63);
	le->lifeRate = duration;
	le->endTime = le->startTime + le->lifeRate;

	// bias the time so all shader effects start correctly
	le->refEntity.shaderTime = cg.time / 1000.0f;

	return le;
}


/*
=================
CG_Bleed

This is the spurt of blood when a character gets hit
=================
*/
void CG_Bleed( vec3_t origin, int entityNum ) {
	localEntity_t	*ex;

	if ( !cg_blood.integer ) {
		return;
	}

	ex = CG_AllocLocalEntity();
	ex->leType = LE_EXPLOSION;

	ex->startTime = cg.time;
	ex->endTime = ex->startTime + 500;
	
	VectorCopy ( origin, ex->refEntity.origin);
	ex->refEntity.reType = RT_SPRITE;
	ex->refEntity.rotation = rand() % 360;
	ex->refEntity.radius = 24;

	ex->refEntity.customShader = cgs.media.bloodExplosionShader;

	// don't show player's own blood in view
	if ( entityNum == cg.snap->ps.clientNum ) {
		ex->refEntity.renderfx |= RF_THIRD_PERSON;
	}
}



/*
==================
CG_LaunchGib
==================
*/
void CG_LaunchGib( vec3_t origin, vec3_t velocity, qhandle_t hModel ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + 5000 + random() * 3000;

	VectorCopy( origin, re->origin );
	AxisCopy( axisDefault, re->axis );
	re->hModel = hModel;

	le->pos.trType = TR_GRAVITY;
	VectorCopy( origin, le->pos.trBase );
	VectorCopy( velocity, le->pos.trDelta );
	le->pos.trTime = cg.time;

	le->bounceFactor = 0.6f;

	le->leBounceSoundType = LEBS_BLOOD;
	le->leMarkType = LEMT_BLOOD;
}

/*
===================
CG_GibPlayer

Generated a bunch of gibs launching out from the bodies location
===================
*/
#define	GIB_VELOCITY	250
#define	GIB_JUMP		250
void CG_GibPlayer( vec3_t playerOrigin ) {
	vec3_t	origin, velocity;

	if ( !cg_blood.integer ) {
		return;
	}

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	if ( rand() & 1 ) {
		CG_LaunchGib( origin, velocity, cgs.media.gibSkull );
	} else {
		CG_LaunchGib( origin, velocity, cgs.media.gibBrain );
	}

	// allow gibs to be turned off for speed
	if ( !cg_gibs.integer ) {
		return;
	}

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibAbdomen );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibArm );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibChest );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibFist );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibFoot );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibForearm );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibIntestine );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibLeg );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibLeg );
}

/*
==================
CG_LaunchGib
==================
*/
void CG_LaunchExplode( vec3_t origin, vec3_t velocity, qhandle_t hModel ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + 10000 + random() * 6000;

	VectorCopy( origin, re->origin );
	AxisCopy( axisDefault, re->axis );
	re->hModel = hModel;

	le->pos.trType = TR_GRAVITY;
	VectorCopy( origin, le->pos.trBase );
	VectorCopy( velocity, le->pos.trDelta );
	le->pos.trTime = cg.time;

	le->bounceFactor = 0.1f;

	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;
}

#define	EXP_VELOCITY	100
#define	EXP_JUMP		150
/*
===================
CG_GibPlayer

Generated a bunch of gibs launching out from the bodies location
===================
*/
void CG_BigExplode( vec3_t playerOrigin ) {
	vec3_t	origin, velocity;

	if ( !cg_blood.integer ) {
		return;
	}

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*EXP_VELOCITY;
	velocity[1] = crandom()*EXP_VELOCITY;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*EXP_VELOCITY;
	velocity[1] = crandom()*EXP_VELOCITY;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*EXP_VELOCITY*1.5;
	velocity[1] = crandom()*EXP_VELOCITY*1.5;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*EXP_VELOCITY*2.0;
	velocity[1] = crandom()*EXP_VELOCITY*2.0;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy( playerOrigin, origin );
	velocity[0] = crandom()*EXP_VELOCITY*2.5;
	velocity[1] = crandom()*EXP_VELOCITY*2.5;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );
}


/*
=================
CG_DebrisExplosion
=================
*/
void CG_DebrisExplosion( vec3_t origin, vec3_t dir ) { // BFP - Debris particles explosion
	int				i;
	vec3_t			sprOrg, sprVel;
	int				numRocks = 26;

	// BFP - NOTE: Debris particles shouldn't be used for bullet and disk weapon types

	// BFP - TODO: Apply number of rocks as indicated on default.cfg file of some character: explosionRocks <weaponNum> <numRocks>

	for ( i = 0; i < numRocks; ++i ) {
		// spawn randomly the shaders with the particles
		int shaderIndex = rand() % 3;

		// that would be the range for debris particles
		VectorMA( origin, 24, dir, sprOrg );
		sprOrg[0] += (rand() % 24);
		sprOrg[1] += (rand() % 24);
		sprOrg[2] += (rand() % 24);

		VectorScale( dir, 1500 + (rand() % 1000), sprVel );
		sprVel[0] += (rand() % 2800) - 1500;
		sprVel[1] += (rand() % 2800) - 1500;
		sprVel[2] += (rand() % 2200) - 1000;

		switch ( shaderIndex ) {
			case 0: {
				CG_ParticleDebris( cgs.media.pebbleShader1, sprOrg, sprVel, qfalse );
				break;
			}
			case 1: {
				CG_ParticleDebris( cgs.media.pebbleShader2, sprOrg, sprVel, qfalse );
				break;
			}
			default: {
				CG_ParticleDebris( cgs.media.pebbleShader3, sprOrg, sprVel, qfalse );
			}
		}
	}
}

/*
=================
CG_SparksExplosion
=================
*/
void CG_SparksExplosion( vec3_t origin, vec3_t dir ) { // BFP - Spark particles explosion
	int				i;
	// spawn randomly the shaders with the particles
	int				shaderIndex;
	vec3_t			sparkOrg, sparkVel;
	int				numSparks = 26;

	// BFP - NOTE: Spark particles shouldn't be used on bullet and disk weapon types

	// BFP - TODO: Apply calling this function for finger blast and these rail gun weapon types when hitting a player

	// BFP - TODO: Apply number of sparks as indicated on default.cfg file of some character: explosionSparks <weaponNum> <numSparks>

	for ( i = 0; i < numSparks; ++i ) {
		shaderIndex = (rand() % 100) < 50 ? 0 : 1; // if the random range was rand() % 2, it would repeat the pattern without randomize correctly

		VectorMA( origin, 0, dir, sparkOrg );

		// move faster
		VectorScale( dir, 1500 + (rand() % 1000), sparkVel );
		sparkVel[0] += 1.5 * (rand() % 3500) - 1500;
		sparkVel[1] += 1.5 * (rand() % 3500) - 1500;
		sparkVel[2] += 1.5 * (rand() % 2100) - 1000;

		switch ( shaderIndex ) {
			case 0: {
				CG_ParticleSparks( cgs.media.sparkShader1, sparkOrg, sparkVel );
				break;
			}
			default: {
				CG_ParticleSparks( cgs.media.sparkShader2, sparkOrg, sparkVel );
			}
		}
	}
}

/*
=================
CG_ExplosionSound
=================
*/
void CG_ExplosionSound( vec3_t origin ) { // BFP - Explosion sounds
	int	i = rand() % 6;

	switch ( i ) {
	case 0:  trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.explosion1Sound ); break;
	case 1:  trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.explosion2Sound ); break;
	case 2:  trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.explosion3Sound ); break;
	case 3:  trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.explosion4Sound ); break;
	case 4:  trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.explosion5Sound ); break;
	default: trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.explosion6Sound ); break;
	}
}


/*
=================
CG_ExplosionEffect
=================
*/
void CG_ExplosionEffect( vec3_t origin, vec3_t dir ) { // BFP - Explosion effects
	// BFP - Low poly sphere
	// BFP - TODO: Apply explosionModel from bfp attack config, bfgExplosionShader is just a test
	qhandle_t	sphereModel = ( cg_lowpolysphere.integer > 0 ) ? cgs.media.lowPolySphereModel : cgs.media.highPolySphereModel;
	localEntity_t *leSphere, *leRing, *leShell;

	VectorMA( origin, 10, dir, origin );

	// BFP - TODO: Apply explosionShader from bfp attack config, bfgExplosionShader is just a test
	leSphere = CG_SpawnExplosionModel( origin, dir, LE_EXPLOSION_SPHERE, sphereModel, cgs.media.bfgExplosionShader, 1000 );
	if ( cg_explosionShell.integer > 0 ) { // BFP - Explosion shell
		leShell = CG_SpawnExplosionModel( origin, dir, LE_EXPLOSION_SHELL, sphereModel, cgs.media.explosionShellShader, 250 );
	}
	if ( cg_explosionRing.integer > 0 ) { // BFP - Explosion ring
		leRing = CG_SpawnExplosionModel( origin, dir, LE_EXPLOSION_RING, cgs.media.ringFlashModel, cgs.media.railExplosionShader, 500 );
	}
	if ( cg_explosionSmoke.integer > 0 ) { // BFP - Explosion smoke
		// BFP - TODO: Apply explosionSmoke as indicated on default.cfg file from some character: explosionSmoke <weaponNum> <numSmokes(int)>
		int	i, numSmokes = 3;
		// BFP - TODO: Apply explosionSmokeRadius as indicated on default.cfg file from some character: explosionSmokeRadius <weaponNum> <radius(int)>
		int	explosionSmokeRadius = 200;
		// BFP - TODO: Apply explosionSmokeLife as indicated on default.cfg file from some character: explosionSmokeLife <weaponNum> <lifetime(int)>
		int	explosionSmokeLife = 1500;
		// BFP - TODO: Apply explosionSmokeSpeed as indicated on default.cfg file from some character: explosionSmokeSpeed <weaponNum> <initialSpeed(int)>
		int	explosionSmokeSpeed = 10;
		for ( i = 0; i < numSmokes; i++ ) {
			localEntity_t	*leSmoke;
			vec3_t	vel, smokeOrg;
			static int	timenonscaled;

			timenonscaled = trap_Milliseconds(); // BFP - That's what the variable makes non-timescaled

			VectorCopy( origin, smokeOrg );
			smokeOrg[0] += ( crandom() * 125 );
			smokeOrg[1] += ( crandom() * 125 );
			smokeOrg[2] += ( crandom() * 25 );

			vel[0] = vel[1] = 0;
			vel[2] = 50 * explosionSmokeSpeed;

			leSmoke = CG_SmokePuff( smokeOrg, vel, 
				explosionSmokeRadius,
				1, 1, 1, 0.33f,
				explosionSmokeLife,
				timenonscaled, 0, 0,
				cgs.media.particleSmokeShader );

			// change to this type, don't use the common smoke puff
			leSmoke->leType = LE_MOVE_DONT_SCALE_FADE;
		}
	}

	if ( cg_bigExplosions.integer > 0 ) { // BFP - Big explosions
		const float	MAX_SCALE = 25.0f, MAX_SCALEFACTOR = 6.0f; // limits to prevent too large scaling
		float	scale = 1;
		int		numPointsChargedOverMin = 1; // that means when reaching to 'READY!', it starts as 1 and if it's charging another charge point, adds 1 more
		// BFP - TODO: Apply explosionScaleFactor as indicated on default.cfg file from some character: explosionScaleFactor <weaponNum> <scaleFactor>
		// BFP - TODO: Apply explosionScaleFactorChargeMult as indicated on default.cfg file from some character: explosionScaleFactorChargeMult <weaponNum> <scaleFactor>
		float	explosionScaleFactor = 0.95, explosionScaleFactorChargeMult = 0;
		// BFP - TODO: Apply explosionRingScaleFactor from default.cfg file from some character: explosionRingScaleFactor <weaponNum> <scaleFactor>
		// BFP - TODO: Apply explosionRingScaleFactorChargeMult from default.cfg file from some character: explosionRingScaleFactorChargeMult <weaponNum> <scaleFactor>
		float	explosionRingScaleFactor = 0.95, explosionRingScaleFactorChargeMult = 0;
		// BFP - TODO: Apply explosionShellScaleFactor from default.cfg file from some character: explosionShellScaleFactor <weaponNum> <scaleFactor>
		// BFP - TODO: Apply explosionShellScaleFactorChargeMult from default.cfg file from some character: explosionShellScaleFactorChargeMult <weaponNum> <scaleFactor>
		float	explosionShellScaleFactor = 0.95, explosionShellScaleFactorChargeMult = 0;

		if ( explosionScaleFactor > MAX_SCALEFACTOR ) explosionScaleFactor = MAX_SCALEFACTOR;
		if ( explosionScaleFactorChargeMult > MAX_SCALEFACTOR ) explosionScaleFactorChargeMult = MAX_SCALEFACTOR;
		scale = explosionScaleFactor + explosionScaleFactorChargeMult * numPointsChargedOverMin;
		if ( scale > MAX_SCALE ) scale = MAX_SCALE;
		VectorScale( leSphere->refEntity.axis[0], scale, leSphere->refEntity.axis[0] );
		VectorScale( leSphere->refEntity.axis[1], scale, leSphere->refEntity.axis[1] );
		VectorScale( leSphere->refEntity.axis[2], scale, leSphere->refEntity.axis[2] );

		if ( explosionRingScaleFactor > MAX_SCALEFACTOR ) explosionRingScaleFactor = MAX_SCALEFACTOR;
		if ( explosionRingScaleFactorChargeMult > MAX_SCALEFACTOR ) explosionRingScaleFactorChargeMult = MAX_SCALEFACTOR;
		scale = explosionRingScaleFactor + explosionRingScaleFactorChargeMult * numPointsChargedOverMin;
		if ( scale > MAX_SCALE ) scale = MAX_SCALE;
		VectorScale( leRing->refEntity.axis[0], scale, leRing->refEntity.axis[0] );
		VectorScale( leRing->refEntity.axis[1], scale, leRing->refEntity.axis[1] );
		VectorScale( leRing->refEntity.axis[2], scale, leRing->refEntity.axis[2] );

		if ( explosionShellScaleFactor > MAX_SCALEFACTOR ) explosionShellScaleFactor = MAX_SCALEFACTOR;
		if ( explosionShellScaleFactorChargeMult > MAX_SCALEFACTOR ) explosionShellScaleFactorChargeMult = MAX_SCALEFACTOR;
		scale = explosionShellScaleFactor + explosionShellScaleFactorChargeMult * numPointsChargedOverMin;
		if ( scale > MAX_SCALE ) scale = MAX_SCALE;
		VectorScale( leShell->refEntity.axis[0], scale, leShell->refEntity.axis[0] );
		VectorScale( leShell->refEntity.axis[1], scale, leShell->refEntity.axis[1] );
		VectorScale( leShell->refEntity.axis[2], scale, leShell->refEntity.axis[2] );
	}

	// BFP - Apply dynamic explosion light values
	leSphere->light = 700;
	leSphere->lightColor[0] = 1;
	leSphere->lightColor[1] = 0.75;
	leSphere->lightColor[2] = 0.0;
}
