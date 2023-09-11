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
//
// ui_team.c
//

#include "ui_local.h"


#define ART_BARLOG		"menu/art/cap_barlog"	// BFP - barlog
#define TEAMMAIN_FRAME	"menu/art/cut_frame"

#define ID_JOINRED		100
#define ID_JOINBLUE		101
#define ID_JOINGAME		102
#define ID_SPECTATE		103


typedef struct
{
	menuframework_s	menu;
	menubitmap_s	barlog; // BFP - barlog
	menutext_s		banner; // BFP - banner
	menubitmap_s	frame;
	menutext_s		joinred;
	menutext_s		joinblue;
	menutext_s		joingame;
	menutext_s		spectate;
} teammain_t;

static teammain_t	s_teammain;

// bk001204 - unused
//static menuframework_s	s_teammain_menu;
//static menuaction_s		s_teammain_orders;
//static menuaction_s		s_teammain_voice;
//static menuaction_s		s_teammain_joinred;
//static menuaction_s		s_teammain_joinblue;
//static menuaction_s		s_teammain_joingame;
//static menuaction_s		s_teammain_spectate;


/*
===============
TeamMain_MenuEvent
===============
*/
static void TeamMain_MenuEvent( void* ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_JOINRED:
		trap_Cmd_ExecuteText( EXEC_APPEND, "cmd team red\n" );
		UI_ForceMenuOff();
		break;

	case ID_JOINBLUE:
		trap_Cmd_ExecuteText( EXEC_APPEND, "cmd team blue\n" );
		UI_ForceMenuOff();
		break;

	case ID_JOINGAME:
		trap_Cmd_ExecuteText( EXEC_APPEND, "cmd team free\n" );
		UI_ForceMenuOff();
		break;

	case ID_SPECTATE:
		trap_Cmd_ExecuteText( EXEC_APPEND, "cmd team spectator\n" );
		UI_ForceMenuOff();
		break;
	}
}


