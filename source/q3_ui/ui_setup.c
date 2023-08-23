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

SETUP MENU

=======================================================================
*/


#include "ui_local.h"


#define SETUP_MENU_VERTICAL_SPACING		34

#define ART_BACK0		"menu/art/back_0"
#define ART_BACK1		"menu/art/back_1"	
#define ART_MENUBG		"menu/art/menubg"		// BFP - Menu background
#define ART_BARLOG		"menu/art/cap_barlog"	// BFP - barlog

#define ID_CUSTOMIZEPLAYER		10
#define ID_BFPOPTIONS			11 // BFP - BFP Options menu
#define ID_CUSTOMIZECONTROLS	12
#define ID_SYSTEMCONFIG			13
#define ID_GAME					14
#define ID_CDKEY				15
#define ID_DEFAULTS				16
#define ID_BACK					17


typedef struct {
	menuframework_s	menu;

	menubitmap_s	menubg; // BFP - Menu background
	menubitmap_s	barlog; // BFP - barlog
	menutext_s		banner;

	menutext_s		setupplayer;
	menutext_s		bfpoptions;
	menutext_s		setupcontrols;
	menutext_s		setupsystem;
	menutext_s		game;
	menutext_s		cdkey;
//	menutext_s		load;
//	menutext_s		save;
	menutext_s		defaults;
	menubitmap_s	back;
} setupMenuInfo_t;

static setupMenuInfo_t	setupMenuInfo;


/*
=================
Setup_ResetDefaults_Action
=================
*/
static void Setup_ResetDefaults_Action( qboolean result ) {
	if( !result ) {
		return;
	}
	trap_Cmd_ExecuteText( EXEC_APPEND, "exec default.cfg\n");
	trap_Cmd_ExecuteText( EXEC_APPEND, "cvar_restart\n");
	trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
}


/*
=================
Setup_ResetDefaults_Draw
=================
*/
static void Setup_ResetDefaults_Draw( void ) {
	UI_DrawProportionalString( SCREEN_WIDTH/2, 356 + PROP_HEIGHT * 0, "WARNING: This will reset *ALL*", UI_CENTER|UI_SMALLFONT, color_yellow );
	UI_DrawProportionalString( SCREEN_WIDTH/2, 356 + PROP_HEIGHT * 1, "options to their default values.", UI_CENTER|UI_SMALLFONT, color_yellow );
}


/*
===============
UI_SetupMenu_Event
===============
*/
static void UI_SetupMenu_Event( void *ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_CUSTOMIZEPLAYER:
		UI_PlayerModelMenu(); // BFP - Redirect to the character selection menu, before UI_PlayerSettingsMenu();
		break;

	case ID_BFPOPTIONS: // BFP - BFP Options menu
		UI_BFPPreferencesMenu();
		break;

	case ID_CUSTOMIZECONTROLS:
		UI_ControlsMenu();
		break;

	case ID_SYSTEMCONFIG:
		UI_GraphicsOptionsMenu();
		break;

	case ID_GAME:
		UI_PreferencesMenu();
		break;

	case ID_CDKEY:
		UI_CDKeyMenu();
		break;

//	case ID_LOAD:
//		UI_LoadConfigMenu();
//		break;

//	case ID_SAVE:
//		UI_SaveConfigMenu();
//		break;

	case ID_DEFAULTS:
		UI_ConfirmMenu( "SET TO DEFAULTS?", Setup_ResetDefaults_Draw, Setup_ResetDefaults_Action );
		break;

	case ID_BACK:
		UI_PopMenu();
		break;
	}
}


