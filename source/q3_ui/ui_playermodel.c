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

#define MODEL_BACK0			"menu/art/back_0"
#define MODEL_BACK1			"menu/art/back_1"
#define MODEL_SELECT		"menu/art/opponents_select"
#define MODEL_SELECTED		"menu/art/opponents_selected"
#define ART_MENUBG			"menu/art/menubg"		// BFP - Menu background
#define ART_BARLOG			"menu/art/cap_barlog"	// BFP - barlog
#define MODEL_ARROWS		"menu/art/gs_arrows_0"
#define MODEL_ARROWSL		"menu/art/gs_arrows_l"
#define MODEL_ARROWSR		"menu/art/gs_arrows_r"

#define LOW_MEMORY			(5 * 1024 * 1024)

static char* playermodel_artlist[] =
{
	MODEL_BACK0,	
	MODEL_BACK1,	
	MODEL_SELECT,
	MODEL_SELECTED,
	ART_MENUBG,	// BFP - Menu background
	ART_BARLOG,	// BFP - barlog
	MODEL_ARROWS,
	MODEL_ARROWSL,
	MODEL_ARROWSR,
	NULL
};

#define PLAYERGRID_COLS		5 // BFP - modified PLAYERGRID_COLS, before 5
#define PLAYERGRID_ROWS		4
#define MAX_MODELSPERPAGE	(PLAYERGRID_ROWS*PLAYERGRID_COLS)

#define MAX_PLAYERMODELS	256

#define ID_PLAYERPIC0		0
#define ID_PLAYERPIC1		1
#define ID_PLAYERPIC2		2
#define ID_PLAYERPIC3		3
#define ID_PLAYERPIC4		4
#define ID_PLAYERPIC5		5
#define ID_PLAYERPIC6		6
#define ID_PLAYERPIC7		7
#define ID_PLAYERPIC8		8
#define ID_PLAYERPIC9		9
#define ID_PLAYERPIC10		10
#define ID_PLAYERPIC11		11
#define ID_PLAYERPIC12		12
#define ID_PLAYERPIC13		13
#define ID_PLAYERPIC14		14
#define ID_PLAYERPIC15		15
#define ID_PREVPAGE			100
#define ID_NEXTPAGE			101
#define ID_BACK				102

#define MAX_KIATTACKS		5	// BFP - Maximum of ki attacks

#define MAX_NAMELENGTH		20	// BFP - Name textinput length

typedef struct
{
	menuframework_s	menu;
	menubitmap_s	pics[MAX_MODELSPERPAGE];
	menubitmap_s	picbuttons[MAX_MODELSPERPAGE];
	menubitmap_s	kipics[MAX_KIATTACKS]; // BFP - Ki attack sets
	menubitmap_s	menubg; // BFP - Menu background
	menubitmap_s	barlog; // BFP - barlog
	menutext_s		banner;
	menufield_s		name; // BFP - Name textinput
	menubitmap_s	back;
	menubitmap_s	player;
	menubitmap_s	arrows;
	menubitmap_s	left;
	menubitmap_s	right;
	menutext_s		modelname;
	menutext_s		skinname;
	menutext_s		playername;
	playerInfo_t	playerinfo;
	int				nummodels;
	char			modelnames[MAX_PLAYERMODELS][128];
	int				modelpage;
	int				numpages;
	char			modelskin[64];
	int				selectedmodel;
} playermodel_t;

static playermodel_t s_playermodel;

/*
=================
PlayerModel_DrawName
=================
*/
static void PlayerModel_DrawName( void* self ) { // BFP - Player name draw
	menufield_s*	f;
	qboolean		focus;
	int				style;
	char*			txt;
	char			c;
	float*			color;
	int				n;
	int				basex, x, y;

	f = (menufield_s*)self;
	basex = f->generic.x;
	y = f->generic.y;
	focus = (f->generic.parent->cursor == f->generic.menuPosition);

	style = UI_LEFT | UI_SMALLFONT;
	color = text_color_normal;
	if ( focus ) {
		style |= UI_PULSE;
		color = text_color_highlight;
	}

	UI_DrawProportionalString( basex, y, "Name:", style, color );

	// draw the actual name
	basex += 75;
	y += 4;
	txt = f->field.buffer;
	color = g_color_table[ColorIndex( COLOR_WHITE )];
	x = basex;
	while ( ( c = *txt ) != 0 ) {
		if ( !focus && Q_IsColorString( txt ) ) {
			n = ColorIndex( *(txt + 1) );
			if ( n == 0 ) {
				n = 7;
			}
			color = g_color_table[n];
			txt += 2;
			continue;
		}
		UI_DrawChar( x, y, c, style, color );
		txt++;
		x += SMALLCHAR_WIDTH;
	}

	// draw cursor if we have focus
	if ( focus ) {
		if ( trap_Key_GetOverstrikeMode() ) {
			c = 11;
		} else {
			c = 10;
		}
		style &= ~UI_PULSE;
		style |= UI_BLINK;

		UI_DrawChar( basex + f->field.cursor * SMALLCHAR_WIDTH, y, c, style, color_white );
	}
}

