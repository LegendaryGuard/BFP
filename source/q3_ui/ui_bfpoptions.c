/*
=======================================================================

BFP OPTIONS MENU

=======================================================================
*/


#include "ui_local.h"


#define	ART_BACK0				"menu/art/back_0"
#define	ART_BACK1				"menu/art/back_1"
#define	ART_MENUBG				"menu/art/menubg"
#define	ART_BARLOG				"menu/art/cap_barlog"

#define	BFPOPTIONS_X_POS		450
#define	BFPOPTIONS_SECTION_Y	(BIGCHAR_HEIGHT * 2)

#define	ID_AURATYPE				138
#define	ID_EXPLOTYPE			139
#define	ID_VIEWPOINT			140
#define	ID_FIX3PERSON			141
#define	ID_PARTICLESFX			142
#define	ID_3DPARTICLESFX		143
#define	ID_DYNAURALIGHT			144
#define	ID_DYNEXPLOLIGHT		145
#define	ID_KITRAILENGTH			146
#define	ID_BEAMCOMPLEXITY		147
#define	ID_TRANSFORMATIONAURA	148
#define	ID_SMALLAURA			149
#define	ID_ULTIMAPERMAGLOW		150
#define	ID_ACCURATECROSSHAIR	151
#define	ID_SIMPLEHUD			152
#define	ID_CHARGEALERT			153
#define	ID_Q3HITSFX				154
#define	ID_FLIGHTTILT			155
#define	ID_BIGHEADS				156
#define	ID_DEFAULTSKINS			157
#define	ID_STFU					158
#define	ID_LOWPOLYSPHERE		159
#define	ID_BIGEXPLOSIONS		160
#define	ID_EXPLOSIONSHELL		161
#define	ID_EXPLOSIONSMOKE		162
#define	ID_EXPLOSIONRING		163
#define	ID_BACK					164
#define	ID_AURASCONFIG			165
#define	ID_EXPLOSIONSCONFIG		166
#define	ID_VIEWEFFSNDCONFIG		167

// Macros to handle the cases in that order
#define SPRITE_AURA				0
#define LIGHTWEIGHT_AURA		1
#define POLYGON_AURA			2
#define HIGHPOLYCOUNT_AURA		3
#define PARTICLE_AURA			4
#define SHADER_AURA				5

#define	WIMPY_EXPLOSION				0
#define	WEAK_EXPLOSION				1
#define	SO_SO_EXPLOSION				2
#define	HARDCORE_EXPLOSION			3
#define	ULTRA_HARDCORE_EXPLOSION	4

