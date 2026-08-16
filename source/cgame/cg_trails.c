/*
===========================================================================

BFP TRAILS

===========================================================================
*/


#include "cg_local.h"

#define	TRAIL_SEGMENTS			99
#define	CORKSCREW_SEGMENTS		150
#define	MISSILE_TRAIL_SEGMENTS	120

typedef struct {
	vec3_t segments[MISSILE_TRAIL_SEGMENTS];
	int segmentTime[MISSILE_TRAIL_SEGMENTS];
	vec3_t color;
	float radius;
	qboolean rainbow;
	int rainbowStartTime;
	qhandle_t shader;
	int numSegments;

	// beam bend state
	vec3_t bendOffset;	
	vec3_t lastImpactOrigin;
	int lastUpdateTime;
	qboolean bendInitialized;
} trail_t;

static trail_t cg_trails[MAX_GENTITIES][MAX_TRAIL_TYPES];
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
        cg_trails[entityNum][TRAIL_TYPE].segmentTime[i] = 0;
	}
	cg_trails[entityNum][TRAIL_TYPE].numSegments = 0;

	// reset beam bend state too, so a PVS pop-in doesn't produce a fake whip
	VectorClear( cg_trails[entityNum][TRAIL_TYPE].bendOffset );
	VectorCopy( origin, cg_trails[entityNum][TRAIL_TYPE].lastImpactOrigin );
	cg_trails[entityNum][TRAIL_TYPE].lastUpdateTime = 0;
	cg_trails[entityNum][TRAIL_TYPE].bendInitialized = qfalse;
}