/*
=================
PlayerModel_UpdateGrid
=================
*/
static void PlayerModel_UpdateGrid( void )
{
	int	i;
    int	j;

	j = s_playermodel.modelpage * MAX_MODELSPERPAGE;
	for (i=0; i<PLAYERGRID_ROWS*PLAYERGRID_COLS; i++,j++)
	{
		if (j < s_playermodel.nummodels)
		{ 
			// model/skin portrait
 			s_playermodel.pics[i].generic.name         = s_playermodel.modelnames[j];
			s_playermodel.picbuttons[i].generic.flags &= (unsigned int)~QMF_INACTIVE;
		}
		else
		{
			// dead slot
 			s_playermodel.pics[i].generic.name         = NULL;
			s_playermodel.picbuttons[i].generic.flags |= QMF_INACTIVE;
		}

 		s_playermodel.pics[i].generic.flags       &= (unsigned int)~QMF_HIGHLIGHT;
 		s_playermodel.pics[i].shader               = 0;
 		s_playermodel.picbuttons[i].generic.flags |= QMF_PULSEIFFOCUS;
	}

	if (s_playermodel.selectedmodel/MAX_MODELSPERPAGE == s_playermodel.modelpage)
	{
		// set selected model
		i = s_playermodel.selectedmodel % MAX_MODELSPERPAGE;

		s_playermodel.pics[i].generic.flags       |= QMF_HIGHLIGHT;
		s_playermodel.picbuttons[i].generic.flags &= (unsigned int)~QMF_PULSEIFFOCUS;
	}

	if (s_playermodel.numpages > 1)
	{
		if (s_playermodel.modelpage > 0)
			s_playermodel.left.generic.flags &= (unsigned int)~QMF_INACTIVE;
		else
			s_playermodel.left.generic.flags |= QMF_INACTIVE;

		if (s_playermodel.modelpage < s_playermodel.numpages-1)
			s_playermodel.right.generic.flags &= (unsigned int)~QMF_INACTIVE;
		else
			s_playermodel.right.generic.flags |= QMF_INACTIVE;
	}
	else
	{
		// hide left/right markers
		s_playermodel.left.generic.flags |= QMF_INACTIVE;
		s_playermodel.right.generic.flags |= QMF_INACTIVE;
	}
}

/*
=================
PlayerModel_UpdateModel
=================
*/
static void PlayerModel_UpdateModel( void )
{
	vec3_t	viewangles;
	vec3_t	moveangles;

	memset( &s_playermodel.playerinfo, 0, sizeof(playerInfo_t) );
	
	viewangles[YAW]   = 180; // BFP - before 180 - 30
	viewangles[PITCH] = 0;
	viewangles[ROLL]  = 0;
	VectorClear( moveangles );

	UI_PlayerInfo_SetModel( &s_playermodel.playerinfo, s_playermodel.modelskin );
	UI_PlayerInfo_SetInfo( &s_playermodel.playerinfo, LEGS_RUN, TORSO_RUN, viewangles, moveangles, WP_NONE, qfalse ); // BFP - before "WP_MACHINEGUN"
}

/*
=================
PlayerModel_SaveChanges
=================
*/
static void PlayerModel_SaveChanges( void )
{
	trap_Cvar_Set( "name", s_playermodel.name.field.buffer ); // BFP - Name textinput
	trap_Cvar_Set( "model", s_playermodel.modelskin );
	trap_Cvar_Set( "headmodel", s_playermodel.modelskin );
	trap_Cvar_Set( "team_model", s_playermodel.modelskin );
	trap_Cvar_Set( "team_headmodel", s_playermodel.modelskin );
}

/*
=================
PlayerModel_MenuEvent
=================
*/
static void PlayerModel_MenuEvent( void* ptr, int event )
{
	if (event != QM_ACTIVATED)
		return;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_PREVPAGE:
			if (s_playermodel.modelpage > 0)
			{
				s_playermodel.modelpage--;
				PlayerModel_UpdateGrid();
			}
			break;

		case ID_NEXTPAGE:
			if (s_playermodel.modelpage < s_playermodel.numpages-1)
			{
				s_playermodel.modelpage++;
				PlayerModel_UpdateGrid();
			}
			break;

		case ID_BACK:
			PlayerModel_SaveChanges();
			UI_PopMenu();
			break;
	}
}

