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
#include "ui_local.h"

#define ART_MENUBG			"menu/art/menubg"		// BFP - Menu background
#define ART_BARLOG			"menu/art/cap_barlog"	// BFP - barlog
#define SERVERINFO_FRAMEL	"menu/art/frame2_l"
#define SERVERINFO_FRAMER	"menu/art/frame1_r"
#define SERVERINFO_BACK0	"menu/art/back_0"
#define SERVERINFO_BACK1	"menu/art/back_1"

// BFP - Arrows
#define ART_ARROWS			"menu/art/gs_arrows_0"
#define ART_ARROWSL			"menu/art/gs_arrows_l"
#define ART_ARROWSR			"menu/art/gs_arrows_r"

static char* serverinfo_artlist[] =
{
	SERVERINFO_BACK0,
	SERVERINFO_BACK1,

	// BFP - Add arrows inside that
	ART_ARROWS,
	ART_ARROWSL,
	ART_ARROWSR,
	NULL
};

#define ID_ADD	 100
#define ID_BACK	 101

// BFP - ID for arrows
#define ID_NEXT	 102
#define ID_PREV	 103

// BFP - Lines per page
#define LINES_PER_PAGE	15

typedef struct
{
	menuframework_s	menu;
	menutext_s		banner;
	menubitmap_s	menubg; // BFP - Menu background
	menubitmap_s	barlog; // BFP - barlog
	menubitmap_s	back;
	menutext_s		add;
	char			info[MAX_INFO_STRING];
	int				numlines;
	int				currentPage;
	int				totalPages;
	menubitmap_s	arrows;
	menubitmap_s	next;
	menubitmap_s	prev;
} serverinfo_t;

static serverinfo_t	s_serverinfo;


/*
=================
Favorites_Add

Add current server to favorites
=================
*/
void Favorites_Add( void )
{
	char	adrstr[128];
	char	serverbuff[128];
	int		i;
	int		best;

	trap_Cvar_VariableStringBuffer( "cl_currentServerAddress", serverbuff, sizeof(serverbuff) );
	if (!serverbuff[0])
		return;

	best = 0;
	for (i=0; i<MAX_FAVORITESERVERS; i++)
	{
		trap_Cvar_VariableStringBuffer( va("server%d",i+1), adrstr, sizeof(adrstr) );
		if (!Q_stricmp(serverbuff,adrstr))
		{
			// already in list
			return;
		}
		
		// use first empty or non-numeric available slot
		if ((adrstr[0]  < '0' || adrstr[0] > '9' ) && !best)
			best = i+1;
	}

	if (best)
		trap_Cvar_Set( va("server%d",best), serverbuff);
}


/*
=================
ServerInfo_Event
=================
*/
static void ServerInfo_Event( void* ptr, int event )
{
	switch (((menucommon_s*)ptr)->id)
	{
		case ID_ADD:
			if (event != QM_ACTIVATED)
				break;
		
			Favorites_Add();
			UI_PopMenu();
			break;

		case ID_BACK:
			if (event != QM_ACTIVATED)
				break;

			UI_PopMenu();
			break;
		
		// BFP - Prev and next events
		case ID_PREV:
			if (event != QM_ACTIVATED)
				break;

			if (s_serverinfo.currentPage > 0) {
				s_serverinfo.currentPage--;
				Menu_Draw( &s_serverinfo.menu );
			}
			break;

		case ID_NEXT:
			if (event != QM_ACTIVATED)
				break;

			if (s_serverinfo.currentPage < s_serverinfo.totalPages - 1) {
				s_serverinfo.currentPage++;
				Menu_Draw( &s_serverinfo.menu );
			}
			break;
	}
}

