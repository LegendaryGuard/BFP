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
/*
=======================================================================

SYSTEM CONFIGURATION MENU

=======================================================================
*/

#include "ui_local.h"


#define ART_MENUBG			"menu/art/menubg"		// BFP - Menu background
#define ART_BARLOG			"menu/art/cap_barlog"	// BFP - barlog
#define ART_BACK0			"menu/art/back_0"
#define ART_BACK1			"menu/art/back_1"

#define ID_GRAPHICS			10
#define ID_DISPLAY			11
#define ID_SOUND			12
#define ID_NETWORK			13
#define ID_BACK				14

#define VERTICAL_SPACING	34

typedef struct {
	menuframework_s	menu;

	menubitmap_s		menubg; // BFP - Menu background
	menubitmap_s		barlog; // BFP - barlog
	menutext_s			banner;

	menutext_s		graphics;
	menutext_s		display;
	menutext_s		sound;
	menutext_s		network;
	menubitmap_s	back;
} optionsmenu_t;

static optionsmenu_t	s_options;


/*
=================
Options_Event
=================
*/
static void Options_Event( void* ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_GRAPHICS:
		UI_GraphicsOptionsMenu();
		break;

	case ID_DISPLAY:
		UI_DisplayOptionsMenu();
		break;

	case ID_SOUND:
		UI_SoundOptionsMenu();
		break;

	case ID_NETWORK:
		UI_NetworkOptionsMenu();
		break;

	case ID_BACK:
		UI_PopMenu();
		break;
	}
}


/*
===============
SystemConfig_Cache
===============
*/
void SystemConfig_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_MENUBG ); // BFP - Menu background
	trap_R_RegisterShaderNoMip( ART_BARLOG ); // BFP - barlog
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
}

/*
===============
Options_MenuInit
===============
*/
void Options_MenuInit( void ) {
	int				y;
	uiClientState_t	cstate;

	memset( &s_options, 0, sizeof(optionsmenu_t) );

	SystemConfig_Cache();
	s_options.menu.wrapAround = qtrue;

	trap_GetClientState( &cstate );
	if ( cstate.connState >= CA_CONNECTED ) {
		s_options.menu.fullscreen = qfalse;
	}
	else {
		s_options.menu.fullscreen = qtrue;
	}

	// BFP - Menu background
	s_options.menubg.generic.type	= MTYPE_BITMAP;
	s_options.menubg.generic.name	= ART_MENUBG;
	s_options.menubg.generic.flags	= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_options.menubg.generic.x		= 0;
	s_options.menubg.generic.y		= 0;
	s_options.menubg.width			= 640;
	s_options.menubg.height			= 480;

	// BFP - barlog
	s_options.barlog.generic.type	= MTYPE_BITMAP;
	s_options.barlog.generic.name	= ART_BARLOG;
	s_options.barlog.generic.flags	= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_options.barlog.generic.x		= 140;
	s_options.barlog.generic.y		= 5;
	s_options.barlog.width			= 355;
	s_options.barlog.height			= 90;

	s_options.banner.generic.type	= MTYPE_PTEXT; // BFP - modified SYSTEM SETUP title type
	s_options.banner.generic.flags	= QMF_CENTER_JUSTIFY|QMF_INACTIVE;
	s_options.banner.generic.x		= 320;
	s_options.banner.generic.y		= 45; // BFP - modified SYSTEM SETUP title y position
	s_options.banner.string		    = "SYSTEM SETUP";
	s_options.banner.color			= color_white;
	s_options.banner.style			= UI_CENTER|UI_BIGFONT; // BFP - modified SYSTEM SETUP title style


	y = 168;
	s_options.graphics.generic.type		= MTYPE_PTEXT;
	s_options.graphics.generic.flags	= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_options.graphics.generic.callback	= Options_Event;
	s_options.graphics.generic.id		= ID_GRAPHICS;
	s_options.graphics.generic.x		= 320;
	s_options.graphics.generic.y		= y;
	s_options.graphics.string			= "GRAPHICS";
	s_options.graphics.color			= color_white; // BFP - modified GRAPHICS button color
	s_options.graphics.style			= UI_CENTER;

	y += VERTICAL_SPACING;
	s_options.display.generic.type		= MTYPE_PTEXT;
	s_options.display.generic.flags		= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_options.display.generic.callback	= Options_Event;
	s_options.display.generic.id		= ID_DISPLAY;
	s_options.display.generic.x			= 320;
	s_options.display.generic.y			= y;
	s_options.display.string			= "DISPLAY";
	s_options.display.color				= color_white; // BFP - modified DISPLAY button color
	s_options.display.style				= UI_CENTER;

	y += VERTICAL_SPACING;
	s_options.sound.generic.type		= MTYPE_PTEXT;
	s_options.sound.generic.flags		= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_options.sound.generic.callback	= Options_Event;
	s_options.sound.generic.id			= ID_SOUND;
	s_options.sound.generic.x			= 320;
	s_options.sound.generic.y			= y;
	s_options.sound.string				= "SOUND";
	s_options.sound.color				= color_white; // BFP - modified SOUND button color
	s_options.sound.style				= UI_CENTER;

	y += VERTICAL_SPACING;
	s_options.network.generic.type		= MTYPE_PTEXT;
	s_options.network.generic.flags		= QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_options.network.generic.callback	= Options_Event;
	s_options.network.generic.id		= ID_NETWORK;
	s_options.network.generic.x			= 320;
	s_options.network.generic.y			= y;
	s_options.network.string			= "NETWORK";
	s_options.network.color				= color_white; // BFP - modified NETWORK button color
	s_options.network.style				= UI_CENTER;

	s_options.back.generic.type	    = MTYPE_BITMAP;
	s_options.back.generic.name     = ART_BACK0;
	s_options.back.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_options.back.generic.callback = Options_Event;
	s_options.back.generic.id	    = ID_BACK;
	s_options.back.generic.x		= 0;
	s_options.back.generic.y		= 480-80; // BFP - modified BACK button y positon
	s_options.back.width  		    = 80; // BFP - modified BACK button width
	s_options.back.height  		    = 80; // BFP - modified BACK button height
	s_options.back.focuspic         = ART_BACK1;

	Menu_AddItem( &s_options.menu, ( void * ) &s_options.menubg ); // BFP - Menu background
	Menu_AddItem( &s_options.menu, ( void * ) &s_options.barlog ); // BFP - barlog
	Menu_AddItem( &s_options.menu, ( void * ) &s_options.banner );
	Menu_AddItem( &s_options.menu, ( void * ) &s_options.graphics );
	Menu_AddItem( &s_options.menu, ( void * ) &s_options.display );
	Menu_AddItem( &s_options.menu, ( void * ) &s_options.sound );
	Menu_AddItem( &s_options.menu, ( void * ) &s_options.network );
	Menu_AddItem( &s_options.menu, ( void * ) &s_options.back );
}


/*
===============
UI_SystemConfigMenu
===============
*/
void UI_SystemConfigMenu( void ) {
	Options_MenuInit();
	UI_PushMenu ( &s_options.menu );
}