/*
=================
PlayerModel_MenuKey
=================
*/
static sfxHandle_t PlayerModel_MenuKey( int key )
{
	menucommon_s*	m;
	int				picnum;

	switch (key)
	{
		case K_KP_UPARROW: // BFP - before K_KP_LEFTARROW
		case K_UPARROW: // BFP - before K_LEFTARROW
			m = Menu_ItemAtCursor(&s_playermodel.menu);
			picnum = m->id - ID_PLAYERPIC0;
			if (picnum >= 0 && picnum <= 15)
			{
				if (picnum > 0)
				{
					Menu_SetCursor(&s_playermodel.menu,s_playermodel.menu.cursor-1);
					return (menu_move_sound);
					
				}
				else if (s_playermodel.modelpage > 0)
				{
					s_playermodel.modelpage--;
					Menu_SetCursor(&s_playermodel.menu,s_playermodel.menu.cursor+15);
					PlayerModel_UpdateGrid();
					return (menu_move_sound);
				}
				else
					return (menu_buzz_sound);
			}
			break;

		case K_KP_DOWNARROW: // BFP - before K_KP_RIGHTARROW
		case K_DOWNARROW: // BFP - before K_RIGHTARROW
			m = Menu_ItemAtCursor(&s_playermodel.menu);
			picnum = m->id - ID_PLAYERPIC0;
			if (picnum >= 0 && picnum <= 15)
			{
				if ((picnum < 15) && (s_playermodel.modelpage*MAX_MODELSPERPAGE + picnum+1 < s_playermodel.nummodels))
				{
					Menu_SetCursor(&s_playermodel.menu,s_playermodel.menu.cursor+1);
					return (menu_move_sound);
				}					
				else if ((picnum == 15) && (s_playermodel.modelpage < s_playermodel.numpages-1))
				{
					s_playermodel.modelpage++;
					Menu_SetCursor(&s_playermodel.menu,s_playermodel.menu.cursor-15);
					PlayerModel_UpdateGrid();
					return (menu_move_sound);
				}
				else
					return (menu_buzz_sound);
			}
			break;
			
		case K_MOUSE2:
		case K_ESCAPE:
			PlayerModel_SaveChanges();
			break;
	}

	return ( Menu_DefaultKey( &s_playermodel.menu, key ) );
}


/*
=================
extractDirectoryName
=================
*/
static void extractDirectoryName( const char *input, char *output, int maxOutputSize ) // BFP - Extract the name of a directory
{
	int i;

	output[0] = '\0';

	// find the first '/'
	for (i = 0; i < strlen(input); i++) {
		if (input[i] == '/') {
			break;
		}
	}

	// copy the characters before the first '/'
	if (i < maxOutputSize) {
		strncpy(output, input, i);
		output[i] = '\0'; // null-terminate the output
	}
}