/*
=====================
CG_KiTrail

Adds ki trail segments
=====================
*/
void CG_KiTrail( int entityNum, vec3_t origin, qboolean remove, qhandle_t hShader ) {
	int i;
	int kiTrailLength = cg_kiTrail.integer;
	trail_t *kiTrail = &cg_trails[entityNum][KI_TRAIL];
	const int NORMAL_KI_TRAIL_WIDTH = 15;
	const int MONSTER_KI_TRAIL_WIDTH = 150;
	const int OVERLAP_TIMES = 3;
	float kiTrailWidth = // for the player monster
		( cg_entities[entityNum].currentState.eFlags & EF_MONSTER )
		? MONSTER_KI_TRAIL_WIDTH
		: NORMAL_KI_TRAIL_WIDTH;
// BFP - A macro to enable/disable poly/refEntity_t rendering. If enabled, render polys, if disabled render the original BFP refEntity_t trails
#define POLYVERT_KI_TRAILS	1

	if ( entityNum < 0 || entityNum >= MAX_GENTITIES
	|| kiTrailLength <= OVERLAP_TIMES ) { // don't draw if ki trail length is less than the number of overlap times
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

	for ( i = 0; i < kiTrail->numSegments - OVERLAP_TIMES; ++i ) {
		vec3_t start, end;

		VectorCopy( kiTrail->segments[i], start );
		VectorCopy( kiTrail->segments[i + OVERLAP_TIMES], end );

#if POLYVERT_KI_TRAILS
		// render the polys
		{
			polyVert_t verts[4];
			vec3_t forward, right;
			vec3_t viewAxis;
			float segLen, t; // textcoord scale

			VectorSubtract( end, start, forward );
			segLen = VectorNormalize( forward );

			// same texcoord scaling as RT_RAIL_CORE:
			// s scales with real distance instead of always spanning 0 ... 1,
			// so the shader repeats along the trail instead of stretching per-segment
			t = segLen * 0.00390625; // segLen / 256
			VectorSubtract( cg.refdef.vieworg, start, viewAxis );
			CrossProduct( viewAxis, forward, right );
			VectorNormalize( right );

			VectorMA( start, kiTrailWidth, right, verts[0].xyz );
			Vector2Set( verts[0].st, 0, 0 );
			Byte4Set( verts[0].modulate, 255, 255, 255, 255 );

			VectorMA( end, kiTrailWidth, right, verts[1].xyz );
			Vector2Set( verts[1].st, t, 0 );
			Byte4Set( verts[1].modulate, 255, 255, 255, 255 );

			VectorMA( end, -kiTrailWidth, right, verts[2].xyz );
			Vector2Set( verts[2].st, t, 1 );
			Byte4Set( verts[2].modulate, 255, 255, 255, 255 );

			VectorMA( start, -kiTrailWidth, right, verts[3].xyz );
			Vector2Set( verts[3].st, 0, 1 );
			Byte4Set( verts[3].modulate, 255, 255, 255, 255 );

			trap_R_AddPolyToScene( hShader, 4, verts );
		}
#else
		{ // I see... so, BFP originally used RT_RAIL_CORE, they didn't care the size, it was already set
			refEntity_t	trail;
			memset( &trail, 0, sizeof( trail ) );
			trail.reType = RT_RAIL_CORE;
			trail.customShader = hShader;
			VectorCopy( start, trail.origin );
			VectorCopy( end, trail.oldorigin );
			trail.shaderRGBA[0] = trail.shaderRGBA[1] = trail.shaderRGBA[2] = trail.shaderRGBA[3] = 255;

			trap_R_AddRefEntityToScene( &trail );
		}
#endif
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

	// beam bend tuning constants
	// LATERAL_GAIN: how much of the impact point's frame-to-frame lateral movement gets converted into bend displacement
	// BEND_DECAY_RATE: exponential decay rate per second (higher = straightens out faster)
	// MAX_BEND_RADIUS: clamp so teleports / large snaps don't produce a huge whip
	const float LATERAL_GAIN = 0.6f;
	const float BEND_DECAY_RATE = 20.0f;
	const float MAX_BEND_RADIUS = 250.0f;
	// lateral displacement is captured as a fraction of beam length (an angle, effectively), 
	// then re-scaled by this constant back into the world units LATERAL_GAIN/MAX_BEND_RADIUS 
	// were tuned against, so beam length no longer changes how strongly 
	// a given turn angle bends the beam
	const float REFERENCE_BEND_LENGTH = 512.0f;

	if ( entityNum < 0 || entityNum >= MAX_GENTITIES ) {
		return;
	}

	// for better visual bendy effect, the number of segments should be equal or more than 10
	if ( nBeamSegments < 10 ) {
		nBeamSegments = 2;
	}

	if ( nBeamSegments > TRAIL_SEGMENTS ) {
		nBeamSegments = TRAIL_SEGMENTS;
	}

	beamTrail->numSegments = nBeamSegments;

	// update the bend offset from how much the impact point moved sideways
	// since last frame, then let it decay so the beam whips and straightens out again
	{
		vec3_t	beamDir, lateralDelta, lateralOnly;
		float	dot, decay, dt, beamLen, angularLateral;

		VectorSubtract( origin, muzzleOrigin, beamDir );
		beamLen = VectorNormalize( beamDir );
		if ( beamLen < 1.0f ) {
			beamLen = 1.0f;	// avoid division by zero
		}

		if ( !beamTrail->bendInitialized ) {
			// first sample for this beam: nothing to compare against yet
			VectorClear( beamTrail->bendOffset );
			VectorCopy( origin, beamTrail->lastImpactOrigin );
			beamTrail->lastUpdateTime = cg.time;
			beamTrail->bendInitialized = qtrue;
		} else {
			dt = ( cg.time - beamTrail->lastUpdateTime ) * 0.001f;
			if ( dt < 0.0f ) {
				dt = 0.0f;
			}
			if ( dt > 0.5f ) {
				// large time gap (pause, hitch, teleport): don't inject a spike
				dt = 0.0f;
				VectorClear( beamTrail->bendOffset );
			}

			// how far the impact point moved this frame
			VectorSubtract( origin, beamTrail->lastImpactOrigin, lateralDelta );

			// keep only the component perpendicular to the beam direction,
			// so the beam moving straight forward/back doesn't cause bending
			dot = DotProduct( lateralDelta, beamDir );
			VectorMA( lateralDelta, -dot, beamDir, lateralOnly );

			// turning near a close target moves the impact point by far
			// fewer world units than the same turn angle against a far target
			// (short radius = short arc). Without correcting for that, short
			// beams barely accumulated any bend. Normalize by beam length so
			// what's actually captured is closer to the turn angle, not the
			// raw absolute displacement - this is what REFERENCE_BEND_LENGTH
			// re-scales back into world units the bend constants were tuned for
			angularLateral = VectorLength( lateralOnly ) / beamLen;
			if ( angularLateral > 0.0f ) {
				VectorNormalize( lateralOnly );
				VectorScale( lateralOnly, angularLateral * REFERENCE_BEND_LENGTH, lateralOnly );
			}

			// sign flipped vs. a naive lateral only add
			VectorMA( beamTrail->bendOffset, -LATERAL_GAIN, lateralOnly, beamTrail->bendOffset );

			// decay back towards a straight beam
			decay = 1.0f / ( 1.0f + BEND_DECAY_RATE * dt );
			VectorScale( beamTrail->bendOffset, decay, beamTrail->bendOffset );

			// clamp so a teleport or big snap can't produce a huge whip
			if ( VectorLength( beamTrail->bendOffset ) > MAX_BEND_RADIUS ) {
				VectorNormalize( beamTrail->bendOffset );
				VectorScale( beamTrail->bendOffset, MAX_BEND_RADIUS, beamTrail->bendOffset );
			}

			VectorCopy( origin, beamTrail->lastImpactOrigin );
			beamTrail->lastUpdateTime = cg.time;
		}
	}

	VectorCopy( muzzleOrigin, beamTrail->segments[0] );
	VectorCopy( origin, beamTrail->segments[nBeamSegments - 1] );

	// fill in the intermediate segments along a quadratic Bezier curve (control point = midpoint + bendOffset)
	// instead of a straight line, so the beam bows out to the side and relaxes back 
	// to straight as bendOffset decays
	if ( nBeamSegments > 2 ) {
		vec3_t controlPoint, mid;

		VectorAdd( muzzleOrigin, origin, mid );
		VectorScale( mid, 0.5f, mid );
		VectorAdd( mid, beamTrail->bendOffset, controlPoint );

		for ( i = 1; i < nBeamSegments - 1; ++i ) {
			float	t = (float)i / (float)( nBeamSegments - 1 );
			float	invT = 1.0f - t;
			vec3_t	a, b;

			// quadratic Bezier: (1-t)^2 * P0 + 2(1-t)t * P1 + t^2 * P2
			VectorScale( muzzleOrigin, invT * invT, a );
			VectorScale( controlPoint, 2.0f * invT * t, b );
			VectorAdd( a, b, beamTrail->segments[i] );
			VectorScale( origin, t * t, b );
			VectorAdd( beamTrail->segments[i], b, beamTrail->segments[i] );
		}
	}

	// draw every beam segment
	{
		refEntity_t beam;
		memset( &beam, 0, sizeof( beam ) );

		beam.customShader = hShader;
		beam.shaderRGBA[0] = beam.shaderRGBA[1] = beam.shaderRGBA[2] = beam.shaderRGBA[3] = 0xff;
		beam.reType = RT_LIGHTNING;

		if ( nBeamSegments == 2 ) {
			VectorCopy( beamTrail->segments[0], beam.origin );
			VectorCopy( beamTrail->segments[1], beam.oldorigin );
			trap_R_AddRefEntityToScene( &beam );
			return;
		}

		// BFP - NOTE: Skip the rendering of the 2 last segments. 
		// Weird. That's why the bendy beam can't visualize that segment correctly.
		// On original BFP also happens
		for ( i = 0; i < nBeamSegments - 2; ++i ) {
			VectorCopy( beamTrail->segments[i], beam.origin );
			if ( i > 0 ) {
				VectorCopy( beamTrail->segments[i - 1], beam.origin );
			}
			VectorCopy( beamTrail->segments[i + 1], beam.oldorigin );
			trap_R_AddRefEntityToScene( &beam );
			beam.reType = RT_SPRITE;
			trap_R_AddRefEntityToScene( &beam );
			beam.reType = RT_LIGHTNING;
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
			VectorCopy( spiralSegments[i], beam.origin );
			if ( i > 0 ) {
				VectorCopy( spiralSegments[i - 1], beam.origin );
			}
			VectorCopy( spiralSegments[i + 1], beam.oldorigin );
			trap_R_AddRefEntityToScene( &beam );
		}
	}
}


/*
=====================
HSVtoRGB
=====================
*/
static float ModFloat( float a, float b ) {
	return ( b <= 0.0f ) ? 0 : ( a - (int)( a / b ) * b );
}
static void HSVtoRGB( float h, float s, float v, float *r, float *g, float *b ) {
	int i;
	float f, p, q, t;
	h = ModFloat( h, 1.0f );
	if ( s <= 0.0f ) {
		*r = *g = *b = v;
		return;
	}
	h *= 6.0f;
	i = floor( h );
	f = h - i;
	p = v * ( 1.0f - s );
	q = v * ( 1.0f - s * f );
	t = v * ( 1.0f - s * ( 1.0f - f ) );
	switch ( i ) {
	case 0: *r = v; *g = t; *b = p; break;
	case 1: *r = q; *g = v; *b = p; break;
	case 2: *r = p; *g = v; *b = t; break;
	case 3: *r = p; *g = q; *b = v; break;
	case 4: *r = t; *g = p; *b = v; break;
	default:*r = v; *g = p; *b = q; break;
	}
}


/*
=====================
CG_MissileTrail

Just adds segments, doesn't draw
=====================
*/
void CG_MissileTrail( int entityNum, vec3_t origin, float radius, qhandle_t hShader, vec3_t color, qboolean rainbow ) {
	int		i;
	trail_t	*missileTrail = &cg_trails[entityNum][MISSILE_TRAIL];

	if ( entityNum < 0 || entityNum >= MAX_GENTITIES ) {
		return;
	}

	if ( missileTrail->numSegments == 0 ) {
		missileTrail->rainbowStartTime = cg.time;
	}

	if ( missileTrail->numSegments < MISSILE_TRAIL_SEGMENTS ) {
		++missileTrail->numSegments;
	}

	for ( i = missileTrail->numSegments - 1; i > 0; --i ) {
		VectorCopy( missileTrail->segments[i-1], missileTrail->segments[i] );
		missileTrail->segmentTime[i] = missileTrail->segmentTime[i-1];
	}

	missileTrail->radius = radius;
	VectorCopy( origin, missileTrail->segments[0] );
	missileTrail->segmentTime[0] = cg.time;
	missileTrail->shader = hShader;
	VectorCopy( color, missileTrail->color );
	missileTrail->rainbow = rainbow;
}


/*
=====================
CG_DrawMissileTrails

Called once per frame.
Draw all active missile segments of all entities
=====================
*/
void CG_DrawMissileTrails( void ) {
	int			entityNum, i;
	const int	SEGMENT_LIFESPAN_MSEC = 900;
	const float	START_WIDTH = 15.0f;
	const float	END_WIDTH = 15.0f;

	for ( entityNum = 0; entityNum < MAX_GENTITIES; ++entityNum ) {
		trail_t		*trail = &cg_trails[entityNum][MISSILE_TRAIL];

		for ( i = 0; i < trail->numSegments - 1; ++i ) {
			vec3_t	start, end;
			float	alpha, width;
			byte	alphaByte;
			int		age = cg.time - trail->segmentTime[i];
			int		ageNext = cg.time - trail->segmentTime[i + 1];

			if ( age >= SEGMENT_LIFESPAN_MSEC ) {
				continue;
			}

			// the old end of the quad has expired — don't draw,
			// but there might be new segments later in the buffer,
			// so we use continue instead of break
			if ( ageNext >= SEGMENT_LIFESPAN_MSEC ) {
				continue;
			}

			// temporal discontinuity: the two ends belong to different firings.
			// a gap greater than ~150ms between consecutive segments means that the
			// buffer was refilled by a new missile on top of the remains of the previous one
			if ( trail->segmentTime[i] - trail->segmentTime[i + 1] > 150 ) {
				continue;
			}

			VectorCopy( trail->segments[i], start );
			VectorCopy( trail->segments[i + 1], end );

			// calculate opacity and width according to age
			alpha = 1.0f - (float)age / SEGMENT_LIFESPAN_MSEC;
			if ( alpha < 0.0f ) alpha = 0.0f;
			alphaByte = (byte)(alpha * 255.0f);
			width = trail->radius * ( 1.0f - (float)age / SEGMENT_LIFESPAN_MSEC )
							+ trail->radius * ( (float)age / SEGMENT_LIFESPAN_MSEC );
			if ( width <= 1 ) {
				width = START_WIDTH * ( 1.0f - (float)age / SEGMENT_LIFESPAN_MSEC )
							+ END_WIDTH * ( (float)age / SEGMENT_LIFESPAN_MSEC );
			}

			// render the polys
			{
				vec3_t		forward, right, viewAxis;
				polyVert_t	verts[4];
				byte		r, g, b;

				r = (byte)(trail->color[0] * alphaByte);
				g = (byte)(trail->color[1] * alphaByte);
				b = (byte)(trail->color[2] * alphaByte);

				// rainbow effect
				if ( trail->rainbow ) {
					float	rf, gf, bf;
					float elapsed = ( cg.time - trail->rainbowStartTime ) * 0.0006f;
					float	hue = (float)age / SEGMENT_LIFESPAN_MSEC;
					hue = ModFloat( elapsed, 1.0f );
					HSVtoRGB( hue, 1.0f, 1.0f, &rf, &gf, &bf );
					r = (byte)(rf * alphaByte);
					g = (byte)(gf * alphaByte);
					b = (byte)(bf * alphaByte);
				}

				VectorSubtract( end, start, forward );
				VectorNormalize( forward );
				VectorSubtract( cg.refdef.vieworg, start, viewAxis );
				CrossProduct( viewAxis, forward, right );
				VectorNormalize( right );

				VectorMA( start,  width, right, verts[0].xyz );
				Vector2Set( verts[0].st, 0, 0 );
				Byte4Set( verts[0].modulate, r, g, b, alphaByte );

				VectorMA( end, width, right, verts[1].xyz );
				Vector2Set( verts[1].st, 1, 0 );
				Byte4Set( verts[1].modulate, r, g, b, alphaByte );

				VectorMA( end, -width, right, verts[2].xyz );
				Vector2Set( verts[2].st, 1, 1 );
				Byte4Set( verts[2].modulate, r, g, b, alphaByte );

				VectorMA( start, -width, right, verts[3].xyz );
				Vector2Set( verts[3].st, 0, 1 );
				Byte4Set( verts[3].modulate, r, g, b, alphaByte );

				trap_R_AddPolyToScene( trail->shader, 4, verts );
			}
		}
	}
}