/*
===============
UI_SetupMenu_Init
===============
*/
static void UI_SetupMenu_Init( void ) {
	int				y;

	UI_SetupMenu_Cache();

	memset( &setupMenuInfo, 0, sizeof(setupMenuInfo) );
	setupMenuInfo.menu.wrapAround = qtrue;
	setupMenuInfo.menu.fullscreen = qtrue;

	// BFP - Menu background
	setupMenuInfo.menubg.generic.type = MTYPE_BITMAP;
	setupMenuInfo.menubg.generic.name = ART_MENUBG;
	setupMenuInfo.menubg.generic.flags = QMF_LEFT_JUSTIFY | QMF_INACTIVE;
	setupMenuInfo.menubg.generic.x = 0;
	setupMenuInfo.menubg.generic.y = 0;
	setupMenuInfo.menubg.width = 640;
	setupMenuInfo.menubg.height = 480;

	// BFP - barlog
	setupMenuInfo.barlog.generic.type				= MTYPE_BITMAP;
	setupMenuInfo.barlog.generic.name				= ART_BARLOG;
	setupMenuInfo.barlog.generic.flags				= QMF_LEFT_JUSTIFY | QMF_INACTIVE;
	setupMenuInfo.barlog.generic.x					= 140;
	setupMenuInfo.barlog.generic.y					= 5;
	setupMenuInfo.barlog.width						= 355;
	setupMenuInfo.barlog.height						= 90;

	// BFP - modified banner
	setupMenuInfo.banner.generic.type				= MTYPE_PTEXT;
	setupMenuInfo.banner.generic.flags				= QMF_CENTER_JUSTIFY | QMF_INACTIVE;
	setupMenuInfo.banner.generic.x					= 320;
	setupMenuInfo.banner.generic.y					= 45;
	setupMenuInfo.banner.string						= "SETUP";
	setupMenuInfo.banner.color						= color_white;
	setupMenuInfo.banner.style						= UI_CENTER | UI_BIGFONT;

	y = 134;
	setupMenuInfo.setupplayer.generic.type			= MTYPE_PTEXT;
	setupMenuInfo.setupplayer.generic.flags			= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	setupMenuInfo.setupplayer.generic.x				= 320;
	setupMenuInfo.setupplayer.generic.y				= y;
	setupMenuInfo.setupplayer.generic.id			= ID_CUSTOMIZEPLAYER;
	setupMenuInfo.setupplayer.generic.callback		= UI_SetupMenu_Event; 
	setupMenuInfo.setupplayer.string				= "PLAYER";
	setupMenuInfo.setupplayer.color					= color_white; // BFP - modified PLAYER color
	setupMenuInfo.setupplayer.style					= UI_CENTER;

	// BFP - BFP Options menu
	y += SETUP_MENU_VERTICAL_SPACING;
	setupMenuInfo.bfpoptions.generic.type			= MTYPE_PTEXT;
	setupMenuInfo.bfpoptions.generic.flags			= QMF_CENTER_JUSTIFY | QMF_PULSEIFFOCUS;
	setupMenuInfo.bfpoptions.generic.x				= 320;
	setupMenuInfo.bfpoptions.generic.y				= y;
	setupMenuInfo.bfpoptions.generic.id				= ID_BFPOPTIONS;
	setupMenuInfo.bfpoptions.generic.callback		= UI_SetupMenu_Event;
	setupMenuInfo.bfpoptions.string					= "BFP OPTIONS";
	setupMenuInfo.bfpoptions.color					= color_white;
	setupMenuInfo.bfpoptions.style					= UI_CENTER;

	y += SETUP_MENU_VERTICAL_SPACING;
	setupMenuInfo.setupcontrols.generic.type		= MTYPE_PTEXT;
	setupMenuInfo.setupcontrols.generic.flags		= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	setupMenuInfo.setupcontrols.generic.x			= 320;
	setupMenuInfo.setupcontrols.generic.y			= y;
	setupMenuInfo.setupcontrols.generic.id			= ID_CUSTOMIZECONTROLS;
	setupMenuInfo.setupcontrols.generic.callback	= UI_SetupMenu_Event; 
	setupMenuInfo.setupcontrols.string				= "CONTROLS";
	setupMenuInfo.setupcontrols.color				= color_white; // BFP - modified CONTROLS color
	setupMenuInfo.setupcontrols.style				= UI_CENTER;

	y += SETUP_MENU_VERTICAL_SPACING;
	setupMenuInfo.setupsystem.generic.type			= MTYPE_PTEXT;
	setupMenuInfo.setupsystem.generic.flags			= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	setupMenuInfo.setupsystem.generic.x				= 320;
	setupMenuInfo.setupsystem.generic.y				= y;
	setupMenuInfo.setupsystem.generic.id			= ID_SYSTEMCONFIG;
	setupMenuInfo.setupsystem.generic.callback		= UI_SetupMenu_Event; 
	setupMenuInfo.setupsystem.string				= "SYSTEM";
	setupMenuInfo.setupsystem.color					= color_white; // BFP - modified SYSTEM color
	setupMenuInfo.setupsystem.style					= UI_CENTER;

	y += SETUP_MENU_VERTICAL_SPACING;
	setupMenuInfo.game.generic.type					= MTYPE_PTEXT;
	setupMenuInfo.game.generic.flags				= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	setupMenuInfo.game.generic.x					= 320;
	setupMenuInfo.game.generic.y					= y;
	setupMenuInfo.game.generic.id					= ID_GAME;
	setupMenuInfo.game.generic.callback				= UI_SetupMenu_Event; 
	setupMenuInfo.game.string						= "GAME OPTIONS";
	setupMenuInfo.game.color						= color_white; // BFP - modified GAME OPTIONS color
	setupMenuInfo.game.style						= UI_CENTER;

	y += SETUP_MENU_VERTICAL_SPACING;
	setupMenuInfo.cdkey.generic.type				= MTYPE_PTEXT;
	setupMenuInfo.cdkey.generic.flags				= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	setupMenuInfo.cdkey.generic.x					= 320;
	setupMenuInfo.cdkey.generic.y					= y;
	setupMenuInfo.cdkey.generic.id					= ID_CDKEY;
	setupMenuInfo.cdkey.generic.callback			= UI_SetupMenu_Event; 
	setupMenuInfo.cdkey.string						= "CD Key";
	setupMenuInfo.cdkey.color						= color_white; // BFP - modified CD Key color
	setupMenuInfo.cdkey.style						= UI_CENTER;

	if( !trap_Cvar_VariableValue( "cl_paused" ) ) {
#if 0
		y += SETUP_MENU_VERTICAL_SPACING;
		setupMenuInfo.load.generic.type					= MTYPE_PTEXT;
		setupMenuInfo.load.generic.flags				= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
		setupMenuInfo.load.generic.x					= 320;
		setupMenuInfo.load.generic.y					= y;
		setupMenuInfo.load.generic.id					= ID_LOAD;
		setupMenuInfo.load.generic.callback				= UI_SetupMenu_Event; 
		setupMenuInfo.load.string						= "LOAD";
		setupMenuInfo.load.color						= color_red;
		setupMenuInfo.load.style						= UI_CENTER;

		y += SETUP_MENU_VERTICAL_SPACING;
		setupMenuInfo.save.generic.type					= MTYPE_PTEXT;
		setupMenuInfo.save.generic.flags				= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
		setupMenuInfo.save.generic.x					= 320;
		setupMenuInfo.save.generic.y					= y;
		setupMenuInfo.save.generic.id					= ID_SAVE;
		setupMenuInfo.save.generic.callback				= UI_SetupMenu_Event; 
		setupMenuInfo.save.string						= "SAVE";
		setupMenuInfo.save.color						= color_red;
		setupMenuInfo.save.style						= UI_CENTER;
#endif
	}

	y += SETUP_MENU_VERTICAL_SPACING;
	setupMenuInfo.defaults.generic.type				= MTYPE_PTEXT;
	setupMenuInfo.defaults.generic.flags			= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	setupMenuInfo.defaults.generic.x				= 320;
	setupMenuInfo.defaults.generic.y				= y;
	setupMenuInfo.defaults.generic.id				= ID_DEFAULTS;
	setupMenuInfo.defaults.generic.callback			= UI_SetupMenu_Event; 
	setupMenuInfo.defaults.string					= "DEFAULTS";
	setupMenuInfo.defaults.color					= color_white; // BFP - modified DEFAULTS color
	setupMenuInfo.defaults.style					= UI_CENTER;

	setupMenuInfo.back.generic.type					= MTYPE_BITMAP;
	setupMenuInfo.back.generic.name					= ART_BACK0;
	setupMenuInfo.back.generic.flags				= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	setupMenuInfo.back.generic.id					= ID_BACK;
	setupMenuInfo.back.generic.callback				= UI_SetupMenu_Event;
	setupMenuInfo.back.generic.x					= 0;
	setupMenuInfo.back.generic.y					= 480-80; // BFP - modified BACK button y postion
	setupMenuInfo.back.width						= 80; // BFP - modified BACK button width
	setupMenuInfo.back.height						= 80; // BFP - modified BACK button height
	setupMenuInfo.back.focuspic						= ART_BACK1;

	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.menubg ); // BFP - Menu background
	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.barlog ); // BFP - barlog
	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.banner ); // BFP - Banner
	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.setupplayer );
	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.bfpoptions ); // BFP - BFP OPTIONS
	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.setupcontrols );
	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.setupsystem );
	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.game );
	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.cdkey );
//	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.load );
//	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.save );
	// BFP - That isn't available
#if 0
	if( !trap_Cvar_VariableValue( "cl_paused" ) ) {
		Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.defaults );
	}
#endif
	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.defaults );
	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.back );
}


/*
=================
UI_SetupMenu_Cache
=================
*/
void UI_SetupMenu_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
	trap_R_RegisterShaderNoMip( ART_MENUBG ); // BFP - Menu background
	trap_R_RegisterShaderNoMip( ART_BARLOG ); // BFP - barlog
}


/*
===============
UI_SetupMenu
===============
*/
void UI_SetupMenu( void ) {
	UI_SetupMenu_Init();
	UI_PushMenu( &setupMenuInfo.menu );
}
