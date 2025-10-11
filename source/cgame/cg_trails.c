/*
===========================================================================

BFP TRAILS

===========================================================================
*/


#include "cg_local.h"

#define TRAIL_SEGMENTS		99
#define CORKSCREW_SEGMENTS	150
#define MONSTER_TRAIL_WIDTH	150

// Trail types
#define KI_TRAIL			0
#define BEAM_TRAIL			1

typedef struct {
	vec3_t segments[TRAIL_SEGMENTS];
	int numSegments;
	vec3_t lastAimAngles;		// track last aim direction
	int lastAimChangeTime;		// last time aim direction changed
} trail_t;

static trail_t cg_trails[MAX_GENTITIES][2];
static vec3_t spiralSegments[CORKSCREW_SEGMENTS];

/*
===============
CG_InitTrails

Initializes the array of trails for all centities.
Should be called from CG_Init in cg_main.c
===============
*/
void CG_InitTrails( void ) {
	memset( &cg_trails, 0, sizeof(cg_trails) );
}

/*
===============
CG_ResetTrail

Reset entity's trail.
Should be called whenever an entity that has to use a trail, wasn't in the PVS the previous frame.
TRAIL_TYPE: Trail type
entityNum: Valid entity number
origin:    Point from where the trail should start.
           (This should be equal to the entity's current position.)
=====================
*/
void CG_ResetTrail( const int TRAIL_TYPE, int entityNum, vec3_t origin ) {
	int i;

	for ( i = 0; i < TRAIL_SEGMENTS; ++i ) {
		VectorCopy( origin, cg_trails[entityNum][TRAIL_TYPE].segments[i] );
	}
	cg_trails[entityNum][TRAIL_TYPE].numSegments = 0;
	VectorClear( cg_trails[entityNum][TRAIL_TYPE].lastAimAngles );
	cg_trails[entityNum][TRAIL_TYPE].lastAimChangeTime = 0;
}

/*
=====================
CG_KiTrail

Adds ki trail segments
=====================
*/
void CG_KiTrail( int entityNum, vec3_t origin, qboolean remove, qhandle_t hShader ) {
	int i, j;
	int kiTrailLength = cg_kiTrail.integer;
	trail_t *kiTrail = &cg_trails[entityNum][KI_TRAIL];

	if ( entityNum < 0 || entityNum >= MAX_GENTITIES ) {
		return;
	}

	if ( kiTrailLength > TRAIL_SEGMENTS ) {
		kiTrailLength = TRAIL_SEGMENTS;
	}

	if ( remove ) { // removes every segment
		kiTrail->numSegments = ( kiTrail->numSegments > 0 ) 
			? kiTrail->numSegments - 1
			: 0;
	} else {
		if ( kiTrail->numSegments < kiTrailLength ) {
			++kiTrail->numSegments;
		}
	}

	// shift points down the buffer
	for ( i = kiTrail->numSegments - 1; i > 0; --i ) {
		VectorCopy( kiTrail->segments[i - 1], kiTrail->segments[i] );
	}

	// add the current position at the start
	VectorCopy( origin, kiTrail->segments[0] );

	for ( i = 0; i < kiTrail->numSegments - 1; ++i ) {
		// loop to render the segment 3 times
		for ( j = 0; j < 3; ++j ) {
			vec3_t start, end;

			if ( i + j >= kiTrail->numSegments - 1 ) {
				return;
			}

			VectorCopy( kiTrail->segments[i + j], start );
			VectorCopy( kiTrail->segments[i + j + 1], end );

			// for the player monster
			if ( cg_entities[entityNum].currentState.eFlags & EF_MONSTER ) {
				polyVert_t verts[4];
				vec3_t forward, right;
				vec3_t viewAxis;

				VectorSubtract( end, start, forward );
				VectorNormalize( forward );

				VectorSubtract( cg.refdef.vieworg, start, viewAxis );
				CrossProduct( viewAxis, forward, right );
				VectorNormalize( right );

				VectorMA( end, MONSTER_TRAIL_WIDTH, right, verts[0].xyz );
				VectorArray2Set( verts[0].st, 0, 1 );
				Vector4Set( verts[0].modulate, 255, 255, 255, 255 );

				VectorMA( end, -MONSTER_TRAIL_WIDTH, right, verts[1].xyz );
				VectorArray2Set( verts[1].st, 1, 0 );
				Vector4Set( verts[1].modulate, 255, 255, 255, 255 );

				VectorMA( start, -MONSTER_TRAIL_WIDTH, right, verts[2].xyz );
				VectorArray2Set( verts[2].st, 1, 0 );
				Vector4Set( verts[2].modulate, 255, 255, 255, 255 );

				VectorMA( start, MONSTER_TRAIL_WIDTH, right, verts[3].xyz );
				VectorArray2Set( verts[3].st, 0, 1 );
				Vector4Set( verts[3].modulate, 255, 255, 255, 255 );

				trap_R_AddPolyToScene( hShader, 4, verts );
			} else { // I see... so, BFP originally used RT_RAIL_CORE, they didn't care the size, it was already set
				refEntity_t	beam;
				memset( &beam, 0, sizeof( beam ) );
				beam.reType = RT_RAIL_CORE;
				beam.customShader = hShader;
				VectorCopy( start, beam.origin );
				VectorCopy( end, beam.oldorigin );
				beam.shaderRGBA[0] = beam.shaderRGBA[1] = beam.shaderRGBA[2] = beam.shaderRGBA[3] = 255;

				trap_R_AddRefEntityToScene( &beam );
			}
		}
	}
}

