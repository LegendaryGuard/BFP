/*
=======================================================================

BFP OPTIONS MENU

=======================================================================
*/


#include "ui_local.h"


#define ART_BACK0				"menu/art/back_0"
#define ART_BACK1				"menu/art/back_1"
#define ART_MENUBG				"menu/art/menubg"
#define ART_BARLOG				"menu/art/cap_barlog"

#define BFPOPTIONS_X_POS		450
#define	BFPOPTIONS_SECTION_Y	(BIGCHAR_HEIGHT * 2)

#define ID_AURATYPE			138
#define ID_EXPLOTYPE		139
#define ID_VIEWPOINT		140
#define ID_FIX3PERSON       141
#define ID_PARTICLESFX      142
#define ID_DYNAURALIGHT     143
#define ID_DYNEXPLOLIGHT    144
#define ID_KITRAILENGTH     145
#define ID_BEAMCMPXY        146
#define ID_TRANSFORMATIONAURA        147
#define ID_SMALLAURA        148
#define ID_ULTIMAPERMAGLOW  149
#define ID_ACCUCROSSHAIR    150
#define ID_SIMPLEHUD        151
#define ID_CHARGEALERT      152
#define ID_Q3HITSFX         153
#define ID_FLIGHTILT        154
#define ID_BIGHEADS         155
#define ID_DEFAULTSKINS    	156
#define ID_STFU             157
#define ID_LOWPOLYSPHERE 	158
#define ID_BIGEXPLOSIONS    159
#define ID_EXPLOSIONSHELL   160
#define ID_EXPLOSIONSMOKE   161
#define ID_EXPLOSIONRING    162
#define ID_BACK				163
#define ID_AURASCONFIG		164
#define ID_EXPLOSIONSCONFIG	165
#define ID_VIEWEFFSNDCONFIG	166

// Macros to handle the cases in that order
#define SPRITE_AURA         0
#define LIGHTWEIGHT_AURA    1
#define POLYGON_AURA        2
#define HIGHPOLYCOUNT_AURA  3
#define PARTICLE_AURA       4
#define SHADER_AURA         5

#define WIMPY_EXPLO         0
#define WEAK_EXPLO          1
#define SO_SO_EXPLO         2
#define HARDCORE_EXPLO      3

static const char *auratype_items[] = {
	"Sprite Aura",
	"Lightweight Aura",
	"Polygonal Aura",
	"High Polycount Aura",
	"Particle Aura (Particle Effects only)",
	"Shader Aura",
	NULL
};

static const char *viewpoint_items[] = {
	"Third Person",
	"First Person",
	"First Person Vis Mode",
	NULL
};

static const char* explotype_items[] = {
	"Wimpy",
	"Weak",
	"So-So",
	"Hardcore",
	NULL
};

typedef struct {
	menuframework_s		menu;
	menubitmap_s		menubg;
	menubitmap_s		barlog;
	menutext_s			banner;

	menutext_s			aurasButton;
	menutext_s			explosionsButton;
	menutext_s			viewEffectsSoundsButton;

	menulist_s			auratype;
	menulist_s			explotype;
	menulist_s			viewpoint;
	menuradiobutton_s	fix3person;
	menuradiobutton_s	particlesfx;
	menuradiobutton_s	dynauralight;
	menuradiobutton_s	dynamiclights;
	menuradiobutton_s	dynexplolights;
	menuradiobutton_s	bigexplosions;
	menuradiobutton_s	explosionshell;
	menuradiobutton_s	explosionsmoke;
	menuradiobutton_s	explosionring;
	menuslider_s		kitrailength;
	menuslider_s		beamcmpxy;
	menuradiobutton_s	transaura;
	menuradiobutton_s	smallaura;
	menuradiobutton_s	ultpermaglow;
	menuradiobutton_s	accucrosshair;
	menuradiobutton_s	simplehud;
	menuradiobutton_s	chargealert;
	menuradiobutton_s	q3hitsfx;
	menuradiobutton_s	flightilt;
	menuradiobutton_s	bigheads;
	menuradiobutton_s	defaultskins;
	menuradiobutton_s	stfu;
	menuradiobutton_s	lowpolysphere;
	menubitmap_s		back;
} bfpoptions_t;

static bfpoptions_t s_bfpoptions;

static int menuBarOption = ID_AURASCONFIG;

void BFPAuraOptions_MenuInit( void );
void BFPExplosionsOptions_MenuInit( void );
void BFPViewEffSndsOptions_MenuInit( void );

static void BFPOptions_MenuItem( int *menu_item_curvalue, const char *cvar, int value ) {
	*menu_item_curvalue = trap_Cvar_VariableValue( cvar ) != value;
}

