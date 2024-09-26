/*
===========================================================================

BFP TRAILS

===========================================================================
*/


#include "cg_local.h"

#define TRAIL_SEGMENTS		99
#define TRAIL_WIDTH			15

typedef struct {
	vec3_t segments[TRAIL_SEGMENTS];
	int numSegments;
} trail_t;

static trail_t cg_trails[MAX_GENTITIES];

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
CG_ResetKiTrail

Reset entity's ki trail.
Should be called whenever an entity that has to use a trail, wasn't in the PVS the previous frame.
entityNum: Valid entity number
origin:    Point from where the trail should start.
           (This should be equal to the entity's current position.)
=====================
*/
void CG_ResetKiTrail( int entityNum, vec3_t origin ) {
	int i;

	for ( i = 0; i < TRAIL_SEGMENTS; ++i ) {
		VectorCopy( origin, cg_trails[entityNum].segments[i] );
	}
	cg_trails[entityNum].numSegments = 0;
}

/*
=====================
CG_KiTrail

Adds ki trail segments
=====================
*/
void CG_KiTrail( int entityNum, vec3_t origin, qboolean remove, qhandle_t hShader ) {
	int i, j;
	polyVert_t verts[4];

	if ( entityNum < 0 || entityNum >= MAX_GENTITIES ) {
		return;
	}

	if ( remove ) { // removes every segment
		cg_trails[entityNum].numSegments--;
	} else {
		if ( cg_trails[entityNum].numSegments < TRAIL_SEGMENTS ) {
			cg_trails[entityNum].numSegments++;
		}
	}

	// shift points down the buffer
	for ( i = cg_trails[entityNum].numSegments - 1; i > 0; --i ) {
		VectorCopy( cg_trails[entityNum].segments[i - 1], cg_trails[entityNum].segments[i] );
	}

	// add the current position at the start
	VectorCopy( origin, cg_trails[entityNum].segments[0] );

	for ( i = 0; i < cg_trails[entityNum].numSegments - 1; ++i ) {
		// loop to render the segment 3 times
		for ( j = 0; j < 3; ++j ) {
			vec3_t start, end, forward, right;
			vec3_t viewAxis;
			int kiTrailLength = cg_kiTrail.integer;

			if ( kiTrailLength > TRAIL_SEGMENTS ) {
				kiTrailLength = TRAIL_SEGMENTS;
			}

			if ( i + j >= cg_trails[entityNum].numSegments - 1 ) {
				return;
			}

			VectorCopy( cg_trails[entityNum].segments[i + j], start );
			VectorCopy( cg_trails[entityNum].segments[i + j + 1], end );

			VectorSubtract( end, start, forward );
			VectorNormalize( forward );

			VectorSubtract( cg.refdef.vieworg, start, viewAxis );
			CrossProduct( viewAxis, forward, right );
			VectorNormalize( right );

			VectorMA( end, TRAIL_WIDTH, right, verts[0].xyz );
			VectorArray2Set( verts[0].st, 0, 1 );
			Vector4Set( verts[0].modulate, 255, 255, 255, 255 );

			VectorMA( end, -TRAIL_WIDTH, right, verts[1].xyz );
			VectorArray2Set( verts[1].st, 1, 0 );
			Vector4Set( verts[1].modulate, 255, 255, 255, 255 );

			VectorMA( start, -TRAIL_WIDTH, right, verts[2].xyz );
			VectorArray2Set( verts[2].st, 1, 0 );
			Vector4Set( verts[2].modulate, 255, 255, 255, 255 );

			VectorMA( start, TRAIL_WIDTH, right, verts[3].xyz );
			VectorArray2Set( verts[3].st, 0, 1 );
			Vector4Set( verts[3].modulate, 255, 255, 255, 255 );

			trap_R_AddPolyToScene( hShader, 4, verts );
		}
	}
}
