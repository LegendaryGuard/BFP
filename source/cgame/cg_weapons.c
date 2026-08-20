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
// cg_weapons.c -- events and effects dealing with weapons
#include "cg_local.h"

// BFP - HIGHLY MODIFIED

// BFP - A macro to debug flash/firing flash/missile scale/size
#define	FLASH_MISSILE_SCALE_DEBUG	0
/*
===================
CG_AddFlash

Adds normal (or charging) flash shader/model
===================
*/
void CG_AddFlash( centity_t *cent, int entityNum, bfpAttackSkinConfig_t *skinAtkCfg, vec3_t origin, refEntity_t *parent, char *tagName ) { // BFP - Flash shader/model
	refEntity_t	flash;
	qhandle_t	flashShader, flashModel;
	float		flashRadius, flashScaleFactor;
	float		scale = 0;
	int			minCharge = 0, totalCharge = 0;
	bfpWeapon_t	*wpCfg = CG_GetBFPWeaponForSlot( cent->currentState.clientNum, cent->currentState.weapon );

	minCharge = ( wpCfg && wpCfg->minCharge > 0 ) ? wpCfg->minCharge : 0;
	totalCharge = cent->currentState.generic1 - minCharge;

	if ( !skinAtkCfg ) {
		return;
	}

	flashRadius = skinAtkCfg->flashRadius;
	flashScaleFactor = skinAtkCfg->flashScaleFactor;
	flashShader = skinAtkCfg->flashShader;
	flashModel = skinAtkCfg->flashModel;

	// don't show the muzzle flash to the player itself on first person camera, even on first person vis mode
	if ( cg_thirdPerson.integer <= 0 && entityNum == cg.snap->ps.clientNum ) {
		return;
	}

	if ( totalCharge < 0 ) {
		totalCharge = 0;
	}

	if ( flashRadius < 0 ) {
		flashRadius = 0;
	}
	if ( flashScaleFactor < 0 ) {
		flashScaleFactor = 0;
	}

	memset( &flash, 0, sizeof( flash ) );
	VectorCopy( origin, flash.origin );

	if ( flashModel ) {
		flash.reType = RT_MODEL;
		flash.hModel = flashModel;

		if ( parent ) {
			AxisCopy( parent->axis, flash.axis );
		} else {
			AxisClear( flash.axis );
		}

		scale = flashScaleFactor;
#if FLASH_MISSILE_SCALE_DEBUG
		CG_Printf( "Flash model scale: %f\n", scale );
#endif
		if ( scale > 1 ) {
			scale = 1;
		}
		if ( scale <= 0 ) {
			scale = 1;
		}
		CG_ModelSize( &flash, scale );
	} else {
		flash.reType = RT_SPRITE;
		scale = flashRadius;
#if FLASH_MISSILE_SCALE_DEBUG
		CG_Printf( "Flash sprite size: %f\n", scale );
#endif
		if ( scale > 500 ) {
			scale = 50;
		}
		// BFP - Make muzzle flash fit better for player monster
		if ( ( cent->currentState.eFlags & EF_MONSTER )
		|| ( cg_entities[ entityNum ].currentState.eFlags & EF_MONSTER ) ) {
			scale *= 6;
			if ( scale > 400 ) {
				scale = 400;
			}
		}
		if ( scale <= 0 ) {
			scale = 1;
		}
		flash.radius = scale;
	}

	if ( !flashShader && flash.reType == RT_SPRITE ) {
		return;
	}
	if ( flashShader ) {
		flash.customShader = flashShader;
	}
	flash.shaderRGBA[0] = 0xff;
	flash.shaderRGBA[1] = 0xff;
	flash.shaderRGBA[2] = 0xff;
	flash.shaderRGBA[3] = 0xff;
	trap_R_AddRefEntityToScene( &flash );
}

/*
===================
CG_AddFiringFlash

Adds firing flash shader/model (used for continuous beam attacks)
===================
*/
void CG_AddFiringFlash( centity_t *cent, int entityNum, bfpAttackSkinConfig_t *skinAtkCfg, vec3_t origin, refEntity_t *parent, char *tagName ) {
	refEntity_t	firingFlash;
	qhandle_t	flashShader, flashModel;
	float		firingFlashRadius, firingFlashScaleFactor;
	float		scale = 0;
	int			minCharge = 0, totalCharge = 0;
	bfpWeapon_t	*wpCfg = CG_GetBFPWeaponForSlot( cent->currentState.clientNum, cent->currentState.weapon );

	minCharge = ( wpCfg && wpCfg->minCharge > 0 ) ? wpCfg->minCharge : 0;
	totalCharge = cent->currentState.generic1 - minCharge;

	if ( !skinAtkCfg ) {
		return;
	}

	firingFlashRadius = skinAtkCfg->firingFlashRadius;
	firingFlashScaleFactor = skinAtkCfg->firingFlashScaleFactor;
	flashShader = skinAtkCfg->flashShader;
	flashModel = skinAtkCfg->flashModel;

	if ( cg_thirdPerson.integer <= 0 && entityNum == cg.snap->ps.clientNum ) {
		return;
	}

	if ( totalCharge < 0 ) {
		totalCharge = 0;
	}

	if ( firingFlashRadius < 0 ) {
		firingFlashRadius = 0;
	}
	if ( firingFlashScaleFactor < 0 ) {
		firingFlashScaleFactor = 0;
	}

	memset( &firingFlash, 0, sizeof( firingFlash ) );
	VectorCopy( origin, firingFlash.origin );

	if ( flashModel ) {
		firingFlash.reType = RT_MODEL;
		firingFlash.hModel = flashModel;

		if ( parent ) {
			AxisCopy( parent->axis, firingFlash.axis );
		} else {
			AxisClear( firingFlash.axis );
		}

		scale = firingFlashScaleFactor;
#if FLASH_MISSILE_SCALE_DEBUG
		CG_Printf( "Firing flash scale: %f\n", scale );
#endif
		if ( scale <= 0 ) {
			scale = 1;
		}
		CG_ModelSize( &firingFlash, scale );
	} else {
		firingFlash.reType = RT_SPRITE;

		scale = firingFlashRadius;
#if FLASH_MISSILE_SCALE_DEBUG
		CG_Printf( "Firing flash size: %f\n", scale );
#endif
		// BFP - Make firing flash fit better for player monster
		if ( ( cent->currentState.eFlags & EF_MONSTER )
		|| ( cg_entities[ entityNum ].currentState.eFlags & EF_MONSTER ) ) {
			scale *= 3;
			if ( scale > 300 ) {
				scale = 300;
			}
		}
		if ( scale <= 0 ) {
			scale = 1;
		}
		firingFlash.radius = scale;
	}

	if ( !flashShader && firingFlash.reType == RT_SPRITE ) {
		return;
	}
	if ( flashShader ) {
		firingFlash.customShader = flashShader;
	}
	firingFlash.shaderRGBA[0] = 0xff;
	firingFlash.shaderRGBA[1] = 0xff;
	firingFlash.shaderRGBA[2] = 0xff;
	firingFlash.shaderRGBA[3] = 0xff;
	trap_R_AddRefEntityToScene( &firingFlash );
}

