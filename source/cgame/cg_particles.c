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
// Rafael particles
// cg_particles.c  

// BFP - HIGHLY MODIFIED

#include "cg_local.h"

#define BLOODRED	2
#define EMISIVEFADE	3
#define GREY75		4

typedef struct particle_s
{
	struct particle_s	*next;

	float		time;
	float		endtime;

	vec3_t		org;
	vec3_t		vel;
	vec3_t		accel;
	int			color;
	vec3_t		confettiColor; // BFPR - For confetti particle
	float		alpha;
	float		alphavel;
	int			type;
	qhandle_t	pshader;

	float		height;
	float		width;

	float		endheight;
	float		endwidth;
	
	float		start;
	float		end;

	float		startfade;
	qboolean	rotate;
	int			custom;
	
	qboolean	stopped;

	// Ridah
	int			rollBounceCount;

	int			accumroll;

	// BFP - Entity num
	int			entityNum;

	// BFP - Model
	qhandle_t	pmodel;
} cparticle_t;

typedef enum
{
	P_NONE,
	P_ANTIGRAV_ROCK, // BFP - Antigrav rock particles
	P_SMOKE,
	P_BUBBLE,
	P_BUBBLE_TURBULENT,
	P_AURA, // BFP - Particle aura
	P_SPARK, // BFP - Beam struggle spark
	P_ROCK_DEBRIS, // BFP - Bouncing rock fragment (explosion impact on ground)
	P_WATER_SPLASH, // BFP - Water entry splash bubble (upward arc, stops at surface)
	P_CONFETTI, // BFPR - Falling confetti/leaf-like square that flutters down and flattens on landing
	P_SPRITE
} particle_type_t;

#define		PARTICLE_GRAVITY	40
#define		MAX_PARTICLES	(1024 * 6)
#define		MAX_PARTICLES_3D	(1024 * 3)

static cparticle_t	*active_particles, *free_particles;
static cparticle_t	particles[MAX_PARTICLES];
static int		cl_numparticles = MAX_PARTICLES;

static qboolean		initparticles = qfalse;
static vec3_t			vforward, vright, vup;
static vec3_t			rforward, rright, rup;

static float			oldtime;

// BFP - NOTE: Particles use non-timescaled, before it was timescaled by game using cg.time
static int				timenonscaled;

#define NORMALSIZE	16
#define LARGESIZE	32

// BFP - Function to handle bubbles
static void CG_BubblesWaterHandling( cparticle_t *p, vec3_t org );
// BFP - Function to handle charge smoke particles
static void CG_ChargeSmokeHandling( cparticle_t *p, vec3_t org );

/*
===============
CL_ClearParticles
===============
*/
void CG_ClearParticles (void)
{
	int		i;

	// BFP - Limit the pool when 3D particles are active; the full array is
	// always allocated but only the first cl_numparticles slots are chained
	cl_numparticles = ( cg_3dparticles.integer > 0 ) ? MAX_PARTICLES_3D : MAX_PARTICLES;

	memset( particles, 0, sizeof(particles) );

	free_particles = &particles[0];
	active_particles = NULL;

	for (i=0 ;i<cl_numparticles ; i++)
	{
		particles[i].next = &particles[i+1];
		particles[i].type = 0;
	}
	particles[cl_numparticles-1].next = NULL;

	oldtime = timenonscaled;

	initparticles = qtrue;
}


/*
===================
AddSpriteParticle
===================
*/
static void AddSpriteParticle( cparticle_t *p, vec3_t org, float alpha )
{
	vec3_t		point;
	polyVert_t	verts[4];
	float		width, height;
	float		time, time2, ratio;
	vec3_t		rr, ru;
	vec3_t		rotate_ang;

	if ( !p->pshader ) {
		return;
	}

	time = timenonscaled - p->time;
	time2 = p->endtime - p->time;
	ratio = time / time2;

	width = p->width + ratio * ( p->endwidth - p->width );
	height = p->height + ratio * ( p->endheight - p->height );

	if ( p->rollBounceCount )
	{
		vectoangles( cg.refdef.viewaxis[0], rotate_ang );
		rotate_ang[ROLL] += p->rollBounceCount;
		AngleVectors( rotate_ang, NULL, rr, ru );
	}

	if ( p->rollBounceCount )
	{
		VectorMA( org, -height, ru, point );
		VectorMA( point, -width, rr, point );
	}
	else
	{
		VectorMA( org, -height, vup, point );
		VectorMA( point, -width, vright, point );
	}
	VectorCopy( point, verts[0].xyz );
	Vector2Set( verts[0].st, 0, 0 );
	Byte4Set( verts[0].modulate, 255, 255, 255, 255 );

	if ( p->rollBounceCount )
	{
		VectorMA( point, 2*height, ru, point );
	}
	else
	{
		VectorMA( point, 2*height, vup, point );
	}
	VectorCopy( point, verts[1].xyz );
	Vector2Set( verts[1].st, 0, 1 );
	Byte4Set( verts[1].modulate, 255, 255, 255, 255 );

	if ( p->rollBounceCount )
	{
		VectorMA( point, 2*width, rr, point );
	}
	else
	{
		VectorMA( point, 2*width, vright, point );
	}
	VectorCopy( point, verts[2].xyz );
	Vector2Set( verts[2].st, 1, 1 );
	Byte4Set( verts[2].modulate, 255, 255, 255, 255 );

	if ( p->rollBounceCount )
	{
		VectorMA( point, -2*height, ru, point );
	}
	else
	{
		VectorMA( point, -2*height, vup, point );
	}
	VectorCopy( point, verts[3].xyz );
	Vector2Set( verts[3].st, 1, 0 );
	Byte4Set( verts[3].modulate, 255, 255, 255, 255 );

	trap_R_AddPolyToScene( p->pshader, 4, verts );
}