static void BFPOptions_SetMenuItems( void ) {
	BFPOptions_MenuItem( &s_bfpoptions.fix3person.curvalue,     "cg_fixedThirdPerson",   0 );
	BFPOptions_MenuItem( &s_bfpoptions.particlesfx.curvalue,    "cg_particles",          0 );
	BFPOptions_MenuItem( &s_bfpoptions.dynauralight.curvalue,   "cg_lightAuras",         0 );
	BFPOptions_MenuItem( &s_bfpoptions.dynexplolights.curvalue, "cg_lightExplosions",    0 );
	BFPOptions_MenuItem( &s_bfpoptions.bigexplosions.curvalue,  "cg_bigExplosions",      0 );
	BFPOptions_MenuItem( &s_bfpoptions.explosionshell.curvalue, "cg_explosionShell",     0 );
	BFPOptions_MenuItem( &s_bfpoptions.explosionsmoke.curvalue, "cg_explosionSmoke",     0 );
	BFPOptions_MenuItem( &s_bfpoptions.explosionring.curvalue,  "cg_explosionRing",      0 );
	BFPOptions_MenuItem( &s_bfpoptions.transaura.curvalue,      "cg_transformationAura", 0 );
	BFPOptions_MenuItem( &s_bfpoptions.smallaura.curvalue,      "cg_smallOwnAura",       0 );
	BFPOptions_MenuItem( &s_bfpoptions.ultpermaglow.curvalue,   "cg_permaglowUltimate",  0 );
	BFPOptions_MenuItem( &s_bfpoptions.accucrosshair.curvalue,  "cg_stableCrosshair",    1 ); // doesn't make sense if the crosshair isn't accurate
	BFPOptions_MenuItem( &s_bfpoptions.simplehud.curvalue,      "cg_simpleHUD",          0 );
	BFPOptions_MenuItem( &s_bfpoptions.chargealert.curvalue,    "cg_chargeupAlert",      0 );
	BFPOptions_MenuItem( &s_bfpoptions.q3hitsfx.curvalue,       "cg_playHitSound",       0 );
	BFPOptions_MenuItem( &s_bfpoptions.flightilt.curvalue,      "cg_flytilt",            0 );
	BFPOptions_MenuItem( &s_bfpoptions.bigheads.curvalue,       "cg_superdeformed",      0 );
	BFPOptions_MenuItem( &s_bfpoptions.defaultskins.curvalue,   "cg_forceSkin",          0 );
	BFPOptions_MenuItem( &s_bfpoptions.stfu.curvalue,           "cg_stfu",               0 );
	BFPOptions_MenuItem( &s_bfpoptions.lowpolysphere.curvalue,  "cg_lowpolysphere",      0 );
}

static void BFPOptions_AuraType_Setup( int sprite, int highpoly, int poly, int light, int particle ) {
	trap_Cvar_SetValue( "cg_spriteAura", sprite );
	trap_Cvar_SetValue( "cg_highPolyAura", highpoly );
	trap_Cvar_SetValue( "cg_polygonAura", poly );
	trap_Cvar_SetValue( "cg_lightweightAuras", light );
	trap_Cvar_SetValue( "cg_particleAura", particle );
}

static void BFPOptions_Viewpoint_Setup( int tp, int ownmodel ) {
	trap_Cvar_SetValue( "cg_thirdPerson", tp );
	trap_Cvar_SetValue( "cg_drawOwnModel", ownmodel );
}

static void BFPOptions_ExploType_Setup( int expShell, int expSmoke, int particles, int expRing ) {
	trap_Cvar_SetValue( "cg_explosionShell", expShell );
	trap_Cvar_SetValue( "cg_explosionSmoke", expSmoke );
	trap_Cvar_SetValue( "cg_explosionRing", expRing );
	trap_Cvar_SetValue( "cg_particles", particles );
	s_bfpoptions.explosionshell.curvalue = expShell;
	s_bfpoptions.explosionsmoke.curvalue = expSmoke;
	s_bfpoptions.explosionring.curvalue = expRing;
	s_bfpoptions.particlesfx.curvalue = particles;
}

static void BFPOptions_ExplosionsTypeCheck( void ) {
	int particles = s_bfpoptions.particlesfx.curvalue;
	int explosionSmoke = s_bfpoptions.explosionsmoke.curvalue;
	int explosionShell = s_bfpoptions.explosionshell.curvalue;
	int explosionRing = s_bfpoptions.explosionring.curvalue;

	if ( particles <= 0 && explosionSmoke <= 0 && explosionShell <= 0 && explosionRing <= 0 ) {
		s_bfpoptions.explotype.curvalue = WIMPY_EXPLO;
	}
	if ( particles <= 0 && explosionSmoke <= 0 && explosionShell >= 1 && explosionRing >= 1 ) {
		s_bfpoptions.explotype.curvalue = WEAK_EXPLO;
	}
	if ( particles >= 1 && explosionSmoke <= 0 && explosionShell >= 1 && explosionRing >= 1 ) {
		s_bfpoptions.explotype.curvalue = SO_SO_EXPLO;
	}
	if ( particles >= 1 && explosionSmoke >= 1 && explosionShell >= 1 && explosionRing >= 1 ) {
		s_bfpoptions.explotype.curvalue = HARDCORE_EXPLO;
	}
}