/*
===================
CG_AddMissile

Adds missile shader/model
===================
*/
void CG_AddMissile( centity_t *cent, int entityNum, qboolean isMissileMoving, bfpAttackSkinConfig_t *skinAtkCfg, vec3_t origin, refEntity_t *parent, char *tagName ) {
	refEntity_t	missile;
	qhandle_t	missileShader, missileModel;
	float		missileRadius, missileScaleFactor, missileRadiusChargeMult, missileScaleFactorChargeMult;
	float		scale = 0;
	int			minCharge = 0, totalCharge = 0;
	bfpWeapon_t	*wpCfg = CG_GetBFPWeaponForSlot( cent->currentState.clientNum, cent->currentState.weapon );

	minCharge = ( wpCfg && wpCfg->minCharge > 0 ) ? wpCfg->minCharge : 0;
	totalCharge = cent->currentState.generic1 - minCharge;

	if ( !skinAtkCfg ) {
		return;
	}

	missileShader = skinAtkCfg->missileShader;
	missileModel = skinAtkCfg->missileModel;
	missileRadius = skinAtkCfg->missileRadius;
	missileRadiusChargeMult = skinAtkCfg->missileRadiusChargeMult;
	missileScaleFactor = skinAtkCfg->missileScaleFactor;
	missileScaleFactorChargeMult = skinAtkCfg->missileScaleFactorChargeMult;

	if ( cg_thirdPerson.integer <= 0 && entityNum == cg.snap->ps.clientNum ) {
		return;
	}

	if ( totalCharge < 0 ) {
		totalCharge = 0;
	}

	if ( missileRadius < 0 ) {
		missileRadius = 0;
	}
	if ( missileScaleFactor < 0 ) {
		missileScaleFactor = 0;
	}

	memset( &missile, 0, sizeof( missile ) );
	VectorCopy( origin, missile.origin );

	if ( missileModel ) {
		missile.reType = RT_MODEL;
		missile.hModel = missileModel;

		if ( parent ) {
			AxisCopy( parent->axis, missile.axis );
		} else {
			AxisClear( missile.axis );
		}

		scale = missileScaleFactor + missileScaleFactorChargeMult * (float)totalCharge;
#if FLASH_MISSILE_SCALE_DEBUG
		CG_Printf( "Missile model scale: %f\n", scale );
#endif
		if ( isMissileMoving && VectorNormalize2( cent->currentState.pos.trDelta, missile.axis[0] ) == 0 ) {
			missile.axis[0][2] = 1;
		}
		if ( !skinAtkCfg->missileSpinHoriz ) {
			RotateAroundDirection( missile.axis, cg.time * skinAtkCfg->missileModelRotation );
		} else {
			vec3_t	temp;
			RotateAroundDirection( missile.axis, cent->currentState.time );
			VectorCopy( missile.axis[0], temp );
			RotatePointAroundVector( missile.axis[0], missile.axis[2], temp, cg.autoAnglesFast[1] );
			VectorCopy( missile.axis[1], temp );
			RotatePointAroundVector( missile.axis[1], missile.axis[2], temp, cg.autoAnglesFast[1] );
		}
		if ( scale <= 0 ) {
			scale = 1;
		}
		CG_ModelSize( &missile, scale );
	} else {
		missile.reType = RT_SPRITE;
		scale = missileRadius + missileRadiusChargeMult * (float)totalCharge;
#if FLASH_MISSILE_SCALE_DEBUG
		CG_Printf( "Missile sprite size: %f\n", scale );
#endif
		if ( scale > 150 ) {
			scale = 150;
		}
		missile.rotation = skinAtkCfg->missileRotation;
		if ( scale <= 0 ) {
			scale = 1;
		}
		missile.radius = scale;
	}

	if ( !missileShader && missile.reType == RT_SPRITE ) {
		return;
	}
	if ( missileShader ) {
		missile.customShader = missileShader;
	}
	missile.shaderRGBA[0] = 0xff;
	missile.shaderRGBA[1] = 0xff;
	missile.shaderRGBA[2] = 0xff;
	missile.shaderRGBA[3] = 0xff;
	trap_R_AddRefEntityToScene( &missile );
}


/*
==========================
CG_RailTrail
==========================
*/
void CG_RailTrail ( clientInfo_t *ci, vec3_t start, vec3_t end ) { // BFP - BFP uses an old version of rail trail on Quake 3 Arena 1st version release
	localEntity_t *le;
	refEntity_t   *re;

	//
	// rings
	//
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + cg_railTrailTime.value;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_RAIL_RINGS;
	re->customShader = cgs.media.railRingsShader;

	VectorCopy( start, re->origin );
	VectorCopy( end, re->oldorigin );

	// nudge down a bit so it isn't exactly in center
	re->origin[2] -= 8;
	re->oldorigin[2] -= 8;

	// BFP - No color1
#if 0
	le->color[0] = ci->color1[0] * 0.75;
	le->color[1] = ci->color1[1] * 0.75;
	le->color[2] = ci->color1[2] * 0.75;
#endif
	le->color[0] = 0.75;
	le->color[1] = 0;
	le->color[2] = 0;
	le->color[3] = 1.0f;

	AxisClear( re->axis );

	//
	// core
	//
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + cg_railTrailTime.value;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_RAIL_CORE;
	re->customShader = cgs.media.railCoreShader;

	VectorCopy( start, re->origin );
	VectorCopy( end, re->oldorigin );

	// nudge down a bit so it isn't exactly in center
	re->origin[2] -= 8;
	re->oldorigin[2] -= 8;

	// BFP - No color1
#if 0
	le->color[0] = ci->color1[0] * 0.75;
	le->color[1] = ci->color1[1] * 0.75;
	le->color[2] = ci->color1[2] * 0.75;
#endif
	le->color[0] = 0.75;
	le->color[1] = 0;
	le->color[2] = 0;
	le->color[3] = 1.0f;

	AxisClear( re->axis );
}