/*
===================
AddSparkParticle
===================
*/
static void AddSparkParticle( cparticle_t *p, vec3_t org, float alpha )
{
	vec3_t		forward, right, endPoint;
	polyVert_t	verts[4];
	float		lifeFraction = (p->endtime - timenonscaled) / (p->endtime - p->time);
	const float	SPARK_MAX_LENGTH = 50.0f;
	const float	SPARK_MIN_LENGTH = 0.5f;
	const float	SPARK_MIN_WIDTH = 1.0f;
	float		length = SPARK_MIN_LENGTH + (SPARK_MAX_LENGTH - SPARK_MIN_LENGTH) * lifeFraction;
	float		currentWidth = SPARK_MIN_WIDTH + (p->width - SPARK_MIN_WIDTH) * lifeFraction;

	if ( !p->pshader ) {
		return;
	}

	p->width = currentWidth;

	// direction of the particle
	VectorNormalize2( p->vel, forward );

	VectorMA( org, length, forward, endPoint );
	PerpendicularVector( right, forward );

	// bottom-left
	VectorMA( org, -p->width, right, verts[0].xyz );
	Vector2Set( verts[0].st, 0, 0 );
	Byte4Set( verts[0].modulate, 255, 255, 255, 255 * alpha );

	// top-left
	VectorMA( org, p->width, right, verts[1].xyz );
	Vector2Set( verts[1].st, 0, 1 );
	Byte4Set( verts[1].modulate, 255, 255, 255, 255 * alpha );

	// top-right (tip narrower)
	VectorMA( endPoint, p->width * 0.2f, right, verts[2].xyz );
	Vector2Set( verts[2].st, 1, 1 );
	Byte4Set( verts[2].modulate, 255, 255, 255, 255 * alpha );

	// bottom-right = top-right (triangle)
	VectorCopy( verts[2].xyz, verts[3].xyz );
	Vector2Set( verts[3].st, 1, 0 );
	Byte4Set( verts[3].modulate, 255, 255, 255, 255 * alpha );

	trap_R_AddPolyToScene( p->pshader, 4, verts );
}

/*
===================
AddConfettiParticle
===================
*/
static void AddConfettiParticle( cparticle_t *p, vec3_t org, float alpha ) // BFPR - Confetti particle
{
	polyVert_t	verts[4];
	polyVert_t	backVerts[4];
	vec3_t		right, up;
	float		width, height;

	// BFP - All confetti physics (initial soft bounce, landing/flattening
	// against the slope, leaf-like flight) lives here because this is the
	// function that actually runs every frame for P_CONFETTI (see the switch
	// in CG_AddParticleToScene) -- the equivalent block inside
	// AddGenericParticle is never reached for this type.
	if ( !p->custom )
	{
		trace_t		trace;
		vec3_t		confettiMins = {0, 0, -2};
		qboolean	settled;
		float		dt = (float)( timenonscaled - p->time ) * 0.001f; // seconds since last update
		if ( dt < 0.0f ) {
			dt = 0.0f;
		} else if ( dt > 0.1f ) {
			dt = 0.1f; // clamp against huge frame gaps (loading, hitches)
		}

		settled = ( VectorLength( p->vel ) < 100 );

		// air drag on the burst velocity
		if ( !settled )
		{
			float	dragFactor = 1.0f - ( 2.2f * dt );
			if ( dragFactor < 0.0f ) {
				dragFactor = 0.0f;
			}
			VectorScale( p->vel, dragFactor, p->vel );
			settled = ( VectorLength( p->vel ) < 100 );
		}

		// same trace-every-frame pattern as P_ROCK_DEBRIS
		CG_Trace( &trace, p->org, confettiMins, confettiMins, org, -1, CONTENTS_SOLID );
		if ( trace.fraction < 1.0f )
		{
			VectorCopy( trace.endpos, p->org );
			VectorCopy( trace.endpos, org );

			if ( settled || trace.plane.normal[2] >= 0.7f )
			{
				VectorCopy( trace.plane.normal, p->accel );
				p->custom = 1;
				VectorClear( p->vel );
				VectorClear( p->accel );
			}
		}
		else
		{
			// No collision this frame -- just advance to the physical position
			VectorCopy( org, p->org );

			if ( settled )
			{
				// light, floaty gravity for the slow leaf-like fall
				p->accel[2] = -500;
				p->vel[2] = -130.0f;
			}
		}

		if ( settled )
		{
			p->accumroll += 3;
			p->rollBounceCount += 2;
		}
		else
		{
			// still carrying the explosion's burst speed: fast, erratic
			// tumble, like a piece of paper being thrown
			p->accumroll += 12;
			p->rollBounceCount += 9;
		}

		if ( p->accumroll >= 360 ) {
			p->accumroll -= 360;
		}
		if ( p->accumroll < 0 ) {
			p->accumroll += 360;
		}
		if ( p->rollBounceCount >= 360 ) {
			p->rollBounceCount -= 360;
		}
		if ( p->rollBounceCount < 0 ) {
			p->rollBounceCount += 360;
		}

		p->time = timenonscaled;
	}

	if ( p->custom )
	{
		vec3_t	normal, tangent;
		VectorCopy( p->accel, normal );
		if ( VectorLength( normal ) < 0.5f ) {
			VectorSet( normal, 0, 0, 1 ); // safety fallback
		}
		PerpendicularVector( tangent, normal );
		RotatePointAroundVector( right, normal, tangent, (float)p->accumroll );
		CrossProduct( normal, right, up );
		width = p->width;
		height = p->height;
	}
	else
	{
		vec3_t	angles;
		vectoangles( rforward, angles );
		angles[ROLL] += (float)p->accumroll;
		angles[PITCH] += (float)p->rollBounceCount;
		AngleVectors( angles, NULL, right, up );
		width = p->width;
		height = p->height;
	}

	VectorMA( org, -height, up, verts[0].xyz );
	VectorMA( verts[0].xyz, -width, right, verts[0].xyz );
	Vector2Set( verts[0].st, 0, 0 );
	Byte4Set( verts[0].modulate, p->confettiColor[0]*255, p->confettiColor[1]*255, p->confettiColor[2]*255, alpha*255 );

	VectorMA( org, -height, up, verts[1].xyz );
	VectorMA( verts[1].xyz, width, right, verts[1].xyz );
	Vector2Set( verts[1].st, 1, 0 );
	Byte4Set( verts[1].modulate, p->confettiColor[0]*255, p->confettiColor[1]*255, p->confettiColor[2]*255, alpha*255 );

	VectorMA( org, height, up, verts[2].xyz );
	VectorMA( verts[2].xyz, width, right, verts[2].xyz );
	Vector2Set( verts[2].st, 1, 1 );
	Byte4Set( verts[2].modulate, p->confettiColor[0]*255, p->confettiColor[1]*255, p->confettiColor[2]*255, alpha*255 );

	VectorMA( org, height, up, verts[3].xyz );
	VectorMA( verts[3].xyz, -width, right, verts[3].xyz );
	Vector2Set( verts[3].st, 0, 1 );
	Byte4Set( verts[3].modulate, p->confettiColor[0]*255, p->confettiColor[1]*255, p->confettiColor[2]*255, alpha*255 );

	trap_R_AddPolyToScene( p->pshader, 4, verts );

	// draw the back face too, since whiteShader has normal backface culling
	backVerts[0] = verts[0];
	backVerts[1] = verts[3];
	backVerts[2] = verts[2];
	backVerts[3] = verts[1];
	trap_R_AddPolyToScene( p->pshader, 4, backVerts );
}

