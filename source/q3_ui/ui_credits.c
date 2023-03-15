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

#define SCROLLSPEED	5.00 // The scrolling speed in pixels per second.
                          // modify as appropriate for our credits
// #define BACKGROUND_SHADER 
// uncomment this to use a background shader, otherwise a solid color
// defined in the vec4_t "color_background" is filled to the screen

typedef struct {
	menuframework_s	menu;
} creditsmenu_t;

static creditsmenu_t	s_credits;

int starttime; // game time at which credits are started
float mvolume; // records the original music volume level, as we will
               // modify it for the credits

// change this to change the background colour on credits
vec4_t color_background	        = {0.00f, 0.35f, 0.69f, 1.00f};
// these are just example colours that are used in credits[] 
vec4_t color_headertext			= {0.00f, 0.00f, 1.00f, 1.00f};
vec4_t color_maintext			= {0.00f, 0.00f, 1.00f, 1.00f};

qhandle_t	BackgroundShader; // definition of the background shader pointer

typedef struct
{
	char *string;
	int style;
	vec4_t *colour;
} cr_line;

cr_line credits[] = { // edit this as necessary for the credits
	// BFP - TODO: MAKE HEADER FORCED UPSIDE
	// HEADER
	{ "Credits", UI_CENTER|UI_BIGFONT|UI_BLINK, &color_red },
	
	// BODY
	{ "", UI_CENTER|UI_BIGFONT, &color_blue },
	{ "Bid For Power Team", UI_CENTER|UI_GIANTFONT, &color_blue },
	{ "", UI_CENTER|UI_BIGFONT, &color_blue },
	{ "Ansel", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Skin Artist / 2D Artist", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "Attack Sprites", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Chris James", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Founder", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Dash", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Level Design", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Gangsta Poodle", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Level Design", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Kit Carson", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Level Design", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "NilreMK", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Modeler / Animator / Skin Artist", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Number 17", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Sound Engineer", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Pyrofragger", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Modeler / Animator", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Rodney", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Lead Artist", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "Modeler / Animator", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "Skin Artist / 2D Artist", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Yrgol", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Game Design, Programmed By", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "Project Lead", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Special Thanks", UI_CENTER|UI_SMALLFONT, &color_blue },
	{ "Disco Stu, Yngwie (menu music), Remisser, ", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "Anthony, Dakota, Badhead, Gigatron, ", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "Perfect Chaos, DethAyngel, Bardock, ", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "$onik, Ebola, Mooky, Timex & Nat", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "", UI_CENTER|UI_SMALLFONT, &color_blue },
	// BFP Team ends here

	{ "Bid For Power", UI_CENTER|UI_GIANTFONT, &color_orange },
	{ "Source Code Recreation", UI_CENTER|UI_GIANTFONT, &color_orange },
	{ "", UI_CENTER|UI_BIGFONT, &color_white },
	{ "LegendGuard, Mjuksel", UI_CENTER|UI_SMALLFONT, &color_white },
	
	{ "", UI_CENTER|UI_BIGFONT, &color_blue },
	{ "", UI_CENTER|UI_BIGFONT, &color_blue },
	{ "", UI_CENTER|UI_BIGFONT, &color_blue },
	
	// Quake III Arena staff
	{ "Quake III Arena(c) 1999-2000, Id Software, Inc.", UI_CENTER|UI_SMALLFONT, &color_red },
	{ "", UI_CENTER|UI_BIGFONT, &color_blue },
	{ "John Carmack, Robert A. Duffy, Jim Dose', ", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "Adrian Carmack, Kevin Cloud, Kenneth Scott, ", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "Seneca Menard, Fred Nilsson, Tim Willits, ", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "Christian Antkow, Paul Jaquays, ", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "Todd Hollenshead, Marty Stratton, ", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "Donna Jackson, Eric Webb", UI_CENTER|UI_SMALLFONT, &color_white },
	{ "", UI_CENTER|UI_SMALLFONT, &color_blue },

	{ NULL }
};

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
        va("s_musicvolume %f; quit\n", mvolume)
	);
	return 0;
}

/*
=================
ScrollingCredits_Draw

This is the main drawing function for the credits.
=================
*/
static void ScrollingCredits_Draw( void )
{
	int x = 320, y, n, ysize = 0, fadetime = 0;
	vec4_t fadecolour = { 0.00f, 0.00f, 0.00f, 0.00f };

	// ysize is used to determine the entire length 
	// of the credits in pixels. 
	// We can then use this in further calculations
	if (!ysize) // ysize not calculated, so calculate it dammit!
	{
		// loop through entire credits array
		for(n = 0; n <= sizeof(credits) - 1; n++) 
		{
			// it is a small character
			if (credits[n].style & UI_SMALLFONT) 
			{
				// add small character height
				ysize += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
				
				// it is a big character
			} else if (credits[n].style & UI_BIGFONT) {
				// add big character size
				ysize += PROP_HEIGHT;
				
				// it is a huge character
			} else if (credits[n].style & UI_GIANTFONT) {
				// add giant character size.
				ysize += PROP_HEIGHT * (1 / PROP_SMALL_SIZE_SCALE); 
			}
		}
	}

	// first, fill the background with the specified colour/shader
	// we are drawing a shader
#ifdef BACKGROUND_SHADER 
	UI_DrawHandlePic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BackgroundShader);

	// we are just filling a color
#else 
	UI_FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color_background);
#endif

	// let's draw the stuff
	// set initial y location
	y = 480 - SCROLLSPEED * (float)(uis.realtime - starttime) / 100;

	// loop through the entire credits sequence
	for( n = 0; n <= sizeof(credits) - 1; n++ )
	{
		// this NULL string marks the end of the credits struct
		if (credits[n].string == NULL) 
		{
			if (y < -16) // credits sequence is completely off screen
			{
				trap_Cmd_ExecuteText( 
					EXEC_APPEND, 
					va("s_musicvolume %f; quit\n", mvolume)
				);
				break; // end of credits
			}
			break;
		}
		
		if ( strlen(credits[n].string) == 1) // spacer string, no need to draw
			continue;
		if ( y > -(PROP_HEIGHT * (1 / PROP_SMALL_SIZE_SCALE))) 
			// the line is within the visible range of the screen
			UI_DrawProportionalString(x, y, credits[n].string, 
									credits[n].style, *credits[n].colour );

		// re-adjust y for next line
		if (credits[n].style & UI_SMALLFONT) {
			y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
		} else if (credits[n].style & UI_BIGFONT) {
			y += PROP_HEIGHT;
		} else if (credits[n].style & UI_GIANTFONT) {
			y += PROP_HEIGHT * (1 / PROP_SMALL_SIZE_SCALE);
		}

		// if y is off the screen, break out of loop
		if (y > 480) 
		break;
	}
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
	mvolume = trap_Cvar_VariableValue( "s_musicvolume" );
	if (mvolume < 0.5)
		trap_Cmd_ExecuteText( EXEC_APPEND, "s_musicvolume 0.5\n" );
	trap_Cmd_ExecuteText( EXEC_APPEND, "music music/fla22k_02\n" );

	// load the background shader
#ifdef BACKGROUND_SHADER
	BackgroundShader = 
		trap_R_RegisterShaderNoMip( "menu/art/menubg" );
#endif
}