static void BFPOptions_Event( void* ptr, int notification ) {
	if ( notification != QM_ACTIVATED ) {
		return;
	}

	switch ( ((menucommon_s*)ptr)->id ) {
		//---------------------------Show options---------------------------------//
	case ID_AURASCONFIG:
		if ( menuBarOption != ID_AURASCONFIG ) {
			menuBarOption = ID_AURASCONFIG;
			BFPAuraOptions_MenuInit();
		}
		break;

	case ID_EXPLOSIONSCONFIG:
		if ( menuBarOption != ID_EXPLOSIONSCONFIG ) {
			menuBarOption = ID_EXPLOSIONSCONFIG;
			BFPExplosionsOptions_MenuInit();
		}
		break;

	case ID_VIEWEFFSNDCONFIG:
		if ( menuBarOption != ID_VIEWEFFSNDCONFIG ) {
			menuBarOption = ID_VIEWEFFSNDCONFIG;
			BFPViewEffSndsOptions_MenuInit();
		}
		break;

		//-----------------------------Aura list---------------------------------//

	case ID_AURATYPE:
		switch ( s_bfpoptions.auratype.curvalue ) {
		case SPRITE_AURA:
			BFPOptions_AuraType_Setup( 1, 0, 0, 0, 0 );
			break;

		case LIGHTWEIGHT_AURA:
			BFPOptions_AuraType_Setup( 0, 0, 0, 1, 0 );
			break;

		case POLYGON_AURA:
			BFPOptions_AuraType_Setup( 0, 0, 1, 0, 0 );
			break;

		case HIGHPOLYCOUNT_AURA:
			BFPOptions_AuraType_Setup( 0, 1, 1, 0, 0 );
			break;

		case PARTICLE_AURA:
			BFPOptions_AuraType_Setup( 0, 0, 0, 0, 1 );
			break;

		case SHADER_AURA:
			BFPOptions_AuraType_Setup( 0, 0, 0, 0, 0 );
			break;
		}
		break;

		//---------------------------View point List---------------------------------------//

	case ID_VIEWPOINT:
		switch ( s_bfpoptions.viewpoint.curvalue ) {
		case 0: // Third Person
			trap_Cvar_SetValue( "cg_thirdPerson", 1 );
			break;

		case 1: // First Person
			BFPOptions_Viewpoint_Setup( 0, 0 );
			break;

		case 2: // First Person Vis Mode
			BFPOptions_Viewpoint_Setup( 0, 1 );
			break;
		}
		break;

		//---------------------------Explosion type list---------------------------------------//

	case ID_EXPLOTYPE:
		switch ( s_bfpoptions.explotype.curvalue ) {
		case WIMPY_EXPLO: // Wimpy
			BFPOptions_ExploType_Setup( 0, 0, 0, 0 );
			break;

		case WEAK_EXPLO: // Weak
			BFPOptions_ExploType_Setup( 1, 0, 0, 1 );
			break;

		case SO_SO_EXPLO: // So-So
			BFPOptions_ExploType_Setup( 1, 0, 1, 1 );
			break;

		case HARDCORE_EXPLO: // Hardcore
			BFPOptions_ExploType_Setup( 1, 1, 1, 1 );
			break;
		}
		break;

		//---------------------------------------------------------------------//

	case ID_FIX3PERSON:
		trap_Cvar_SetValue( "cg_fixedThirdPerson", s_bfpoptions.fix3person.curvalue );
		break;

	case ID_PARTICLESFX:
		trap_Cvar_SetValue( "cg_particles", s_bfpoptions.particlesfx.curvalue );
		BFPOptions_ExplosionsTypeCheck();
		break;
	
	case ID_DYNAURALIGHT:
		trap_Cvar_SetValue( "cg_lightAuras", s_bfpoptions.dynauralight.curvalue );
		break;
	
	case ID_DYNEXPLOLIGHT:
		trap_Cvar_SetValue( "cg_lightExplosions", s_bfpoptions.dynexplolights.curvalue );
		break;

	case ID_BIGEXPLOSIONS:
		trap_Cvar_SetValue( "cg_bigExplosions", s_bfpoptions.bigexplosions.curvalue );
		break;
	
	case ID_EXPLOSIONSHELL:
		trap_Cvar_SetValue( "cg_explosionShell", s_bfpoptions.explosionshell.curvalue );
		BFPOptions_ExplosionsTypeCheck();
		break;
	
	case ID_EXPLOSIONSMOKE:
		trap_Cvar_SetValue( "cg_explosionSmoke", s_bfpoptions.explosionsmoke.curvalue );
		BFPOptions_ExplosionsTypeCheck();
		break;
	
	case ID_EXPLOSIONRING:
		trap_Cvar_SetValue( "cg_explosionRing", s_bfpoptions.explosionring.curvalue );
		BFPOptions_ExplosionsTypeCheck();
		break;


		//----------------------------Sliders------------------------------------//


	case ID_KITRAILENGTH:
		trap_Cvar_SetValue( "cg_kiTrail", s_bfpoptions.kitrailength.curvalue  );
		break;

	case ID_BEAMCMPXY:
		trap_Cvar_SetValue( "cg_beamTrail", s_bfpoptions.beamcmpxy.curvalue  );
		break;
	
		//-----------------------------------------------------------------------//


	case ID_TRANSFORMATIONAURA:
		trap_Cvar_SetValue( "cg_transformationAura", s_bfpoptions.transaura.curvalue );
		break;
	
	case ID_SMALLAURA:
		trap_Cvar_SetValue( "cg_smallOwnAura", s_bfpoptions.smallaura.curvalue );
		break;

	case ID_ULTIMAPERMAGLOW:
		trap_Cvar_SetValue( "cg_permaglowUltimate", s_bfpoptions.ultpermaglow.curvalue );
		break;

	case ID_ACCUCROSSHAIR:
		trap_Cvar_SetValue( "cg_stableCrosshair", s_bfpoptions.accucrosshair.curvalue ? 0 : 1 ); // doesn't make sense if the crosshair isn't accurate
		break;

	case ID_SIMPLEHUD:
		trap_Cvar_SetValue( "cg_simpleHUD", s_bfpoptions.simplehud.curvalue );
		break;

	case ID_CHARGEALERT:
		trap_Cvar_SetValue( "cg_chargeupAlert", s_bfpoptions.chargealert.curvalue );
		break;

	case ID_Q3HITSFX:
		trap_Cvar_SetValue( "cg_playHitSound", s_bfpoptions.q3hitsfx.curvalue );
		break;

	case ID_FLIGHTILT:
		trap_Cvar_SetValue( "cg_flytilt", s_bfpoptions.flightilt.curvalue );
		break;

	case ID_BIGHEADS:
		trap_Cvar_SetValue( "cg_superdeformed", s_bfpoptions.bigheads.curvalue );
		break;

	case ID_DEFAULTSKINS:
		trap_Cvar_SetValue( "cg_forceSkin", s_bfpoptions.defaultskins.curvalue );
		break;

	case ID_STFU:
		trap_Cvar_SetValue( "cg_stfu", s_bfpoptions.stfu.curvalue );
		break;

	case ID_LOWPOLYSPHERE:
		trap_Cvar_SetValue( "cg_lowpolysphere", s_bfpoptions.lowpolysphere.curvalue );
		break;

	case ID_BACK:
		UI_PopMenu();
		break;
	}
}