/*
===================
AddGenericParticle
===================
*/
static void AddGenericParticle( cparticle_t *p, vec3_t org, float alpha )
{
	vec3_t		point;
	polyVert_t	verts[4];
	float		width, height;
	float		time, time2, ratio, invratio;
	vec3_t		color = {1.0f, 1.0f, 1.0f};
	vec3_t		rright2, rup2;

	time = timenonscaled - p->time;
	time2 = p->endtime - p->time;
	ratio = time / time2;

	if (p->color == BLOODRED)
		VectorSet (color, 0.22f, 0.0f, 0.0f);
	else if (p->color == GREY75)
	{
		float	len;
		float	greyit;
		float	val;
		len = Distance (cg.snap->ps.origin, org);
		if (!len)
			len = 1;

		val = 4096/len;
		greyit = 0.25 * val;
		if (greyit > 0.5)
			greyit = 0.5;

		VectorSet (color, greyit, greyit, greyit);
	}
	else
		VectorSet (color, 1.0, 1.0, 1.0);

	if ( timenonscaled > p->startfade )
	{
		invratio = 1.0f - ( (float)(timenonscaled - p->startfade) / (float)(p->endtime - p->startfade) );

		if (p->color == EMISIVEFADE)
		{
			float fval;
			fval = (invratio * invratio);
			if (fval < 0)
				fval = 0;
			VectorSet (color, fval , fval , fval );
		}
		invratio *= p->alpha;
	}
	else
	{
		invratio = p->alpha;
	}

	// BFP - Don't disappear opaquely the bubbles even the debris
	if ( invratio > 1.0f
	|| p->type == P_BUBBLE || p->type == P_BUBBLE_TURBULENT
	|| p->type == P_ROCK_DEBRIS || p->type == P_WATER_SPLASH )
	{
		invratio = 1.0f;
	}

	width = p->width + ratio * ( p->endwidth - p->width );
	height = p->height + ratio * ( p->endheight - p->height );

	// compute rotated axes for non-smoke and non-aura types
	if ( p->type != P_SMOKE && p->type != P_AURA
	&& p->type != P_BUBBLE && p->type != P_BUBBLE_TURBULENT
	&& p->type != P_WATER_SPLASH )
	{
		vec3_t temp;
		vectoangles( rforward, temp );
		if ( p->stopped ) {
			p->rollBounceCount = 0;
		}
		if ( p->rollBounceCount > 0 ) {
			p->accumroll += p->rollBounceCount;
			temp[ROLL] += p->accumroll * 0.1f;
		}
		AngleVectors( temp, NULL, rright2, rup2 );
	}
	else
	{
		VectorCopy( rright, rright2 );
		VectorCopy( rup, rup2 );
	}

	// BFP - Bubble types here
	if ( p->type == P_BUBBLE || p->type == P_BUBBLE_TURBULENT )
	{
		CG_BubblesWaterHandling( p, org );

		// BFP - Apply more end time to remove particles if the player stops charging
		if ( p->type == P_BUBBLE ) {
			if ( p->entityNum == cg.snap->ps.clientNum
			&& ( cg.snap->ps.legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_CHARGE
			&& !p->stopped )
			{
				p->endtime = timenonscaled + 2500 + (crandom() * 150);
				p->stopped = qtrue;
			}
		}
	}

	// BFP - Charge smoke particle handling here
	if ( p->type == P_SMOKE && p->custom == 1 )
	{
		CG_ChargeSmokeHandling( p, org );
	}

	// BFP - Antigrav rock type
	if ( p->type == P_ANTIGRAV_ROCK )
	{
		// BFP - When the particle, checked to be fallen, won't be reactivated when entering ki charging status again
		if ( p->stopped )
		{
			// BFP - To detect if there is something solid
			trace_t		trace;
			vec3_t		rockMins = {0, 0, -2}; // place a bit above
			// contents should be CONTENTS_SOLID, so the particles don't touch any entity like the player
			CG_Trace( &trace, p->org, rockMins, rockMins, org, -1, CONTENTS_SOLID );

			p->time = timenonscaled;
			p->custom = 1; // handle the p->custom when already entered in this phase for correction of client side visuals
			// not hit anything or not a collider
			if ( trace.fraction == 1.0f )
			{
				VectorCopy( org, p->org );
				p->vel[2] -= 50;
				p->accel[2] -= 200;
			}
			else // bouncing
			{
				if ( trace.plane.normal[2] >= 0.7f && Q_fabs( p->vel[2] ) < 1.0f ) {
					// stop bouncing
					VectorClear( p->vel );
					VectorClear( p->accel );
					VectorCopy( org, p->org );
				}
				else
				{
					// similar to CG_ReflectVelocity
					const float BOUNCEFACTOR = 0.5f;
					float dot = DotProduct( p->vel, trace.plane.normal );
					if ( trace.plane.normal[2] < 0.91f && VectorLength( p->vel ) < 110.0f ) {
						VectorMA( p->vel, 3.0f * dot, trace.plane.normal, p->vel );
						VectorScale( p->vel, 3.0f, p->vel );
					} else {
						VectorMA( p->vel, -2.0f * dot, trace.plane.normal, p->vel );
						VectorScale( p->vel, BOUNCEFACTOR, p->vel );
					}
					VectorCopy( trace.endpos, p->org );
				}
			}
		}

		// BFP - When reaching into this top, remove the particle!
		if ( org[2] > p->end )
		{
			p->next = NULL;
			p->color = p->alpha = 0;
			return;
		}
	}
	else if ( p->type == P_ROCK_DEBRIS || p->type == P_WATER_SPLASH )
	{
		// BFP - To detect if there is something solid
		trace_t		trace;
		vec3_t		debrisMins = {0, 0, -2}; // place a bit above
		int 		contents;
		// contents should be CONTENTS_SOLID, so the particles don't touch any entity like the player
		CG_Trace( &trace, p->org, debrisMins, debrisMins, org, -1, CONTENTS_SOLID );

		// keep detecting the position, also helps to pass through map bounds
		VectorCopy( org, p->org );

		p->time = timenonscaled;
		// not hit anything or not a collider
		contents = trap_CM_PointContents( trace.endpos, 0 );

		// bouncing rock
		if ( p->rollBounceCount <= 0 && !p->stopped ) { // stop
			VectorClear( p->accel );
			VectorClear( p->vel );
		}

		if ( trace.fraction == 1.0f ) {
			p->vel[2] -= (p->stopped) ? 100 : 80;
			p->accel[2] -= (p->stopped) ? 10 : 100;
		}
		else // bouncing
		{
			if ( p->stopped ) { // water touching something solid
				VectorClear( p->vel );
				VectorClear( p->accel );
				VectorCopy( trace.endpos, p->org );
				p->height = p->width *= 0.9f; // make it tinier when that happens
			}

			if ( trace.plane.normal[2] >= 0.7f && Q_fabs( p->vel[2] ) < 1.0f ) {
				// stop bouncing
				VectorClear( p->vel );
				VectorClear( p->accel );
				p->height = p->width *= 0.9f;
			}
			else
			{
				// similar to CG_ReflectVelocity
				const float BOUNCEFACTOR = 0.55f;
				float dot = DotProduct( p->vel, trace.plane.normal );
				if ( trace.plane.normal[2] < 0.91f && VectorLength( p->vel ) < 110.0f ) {
					VectorMA( p->vel, 3.0f * dot, trace.plane.normal, p->vel );
					VectorScale( p->vel, 3.0f, p->vel );
				} else {
					VectorMA( p->vel, -2.5f * dot, trace.plane.normal, p->vel );
					VectorScale( p->vel, BOUNCEFACTOR, p->vel );
				}
			}
		}

		// if it's assigned to water, then detect when going underwater and changing to P_BUBBLE type
		if ( p->stopped && ( contents & CONTENTS_WATER ) ) {
			p->type = P_BUBBLE_TURBULENT;
			p->endtime = timenonscaled + 600;
			VectorCopy( trace.endpos, p->org );
			p->vel[2] = p->accel[2] = 0;
		}

		// for a short time, the debris begin to get tinier
		if ( p->time > p->endtime - 500 ) {
			p->height = p->width *= 0.9f;
		}
	}

	// BFP - Render as 3D model
	if ( cg_3dparticles.integer > 0 && p->pmodel
	&& ( p->type == P_ANTIGRAV_ROCK || p->type == P_AURA || p->type == P_ROCK_DEBRIS
	|| p->type == P_WATER_SPLASH || p->type == P_BUBBLE || p->type == P_BUBBLE_TURBULENT ) )
	{
		refEntity_t	modelRef;
		vec3_t		angles;

		memset( &modelRef, 0, sizeof(modelRef) );
		modelRef.reType = RT_MODEL;
		modelRef.hModel = p->pmodel;
		if ( p->type == P_AURA || p->type == P_BUBBLE || p->type == P_BUBBLE_TURBULENT || p->type == P_WATER_SPLASH ) {
			modelRef.customShader = p->pshader;
		}

		// BFP - Advance yaw and pitch only while the rock is floating (ki active)
		// endwidth/endheight hold per-particle spin speeds set at spawn
		// startfade is reused as the pitch accumulator
		if ( p->type == P_ANTIGRAV_ROCK && !p->stopped ) {
			p->accumroll += p->endwidth;
			if ( p->accumroll >= 360.0f ) {
				p->accumroll -= 360.0f;
			}
			p->startfade += p->endheight;
			if ( p->startfade >= 360.0f ) {
				p->startfade -= 360.0f;
			}
		}

		AxisClear( modelRef.axis );
		VectorSet( angles, 
			( p->type == P_ANTIGRAV_ROCK ) ? p->startfade : 0, 
			(float)p->accumroll, 
			0 );
		AnglesToAxis( angles, modelRef.axis );

		VectorCopy( org, modelRef.origin );
		VectorCopy( org, modelRef.oldorigin );

		VectorScale( modelRef.axis[0], p->width, modelRef.axis[0] );
		VectorScale( modelRef.axis[1], p->width, modelRef.axis[1] );
		VectorScale( modelRef.axis[2], p->width, modelRef.axis[2] );
		modelRef.nonNormalizedAxes = qtrue;

		trap_R_AddRefEntityToScene( &modelRef );
		return;
	}

	// build the quad (non-3D case)
	if ( !p->pshader ) {
// (SA) temp commented out for DM
//		CG_Printf ("CG_AddParticleToScene type %d p->pshader == ZERO\n", p->type);
		return;
	}

	if (p->rotate)
	{
		VectorMA (org, -height, rup2, point);	
		VectorMA (point, -width, rright2, point);	
	}
	else
	{
		VectorMA (org, -p->height, vup, point);	
		VectorMA (point, -p->width, vright, point);	
	}
	VectorCopy( point, verts[0].xyz );
	Vector2Set( verts[0].st, 0, 0 );
	Byte4Set( verts[0].modulate, 255 * color[0], 255 * color[1], 255 * color[2], 255 * invratio );

	if (p->rotate)
	{
		VectorMA (org, -height, rup2, point);	
		VectorMA (point, width, rright2, point);	
	}
	else
	{
		VectorMA (org, -p->height, vup, point);	
		VectorMA (point, p->width, vright, point);	
	}
	VectorCopy( point, verts[1].xyz );
	Vector2Set( verts[1].st, 0, 1 );
	Byte4Set( verts[1].modulate, 255 * color[0], 255 * color[1], 255 * color[2], 255 * invratio );

	if (p->rotate)
	{
		VectorMA (org, height, rup2, point);	
		VectorMA (point, width, rright2, point);	
	}
	else
	{
		VectorMA (org, p->height, vup, point);	
		VectorMA (point, p->width, vright, point);	
	}
	VectorCopy( point, verts[2].xyz );
	Vector2Set( verts[2].st, 1, 1 );
	Byte4Set( verts[2].modulate, 255 * color[0], 255 * color[1], 255 * color[2], 255 * invratio );

	if (p->rotate)
	{
		VectorMA (org, height, rup2, point);	
		VectorMA (point, -width, rright2, point);	
	}
	else
	{
		VectorMA (org, p->height, vup, point);	
		VectorMA (point, -p->width, vright, point);	
	}
	VectorCopy( point, verts[3].xyz );
	Vector2Set( verts[3].st, 1, 0 );
	Byte4Set( verts[3].modulate, 255 * color[0], 255 * color[1], 255 * color[2], 255 * invratio );

	trap_R_AddPolyToScene( p->pshader, 4, verts );
}

/*
=====================
CG_AddParticleToScene
=====================
*/
void CG_AddParticleToScene (cparticle_t *p, vec3_t org, float alpha)
{
	// BFP - Don't draw if the particles are very far
	if ( Distance( cg.snap->ps.origin, org ) > 20000 ) {
		return;
	}

	switch (p->type)
	{
	case P_SPRITE:
		AddSpriteParticle(p, org, alpha);
		break;
	case P_SPARK:
		AddSparkParticle(p, org, alpha);
		break;
	case P_CONFETTI: // BFPR - Confetti particle
		AddConfettiParticle(p, org, alpha);
		break;
	default:
		AddGenericParticle(p, org, alpha);
		break;
	}
}


// Ridah, made this static so it doesn't interfere with other files
static float roll = 0.0;

/*
===============
CG_AddParticles
===============
*/
void CG_AddParticles (void)
{
	cparticle_t		*p, *next;
	float			alpha;
	float			time, time2;
	vec3_t			org;
	cparticle_t		*active, *tail;
	vec3_t			rotate_ang;

	timenonscaled = trap_Milliseconds(); // BFP - That's what the variable makes non-timescaled

	if (!initparticles)
		CG_ClearParticles ();

	VectorCopy( cg.refdef.viewaxis[0], vforward );
	VectorCopy( cg.refdef.viewaxis[1], vright );
	VectorCopy( cg.refdef.viewaxis[2], vup );

	vectoangles( cg.refdef.viewaxis[0], rotate_ang );
	roll += ((timenonscaled - oldtime) * 0.1) ;
	rotate_ang[ROLL] += (roll*0.9);
	AngleVectors ( rotate_ang, rforward, rright, rup);
	
	oldtime = timenonscaled;

	active = NULL;
	tail = NULL;

	for (p=active_particles ; p ; p=next)
	{
		next = p->next;

		time = (timenonscaled - p->time)*0.001;

		// BFP - Make alpha timescaled for smoke-like particles
		alpha = p->alpha + time*p->alphavel*(cg_timescale.value <= 0.1 ? 0.1 : cg_timescale.value);
		if (p->alphavel < 0.0f) p->alpha = alpha; // BFP - Alpha fading out
		if (p->alpha <= 0)
		{	// faded out
			p->next = free_particles;
			free_particles = p;
			p->type = p->color = p->alpha = 0;
			continue;
		}

		if (p->type == P_SMOKE
		|| p->type == P_BUBBLE || p->type == P_BUBBLE_TURBULENT // BFP - Add P_BUBBLE types to remove particles
		|| p->type == P_ANTIGRAV_ROCK // BFP - Add P_ANTIGRAV_ROCK to remove particles
		|| p->type == P_AURA // BFP - Add P_AURA to remove particles
		|| p->type == P_ROCK_DEBRIS // BFP - Add P_ROCK_DEBRIS to remove particles
		|| p->type == P_WATER_SPLASH // BFP - Add P_WATER_SPLASH to remove particles
		|| p->type == P_CONFETTI // BFPR - Add P_CONFETTI to remove particles
		|| p->type == P_SPARK) // BFP - Add P_SPARK to remove particles
		{
			if (timenonscaled > p->endtime)
			{
				p->next = free_particles;
				free_particles = p;
				p->color = p->alpha = 0;
				p->height = p->width = p->endheight = p->endwidth = 0;
				continue;
			}
		}

		if (p->type == P_SPRITE && p->endtime < 0) {
			// temporary sprite
			CG_AddParticleToScene (p, p->org, alpha);
			p->next = free_particles;
			free_particles = p;
			p->color = p->alpha = 0;
			continue;
		}

		p->next = NULL;
		if (!tail)
			active = tail = p;
		else
		{
			tail->next = p;
			tail = p;
		}

		if (alpha > 1.0)
			alpha = 1;

		time2 = time*time;

		org[0] = p->org[0] + p->vel[0]*time + p->accel[0]*time2;
		org[1] = p->org[1] + p->vel[1]*time + p->accel[1]*time2;
		org[2] = p->org[2] + p->vel[2]*time + p->accel[2]*time2;

		CG_AddParticleToScene (p, org, alpha);
	}

	active_particles = active;
}

void CG_ParticleBubble (centity_t *cent, qhandle_t pshader, qhandle_t pmodel, vec3_t origin, vec3_t origin2, int turbtime, float range, float size)
{
	cparticle_t	*p;

	// if (!pshader) CG_Printf ("CG_ParticleSnow pshader == ZERO!\n");

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;

	// BFP - Keep entity number to identify who is using
	p->entityNum = cent->currentState.number;

	// BFP - Add end time to remove particles, if there's no end time the particles will remain there
	p->endtime = timenonscaled + 600;
	p->startfade = timenonscaled + 200;

	// BFP - Monster gamemode, player monster bubble particles last a bit more
	if ( cgs.gametype == GT_MONSTER
	&& ( cent->currentState.eFlags & EF_MONSTER ) ) {
		p->endtime += 200;
		p->startfade += 200;
	}

	p->color = 0;
	p->alpha = 1;
	p->alphavel = 0;
	// BFP - Apply to entity's origin
	p->start = cent->currentState.origin[2];
	p->end = cent->currentState.origin2[2];
	p->pshader = pshader;
	p->pmodel = pmodel;
	p->height = p->width = (rand() % (int)size) + 1;
	if ( cg_3dparticles.integer > 0 && pmodel ) {
		p->width = p->width * 0.075;
		p->height = p->width;
	}

	VectorCopy(origin, p->org);

	if (turbtime)
	{
		p->type = P_BUBBLE_TURBULENT;
		// BFP - Apply end time to remove particles in that case, if there's no end time the particles will remain there
		p->endtime = timenonscaled + turbtime;
		p->height = p->width = (rand() % 2) + size;
		if ( cg_3dparticles.integer > 0 && pmodel ) {
			p->width = p->width * 0.075;
			p->height = p->width;
		}

		// BFP - Monster gamemode, player monster bubble particles has different spawning origin
		if ( cgs.gametype == GT_MONSTER
		&& ( cent->currentState.eFlags & EF_MONSTER ) ) {
			p->org[0] += (crandom() * range);
			p->org[1] += (crandom() * range);
		}

		VectorSet( p->vel, 
				crandom() * 300,
				crandom() * 300,
				30 * (rand() % (int)range) );

		// dispersion
		VectorSet( p->accel, 
				crandom() * 10, 
				crandom() * 10, 
				20 * (rand() % (int)range) );

		// avoid if both upwards are zero or less
		if ( p->vel[2] <= 0 ) p->vel[2] = 10 + (rand() % (int)range);
		if ( p->accel[2] <= 0 ) p->accel[2] = 10 + (rand() % (int)range);
	}
	else
	{
		// spawn in one point
		float angle = crandom() * M_PI;
		float radius = crandom() * range;

		p->type = P_BUBBLE;

		p->org[0] += cos( angle ) * radius;
		p->org[1] += sin( angle ) * radius;
		p->org[2] += (crandom() * 5);

		VectorSet( p->vel, 
				crandom() * 360,
				crandom() * 360,
				20 );

		// dispersion
		VectorSet( p->accel, 
				crandom() * 10, 
				crandom() * 10, 
				1200 );
	}

	p->accumroll = 0;
	p->custom = 3 - (crandom() * 6); // used to randomize where the bubbles stop when these touches the surface
	p->stopped = qfalse; // used to handle the bubbles when touching the surface
}

// BFP - Handle bubble particles when reaching to the top
static void CG_BubblesWaterHandling( cparticle_t *p, vec3_t org ) {
	trace_t		trace;
	vec3_t		start, end;
	int			contents;

	VectorCopy( org, end );
	end[2] -= 1;

	VectorCopy( org, start );
	start[2] += 10;

	// decelerate
	if ( Q_fabs(p->vel[0]) > 0 ) {
		p->vel[0] *= 0.995;
	}
	if ( Q_fabs(p->vel[1]) > 0 ) {
		p->vel[1] *= 0.995;
	}

	// trace down to find the surface
	trap_CM_BoxTrace( &trace, start, end, vec3_origin, vec3_origin, 0, CONTENTS_WATER );

	// if the particle is touching something solid, it will skip instead stopping
	contents = trap_CM_PointContents( trace.endpos, 0 );
	if ( contents & CONTENTS_SOLID ) { // remove when grazing something solid
		p->next = NULL;
		p->color = p->alpha = 0;
		return;
	}
	if ( !( contents & CONTENTS_WATER ) ) {
		p->time = timenonscaled;
		VectorCopy (trace.endpos, p->org);
		p->org[2] = trace.endpos[2] - p->custom;

		// stop going up and decrease dispersion speed
		p->vel[2] = 0;
		VectorClear( p->accel );

		// trace again if the bubble went outside, then set it near to the surface
		contents = trap_CM_PointContents( p->org, 0 );
		if ( !( contents & CONTENTS_WATER ) ) {
			VectorCopy (trace.endpos, p->org);
		}
		if ( p->type == P_BUBBLE ) {
			if ( p->vel[0] != 0 ) p->vel[0] *= 0.9;
			if ( p->vel[1] != 0 ) p->vel[1] *= 0.9;
			// stop after few milliseconds
			if ( p->stopped && p->time > p->endtime - 2250 ) {
				p->vel[0] = 0;
				p->vel[1] = 0;
			}
		} else {
			if ( p->vel[0] != 0 ) p->vel[0] *= 0.97;
			if ( p->vel[1] != 0 ) p->vel[1] *= 0.97;
		}
	}
}

// BFP - Particle for dash smoke when using ki boost and moving on the ground
void CG_ParticleDashSmoke (centity_t *cent, qhandle_t pshader, vec3_t origin, float size, float velocityDisp, float upVelocity, float accel)
{
	cparticle_t	*p;

	// if (!pshader) CG_Printf ("CG_ParticleDashSmoke pshader == ZERO!\n");

	// BFP - Don't spawn on pause
	if ( cg.frametime <= 0.0f ) {
		return;
	}

	// Too much smoke...
	// That cent->trailTime can be handled to avoid spawning too much and only spawn when the game isn't paused, hehehe :P
	if ( cent->trailTime > cg.time ) {
		return;
	}
	cent->trailTime += 35;
	if ( cent->trailTime < cg.time ) {
		cent->trailTime = cg.time;
	}

	if (!free_particles)
		return;

	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;

	p->time = timenonscaled;

	// BFP - Keep entity number to identify who is using
	p->entityNum = cent->currentState.clientNum;

	p->alpha = 0.45;
	p->alphavel = -0.1;

	p->pshader = pshader;
	p->start = cent->currentState.origin[2];
	p->end = cent->currentState.origin2[2];

	p->endtime = timenonscaled + 2000;
	p->startfade = timenonscaled + 100;

	p->height = p->width = size;

	p->endheight = p->height * 2;
	p->endwidth = p->width * 2;

	p->type = P_SMOKE;

	VectorCopy( origin, p->org );
	VectorSet( p->vel, 
				(rand() % (int)velocityDisp) - velocityDisp * 0.5,
				(rand() % (int)velocityDisp) - velocityDisp * 0.5,
				upVelocity );

	// dispersion
	VectorSet( p->accel, 
			crandom() * accel, 
			crandom() * accel, 
			1800 );

	p->stopped = qfalse; // to distinguish the type of smoke
}

// BFP - Particle for charge smoke when using ki charge near the ground
void CG_ParticleChargeSmoke (centity_t *cent, qhandle_t pshader, vec3_t origin, float size, float radialVel, float baseRadius)
{
	cparticle_t *p;
	float angle, radius;
	vec3_t dir, radial, angular;

	// if (!pshader) CG_Printf ("CG_ParticleChargeSmoke pshader == ZERO!\n");

	// Too much smoke...
	// That cent->pe.chargeSmokeTime can be handled to avoid spawning too much and only spawn when the game isn't paused, hehehe :P
	// It isn't possible reusing cent->trailTime, it would have client visual issues
#if 0
	if ( cent->pe.chargeSmokeTime > timenonscaled ) {
		return;
	}
	cent->pe.chargeSmokeTime += 10;
	if ( cent->pe.chargeSmokeTime < timenonscaled ) {
		cent->pe.chargeSmokeTime = timenonscaled;
	}
#endif

	if (!free_particles)
		return;

	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;

	p->time = timenonscaled;

	// BFP - Keep entity number to identify who is using
	p->entityNum = cent->currentState.clientNum;

	p->alpha = 0.31;
	p->alphavel = -0.09;

	p->pshader = pshader;
	p->start = cent->currentState.origin[2];
	p->end = cent->currentState.origin2[2];

	p->endtime = timenonscaled + 2000;
	p->startfade = timenonscaled + 100;

	p->height = p->width = size;

	p->endheight = p->height * 2;
	p->endwidth = p->width * 2;

	p->type = P_SMOKE;

	VectorCopy( origin, p->org );

	// randomize angle and radius for circular motion
	angle = crandom() * 360.0f;
	radius = baseRadius + crandom() * 10; // radius around the origin

	// compute radial direction
	dir[0] = cos( DEG2RAD( angle ) );
	dir[1] = sin( DEG2RAD( angle ) );
	dir[2] = 0;

	// initial position offset for circular spread
	VectorMA( origin, radius, dir, p->org );

	// velocity for circular motion
	VectorScale( dir, radialVel, radial ); // radial component of velocity
	VectorSet( angular, -dir[1], dir[0], 0 ); // perpendicular for tangential motion
	VectorScale( angular, 100, angular ); // tangential speed

	// combine radial and angular velocity
	p->vel[0] = radial[0] + angular[0];
	p->vel[1] = radial[1] + angular[1];
	p->vel[2] = radial[2] + angular[2];

	// vertical lift
	p->vel[2] = 50 + crandom() * 10;

	// dispersion
	VectorSet( p->accel, 
			crandom() * 20, 
			crandom() * 20, 
			5);

	p->stopped = qfalse;
	p->custom = 1;
}

// BFP - Handle charge smoke particles when touching something solid
static void CG_ChargeSmokeHandling( cparticle_t *p, vec3_t org ) {
	trace_t		trace;
	vec3_t		start, end;
	int			contents;

	VectorCopy( org, start );
	VectorCopy( org, end );

	// trace to check the collision
	trap_CM_BoxTrace( &trace, start, end, vec3_origin, vec3_origin, 0, CONTENTS_SOLID );

	contents = trap_CM_PointContents( trace.endpos, 0 );
	if ( contents & CONTENTS_SOLID ) { // remove when grazing something solid
		p->next = NULL;
		p->color = p->alpha = 0;
	}
}

// BFP - Antigrav rock particles for ki charging status
void CG_ParticleAntigravRock (qhandle_t pshader, qhandle_t pmodel, centity_t *cent, int entityNum, vec3_t origin, float size, float spawnRange, float endTime)
{
	cparticle_t	*p;

	// if (!pshader) CG_Printf ("CG_ParticleAntigravRock == ZERO!\n");

	// Too many rocks... That cent->dustTrailTime can be handled to avoid spawning too many, hehehe :P
	// cent->dustTrailTime was unused on Q3 before, so now it's being used for this kind of particles
	// reusing cent->trailTime would make the time more delayed to spawn the particles, so not visually good
	if ( cent->dustTrailTime > timenonscaled ) {
		return;
	}
	cent->dustTrailTime += 20;
	if ( cent->dustTrailTime < timenonscaled ) {
		cent->dustTrailTime = timenonscaled;
	}

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;

	// BFP - Keep entity number to identify who is using
	p->entityNum = entityNum;

	p->endtime = timenonscaled + endTime + (crandom() * 20);
	if ( cg_3dparticles.integer > 0 && pmodel ) {
		p->endtime += 350;
	}

	p->color = 0;
	p->alpha = 1;
	p->alphavel = 0;
	p->pshader = pshader;
	p->pmodel = pmodel;
	p->height = p->width = (rand() % (int)size) + size;
	if ( cg_3dparticles.integer > 0 && pmodel ) {
		p->width *= 12;
		p->height = p->width;
	}
	p->type = P_ANTIGRAV_ROCK;

	VectorCopy( origin, p->org );

	p->org[0] += (crandom() * spawnRange);
	p->org[1] += (crandom() * spawnRange);

	p->start = cent->currentState.origin[2];
	p->end = p->org[2] + 800 + (crandom() * 10);

	p->vel[0] = 0;
	p->vel[1] = 0;
	p->vel[2] = 450;
	if ( cg_3dparticles.integer > 0 && pmodel ) {
		p->vel[2] = 300;
	}

	p->accel[0] = 0;
	p->accel[1] = 0;
	p->accel[2] = 20;

	p->accumroll = rand() % 360;
	p->startfade = crandom() * 360; // initial pitch (degrees) - reused field
	p->endwidth = 1.5f + (crandom() * 2.5f); // yaw spin speed (deg/frame) - reused field
	p->endheight = 0.8f + (crandom() * 1.5f); // pitch spin speed (deg/frame) - reused field
	// BFP - rollBounceCount is reused by other particle types for unrelated
	// purposes (P_CONFETTI's pitch tumble accumulator, P_ROCK_DEBRIS's bounce
	// counter). AddGenericParticle reads it as an extra per-frame roll speed
	// for every type routed through it (including this one), so a particle
	// recycled from the pool without resetting it here would spin this rock
	// far too fast, using whatever leftover value the previous owner left
	// behind. Must always be explicitly zeroed on spawn.
	p->rollBounceCount = 0;
	p->stopped = qfalse; // to handle the ki charging status
	p->custom = 0; // to handle the client side visuals
}

// BFP - To handle the client side visuals of antigrav rock particles
void CG_AntigravRockHandling (centity_t *cent)
{
	cparticle_t		*p, *next;

	for (p=active_particles ; p ; p=next)
	{
		next = p->next;
		if ( p->type != P_ANTIGRAV_ROCK ) continue;

		if ( p->entityNum == cent->currentState.clientNum
		&& !( cent->currentState.eFlags & EF_DEAD )
		&& ( ( !( cent->currentState.eFlags & EF_AURA ) && !( cg.time < cent->pe.tierAuraTime ) )
			|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_CHARGE
			|| ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_CHARGE && !( cent->currentState.eFlags & EF_AURA ) && !( cg.time < cent->pe.tierAuraTime ) ) )
		&& !p->stopped ) { // BFP - Make each particle fall when they aren't on ki charging status
			p->endtime = timenonscaled + 1650;
			p->stopped = qtrue;
		}
	}
}

