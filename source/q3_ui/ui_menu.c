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

MAIN MENU

=======================================================================
*/


#include "ui_local.h"

#define ART_MENUBG			"menu/art/menubg"		// BFP - Menu background
#define ART_BFPLOGO			"menu/art/bfp_logol"	// BFP - Logo
#define ART_CAPBAR			"menu/art/cap_bar"		// BFP - cap bar
#define ART_CRBANNER		"menu/art/cr"			// BFP - copyright banner

// #define ID_SINGLEPLAYER			10 // BFP - Disabled due to full BFP vanilla implementation
#define ID_MULTIPLAYER			10
#define ID_SETUP				11
#define ID_DEMOS				12
// BFP - NOTE: Some stuff could be done after the full BFP vanilla implementation
#if 0
#define ID_CINEMATICS			14 // BFP - Maybe some wonderful .ROQ videos? Not sure...
#define ID_MODS					15 // BFP - Maybe not necessary
#endif
#define ID_EXIT					13

// BFP - That's the main banner 3d model of Quake 3 words, it would be a good idea to add a proper 3d banner model :P
#if 0
#define MAIN_BANNER_MODEL				"models/mapobjects/banner/banner5.md3"
#define MAIN_MENU_VERTICAL_SPACING		34
#endif
#define MAIN_MENU_VERTICAL_SPACING		65 // BFP - Set vertical spacing to 65

typedef struct {
	menuframework_s	menu;

	menubitmap_s		menubg;		// BFP - Menu background
	menubitmap_s		bfplogo;	// BFP - Logo
	menubitmap_s		crbanner;	// BFP - copyright banner

	menubitmap_s		capbar0;	// BFP - PLAY cap bar
	menubitmap_s		capbar1;	// BFP - SETUP cap bar
	menubitmap_s		capbar2;	// BFP - DEMOS cap bar
	menubitmap_s		capbar3;	// BFP - EXIT cap bar

	// BFP - Just wondering if anyone will come up with Single Player/Campaign/Story mode in their mind
	// menutext_s		singleplayer;
	menutext_s		multiplayer;
	menutext_s		setup;
	menutext_s		demos;

// BFP - NOTE: Same note as said before
#if 0
	menutext_s		cinematics;
	menutext_s		mods;
#endif
	menutext_s		exit;

	// BFP - As said before, idea for a proper 3d banner model
	// qhandle_t		bannerModel;
} mainmenu_t;


static mainmenu_t s_main;

typedef struct {
	menuframework_s menu;	
	char errorMessage[4096];
} errorMessage_t;

static errorMessage_t s_errorMessage;

/*
=================
MainMenu_ExitAction
=================
*/
static void MainMenu_ExitAction( qboolean result ) {
	if( !result ) {
		return;
	}
	UI_PopMenu();
	UI_CreditMenu();
}



/*
=================
Main_MenuEvent
=================
*/
void Main_MenuEvent (void* ptr, int event) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	// BFP - As said before, if anyone has an idea in their mind about Single Player stuff
#if 0
	case ID_SINGLEPLAYER:
		UI_SPLevelMenu();
		break;
#endif

	case ID_MULTIPLAYER:
		UI_ArenaServersMenu();
		break;

	case ID_SETUP:
		UI_SetupMenu();
		break;

	case ID_DEMOS:
		UI_DemosMenu();
		break;

// BFP - NOTE: Same note as said before
#if 0
	case ID_CINEMATICS:
		UI_CinematicsMenu();
		break;

	case ID_MODS:
		UI_ModsMenu();
		break;
#endif

	case ID_EXIT:
		UI_ConfirmMenu( "EXIT GAME?", NULL, MainMenu_ExitAction );
		break;
	}
}


/*
===============
MainMenu_Cache
===============
*/
void MainMenu_Cache( void ) {
	// BFP - Here's where sets the 3d banner model :P
	// s_main.bannerModel = trap_R_RegisterModel( MAIN_BANNER_MODEL );
	trap_R_RegisterShaderNoMip( ART_MENUBG );	// BFP - Menu background
	trap_R_RegisterShaderNoMip( ART_BFPLOGO );	// BFP - Logo
	trap_R_RegisterShaderNoMip( ART_CRBANNER );	// BFP - copyright banner
}

sfxHandle_t ErrorMessage_Key(int key)
{
	trap_Cvar_Set( "com_errorMessage", "" );
	UI_MainMenu();
	return (menu_null_sound);
}