/*
==========================
CG_RocketTrail
==========================
*/
void CG_RocketTrail( centity_t *ent, bfpAttackSkinConfig_t *skinAtkCfg ) {
	vec3_t	origin, lastPos;
	int		lastContents, contents;
	entityState_t	*es;

	// BFP - NOTE: cg_oldRocketTrail is for these who wanna to play Q3 rocket smoke trails

	int		step;
	int		t;
	int		startTime;
	vec3_t	up;
	localEntity_t	*smoke;
	vec3_t	color = {1, 0.75, 0}; // BFP - Color for missile trail
	color[0] = skinAtkCfg->missileDlightColor[0];
	color[1] = skinAtkCfg->missileDlightColor[1];
	color[2] = skinAtkCfg->missileDlightColor[2];

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 50;

	startTime = ent->trailTime;
	t = step * ( (startTime + step) / step );

	if ( cg_noProjectileTrail.integer ) {
		return;
	}

	es = &ent->currentState;

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if ( es->pos.trType == TR_STATIONARY ) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
	lastContents = CG_PointContents( lastPos, -1 );

	// BFP - Missile trail
	if ( cg_oldRocketTrail.integer <= 0 ) {
		CG_MissileTrail( ent->currentState.number, origin, skinAtkCfg->missileTrailRadius, cgs.media.railCoreShader, color, qfalse );
	}

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		if ( ( contents & lastContents & CONTENTS_WATER ) 
		&& cg.frametime > 0.0f ) { // BFP - If paused, don't spawn bubble particles (cg_paused.integer < 1 is another solution, but not good enough for server responses)
			// BFP - Apply particle bubble effect in that case
			CG_ParticleBubble( ent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, origin, lastPos, 900, 10, 2 );
			CG_ParticleBubble( ent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, origin, lastPos, 900, 10, 2 );
			CG_ParticleBubble( ent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, origin, lastPos, 900, 10, 2 );
			// CG_BubbleTrail( lastPos, origin, 8 );
		}
		return;
	}

	if ( cg_oldRocketTrail.integer > 0 ) {
		ent->trailTime = cg.time;

		for ( ; t <= ent->trailTime ; t += step ) {
			BG_EvaluateTrajectory( &es->pos, t, lastPos );

			smoke = CG_SmokePuff( lastPos, up, 
						skinAtkCfg->missileTrailRadius, 
						1, 1, 1, 0.33f,
						skinAtkCfg->missileTrailTime, 
						t,
						0,
						0, 
						cgs.media.smokePuffShader );
			// use the optimized local entity add
			smoke->leType = LE_SCALE_FADE;
		}
	}
}


/*
==========================
CG_BFPBeamTrail
==========================
*/
void CG_BFPBeamTrail( centity_t *ent, bfpAttackSkinConfig_t *skinAtkCfg ) { // BFP - BFP Beam trail handling
	vec3_t	origin, muzzleOrigin;
	entityState_t	*es;
	bfpWeapon_t		*wpCfg;

	es = &ent->currentState;
	wpCfg = CG_GetBFPWeaponForSlot( es->clientNum, es->weapon );
	if ( wpCfg && ( wpCfg->attackType == ATK_MISSILE || wpCfg->attackType == ATK_RDMISSILE )
	// BFP - Monster gamemode, avoid detaching the muzzle origin from its mouth
	&& !( cgs.gametype == GT_MONSTER && cgs.monster > 0 
		&& ( cg_entities[ es->clientNum ].currentState.eFlags & EF_MONSTER ) ) ) {
		VectorCopy( ent->pe.muzzleOrigin, muzzleOrigin );
	} else {
		VectorCopy( cg_entities[ es->clientNum ].pe.muzzleOrigin, muzzleOrigin );
	}

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	// ent->trailTime = cg.time;

	// BFP - NOTE: That's where we apply the flash properties read from skin config
	CG_AddFiringFlash( ent, es->clientNum, skinAtkCfg, muzzleOrigin, NULL, "" );
	CG_AddMissile( ent, es->number, qtrue, skinAtkCfg, origin, NULL, "" );

	CG_BeamTrail( es->number, origin, muzzleOrigin, skinAtkCfg->beamShader );
}

/*
==========================
CG_BFPSpiralBeamTrail
==========================
*/
void CG_BFPSpiralBeamTrail( centity_t *ent, bfpAttackSkinConfig_t *skinAtkCfg ) { // BFP - BFP Spiral beam trail handling
	vec3_t	origin, muzzleOrigin;
	entityState_t	*es;
	bfpWeapon_t		*wpCfg;

	es = &ent->currentState;
	wpCfg = CG_GetBFPWeaponForSlot( es->clientNum, es->weapon );
	if ( wpCfg && ( wpCfg->attackType == ATK_MISSILE || wpCfg->attackType == ATK_RDMISSILE ) ) {
		VectorCopy( ent->pe.muzzleOrigin, muzzleOrigin );
	} else {
		VectorCopy( cg_entities[ es->clientNum ].pe.muzzleOrigin, muzzleOrigin );
	}

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	// ent->trailTime = cg.time;

	// BFP - NOTE: That's where we apply the flash properties read from skin config
	CG_AddFiringFlash( ent, es->clientNum, skinAtkCfg, muzzleOrigin, NULL, "" );
	CG_AddMissile( ent, es->number, qtrue, skinAtkCfg, origin, NULL, "" );

	CG_CorkscrewTrail( es->number, origin, muzzleOrigin, skinAtkCfg->beamShader, skinAtkCfg->spiralBeamShader );
}

/*
=================
CG_RegisterItemVisuals

The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals( int itemNum ) {
	itemInfo_t		*itemInfo;
	gitem_t			*item;

	if ( itemNum < 0 || itemNum >= bg_numItems ) {
		CG_Error( "CG_RegisterItemVisuals: itemNum %d out of range [0-%d]", itemNum, bg_numItems-1 );
	}

	itemInfo = &cg_items[ itemNum ];
	if ( itemInfo->registered ) {
		return;
	}

	item = &bg_itemlist[ itemNum ];

	memset( itemInfo, 0, sizeof( &itemInfo ) );
	itemInfo->registered = qtrue;

	itemInfo->models[0] = trap_R_RegisterModel( item->world_model[0] );

	itemInfo->icon = trap_R_RegisterShader( item->icon );

	// BFP - CG_RegisterWeapon is removed
	//if ( item->giType == IT_WEAPON ) {
		//CG_RegisterWeapon( item->giTag );
	//}

	//
	// powerups have an accompanying ring or sphere
	//
	if ( item->giType == IT_POWERUP || item->giType == IT_HEALTH || 
		item->giType == IT_ARMOR || item->giType == IT_HOLDABLE ) {
		if ( item->world_model[1] ) {
			itemInfo->models[1] = trap_R_RegisterModel( item->world_model[1] );
		}
	}
}


/*
========================================================================================

VIEW WEAPON

========================================================================================
*/