// BFP - Particle aura
void CG_ParticleAura (centity_t *cent, int entityNum, qhandle_t pshader, qhandle_t pmodel, vec3_t origin, vec3_t origin2, float range)
{
	cparticle_t	*p;

	// if (!pshader) CG_Printf ("CG_ParticleAura pshader == ZERO!\n");

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;

	// BFP - Keep entity number to identify who is using
	p->entityNum = entityNum;

	// BFP - Add end time to remove particles, if there's no end time the particles will remain there
	p->endtime = timenonscaled + 400;
	p->startfade = timenonscaled + 200;

	p->color = 0;
	p->alpha = 0.5;
	p->alphavel = -0.075;

	// BFP - Apply to player's origin
	p->start = cent->currentState.origin[2];
	p->end = cent->currentState.origin2[2];
	p->pshader = pshader;
	p->pmodel = pmodel;
	p->height = p->width = 40;
	if ( cg_3dparticles.integer > 0 && pmodel ) {
		p->width *= 0.015f;
		p->height = p->width;
	}

	VectorCopy(origin, p->org);

	p->type = P_AURA;

	p->org[0] += (crandom() * range);
	p->org[1] += (crandom() * range);
	p->org[2] += (crandom() * 5);

	VectorSet( p->vel, 
			crandom() * 360,
			crandom() * 360,
			100 );

	// dispersion
	VectorSet( p->accel, 
			crandom() * 20, 
			crandom() * 20, 
			1200 );

	// BFP - Monster gamemode, player monster aura particles are bigger
	if ( cent->currentState.eFlags & EF_MONSTER ) {
		p->org[2] -= 50;
		p->height *= 3.75;
		p->width *= 3.75;
		VectorScale( p->vel, 3.75, p->vel );
		VectorScale( p->accel, 3.75, p->accel );
	}

	p->stopped = qfalse;
}