static void BFPBarBackground_MenuSet( void ) {
	s_bfpoptions.menubg.generic.type		= MTYPE_BITMAP;
	s_bfpoptions.menubg.generic.name		= ART_MENUBG;
	s_bfpoptions.menubg.generic.flags		= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_bfpoptions.menubg.generic.x			= 0;
	s_bfpoptions.menubg.generic.y			= 0;
	s_bfpoptions.menubg.width				= 640;
	s_bfpoptions.menubg.height				= 480;

	s_bfpoptions.barlog.generic.type		= MTYPE_BITMAP;
	s_bfpoptions.barlog.generic.name		= ART_BARLOG;
	s_bfpoptions.barlog.generic.flags		= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_bfpoptions.barlog.generic.x			= 140;
	s_bfpoptions.barlog.generic.y			= 5;
	s_bfpoptions.barlog.width				= 355;
	s_bfpoptions.barlog.height				= 90;

	s_bfpoptions.banner.generic.type		= MTYPE_PTEXT;
	s_bfpoptions.banner.generic.flags		= QMF_CENTER_JUSTIFY|QMF_INACTIVE;
	s_bfpoptions.banner.generic.x 			= 320;
	s_bfpoptions.banner.generic.y 			= 45;
	s_bfpoptions.banner.string				= "BFP OPTIONS";
	s_bfpoptions.banner.color				= color_white;
	s_bfpoptions.banner.style				= UI_CENTER|UI_BIGFONT;

	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.menubg );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.barlog );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.banner );
}

static void BFPButtonOptions_MenuSet( void ) {
	BFPBarBackground_MenuSet();

	s_bfpoptions.aurasButton.generic.type		= MTYPE_PTEXT;
	s_bfpoptions.aurasButton.generic.flags		= ( menuBarOption == ID_AURASCONFIG ) ? QMF_RIGHT_JUSTIFY : (QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS);
	s_bfpoptions.aurasButton.generic.id			= ID_AURASCONFIG;
	s_bfpoptions.aurasButton.generic.callback	= BFPOptions_Event;
	s_bfpoptions.aurasButton.generic.x			= 216;
	s_bfpoptions.aurasButton.generic.y			= 240 - 2 * PROP_HEIGHT;
	s_bfpoptions.aurasButton.string				= "AURAS";
	s_bfpoptions.aurasButton.style				= UI_RIGHT;
	s_bfpoptions.aurasButton.color				= color_white;
	
	s_bfpoptions.explosionsButton.generic.type		= MTYPE_PTEXT;
	s_bfpoptions.explosionsButton.generic.flags		= ( menuBarOption == ID_EXPLOSIONSCONFIG ) ? QMF_RIGHT_JUSTIFY : (QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS);
	s_bfpoptions.explosionsButton.generic.id		= ID_EXPLOSIONSCONFIG;
	s_bfpoptions.explosionsButton.generic.callback	= BFPOptions_Event;
	s_bfpoptions.explosionsButton.generic.x			= 216;
	s_bfpoptions.explosionsButton.generic.y			= 240 - PROP_HEIGHT;
	s_bfpoptions.explosionsButton.string			= "EXPLOSIONS";
	s_bfpoptions.explosionsButton.style				= UI_RIGHT;
	s_bfpoptions.explosionsButton.color				= color_white;

	s_bfpoptions.viewEffectsSoundsButton.generic.type			= MTYPE_PTEXT;
	s_bfpoptions.viewEffectsSoundsButton.generic.flags			= ( menuBarOption == ID_VIEWEFFSNDCONFIG ) ? QMF_RIGHT_JUSTIFY : (QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS);
	s_bfpoptions.viewEffectsSoundsButton.generic.id				= ID_VIEWEFFSNDCONFIG;
	s_bfpoptions.viewEffectsSoundsButton.generic.callback		= BFPOptions_Event;
	s_bfpoptions.viewEffectsSoundsButton.generic.x				= 216;
	s_bfpoptions.viewEffectsSoundsButton.generic.y				= 240;
	s_bfpoptions.viewEffectsSoundsButton.string					= "VIEW & FX";
	s_bfpoptions.viewEffectsSoundsButton.style					= UI_RIGHT;
	s_bfpoptions.viewEffectsSoundsButton.color					= color_white;

	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.aurasButton );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.explosionsButton );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.viewEffectsSoundsButton );
}