// BFP - No CG_MapTorsoToWeaponFrame and CG_CalculateWeaponPosition
#if 0
/*
=================
CG_MapTorsoToWeaponFrame

=================
*/
static int CG_MapTorsoToWeaponFrame( clientInfo_t *ci, int frame ) {

	// BFP - That was changed for the animations, but in that case, it isn't necessary

	// change weapon
	// BFP doesn't use this animation
	if ( frame >= ci->animations[TORSO_ATTACK0_PREPARE].firstFrame 
		&& frame < ci->animations[TORSO_ATTACK0_PREPARE].firstFrame + 9 ) {
		return frame - ci->animations[TORSO_ATTACK0_PREPARE].firstFrame + 6;
	}

	// stand attack
	if ( frame >= ci->animations[TORSO_STAND].firstFrame 
		&& frame < ci->animations[TORSO_STAND].firstFrame + 6 ) {
		return 1 + frame - ci->animations[TORSO_STAND].firstFrame;
	}

	// stand attack 2
	if ( frame >= ci->animations[TORSO_ATTACK0_PREPARE].firstFrame 
		&& frame < ci->animations[TORSO_ATTACK0_PREPARE].firstFrame + 6 ) {
		return 1 + frame - ci->animations[TORSO_ATTACK0_PREPARE].firstFrame;
	}
	
	return 0;
}


/*
==============
CG_CalculateWeaponPosition
==============
*/
static void CG_CalculateWeaponPosition( vec3_t origin, vec3_t angles ) {
	float	scale;
	int		delta;
	float	fracsin;

	VectorCopy( cg.refdef.vieworg, origin );
	VectorCopy( cg.refdefViewAngles, angles );

	// on odd legs, invert some angles
	if ( cg.bobcycle & 1 ) {
		scale = -cg.xyspeed;
	} else {
		scale = cg.xyspeed;
	}

	// gun angles from bobbing
	angles[ROLL] += scale * cg.bobfracsin * 0.005;
	angles[YAW] += scale * cg.bobfracsin * 0.01;
	angles[PITCH] += cg.xyspeed * cg.bobfracsin * 0.005;

	// drop the weapon when landing
	delta = cg.time - cg.landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		origin[2] += cg.landChange*0.25 * delta / LAND_DEFLECT_TIME;
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		origin[2] += cg.landChange*0.25 * 
			(LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME;
	}

#if 0
	// drop the weapon when stair climbing
	delta = cg.time - cg.stepTime;
	if ( delta < STEP_TIME/2 ) {
		origin[2] -= cg.stepChange*0.25 * delta / (STEP_TIME/2);
	} else if ( delta < STEP_TIME ) {
		origin[2] -= cg.stepChange*0.25 * (STEP_TIME - delta) / (STEP_TIME/2);
	}
#endif

	// idle drift
	scale = cg.xyspeed + 40;
	fracsin = sin( cg.time * 0.001 );
	angles[ROLL] += scale * fracsin * 0.01;
	angles[YAW] += scale * fracsin * 0.01;
	angles[PITCH] += scale * fracsin * 0.01;
}
#endif


/*
===============
CG_LightningBolt

Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
The cent should be the non-predicted cent if it is from the player,
so the endpoint will reflect the simulated strike (lagging the predicted
angle)
===============
*/
static void CG_LightningBolt( centity_t *cent, vec3_t origin, bfpAttackSkinConfig_t *skinAtkCfg ) {
	trace_t		trace;
	refEntity_t	beam;
	vec3_t		forward;
	vec3_t		muzzlePoint, endPoint;
	bfpWeapon_t	*wpCfg = CG_GetBFPWeaponForSlot( cent->currentState.clientNum, cent->currentState.weapon );
	float		range = wpCfg->range;

	if ( !wpCfg ) {
		range = 1;
	}

	if ( !skinAtkCfg ) {
		return;
	}

	if ( !skinAtkCfg->lightningBolt ) {
		return;
	}

	memset( &beam, 0, sizeof( beam ) );

	// !CPMA
	AngleVectors( cent->lerpAngles, forward, NULL, NULL );
	VectorCopy(cent->lerpOrigin, muzzlePoint );

	// project forward by the lightning range
	VectorMA( muzzlePoint, range, forward, endPoint );

	// see if it hit a wall
	CG_Trace( &trace, muzzlePoint, vec3_origin, vec3_origin, endPoint, 
		cent->currentState.number, MASK_SHOT );

	// this is the endpoint
	VectorCopy( trace.endpos, beam.oldorigin );

	// use the provided origin, even though it may be slightly
	// different than the muzzle origin
	VectorCopy( origin, beam.origin );

	beam.reType = RT_LIGHTNING;
	beam.customShader = skinAtkCfg->beamShader;
	beam.shaderRGBA[0] = 0xff;
	beam.shaderRGBA[1] = 0xff;
	beam.shaderRGBA[2] = 0xff;
	beam.shaderRGBA[3] = 0xff;
	trap_R_AddRefEntityToScene( &beam );

	// BFP - No impact flare effect
#if 0
	// add the impact flare if it hit something
	if ( trace.fraction < 1.0 ) {
		vec3_t	angles;
		vec3_t	dir;

		VectorSubtract( beam.oldorigin, beam.origin, dir );
		VectorNormalize( dir );

		memset( &beam, 0, sizeof( beam ) );
		beam.hModel = cgs.media.lightningExplosionModel;

		VectorMA( trace.endpos, -16, dir, beam.origin );

		// make a random orientation
		angles[0] = rand() % 360;
		angles[1] = rand() % 360;
		angles[2] = rand() % 360;
		AnglesToAxis( angles, beam.axis );
		trap_R_AddRefEntityToScene( &beam );
	}
#endif
}


