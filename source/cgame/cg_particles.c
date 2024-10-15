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
	int			snum;
	
	qboolean	link;

	// Ridah
	int			shaderAnim;
	int			roll;

	int			accumroll;

	// BFP - Entity num
	int			entityNum;

} cparticle_t;

typedef enum
{
	P_NONE,
	P_ANTIGRAV_ROCK, // BFP - Antigrav rock particles
	P_SMOKE,
	P_ANIM,	// Ridah
	P_BLEED,
	P_DEBRIS, // BFP - Debris
	P_SMOKE_IMPACT,
	P_BUBBLE,
	P_BUBBLE_TURBULENT,
	P_AURA, // BFP - Particle aura
	P_SPRITE
// BFP - Unused particle types, saved for later :P
#if 0
	P_FLAT,
	P_FLAT_SCALEUP,
	P_FLAT_SCALEUP_FADE,
	P_WEATHER,
	P_WEATHER_TURBULENT,
	P_WEATHER_FLURRY,
#endif
} particle_type_t;

#define	MAX_SHADER_ANIMS		32
#define	MAX_SHADER_ANIM_FRAMES	64

static char *shaderAnimNames[MAX_SHADER_ANIMS] = {
	"explode1",
	"blacksmokeanim",
	"twiltb2",
	"expblue",
	"blacksmokeanimb",	// uses 'explode1' sequence
	"blood",
	NULL
};
static qhandle_t shaderAnims[MAX_SHADER_ANIMS][MAX_SHADER_ANIM_FRAMES];
static int	shaderAnimCounts[MAX_SHADER_ANIMS] = {
	23,
	25,
	45,
	25,
	23,
	5,
};
static float	shaderAnimSTRatio[MAX_SHADER_ANIMS] = {
	1.405f,
	1.0f,
	1.0f,
	1.0f,
	1.0f,
	1.0f,
};
static int	numShaderAnims;
// done.

#define		PARTICLE_GRAVITY	40
#define		MAX_PARTICLES	1024 * 3

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
void CG_BubblesWaterHandling( cparticle_t *p, vec3_t org );

/*
===============
CL_ClearParticles
===============
*/
void CG_ClearParticles (void)
{
	int		i;

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

	// Ridah, init the shaderAnims
	for (i=0; shaderAnimNames[i]; i++) {
		int j;

		for (j=0; j<shaderAnimCounts[i]; j++) {
			shaderAnims[i][j] = trap_R_RegisterShader( va("%s%i", shaderAnimNames[i], j+1) );
		}
	}
	numShaderAnims = i;
	// done.

	initparticles = qtrue;
}


