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
/*
=======================================================================

CREDITS

=======================================================================
*/


#include "ui_local.h"

#define ART_BFPLOGO			"menu/art/bfp_logol"	// BFP - Logo
#define SCROLLSPEED			6.50	// The scrolling speed in pixels per second. Modify as appropriate for our credits
// uncomment this to use a background shader:
// #define BACKGROUND_SHADER 
#define ART_MENUBG			"menu/art/menubg"		// BFP - Menu background

typedef struct {
	menuframework_s	menu;
	menubitmap_s	header; // BFP - header
} creditsmenu_t;

static creditsmenu_t	s_credits;

int starttime; // game time at which credits are started
float mvolume; // records the original music volume level, as we will. Modify it for the credits

qhandle_t	BackgroundShader; // definition of the background shader pointer

typedef struct
{
	char *string;
	int style;
	vec4_t *colour;
} cr_line;

// BFP - Macros for credits. This way is easier to add in the array
#define CR_BIG_SEPARATOR    { "",     UI_CENTER | UI_BIGFONT,   & color_white },
#define CR_SMALL_SEPARATOR  { "",     UI_CENTER | UI_SMALLFONT, & color_white },

#define CR_CUT              CR_BIG_SEPARATOR CR_BIG_SEPARATOR
#define CR_START            CR_CUT CR_CUT
#define CR_LOGOEND          CR_SMALL_SEPARATOR CR_SMALL_SEPARATOR CR_SMALL_SEPARATOR CR_SMALL_SEPARATOR CR_SMALL_SEPARATOR \
                            CR_SMALL_SEPARATOR CR_SMALL_SEPARATOR CR_SMALL_SEPARATOR CR_SMALL_SEPARATOR CR_SMALL_SEPARATOR \
                            CR_SMALL_SEPARATOR CR_SMALL_SEPARATOR CR_SMALL_SEPARATOR

#define CR_TITLE( title1, title2 ) { title1, UI_CENTER | UI_GIANTFONT, & color_orange }, \
                                   { title2, UI_CENTER | UI_GIANTFONT, & color_orange },
#define CR_BFP_TEAM_TITLE( title ) { title,  UI_CENTER | UI_GIANTFONT, & color_blue },

#define CR_PERSON( name )          { name,   UI_CENTER | UI_SMALLFONT, & color_blue },
#define CR_ROLE( role )            { role,   UI_CENTER | UI_SMALLFONT, & color_white },