void BFPAuraOptions_MenuInit( void ) {
	int		y;
	int		highpolyaura, polygonalaura, lightweightaura, spriteaura, particleaura;

	memset( &s_bfpoptions, 0, sizeof(bfpoptions_t) );

	BFPOptions_Cache();

	s_bfpoptions.menu.wrapAround = qtrue;
	s_bfpoptions.menu.fullscreen = qtrue;

	y = 240 - 3 * (BIGCHAR_HEIGHT+2);
	s_bfpoptions.auratype.generic.type		= MTYPE_SPINCONTROL;
	s_bfpoptions.auratype.generic.name		= "Aura Type:";
	s_bfpoptions.auratype.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.auratype.generic.callback	= BFPOptions_Event;
	s_bfpoptions.auratype.generic.id		= ID_AURATYPE;
	s_bfpoptions.auratype.generic.x			= BFPOPTIONS_X_POS-130;
	s_bfpoptions.auratype.generic.y			= y;
	s_bfpoptions.auratype.itemnames			= auratype_items;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.dynauralight.generic.type        = MTYPE_RADIOBUTTON;
	s_bfpoptions.dynauralight.generic.name	      = "Dynamic Aura Lights:";
	s_bfpoptions.dynauralight.generic.flags	      = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.dynauralight.generic.callback    = BFPOptions_Event;
	s_bfpoptions.dynauralight.generic.id          = ID_DYNAURALIGHT;
	s_bfpoptions.dynauralight.generic.x	          = BFPOPTIONS_X_POS;
	s_bfpoptions.dynauralight.generic.y	          = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.transaura.generic.type        = MTYPE_RADIOBUTTON;
	s_bfpoptions.transaura.generic.name		   = "Transformation Aura:";
	s_bfpoptions.transaura.generic.flags	   = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.transaura.generic.callback    = BFPOptions_Event;
	s_bfpoptions.transaura.generic.id          = ID_TRANSFORMATIONAURA;
	s_bfpoptions.transaura.generic.x	       = BFPOPTIONS_X_POS;
	s_bfpoptions.transaura.generic.y	       = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.smallaura.generic.type     = MTYPE_RADIOBUTTON;
	s_bfpoptions.smallaura.generic.name	  	= "Small Own Aura:";
	s_bfpoptions.smallaura.generic.flags    = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.smallaura.generic.callback = BFPOptions_Event;
	s_bfpoptions.smallaura.generic.id       = ID_SMALLAURA;
	s_bfpoptions.smallaura.generic.x	    = BFPOPTIONS_X_POS;
	s_bfpoptions.smallaura.generic.y	    = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.ultpermaglow.generic.type    = MTYPE_RADIOBUTTON;
	s_bfpoptions.ultpermaglow.generic.name	  = "Ultimate Perma-Glow:";
	s_bfpoptions.ultpermaglow.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.ultpermaglow.generic.callback = BFPOptions_Event;
	s_bfpoptions.ultpermaglow.generic.id      = ID_ULTIMAPERMAGLOW;
	s_bfpoptions.ultpermaglow.generic.x	      = BFPOPTIONS_X_POS;
	s_bfpoptions.ultpermaglow.generic.y	      = y;

	s_bfpoptions.back.generic.type		= MTYPE_BITMAP;
	s_bfpoptions.back.generic.name		= ART_BACK0;
	s_bfpoptions.back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_bfpoptions.back.generic.callback	= BFPOptions_Event;
	s_bfpoptions.back.generic.id	    = ID_BACK;
	s_bfpoptions.back.generic.x			= 0;
	s_bfpoptions.back.generic.y			= 480-80;
	s_bfpoptions.back.width  		    = 80;
	s_bfpoptions.back.height  		    = 80;
	s_bfpoptions.back.focuspic			= ART_BACK1;

	BFPButtonOptions_MenuSet();

	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.auratype );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.dynauralight );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.transaura );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.smallaura );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.ultpermaglow );

	highpolyaura = trap_Cvar_VariableValue( "cg_highPolyAura" );
	polygonalaura = trap_Cvar_VariableValue( "cg_polygonAura" );
	lightweightaura = trap_Cvar_VariableValue( "cg_lightweightAuras" );
	spriteaura = trap_Cvar_VariableValue( "cg_spriteAura" );
	particleaura = trap_Cvar_VariableValue( "cg_particleAura" );

	if ( highpolyaura >= 1 ) {
		s_bfpoptions.auratype.curvalue = HIGHPOLYCOUNT_AURA;
	} else if ( polygonalaura >= 1 )  {
		s_bfpoptions.auratype.curvalue = POLYGON_AURA;
	} else if ( lightweightaura >= 1 ) {
		s_bfpoptions.auratype.curvalue = LIGHTWEIGHT_AURA;
	} else if ( spriteaura >= 1 ) {
		s_bfpoptions.auratype.curvalue = SPRITE_AURA;
	} else if ( particleaura >= 1 ) {
		s_bfpoptions.auratype.curvalue = PARTICLE_AURA;
	} else {
		s_bfpoptions.auratype.curvalue = SHADER_AURA;
	}

	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.back );

	BFPOptions_SetMenuItems();
}