/*
=====================
CG_AddParticleToScene
=====================
*/
void CG_AddParticleToScene (cparticle_t *p, vec3_t org, float alpha)
{
	vec3_t		point;
	polyVert_t	verts[4];
	float		width;
	float		height;
	float		time, time2;
	float		ratio;
	float		invratio;
	vec3_t		color;
	polyVert_t	TRIverts[3];
	vec3_t		rright2, rup2;

#if 0 /* // BFP - Unused particle type conditionals  */
	if (p->type == P_WEATHER || p->type == P_WEATHER_TURBULENT || p->type == P_WEATHER_FLURRY)
	{// create a front facing polygon
			
		if (p->type != P_WEATHER_FLURRY)
		{
			if (org[2] < p->end)			
			{	
				p->time = timenonscaled;
				VectorCopy (org, p->org); // Ridah, fixes rare snow flakes that flicker on the ground
								
				while (p->org[2] < p->end) 
				{
					p->org[2] += (p->start - p->end); 
				}
				
				if (p->type == P_WEATHER_TURBULENT)
				{
					p->vel[0] = crandom() * 16;
					p->vel[1] = crandom() * 16;
				}
			
			}

			// Rafael snow pvs check
			if (!p->link)
				return;

			p->alpha = 1;
		}
		
		// Ridah, had to do this or MAX_POLYS is being exceeded in village1.bsp
		if (Distance( cg.snap->ps.origin, org ) > 1024) {
			return;
		}
		// done.

		VectorMA (org, -p->height, vup, point);
		VectorMA (point, -p->width, vright, point);
		VectorCopy( point, TRIverts[0].xyz );
		VectorArray2Set( TRIverts[0].st, 1, 0 );
		Vector4Set( TRIverts[0].modulate, 255, 255, 255, 255 * p->alpha );

		VectorMA (org, p->height, vup, point);
		VectorMA (point, -p->width, vright, point);
		VectorCopy (point, TRIverts[1].xyz);
		VectorArray2Set( TRIverts[0].st, 0, 0 );
		Vector4Set( TRIverts[1].modulate, 255, 255, 255, 255 * p->alpha );

		VectorMA (org, p->height, vup, point);
		VectorMA (point, p->width, vright, point);
		VectorCopy (point, TRIverts[2].xyz);
		VectorArray2Set( TRIverts[0].st, 0, 1 );
		Vector4Set( TRIverts[2].modulate, 255, 255, 255, 255 * p->alpha );
	}
	// else if ...
#endif
	if (p->type == P_SPRITE)
	{
		vec3_t	rr, ru;
		vec3_t	rotate_ang;

		VectorSet (color, 1.0, 1.0, 1.0);
		time = timenonscaled - p->time;
		time2 = p->endtime - p->time;
		ratio = time / time2;

		width = p->width + ( ratio * ( p->endwidth - p->width) );
		height = p->height + ( ratio * ( p->endheight - p->height) );

		if (p->roll) {
			vectoangles( cg.refdef.viewaxis[0], rotate_ang );
			rotate_ang[ROLL] += p->roll;
			AngleVectors ( rotate_ang, NULL, rr, ru);
		}

		if (p->roll) {
			VectorMA (org, -height, ru, point);	
			VectorMA (point, -width, rr, point);	
		} else {
			VectorMA (org, -height, vup, point);	
			VectorMA (point, -width, vright, point);	
		}
		VectorCopy (point, verts[0].xyz);	
		VectorArray2Set( verts[0].st, 0, 0 );
		Vector4Set( verts[0].modulate, 255, 255, 255, 255 );

		if (p->roll) {
			VectorMA (point, 2*height, ru, point);	
		} else {
			VectorMA (point, 2*height, vup, point);	
		}
		VectorCopy (point, verts[1].xyz);	
		VectorArray2Set( verts[1].st, 0, 1 );
		Vector4Set( verts[1].modulate, 255, 255, 255, 255 );

		if (p->roll) {
			VectorMA (point, 2*width, rr, point);	
		} else {
			VectorMA (point, 2*width, vright, point);	
		}
		VectorCopy (point, verts[2].xyz);	
		VectorArray2Set( verts[2].st, 1, 1 );
		Vector4Set( verts[2].modulate, 255, 255, 255, 255 );

		if (p->roll) {
			VectorMA (point, -2*height, ru, point);	
		} else {
			VectorMA (point, -2*height, vup, point);	
		}
		VectorCopy (point, verts[3].xyz);	
		VectorArray2Set( verts[3].st, 1, 0 );
		Vector4Set( verts[3].modulate, 255, 255, 255, 255 );
	}
	else if (p->type == P_SMOKE || p->type == P_SMOKE_IMPACT
	|| p->type == P_BUBBLE || p->type == P_BUBBLE_TURBULENT // BFP - Bubble types moved here for better management
	|| p->type == P_ANTIGRAV_ROCK // BFP - Added antigrav rock type
	|| p->type == P_AURA // BFP - Particle aura
	|| p->type == P_DEBRIS) // BFP - Debris type
	{// create a front rotating facing polygon

// BFP - No smoke distance
#if 0
		if ( p->type == P_SMOKE_IMPACT && Distance( cg.snap->ps.origin, org ) > 1024) {
			return;
		}
#endif

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

		time = timenonscaled - p->time;
		time2 = p->endtime - p->time;
		ratio = time / time2;
		
		if (timenonscaled > p->startfade)
		{
			invratio = 1 - ( (timenonscaled - p->startfade) / (p->endtime - p->startfade) );

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
			invratio = 1 * p->alpha;

		if (invratio > 1 || p->type == P_BUBBLE || p->type == P_BUBBLE_TURBULENT || p->type == P_DEBRIS) // BFP - Don't disappear opaquely the bubbles even the debris
			invratio = 1;
	
		width = p->width + ( ratio * ( p->endwidth - p->width) );
		height = p->height + ( ratio * ( p->endheight - p->height) );

		if (p->type != P_SMOKE_IMPACT 
		&& p->type != P_AURA) // BFP - Don't apply for P_AURA particle position
		{
			vec3_t temp;

			vectoangles (rforward, temp);
			p->accumroll += p->roll;
			temp[ROLL] += p->accumroll * 0.1;
			AngleVectors ( temp, NULL, rright2, rup2);
		}
		else
		{
			VectorCopy (rright, rright2);
			VectorCopy (rup, rup2);
		}

		// BFP - Bubble types here
		if (p->type == P_BUBBLE || p->type == P_BUBBLE_TURBULENT)
		{
			CG_BubblesWaterHandling( p, org );

			// BFP - Apply more end time to remove particles if the player stops charging
			if (p->type == P_BUBBLE) {
				if ( p->entityNum == cg.snap->ps.clientNum 
				&& ( cg.snap->ps.legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_CHARGE
				&& !p->link )
				{
					p->endtime = timenonscaled + 2500 + (crandom() * 150);
					p->link = qtrue;
				}
			}
		}

		// BFP - Antigrav rock type
		if (p->type == P_ANTIGRAV_ROCK)
		{
			// BFP - When the particle, checked to be fallen, won't be reactivated when entering ki charging status again
			if (p->link)
			{
				// BFP - To detect if there is something solid
				trace_t		trace;
				vec3_t		rockMins = {0, 0, -1}; // place a bit above
				// contents should be CONTENTS_SOLID, so the particles don't touch any entity like the player
				CG_Trace( &trace, p->org, rockMins, rockMins, org, -1, CONTENTS_SOLID );

				p->time = timenonscaled;
				p->snum = 1; // handle the p->snum when already entered in this phase for correction of client side visuals
				p->vel[0] = 0;
				p->vel[1] = 0;
				p->accel[0] = 0;
				p->accel[1] = 0;
				// not hit anything or not a collider
				if ( trace.fraction == 1.0f )
				{
					VectorCopy (org, p->org);
					p->vel[2] -= 50;
					p->accel[2] -= 200;
				}
				else // bouncing
				{
					// if the particle is touching a mover and moves down, so keep bouncing
					if ( trace.fraction <= 0 ) {
						p->roll = 6;
					} else {
						p->vel[2] = p->accel[2] = (p->roll > 0) ? 50 * p->roll : 0;
						p->roll--; // that decreases bounces 
					}
				}
				if ( p->roll <= 0 ) { // keep detecting the position
					VectorCopy (trace.endpos, p->org);
				}
			}

			// BFP - When reaching into this top, remove the particle!
			if (org[2] > p->end)
			{
				p->next = NULL;
				p->type = p->color = p->alpha = p->snum = 0;
			}
		}
		else if (p->type == P_DEBRIS) // BFP - Debris type
		{
			// BFP - To detect if there is something solid
			trace_t		trace;
			vec3_t		debrisMins = {0, 0, -1}; // place a bit above
			vec3_t		up = {0, 0, 1};
			int 		contents;
			// contents should be CONTENTS_SOLID, so the particles don't touch any entity like the player
			CG_Trace( &trace, p->org, debrisMins, debrisMins, org, -1, CONTENTS_SOLID );

			// keep detecting the position, also helps to pass through map bounds
			VectorCopy ( org, p->org );

			p->time = timenonscaled;
			// not hit anything or not a collider
			contents = trap_CM_PointContents( trace.endpos, 0 );
			if ( p->roll <= 0 && !p->link && p->snum <= 0 ) { // stop
				VectorClear( p->accel );
				VectorClear( p->vel );
			}

			if ( p->snum > 0 ) { // for sparks
				p->vel[2] -= 110;
				p->accel[2] -= 150;
			} else {
				if ( trace.fraction == 1.0f ) {
					p->vel[2] -= (p->link) ? 100 : 80;
					p->accel[2] -= (p->link) ? 10 : 100;
				}
				else // bouncing
				{
					// if it's on a slope
					if ( DotProduct( trace.plane.normal, up ) < 0.7 ) {
						if ( fabs( p->vel[2] ) < 1 ) {
							VectorClear( p->accel );
							VectorClear( p->vel );
							p->height = p->width *= 0.9; // make it tinier when that happens
						} else {
							p->vel[2] -= (p->link) ? 100 : 80;
							p->accel[2] -= (p->link) ? 10 : 100;
						}
					} else {
						if ( trace.fraction <= 0 ) {
							p->roll = 3;
						} else {
							p->vel[2] = (p->roll > 0) ? 500 * p->roll : 0;
							p->accel[2] = (p->roll > 0) ? 500 * p->roll : 0;
							p->roll--; // decreases bounces
						}
					}
				}
			}

			// if it's assigned to water, then detect when going underwater and changing to P_BUBBLE type
			if ( p->link && ( contents & CONTENTS_WATER ) ) {
				p->type = P_BUBBLE_TURBULENT;
				p->endtime = timenonscaled + 600;
				VectorCopy (trace.endpos, p->org);
				p->vel[2] = p->accel[2] = 0;
			}

			// for a short time, the debris begin to get tinier
			if ( p->time > p->endtime - 500 ) {
				p->height = p->width *= 0.9;
			}
			if ( p->width <= 0.1 ) {
				p->next = NULL;
				p->type = p->color = p->alpha = 0;
			}
		}

		if (p->type == P_DEBRIS && p->snum > 0) { // render spark particles
			vec3_t	forward, right, endPoint;
			float	length;

			// direction of the particle
			VectorNormalize2( p->vel, forward );

			length = VectorLength( p->vel ) * 0.015f; // multiplier to tweak trail length
			VectorMA( org, length, forward, endPoint );
			PerpendicularVector( right, forward );

			// bottom-left
			VectorMA( org, -p->width, right, verts[0].xyz );
			VectorArray2Set( verts[0].st, 0, 0 );
			Vector4Set( verts[0].modulate, 255, 255, 255, 255 * alpha );

			// top-left
			VectorMA( org, p->width, right, verts[1].xyz );
			VectorArray2Set( verts[1].st, 0, 1 );
			Vector4Set( verts[1].modulate, 255, 255, 255, 255 * alpha );

			// top-right
			VectorMA( endPoint, p->width * 0.2f, right, verts[2].xyz ); // make the tip narrower
			VectorArray2Set( verts[2].st, 1, 1 );
			Vector4Set( verts[2].modulate, 255, 255, 255, 255 * alpha );

			// bottom-right, but copied to top-right, making it similar to a triangle
			VectorCopy( verts[2].xyz, verts[3].xyz );
			VectorArray2Set( verts[3].st, 1, 0 );
			Vector4Set( verts[3].modulate, 255, 255, 255, 255 * alpha );

			trap_R_AddPolyToScene( p->pshader, 4, verts );
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
		VectorCopy (point, verts[0].xyz);	
		VectorArray2Set( verts[0].st, 0, 0 );
		Vector4Set( verts[0].modulate, 255 * color[0], 255 * color[1], 255 * color[2], 255 * invratio );

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
		VectorCopy (point, verts[1].xyz);	
		VectorArray2Set( verts[1].st, 0, 1 );
		Vector4Set( verts[1].modulate, 255 * color[0], 255 * color[1], 255 * color[2], 255 * invratio );

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
		VectorCopy (point, verts[2].xyz);	
		VectorArray2Set( verts[2].st, 1, 1 );
		Vector4Set( verts[2].modulate, 255 * color[0], 255 * color[1], 255 * color[2], 255 * invratio );

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
		VectorCopy (point, verts[3].xyz);	
		VectorArray2Set( verts[3].st, 1, 0 );
		Vector4Set( verts[3].modulate, 255 * color[0], 255 * color[1], 255 * color[2], 255 * invratio );
	}
	else if (p->type == P_BLEED)
	{
		vec3_t	rr, ru;
		vec3_t	rotate_ang;
		float	alpha;

		alpha = p->alpha;
		
		if (p->roll) 
		{
			vectoangles( cg.refdef.viewaxis[0], rotate_ang );
			rotate_ang[ROLL] += p->roll;
			AngleVectors ( rotate_ang, NULL, rr, ru);
		}
		else
		{
			VectorCopy (vup, ru);
			VectorCopy (vright, rr);
		}

		VectorMA (org, -p->height, ru, point);	
		VectorMA (point, -p->width, rr, point);	
		VectorCopy (point, verts[0].xyz);	
		VectorArray2Set( verts[0].st, 0, 0 );
		Vector4Set( verts[0].modulate, 111, 19, 9, 255 * alpha );

		VectorMA (org, -p->height, ru, point);	
		VectorMA (point, p->width, rr, point);	
		VectorCopy (point, verts[1].xyz);	
		VectorArray2Set( verts[1].st, 0, 1 );
		Vector4Set( verts[1].modulate, 111, 19, 9, 255 * alpha );

		VectorMA (org, p->height, ru, point);	
		VectorMA (point, p->width, rr, point);	
		VectorCopy (point, verts[2].xyz);		
		VectorArray2Set( verts[2].st, 1, 1 );
		Vector4Set( verts[2].modulate, 111, 19, 9, 255 * alpha );

		VectorMA (org, p->height, ru, point);	
		VectorMA (point, -p->width, rr, point);	
		VectorCopy (point, verts[3].xyz);	
		VectorArray2Set( verts[3].st, 1, 0 );
		Vector4Set( verts[3].modulate, 111, 19, 9, 255 * alpha );

	}
#if 0 /* // BFP - Unused particle type conditionals  */
	else if (p->type == P_FLAT_SCALEUP)
	{
		float width, height;
		float sinR, cosR;

		if (p->color == BLOODRED)
			VectorSet (color, 1, 1, 1);
		else
			VectorSet (color, 0.5, 0.5, 0.5);
		
		time = timenonscaled - p->time;
		time2 = p->endtime - p->time;
		ratio = time / time2;

		width = p->width + ( ratio * ( p->endwidth - p->width) );
		height = p->height + ( ratio * ( p->endheight - p->height) );

		if (width > p->endwidth)
			width = p->endwidth;

		if (height > p->endheight)
			height = p->endheight;

		sinR = height * sin(DEG2RAD(p->roll)) * sqrt(2);
		cosR = width * cos(DEG2RAD(p->roll)) * sqrt(2);

		VectorCopy (org, verts[0].xyz);	
		verts[0].xyz[0] -= sinR;
		verts[0].xyz[1] -= cosR;
		VectorArray2Set( verts[0].st, 0, 0 );
		Vector4Set( verts[0].modulate, 255 * color[0], 255 * color[1], 255 * color[2], 255 );

		VectorCopy (org, verts[1].xyz);	
		verts[1].xyz[0] -= cosR;	
		verts[1].xyz[1] += sinR;	
		VectorArray2Set( verts[1].st, 0, 1 );
		Vector4Set( verts[1].modulate, 255 * color[0], 255 * color[1], 255 * color[2], 255 );

		VectorCopy (org, verts[2].xyz);	
		verts[2].xyz[0] += sinR;	
		verts[2].xyz[1] += cosR;	
		VectorArray2Set( verts[2].st, 1, 1 );
		Vector4Set( verts[2].modulate, 255 * color[0], 255 * color[1], 255 * color[2], 255 );

		VectorCopy (org, verts[3].xyz);	
		verts[3].xyz[0] += cosR;	
		verts[3].xyz[1] -= sinR;	
		VectorArray2Set( verts[3].st, 1, 0 );
		Vector4Set( verts[3].modulate, 255 * color[0], 255 * color[1], 255 * color[2], 255 );
	}
	else if (p->type == P_FLAT)
	{

		VectorCopy (org, verts[0].xyz);	
		verts[0].xyz[0] -= p->height;	
		verts[0].xyz[1] -= p->width;	
		VectorArray2Set( verts[0].st, 0, 0 );
		Vector4Set( verts[0].modulate, 255, 255, 255, 255 );

		VectorCopy (org, verts[1].xyz);	
		verts[1].xyz[0] -= p->height;	
		verts[1].xyz[1] += p->width;	
		VectorArray2Set( verts[1].st, 0, 1 );
		Vector4Set( verts[1].modulate, 255, 255, 255, 255 );

		VectorCopy (org, verts[2].xyz);	
		verts[2].xyz[0] += p->height;	
		verts[2].xyz[1] += p->width;	
		VectorArray2Set( verts[2].st, 1, 1 );
		Vector4Set( verts[2].modulate, 255, 255, 255, 255 );

		VectorCopy (org, verts[3].xyz);	
		verts[3].xyz[0] += p->height;	
		verts[3].xyz[1] -= p->width;	
		VectorArray2Set( verts[3].st, 1, 0 );
		Vector4Set( verts[3].modulate, 255, 255, 255, 255 );
	}
#endif
	// Ridah
	else if (p->type == P_ANIM) {
		vec3_t	rr, ru;
		vec3_t	rotate_ang;
		int i, j;

		time = timenonscaled - p->time;
		time2 = p->endtime - p->time;
		ratio = time / time2;
		if (ratio >= 1.0f) {
			ratio = 0.9999f;
		}

		width = p->width + ( ratio * ( p->endwidth - p->width) );
		height = p->height + ( ratio * ( p->endheight - p->height) );

		// if we are "inside" this sprite, don't draw
		if (Distance( cg.snap->ps.origin, org ) < width/1.5) {
			return;
		}

		i = p->shaderAnim;
		j = (int)floor(ratio * shaderAnimCounts[p->shaderAnim]);
		p->pshader = shaderAnims[i][j];

		if (p->roll) {
			vectoangles( cg.refdef.viewaxis[0], rotate_ang );
			rotate_ang[ROLL] += p->roll;
			AngleVectors ( rotate_ang, NULL, rr, ru);
		}

		if (p->roll) {
			VectorMA (org, -height, ru, point);	
			VectorMA (point, -width, rr, point);	
		} else {
			VectorMA (org, -height, vup, point);	
			VectorMA (point, -width, vright, point);	
		}
		VectorCopy (point, verts[0].xyz);	
		VectorArray2Set( verts[0].st, 0, 0 );
		Vector4Set( verts[0].modulate, 255, 255, 255, 255 );

		if (p->roll) {
			VectorMA (point, 2*height, ru, point);	
		} else {
			VectorMA (point, 2*height, vup, point);	
		}
		VectorCopy (point, verts[1].xyz);	
		VectorArray2Set( verts[1].st, 0, 1 );
		Vector4Set( verts[1].modulate, 255, 255, 255, 255 );

		if (p->roll) {
			VectorMA (point, 2*width, rr, point);	
		} else {
			VectorMA (point, 2*width, vright, point);	
		}
		VectorCopy (point, verts[2].xyz);	
		VectorArray2Set( verts[2].st, 1, 1 );
		Vector4Set( verts[2].modulate, 255, 255, 255, 255 );

		if (p->roll) {
			VectorMA (point, -2*height, ru, point);	
		} else {
			VectorMA (point, -2*height, vup, point);	
		}
		VectorCopy (point, verts[3].xyz);	
		VectorArray2Set( verts[3].st, 1, 0 );
		Vector4Set( verts[3].modulate, 255, 255, 255, 255 );
	}
	// done.
	
	if (!p->pshader) {
// (SA) temp commented out for DM
//		CG_Printf ("CG_AddParticleToScene type %d p->pshader == ZERO\n", p->type);
		return;
	}

#if 0 /* // BFP - Unused particle type conditionals  */
	if (p->type == P_WEATHER || p->type == P_WEATHER_TURBULENT || p->type == P_WEATHER_FLURRY)
		trap_R_AddPolyToScene( p->pshader, 3, TRIverts );
	else
#endif
		trap_R_AddPolyToScene( p->pshader, 4, verts );

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
	int				color;
	cparticle_t		*active, *tail;
	int				type;
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
		if (p->alpha <= 0 && cg.frametime > 0.0f) // BFP - If paused, don't fade out
		{	// faded out
			p->next = free_particles;
			free_particles = p;
			p->type = p->color = p->alpha = 0;
			continue;
		}

		if (p->type == P_SMOKE || p->type == P_ANIM || p->type == P_BLEED || p->type == P_SMOKE_IMPACT
#if 0 /* // BFP - Unused particle type conditionals  */
		|| p->type == P_WEATHER_FLURRY || p->type == P_FLAT_SCALEUP_FADE
#endif
		|| p->type == P_BUBBLE || p->type == P_BUBBLE_TURBULENT // BFP - Add P_BUBBLE types to remove particles
		|| p->type == P_ANTIGRAV_ROCK // BFP - Add P_ANTIGRAV_ROCK to remove particles
		|| p->type == P_AURA // BFP - Add P_AURA to remove particles
		|| p->type == P_DEBRIS) // BFP - Add P_DEBRIS to remove particles
		{
			if (timenonscaled > p->endtime)
			{
				p->next = free_particles;
				free_particles = p;
				p->type = p->color = p->alpha = p->snum = 0;
				p->height = p->width = p->endheight = p->endwidth = 0;
				continue;
			}
		}

		if (p->type == P_SPRITE && p->endtime < 0) {
			// temporary sprite
			CG_AddParticleToScene (p, p->org, alpha);
			p->next = free_particles;
			free_particles = p;
			p->type = p->color = p->alpha = 0;
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

		color = p->color;

		time2 = time*time;

		org[0] = p->org[0] + p->vel[0]*time + p->accel[0]*time2;
		org[1] = p->org[1] + p->vel[1]*time + p->accel[1]*time2;
		org[2] = p->org[2] + p->vel[2]*time + p->accel[2]*time2;

		type = p->type;

		CG_AddParticleToScene (p, org, alpha);
	}

	active_particles = active;
}

void CG_ParticleBubble (centity_t *cent, qhandle_t pshader, vec3_t origin, vec3_t origin2, int turbtime, float range)
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

	p->color = 0;
	p->alpha = 1;
	p->alphavel = 0;
	// BFP - Apply to entity's origin
	p->start = cent->currentState.origin[2];
	p->end = cent->currentState.origin2[2];
	p->pshader = pshader;
	p->height = p->width = (rand() % 2) + 1;

	VectorCopy(origin, p->org);

	if (turbtime)
	{
		p->type = P_BUBBLE_TURBULENT;
		// BFP - Apply end time to remove particles in that case, if there's no end time the particles will remain there
		p->endtime = timenonscaled + turbtime;
		p->height = p->width = (rand() % 1) + 2;

		p->org[0] += (crandom() * range);
		p->org[1] += (crandom() * range);
		p->org[2] += (rand() % (int)20);

		VectorSet( p->vel, 
				(rand() % 521) - 250,
				(rand() % 521) - 250,
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
		p->type = P_BUBBLE;

		p->org[0] += (crandom() * range);
		p->org[1] += (crandom() * range);
		p->org[2] += (crandom() * 5);

		VectorSet( p->vel, 
				(rand() % 521) - 250,
				(rand() % 521) - 250,
				20 );

		// dispersion
		VectorSet( p->accel, 
				crandom() * 10, 
				crandom() * 10, 
				900 );
	}

	p->snum = 3 - (crandom() * 6); // used to randomize where the bubbles stop when these touches the surface
	p->link = qfalse; // used to handle the bubbles when touching the surface
}

// BFP - Handle bubble particles when reaching to the top
void CG_BubblesWaterHandling( cparticle_t *p, vec3_t org ) {
	trace_t		trace;
	vec3_t		start, end;
	int			contents;
	int			i;

	VectorCopy( org, end );
	end[2] -= 1;

	VectorCopy( org, start );
	start[2] += 10;

	// decelerate
	for (i = 0; i < 2; ++i) {
		p->vel[i] *= 0.99;

		// if the velocity is very small, clamp it to zero
		if (fabs(p->vel[i]) < 1) {
			p->vel[i] = 0;
		}
	}

	for (i = 0; i < 2; ++i) {
		p->accel[i] *= 0.99;

		// if the acceleration is very small, clamp it to zero
		if (fabs(p->accel[i]) < 1) {
			p->accel[i] = 0;
		}
	}

	// trace down to find the surface
	trap_CM_BoxTrace( &trace, start, end, vec3_origin, vec3_origin, 0, CONTENTS_WATER );

	// if the particle is touching something solid, it will skip instead stopping
	contents = trap_CM_PointContents( trace.endpos, 0 );
	if ( contents & CONTENTS_SOLID ) { // remove when grazing something solid
		p->next = NULL;
		p->type = p->color = p->alpha = 0;
		return;
	}
	if ( !( contents & CONTENTS_WATER ) ) {
		p->time = timenonscaled;
		VectorCopy (trace.endpos, p->org);
		p->org[2] = trace.endpos[2] - p->snum;

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
			if ( p->link && p->time > p->endtime - 2250 ) {
				p->vel[0] = 0;
				p->vel[1] = 0;
			}
		} else {
			if ( p->vel[0] != 0 ) p->vel[0] *= 0.97;
			if ( p->vel[1] != 0 ) p->vel[1] *= 0.97;
		}
	}
}

// BFP - Particle for dash smoke when using ki boost and moving in the ground
void CG_ParticleDashSmoke (centity_t *cent, qhandle_t pshader, vec3_t origin)
{
	cparticle_t	*p;

	// if (!pshader) CG_Printf ("CG_ParticleDashSmoke pshader == ZERO!\n");

	// Too much smoke...
	// That cent->trailTime can be handled to avoid spawning too much and only spawn when the game isn't paused, hehehe :P
	if ( cent->trailTime > cg.time ) {
		return;
	}
	cent->trailTime += 50;
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

	p->alpha = 0.45;
	p->alphavel = -0.1;

	p->pshader = pshader;
	p->start = cent->currentState.origin[2];
	p->end = cent->currentState.origin2[2];

	p->endtime = timenonscaled + 2000;
	p->startfade = timenonscaled + 100;

	p->height = p->width = 25;

	p->endheight = p->height * 2;
	p->endwidth = p->width * 2;

	p->type = P_SMOKE_IMPACT;

	VectorCopy( origin, p->org );
	VectorSet( p->vel, 
				(rand() % 401) - 200,
				(rand() % 401) - 200,
				20 );

	// dispersion
	VectorSet( p->accel, 
			crandom() * 10, 
			crandom() * 10, 
			1400 );

	p->link = qfalse; // to distinguish the type of smoke
}

// BFP - Antigrav rock particles for ki charging status
void CG_ParticleAntigravRock (qhandle_t pshader, centity_t *cent, int entityNum, vec3_t origin)
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

	p->endtime = timenonscaled + 450 + (crandom() * 20);

	p->color = 0;
	p->alpha = 1;
	p->alphavel = 0;
	p->pshader = pshader;
	p->height = p->width = (rand() % 2) + 2;
	p->type = P_ANTIGRAV_ROCK;

	VectorCopy( origin, p->org );

	p->org[0] += (crandom() * 50);
	p->org[1] += (crandom() * 50);

	p->start = cent->currentState.origin[2];
	// maybe BFP used to debug the client side visual of the other player with this top limit
	p->end = (cent->currentState.clientNum != cg.snap->ps.clientNum) 
		? p->org[2] + 130 + (crandom() * 10)
		: p->org[2] + 200 + (crandom() * 10);

	p->vel[0] = 0;
	p->vel[1] = 0;
	// maybe BFP used to debug the client side visual of the other player with this velocity
	p->vel[2] = (cent->currentState.clientNum != cg.snap->ps.clientNum) 
		? 200
		: 450;

	p->accel[0] = 0;
	p->accel[1] = 0;
	// maybe BFP used to debug the client side visual of the other player with this acceleration
	p->accel[2] = (cent->currentState.clientNum != cg.snap->ps.clientNum) 
		? 10
		: 20;

	p->roll = 5; // used as bounce counter
	p->link = qfalse; // to handle the ki charging status
	p->snum = 0; // to handle the client side visuals
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
		&& ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_CHARGE 
		&& !p->link ) { // BFP - Make each particle fall when they aren't on ki charging status
			p->endtime = timenonscaled + 1650;
			p->link = qtrue;
		}
	}
}