/*
===============
Main_MenuDraw
TTimo: this function is common to the main menu and errorMessage menu
===============
*/

static void Main_MenuDraw( void ) {
	refdef_t		refdef;

	// BFP - 3d banner model disabled :P
#if 0
	refEntity_t		ent;
	vec3_t			origin;
	vec3_t			angles;
	vec4_t			color = {0.5, 0, 0, 1};
#endif

	float			adjust;
	float			x, y, w, h;

	// setup the refdef

	memset( &refdef, 0, sizeof( refdef ) );

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	x = 0;
	y = 0;
	w = 640;
	h = 120;
	UI_AdjustFrom640( &x, &y, &w, &h );
	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	adjust = 0; // JDC: Kenneth asked me to stop this 1.0 * sin( (float)uis.realtime / 1000 );
	refdef.fov_x = 60 + adjust;
	refdef.fov_y = 19.6875 + adjust;

	refdef.time = uis.realtime;

	// BFP - 3d banner model disabled :P
#if 0
	origin[0] = 300;
	origin[1] = 0;
	origin[2] = -32;

	trap_R_ClearScene();

	// add the model

	memset( &ent, 0, sizeof(ent) );

	adjust = 5.0 * sin( (float)uis.realtime / 5000 );
	VectorSet( angles, 0, 180 + adjust, 0 );
	AnglesToAxis( angles, ent.axis );
	ent.hModel = s_main.bannerModel;
	VectorCopy( origin, ent.origin );
	VectorCopy( origin, ent.lightingOrigin );
	ent.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( ent.origin, ent.oldorigin );

	trap_R_AddRefEntityToScene( &ent );

	trap_R_RenderScene( &refdef );
#endif

	if (strlen(s_errorMessage.errorMessage))
	{
		UI_DrawProportionalString_AutoWrapped( 320, 192, 600, 20, s_errorMessage.errorMessage, UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, menu_text_color );
	}
	else
	{
		// standard menu drawing
		Menu_Draw( &s_main.menu );		
	}

	// BFP - Demo version disabled, possibly we may remove that :P
#if 0
	if (uis.demoversion) {
		UI_DrawProportionalString( 320, 372, "DEMO      FOR MATURE AUDIENCES      DEMO", UI_CENTER|UI_SMALLFONT, color );
		UI_DrawString( 320, 400, "Quake III Arena(c) 1999-2000, Id Software, Inc.  All Rights Reserved", UI_CENTER|UI_SMALLFONT, color );
	} else {
		UI_DrawString( 320, 450, "Quake III Arena(c) 1999-2000, Id Software, Inc.  All Rights Reserved", UI_CENTER|UI_SMALLFONT, color );
	}
#endif
}