// BFP - To handle the client side visuals of aura particle
void CG_ParticleAuraHandling (centity_t *cent)
{
	cparticle_t		*p, *next;

	for (p=active_particles ; p ; p=next)
	{
		next = p->next;
		if ( p->type != P_AURA ) continue;

		if ( p->entityNum == cent->currentState.clientNum
		&& !( cent->currentState.eFlags & EF_DEAD )
		&& !( cent->currentState.eFlags & EF_AURA ) 
		&& !p->stopped ) { // BFP - Make each particle fall when there's no aura at this moment
			p->alphavel = -0.03;
			p->accel[0] = 0;
			p->accel[1] = 0;
			p->endtime = timenonscaled + 600;
			p->stopped = qtrue;
		}
	}
}

// BFP - Spawn a bouncing rock fragment (explosion debris on ground)
void CG_ParticleRockDebris (qhandle_t pshader, qhandle_t pmodel, vec3_t origin, vec3_t vel, float size, float velocity, float accel)
{
	cparticle_t	*p;

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;

	p->startfade = timenonscaled + 200;
	p->endtime = timenonscaled + 1200;

	p->color = 0;
	p->alpha = 1;
	p->alphavel = 0;
	p->pshader = pshader;
	p->pmodel = pmodel;
	p->height = p->width = (rand() % 6) + 3;
	if ( cg_3dparticles.integer > 0 && pmodel ) {
		p->width *= 12;
		p->height = p->width;
	}

	p->type = P_ROCK_DEBRIS;

	VectorCopy( origin, p->org );
	p->start = origin[2];

	VectorCopy( vel, p->vel );

	p->accel[0] = (crandom() * accel);
	p->accel[1] = (crandom() * accel);
	p->accel[2] = accel + (crandom() * 50);

	p->rollBounceCount = 3;       // bounce counter
	p->stopped = qfalse;
	p->custom = 0;

	// BFP - Random fixed yaw per rock
	p->accumroll = rand() % 360;
}