// BFP - Particle aura
void CG_ParticleAura (centity_t *cent, int entityNum, qhandle_t pshader, vec3_t origin, vec3_t origin2, float range)
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
	p->height = p->width = 40;

	VectorCopy(origin, p->org);

	p->type = P_AURA;

	p->org[0] += (crandom() * range);
	p->org[1] += (crandom() * range);
	p->org[2] += (crandom() * 5);

	VectorSet( p->vel, 
			(rand() % 621) - 250,
			(rand() % 621) - 250,
			100 );

	// dispersion
	VectorSet( p->accel, 
			crandom() * 20, 
			crandom() * 20, 
			1200 );

	p->link = qfalse;
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
		&& !p->link ) { // BFP - Make each particle fall when there's no aura at this moment
			p->alphavel = -0.03;
			p->accel[0] = 0;
			p->accel[1] = 0;
			p->endtime = timenonscaled + 600;
			p->link = qtrue;
		}
	}
}

// BFP - Debris particles for ki explosions and water splash
void CG_ParticleDebris (qhandle_t pshader, vec3_t origin, vec3_t vel, qboolean water)
{
	cparticle_t	*p;

	// if (!pshader) CG_Printf ("CG_ParticleDebris == ZERO!\n");

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;

	p->startfade = timenonscaled + 200;
	p->endtime = (water) 
		? timenonscaled + 2450 + (crandom() * 20)
		: timenonscaled + 1200;

	p->color = 0;
	p->alpha = 1;
	p->alphavel = 0;
	p->pshader = pshader;
	p->height = p->width = (water) 
		? (rand() % 3) + 1 
		: (rand() % 6) + 3;

	p->type = P_DEBRIS;

	VectorCopy( origin, p->org );

	if ( water ) {
		p->org[0] += (crandom() * 15);
		p->org[1] += (crandom() * 15);
	}

	p->start = origin[2];

	VectorCopy( vel, p->vel );

	if ( water ) {
		p->vel[0] = (crandom() * 150);
		p->vel[1] = (crandom() * 150);
		p->vel[2] = 1050;
	}

	p->accel[0] = (crandom() * 250);
	p->accel[1] = (crandom() * 250);
	p->accel[2] = 250 + (crandom() * 50);

	p->roll = (water) // used as bounce counter
		? 0
		: 3;
	p->link = water; // if it's water, it'll stop above water
	p->snum = 0;
}