/*
=================
PlayerModel_SetKiAttacks
=================
*/
static void PlayerModel_SetKiAttacks( void ) // BFP - Set ki attack pics
{
	int		numdirs;
	char	dirlist[2048], modelselected[2048];
	char*	dirptr;
	int		i, j, dirlen, bfpnumber;

	// BFP - NOTE: BFP vanilla uses the static icons for every character from bfp1 to bfp6.
	// It would be cool to parse bfp_attacksets.cfg file and set every character their own ki attacks. 
	// These are the strings used in the code to parse that cfg file:
	/*
		// 1st parameter strings used in their conditionals when parsing the cfg file and setting their values in the 2nd parameter:
		"defaultModel"
		"modelPrefix"
		"attack"
		"attackset"
		"end"
		// printing messages when reading the cfg file:
		"BFP weapon config file too long\n"
		"unable to open weapon config file.\n"
		"reading bfp_attacksets.cfg\n"
	*/

	// iterate directory of all player models
	numdirs = trap_FS_GetFileList("models/players", "/", dirlist, 2048 );
	dirptr  = dirlist;
	for (i=0; i<numdirs && s_playermodel.nummodels < MAX_PLAYERMODELS; i++,dirptr+=dirlen+1)
	{
		dirlen = (int)strlen(dirptr);

		if (dirlen && dirptr[dirlen-1]=='/') dirptr[dirlen-1]='\0';

		if (!strcmp(dirptr,".") || !strcmp(dirptr,".."))
			continue;

		extractDirectoryName(
			s_playermodel.modelnames[s_playermodel.selectedmodel] + (int)strlen("models/players/"), 
			modelselected, 
			sizeof(modelselected)
		);

		bfpnumber = atoi(dirptr + 3);

		// set to these shaders if the directory name doesn't contain "-"
		if (strchr(dirptr, '-') == NULL) bfpnumber = 6;

		// setting ki attack pics
		switch( bfpnumber ) {
			case 2:
				s_playermodel.kipics[1].shader = trap_R_RegisterShaderNoMip( "icons/homingball" );
				s_playermodel.kipics[2].shader = trap_R_RegisterShaderNoMip( "icons/lstorm_icon" );
				s_playermodel.kipics[3].shader = trap_R_RegisterShaderNoMip( "icons/mantisblast" );
				s_playermodel.kipics[4].shader = trap_R_RegisterShaderNoMip( "icons/aga" );
				break;
			case 3:
				s_playermodel.kipics[1].shader = trap_R_RegisterShaderNoMip( "icons/tornadoblast" );
				s_playermodel.kipics[2].shader = trap_R_RegisterShaderNoMip( "icons/redattack_icon" );
				s_playermodel.kipics[3].shader = trap_R_RegisterShaderNoMip( "icons/earthseeker_icon" );
				s_playermodel.kipics[4].shader = trap_R_RegisterShaderNoMip( "icons/powerwaveblast" );
				break;
			case 4:
				s_playermodel.kipics[1].shader = trap_R_RegisterShaderNoMip( "icons/blindingflash" );
				s_playermodel.kipics[2].shader = trap_R_RegisterShaderNoMip( "icons/chaosorb_icon" );
				s_playermodel.kipics[3].shader = trap_R_RegisterShaderNoMip( "icons/homingspecial" );
				s_playermodel.kipics[4].shader = trap_R_RegisterShaderNoMip( "icons/razordisk" );
				break;
			case 5:
				s_playermodel.kipics[1].shader = trap_R_RegisterShaderNoMip( "icons/eyebeam" );
				s_playermodel.kipics[2].shader = trap_R_RegisterShaderNoMip( "icons/kistorm" );
				s_playermodel.kipics[3].shader = trap_R_RegisterShaderNoMip( "icons/superhoming" );
				s_playermodel.kipics[4].shader = trap_R_RegisterShaderNoMip( "icons/ssb" );
				break;
			default:
				s_playermodel.kipics[1].shader = trap_R_RegisterShaderNoMip( "icons/largekiblast" );
				s_playermodel.kipics[2].shader = trap_R_RegisterShaderNoMip( "icons/kistorm" );
				s_playermodel.kipics[3].shader = trap_R_RegisterShaderNoMip( "icons/impactbeam" );
				s_playermodel.kipics[4].shader = trap_R_RegisterShaderNoMip( "icons/ultimateblast" );
				break;
		}
		if ( bfpnumber <= 1 ) {
			s_playermodel.kipics[1].shader = trap_R_RegisterShaderNoMip( "icons/fingerblast" );
			s_playermodel.kipics[2].shader = trap_R_RegisterShaderNoMip( "icons/piercingblast" );
			s_playermodel.kipics[3].shader = trap_R_RegisterShaderNoMip( "icons/homingdisk" );
			s_playermodel.kipics[4].shader = trap_R_RegisterShaderNoMip( "icons/deathball" );
		}
		// the 1st ki attack is always the same
		s_playermodel.kipics[0].shader = trap_R_RegisterShaderNoMip( "icons/kiblast" );

		// stop iterating if it's this player already
		if (!Q_stricmp(dirptr, modelselected)) return;
	}
}