// BFP - Spawn a water entry splash bubble (arcs upward, stops at water surface)
void CG_ParticleWaterSplash (qhandle_t pshader, qhandle_t pmodel, vec3_t origin, vec3_t vel, float size, float velocity, float accel)
{
	cparticle_t	*p;

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;

	p->startfade = timenonscaled + 200;
	p->endtime = timenonscaled + 2450 + (crandom() * 20);

	p->color = 0;
	p->alpha = 1;
	p->alphavel = 0;
	p->pshader = pshader;
	p->pmodel = pmodel;
	p->height = p->width = (rand() % (int)size) + size * 0.335f;
	if ( cg_3dparticles.integer > 0 && pmodel ) {
		p->width *= 0.075f;
		p->height = p->width;
	}

	p->type = P_WATER_SPLASH;

	VectorCopy( origin, p->org );
	p->org[0] += (crandom() * 15);
	p->org[1] += (crandom() * 15);
	p->start = origin[2];

	p->vel[0] = (crandom() * velocity);
	p->vel[1] = (crandom() * velocity);
	p->vel[2] = velocity * 7;

	p->accel[0] = (crandom() * accel);
	p->accel[1] = (crandom() * accel);
	p->accel[2] = accel + (crandom() * 50);

	p->rollBounceCount = 0;
	p->accumroll = 0;
	p->stopped = qtrue;   // stops when it reaches the water surface
	p->custom = 0;
}