/*
=============
CG_ChargingTorsoAnim
=============
*/
static qboolean CG_ChargingTorsoAnim( centity_t *cent, bfpWeapon_t *wpCfg ) { // BFP - To handle torso charging animation
	int	torsoAnim = cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT;

	if ( !wpCfg || wpCfg->chargeAutoFire ) {
		return qfalse;
	}

	if ( wpCfg->noAttackAnim ) {
		return ( torsoAnim == TORSO_ATTACK0_STRIKE
			|| torsoAnim == TORSO_ATTACK1_STRIKE
			|| torsoAnim == TORSO_ATTACK2_STRIKE
			|| torsoAnim == TORSO_ATTACK3_STRIKE
			|| torsoAnim == TORSO_ATTACK4_STRIKE );
	}

	return ( torsoAnim == TORSO_ATTACK0_PREPARE
		|| torsoAnim == TORSO_ATTACK1_PREPARE
		|| torsoAnim == TORSO_ATTACK2_PREPARE
		|| torsoAnim == TORSO_ATTACK3_PREPARE
		|| torsoAnim == TORSO_ATTACK4_PREPARE );
}

/*
=============
CG_AddPlayerWeapon

Used for both the view weapon (ps is valid) and the world modelother character models (ps is NULL)
The main player will have this called for BOTH cases, so effects like light and
sound should only be done on the world model case.
=============
*/
// BFP - CG_AddPlayerWeapon now has a new argument called tagName to attach the flash to a tag
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team, char *tagName ) {
	weapon_t	weaponNum;
	centity_t	*nonPredictedCent;
	refEntity_t	tagEnt;
	bfpWeapon_t				*wpCfg;
	bfpAttackSkinConfig_t	*skinAtkCfg;

	weaponNum = cent->currentState.weapon;
	skinAtkCfg = CG_GetAttackConfig( cent->currentState.clientNum, weaponNum );
	wpCfg = CG_GetBFPWeaponForSlot( cent->currentState.clientNum, cent->currentState.weapon );

	if ( !ps ) {
		int			chargeId = cent->currentState.generic1 - 1;
		qboolean	isCharging = CG_ChargingTorsoAnim( cent, wpCfg );

		// BFP - With constantFireAttack, don't play and start the sound looply
		if ( !( cent->currentState.eFlags & EF_FIRING ) ) {
			cent->pe.constantFireAtkPlayed = qfalse;
		}

		// BFP - attackChargeVoice [attack index] [charge count] ["path of sound"]
		if ( cg_stfu.integer <= 0
		&& skinAtkCfg && chargeId >= 0 && skinAtkCfg->attackChargeVoice[chargeId]
		&& isCharging ) {
			if ( cent->pe.lastChargeVoiceLevel != chargeId ) {
				if ( cent->currentState.number == cg.snap->ps.clientNum ) {
					trap_S_StartLocalSound( skinAtkCfg->attackChargeVoice[chargeId], CHAN_VOICE );
				} else { // make the others hear that
					trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_VOICE, skinAtkCfg->attackChargeVoice[chargeId] );
				}
				cent->pe.lastChargeVoiceLevel = chargeId;
			}
		} else if ( !isCharging ) { // stopped charging ki attack, resets to the next cycle
			cent->pe.lastChargeVoiceLevel = -1;
		}

		// BFP - chargeSound
		if ( skinAtkCfg && skinAtkCfg->chargeSound && isCharging ) {
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, 
				vec3_origin, skinAtkCfg->chargeSound );
		} else if ( ( cent->currentState.eFlags & EF_FIRING ) && skinAtkCfg && skinAtkCfg->firingSound ) {
			// lightning gun and guantlet make a different sound when fire is held down
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, skinAtkCfg->firingSound );
		}
	}

	// make sure we aren't looking at cg.predictedPlayerEntity for LG
	nonPredictedCent = &cg_entities[cent->currentState.clientNum];

	// if the index of the nonPredictedCent is not the same as the clientNum
	// then this is a fake player (like on teh single player podiums), so
	// go ahead and use the cent
	if( ( nonPredictedCent - cg_entities ) != cent->currentState.clientNum ) {
		nonPredictedCent = cent;
	}
	
	// BFP - NOTE: Here's where the player gets the muzzle attached from some of the tags (apply that to client cfg side) (tag_left, tag_right, tag_eyes, ...)
	memset( &tagEnt, 0, sizeof( tagEnt ) );
	CG_PositionEntityOnTag( &tagEnt, parent, parent->hModel, tagName );
	VectorCopy( tagEnt.origin, nonPredictedCent->pe.muzzleOrigin );

	// add the flash
	// BFP - constantFireAttack
	if ( skinAtkCfg && skinAtkCfg->constantFireAttack
		&& ( nonPredictedCent->currentState.eFlags & EF_FIRING ) ) 
	{
		// continuous flash
	} else {
		// impulse flash
		if ( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME 
		&& skinAtkCfg && !skinAtkCfg->constantFireAttack && !( nonPredictedCent->currentState.eFlags & EF_FIRING ) ) {
			return;
		}
	}

	// BFP - Displaying the muzzle flash to the other player correctly
	if ( skinAtkCfg && skinAtkCfg->constantFireAttack ) {
		// constant fire: show flash when firing or when charging, if chargeAttack is set
		if ( wpCfg && !wpCfg->chargeAttack
		&& ( nonPredictedCent->currentState.eFlags & EF_FIRING ) ) {
			if ( !skinAtkCfg ) {
				return;
			}
			if ( skinAtkCfg->firingFlashScaleFactor > 0 ) {
				CG_AddFiringFlash( nonPredictedCent, -1, skinAtkCfg, nonPredictedCent->pe.muzzleOrigin, &tagEnt, tagName );
			} else {
				CG_AddFlash( nonPredictedCent, -1, skinAtkCfg, nonPredictedCent->pe.muzzleOrigin, &tagEnt, tagName );
			}
		}
		if ( wpCfg && ( ( wpCfg->chargeAttack && !wpCfg->chargeAutoFire 
		&& nonPredictedCent->currentState.generic1 >= wpCfg->minCharge ) ) ) {
			CG_AddFlash( nonPredictedCent, -1, skinAtkCfg, nonPredictedCent->pe.muzzleOrigin, &tagEnt, tagName );
		}
	} else {
		// impulse or charge-based flash, non-constant
		if ( ( nonPredictedCent->currentState.eFlags & EF_FIRING ) 
		&& cg.time - cent->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
			if ( !skinAtkCfg ) {
				return;
			}
			if ( skinAtkCfg->firingFlashScaleFactor > 0 ) {
				CG_AddFiringFlash( nonPredictedCent, -1, skinAtkCfg, nonPredictedCent->pe.muzzleOrigin, &tagEnt, tagName );
			} else {
				CG_AddFlash( nonPredictedCent, -1, skinAtkCfg, nonPredictedCent->pe.muzzleOrigin, &tagEnt, tagName );
			}
		} else if ( wpCfg && nonPredictedCent->currentState.generic1 >= wpCfg->minCharge
		&& wpCfg->chargeAttack && !wpCfg->chargeAutoFire ) {
			CG_AddFlash( nonPredictedCent, -1, skinAtkCfg, nonPredictedCent->pe.muzzleOrigin, &tagEnt, tagName );
		}
	}

	// BFP - Play firing sound on constantFireAttack
	if ( skinAtkCfg && skinAtkCfg->constantFireAttack && ( nonPredictedCent->currentState.eFlags & EF_FIRING )
	&& wpCfg && ( wpCfg->chargeAttack || wpCfg->chargeAutoFire ) ) {
		if ( skinAtkCfg->firingSound ) {
			trap_S_AddLoopingSound( nonPredictedCent->currentState.number, nonPredictedCent->lerpOrigin, vec3_origin, skinAtkCfg->firingSound );
		}
	}

	if ( ( skinAtkCfg
	&& ( ( skinAtkCfg->constantFireAttack && wpCfg && !wpCfg->chargeAttack && ( nonPredictedCent->currentState.eFlags & EF_FIRING ) )
		|| ( !skinAtkCfg->constantFireAttack && ( nonPredictedCent->currentState.eFlags & EF_FIRING ) && cg.time - cent->muzzleFlashTime <= MUZZLE_FLASH_TIME ) ) )
	&& ( ps || cg.renderingThirdPerson || cent->currentState.number != cg.predictedPlayerState.clientNum ) ) {
		// BFP - NOTE: That avoids adding the muzzle light using the beam,
		// it would be cool adding a light to the player while charging or firing their ki,
		// but in a custom way

		// add lightning bolt
		CG_LightningBolt( nonPredictedCent, nonPredictedCent->pe.muzzleOrigin, skinAtkCfg );

		if ( skinAtkCfg->missileDlightColor[0] > 0
		|| skinAtkCfg->missileDlightColor[1] > 0
		|| skinAtkCfg->missileDlightColor[1] > 0 ) {
			trap_R_AddLightToScene( parent->origin, 300 + (rand()&31), skinAtkCfg->missileDlightColor[0], 
				skinAtkCfg->missileDlightColor[1], skinAtkCfg->missileDlightColor[2] );
		} else {
			trap_R_AddLightToScene( parent->origin, 300 + (rand()&31), 0.6f, 0.6f, 1.0f );
		}
	}
}