void CG_ParticleSmoke (qhandle_t pshader, centity_t *cent)
{

	// using cent->density = enttime
	//		 cent->frame = startfade
	cparticle_t	*p;

	// if (!pshader) CG_Printf ("CG_ParticleSmoke == ZERO!\n");

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;
	
	p->endtime = timenonscaled + cent->currentState.time;
	p->startfade = timenonscaled + cent->currentState.time2;
	
	p->color = 0;
	p->alpha = 1.0;
	p->alphavel = 0;
	p->start = cent->currentState.origin[2];
	p->end = cent->currentState.origin2[2];
	p->pshader = pshader;
	p->rotate = qfalse;
	p->height = 8;
	p->width = 8;
	p->endheight = 32;
	p->endwidth = 32;
	p->type = P_SMOKE;
	
	VectorCopy(cent->currentState.origin, p->org);

	p->vel[0] = p->vel[1] = 0;
	p->accel[0] = p->accel[1] = p->accel[2] = 0;

	p->vel[2] = 5;

	if (cent->currentState.frame == 1)// reverse gravity	
		p->vel[2] *= -1;

	p->roll = 8 + (crandom() * 4);
}


void CG_ParticleBulletDebris (vec3_t org, vec3_t vel, int duration)
{

	cparticle_t	*p;

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;
	
	p->endtime = timenonscaled + duration;
	p->startfade = timenonscaled + duration/2;
	
	p->color = EMISIVEFADE;
	p->alpha = 1.0;
	p->alphavel = 0;

	p->height = 0.5;
	p->width = 0.5;
	p->endheight = 0.5;
	p->endwidth = 0.5;

	p->pshader = cgs.media.tracerShader;

	p->type = P_SMOKE;
	
	VectorCopy(org, p->org);

	p->vel[0] = vel[0];
	p->vel[1] = vel[1];
	p->vel[2] = vel[2];
	p->accel[0] = p->accel[1] = p->accel[2] = 0;

	p->accel[2] = -60;
	p->vel[2] += -20;
	
}