/*
=================
PlayerModel_PicEvent
=================
*/
static void PlayerModel_PicEvent( void* ptr, int event )
{
	int				modelnum;
	int				maxlen;
	char*			buffptr;
	char*			pdest;
	int				i;

	if (event != QM_ACTIVATED)
		return;

	for (i=0; i<PLAYERGRID_ROWS*PLAYERGRID_COLS; i++)
	{
		// reset
 		s_playermodel.pics[i].generic.flags       &= (unsigned int)~QMF_HIGHLIGHT;
 		s_playermodel.picbuttons[i].generic.flags |= QMF_PULSEIFFOCUS;
	}

	// set selected
	i = ((menucommon_s*)ptr)->id - ID_PLAYERPIC0;
	s_playermodel.pics[i].generic.flags       |= QMF_HIGHLIGHT;
	s_playermodel.picbuttons[i].generic.flags &= (unsigned int)~QMF_PULSEIFFOCUS;

	// get model and strip icon_
	modelnum = s_playermodel.modelpage*MAX_MODELSPERPAGE + i;
	buffptr  = s_playermodel.modelnames[modelnum] + (int)strlen("models/players/");
	pdest    = strstr(buffptr,"icon_");
	if (pdest)
	{
		// track the whole model/skin name
		Q_strncpyz(s_playermodel.modelskin,buffptr,pdest-buffptr+1);
		strcat(s_playermodel.modelskin,pdest + 5);

		// seperate the model name
		maxlen = pdest-buffptr;
		if (maxlen > 16)
			maxlen = 16;
		Q_strncpyz( s_playermodel.modelname.string, buffptr, maxlen );
		Q_strupr( s_playermodel.modelname.string );

		// seperate the skin name
		maxlen = (int)strlen(pdest+5)+1;
		if (maxlen > 16)
			maxlen = 16;
		Q_strncpyz( s_playermodel.skinname.string, pdest+5, maxlen );
		Q_strupr( s_playermodel.skinname.string );

		s_playermodel.selectedmodel = modelnum;

		if( trap_MemoryRemaining() > LOW_MEMORY ) {
			PlayerModel_UpdateModel();
			PlayerModel_SetKiAttacks(); // BFP - Set ki attack pics
		}
	}
}

/*
=================
PlayerModel_DrawPlayer
=================
*/
static void PlayerModel_DrawPlayer( void *self )
{
	menubitmap_s*	b;

	b = (menubitmap_s*) self;

	if( trap_MemoryRemaining() <= LOW_MEMORY ) {
		UI_DrawProportionalString( b->generic.x, b->generic.y + b->height / 2, "LOW MEMORY", UI_LEFT, color_white ); // BFP - modified LOW MEMORY warning color
		return;
	}

	UI_DrawPlayer( b->generic.x, b->generic.y, b->width, b->height, &s_playermodel.playerinfo, uis.realtime/2 );
}

/*
=================
PlayerModel_BuildList
=================
*/
static void PlayerModel_BuildList( void )
{
	int		numdirs;
	int		numfiles;
	char	dirlist[2048];
	char	filelist[2048];
	char	skinname[64];
	char*	dirptr;
	char*	fileptr;
	int		i;
	int		j;
	int		dirlen;
	int		filelen;
	qboolean precache;

	precache = trap_Cvar_VariableValue("com_buildscript");

	s_playermodel.modelpage = 0;
	s_playermodel.nummodels = 0;

	// iterate directory of all player models
	numdirs = trap_FS_GetFileList("models/players", "/", dirlist, 2048 );
	dirptr  = dirlist;
	for (i=0; i<numdirs && s_playermodel.nummodels < MAX_PLAYERMODELS; i++,dirptr+=dirlen+1)
	{
		dirlen = (int)strlen(dirptr);
		
		if (dirlen && dirptr[dirlen-1]=='/') dirptr[dirlen-1]='\0';

		if (!strcmp(dirptr,".") || !strcmp(dirptr,".."))
			continue;
			
		// iterate all skin files in directory
		numfiles = trap_FS_GetFileList( va("models/players/%s",dirptr), "tga", filelist, 2048 );
		fileptr  = filelist;
		for (j=0; j<numfiles && s_playermodel.nummodels < MAX_PLAYERMODELS;j++,fileptr+=filelen+1)
		{
			filelen = (int)strlen(fileptr);

			COM_StripExtension(fileptr,skinname);

			// look for icon_????
			if (!Q_stricmpn(skinname,"icon_",5))
			{
				Com_sprintf( s_playermodel.modelnames[s_playermodel.nummodels++],
					sizeof( s_playermodel.modelnames[s_playermodel.nummodels] ),
					"models/players/%s/%s", dirptr, skinname );
				//if (s_playermodel.nummodels >= MAX_PLAYERMODELS)
				//	return;
			}

			if( precache ) {
				trap_S_RegisterSound( va( "sound/player/announce/%s_wins.wav", skinname), qfalse );
			}
		}
	}	

	//APSFIXME - Degenerate no models case

	s_playermodel.numpages = s_playermodel.nummodels/MAX_MODELSPERPAGE;
	if (s_playermodel.nummodels % MAX_MODELSPERPAGE)
		s_playermodel.numpages++;
}