static const char *auraType_items[] = {
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

static const char* explosionType_items[] = {
	"Wimpy",
	"Weak",
	"So-So",
	"Hardcore",
	"Ultra Hardcore",
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

	menulist_s			auraType;
	menulist_s			explosionType;
	menulist_s			viewpoint;
	menuradiobutton_s	fix3person;
	menuradiobutton_s	particlesFX;
	menuradiobutton_s	particles3dFX;
	menuradiobutton_s	dynAuraLight;
	menuradiobutton_s	dynamicLights;
	menuradiobutton_s	dynExploLights;
	menuradiobutton_s	bigExplosions;
	menuradiobutton_s	explosionShell;
	menuradiobutton_s	explosionSmoke;
	menuradiobutton_s	explosionRing;
	menuslider_s		kiTrailLength;
	menuslider_s		beamComplexity;
	menuradiobutton_s	transformationAura;
	menuradiobutton_s	smallAura;
	menuradiobutton_s	ultimatePermaGlow;
	menuradiobutton_s	accurateCrosshair;
	menuradiobutton_s	simpleHud;
	menuradiobutton_s	chargeAlert;
	menuradiobutton_s	q3HitsFX;
	menuradiobutton_s	flightTilt;
	menuradiobutton_s	bigHeads;
	menuradiobutton_s	defaultSkins;
	menuradiobutton_s	stfu;
	menuradiobutton_s	lowPolySphere;
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
	BFPOptions_MenuItem( &s_bfpoptions.fix3person.curvalue,			"cg_fixedThirdPerson",		0 );
	BFPOptions_MenuItem( &s_bfpoptions.particlesFX.curvalue,		"cg_particles",				0 );
	BFPOptions_MenuItem( &s_bfpoptions.particles3dFX.curvalue,		"cg_3dparticles",			0 );
	BFPOptions_MenuItem( &s_bfpoptions.dynAuraLight.curvalue,		"cg_lightAuras",			0 );
	BFPOptions_MenuItem( &s_bfpoptions.dynExploLights.curvalue,		"cg_lightExplosions",		0 );
	BFPOptions_MenuItem( &s_bfpoptions.bigExplosions.curvalue,		"cg_bigExplosions",			0 );
	BFPOptions_MenuItem( &s_bfpoptions.explosionShell.curvalue,		"cg_explosionShell",		0 );
	BFPOptions_MenuItem( &s_bfpoptions.explosionSmoke.curvalue,		"cg_explosionSmoke",		0 );
	BFPOptions_MenuItem( &s_bfpoptions.explosionRing.curvalue,		"cg_explosionRing",			0 );
	BFPOptions_MenuItem( &s_bfpoptions.transformationAura.curvalue,	"cg_transformationAura",	0 );
	BFPOptions_MenuItem( &s_bfpoptions.smallAura.curvalue,			"cg_smallOwnAura",			0 );
	BFPOptions_MenuItem( &s_bfpoptions.ultimatePermaGlow.curvalue,	"cg_permaglowUltimate",		0 );
	BFPOptions_MenuItem( &s_bfpoptions.accurateCrosshair.curvalue,	"cg_stableCrosshair",		1 ); // doesn't make sense if the crosshair isn't accurate
	BFPOptions_MenuItem( &s_bfpoptions.simpleHud.curvalue,			"cg_simpleHUD",				0 );
	BFPOptions_MenuItem( &s_bfpoptions.chargeAlert.curvalue,		"cg_chargeupAlert",			0 );
	BFPOptions_MenuItem( &s_bfpoptions.q3HitsFX.curvalue,			"cg_playHitSound",			0 );
	BFPOptions_MenuItem( &s_bfpoptions.flightTilt.curvalue,			"cg_flytilt",				0 );
	BFPOptions_MenuItem( &s_bfpoptions.bigHeads.curvalue,			"cg_superdeformed",			0 );
	BFPOptions_MenuItem( &s_bfpoptions.defaultSkins.curvalue,		"cg_forceSkin",				0 );
	BFPOptions_MenuItem( &s_bfpoptions.stfu.curvalue,				"cg_stfu",					0 );
	BFPOptions_MenuItem( &s_bfpoptions.lowPolySphere.curvalue,		"cg_lowPolySphere",			0 );
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

static void BFPOptions_ExploType_Setup( int exShell, int exSmoke, int particles, int exRing, int particles3d ) {
	trap_Cvar_SetValue( "cg_explosionShell", exShell );
	trap_Cvar_SetValue( "cg_explosionSmoke", exSmoke );
	trap_Cvar_SetValue( "cg_explosionRing", exRing );
	trap_Cvar_SetValue( "cg_particles", particles );
	trap_Cvar_SetValue( "cg_3dparticles", particles3d );
	s_bfpoptions.explosionShell.curvalue = exShell;
	s_bfpoptions.explosionSmoke.curvalue = exSmoke;
	s_bfpoptions.explosionRing.curvalue = exRing;
	s_bfpoptions.particlesFX.curvalue = particles;
	s_bfpoptions.particles3dFX.curvalue = particles3d;
}

static void BFPOptions_ExplosionsTypeCheck( void ) {
	int particles = s_bfpoptions.particlesFX.curvalue;
	int particles3d = s_bfpoptions.particles3dFX.curvalue;
	int explosionSmoke = s_bfpoptions.explosionSmoke.curvalue;
	int explosionShell = s_bfpoptions.explosionShell.curvalue;
	int explosionRing = s_bfpoptions.explosionRing.curvalue;

	if ( particles <= 0 && particles3d <= 0 && explosionSmoke <= 0 && explosionShell <= 0 && explosionRing <= 0 ) {
		s_bfpoptions.explosionType.curvalue = WIMPY_EXPLOSION;
	}
	if ( particles <= 0 && particles3d <= 0 && explosionSmoke <= 0 && explosionShell >= 1 && explosionRing >= 1 ) {
		s_bfpoptions.explosionType.curvalue = WEAK_EXPLOSION;
	}
	if ( particles >= 1 && particles3d <= 0 && explosionSmoke <= 0 && explosionShell >= 1 && explosionRing >= 1 ) {
		s_bfpoptions.explosionType.curvalue = SO_SO_EXPLOSION;
	}
	if ( particles >= 1 && particles3d <= 0 && explosionSmoke >= 1 && explosionShell >= 1 && explosionRing >= 1 ) {
		s_bfpoptions.explosionType.curvalue = HARDCORE_EXPLOSION;
	}
	if ( particles >= 1 && particles3d >= 1 && explosionSmoke >= 1 && explosionShell >= 1 && explosionRing >= 1 ) {
		s_bfpoptions.explosionType.curvalue = ULTRA_HARDCORE_EXPLOSION;
	}
}

static void BFPOptions_ParticlesCheck( void ) {
	int particles   = s_bfpoptions.particlesFX.curvalue;
	int particles3d  = s_bfpoptions.particles3dFX.curvalue;

	if ( particles <= 0 && particles3d >= 1 ) {
		s_bfpoptions.particles3dFX.curvalue = 0;
		trap_Cvar_SetValue( "cg_3dparticles", 0 );
	}
}

static void BFPOptions_Event( void* ptr, int notification ) {
	if ( notification != QM_ACTIVATED ) {
		return;
	}

	switch ( ((menucommon_s*)ptr)->id ) {
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

	// AURAS

	case ID_AURATYPE:
		switch ( s_bfpoptions.auraType.curvalue ) {
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

	// EXPLOSIONS

	case ID_EXPLOTYPE:
		switch ( s_bfpoptions.explosionType.curvalue ) {
		case WIMPY_EXPLOSION: // Wimpy
			BFPOptions_ExploType_Setup( 0, 0, 0, 0, 0 );
			break;

		case WEAK_EXPLOSION: // Weak
			BFPOptions_ExploType_Setup( 1, 0, 0, 1, 0 );
			break;

		case SO_SO_EXPLOSION: // So-So
			BFPOptions_ExploType_Setup( 1, 0, 1, 1, 0 );
			break;

		case HARDCORE_EXPLOSION: // Hardcore
			BFPOptions_ExploType_Setup( 1, 1, 1, 1, 0 );
			break;

		case ULTRA_HARDCORE_EXPLOSION: // Ultra Hardcore
			BFPOptions_ExploType_Setup( 1, 1, 1, 1, 1 );
			break;
		}
		break;


	case ID_FIX3PERSON:
		trap_Cvar_SetValue( "cg_fixedThirdPerson", s_bfpoptions.fix3person.curvalue );
		break;

	case ID_PARTICLESFX:
		trap_Cvar_SetValue( "cg_particles", s_bfpoptions.particlesFX.curvalue );
		BFPOptions_ParticlesCheck();
		BFPOptions_ExplosionsTypeCheck();
		break;

	case ID_3DPARTICLESFX:
		if ( s_bfpoptions.particles3dFX.curvalue >= 1 && s_bfpoptions.particlesFX.curvalue <= 0 ) {
			s_bfpoptions.particlesFX.curvalue = 1;
			trap_Cvar_SetValue( "cg_particles", 1 );
		}
		trap_Cvar_SetValue( "cg_3dparticles", s_bfpoptions.particles3dFX.curvalue );
		BFPOptions_ExplosionsTypeCheck();
		break;
	
	case ID_DYNAURALIGHT:
		trap_Cvar_SetValue( "cg_lightAuras", s_bfpoptions.dynAuraLight.curvalue );
		break;
	
	case ID_DYNEXPLOLIGHT:
		trap_Cvar_SetValue( "cg_lightExplosions", s_bfpoptions.dynExploLights.curvalue );
		break;

	case ID_BIGEXPLOSIONS:
		trap_Cvar_SetValue( "cg_bigExplosions", s_bfpoptions.bigExplosions.curvalue );
		break;
	
	case ID_EXPLOSIONSHELL:
		trap_Cvar_SetValue( "cg_explosionShell", s_bfpoptions.explosionShell.curvalue );
		BFPOptions_ExplosionsTypeCheck();
		break;
	
	case ID_EXPLOSIONSMOKE:
		trap_Cvar_SetValue( "cg_explosionSmoke", s_bfpoptions.explosionSmoke.curvalue );
		BFPOptions_ExplosionsTypeCheck();
		break;
	
	case ID_EXPLOSIONRING:
		trap_Cvar_SetValue( "cg_explosionRing", s_bfpoptions.explosionRing.curvalue );
		BFPOptions_ExplosionsTypeCheck();
		break;


	case ID_KITRAILENGTH:
		trap_Cvar_SetValue( "cg_kiTrail", s_bfpoptions.kiTrailLength.curvalue  );
		break;

	case ID_BEAMCOMPLEXITY:
		trap_Cvar_SetValue( "cg_beamTrail", s_bfpoptions.beamComplexity.curvalue  );
		break;


	case ID_TRANSFORMATIONAURA:
		trap_Cvar_SetValue( "cg_transformationAura", s_bfpoptions.transformationAura.curvalue );
		break;
	
	case ID_SMALLAURA:
		trap_Cvar_SetValue( "cg_smallOwnAura", s_bfpoptions.smallAura.curvalue );
		break;

	case ID_ULTIMAPERMAGLOW:
		trap_Cvar_SetValue( "cg_permaglowUltimate", s_bfpoptions.ultimatePermaGlow.curvalue );
		break;

	case ID_ACCURATECROSSHAIR:
		trap_Cvar_SetValue( "cg_stableCrosshair", s_bfpoptions.accurateCrosshair.curvalue ? 0 : 1 ); // doesn't make sense if the crosshair isn't accurate
		break;

	case ID_SIMPLEHUD:
		trap_Cvar_SetValue( "cg_simpleHUD", s_bfpoptions.simpleHud.curvalue );
		break;

	case ID_CHARGEALERT:
		trap_Cvar_SetValue( "cg_chargeupAlert", s_bfpoptions.chargeAlert.curvalue );
		break;

	case ID_Q3HITSFX:
		trap_Cvar_SetValue( "cg_playHitSound", s_bfpoptions.q3HitsFX.curvalue );
		break;

	case ID_FLIGHTTILT:
		trap_Cvar_SetValue( "cg_flytilt", s_bfpoptions.flightTilt.curvalue );
		break;

	case ID_BIGHEADS:
		trap_Cvar_SetValue( "cg_superdeformed", s_bfpoptions.bigHeads.curvalue );
		break;

	case ID_DEFAULTSKINS:
		trap_Cvar_SetValue( "cg_forceSkin", s_bfpoptions.defaultSkins.curvalue );
		break;

	case ID_STFU:
		trap_Cvar_SetValue( "cg_stfu", s_bfpoptions.stfu.curvalue );
		break;

	case ID_LOWPOLYSPHERE:
		trap_Cvar_SetValue( "cg_lowPolySphere", s_bfpoptions.lowPolySphere.curvalue );
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
	s_bfpoptions.auraType.generic.type		= MTYPE_SPINCONTROL;
	s_bfpoptions.auraType.generic.name		= "Aura Type:";
	s_bfpoptions.auraType.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.auraType.generic.callback	= BFPOptions_Event;
	s_bfpoptions.auraType.generic.id		= ID_AURATYPE;
	s_bfpoptions.auraType.generic.x			= BFPOPTIONS_X_POS-130;
	s_bfpoptions.auraType.generic.y			= y;
	s_bfpoptions.auraType.itemnames			= auraType_items;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.dynAuraLight.generic.type        = MTYPE_RADIOBUTTON;
	s_bfpoptions.dynAuraLight.generic.name	      = "Dynamic Aura Lights:";
	s_bfpoptions.dynAuraLight.generic.flags	      = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.dynAuraLight.generic.callback    = BFPOptions_Event;
	s_bfpoptions.dynAuraLight.generic.id          = ID_DYNAURALIGHT;
	s_bfpoptions.dynAuraLight.generic.x	          = BFPOPTIONS_X_POS;
	s_bfpoptions.dynAuraLight.generic.y	          = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.transformationAura.generic.type        = MTYPE_RADIOBUTTON;
	s_bfpoptions.transformationAura.generic.name		   = "Transformation Aura:";
	s_bfpoptions.transformationAura.generic.flags	   = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.transformationAura.generic.callback    = BFPOptions_Event;
	s_bfpoptions.transformationAura.generic.id          = ID_TRANSFORMATIONAURA;
	s_bfpoptions.transformationAura.generic.x	       = BFPOPTIONS_X_POS;
	s_bfpoptions.transformationAura.generic.y	       = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.smallAura.generic.type     = MTYPE_RADIOBUTTON;
	s_bfpoptions.smallAura.generic.name	  	= "Small Own Aura:";
	s_bfpoptions.smallAura.generic.flags    = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.smallAura.generic.callback = BFPOptions_Event;
	s_bfpoptions.smallAura.generic.id       = ID_SMALLAURA;
	s_bfpoptions.smallAura.generic.x	    = BFPOPTIONS_X_POS;
	s_bfpoptions.smallAura.generic.y	    = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.ultimatePermaGlow.generic.type    = MTYPE_RADIOBUTTON;
	s_bfpoptions.ultimatePermaGlow.generic.name	  = "Ultimate Perma-Glow:";
	s_bfpoptions.ultimatePermaGlow.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.ultimatePermaGlow.generic.callback = BFPOptions_Event;
	s_bfpoptions.ultimatePermaGlow.generic.id      = ID_ULTIMAPERMAGLOW;
	s_bfpoptions.ultimatePermaGlow.generic.x	      = BFPOPTIONS_X_POS;
	s_bfpoptions.ultimatePermaGlow.generic.y	      = y;

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

	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.auraType );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.dynAuraLight );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.transformationAura );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.smallAura );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.ultimatePermaGlow );

	highpolyaura = trap_Cvar_VariableValue( "cg_highPolyAura" );
	polygonalaura = trap_Cvar_VariableValue( "cg_polygonAura" );
	lightweightaura = trap_Cvar_VariableValue( "cg_lightweightAuras" );
	spriteaura = trap_Cvar_VariableValue( "cg_spriteAura" );
	particleaura = trap_Cvar_VariableValue( "cg_particleAura" );

	if ( highpolyaura >= 1 ) {
		s_bfpoptions.auraType.curvalue = HIGHPOLYCOUNT_AURA;
	} else if ( polygonalaura >= 1 )  {
		s_bfpoptions.auraType.curvalue = POLYGON_AURA;
	} else if ( lightweightaura >= 1 ) {
		s_bfpoptions.auraType.curvalue = LIGHTWEIGHT_AURA;
	} else if ( spriteaura >= 1 ) {
		s_bfpoptions.auraType.curvalue = SPRITE_AURA;
	} else if ( particleaura >= 1 ) {
		s_bfpoptions.auraType.curvalue = PARTICLE_AURA;
	} else {
		s_bfpoptions.auraType.curvalue = SHADER_AURA;
	}

	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.back );

	BFPOptions_SetMenuItems();
}