/*
======================
CG_ParticleExplosion
======================
*/
void CG_ParticleExplosion (char *animStr, vec3_t origin, vec3_t vel, int duration, int sizeStart, int sizeEnd)
{
	cparticle_t	*p;
	int anim;

	if (animStr < (char *)10)
		CG_Error( "CG_ParticleExplosion: animStr is probably an index rather than a string" );

	// find the animation string
	for (anim=0; shaderAnimNames[anim]; anim++) {
		if (!Q_stricmp( animStr, shaderAnimNames[anim] ))
			break;
	}
	if (!shaderAnimNames[anim]) {
		CG_Error("CG_ParticleExplosion: unknown animation string: %s\n", animStr);
		return;
	}

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;
	p->alpha = 1.0;
	p->alphavel = 0;

	if (duration < 0) {
		duration *= -1;
		p->roll = 0;
	} else {
		p->roll = crandom()*179;
	}

	p->shaderAnim = anim;

	p->width = sizeStart;
	p->height = sizeStart*shaderAnimSTRatio[anim];	// for sprites that are stretch in either direction

	p->endheight = sizeEnd;
	p->endwidth = sizeEnd*shaderAnimSTRatio[anim];

	p->endtime = timenonscaled + duration;

	p->type = P_ANIM;

	VectorCopy( origin, p->org );
	VectorCopy( vel, p->vel );
	VectorClear( p->accel );

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
	p->endtime = timenonscaled + 400;

	p->color = 0;
	p->alpha = 1;
	p->alphavel = 0;
	p->pshader = pshader;
	p->height = p->width = (rand() % 10) + 5;

	p->type = P_DEBRIS;

	VectorCopy( origin, p->org );

	p->start = origin[2];

	VectorCopy( vel, p->vel );

	p->accel[0] = (crandom() * 450);
	p->accel[1] = (crandom() * 450);
	p->accel[2] = 250 + (crandom() * 50);

	p->roll = 0; // no bounce
	p->link = qfalse;
	p->snum = 1; // treat it as non-solid
}