/*
===============
TeamMain_MenuInit
===============
*/
void TeamMain_MenuInit( void ) {
	int		y;
	int		gametype;
	char	info[MAX_INFO_STRING];

	memset( &s_teammain, 0, sizeof(s_teammain) );

	TeamMain_Cache();

	s_teammain.menu.wrapAround = qtrue;
	s_teammain.menu.fullscreen = qfalse;

	// BFP - barlog
	s_teammain.barlog.generic.type		= MTYPE_BITMAP;
	s_teammain.barlog.generic.name		= ART_BARLOG;
	s_teammain.barlog.generic.flags		= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_teammain.barlog.generic.x			= 140;
	s_teammain.barlog.generic.y			= 5;
	s_teammain.barlog.width				= 355;
	s_teammain.barlog.height			= 90;

	// BFP - banner
	s_teammain.banner.generic.type		= MTYPE_PTEXT;
	s_teammain.banner.generic.x			= 320;
	s_teammain.banner.generic.y			= 45;
	s_teammain.banner.string			= "SELECT TEAM";
	s_teammain.banner.color				= color_white;
	s_teammain.banner.style				= UI_CENTER|UI_BIGFONT;

	s_teammain.frame.generic.type   = MTYPE_BITMAP;
	s_teammain.frame.generic.flags	= QMF_INACTIVE;
	s_teammain.frame.generic.name   = TEAMMAIN_FRAME;
	s_teammain.frame.generic.x		= 142;
	s_teammain.frame.generic.y		= 118;
	s_teammain.frame.width			= 359;
	s_teammain.frame.height			= 256;

	y = 194;

	s_teammain.joinred.generic.type     = MTYPE_PTEXT;
	s_teammain.joinred.generic.flags    = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_teammain.joinred.generic.id       = ID_JOINRED;
	s_teammain.joinred.generic.callback = TeamMain_MenuEvent;
	s_teammain.joinred.generic.x        = 320;
	s_teammain.joinred.generic.y        = y;
	s_teammain.joinred.string           = "JOIN RED";
	s_teammain.joinred.style            = UI_CENTER|UI_SMALLFONT;
	s_teammain.joinred.color            = color_white; // BFP - modified JOIN RED button color
	y += 20;

	s_teammain.joinblue.generic.type     = MTYPE_PTEXT;
	s_teammain.joinblue.generic.flags    = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_teammain.joinblue.generic.id       = ID_JOINBLUE;
	s_teammain.joinblue.generic.callback = TeamMain_MenuEvent;
	s_teammain.joinblue.generic.x        = 320;
	s_teammain.joinblue.generic.y        = y;
	s_teammain.joinblue.string           = "JOIN BLUE";
	s_teammain.joinblue.style            = UI_CENTER|UI_SMALLFONT;
	s_teammain.joinblue.color            = color_white; // BFP - modified JOIN BLUE button color
	y += 20;

	s_teammain.joingame.generic.type     = MTYPE_PTEXT;
	s_teammain.joingame.generic.flags    = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_teammain.joingame.generic.id       = ID_JOINGAME;
	s_teammain.joingame.generic.callback = TeamMain_MenuEvent;
	s_teammain.joingame.generic.x        = 320;
	s_teammain.joingame.generic.y        = y;
	s_teammain.joingame.string           = "JOIN GAME";
	s_teammain.joingame.style            = UI_CENTER|UI_SMALLFONT;
	s_teammain.joingame.color            = color_white; // BFP - modified JOIN GAME button color
	y += 20;

	s_teammain.spectate.generic.type     = MTYPE_PTEXT;
	s_teammain.spectate.generic.flags    = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_teammain.spectate.generic.id       = ID_SPECTATE;
	s_teammain.spectate.generic.callback = TeamMain_MenuEvent;
	s_teammain.spectate.generic.x        = 320;
	s_teammain.spectate.generic.y        = y;
	s_teammain.spectate.string           = "SPECTATE";
	s_teammain.spectate.style            = UI_CENTER|UI_SMALLFONT;
	s_teammain.spectate.color            = color_white; // BFP - modified SPECTATE button color
	y += 20;

	trap_GetConfigString(CS_SERVERINFO, info, MAX_INFO_STRING);   
	gametype = atoi( Info_ValueForKey( info,"g_gametype" ) );
			      
	// set initial states
	switch( gametype ) {
	case GT_SINGLE_PLAYER:
	case GT_FFA:
	case GT_TOURNAMENT:
		s_teammain.joinred.generic.flags  |= QMF_GRAYED;
		s_teammain.joinblue.generic.flags |= QMF_GRAYED;
		break;

	default:
	case GT_TEAM:
	case GT_CTF:
		s_teammain.joingame.generic.flags |= QMF_GRAYED;
		break;
	}

	Menu_AddItem( &s_teammain.menu, (void*) &s_teammain.barlog ); // BFP - barlog
	Menu_AddItem( &s_teammain.menu, (void*) &s_teammain.banner ); // BFP - banner
	Menu_AddItem( &s_teammain.menu, (void*) &s_teammain.frame );
	Menu_AddItem( &s_teammain.menu, (void*) &s_teammain.joinred );
	Menu_AddItem( &s_teammain.menu, (void*) &s_teammain.joinblue );
	Menu_AddItem( &s_teammain.menu, (void*) &s_teammain.joingame );
	Menu_AddItem( &s_teammain.menu, (void*) &s_teammain.spectate );
}


/*
===============
TeamMain_Cache
===============
*/
void TeamMain_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_BARLOG ); // BFP - barlog
	trap_R_RegisterShaderNoMip( TEAMMAIN_FRAME );
}


/*
===============
UI_TeamMainMenu
===============
*/
void UI_TeamMainMenu( void ) {
	TeamMain_MenuInit();
	UI_PushMenu ( &s_teammain.menu );
}