/*
==============
CG_AddViewWeapon

Add the weapon, and flash for the player's view
==============
*/
void CG_AddViewWeapon( playerState_t *ps ) {
	bfpAttackSkinConfig_t	*skinAtkCfg = CG_GetAttackConfig( ps->clientNum, ps->weapon );

	if ( ps->persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
		return;
	}

	if ( ps->pm_type == PM_INTERMISSION ) {
		return;
	}

	// no gun if in third person view or a camera is active
	//if ( cg.renderingThirdPerson || cg.cameraMode) {
	if ( cg.renderingThirdPerson ) {
		return;
	}


	// allow the gun to be completely removed
	if ( skinAtkCfg ) {
		vec3_t		origin;

		if ( cg.predictedPlayerState.eFlags & EF_FIRING ) {
			// special hack for lightning gun...
			VectorCopy( cg.refdef.vieworg, origin );
			VectorMA( origin, -8, cg.refdef.viewaxis[2], origin );
			CG_LightningBolt( &cg_entities[ps->clientNum], origin, skinAtkCfg );
		}
		return;
	}
}

/*
==============================================================================

WEAPON SELECTION

==============================================================================
*/

/*
===================
CG_DrawWeaponSelect
===================
*/
void CG_DrawWeaponSelect( void ) { // BFP - Modified Q3 selectable weapon HUD
	int		i;
	int		bits;
	//int		count;
	int		x, y, w;
	float	*color;
	// BFP - To display selectable ki attacks
	bfpAttackSkinConfig_t	*skinAtkCfg;

	// don't display if dead
	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	color = CG_FadeColor( cg.weaponSelectTime, WEAPON_SELECT_TIME );
	if ( !color ) {
		return;
	}
	trap_R_SetColor( color );

	// showing weapon select clears pickup item display, but not the blend blob
	cg.itemPickupTime = 0;

	// count the number of weapons owned
	bits = cg.snap->ps.stats[ STAT_WEAPONS ];
	// BFP - NOTE: This looks a way to debug the count for weapons, maybe to be removed
#if 0
	count = 0;
	while ( ++i < 16 ) {
		if ( bits & ( 1 << i ) ) {
			++count;
		}
	}
#endif

	x = 10;
	y = 75;

	i = -1; // BFP - -1 instead 0, because first weapon can be selected
	while ( ++i < BFP_NUM_WEAPONS ) {
		skinAtkCfg = CG_GetAttackConfig( cg.snap->ps.clientNum, i );
		if ( !( bits & ( 1 << i ) ) ) {
			continue;
		}

		// draw weapon icon
		if ( skinAtkCfg ) {
			CG_DrawPic( x, y, 91, 46, skinAtkCfg->attackIcon );
		}

		// draw selection marker
		if ( i == cg.weaponSelect ) {
			CG_DrawPic( x, y, 91, 46, cgs.media.selectShader );
		}

		// no ammo cross on top
		if ( !cg.snap->ps.ammo[ i ] ) {
			CG_DrawPic( x, y, 91, 46, cgs.media.noammoShader );
		}

		y += 47;
	}

	// BFP - attackName

	// draw the selected name
	skinAtkCfg = CG_GetAttackConfig( cg.snap->ps.clientNum, cg.weaponSelect );
	if ( skinAtkCfg && skinAtkCfg->attackName[0] ) {
		w = CG_DrawStrlen( skinAtkCfg->attackName ) * BIGCHAR_WIDTH;
		x = ( SCREEN_WIDTH - w ) / 2;
		CG_DrawBigStringColor(x, 288, skinAtkCfg->attackName, color);
	}

	trap_R_SetColor( NULL );
}


