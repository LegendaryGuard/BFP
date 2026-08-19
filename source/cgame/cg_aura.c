/*
===========================================================================

BFP AURAS

===========================================================================
*/

#include "cg_local.h"

/*
==========================
CG_AuraPowerlevelSetShaderColor

Powerlevel is divided into tiers. 
While most powerlevel effects are based on the powerlevel itself, 
aura color is determined by the tier.

- Tier 1:			< 100,000 PL			Blue aura
- Tier 2:			100,000 – 250,000 PL	Red aura
- Tier 3:			250,000 – 500,000 PL	Red aura
- Tier 4:			500,000 – 999,000 PL	Red aura
- Ultimate Tier:	1 mil PL				Yellow aura
==========================
*/
qhandle_t CG_AuraPowerlevelSetShaderColor( centity_t *cent ) {
	entityState_t	*state = &cent->currentState;
	qhandle_t	auraShader = cgs.media.auraRedTinyShader;
	int			powerlevel = state->frame;

	if ( state->clientNum == cg.snap->ps.clientNum ) { // fixes a weird bug when trying to see the powerlevel of itself
		powerlevel = cg.snap->ps.persistant[PERS_POWERLEVEL];
	}

	// red
	if ( cg_lightweightAuras.integer <= 0
	&& cg_polygonAura.integer <= 0
	&& cg_spriteAura.integer <= 0
	&& cg_particleAura.integer <= 0 ) {
		auraShader = cgs.media.auraRedChargeShader;
		if ( ( state->legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_CHARGE
		&& !( cg.time < cent->pe.tierAuraTime ) ) {
			auraShader = cgs.media.auraRedUseShader;
		}
	}
	// blue
	if ( powerlevel < 100 
	|| ( cgs.gametype >= GT_TEAM && cgs.clientinfo[ state->clientNum ].team == TEAM_BLUE ) ) {
		auraShader = cgs.media.auraBlueTinyShader;

		if ( cg_lightweightAuras.integer <= 0
		&& cg_polygonAura.integer <= 0
		&& cg_spriteAura.integer <= 0
		&& cg_particleAura.integer <= 0 ) {
			auraShader = cgs.media.auraBlueChargeShader;
			if ( ( state->legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_CHARGE
			&& !( cg.time < cent->pe.tierAuraTime ) ) {
				auraShader = cgs.media.auraBlueUseShader;
			}
		}
	}
	// yellow
	if ( powerlevel >= 1000
	&& !( cgs.gametype >= GT_TEAM ) ) {
		auraShader = cgs.media.auraYellowTinyShader;

		if ( cg_lightweightAuras.integer <= 0
		&& cg_polygonAura.integer <= 0
		&& cg_spriteAura.integer <= 0
		&& cg_particleAura.integer <= 0 ) {
			auraShader = cgs.media.auraYellowChargeShader;
			if ( ( state->legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_CHARGE
			&& !( cg.time < cent->pe.tierAuraTime ) ) {
				auraShader = cgs.media.auraYellowUseShader;
			}
		}
	}
	return auraShader;
}

/*
===============
CG_AuraAnims

Handle aura animations, when idling it sets the aura vertical rotation, so the aura rotates vertically
===============
*/
static void CG_AuraAnims( centity_t *cent, refEntity_t *aura, qboolean reversed, vec3_t auraInverseRotation ) { // BFP - Aura animations (change model shaders)
	if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_RUN
	|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_JUMP ) {
		aura->hModel = cgs.media.runauraModel;
	} else if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_BACK
	|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_JUMPB
	|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_FLYB ) {
		aura->hModel = cgs.media.backauraModel;
	} else if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_FLYA ) {
		aura->hModel = cgs.media.flyauraModel;
	} else {
		aura->hModel = cgs.media.auraModel;
		if ( reversed ) {
			VectorNegate( cg.autoAngles, auraInverseRotation );
			AnglesToAxis( auraInverseRotation, aura->axis );
		} else {
			AnglesToAxis( cg.autoAngles, aura->axis );
		}
	}
}