/*
=====================
CG_BeamTrail

Adds beam trail segments
=====================
*/
void CG_BeamTrail( int entityNum, vec3_t origin, vec3_t muzzleOrigin, qhandle_t hShader ) {
	int i;
	int nBeamSegments = cg_beamTrail.integer;
	trail_t *beamTrail = &cg_trails[entityNum][BEAM_TRAIL];
	const float BEAM_STRAIGHTEN_RATE = 0.2f;	// how quickly segments straighten (0.0 - 1.0)
	const float BEAM_AIM_THRESHOLD = 1.0f;		// minimum angle change to consider "aiming"

	if ( entityNum < 0 || entityNum >= MAX_GENTITIES ) {
		return;
	}

	if ( nBeamSegments > TRAIL_SEGMENTS ) {
		nBeamSegments = TRAIL_SEGMENTS;
	}

	// for better visual bendy effect, the number of segments should be equal or more than 10
	if ( nBeamSegments < 10 ) {
		nBeamSegments = 2;
	}

	beamTrail->numSegments = nBeamSegments;

	VectorCopy( muzzleOrigin, beamTrail->segments[0] );
	VectorCopy( origin, beamTrail->segments[nBeamSegments - 1] );

	// start stretching segments
	if ( nBeamSegments >= 10 ) {
		vec3_t direction, currentAngles, beamDir;
		float angleDelta, beamLength, lengthFactor;
		qboolean isAiming;

		// beam length and direction
		VectorSubtract( origin, muzzleOrigin, beamDir );
		beamLength = VectorLength( beamDir );
		VectorNormalize( beamDir );
		vectoangles( beamDir, currentAngles );

		// length factor for straightening based on beam length
		// longer beam = more straightening (less curve)
		// reference length of 5000 units as baseline
		lengthFactor = 1.0f + ( beamLength / 5000.0f );
		if ( lengthFactor > 2.5f ) {
			lengthFactor = 2.5f;
		}

		if ( beamTrail->lastAimChangeTime > 0 ) {
			angleDelta = Distance( currentAngles, beamTrail->lastAimAngles );
			isAiming = ( angleDelta > BEAM_AIM_THRESHOLD );
		} else {
			isAiming = qfalse;
			angleDelta = 0;
		}

		if ( isAiming ) {
			beamTrail->lastAimChangeTime = cg.time + 200;
		}
		VectorCopy( currentAngles, beamTrail->lastAimAngles );

		// shift all segments down by one position (creating trail effect)
		for ( i = nBeamSegments - 1; i > 0; --i ) {
			VectorCopy( beamTrail->segments[i - 1], beamTrail->segments[i] );
		}

		// update all segments with distance-based straightening
		for ( i = 1; i < nBeamSegments; ++i ) {
			vec3_t targetPos, segmentToOrigin;
			float distanceToOrigin, segmentFraction;
			float straightenFactor, maxExpectedDistance, stretchRatio;

			// ideal position on straight line from muzzle to origin
			segmentFraction = (float)i / (float)( nBeamSegments - 1 );
			VectorMA( muzzleOrigin, segmentFraction * beamLength, beamDir, targetPos );

			// how far this segment is from the impact point
			VectorSubtract( origin, beamTrail->segments[i], segmentToOrigin );
			distanceToOrigin = VectorLength( segmentToOrigin );

			// expected distance if beam was straight
			maxExpectedDistance = beamLength * ( 1.0f - segmentFraction );

			// stretch ratio (how much segments are stretched)
			stretchRatio = 1.0f;
			if ( maxExpectedDistance > 0.1f ) {
				stretchRatio = distanceToOrigin / maxExpectedDistance;
			}

			// more stretch = more straightening
			// stretchRatio: 1.0 = no stretch, >1.0 = stretched

			// currently aiming - allow bending with minimal straightening
			// length factor: longer beams still straighten a bit more even while aiming
			straightenFactor = 0.05f * lengthFactor;
			if ( straightenFactor > 0.3f ) {
				straightenFactor = 0.3f;
			}

			if ( stretchRatio > 1.0f ) {
				// segments are stretching, straighten them out more aggressively
				// length factor: longer beams straighten more
				straightenFactor = BEAM_STRAIGHTEN_RATE * stretchRatio * lengthFactor;
				if ( straightenFactor > 1.0f ) {
					straightenFactor = 1.0f;
				}
			} else if ( cg.time - beamTrail->lastAimChangeTime > 100 ) {
				// not aiming and not stretched - gentle straightening
				// length factor: longer beams straighten faster
				straightenFactor = BEAM_STRAIGHTEN_RATE * lengthFactor;
				if ( straightenFactor > 1.0f ) {
					straightenFactor = 1.0f;
				}
			}

			// transition toward target position
			VectorScale( beamTrail->segments[i], 1.0f - straightenFactor, beamTrail->segments[i] );
			VectorMA( beamTrail->segments[i], straightenFactor, targetPos, beamTrail->segments[i] );
		}
	}

	// draw every beam segment
	{
		refEntity_t beam;
		memset( &beam, 0, sizeof( beam ) );

		beam.customShader = hShader;
		beam.shaderRGBA[0] = beam.shaderRGBA[1] = beam.shaderRGBA[2] = beam.shaderRGBA[3] = 0xff;

		for ( i = 0; i < nBeamSegments - 1; ++i ) {
			// BFP - NOTE: Skip the rendering of the 2 last segments. 
			// Weird. That's why the bendy beam can't visualize that segment correctly.
			// On original BFP also happens
			if ( nBeamSegments >= 10 && i >= (nBeamSegments - 2) ) {
				return;
			}
			beam.reType = RT_LIGHTNING;
			VectorCopy( beamTrail->segments[i], beam.origin );
			VectorCopy( beamTrail->segments[i + 1], beam.oldorigin );

			trap_R_AddRefEntityToScene( &beam );

			// apply sprite in the middle of every segment to avoid showing ugly visual effect
			beam.reType = RT_SPRITE;
			trap_R_AddRefEntityToScene( &beam );
		}
	}
}