void BFPExplosionsOptions_MenuInit( void ) {
	int		y;
	int		explosionRing, explosionShell, explosionSmoke;
	int		particles;

	memset( &s_bfpoptions, 0, sizeof(bfpoptions_t) );

	BFPOptions_Cache();

	s_bfpoptions.menu.wrapAround = qtrue;
	s_bfpoptions.menu.fullscreen = qtrue;

	y = 240 - 4 * (BIGCHAR_HEIGHT+2);
	s_bfpoptions.explotype.generic.type		= MTYPE_SPINCONTROL;
	s_bfpoptions.explotype.generic.name		= "Explosion Type:";
	s_bfpoptions.explotype.generic.flags	= QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_bfpoptions.explotype.generic.callback = BFPOptions_Event;
	s_bfpoptions.explotype.generic.id		= ID_EXPLOTYPE;
	s_bfpoptions.explotype.generic.x		= BFPOPTIONS_X_POS;
	s_bfpoptions.explotype.generic.y		= y;
	s_bfpoptions.explotype.itemnames		= explotype_items;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.bigexplosions.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.bigexplosions.generic.name		= "Big Explosions:";
	s_bfpoptions.bigexplosions.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.bigexplosions.generic.callback	= BFPOptions_Event;
	s_bfpoptions.bigexplosions.generic.id		= ID_BIGEXPLOSIONS;
	s_bfpoptions.bigexplosions.generic.x		= BFPOPTIONS_X_POS;
	s_bfpoptions.bigexplosions.generic.y		= y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.dynexplolights.generic.type        = MTYPE_RADIOBUTTON;
	s_bfpoptions.dynexplolights.generic.name	    = "Dynamic Explosion Lights:";
	s_bfpoptions.dynexplolights.generic.flags	    = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.dynexplolights.generic.callback    = BFPOptions_Event;
	s_bfpoptions.dynexplolights.generic.id          = ID_DYNEXPLOLIGHT;
	s_bfpoptions.dynexplolights.generic.x	        = BFPOPTIONS_X_POS;
	s_bfpoptions.dynexplolights.generic.y	        = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.lowpolysphere.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.lowpolysphere.generic.name		= "Low Polycount Sphere:";
	s_bfpoptions.lowpolysphere.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.lowpolysphere.generic.callback	= BFPOptions_Event;
	s_bfpoptions.lowpolysphere.generic.id		= ID_LOWPOLYSPHERE;
	s_bfpoptions.lowpolysphere.generic.x		= BFPOPTIONS_X_POS;
	s_bfpoptions.lowpolysphere.generic.y		= y;

	y += BFPOPTIONS_SECTION_Y;
	s_bfpoptions.explosionsmoke.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.explosionsmoke.generic.name		= "Smoke:";
	s_bfpoptions.explosionsmoke.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.explosionsmoke.generic.callback	= BFPOptions_Event;
	s_bfpoptions.explosionsmoke.generic.id			= ID_EXPLOSIONSMOKE;
	s_bfpoptions.explosionsmoke.generic.x			= BFPOPTIONS_X_POS;
	s_bfpoptions.explosionsmoke.generic.y			= y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.explosionshell.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.explosionshell.generic.name		= "Shell:";
	s_bfpoptions.explosionshell.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.explosionshell.generic.callback	= BFPOptions_Event;
	s_bfpoptions.explosionshell.generic.id			= ID_EXPLOSIONSHELL;
	s_bfpoptions.explosionshell.generic.x			= BFPOPTIONS_X_POS;
	s_bfpoptions.explosionshell.generic.y			= y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.explosionring.generic.type			= MTYPE_RADIOBUTTON;
	s_bfpoptions.explosionring.generic.name			= "Ring:";
	s_bfpoptions.explosionring.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.explosionring.generic.callback		= BFPOptions_Event;
	s_bfpoptions.explosionring.generic.id			= ID_EXPLOSIONRING;
	s_bfpoptions.explosionring.generic.x			= BFPOPTIONS_X_POS;
	s_bfpoptions.explosionring.generic.y			= y;

	s_bfpoptions.back.generic.type		= MTYPE_BITMAP;
	s_bfpoptions.back.generic.name		= ART_BACK0;
	s_bfpoptions.back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_bfpoptions.back.generic.callback	= BFPOptions_Event;
	s_bfpoptions.back.generic.id	    = ID_BACK;
	s_bfpoptions.back.generic.x			= 0;
	s_bfpoptions.back.generic.y			= 480-80;
	s_bfpoptions.back.width  		    = 80;
	s_bfpoptions.back.height  		    = 80;
	s_bfpoptions.back.focuspic			= ART_BACK1;

	BFPButtonOptions_MenuSet();

	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.explotype );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.bigexplosions );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.dynexplolights );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.lowpolysphere );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.explosionsmoke );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.explosionshell );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.explosionring );

	explosionSmoke = trap_Cvar_VariableValue( "cg_explosionSmoke" );
	explosionShell = trap_Cvar_VariableValue( "cg_explosionShell" );
	explosionRing = trap_Cvar_VariableValue( "cg_explosionRing" );
	particles = trap_Cvar_VariableValue( "cg_particles" );

	s_bfpoptions.bigexplosions.curvalue = trap_Cvar_VariableValue( "cg_bigExplosions" );
	s_bfpoptions.explosionsmoke.curvalue = explosionSmoke;
	s_bfpoptions.explosionshell.curvalue = explosionShell;
	s_bfpoptions.explosionring.curvalue = explosionRing;

	s_bfpoptions.particlesfx.curvalue = particles;

	BFPOptions_ExplosionsTypeCheck();

	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.back );

	BFPOptions_SetMenuItems();
}