cr_line credits[] = { // edit this as necessary for the credits

	CR_START

	// BFP - Source Code Recreation credits
	CR_TITLE( "Bid For Power", "Source Code Recreation" )

	CR_SMALL_SEPARATOR

	CR_PERSON( "LegendGuard" )
	CR_ROLE( "Maintainer" )

	CR_SMALL_SEPARATOR

	CR_PERSON( "Mjuksel" )
	CR_ROLE( "Contributor" )

	CR_CUT

	// BFP - BFP Team credits credits
	CR_BFP_TEAM_TITLE( "The Bid For Power Team" )

	CR_SMALL_SEPARATOR

	CR_PERSON( "Ansel" )
	CR_ROLE( "Skin Artist / 2D Artist" )
	CR_ROLE( "Attack Sprites" )

	CR_SMALL_SEPARATOR

	CR_PERSON( "Chris James" )
	CR_ROLE( "Founder" )

	CR_SMALL_SEPARATOR

	CR_PERSON( "Dash" )
	CR_ROLE( "Level Design" )

	CR_SMALL_SEPARATOR

	CR_PERSON( "Gangsta Poodle" )
	CR_ROLE( "Level Design" )

	CR_SMALL_SEPARATOR

	CR_PERSON( "Kit Carson" )
	CR_ROLE( "Level Design" )

	CR_SMALL_SEPARATOR

	CR_PERSON( "NilreMK" )
	CR_ROLE( "Modeler / Animator / Skin Artist" )

	CR_SMALL_SEPARATOR

	CR_PERSON( "Number 17" )
	CR_ROLE( "Sound Engineer" )

	CR_SMALL_SEPARATOR

	CR_PERSON( "Pyrofragger" )
	CR_ROLE( "Modeler / Animator" )

	CR_SMALL_SEPARATOR

	CR_PERSON( "Rodney" )
	CR_ROLE( "Lead Artist" )
	CR_ROLE( "Modeler / Animator" )
	CR_ROLE( "Skin Artist / 2D Artist" )

	CR_SMALL_SEPARATOR

	CR_PERSON( "Yrgol" )
	CR_ROLE( "Game Design, Programmed By" )
	CR_ROLE( "Project Lead" )

	CR_SMALL_SEPARATOR

	// Over here, we can't call them as persons nor roles, but company, name and title :P
	CR_PERSON( "Id Software" )
	CR_ROLE( "Quake 3 engine" )

	CR_SMALL_SEPARATOR

	CR_PERSON( "Special Thanks" )
	CR_ROLE( "Disco Stu, Yngwie (menu music), Remisser, " )
	CR_ROLE( "Anthony, Dakota, Badhead, Gigatron, " )
	CR_ROLE( "Perfect Chaos, DethAyngel, Bardock, " )
	CR_ROLE( "$onik, Ebola, Mooky, Timex & Nat" )
	// BFP Team ends here

	// BFP - Separator for the logo
	CR_LOGOEND

	{ NULL }
};
#undef CR_BIG_SEPARATOR
#undef CR_SMALL_SEPARATOR
#undef CR_CUT
#undef CR_START
#undef CR_LOGOEND
#undef CR_TITLE
#undef CR_BFP_TEAM_TITLE
#undef CR_PERSON
#undef CR_ROLE


/*
=================
UI_CreditMenu_Key
=================
*/
static sfxHandle_t UI_CreditMenu_Key( int key ) {
	if( key & K_CHAR_FLAG ) {
		return 0;
	}

	trap_Cmd_ExecuteText( 
		EXEC_APPEND, 
		va( "s_musicvolume %f; quit\n", mvolume )
	);
	return 0;
}

/*
=================
Credits_DrawHeader

Draws header.
=================
*/
static void Credits_DrawHeader( void ) // BFP - Header
{
	// BFP - NOTE: It would be cool using a fading box or not. 
	// BFP vanilla uses a black box on it and pop ups when text alpha color is near to 0.90f
#define BLACK_BOX_FADE 0
	float fadetime;
	vec4_t fadeBluecolour = { 0.00f, 0.00f, 1.00f, 0.00f };
#if BLACK_BOX_FADE
	vec4_t fadeBlackcolour = { 0.00f, 0.00f, 0.00f, 0.00f };
#endif
	static float lastalpha = 0.00f;

	fadetime = (float)(uis.frametime) / 1000.00f;

	fadeBluecolour[3] = LERP( lastalpha, 1.00f, fadetime * 2.00f );
#if BLACK_BOX_FADE
	fadeBlackcolour[3] = LERP( lastalpha, 1.00f, fadetime * 2.00f );
	UI_FillRect( 0, 0, 640, 40, fadeBlackcolour );
#else
	// pop ups black box
	if ( lastalpha >= 0.90f ) {
		UI_FillRect( 0, 0, 640, 40, colorBlack );
	}
#endif

	UI_DrawProportionalString(320, 10, "CREDITS", // "The Bid For Power Team:" (original string)
							UI_CENTER|UI_BIGFONT, fadeBluecolour );
	lastalpha = fadeBluecolour[3];
}
#undef BLACK_BOX_FADE