/*
=====================
CG_CorkscrewTrail

Adds corkscrew trail segments
=====================
*/
void CG_CorkscrewTrail( int entityNum, vec3_t origin, vec3_t muzzleOrigin, qhandle_t beamShader, qhandle_t corkscrewShader ) {
	int i;
	float length;
	vec3_t fullDir, forward, right, up;
	vec3_t temp = {0, 0, 1};
	const float ROTATIONS = 5.0f;
	const float MAX_RADIUS = 12.0f;
	const int TAPER_SEGMENTS = 7;

	if ( entityNum < 0 || entityNum >= MAX_GENTITIES ) {
		return;
	}

	VectorCopy( origin, spiralSegments[CORKSCREW_SEGMENTS - 1] );
	VectorCopy( muzzleOrigin, spiralSegments[0] );

	// starting corkscrew segment spiral positions
	VectorSubtract( origin, muzzleOrigin, fullDir );
	length = VectorNormalize( fullDir );
	VectorCopy( fullDir, forward );

	// generate robust perpendicular vectors
	CrossProduct( forward, temp, right );
	VectorNormalize( right );
	CrossProduct( forward, right, up );
	VectorNormalize( up );

	// generate spiral points around the beam
	for ( i = 0; i < CORKSCREW_SEGMENTS; ++i ) {
		float t = (float)i / (float)( CORKSCREW_SEGMENTS - 1 );
		vec3_t basePoint, offset, radial;
		float radius = MAX_RADIUS;
		float angle = ROTATIONS * 2 * M_PI * t;

		// point along the beam
		VectorMA( muzzleOrigin, t * length, forward, basePoint );

		// calculate spiral offset (orbit around beam)
		VectorScale( right, cos( angle ), radial );
		VectorMA( radial, sin( angle ), up, radial );
		VectorNormalize( radial );

		// taper radius near muzzle for the first few segments
		if ( i < TAPER_SEGMENTS ) {
			// linear taper: radius starts small and increases
			radius = MAX_RADIUS * ( i / (float)TAPER_SEGMENTS );
		}
		// taper radius near origin for the last few segments
		else if ( i > CORKSCREW_SEGMENTS - ( TAPER_SEGMENTS + 14 ) - 1 ) {
			// linear taper: radius decreases to zero at the end
			int segmentsFromEnd = CORKSCREW_SEGMENTS - 1 - i;
			radius = MAX_RADIUS * ( segmentsFromEnd / (float)( TAPER_SEGMENTS + 14 ) );
		}
		VectorScale( radial, radius, offset );
		
		// final spiral point
		VectorAdd( basePoint, offset, spiralSegments[i] );
	}

	{
		// draw beam segment
		refEntity_t beam;
		memset( &beam, 0, sizeof( beam ) );
		beam.reType = RT_RAIL_CORE;
		beam.customShader = beamShader;
		beam.shaderRGBA[0] = beam.shaderRGBA[1] = beam.shaderRGBA[2] = beam.shaderRGBA[3] = 0xff;
		VectorCopy( muzzleOrigin, beam.origin );
		VectorCopy( origin, beam.oldorigin );

		trap_R_AddRefEntityToScene( &beam );

		// draw every corkscrew segment
		beam.customShader = corkscrewShader;

		for ( i = 0; i < CORKSCREW_SEGMENTS - 1; ++i ) {
			beam.reType = RT_RAIL_CORE;
			VectorCopy( spiralSegments[i], beam.origin );
			VectorCopy( spiralSegments[i + 1], beam.oldorigin );
			trap_R_AddRefEntityToScene( &beam );
		}
	}
}