void CG_ParticleDust (centity_t *cent, vec3_t origin, vec3_t dir)
{
	float	length;
	float	dist;
	float	crittersize;
	vec3_t	angles, forward;
	vec3_t	point;
	cparticle_t	*p;
	int		i;
	
	dist = 0;

	VectorNegate (dir, dir);
	length = VectorLength (dir);
	vectoangles (dir, angles);
	AngleVectors (angles, forward, NULL, NULL);

	crittersize = LARGESIZE;

	if (length)
		dist = length / crittersize;

	if (dist < 1)
		dist = 1;

	VectorCopy (origin, point);

	for (i=0; i<dist; i++)
	{
		VectorMA (point, crittersize, forward, point);	
				
		if (!free_particles)
			return;

		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->time = timenonscaled;
		p->alpha = 5.0;
		p->alphavel = 0;
		p->roll = 0;

		p->pshader = cgs.media.smokePuffShader;

		// RF, stay around for long enough to expand and dissipate naturally
		if (length)
			p->endtime = timenonscaled + 4500 + (crandom() * 3500);
		else
			p->endtime = timenonscaled + 750 + (crandom() * 500);
		
		p->startfade = timenonscaled;
		
		p->width = LARGESIZE;
		p->height = LARGESIZE;

		// RF, expand while falling
		p->endheight = LARGESIZE*3.0;
		p->endwidth = LARGESIZE*3.0;

		if (!length)
		{
			p->width *= 0.2f;
			p->height *= 0.2f;

			p->endheight = NORMALSIZE;
			p->endwidth = NORMALSIZE;
		}

		p->type = P_SMOKE;

		VectorCopy( point, p->org );
		
		p->vel[0] = crandom()*6;
		p->vel[1] = crandom()*6;
		p->vel[2] = random()*20;

		// RF, add some gravity/randomness
		p->accel[0] = crandom()*3;
		p->accel[1] = crandom()*3;
		p->accel[2] = -PARTICLE_GRAVITY*0.4;

		VectorClear( p->accel );

		p->rotate = qfalse;

		p->roll = rand()%179;
		
		p->alpha = 0.75;
		
	}

	
}

void CG_ParticleMisc (qhandle_t pshader, vec3_t origin, int size, int duration, float alpha)
{
	cparticle_t	*p;

	// if (!pshader) CG_Printf ("CG_ParticleImpactSmokePuff pshader == ZERO!\n");

	if (!free_particles)
		return;

	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;
	p->alpha = 1.0;
	p->alphavel = 0;
	p->roll = rand()%179;

	p->pshader = pshader;

	if (duration > 0)
		p->endtime = timenonscaled + duration;
	else
		p->endtime = duration;

	p->startfade = timenonscaled;

	p->width = size;
	p->height = size;

	p->endheight = size;
	p->endwidth = size;

	p->type = P_SPRITE;

	VectorCopy( origin, p->org );

	p->rotate = qfalse;
}