/*
===============
CG_DynamicAuraLight

Dynamic aura light, note: when charging it changes the shinning a bit
Aura lights like cg_smallOwnAura only can be shown to itself and not the other clients, 
the other clients only show small lights. 
===============
*/
static void CG_DynamicAuraLight( centity_t *cent, int clientNum, float r, float g, float b ) { // BFP - Dynamic aura light
	int dLightSize = 200;
	int rndDLight = dLightSize * 0.7845;
	int firstRndDlight = dLightSize * 1.26;

	// BFP - Monster gamemode, player monster dynamic light size
	if ( cent->currentState.eFlags & EF_MONSTER ) {
		dLightSize = 1000;
	}
	// BFP - NOTE: Originally, if cg_spriteAura or cg_particleAura is on, the lights aren't displayed. 
	// But in that case, that can displayed, so it makes no sense not being displayed and 
	// maybe these things were broken on original BFP.
	if ( cg_lightAuras.integer > 0 ) {
		if ( clientNum == cg.snap->ps.clientNum && cg_smallOwnAura.integer > 0 ) {
			trap_R_AddLightToScene( cent->lerpOrigin, dLightSize, r, g, b );
			trap_R_AddLightToScene( cent->lerpOrigin, dLightSize, r, g, b );
			if ( !cg.predictedKiCharging ) {
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&rndDLight), r, g, b );
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&rndDLight), r, g, b );
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&rndDLight), r, g, b );
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&rndDLight), r, g, b );
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&rndDLight), r, g, b );
			} else {
				dLightSize = 100;
				// BFP - Monster gamemode, player monster dynamic light size
				if ( cent->currentState.eFlags & EF_MONSTER ) {
					dLightSize = 500;
				}
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&dLightSize), r, g, b );
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&dLightSize), r, g, b );
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&dLightSize), r, g, b );
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&dLightSize), r, g, b );
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&dLightSize), r, g, b );
			}
		} else if ( clientNum != cg.snap->ps.clientNum || cg_lightweightAuras.integer > 0 || cg_polygonAura.integer > 0 || cg_highPolyAura.integer > 0 ) {
			dLightSize = 50;
			// BFP - Monster gamemode, player monster dynamic light size
			if ( cent->currentState.eFlags & EF_MONSTER ) {
				dLightSize = 250;
			}
			trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&(dLightSize * 2)), r, g, b );
			trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&(dLightSize * 2)), r, g, b );
			trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&(dLightSize * 2)), r, g, b );
		} else {
			trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&firstRndDlight), r, g, b );
			trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&rndDLight), r, g, b );
			trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&rndDLight), r, g, b );
			if ( !cg.predictedKiCharging ) {
				dLightSize = 100;
				// BFP - Monster gamemode, player monster dynamic light size
				if ( cent->currentState.eFlags & EF_MONSTER ) {
					dLightSize = 500;
				}
				firstRndDlight = dLightSize * 1.5;
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&firstRndDlight), r, g, b );
			}
		}
	}
}
/*
============
CG_SpriteAura

Adds sprite aura, just one quad
============
*/
static void CG_SpriteAura( refEntity_t aura ) { // BFP - Sprite aura
	// BFP - NOTE: Originally, BFP didn't finish the shader to attach or they forgot...
	// That radius looks a bit big for an aura, maybe they thought to fit the texture that way or some circular aura?
	// And... What the heck? This sprite view depends of pitch angle until some client connects?
	// Also when cg_smallOwnAura cvar is enabled, it doesn't display any aura to the client itself. 
	// Moreover, the lights are disabled as mentioned previously in CG_DynamicAuraLight function comments
	// In the future, the shader should be added, not sure what kind of aura is this...
	float pitchView = cg.refdefViewAngles[PITCH];
	int i, connectedClients = 1;

	for ( i = 0; i < MAX_CLIENTS; ++i ) {
		if ( cg_entities[i].currentValid ) {
			++connectedClients;
		}
	}
	aura.reType = RT_SPRITE;
	aura.customShader = cgs.media.spriteAura;
	aura.radius += 75;
	if ( connectedClients > 1 ) {
		pitchView = -15;
	}
	aura.rotation = pitchView;

	aura.shaderRGBA[0] = 255;
	aura.shaderRGBA[1] = 255;
	aura.shaderRGBA[2] = 255;
	aura.shaderRGBA[3] = 255;
	trap_R_AddRefEntityToScene( &aura );
}


/*
===============
CG_RemoveKiTrails

Handle aura animations, when idling it sets the aura vertical rotation, so the aura rotates vertically
===============
*/
static void CG_RemoveKiTrails( centity_t *cent, int clientNum, vec3_t kiTrailOrigin, qhandle_t kiTrailShader, qboolean fastRemove ) { // BFP - Remove ki trails
	if ( cg.time > cent->pe.kiTrailTime ) { // reset ki trail position avoid being zeroed
		CG_ResetTrail( KI_TRAIL, clientNum, kiTrailOrigin );
	} else { // ki trails keep running in that moment, but their segments are being removed
		CG_KiTrail( clientNum, kiTrailOrigin, fastRemove, kiTrailShader );
	}
}