/*
=================
ServerInfo_MenuDraw
=================
*/
static void ServerInfo_MenuDraw( void )
{
	const char		*s;
	char			key[MAX_INFO_KEY];
	char			value[MAX_INFO_VALUE];
	int				y;
	// BFP - For pagination
	char			pageIndicator[64];
	int				startLine, endLine;
	int				lineIndex;
	int				len, wrappedLen, i, j;
	char			wrappedValue[MAX_INFO_VALUE];
#define LIMIT_CHARACTERS		39

	// BFP - NOTE: On original BFP, Menu_Draw is used at the end of the function, so hides completely the server info (·_·)
	// To avoid the hidden info issue, the function needs to be called at the beginning, just here:
	Menu_Draw( &s_serverinfo.menu );

	// BFP - NOTE: Pagination is implemented here, also looks better when the user needs to see all info avoiding letters going off the screen

	startLine = s_serverinfo.currentPage * LINES_PER_PAGE;
	endLine = startLine + LINES_PER_PAGE;
	if ( endLine > s_serverinfo.numlines )
		endLine = s_serverinfo.numlines;

	y = SCREEN_HEIGHT/2 - ( endLine - startLine ) * ( SMALLCHAR_HEIGHT )/2 - 20;
	s = s_serverinfo.info;
	lineIndex = 0;

	while ( s ) {
		Info_NextPair( &s, key, value );
		if ( !key[0] ) {
			break;
		}

		// show only these lines in the page
		if ( lineIndex >= startLine && lineIndex < endLine ) {
			Q_strcat( key, MAX_INFO_KEY, ":" ); 

			UI_DrawString( SCREEN_WIDTH*0.50 - 2, y, key, UI_RIGHT|UI_SMALLFONT, color_white );

			// wrap the value if it exceeds the limit of characters
			len = strlen( value );
			wrappedLen = 0;
			while ( len > 0 ) {
				if ( len > LIMIT_CHARACTERS ) {
					// find the last occurrence of ".", ",", "-", "_", or " " within the limit
					for ( i = LIMIT_CHARACTERS; i > 0; i-- ) {
						if ( value[wrappedLen + i] == '.' || value[wrappedLen + i] == ',' 
						|| value[wrappedLen + i] == '-' || value[wrappedLen + i] == '_' 
						|| value[wrappedLen + i] == ' ' ) {
							break;
						}
					}
					if ( i == 0 ) {
						i = LIMIT_CHARACTERS; // no delimiter found, use the limit
					}
					// include the delimiter in the first part
					strncpy( wrappedValue, value + wrappedLen, i + 1 );
					wrappedValue[i + 1] = '\0';
					wrappedLen += i + 1;
					len -= i + 1;
				} else {
					strcpy( wrappedValue, value + wrappedLen );
					len = 0;
				}
				UI_DrawString( SCREEN_WIDTH*0.50 + 2, y, wrappedValue, UI_LEFT|UI_SMALLFONT, text_color_normal );
				y += SMALLCHAR_HEIGHT;
			}
		}

		lineIndex++;
	}
	Com_sprintf( pageIndicator, sizeof(pageIndicator), "Page %d of %d", s_serverinfo.currentPage + 1, s_serverinfo.totalPages );
	UI_DrawString( 320, 420, pageIndicator, UI_CENTER|UI_SMALLFONT, color_white );
}



/*
=================
ServerInfo_MenuKey
=================
*/
static sfxHandle_t ServerInfo_MenuKey( int key )
{
	return ( Menu_DefaultKey( &s_serverinfo.menu, key ) );
}

/*
=================
ServerInfo_Cache
=================
*/
void ServerInfo_Cache( void )
{
	int	i;

	// touch all our pics
	for (i=0; ;i++)
	{
		if (!serverinfo_artlist[i])
			break;
		trap_R_RegisterShaderNoMip(serverinfo_artlist[i]);
	}
}