/*
===============
CG_WeaponSelectable
===============
*/
static qboolean CG_WeaponSelectable( int i ) {
	if ( !cg.snap->ps.ammo[i] ) {
		return qfalse;
	}
	if ( ! (cg.snap->ps.stats[ STAT_WEAPONS ] & ( 1 << i ) ) ) {
		return qfalse;
	}

	return qtrue;
}

/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f( void ) {
	int		i;
	int		original;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for ( i = 0 ; i < BFP_NUM_WEAPONS ; i++ ) {
		cg.weaponSelect++;
		if ( cg.weaponSelect == BFP_NUM_WEAPONS ) {
			cg.weaponSelect = 0;
		}
		// BFP - Don't lock the scroll to this weapon index
#if 0
		if ( cg.weaponSelect == WP_ATTACK_1 ) {
			continue;		// never cycle to gauntlet
		}
#endif
		if ( CG_WeaponSelectable( cg.weaponSelect ) ) {
			break;
		}
	}
	if ( i == BFP_NUM_WEAPONS ) {
		cg.weaponSelect = original;
	}
}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f( void ) {
	int		i;
	int		original;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for ( i = 0 ; i < BFP_NUM_WEAPONS ; i++ ) {
		cg.weaponSelect--;
		if ( cg.weaponSelect < 0 ) {
			cg.weaponSelect = BFP_NUM_WEAPONS - 1;
		}
		// BFP - Don't lock the scroll to this weapon index
#if 0
		if ( cg.weaponSelect == WP_ATTACK_1 ) {
			continue;		// never cycle to gauntlet
		}
#endif
		if ( CG_WeaponSelectable( cg.weaponSelect ) ) {
			break;
		}
	}
	if ( i == BFP_NUM_WEAPONS ) {
		cg.weaponSelect = original;
	}
}

/*
===============
CG_Weapon_f
===============
*/
void CG_Weapon_f( void ) {
	int		num;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	num = atoi( CG_Argv( 1 ) );

	if ( num < 1 || num > BFP_NUM_WEAPONS ) { // BFP - Changed num > 15 to num > BFP_NUM_WEAPONS
		return;
	}

	cg.weaponSelectTime = cg.time;

	// BFP - Adjust with - 1 because it's like an array
	if ( ! ( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << (num - 1) ) ) ) {
		return;		// don't have the weapon
	}

	// BFP - Don't select a weapon slot locked by powerlevel
	if ( !CG_WeaponSelectable( num - 1 ) ) {
		return;
	}

	cg.weaponSelect = num - 1;
}

/*
===================
CG_OutOfAmmoChange

The current weapon has just run out of ammo
===================
*/
void CG_OutOfAmmoChange( void ) {
	int		i;

	cg.weaponSelectTime = cg.time;

	for ( i = BFP_NUM_WEAPONS - 1 ; i >= 0 ; i-- ) { // BFP - Changed i = 15 to BFP_NUM_WEAPONS - 1, and i > 0 to i >= 0 to get the first selected weapon
		if ( CG_WeaponSelectable( i ) ) {
			cg.weaponSelect = i;
			break;
		}
	}
}



/*
===================================================================================================

WEAPON EVENTS

===================================================================================================
*/

/*
================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void CG_FireWeapon( centity_t *cent ) {
	entityState_t *ent;
	bfpAttackSkinConfig_t	*skinAtkCfg;
	bfpWeapon_t		*wpCfg = CG_GetBFPWeaponForSlot( cent->currentState.clientNum, cent->currentState.weapon );

	ent = &cent->currentState;
	skinAtkCfg = CG_GetAttackConfig( ent->clientNum, ent->weapon );

	if ( ent->weapon >= BFP_NUM_WEAPONS ) {
		CG_Error( "CG_FireWeapon: ent->weapon >= BFP_NUM_WEAPONS(%d)", BFP_NUM_WEAPONS );
		return;
	}

	if ( !skinAtkCfg ) {
		return;
	}

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
	cent->muzzleFlashTime = cg.time;

	// BFP - constantFireAttack and chargeAutoFire handling
	if ( skinAtkCfg->constantFireAttack
	|| ( wpCfg && wpCfg->chargeAutoFire ) ) {
		if ( cent->pe.constantFireAtkPlayed ) {
			goto _skipFlashSound;
		}
		cent->pe.constantFireAtkPlayed = qtrue;
	}

	// BFP - attackFireVoice
	if ( cg_stfu.integer <= 0 && skinAtkCfg->attackFireVoice ) {
		if ( cent->currentState.number == cg.snap->ps.clientNum ) {
			trap_S_StartLocalSound( skinAtkCfg->attackFireVoice, CHAN_VOICE );
		} else { // make the others hear that
			trap_S_StartSound( cent->lerpOrigin, ent->number, CHAN_VOICE, skinAtkCfg->attackFireVoice );
		}
	}

	// BFP - Forcefield with chargeAutoFire
	if ( wpCfg && wpCfg->attackType == ATK_FORCEFIELD && wpCfg->chargeAutoFire ) {
		cent->pe.chargeAutoFire = qtrue;
	} else {
		cent->pe.chargeAutoFire = qfalse;
	}
	if ( wpCfg && wpCfg->attackType == ATK_FORCEFIELD && !wpCfg->chargeAutoFire && !cent->pe.chargeAutoFire ) {
		// BFP - Use that as blinding_flash weapon, no chargeAutoFire set
		// this is when noExplosion is set as weapon config dictates

		// BFP - Low poly sphere
		qhandle_t		explosionModel = ( cg_lowpolysphere.integer > 0 && skinAtkCfg->explosionModel == cgs.media.highPolySphereModel ) ? cgs.media.lowPolySphereModel : skinAtkCfg->explosionModel;
		localEntity_t	*leSphere;
		const float		MAX_SCALE = 27, MAX_SCALEFACTOR = 6.0f; // limits to prevent too large scaling
		int		minCharge = ( wpCfg && wpCfg->minCharge >= 0 ) ? wpCfg->minCharge : 0;
		int		numPointsChargedOverMin = ( cent->currentState.generic1 > 0 ) ? ( cent->currentState.generic1 - minCharge ) : 0; // that means when reaching to 'READY!', it starts as 1 and if it's charging another charge point, adds 1 more
		float	explosionScaleFactor = skinAtkCfg->explosionScaleFactor, explosionScaleFactorChargeMult = skinAtkCfg->explosionScaleFactorChargeMult;
		float	scale = 1;

		if ( explosionScaleFactor > MAX_SCALEFACTOR ) explosionScaleFactor = MAX_SCALEFACTOR;
		if ( explosionScaleFactorChargeMult > MAX_SCALEFACTOR ) explosionScaleFactorChargeMult = MAX_SCALEFACTOR;
		scale = explosionScaleFactor + explosionScaleFactorChargeMult * numPointsChargedOverMin;
		if ( scale > MAX_SCALE ) scale = MAX_SCALE;

		leSphere = CG_SpawnExplosionModel( cent->lerpOrigin, NULL, LE_EXPLOSION_SPHERE, explosionModel, skinAtkCfg->explosionShader, 1000 );
		VectorScale( leSphere->refEntity.axis[0], scale, leSphere->refEntity.axis[0] );
		VectorScale( leSphere->refEntity.axis[1], scale, leSphere->refEntity.axis[1] );
		VectorScale( leSphere->refEntity.axis[2], scale, leSphere->refEntity.axis[2] );
	}

	// play quad sound if needed
	if ( cent->currentState.powerups & ( 1 << PW_QUAD ) ) {
		trap_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.media.quadSound );
	}

	// play a sound
	if ( skinAtkCfg->flashSound ) {
		trap_S_StartSound( NULL, ent->number, CHAN_WEAPON, skinAtkCfg->flashSound );
	}

// BFP - constantFireAttack handling
_skipFlashSound:
	return;
}

/*
=================
CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType, bfpAttackSkinConfig_t *skinAtkCfg, centity_t *cent ) {
	float			radius = 64;

	if ( !skinAtkCfg ) {
		return;
	}

	// BFP - NOTE: Crack mark shader replaces all other mark shaders, the radius is the same (64), there's no alpha fade

	radius += skinAtkCfg->missileRadius;
	if ( radius < 64 ) {
		radius = 64;
	}

	// BFP - Explosion sounds
	CG_ExplosionSound( origin, skinAtkCfg );

	// BFP - Explosion smoke
	CG_SmokeExplosion( origin, dir, skinAtkCfg );

	// BFP - Explosion effects
	CG_ExplosionEffect( origin, dir, skinAtkCfg, cent );

	//
	// impact mark
	//
	CG_ImpactMark( cgs.media.crackMarkShader, origin, dir, random()*360, 1,1,1,1, 0, radius, qfalse );
}


/*
=================
CG_MissileHitPlayer
=================
*/
void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int entityNum, bfpAttackSkinConfig_t *skinAtkCfg, centity_t *cent ) {
	// BFP - Seems like a small redo from CG_MissileHitWall to adjust the needed effects
	if ( !skinAtkCfg ) {
		return;
	}

	// BFP - NOTE: Originally on BFP, players don't bleed, that's a friendly mod :P
	// CG_Bleed( origin, entityNum );

	CG_ExplosionSound( origin, skinAtkCfg ); // BFP - Explosion sounds

	CG_SparksExplosion( origin, dir, skinAtkCfg ); // BFP - Spark particles explosion
	CG_ExplosionEffect( origin, dir, skinAtkCfg, cent ); // BFP - Explosion effects
}