/*
============
CG_Aura

Adds aura and ki trails
============
*/
void CG_Aura( centity_t *cent, int clientNum, clientInfo_t *ci, int renderfx, refEntity_t legs, qhandle_t kiTrailShader ) { // BFP - Aura and ki trails
	refEntity_t		aura;
	refEntity_t		aura2; // secondary aura
	vec3_t			auraInverseRotation; // for aura inverse rotation
	vec3_t			kiTrailOrigin;
	int				powerlevel = cent->currentState.frame;
	const int		KI_TRAIL_ZPOS = 5;

	if ( clientNum == cg.snap->ps.clientNum ) { // fixes a weird bug when trying to see the powerlevel of itself
		powerlevel = cg.snap->ps.persistant[PERS_POWERLEVEL];
	}

	memset( &aura, 0, sizeof(aura) );
	memset( &aura2, 0, sizeof(aura2) );

	// origin setup for ki trails
	VectorCopy( cent->lerpOrigin, kiTrailOrigin );
	kiTrailOrigin[2] += KI_TRAIL_ZPOS;

	if ( ( cent->currentState.eFlags & EF_AURA ) || cg.time < cent->pe.tierAuraTime ) {
		// trace for bubble particles only when moving in the water and charging
		int destContentType = CG_PointContents( legs.origin, -1 );

		// BFP - Ki trail
		if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_CHARGE 
		&& cg_kiTrail.integer > 0 ) {
			// apply time for using ki trail
			cent->pe.kiTrailTime = cg.time + cg_kiTrail.integer*7;

			CG_KiTrail( clientNum, kiTrailOrigin, qfalse, kiTrailShader );
		} else { // handle when the ki trail was being used previously
			CG_RemoveKiTrails( cent, clientNum, kiTrailOrigin, kiTrailShader, qtrue );
		}

		// spawning bubble particles
		if ( destContentType & CONTENTS_WATER ) {
			trace_t trace;
			vec3_t start, bubbleOrigin;
			float bubbleSize = 2;
			float bubbleRange = 10;

			VectorCopy( legs.origin, bubbleOrigin );
			trap_CM_BoxTrace( &trace, start, bubbleOrigin, NULL, NULL, 0, CONTENTS_WATER );

			bubbleOrigin[2] += -17; // put the origin below the character's feet

			// BFP - Monster gamemode, player monster bubble particle size, range and position
			if ( cent->currentState.eFlags & EF_MONSTER ) {
				bubbleSize = 8;
				bubbleRange = 100;
				bubbleOrigin[2] += -85; // put the origin below the character's feet
				trace.endpos[2] += 100;
			}

			if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_FLYA
			|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_FLYB ) {
				bubbleOrigin[2] += 6; // put the origin near the player origin point
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, bubbleOrigin, trace.endpos, 700, bubbleRange, bubbleSize );
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, bubbleOrigin, trace.endpos, 700, bubbleRange, bubbleSize );
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, bubbleOrigin, trace.endpos, 700, bubbleRange, bubbleSize );
			} else if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_CHARGE ) {
				bubbleOrigin[2] += -3; // put the origin a little below
				bubbleRange *= 2;

				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, bubbleOrigin, trace.endpos, 0, bubbleRange, bubbleSize );
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, bubbleOrigin, trace.endpos, 0, bubbleRange, bubbleSize );
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, bubbleOrigin, trace.endpos, 0, bubbleRange, bubbleSize );
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, bubbleOrigin, trace.endpos, 0, bubbleRange, bubbleSize );
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, bubbleOrigin, trace.endpos, 0, bubbleRange, bubbleSize );
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, bubbleOrigin, trace.endpos, 0, bubbleRange, bubbleSize );
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, bubbleOrigin, trace.endpos, 0, bubbleRange, bubbleSize );
			}
		}

		// apply the render type
		aura.reType = aura2.reType = RT_MODEL;

		// clear the axis to keep the position
		AxisClear( aura.axis );
		AxisClear( aura2.axis );

		// if the player is moving like going forward and backwards, then use other aura model
		CG_AuraAnims( cent, &aura, 0, auraInverseRotation );
		CG_AuraAnims( cent, &aura2, 1, auraInverseRotation );

		// resize the aura
		CG_ModelSize( &aura, 1.3f );
		CG_ModelSize( &aura2, 1.5f );

		// set aura position to the player
		VectorCopy( legs.origin, aura.origin );
		VectorCopy( legs.lightingOrigin, aura.lightingOrigin );
		VectorCopy( legs.origin, aura2.origin );
		VectorCopy( legs.lightingOrigin, aura2.lightingOrigin );

		// apply light blinking
		aura.customShader = aura2.customShader = CG_AuraPowerlevelSetShaderColor( cent );
		// blue
		if ( powerlevel < 100 
		|| ( cgs.gametype >= GT_TEAM && ci->team == TEAM_BLUE ) ) {
			CG_DynamicAuraLight( cent, clientNum, 0.2f, 0.2f, 1.0 );
		}
		// yellow
		else if ( powerlevel >= 1000
		&& !( cgs.gametype >= GT_TEAM ) ) {
			//CG_DynamicAuraLight( cent, clientNum, 1.0, 1.0, 0 );
			CG_DynamicAuraLight( cent, clientNum, 1.0, 1.0, 0.2f );
		}
		// red
		else {
			CG_DynamicAuraLight( cent, clientNum, 1.0, 0.2f, 0.2f );
		}

		aura.renderfx = aura2.renderfx = renderfx;
		VectorCopy( aura.origin, aura.oldorigin );	// don't positionally lerp at all
		VectorCopy( aura2.origin, aura2.oldorigin );	// don't positionally lerp at all

		// ki boost and ki charge sounds
		if ( !( cg.time < cent->pe.tierAuraTime ) ) {
			if ( !( cent->currentState.eFlags & EF_KI_BOOST )
			&& ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_CHARGE ) {
				trap_S_AddLoopingSound( cent->currentState.clientNum, cent->lerpOrigin, 
					vec3_origin, cgs.media.kiChargeSound );
			} else {
				trap_S_AddLoopingSound( cent->currentState.clientNum, cent->lerpOrigin, 
					vec3_origin, cgs.media.kiUseSound );
			}
		}

		// aura tier
		if ( cg.time < cent->pe.tierAuraTime ) {
			// resize the aura
			CG_ModelSize( &aura, 1.3f );
			CG_ModelSize( &aura2, 1.5f );
		}

		// keep the aura pivot tagged in tag_torso
		CG_PositionRotatedEntityOnTag( &aura, &legs, legs.hModel /*ci->legsModel*/, "tag_torso" );
		CG_PositionRotatedEntityOnTag( &aura2, &legs, legs.hModel /*ci->legsModel*/, "tag_torso" );

		// BFP - Sprite aura
		if ( ( cg_spriteAura.integer > 0 && cg_smallOwnAura.integer <= 0 ) 
		|| ( cg_spriteAura.integer > 0 && cg_smallOwnAura.integer > 0 && clientNum != cg.snap->ps.clientNum ) ) {
			// BFP - Monster gamemode, player monster sprite aura is bigger
			if ( cent->currentState.eFlags & EF_MONSTER ) {
				aura.radius = 325;
			}
			CG_SpriteAura( aura );
			return;
		}

		// BFP - Particle aura
		if ( ( cg_particleAura.integer > 0 && cg_smallOwnAura.integer <= 0 ) 
		|| ( cg_particleAura.integer > 0 && cg_smallOwnAura.integer > 0 && clientNum != cg.snap->ps.clientNum ) ) {
			// BFP - NOTE: Particle aura wasn't fully implemented on original BFP.
			// Originally, particle aura wasn't correctly placed on player's origin, it was zeroed and 
			// when the player moves up, the aura was moving to right, and when moves down, it was moving to left; 
			// moreover, spawns too many particles; also the shader uses bubble ones and the particle size is a bit big. 
			// It's unknown what they planned in their future.
			// But this time, it's placed to player's origin like when being underwater, more fading is added, 
			// also it doesn't spawn too many particles.
			// In the future, that should be tweaked, bubble shader doesn't seem to fit well.
			vec3_t pAuraOrigin;
			VectorCopy( legs.origin, pAuraOrigin );

			pAuraOrigin[2] += -18; // put the origin a little below

			CG_ParticleAura( cent, clientNum, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, pAuraOrigin, NULL, 20 );
			CG_ParticleAura( cent, clientNum, cgs.media.waterBubbleShader, cgs.media.lowPolySphereModel, pAuraOrigin, NULL, 20 );
			return;
		}

		// BFP - Small own aura only can be shown to the one who enables it for themself, not everyone
		if ( clientNum != cg.snap->ps.clientNum || cg_smallOwnAura.integer <= 0 ) {
			// BFP - Transformation aura
			if ( cg.time < cent->pe.tierAuraTime && cg_transformationAura.integer <= 0 ) {
				return;
			}

			// add aura
			if ( cg_spriteAura.integer <= 0 && cg_particleAura.integer <= 0 
			&& cg_polygonAura.integer > 0 && cg_lightweightAuras.integer <= 0 ) {
				trap_R_AddRefEntityToScene( &aura );
			}

			// add secondary aura to make look cooler, a bit bigger than the other
			if ( cg_spriteAura.integer <= 0 && cg_particleAura.integer <= 0 
			&& cg_polygonAura.integer > 0 && cg_highPolyAura.integer > 0 && cg_lightweightAuras.integer <= 0 ) {
				trap_R_AddRefEntityToScene( &aura2 );
			}
		}
	} else {
		// BFP - Ki trail being removed
		if ( cg_kiTrail.integer > 0 ) {
			CG_RemoveKiTrails( cent, clientNum, kiTrailOrigin, kiTrailShader, qtrue );
		}
	}
}