/*
=================
UI_ServerInfoMenu
=================
*/
void UI_ServerInfoMenu( void )
{
	const char		*s;
	char			key[MAX_INFO_KEY];
	char			value[MAX_INFO_VALUE];

	// zero set all our globals
	memset( &s_serverinfo, 0 ,sizeof(serverinfo_t) );

	ServerInfo_Cache();

	s_serverinfo.menu.draw       = ServerInfo_MenuDraw;
	s_serverinfo.menu.key        = ServerInfo_MenuKey;
	s_serverinfo.menu.wrapAround = qtrue;
	s_serverinfo.menu.fullscreen = qtrue;

	// BFP - Menu background
	s_serverinfo.menubg.generic.type  = MTYPE_BITMAP;
	s_serverinfo.menubg.generic.name  = ART_MENUBG;
	s_serverinfo.menubg.generic.flags = QMF_LEFT_JUSTIFY | QMF_INACTIVE;
	s_serverinfo.menubg.generic.x	  = 0;
	s_serverinfo.menubg.generic.y	  = 0;
	s_serverinfo.menubg.width		  = 640;
	s_serverinfo.menubg.height		  = 480;

	// BFP - barlog
	s_serverinfo.barlog.generic.type  = MTYPE_BITMAP;
	s_serverinfo.barlog.generic.name  = ART_BARLOG;
	s_serverinfo.barlog.generic.flags = QMF_LEFT_JUSTIFY | QMF_INACTIVE;
	s_serverinfo.barlog.generic.x	  = 140;
	s_serverinfo.barlog.generic.y	  = 5;
	s_serverinfo.barlog.width		  = 355;
	s_serverinfo.barlog.height		  = 90;

	s_serverinfo.banner.generic.type  = MTYPE_PTEXT; // BFP - modified SERVER INFO title type
	s_serverinfo.banner.generic.flags = QMF_CENTER_JUSTIFY|QMF_INACTIVE; // BFP - modified SERVER INFO title flags
	s_serverinfo.banner.generic.x	  = 320;
	s_serverinfo.banner.generic.y	  = 45; // BFP - modified SERVER INFO title y position
	s_serverinfo.banner.string		  = "SERVER INFO";
	s_serverinfo.banner.color	      = color_white;
	s_serverinfo.banner.style	      = UI_CENTER|UI_BIGFONT; // BFP - modified SERVER INFO title color

	s_serverinfo.add.generic.type	  = MTYPE_PTEXT;
	s_serverinfo.add.generic.flags    = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_serverinfo.add.generic.callback = ServerInfo_Event;
	s_serverinfo.add.generic.id	      = ID_ADD;
	s_serverinfo.add.generic.x		  = 320;
	s_serverinfo.add.generic.y		  = 450; // BFP - (originally 400 on BFP) modified ADD TO FAVORITES button y position
	s_serverinfo.add.string  		  = "ADD TO FAVORITES";
	s_serverinfo.add.style  		  = UI_CENTER|UI_SMALLFONT;
	s_serverinfo.add.color			  =	color_white; // BFP - modified ADD TO FAVORITES button color
	if( trap_Cvar_VariableValue( "sv_running" ) ) {
		s_serverinfo.add.generic.flags |= QMF_GRAYED;
	}

	s_serverinfo.back.generic.type	   = MTYPE_BITMAP;
	s_serverinfo.back.generic.name     = SERVERINFO_BACK0;
	s_serverinfo.back.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_serverinfo.back.generic.callback = ServerInfo_Event;
	s_serverinfo.back.generic.id	   = ID_BACK;
	s_serverinfo.back.generic.x		   = 0;
	s_serverinfo.back.generic.y		   = 480-80; // BFP - modified BACK button y position
	s_serverinfo.back.width  		   = 80; // BFP - modified BACK button width
	s_serverinfo.back.height  		   = 80; // BFP - modified BACK button height
	s_serverinfo.back.focuspic         = SERVERINFO_BACK1;

	// BFP - Arrows to go prev or next
#define ART_ARROWS_X	260
#define ART_ARROWS_Y	375
	s_serverinfo.arrows.generic.type		= MTYPE_BITMAP;
	s_serverinfo.arrows.generic.name		= ART_ARROWS;
	s_serverinfo.arrows.generic.flags		= QMF_INACTIVE;
	s_serverinfo.arrows.generic.x			= ART_ARROWS_X;
	s_serverinfo.arrows.generic.y			= ART_ARROWS_Y;
	s_serverinfo.arrows.width				= 128;
	s_serverinfo.arrows.height				= 32;

	s_serverinfo.prev.generic.type			= MTYPE_BITMAP;
	s_serverinfo.prev.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_serverinfo.prev.generic.callback		= ServerInfo_Event;
	s_serverinfo.prev.generic.id			= ID_PREV;
	s_serverinfo.prev.generic.x				= ART_ARROWS_X;
	s_serverinfo.prev.generic.y				= ART_ARROWS_Y;
	s_serverinfo.prev.width  				= 64;
	s_serverinfo.prev.height  				= 32;
	s_serverinfo.prev.focuspic				= ART_ARROWSL;

	s_serverinfo.next.generic.type	    = MTYPE_BITMAP;
	s_serverinfo.next.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_serverinfo.next.generic.callback	= ServerInfo_Event;
	s_serverinfo.next.generic.id		= ID_NEXT;
	s_serverinfo.next.generic.x			= ART_ARROWS_X+61;
	s_serverinfo.next.generic.y			= ART_ARROWS_Y;
	s_serverinfo.next.width  			= 64;
	s_serverinfo.next.height  		    = 32;
	s_serverinfo.next.focuspic			= ART_ARROWSR;
#undef ART_ARROWS_X
#undef ART_ARROWS_Y
	// BFP - End of arrows to go prev or next ^

	trap_GetConfigString( CS_SERVERINFO, s_serverinfo.info, MAX_INFO_STRING );

	s_serverinfo.numlines = 0;
	s = s_serverinfo.info;
	while ( s ) {
		Info_NextPair( &s, key, value );
		if ( !key[0] ) {
			break;
		}
		s_serverinfo.numlines++;
	}

	// BFP - Handle total pages
	s_serverinfo.totalPages = (s_serverinfo.numlines + LINES_PER_PAGE - 1) / LINES_PER_PAGE;

	Menu_AddItem( &s_serverinfo.menu, (void*) &s_serverinfo.menubg ); // BFP - Menu background
	Menu_AddItem( &s_serverinfo.menu, (void*) &s_serverinfo.barlog ); // BFP - barlog
	Menu_AddItem( &s_serverinfo.menu, (void*) &s_serverinfo.banner );
	Menu_AddItem( &s_serverinfo.menu, (void*) &s_serverinfo.add );
	Menu_AddItem( &s_serverinfo.menu, (void*) &s_serverinfo.back );
	// BFP - Arrows to handle the pages
	Menu_AddItem( &s_serverinfo.menu, (void*) &s_serverinfo.arrows );
	if ( s_serverinfo.totalPages > 1 ) {
		Menu_AddItem( &s_serverinfo.menu, (void*) &s_serverinfo.next );
		Menu_AddItem( &s_serverinfo.menu, (void*) &s_serverinfo.prev );
	}

	// BFP - Initialize the current page to zero
	s_serverinfo.currentPage = 0;

	UI_PushMenu( &s_serverinfo.menu );
}