/*
============================================================================

BULLETS

============================================================================
*/


/*
===============
CG_Tracer
===============
*/
void CG_Tracer( vec3_t source, vec3_t dest ) {
	vec3_t		forward, right;
	polyVert_t	verts[4];
	vec3_t		line;
	float		len, begin, end;
	vec3_t		start, finish;
	vec3_t		midpoint;

	// tracer
	VectorSubtract( dest, source, forward );
	len = VectorNormalize( forward );

	// start at least a little ways from the muzzle
	if ( len < 100 ) {
		return;
	}
	begin = 50 + random() * (len - 60);
	end = begin + cg_tracerLength.value;
	if ( end > len ) {
		end = len;
	}
	VectorMA( source, begin, forward, start );
	VectorMA( source, end, forward, finish );

	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

	VectorScale( cg.refdef.viewaxis[1], line[1], right );
	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
	VectorNormalize( right );

	VectorMA( finish, cg_tracerWidth.value, right, verts[0].xyz );
	Vector2Set( verts[0].st, 0, 1 );
	Byte4Set( verts[0].modulate, 255, 255, 255, 255 );

	VectorMA( finish, -cg_tracerWidth.value, right, verts[1].xyz );
	Vector2Set( verts[1].st, 1, 0 );
	Byte4Set( verts[1].modulate, 255, 255, 255, 255 );

	VectorMA( start, -cg_tracerWidth.value, right, verts[2].xyz );
	Vector2Set( verts[2].st, 1, 1 );
	Byte4Set( verts[2].modulate, 255, 255, 255, 255 );

	VectorMA( start, cg_tracerWidth.value, right, verts[3].xyz );
	Vector2Set( verts[3].st, 0, 0 );
	Byte4Set( verts[3].modulate, 255, 255, 255, 255 );

	trap_R_AddPolyToScene( cgs.media.tracerShader, 4, verts );

	midpoint[0] = ( start[0] + finish[0] ) * 0.5;
	midpoint[1] = ( start[1] + finish[1] ) * 0.5;
	midpoint[2] = ( start[2] + finish[2] ) * 0.5;

	// add the tracer sound
	trap_S_StartSound( midpoint, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.tracerSound );

}

// BFP - Unused CG_CalcMuzzlePoint function
#if 0
/*
======================
CG_CalcMuzzlePoint
======================
*/
static qboolean	CG_CalcMuzzlePoint( int entityNum, vec3_t muzzle ) {
	vec3_t		forward;
	centity_t	*cent;
	int			anim;

	if ( entityNum == cg.snap->ps.clientNum ) {
		VectorCopy( cg.snap->ps.origin, muzzle );
		muzzle[2] += cg.snap->ps.viewheight;
		AngleVectors( cg.snap->ps.viewangles, forward, NULL, NULL );
		VectorMA( muzzle, 14, forward, muzzle );
		return qtrue;
	}

	cent = &cg_entities[entityNum];
	if ( !cent->currentValid ) {
		return qfalse;
	}

	VectorCopy( cent->currentState.pos.trBase, muzzle );

	AngleVectors( cent->currentState.apos.trBase, forward, NULL, NULL );
	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if ( anim == LEGS_WALKCR || anim == LEGS_IDLECR ) {
		muzzle[2] += CROUCH_VIEWHEIGHT;
	} else {
		muzzle[2] += DEFAULT_VIEWHEIGHT;
	}

	VectorMA( muzzle, 14, forward, muzzle );

	return qtrue;

}
#endif