// BFP - Unused function for particles, looks like here is to determine in the areas
// CG_SnowLink was never used in Q3
#if 0
int CG_NewParticleArea (int num)
{
	// const char *str;
	char *str, *token;
	int type;
	vec3_t origin, origin2;
	int		i;
	float range = 0;
	int turb, numparticles, snum;
	
	str = (char *) CG_ConfigString (num);
	if (!str[0])
		return (0);
	
	// returns type 128 64 or 32
	token = COM_Parse (&str);
	type = atoi (token);
	
	switch( type ) {
		case 4: range =   8; break;
		case 5: range =  16; break;
		case 3:
		case 6: range =  32; break;
		case 2: 
		case 7: range =  64; break;
		case 1: range = 128; break;
		case 0: range = 256; break;
	}

	for (i=0; i<3; i++)
	{
		token = COM_Parse (&str);
		origin[i] = atof (token);
	}

	for (i=0; i<3; i++)
	{
		token = COM_Parse (&str);
		origin2[i] = atof (token);
	}
		
	token = COM_Parse (&str);
	numparticles = atoi (token);
	
	token = COM_Parse (&str);
	turb = atoi (token);

	token = COM_Parse (&str);
	snum = atoi (token);
	
	for (i=0; i<numparticles; i++)
	{
		/*if (type >= 4)
			CG_ParticleBubble (cgs.media.waterBubbleShader, origin, origin2, turb, range, snum);
		else*/
			CG_ParticleSnow (cgs.media.waterBubbleShader, origin, origin2, turb, range, snum);
	}

	return (1);
}

void	CG_SnowLink (centity_t *cent, qboolean particleOn)
{
	cparticle_t		*p, *next;
	int id;

	id = cent->currentState.frame;

	for (p=active_particles ; p ; p=next)
	{
		next = p->next;
		
		if (p->type == P_WEATHER || p->type == P_WEATHER_TURBULENT)
		{
			if (p->snum == id)
			{
				if (particleOn)
					p->link = qtrue;
				else
					p->link = qfalse;
			}
		}

	}
}
#endif


// BFP - Unused particle stuff, saved for later :P
#if 0
void CG_ParticleSnowFlurry (qhandle_t pshader, centity_t *cent)
{
	cparticle_t	*p;
	qboolean turb = qtrue;

	// if (!pshader) CG_Printf ("CG_ParticleSnowFlurry pshader == ZERO!\n");

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;
	p->color = 0;
	p->alpha = 0.90f;
	p->alphavel = 0;

	p->start = cent->currentState.origin2[0];
	p->end = cent->currentState.origin2[1];
	
	p->endtime = timenonscaled + cent->currentState.time;
	p->startfade = timenonscaled + cent->currentState.time2;
	
	p->pshader = pshader;
	
	if (rand()%100 > 90)
	{
		p->height = 32;
		p->width = 32;
		p->alpha = 0.10f;
	}
	else
	{
		p->height = 1;
		p->width = 1;
	}

	p->vel[2] = -20;

	p->type = P_WEATHER_FLURRY;
	
	if (turb)
		p->vel[2] = -10;
	
	VectorCopy(cent->currentState.origin, p->org);

	p->org[0] = p->org[0];
	p->org[1] = p->org[1];
	p->org[2] = p->org[2];

	p->vel[0] = p->vel[1] = 0;
	
	p->accel[0] = p->accel[1] = p->accel[2] = 0;

	p->vel[0] += cent->currentState.angles[0] * 32 + (crandom() * 16);
	p->vel[1] += cent->currentState.angles[1] * 32 + (crandom() * 16);
	p->vel[2] += cent->currentState.angles[2];

	if (turb)
	{
		p->accel[0] = crandom () * 16;
		p->accel[1] = crandom () * 16;
	}

}

void CG_ParticleSnow (qhandle_t pshader, vec3_t origin, vec3_t origin2, int turb, float range, int snum)
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
	p->color = 0;
	p->alpha = 0.40f;
	p->alphavel = 0;
	p->start = origin[2];
	p->end = origin2[2];
	p->pshader = pshader;
	p->height = 1;
	p->width = 1;
	
	p->vel[2] = -50;

	if (turb)
	{
		p->type = P_WEATHER_TURBULENT;
		p->vel[2] = -50 * 1.3;
	}
	else
	{
		p->type = P_WEATHER;
	}
	
	VectorCopy(origin, p->org);

	p->org[0] = p->org[0] + ( crandom() * range);
	p->org[1] = p->org[1] + ( crandom() * range);
	p->org[2] = p->org[2] + ( crandom() * (p->start - p->end)); 

	p->vel[0] = p->vel[1] = 0;
	
	p->accel[0] = p->accel[1] = p->accel[2] = 0;

	if (turb)
	{
		p->vel[0] = crandom() * 16;
		p->vel[1] = crandom() * 16;
	}

	// Rafael snow pvs check
	p->snum = snum;
	p->link = qtrue;

}

void CG_ParticleImpactSmokePuff (qhandle_t pshader, vec3_t origin)
{
	cparticle_t	*p;

	// if (!pshader) CG_Printf ("CG_ParticleImpactSmokePuff pshader == ZERO!\n");

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;
	p->alpha = 0.25;
	p->alphavel = 0;
	p->roll = crandom()*179;

	p->pshader = pshader;

	p->endtime = timenonscaled + 1000;
	p->startfade = timenonscaled + 100;

	p->width = rand()%4 + 8;
	p->height = rand()%4 + 8;

	p->endheight = p->height *2;
	p->endwidth = p->width * 2;

	p->endtime = timenonscaled + 500;

	p->type = P_SMOKE_IMPACT;

	VectorCopy( origin, p->org );
	VectorSet(p->vel, 0, 0, 20);
	VectorSet(p->accel, 0, 0, 20);

	p->rotate = qtrue;
}

void CG_Particle_Bleed (qhandle_t pshader, vec3_t start, vec3_t dir, int fleshEntityNum, int duration)
{
	cparticle_t	*p;

	// if (!pshader) CG_Printf ("CG_Particle_Bleed pshader == ZERO!\n");

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;
	p->alpha = 1.0;
	p->alphavel = 0;
	p->roll = 0;

	p->pshader = pshader;

	p->endtime = timenonscaled + duration;
	
	if (fleshEntityNum)
		p->startfade = timenonscaled;
	else
		p->startfade = timenonscaled + 100;

	p->width = 4;
	p->height = 4;

	p->endheight = 4+rand()%3;
	p->endwidth = p->endheight;

	p->type = P_SMOKE;

	VectorCopy( start, p->org );
	p->vel[0] = 0;
	p->vel[1] = 0;
	p->vel[2] = -20;
	VectorClear( p->accel );

	p->rotate = qfalse;

	p->roll = rand()%179;
	
	p->color = BLOODRED;
	p->alpha = 0.75;

}