void BFPViewEffSndsOptions_MenuInit( void ) {
	int		y;
	int		thirdperson, firstpersonvis;

	memset( &s_bfpoptions, 0, sizeof(bfpoptions_t) );

	BFPOptions_Cache();

	s_bfpoptions.menu.wrapAround = qtrue;
	s_bfpoptions.menu.fullscreen = qtrue;

	// -------------------------------View & HUD---------------------------------
	y = 240 - 7 * (BIGCHAR_HEIGHT+2);
	s_bfpoptions.viewpoint.generic.type		= MTYPE_SPINCONTROL;
	s_bfpoptions.viewpoint.generic.name		= "Viewpoint:";
	s_bfpoptions.viewpoint.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.viewpoint.generic.callback	= BFPOptions_Event;
	s_bfpoptions.viewpoint.generic.id		= ID_VIEWPOINT;
	s_bfpoptions.viewpoint.generic.x		= BFPOPTIONS_X_POS;
	s_bfpoptions.viewpoint.generic.y		= y;
	s_bfpoptions.viewpoint.itemnames		= viewpoint_items;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.fix3person.generic.type        = MTYPE_RADIOBUTTON;
	s_bfpoptions.fix3person.generic.name	    = "Fixed Third Person:";
	s_bfpoptions.fix3person.generic.flags	    = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.fix3person.generic.callback    = BFPOptions_Event;
	s_bfpoptions.fix3person.generic.id          = ID_FIX3PERSON;
	s_bfpoptions.fix3person.generic.x	        = BFPOPTIONS_X_POS;
	s_bfpoptions.fix3person.generic.y	        = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.simplehud.generic.type			= MTYPE_RADIOBUTTON;
	s_bfpoptions.simplehud.generic.name			= "Simple HUD:";
	s_bfpoptions.simplehud.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.simplehud.generic.callback		= BFPOptions_Event;
	s_bfpoptions.simplehud.generic.id			= ID_SIMPLEHUD;
	s_bfpoptions.simplehud.generic.x			= BFPOPTIONS_X_POS;
	s_bfpoptions.simplehud.generic.y			= y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.flightilt.generic.type     = MTYPE_RADIOBUTTON;
	s_bfpoptions.flightilt.generic.name		= "Flight Tilt:";
	s_bfpoptions.flightilt.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.flightilt.generic.callback = BFPOptions_Event;
	s_bfpoptions.flightilt.generic.id       = ID_FLIGHTILT;
	s_bfpoptions.flightilt.generic.x	    = BFPOPTIONS_X_POS;
	s_bfpoptions.flightilt.generic.y	    = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.accucrosshair.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.accucrosshair.generic.name		= "Accurate Crosshair:";
	s_bfpoptions.accucrosshair.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.accucrosshair.generic.callback	= BFPOptions_Event;
	s_bfpoptions.accucrosshair.generic.id		= ID_ACCUCROSSHAIR;
	s_bfpoptions.accucrosshair.generic.x		= BFPOPTIONS_X_POS;
	s_bfpoptions.accucrosshair.generic.y		= y;


	// -------------------------------Effects---------------------------------
	y += BFPOPTIONS_SECTION_Y;
	s_bfpoptions.particlesfx.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.particlesfx.generic.name		= "Particle Effects:";
	s_bfpoptions.particlesfx.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.particlesfx.generic.callback	= BFPOptions_Event;
	s_bfpoptions.particlesfx.generic.id			= ID_PARTICLESFX;
	s_bfpoptions.particlesfx.generic.x			= BFPOPTIONS_X_POS;
	s_bfpoptions.particlesfx.generic.y			= y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.kitrailength.generic.type		= MTYPE_SLIDER;
	s_bfpoptions.kitrailength.generic.name		= "Ki Trail Length:";
	s_bfpoptions.kitrailength.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.kitrailength.generic.callback	= BFPOptions_Event;
	s_bfpoptions.kitrailength.generic.id		= ID_KITRAILENGTH;
	s_bfpoptions.kitrailength.generic.x	        = BFPOPTIONS_X_POS;
	s_bfpoptions.kitrailength.generic.y	        = y;
	s_bfpoptions.kitrailength.minvalue			= 0;
	s_bfpoptions.kitrailength.maxvalue			= 100;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.beamcmpxy.generic.type		= MTYPE_SLIDER;
	s_bfpoptions.beamcmpxy.generic.name		= "Beam Complexity:";
	s_bfpoptions.beamcmpxy.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.beamcmpxy.generic.callback	= BFPOptions_Event;
	s_bfpoptions.beamcmpxy.generic.id		= ID_BEAMCMPXY;
	s_bfpoptions.beamcmpxy.generic.x	    = BFPOPTIONS_X_POS;
	s_bfpoptions.beamcmpxy.generic.y	    = y;
	s_bfpoptions.beamcmpxy.minvalue			= 0;
	s_bfpoptions.beamcmpxy.maxvalue			= 100;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.bigheads.generic.type     = MTYPE_RADIOBUTTON;
	s_bfpoptions.bigheads.generic.name	   = "Superdeformed Heads:";
	s_bfpoptions.bigheads.generic.flags	   = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.bigheads.generic.callback = BFPOptions_Event;
	s_bfpoptions.bigheads.generic.id       = ID_BIGHEADS;
	s_bfpoptions.bigheads.generic.x  	   = BFPOPTIONS_X_POS;
	s_bfpoptions.bigheads.generic.y	 	   = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.defaultskins.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.defaultskins.generic.name		= "Force Default Skins:";
	s_bfpoptions.defaultskins.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.defaultskins.generic.callback	= BFPOptions_Event;
	s_bfpoptions.defaultskins.generic.id		= ID_DEFAULTSKINS;
	s_bfpoptions.defaultskins.generic.x			= BFPOPTIONS_X_POS;
	s_bfpoptions.defaultskins.generic.y			= y;


	// -------------------------------Sounds---------------------------------
	y += BFPOPTIONS_SECTION_Y;
	s_bfpoptions.stfu.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.stfu.generic.name		= "Disable Voices:";
	s_bfpoptions.stfu.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.stfu.generic.callback	= BFPOptions_Event;
	s_bfpoptions.stfu.generic.id		= ID_STFU;
	s_bfpoptions.stfu.generic.x			= BFPOPTIONS_X_POS;
	s_bfpoptions.stfu.generic.y			= y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.chargealert.generic.type     = MTYPE_RADIOBUTTON;
	s_bfpoptions.chargealert.generic.name	  = "Chargeup Alerts:";
	s_bfpoptions.chargealert.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.chargealert.generic.callback = BFPOptions_Event;
	s_bfpoptions.chargealert.generic.id       = ID_CHARGEALERT;
	s_bfpoptions.chargealert.generic.x	      = BFPOPTIONS_X_POS;
	s_bfpoptions.chargealert.generic.y	      = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.q3hitsfx.generic.type    	= MTYPE_RADIOBUTTON;
	s_bfpoptions.q3hitsfx.generic.name	  	= "Q3 Hit Sound:";
	s_bfpoptions.q3hitsfx.generic.flags	  	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.q3hitsfx.generic.callback	= BFPOptions_Event;
	s_bfpoptions.q3hitsfx.generic.id      	= ID_Q3HITSFX;
	s_bfpoptions.q3hitsfx.generic.x	     	= BFPOPTIONS_X_POS;
	s_bfpoptions.q3hitsfx.generic.y	    	= y;

	s_bfpoptions.back.generic.type		= MTYPE_BITMAP;
	s_bfpoptions.back.generic.name		= ART_BACK0;
	s_bfpoptions.back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_bfpoptions.back.generic.callback	= BFPOptions_Event;
	s_bfpoptions.back.generic.id	    = ID_BACK;
	s_bfpoptions.back.generic.x			= 0;
	s_bfpoptions.back.generic.y			= 480-80;
	s_bfpoptions.back.width  		    = 80;
	s_bfpoptions.back.height  		    = 80;
	s_bfpoptions.back.focuspic			= ART_BACK1;

	BFPButtonOptions_MenuSet();

	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.viewpoint );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.fix3person );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.simplehud );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.flightilt );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.accucrosshair );

	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.particlesfx );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.kitrailength );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.beamcmpxy );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.bigheads );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.defaultskins );

	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.chargealert );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.q3hitsfx );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.stfu );

	thirdperson = trap_Cvar_VariableValue( "cg_thirdPerson" );
	firstpersonvis = trap_Cvar_VariableValue( "cg_drawOwnModel" );

	if ( thirdperson == 1 ) {
		s_bfpoptions.viewpoint.curvalue = 0;
	} else if ( firstpersonvis == 1 ) {
		s_bfpoptions.viewpoint.curvalue = 2;
	} else if ( firstpersonvis == 0 ) {
		s_bfpoptions.viewpoint.curvalue = 1;
	}

	s_bfpoptions.kitrailength.curvalue  = trap_Cvar_VariableValue( "cg_kiTrail" );
	s_bfpoptions.beamcmpxy.curvalue  = trap_Cvar_VariableValue( "cg_beamTrail" );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.back );

	BFPOptions_SetMenuItems();
}


/*
===============
BFPOptions_Cache
===============
*/
void BFPOptions_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_MENUBG );
	trap_R_RegisterShaderNoMip( ART_BARLOG );
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
}


/*
===============
UI_BFPOptionsMenu
===============
*/
void UI_BFPOptionsMenu( void ) {
	menuBarOption = ID_AURASCONFIG;
	BFPAuraOptions_MenuInit();
	UI_PushMenu( &s_bfpoptions.menu );
}