void BFPExplosionsOptions_MenuInit( void ) {
	int		y;
	int		explosionRing, explosionShell, explosionSmoke;
	int		particles, particles3d;

	memset( &s_bfpoptions, 0, sizeof(bfpoptions_t) );

	BFPOptions_Cache();

	s_bfpoptions.menu.wrapAround = qtrue;
	s_bfpoptions.menu.fullscreen = qtrue;

	y = 240 - 4 * (BIGCHAR_HEIGHT+2);
	s_bfpoptions.explosionType.generic.type		= MTYPE_SPINCONTROL;
	s_bfpoptions.explosionType.generic.name		= "Explosion Type:";
	s_bfpoptions.explosionType.generic.flags	= QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_bfpoptions.explosionType.generic.callback = BFPOptions_Event;
	s_bfpoptions.explosionType.generic.id		= ID_EXPLOTYPE;
	s_bfpoptions.explosionType.generic.x		= BFPOPTIONS_X_POS;
	s_bfpoptions.explosionType.generic.y		= y;
	s_bfpoptions.explosionType.itemnames		= explosionType_items;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.bigExplosions.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.bigExplosions.generic.name		= "Big Explosions:";
	s_bfpoptions.bigExplosions.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.bigExplosions.generic.callback	= BFPOptions_Event;
	s_bfpoptions.bigExplosions.generic.id		= ID_BIGEXPLOSIONS;
	s_bfpoptions.bigExplosions.generic.x		= BFPOPTIONS_X_POS;
	s_bfpoptions.bigExplosions.generic.y		= y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.dynExploLights.generic.type        = MTYPE_RADIOBUTTON;
	s_bfpoptions.dynExploLights.generic.name	    = "Dynamic Explosion Lights:";
	s_bfpoptions.dynExploLights.generic.flags	    = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.dynExploLights.generic.callback    = BFPOptions_Event;
	s_bfpoptions.dynExploLights.generic.id          = ID_DYNEXPLOLIGHT;
	s_bfpoptions.dynExploLights.generic.x	        = BFPOPTIONS_X_POS;
	s_bfpoptions.dynExploLights.generic.y	        = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.lowPolySphere.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.lowPolySphere.generic.name		= "Low Polycount Sphere:";
	s_bfpoptions.lowPolySphere.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.lowPolySphere.generic.callback	= BFPOptions_Event;
	s_bfpoptions.lowPolySphere.generic.id		= ID_LOWPOLYSPHERE;
	s_bfpoptions.lowPolySphere.generic.x		= BFPOPTIONS_X_POS;
	s_bfpoptions.lowPolySphere.generic.y		= y;

	y += BFPOPTIONS_SECTION_Y;
	s_bfpoptions.explosionSmoke.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.explosionSmoke.generic.name		= "Smoke:";
	s_bfpoptions.explosionSmoke.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.explosionSmoke.generic.callback	= BFPOptions_Event;
	s_bfpoptions.explosionSmoke.generic.id			= ID_EXPLOSIONSMOKE;
	s_bfpoptions.explosionSmoke.generic.x			= BFPOPTIONS_X_POS;
	s_bfpoptions.explosionSmoke.generic.y			= y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.explosionShell.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.explosionShell.generic.name		= "Shell:";
	s_bfpoptions.explosionShell.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.explosionShell.generic.callback	= BFPOptions_Event;
	s_bfpoptions.explosionShell.generic.id			= ID_EXPLOSIONSHELL;
	s_bfpoptions.explosionShell.generic.x			= BFPOPTIONS_X_POS;
	s_bfpoptions.explosionShell.generic.y			= y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.explosionRing.generic.type			= MTYPE_RADIOBUTTON;
	s_bfpoptions.explosionRing.generic.name			= "Ring:";
	s_bfpoptions.explosionRing.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.explosionRing.generic.callback		= BFPOptions_Event;
	s_bfpoptions.explosionRing.generic.id			= ID_EXPLOSIONRING;
	s_bfpoptions.explosionRing.generic.x			= BFPOPTIONS_X_POS;
	s_bfpoptions.explosionRing.generic.y			= y;

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

	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.explosionType );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.bigExplosions );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.dynExploLights );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.lowPolySphere );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.explosionSmoke );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.explosionShell );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.explosionRing );

	explosionSmoke = trap_Cvar_VariableValue( "cg_explosionSmoke" );
	explosionShell = trap_Cvar_VariableValue( "cg_explosionShell" );
	explosionRing = trap_Cvar_VariableValue( "cg_explosionRing" );
	particles = trap_Cvar_VariableValue( "cg_particles" );
	particles3d = trap_Cvar_VariableValue( "cg_3dparticles" );

	s_bfpoptions.bigExplosions.curvalue = trap_Cvar_VariableValue( "cg_bigExplosions" );
	s_bfpoptions.explosionSmoke.curvalue = explosionSmoke;
	s_bfpoptions.explosionShell.curvalue = explosionShell;
	s_bfpoptions.explosionRing.curvalue = explosionRing;

	s_bfpoptions.particlesFX.curvalue = particles;
	s_bfpoptions.particles3dFX.curvalue = particles3d;

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

	// VIEW & HUD
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
	s_bfpoptions.simpleHud.generic.type			= MTYPE_RADIOBUTTON;
	s_bfpoptions.simpleHud.generic.name			= "Simple HUD:";
	s_bfpoptions.simpleHud.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.simpleHud.generic.callback		= BFPOptions_Event;
	s_bfpoptions.simpleHud.generic.id			= ID_SIMPLEHUD;
	s_bfpoptions.simpleHud.generic.x			= BFPOPTIONS_X_POS;
	s_bfpoptions.simpleHud.generic.y			= y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.flightTilt.generic.type     = MTYPE_RADIOBUTTON;
	s_bfpoptions.flightTilt.generic.name		= "Flight Tilt:";
	s_bfpoptions.flightTilt.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.flightTilt.generic.callback = BFPOptions_Event;
	s_bfpoptions.flightTilt.generic.id       = ID_FLIGHTTILT;
	s_bfpoptions.flightTilt.generic.x	    = BFPOPTIONS_X_POS;
	s_bfpoptions.flightTilt.generic.y	    = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.accurateCrosshair.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.accurateCrosshair.generic.name		= "Accurate Crosshair:";
	s_bfpoptions.accurateCrosshair.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.accurateCrosshair.generic.callback	= BFPOptions_Event;
	s_bfpoptions.accurateCrosshair.generic.id		= ID_ACCURATECROSSHAIR;
	s_bfpoptions.accurateCrosshair.generic.x		= BFPOPTIONS_X_POS;
	s_bfpoptions.accurateCrosshair.generic.y		= y;


	// EFFECTS
	y += BFPOPTIONS_SECTION_Y;
	s_bfpoptions.particlesFX.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.particlesFX.generic.name		= "Particle Effects:";
	s_bfpoptions.particlesFX.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.particlesFX.generic.callback	= BFPOptions_Event;
	s_bfpoptions.particlesFX.generic.id			= ID_PARTICLESFX;
	s_bfpoptions.particlesFX.generic.x			= BFPOPTIONS_X_POS;
	s_bfpoptions.particlesFX.generic.y			= y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.particles3dFX.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.particles3dFX.generic.name		= "3D Particle Effects:";
	s_bfpoptions.particles3dFX.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.particles3dFX.generic.callback	= BFPOptions_Event;
	s_bfpoptions.particles3dFX.generic.id		= ID_3DPARTICLESFX;
	s_bfpoptions.particles3dFX.generic.x		= BFPOPTIONS_X_POS;
	s_bfpoptions.particles3dFX.generic.y		= y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.kiTrailLength.generic.type		= MTYPE_SLIDER;
	s_bfpoptions.kiTrailLength.generic.name		= "Ki Trail Length:";
	s_bfpoptions.kiTrailLength.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.kiTrailLength.generic.callback	= BFPOptions_Event;
	s_bfpoptions.kiTrailLength.generic.id		= ID_KITRAILENGTH;
	s_bfpoptions.kiTrailLength.generic.x	        = BFPOPTIONS_X_POS;
	s_bfpoptions.kiTrailLength.generic.y	        = y;
	s_bfpoptions.kiTrailLength.minvalue			= 0;
	s_bfpoptions.kiTrailLength.maxvalue			= 100;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.beamComplexity.generic.type		= MTYPE_SLIDER;
	s_bfpoptions.beamComplexity.generic.name		= "Beam Complexity:";
	s_bfpoptions.beamComplexity.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.beamComplexity.generic.callback	= BFPOptions_Event;
	s_bfpoptions.beamComplexity.generic.id		= ID_BEAMCOMPLEXITY;
	s_bfpoptions.beamComplexity.generic.x	    = BFPOPTIONS_X_POS;
	s_bfpoptions.beamComplexity.generic.y	    = y;
	s_bfpoptions.beamComplexity.minvalue			= 0;
	s_bfpoptions.beamComplexity.maxvalue			= 100;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.bigHeads.generic.type     = MTYPE_RADIOBUTTON;
	s_bfpoptions.bigHeads.generic.name	   = "Superdeformed Heads:";
	s_bfpoptions.bigHeads.generic.flags	   = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.bigHeads.generic.callback = BFPOptions_Event;
	s_bfpoptions.bigHeads.generic.id       = ID_BIGHEADS;
	s_bfpoptions.bigHeads.generic.x  	   = BFPOPTIONS_X_POS;
	s_bfpoptions.bigHeads.generic.y	 	   = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.defaultSkins.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.defaultSkins.generic.name		= "Force Default Skins:";
	s_bfpoptions.defaultSkins.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.defaultSkins.generic.callback	= BFPOptions_Event;
	s_bfpoptions.defaultSkins.generic.id		= ID_DEFAULTSKINS;
	s_bfpoptions.defaultSkins.generic.x			= BFPOPTIONS_X_POS;
	s_bfpoptions.defaultSkins.generic.y			= y;


	// SOUNDS
	y += BFPOPTIONS_SECTION_Y;
	s_bfpoptions.stfu.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.stfu.generic.name		= "Disable Voices:";
	s_bfpoptions.stfu.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.stfu.generic.callback	= BFPOptions_Event;
	s_bfpoptions.stfu.generic.id		= ID_STFU;
	s_bfpoptions.stfu.generic.x			= BFPOPTIONS_X_POS;
	s_bfpoptions.stfu.generic.y			= y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.chargeAlert.generic.type     = MTYPE_RADIOBUTTON;
	s_bfpoptions.chargeAlert.generic.name	  = "Chargeup Alerts:";
	s_bfpoptions.chargeAlert.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.chargeAlert.generic.callback = BFPOptions_Event;
	s_bfpoptions.chargeAlert.generic.id       = ID_CHARGEALERT;
	s_bfpoptions.chargeAlert.generic.x	      = BFPOPTIONS_X_POS;
	s_bfpoptions.chargeAlert.generic.y	      = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.q3HitsFX.generic.type    	= MTYPE_RADIOBUTTON;
	s_bfpoptions.q3HitsFX.generic.name	  	= "Q3 Hit Sound:";
	s_bfpoptions.q3HitsFX.generic.flags	  	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.q3HitsFX.generic.callback	= BFPOptions_Event;
	s_bfpoptions.q3HitsFX.generic.id      	= ID_Q3HITSFX;
	s_bfpoptions.q3HitsFX.generic.x	     	= BFPOPTIONS_X_POS;
	s_bfpoptions.q3HitsFX.generic.y	    	= y;

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
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.simpleHud );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.flightTilt );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.accurateCrosshair );

	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.particlesFX );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.particles3dFX );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.kiTrailLength );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.beamComplexity );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.bigHeads );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.defaultSkins );

	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.chargeAlert );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.q3HitsFX );
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

	s_bfpoptions.kiTrailLength.curvalue  = trap_Cvar_VariableValue( "cg_kiTrail" );
	s_bfpoptions.beamComplexity.curvalue  = trap_Cvar_VariableValue( "cg_beamTrail" );
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