/*
=================
ScrollingCredits_Draw

This is the main drawing function for the credits.
=================
*/
static void ScrollingCredits_Draw( void )
{
	int x = 320, y, n, ysize = 0, endcount;

	// ysize is used to determine the entire length 
	// of the credits in pixels. 
	// We can then use this in further calculations
	if ( !ysize ) // ysize not calculated, so calculate it dammit!
	{
		// loop through entire credits array
		for( n = 0; n <= sizeof(credits) - 1; n++ ) 
		{
			// it is a small character
			if ( credits[n].style & UI_SMALLFONT ) 
			{
				// add small character height
				ysize += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
				
				// it is a big character
			} else if ( credits[n].style & UI_BIGFONT ) {
				// add big character size
				ysize += PROP_HEIGHT;
				
				// it is a huge character
			} else if ( credits[n].style & UI_GIANTFONT ) {
				// add giant character size.
				ysize += PROP_HEIGHT * ( 1 / PROP_SMALL_SIZE_SCALE ); 
			}
		}
	}

	// first, fill the background with the specified colour/shader
	// we are drawing a shader
#ifdef BACKGROUND_SHADER 
	UI_DrawHandlePic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BackgroundShader);
#endif

	// let's draw the stuff
	// set initial y location
	y = 480 - SCROLLSPEED * (float)( uis.realtime - starttime ) / 100;

	// loop through the entire credits sequence
	for( n = 0; n <= sizeof(credits) - 1; n++ )
	{
		// this NULL string marks the end of the credits struct
		if ( credits[n].string == NULL ) 
		{
			endcount = y;
			if ( endcount <= 200 ) {
				endcount = 200;
			}
			UI_DrawNamedPic( 100, endcount, 465, 125, ART_BFPLOGO ); // BFP - Draw logo when it's the last credit and stop during 7 seconds
			if ( y < -(16*12) ) // credits sequence is completely off screen, note: 16 are 3 seconds if you multiply by 3, you add 1 second more to last (more or less)
			{
				trap_Cmd_ExecuteText( 
					EXEC_APPEND, 
					va( "s_musicvolume %f; quit\n", mvolume )
				);
				break; // end of credits
			}
			break;
		}
		
		if ( strlen( credits[n].string ) == 1 ) // spacer string, no need to draw
			continue;
		if ( y > -( PROP_HEIGHT * (1 / PROP_SMALL_SIZE_SCALE) ) ) {
			// the line is within the visible range of the screen
			UI_DrawProportionalString( x, y, credits[n].string, 
									credits[n].style, *credits[n].colour );
		}

		// re-adjust y for next line
		if ( credits[n].style & UI_SMALLFONT ) {
			y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
		} else if ( credits[n].style & UI_BIGFONT ) {
			y += PROP_HEIGHT;
		} else if ( credits[n].style & UI_GIANTFONT ) {
			y += PROP_HEIGHT * (1 / PROP_SMALL_SIZE_SCALE);
		}

		// if y is off the screen, break out of loop
		if ( y > 480 ) 
			break;
	}

	Credits_DrawHeader(); // BFP - Header
}

/*
===============
UI_CreditMenu
===============
*/
void UI_CreditMenu( void ) {
	memset( &s_credits, 0 ,sizeof(s_credits) );

	s_credits.menu.draw = ScrollingCredits_Draw;
	s_credits.menu.key = UI_CreditMenu_Key;
	s_credits.menu.fullscreen = qtrue;
	UI_PushMenu ( &s_credits.menu );

	starttime = uis.realtime; // record start time for credits to scroll properly
// BFP - There's no music in the credits
#if 0
	mvolume = trap_Cvar_VariableValue( "s_musicvolume" );
	if ( mvolume < 0.5 )
		trap_Cmd_ExecuteText( EXEC_APPEND, "s_musicvolume 0.5\n" );
	trap_Cmd_ExecuteText( EXEC_APPEND, "music music/fla22k_02\n" );
#endif

	// load the background shader
#ifdef BACKGROUND_SHADER
	BackgroundShader = 
		trap_R_RegisterShaderNoMip( ART_MENUBG );
#undef BACKGROUND_SHADER
#endif
}