/*
===============
UI_MainMenu

The main menu only comes up when not in a game,
so make sure that the attract loop server is down
and that local cinematics are killed
===============
*/
void UI_MainMenu( void ) {
	int		y;
	int		style = UI_CENTER | UI_DROPSHADOW;

	trap_Cvar_Set( "sv_killserver", "1" );

	// BFP - NOTE: Remove CD key verification? BFP vanilla still waits you to verify the CD key
	if( !uis.demoversion && !ui_cdkeychecked.integer ) {
		char	key[17];

		trap_GetCDKey( key, sizeof(key) );
		if( trap_VerifyCDKey( key, NULL ) == qfalse ) {
			UI_CDKeyMenu();
			return;
		}
	}
	
	memset( &s_main, 0 ,sizeof(mainmenu_t) );
	memset( &s_errorMessage, 0 ,sizeof(errorMessage_t) );

	// com_errorMessage would need that too
	MainMenu_Cache();
	
	trap_Cvar_VariableStringBuffer( "com_errorMessage", s_errorMessage.errorMessage, sizeof(s_errorMessage.errorMessage) );
	if (strlen(s_errorMessage.errorMessage))
	{	
		s_errorMessage.menu.draw = Main_MenuDraw;
		s_errorMessage.menu.key = ErrorMessage_Key;
		s_errorMessage.menu.fullscreen = qtrue;
		s_errorMessage.menu.wrapAround = qtrue;
		s_errorMessage.menu.showlogo = qfalse;	// BFP - Don't show logo

		trap_Key_SetCatcher( KEYCATCH_UI );
		uis.menusp = 0;
		UI_PushMenu ( &s_errorMessage.menu );
		
		return;
	}

	s_main.menu.draw = Main_MenuDraw;
	s_main.menu.fullscreen = qtrue;
	s_main.menu.wrapAround = qtrue;
	s_main.menu.showlogo = qfalse;	// BFP - Don't show logo

	// BFP - Menu background
	s_main.menubg.generic.type			= MTYPE_BITMAP;
	s_main.menubg.generic.name			= ART_MENUBG;
	s_main.menubg.generic.flags			= QMF_LEFT_JUSTIFY | QMF_INACTIVE;
	s_main.menubg.generic.x				= 0;
	s_main.menubg.generic.y				= 0;
	s_main.menubg.width					= 640;
	s_main.menubg.height				= 480;

	// BFP - Logo
	s_main.bfplogo.generic.type			= MTYPE_BITMAP;
	s_main.bfplogo.generic.name			= ART_BFPLOGO;
	s_main.bfplogo.generic.flags		= QMF_LEFT_JUSTIFY | QMF_INACTIVE;
	s_main.bfplogo.generic.x			= 90;
	s_main.bfplogo.generic.y			= 10;
	s_main.bfplogo.width				= 460;
	s_main.bfplogo.height				= 115;

	// BFP - PLAY cap bar
	y = 155; // BFP - Cap bar initial vertical position
	s_main.capbar0.generic.type			= MTYPE_BITMAP;
	s_main.capbar0.generic.name			= ART_CAPBAR;
	s_main.capbar0.generic.flags		= QMF_LEFT_JUSTIFY | QMF_INACTIVE;
	s_main.capbar0.generic.x			= 160;
	s_main.capbar0.generic.y			= y;
	s_main.capbar0.width				= 340;
	s_main.capbar0.height				= 78;

	// BFP - SETUP cap bar
	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.capbar1.generic.type			= MTYPE_BITMAP;
	s_main.capbar1.generic.name			= ART_CAPBAR;
	s_main.capbar1.generic.flags		= QMF_LEFT_JUSTIFY | QMF_INACTIVE;
	s_main.capbar1.generic.x			= 160;
	s_main.capbar1.generic.y			= y;
	s_main.capbar1.width				= 340;
	s_main.capbar1.height				= 78;

	// BFP - DEMOS cap bar
	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.capbar2.generic.type			= MTYPE_BITMAP;
	s_main.capbar2.generic.name			= ART_CAPBAR;
	s_main.capbar2.generic.flags		= QMF_LEFT_JUSTIFY | QMF_INACTIVE;
	s_main.capbar2.generic.x			= 160;
	s_main.capbar2.generic.y			= y;
	s_main.capbar2.width				= 340;
	s_main.capbar2.height				= 78;

	// BFP - EXIT cap bar
	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.capbar3.generic.type			= MTYPE_BITMAP;
	s_main.capbar3.generic.name			= ART_CAPBAR;
	s_main.capbar3.generic.flags		= QMF_LEFT_JUSTIFY | QMF_INACTIVE;
	s_main.capbar3.generic.x			= 160;
	s_main.capbar3.generic.y			= y;
	s_main.capbar3.width				= 340;
	s_main.capbar3.height				= 78;

	// BFP - copyright banner
	s_main.crbanner.generic.type			= MTYPE_BITMAP;
	s_main.crbanner.generic.name			= ART_CRBANNER;
	s_main.crbanner.generic.flags			= QMF_LEFT_JUSTIFY | QMF_INACTIVE;
	s_main.crbanner.generic.x				= 0;
	s_main.crbanner.generic.y				= 460;
	s_main.crbanner.width					= 640;
	s_main.crbanner.height					= 20;

	// BFP - As said before, if anyone has an idea in their mind about Single Player stuff
#if 0
	// y = 134; // BFP - That's the initial vertical position of Q3 buttons
	s_main.singleplayer.generic.type		= MTYPE_PTEXT;
	s_main.singleplayer.generic.flags		= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.singleplayer.generic.x			= 350;
	s_main.singleplayer.generic.y			= y;
	s_main.singleplayer.generic.id			= ID_SINGLEPLAYER;
	s_main.singleplayer.generic.callback	= Main_MenuEvent; 
	s_main.singleplayer.string				= "SINGLE PLAYER"; // BFP - Or CAMPAIGN :P
	s_main.singleplayer.color				= color_white; // BFP - modified SINGLE PLAYER color 
	s_main.singleplayer.style				= style;
#endif

	y = 170; // BFP - Text initial vertical position
	s_main.multiplayer.generic.type			= MTYPE_PTEXT;
	s_main.multiplayer.generic.flags		= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.multiplayer.generic.x			= 350;
	s_main.multiplayer.generic.y			= y;
	s_main.multiplayer.generic.id			= ID_MULTIPLAYER;
	s_main.multiplayer.generic.callback		= Main_MenuEvent; 
	s_main.multiplayer.string				= "PLAY"; // BFP - before MULTIPLAYER
	s_main.multiplayer.color				= color_white; // BFP - modified PLAY(MULTIPLAYER) color
	s_main.multiplayer.style				= style;

	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.setup.generic.type				= MTYPE_PTEXT;
	s_main.setup.generic.flags				= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.setup.generic.x					= 350;
	s_main.setup.generic.y					= y;
	s_main.setup.generic.id					= ID_SETUP;
	s_main.setup.generic.callback			= Main_MenuEvent; 
	s_main.setup.string						= "SETUP";
	s_main.setup.color						= color_white;
	s_main.setup.style						= style;

	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.demos.generic.type				= MTYPE_PTEXT;
	s_main.demos.generic.flags				= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.demos.generic.x					= 350;
	s_main.demos.generic.y					= y;
	s_main.demos.generic.id					= ID_DEMOS;
	s_main.demos.generic.callback			= Main_MenuEvent; 
	s_main.demos.string						= "DEMOS";
	s_main.demos.color						= color_white; // BFP - modified DEMOS color
	s_main.demos.style						= style;

// BFP - NOTE: Same note as said before
#if 0
	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.cinematics.generic.type			= MTYPE_PTEXT;
	s_main.cinematics.generic.flags			= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.cinematics.generic.x				= 350;
	s_main.cinematics.generic.y				= y;
	s_main.cinematics.generic.id			= ID_CINEMATICS;
	s_main.cinematics.generic.callback		= Main_MenuEvent; 
	s_main.cinematics.string				= "CINEMATICS";
	s_main.cinematics.color					= color_white; // BFP - modified CINEMATICS color
	s_main.cinematics.style					= style;

	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.mods.generic.type			= MTYPE_PTEXT;
	s_main.mods.generic.flags			= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.mods.generic.x				= 350;
	s_main.mods.generic.y				= y;
	s_main.mods.generic.id				= ID_MODS;
	s_main.mods.generic.callback		= Main_MenuEvent; 
	s_main.mods.string					= "MODS";
	s_main.mods.color					= color_white; // BFP - modified MODS color
	s_main.mods.style					= style;
#endif

	y += MAIN_MENU_VERTICAL_SPACING;
	s_main.exit.generic.type				= MTYPE_PTEXT;
	s_main.exit.generic.flags				= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_main.exit.generic.x					= 350;
	s_main.exit.generic.y					= y;
	s_main.exit.generic.id					= ID_EXIT;
	s_main.exit.generic.callback			= Main_MenuEvent; 
	s_main.exit.string						= "EXIT";
	s_main.exit.color						= color_white;
	s_main.exit.style						= style;

	Menu_AddItem( &s_main.menu, &s_main.menubg ); // BFP - Menu background
	Menu_AddItem( &s_main.menu, &s_main.bfplogo ); // BFP - Logo

	Menu_AddItem( &s_main.menu, &s_main.crbanner ); // BFP - copyright banner

	// Menu_AddItem( &s_main.menu,	&s_main.singleplayer ); // BFP - As said before, if anyone has an idea in their mind about Single Player stuff
	Menu_AddItem( &s_main.menu, &s_main.capbar0 ); // BFP - cap bar for PLAY(MULTIPLAYER) button
	Menu_AddItem( &s_main.menu,	&s_main.multiplayer );
	// BFP - The order of cap bars is made of this way
	Menu_AddItem( &s_main.menu, &s_main.capbar3 ); // BFP - cap bar for EXIT button
	Menu_AddItem( &s_main.menu,	&s_main.exit );    
	Menu_AddItem( &s_main.menu, &s_main.capbar2 ); // BFP - cap bar for DEMOS button
	Menu_AddItem( &s_main.menu,	&s_main.demos );
	Menu_AddItem( &s_main.menu, &s_main.capbar1 ); // BFP - cap bar for SETUP button
	Menu_AddItem( &s_main.menu,	&s_main.setup );

	// BFP - NOTE: Same note as said before
#if 0	
	Menu_AddItem( &s_main.menu,	&s_main.cinematics );
	Menu_AddItem( &s_main.menu,	&s_main.mods );
#endif

	trap_Key_SetCatcher( KEYCATCH_UI );
	uis.menusp = 0;
	UI_PushMenu ( &s_main.menu );
		
}