void CG_ParticleSparks (qhandle_t pshader, vec3_t origin, vec3_t vel)
{
	cparticle_t	*p;

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;
	p->endtime = timenonscaled + 250;

	p->color = 0;
	p->alpha = 1;
	p->alphavel = 0;
	p->pshader = pshader;
	p->height = p->width = (rand() % 6) + 5;

	p->type = P_SPARK;

	VectorCopy( origin, p->org );
	p->start = origin[2];
	VectorCopy( vel, p->vel );

	p->accel[0] = (crandom() * 600);
	p->accel[1] = (crandom() * 600);
	p->accel[2] = 50 + (crandom() * 25);

	p->rollBounceCount = 0;
	p->stopped = qfalse;
	p->custom = 1; // enable gravity
}

void CG_ParticleBeamStruggleSpark (qhandle_t pshader, vec3_t origin, vec3_t vel)
{
	cparticle_t	*p;

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;
	p->endtime = timenonscaled + 250;

	VectorCopy( origin, p->org );
	p->start = origin[2];
	VectorCopy( vel, p->vel );

	p->accel[0] = (crandom() * 300);
	p->accel[1] = (crandom() * 300);
	p->accel[2] = -10 - (crandom() * 20);

	p->type = P_SPARK;
	p->color = 0;
	p->alpha = 1.0;
	p->alphavel = 0;
	p->pshader = pshader;
	p->height = p->width = (rand() % 4) + 2;
}

