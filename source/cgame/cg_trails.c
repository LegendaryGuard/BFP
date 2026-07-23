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
	qboolean rainbow;
	int rainbowStartTime;
	qhandle_t shader;
	int numSegments;
} trail_t;

static trail_t cg_trails[MAX_GENTITIES][3];
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

			VectorSubtract( end, start, forward );
			VectorNormalize( forward );

			VectorSubtract( cg.refdef.vieworg, start, viewAxis );
			CrossProduct( viewAxis, forward, right );
			VectorNormalize( right );

			VectorMA( start, kiTrailWidth, right, verts[0].xyz );
			Vector2Set( verts[0].st, 0, 0 );
			Byte4Set( verts[0].modulate, 255, 255, 255, 255 );

			VectorMA( end, kiTrailWidth, right, verts[1].xyz );
			Vector2Set( verts[1].st, 1, 0 );
			Byte4Set( verts[1].modulate, 255, 255, 255, 255 );

			VectorMA( end, -kiTrailWidth, right, verts[2].xyz );
			Vector2Set( verts[2].st, 1, 1 );
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
	const float BEAM_STRAIGHTEN_RATE = 0.2f;	// how quickly segments straighten (0.0 - 1.0)

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

	VectorCopy( muzzleOrigin, beamTrail->segments[0] );
	VectorCopy( origin, beamTrail->segments[nBeamSegments - 1] );

	// start stretching segments
	if ( nBeamSegments >= 10 ) {
		vec3_t currentAngles, beamDir;
		float beamLength, lengthFactor;

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

		// shift all segments down by one position (creating trail effect)
		for ( i = nBeamSegments - 1; i > 0; --i ) {
			VectorCopy( beamTrail->segments[i - 1], beamTrail->segments[i] );
		}

		// update all segments with distance-based straightening
		for ( i = 1; i < nBeamSegments; ++i ) {
			vec3_t targetPos, segmentToOrigin;
			float distanceToOrigin, segmentFraction;
			float straightenFactor, maxExpectedDistance, stretchRatio = 1.0f;

			// ideal position on straight line from muzzle to origin
			segmentFraction = (float)i / (float)( nBeamSegments - 1 );
			VectorMA( muzzleOrigin, segmentFraction * beamLength, beamDir, targetPos );

			// how far this segment is from the impact point
			VectorSubtract( origin, beamTrail->segments[i], segmentToOrigin );
			distanceToOrigin = VectorLength( segmentToOrigin );

			// expected distance if beam was straight
			maxExpectedDistance = beamLength * ( 1.0f - segmentFraction );

			// stretch ratio (how much segments are stretched)
			if ( maxExpectedDistance > 0.1f ) {
				stretchRatio = distanceToOrigin / maxExpectedDistance;
			}

			if ( stretchRatio > 1.0f ) {
				// segments are stretching, straighten them out more aggressively
				// length factor: longer beams straighten more
				straightenFactor = BEAM_STRAIGHTEN_RATE * stretchRatio * lengthFactor;
				if ( straightenFactor > 1.0f ) {
					straightenFactor = 1.0f;
				}
			} else {
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
void CG_MissileTrail( int entityNum, vec3_t origin, qhandle_t hShader, vec3_t color, qboolean rainbow ) {
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
			width = START_WIDTH * ( 1.0f - (float)age / SEGMENT_LIFESPAN_MSEC )
						+ END_WIDTH * ( (float)age / SEGMENT_LIFESPAN_MSEC );

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