/*
=================
PlayerModel_SetMenuItems
=================
*/
static void PlayerModel_SetMenuItems( void )
{
	int				i;
	int				maxlen;
	char			modelskin[64];
	char*			buffptr;
	char*			pdest;

	// name
	Q_strncpyz( s_playermodel.name.field.buffer, UI_Cvar_VariableString( "name" ), sizeof(s_playermodel.name.field.buffer) ); // BFP - Player name string

	// model
	trap_Cvar_VariableStringBuffer( "model", s_playermodel.modelskin, 64 );
	
	// find model in our list
	for (i=0; i<s_playermodel.nummodels; i++)
	{
		// strip icon_
		buffptr  = s_playermodel.modelnames[i] + (int)strlen("models/players/");
		pdest    = strstr(buffptr,"icon_");
		if (pdest)
		{
			Q_strncpyz(modelskin,buffptr,pdest-buffptr+1);
			strcat(modelskin,pdest + 5);
		}
		else
			continue;

		if (!Q_stricmp( s_playermodel.modelskin, modelskin ))
		{
			// found pic, set selection here		
			s_playermodel.selectedmodel = i;
			s_playermodel.modelpage     = i/MAX_MODELSPERPAGE;

			// seperate the model name
			maxlen = pdest-buffptr;
			if (maxlen > 16)
				maxlen = 16;
			Q_strncpyz( s_playermodel.modelname.string, buffptr, maxlen );
			Q_strupr( s_playermodel.modelname.string );

			// seperate the skin name
			maxlen = (int)strlen(pdest+5)+1;
			if (maxlen > 16)
				maxlen = 16;
			Q_strncpyz( s_playermodel.skinname.string, pdest+5, maxlen );
			Q_strupr( s_playermodel.skinname.string );
			break;
		}
	}
}