// BFPR - Confetti particle
void CG_ParticleConfetti (qhandle_t pshader, vec3_t origin, vec3_t dir, int num, float size, float speed) {
	int i;
	vec3_t right, up, forward;
	vec3_t vel, spawnOrg;
	float sz, rSpread, uSpread;

	if ( num <= 0 ) {
		return;
	}

	// Build axes for the spread
	VectorCopy( dir, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );
	VectorNormalize( right );
	VectorNormalize( up );

	for ( i = 0; i < num; i++ ) {
		cparticle_t *p;

		if ( !free_particles ) return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->time = timenonscaled;
		p->endtime = timenonscaled + 8000 + (rand() % 2000); // 8-10 seconds lifetime
		p->startfade = p->endtime - 500; // starts fading out near the end

		VectorSet( p->confettiColor, random(), random(), random() );
		p->alpha = 1.0f;
		p->alphavel = 0.0f; // fade is handled manually

		p->pmodel = 0;
		p->type = P_CONFETTI;
		p->pshader = pshader;

		// initial position with a bit of spread
		VectorCopy( origin, spawnOrg );
		spawnOrg[0] += (rand() % 20) - 10;
		spawnOrg[1] += (rand() % 20) - 10;
		spawnOrg[2] += (rand() % 20) - 10;
		VectorCopy( spawnOrg, p->org );

		// initial explosive burst velocity
		rSpread = (rand() % 1800) - 1000;
		uSpread = (rand() % 4500) - 1000;
		VectorScale( dir, speed, vel );
		VectorMA( vel, rSpread, right, vel );
		VectorMA( vel, uSpread, up, vel );
		VectorCopy( vel, p->vel );

		p->accel[0] = 0;
		p->accel[1] = 0;
		p->accel[2] = -300.0f; // gravity

		// random size
		sz = size * (0.5f + random());
		p->width = sz;
		p->height = sz;
		p->endwidth = sz;
		p->endheight = sz;

		// random initial rotation
		p->rollBounceCount = rand() % 360;
		p->accumroll = 360;

		// phase/frequency for the leaf-like sideways sway
		p->start = crandom() * (float)M_PI * 2.0f;
		p->end = 1.5f + (crandom() * 1.5f);

		p->stopped = qfalse;
		p->custom = 0; // 0 = still airborne, 1 = landed and flattened
	}
}