void CG_Particle_OilParticle (qhandle_t pshader, centity_t *cent)
{
	cparticle_t	*p;

	int			time;
	int			time2;
	float		ratio;

	float	duration = 1500;

	time = timenonscaled;
	time2 = timenonscaled + cent->currentState.time;

	ratio =(float)1 - ((float)time / (float)time2);

	// if (!pshader) CG_Printf ("CG_Particle_OilParticle == ZERO!\n");

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;
	p->alpha = 1.0;
	p->alphavel = 0;
	p->roll = 0;

	p->pshader = pshader;

	p->endtime = timenonscaled + duration;
	
	p->startfade = p->endtime;

	p->width = 1;
	p->height = 3;

	p->endheight = 3;
	p->endwidth = 1;

	p->type = P_SMOKE;

	VectorCopy(cent->currentState.origin, p->org );	
	
	p->vel[0] = (cent->currentState.origin2[0] * (16 * ratio));
	p->vel[1] = (cent->currentState.origin2[1] * (16 * ratio));
	p->vel[2] = (cent->currentState.origin2[2]);

	p->snum = 1.0f;

	VectorClear( p->accel );

	p->accel[2] = -20;

	p->rotate = qfalse;

	p->roll = rand()%179;
	
	p->alpha = 0.75;

}


void CG_Particle_OilSlick (qhandle_t pshader, centity_t *cent)
{
	cparticle_t	*p;
	
  	// if (!pshader) CG_Printf ("CG_Particle_OilSlick == ZERO!\n");

	if (!free_particles)
		return;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;
	
	if (cent->currentState.angles2[2])
		p->endtime = timenonscaled + cent->currentState.angles2[2];
	else
		p->endtime = timenonscaled + 60000;

	p->startfade = p->endtime;

	p->alpha = 1.0;
	p->alphavel = 0;
	p->roll = 0;

	p->pshader = pshader;

	if (cent->currentState.angles2[0] || cent->currentState.angles2[1])
	{
		p->width = cent->currentState.angles2[0];
		p->height = cent->currentState.angles2[0];

		p->endheight = cent->currentState.angles2[1];
		p->endwidth = cent->currentState.angles2[1];
	}
	else
	{
		p->width = 8;
		p->height = 8;

		p->endheight = 16;
		p->endwidth = 16;
	}

	p->type = P_FLAT_SCALEUP;

	p->snum = 1.0;

	VectorCopy(cent->currentState.origin, p->org );
	
	p->org[2]+= 0.55 + (crandom() * 0.5);

	p->vel[0] = 0;
	p->vel[1] = 0;
	p->vel[2] = 0;
	VectorClear( p->accel );

	p->rotate = qfalse;

	p->roll = rand()%179;
	
	p->alpha = 0.75;

}

void CG_OilSlickRemove (centity_t *cent)
{
	cparticle_t		*p, *next;
	int				id;

	id = 1.0f;

	if (!id)
		CG_Printf ("CG_OilSlickRevove NULL id\n");

	for (p=active_particles ; p ; p=next)
	{
		next = p->next;
		
		if (p->type == P_FLAT_SCALEUP)
		{
			if (p->snum == id)
			{
				p->endtime = timenonscaled + 100;
				p->startfade = p->endtime;
				p->type = P_FLAT_SCALEUP_FADE;

			}
		}

	}
}

qboolean ValidBloodPool (vec3_t start)
{
#define EXTRUDE_DIST	0.5

	vec3_t	angles;
	vec3_t	right, up;
	vec3_t	this_pos, x_pos, center_pos, end_pos;
	float	x, y;
	float	fwidth, fheight;
	trace_t	trace;
	vec3_t	normal;

	fwidth = 16;
	fheight = 16;

	VectorSet (normal, 0, 0, 1);

	vectoangles (normal, angles);
	AngleVectors (angles, NULL, right, up);

	VectorMA (start, EXTRUDE_DIST, normal, center_pos);

	for (x= -fwidth/2; x<fwidth; x+= fwidth)
	{
		VectorMA (center_pos, x, right, x_pos);

		for (y= -fheight/2; y<fheight; y+= fheight)
		{
			VectorMA (x_pos, y, up, this_pos);
			VectorMA (this_pos, -EXTRUDE_DIST*2, normal, end_pos);
			
			CG_Trace (&trace, this_pos, NULL, NULL, end_pos, -1, CONTENTS_SOLID);

			
			if (trace.entityNum < (MAX_ENTITIES - 1)) // may only land on world
				return qfalse;

			if (!(!trace.startsolid && trace.fraction < 1))
				return qfalse;
		
		}
	}

	return qtrue;
}

void CG_BloodPool (localEntity_t *le, qhandle_t pshader, trace_t *tr)
{	
	cparticle_t	*p;
	qboolean	legit;
	vec3_t		start;
	float		rndSize;
	
	// if (!pshader) CG_Printf ("CG_BloodPool pshader == ZERO!\n");

	if (!free_particles)
		return;
	
	VectorCopy (tr->endpos, start);
	legit = ValidBloodPool (start);

	if (!legit) 
		return;

	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;
	p->time = timenonscaled;
	
	p->endtime = timenonscaled + 3000;
	p->startfade = p->endtime;

	p->alpha = 1.0;
	p->alphavel = 0;
	p->roll = 0;

	p->pshader = pshader;

	rndSize = 0.4 + random()*0.6;

	p->width = 8*rndSize;
	p->height = 8*rndSize;

	p->endheight = 16*rndSize;
	p->endwidth = 16*rndSize;
	
	p->type = P_FLAT_SCALEUP;

	VectorCopy(start, p->org );
	
	p->vel[0] = 0;
	p->vel[1] = 0;
	p->vel[2] = 0;
	VectorClear( p->accel );

	p->rotate = qfalse;

	p->roll = rand()%179;
	
	p->alpha = 0.75;
	
	p->color = BLOODRED;
}

void CG_ParticleBloodCloud (centity_t *cent, vec3_t origin, vec3_t dir)
{
	float	length;
	float	dist;
	float	crittersize;
	vec3_t	angles, forward;
	vec3_t	point;
	cparticle_t	*p;
	int		i;
	
	dist = 0;

	length = VectorLength (dir);
	vectoangles (dir, angles);
	AngleVectors (angles, forward, NULL, NULL);

	crittersize = LARGESIZE;

	if (length)
		dist = length / crittersize;

	if (dist < 1)
		dist = 1;

	VectorCopy (origin, point);

	for (i=0; i<dist; i++)
	{
		VectorMA (point, crittersize, forward, point);	
		
		if (!free_particles)
			return;

		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;

		p->time = timenonscaled;
		p->alpha = 1.0;
		p->alphavel = 0;
		p->roll = 0;

		p->pshader = cgs.media.smokePuffShader;

		p->endtime = timenonscaled + 350 + (crandom() * 100);
		
		p->startfade = timenonscaled;
		
		p->width = LARGESIZE;
		p->height = LARGESIZE;
		p->endheight = LARGESIZE;
		p->endwidth = LARGESIZE;

		p->type = P_SMOKE;

		VectorCopy( origin, p->org );
		
		p->vel[0] = 0;
		p->vel[1] = 0;
		p->vel[2] = -1;
		
		VectorClear( p->accel );

		p->rotate = qfalse;

		p->roll = rand()%179;
		
		p->color = BLOODRED;
		
		p->alpha = 0.75;
		
	}

	
}
#endif