/*
=================
PlayerModel_MenuInit
=================
*/
static void PlayerModel_MenuInit( void )
{
	int			i;
	int			j;
	int			k;
	int			x;
	int			y;
	static char	modelname[32];
	static char	skinname[32];

	// zero set all our globals
	memset( &s_playermodel, 0 ,sizeof(playermodel_t) );

	PlayerModel_Cache();

	s_playermodel.menu.key        = PlayerModel_MenuKey;
	s_playermodel.menu.wrapAround = qtrue;
	s_playermodel.menu.fullscreen = qtrue;

	// BFP - Menu background
	s_playermodel.menubg.generic.type	= MTYPE_BITMAP;
	s_playermodel.menubg.generic.name	= ART_MENUBG;
	s_playermodel.menubg.generic.flags	= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_playermodel.menubg.generic.x		= 0;
	s_playermodel.menubg.generic.y		= 0;
	s_playermodel.menubg.width			= 640;
	s_playermodel.menubg.height			= 480;

	// BFP - barlog
	s_playermodel.barlog.generic.type	= MTYPE_BITMAP;
	s_playermodel.barlog.generic.name	= ART_BARLOG;
	s_playermodel.barlog.generic.flags	= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_playermodel.barlog.generic.x		= 140;
	s_playermodel.barlog.generic.y		= 5;
	s_playermodel.barlog.width			= 355;
	s_playermodel.barlog.height			= 90;

	s_playermodel.banner.generic.type  = MTYPE_PTEXT; // BFP - modified banner type
	s_playermodel.banner.generic.flags = QMF_LEFT_JUSTIFY|QMF_INACTIVE; // BFP - modified banner flags
	s_playermodel.banner.generic.x     = 320;
	s_playermodel.banner.generic.y     = 45; // BFP - modified banner y position
	s_playermodel.banner.string        = "PLAYER SETTINGS"; // BFP - before it was called as "PLAYER MODEL"
	s_playermodel.banner.color         = color_white;
	s_playermodel.banner.style         = UI_CENTER|UI_BIGFONT; // BFP - modified banner style

	// BFP - Name textinput
	s_playermodel.name.generic.type			= MTYPE_FIELD;
	s_playermodel.name.generic.flags		= QMF_NODEFAULTINIT;
	s_playermodel.name.generic.ownerdraw	= PlayerModel_DrawName;
	s_playermodel.name.field.widthInChars	= MAX_NAMELENGTH;
	s_playermodel.name.field.maxchars		= MAX_NAMELENGTH;
	s_playermodel.name.generic.x			= 40;
	s_playermodel.name.generic.y			= 80;
	s_playermodel.name.generic.left			= 150 - 8;
	s_playermodel.name.generic.top			= 85 - 8;
	s_playermodel.name.generic.right		= 150 + 200;
	s_playermodel.name.generic.bottom		= 85 + 2 * PROP_HEIGHT;

	y =	110;
	for (i=0,k=0; i<PLAYERGRID_ROWS; i++)
	{
		x =	15; // BFP - modified rows horizontal position, before 50
		for (j=0; j<PLAYERGRID_COLS; j++,k++)
		{
			s_playermodel.pics[k].generic.type	   = MTYPE_BITMAP;
			s_playermodel.pics[k].generic.flags    = QMF_LEFT_JUSTIFY|QMF_INACTIVE;
			s_playermodel.pics[k].generic.x		   = x;
			s_playermodel.pics[k].generic.y		   = y;
			s_playermodel.pics[k].width  		   = 64;
			s_playermodel.pics[k].height  		   = 64;
			s_playermodel.pics[k].focuspic         = MODEL_SELECTED;
			s_playermodel.pics[k].focuscolor       = colorRed;

			s_playermodel.picbuttons[k].generic.type	 = MTYPE_BITMAP;
			s_playermodel.picbuttons[k].generic.flags    = QMF_LEFT_JUSTIFY|QMF_NODEFAULTINIT|QMF_PULSEIFFOCUS;
			s_playermodel.picbuttons[k].generic.id	     = ID_PLAYERPIC0+k;
			s_playermodel.picbuttons[k].generic.callback = PlayerModel_PicEvent;
			s_playermodel.picbuttons[k].generic.x    	 = x - 16;
			s_playermodel.picbuttons[k].generic.y		 = y - 16;
			s_playermodel.picbuttons[k].generic.left	 = x;
			s_playermodel.picbuttons[k].generic.top		 = y;
			s_playermodel.picbuttons[k].generic.right	 = x + 64;
			s_playermodel.picbuttons[k].generic.bottom   = y + 64;
			s_playermodel.picbuttons[k].width  		     = 128;
			s_playermodel.picbuttons[k].height  		 = 128;
			s_playermodel.picbuttons[k].focuspic  		 = MODEL_SELECT;
			s_playermodel.picbuttons[k].focuscolor  	 = colorRed;

			x += 64+6;
		}
		y += 64+6;
	}

	// BFP - Draw ki attacks of the selected player
	y =	150;
	for (i=0; i<MAX_KIATTACKS; i++)
	{
		s_playermodel.kipics[i].generic.type	 = MTYPE_BITMAP;
		s_playermodel.kipics[i].generic.flags	 = QMF_LEFT_JUSTIFY|QMF_INACTIVE;
		s_playermodel.kipics[i].generic.x		 = 400;
		s_playermodel.kipics[i].generic.y		 = y;
		s_playermodel.kipics[i].width			 = 79;
		s_playermodel.kipics[i].height			 = 34;
		y += 64-24;
	}

	s_playermodel.modelname.generic.type  = MTYPE_PTEXT;
	s_playermodel.modelname.generic.flags = QMF_CENTER_JUSTIFY|QMF_INACTIVE;
	s_playermodel.modelname.generic.x	  = 497;
	s_playermodel.modelname.generic.y	  = 54;
	s_playermodel.modelname.string	      = modelname;
	s_playermodel.modelname.style		  = UI_CENTER;
	s_playermodel.modelname.color         = text_color_normal;

	s_playermodel.skinname.generic.type   = MTYPE_PTEXT;
	s_playermodel.skinname.generic.flags  = QMF_CENTER_JUSTIFY|QMF_INACTIVE;
	s_playermodel.skinname.generic.x	  = 497;
	s_playermodel.skinname.generic.y	  = 394;
	s_playermodel.skinname.string	      = skinname;
	s_playermodel.skinname.style		  = UI_CENTER;
	s_playermodel.skinname.color          = text_color_normal;

	s_playermodel.player.generic.type      = MTYPE_BITMAP;
	s_playermodel.player.generic.flags     = QMF_INACTIVE;
	s_playermodel.player.generic.ownerdraw = PlayerModel_DrawPlayer;
	s_playermodel.player.generic.x	       = 400;
	s_playermodel.player.generic.y	       = -40;
	s_playermodel.player.width	           = 32*10;
	s_playermodel.player.height            = 56*10;

	s_playermodel.arrows.generic.type		= MTYPE_BITMAP;
	s_playermodel.arrows.generic.name		= MODEL_ARROWS;
	s_playermodel.arrows.generic.flags		= QMF_INACTIVE;
	s_playermodel.arrows.generic.x			= 260; // BFP - modified arrows x position
	s_playermodel.arrows.generic.y			= 390; // BFP - modified arrows y position
	s_playermodel.arrows.width				= 128;
	s_playermodel.arrows.height				= 32;

	s_playermodel.left.generic.type			= MTYPE_BITMAP;
	s_playermodel.left.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_playermodel.left.generic.callback		= PlayerModel_MenuEvent;
	s_playermodel.left.generic.id			= ID_PREVPAGE;
	s_playermodel.left.generic.x			= 260; // BFP - modified PREVPAGE x position
	s_playermodel.left.generic.y			= 390; // BFP - modified PREVPAGE y position
	s_playermodel.left.width  				= 64;
	s_playermodel.left.height  				= 32;
	s_playermodel.left.focuspic				= MODEL_ARROWSL;

	s_playermodel.right.generic.type	    = MTYPE_BITMAP;
	s_playermodel.right.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_playermodel.right.generic.callback	= PlayerModel_MenuEvent;
	s_playermodel.right.generic.id			= ID_NEXTPAGE;
	s_playermodel.right.generic.x			= 260+61; // BFP - modified NEXTPAGE x position
	s_playermodel.right.generic.y			= 390; // BFP - modified NEXTPAGE y position
	s_playermodel.right.width  				= 64;
	s_playermodel.right.height  		    = 32;
	s_playermodel.right.focuspic			= MODEL_ARROWSR;

	s_playermodel.back.generic.type	    = MTYPE_BITMAP;
	s_playermodel.back.generic.name     = MODEL_BACK0;
	s_playermodel.back.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_playermodel.back.generic.callback = PlayerModel_MenuEvent;
	s_playermodel.back.generic.id	    = ID_BACK;
	s_playermodel.back.generic.x		= 0;
	s_playermodel.back.generic.y		= 480-64; // BFP - modified BACK button y position
	s_playermodel.back.width  		    = 64; // BFP - modified BACK button width
	s_playermodel.back.height  		    = 64; // BFP - modified BACK button height
	s_playermodel.back.focuspic         = MODEL_BACK1;

	Menu_AddItem( &s_playermodel.menu,	&s_playermodel.menubg ); // BFP - Menu background
	Menu_AddItem( &s_playermodel.menu,	&s_playermodel.barlog ); // BFP - barlog
	Menu_AddItem( &s_playermodel.menu,	&s_playermodel.banner );
	Menu_AddItem( &s_playermodel.menu,	&s_playermodel.name ); // BFP - Name textinput
	// BFP - Model and skin names disabled
#if 0
	Menu_AddItem( &s_playermodel.menu,	&s_playermodel.modelname );
	Menu_AddItem( &s_playermodel.menu,	&s_playermodel.skinname );
#endif

	for (i=0; i<MAX_MODELSPERPAGE; i++)
	{
		Menu_AddItem( &s_playermodel.menu,	&s_playermodel.pics[i] );
		Menu_AddItem( &s_playermodel.menu,	&s_playermodel.picbuttons[i] );
	}

	Menu_AddItem( &s_playermodel.menu,	&s_playermodel.player );
	Menu_AddItem( &s_playermodel.menu,	&s_playermodel.arrows );
	Menu_AddItem( &s_playermodel.menu,	&s_playermodel.left );
	Menu_AddItem( &s_playermodel.menu,	&s_playermodel.right );
	Menu_AddItem( &s_playermodel.menu,	&s_playermodel.back );

#if 1
	for (i=0; i<MAX_KIATTACKS; i++)
	{
		Menu_AddItem( &s_playermodel.menu,	&s_playermodel.kipics[i] );
	}
#endif
	// find all available models
//	PlayerModel_BuildList();

	// set initial states
	PlayerModel_SetMenuItems();

	// update user interface
	PlayerModel_UpdateGrid();
	PlayerModel_UpdateModel();
	PlayerModel_SetKiAttacks(); // BFP - Set ki attack pics
}

/*
=================
PlayerModel_Cache
=================
*/
void PlayerModel_Cache( void )
{
	int	i;

	for( i = 0; playermodel_artlist[i]; i++ ) {
		trap_R_RegisterShaderNoMip( playermodel_artlist[i] );
	}

	PlayerModel_BuildList();
	for( i = 0; i < s_playermodel.nummodels; i++ ) {
		trap_R_RegisterShaderNoMip( s_playermodel.modelnames[i] );
	}
}

void UI_PlayerModelMenu(void)
{
	PlayerModel_MenuInit();

	UI_PushMenu( &s_playermodel.menu );

	Menu_SetCursorToItem( &s_playermodel.menu, &s_playermodel.pics[s_playermodel.selectedmodel % MAX_MODELSPERPAGE] );
}


