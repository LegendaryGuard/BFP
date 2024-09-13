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

#define BFPOPTIONS_X_POS		360

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
#define ID_SSJGLOW          149
#define ID_ACCUCROSSHAIR    150
#define ID_SIMPLEHUD        151
#define ID_CHARGEALERT      152
#define ID_Q3HITSFX         153
#define ID_FLIGHTILT        154
#define ID_BIGHEADS         155
#define ID_DEFAULTSKINS    	156
#define ID_STFU             157
#define ID_LOWPOLYSPHERE 	158
#define ID_BACK				159

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
	"First Person Vis",
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

	menulist_s			auratype;
	menulist_s			explotype;
	menulist_s			viewpoint;
	menuradiobutton_s	fix3person;
	menuradiobutton_s	particlesfx;
	menuradiobutton_s	dynauralight;
	menuradiobutton_s	dynamiclights;
	menuradiobutton_s	dynexplolights;
	menuslider_s		kitrailength;
	menuslider_s		beamcmpxy;
	menuradiobutton_s	transaura;
	menuradiobutton_s	smallaura;
	menuradiobutton_s	ssjglow;
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

// A macro to look better the code
#define BFPOPTIONS_MENUITEM( menu_item_curvalue, cvar ) \
	menu_item_curvalue = trap_Cvar_VariableValue( cvar ) != 0;

static void BFPOptions_SetMenuItems( void ) {
	BFPOPTIONS_MENUITEM( s_bfpoptions.fix3person.curvalue,     "cg_fixedThirdPerson" )
	BFPOPTIONS_MENUITEM( s_bfpoptions.particlesfx.curvalue,    "cg_particles" )
	BFPOPTIONS_MENUITEM( s_bfpoptions.dynauralight.curvalue,   "cg_lightAuras" )
	BFPOPTIONS_MENUITEM( s_bfpoptions.dynexplolights.curvalue, "cg_lightExplosions" )
	BFPOPTIONS_MENUITEM( s_bfpoptions.transaura.curvalue,      "cg_transformationAura" )
	BFPOPTIONS_MENUITEM( s_bfpoptions.smallaura.curvalue,      "cg_smallOwnAura" )
	BFPOPTIONS_MENUITEM( s_bfpoptions.ssjglow.curvalue,        "cg_permaglowUltimate" )
	BFPOPTIONS_MENUITEM( s_bfpoptions.accucrosshair.curvalue,  "cg_stableCrosshair" )
	BFPOPTIONS_MENUITEM( s_bfpoptions.simplehud.curvalue,      "cg_simpleHUD" )
	BFPOPTIONS_MENUITEM( s_bfpoptions.chargealert.curvalue,    "cg_chargeupAlert" )
	BFPOPTIONS_MENUITEM( s_bfpoptions.q3hitsfx.curvalue,       "cg_playHitSound" )
	BFPOPTIONS_MENUITEM( s_bfpoptions.flightilt.curvalue,      "cg_flytilt" )
	BFPOPTIONS_MENUITEM( s_bfpoptions.bigheads.curvalue,       "cg_superdeformed" )
	BFPOPTIONS_MENUITEM( s_bfpoptions.defaultskins.curvalue,   "cg_forceSkin" )
	BFPOPTIONS_MENUITEM( s_bfpoptions.stfu.curvalue,           "cg_stfu" )
	BFPOPTIONS_MENUITEM( s_bfpoptions.lowpolysphere.curvalue,  "cg_lowpolysphere" )
}
#undef BFPOPTIONS_MENUITEM

// Macros to look better the code
#define AURATYPE_SETUP(sprite, highpoly, poly, light, particle) \
		trap_Cvar_SetValue( "cg_spriteAura", sprite ); \
		trap_Cvar_SetValue( "cg_highPolyAura", highpoly ); \
		trap_Cvar_SetValue( "cg_polygonAura", poly ); \
		trap_Cvar_SetValue( "cg_lightweightAuras", light );\
		trap_Cvar_SetValue( "cg_particleAura", particle );

#define VIEWPOINT_SETUP(tp, ownmodel) \
		trap_Cvar_SetValue( "cg_thirdPerson", tp ); \
		trap_Cvar_SetValue( "cg_drawOwnModel", ownmodel );

#define EXPLOTYPE_SETUP(expShell, expSmoke, particles, expRing ) \
		trap_Cvar_SetValue( "cg_explosionShell", expShell ); \
		trap_Cvar_SetValue( "cg_explosionSmoke", expSmoke ); \
		trap_Cvar_SetValue( "cg_particles", particles ); \
		trap_Cvar_SetValue( "cg_explosionRing", expRing ); \
		s_bfpoptions.particlesfx.curvalue = particles;

static void BFPOptions_Event( void* ptr, int notification ) {
	if ( notification != QM_ACTIVATED ) {
		return;
	}

	switch ( ((menucommon_s*)ptr)->id ) {

		//-----------------------------Aura list---------------------------------//

	case ID_AURATYPE:
		switch ( s_bfpoptions.auratype.curvalue ) {
		case SPRITE_AURA:
			AURATYPE_SETUP( 1, 0, 0, 0, 0 )
			break;

		case LIGHTWEIGHT_AURA:
			AURATYPE_SETUP( 0, 0, 0, 1, 0 )
			break;

		case POLYGON_AURA:
			AURATYPE_SETUP( 0, 0, 1, 0, 0 )
			break;

		case HIGHPOLYCOUNT_AURA:
			AURATYPE_SETUP( 0, 1, 1, 0, 0 )
			break;

		case PARTICLE_AURA:
			AURATYPE_SETUP( 0, 0, 0, 0, 1 )
			break;

		case SHADER_AURA:
			AURATYPE_SETUP( 0, 0, 0, 0, 0 )
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
			VIEWPOINT_SETUP( 0, 0 )
			break;

		case 2: // First Person Vis
			VIEWPOINT_SETUP( 0, 1 )
			break;
		}
		break;

		//---------------------------Explosion type list---------------------------------------//

	case ID_EXPLOTYPE:
		switch ( s_bfpoptions.explotype.curvalue ) {
		case WIMPY_EXPLO: // Wimpy
			EXPLOTYPE_SETUP( 0, 0, 0, 0 )
			break;

		case WEAK_EXPLO: // Weak
			EXPLOTYPE_SETUP( 1, 0, 0, 1 )
			break;

		case SO_SO_EXPLO: // So-So
			EXPLOTYPE_SETUP( 1, 0, 1, 1 )
			break;

		case HARDCORE_EXPLO: // Hardcore
			EXPLOTYPE_SETUP( 1, 1, 1, 1 )
			break;
		}
		break;

		//---------------------------------------------------------------------//

	case ID_FIX3PERSON:
		trap_Cvar_SetValue( "cg_fixedThirdPerson", s_bfpoptions.fix3person.curvalue );
		break;

	case ID_PARTICLESFX:
		trap_Cvar_SetValue( "cg_particles", s_bfpoptions.particlesfx.curvalue );
		break;
	
	case ID_DYNAURALIGHT:
		trap_Cvar_SetValue( "cg_lightAuras", s_bfpoptions.dynauralight.curvalue );
		break;
	
	case ID_DYNEXPLOLIGHT:
		trap_Cvar_SetValue( "cg_lightExplosions", s_bfpoptions.dynexplolights.curvalue );
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

	case ID_SSJGLOW:
		trap_Cvar_SetValue( "cg_permaglowUltimate", s_bfpoptions.ssjglow.curvalue );
		break;

	case ID_ACCUCROSSHAIR:
		trap_Cvar_SetValue( "cg_stableCrosshair", s_bfpoptions.accucrosshair.curvalue );
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
#undef AURATYPE_SETUP
#undef VIEWPOINT_SETUP
#undef EXPLOTYPE_SETUP


static void BFPOptions_MenuInit( void ) {
	int		y;
	int		highpolyaura, polygonalaura, lightweightaura, spriteaura, particleaura;
	int 	thirdperson, firstpersonvis;
	int 	explosionShell, explosionSmoke;
	int		explosionring;
	int 	particles;

	memset( &s_bfpoptions, 0, sizeof(bfpoptions_t) );

	BFPOptions_Cache();

	s_bfpoptions.menu.wrapAround = qtrue;
	s_bfpoptions.menu.fullscreen = qtrue;

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

	y = 90;
	s_bfpoptions.auratype.generic.type		= MTYPE_SPINCONTROL;
	s_bfpoptions.auratype.generic.name		= "Aura Type:";
	s_bfpoptions.auratype.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.auratype.generic.callback	= BFPOptions_Event;
	s_bfpoptions.auratype.generic.id		= ID_AURATYPE;
	s_bfpoptions.auratype.generic.x			= 320;
	s_bfpoptions.auratype.generic.y			= y;
	s_bfpoptions.auratype.itemnames			= auratype_items;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.explotype.generic.type		= MTYPE_SPINCONTROL;
	s_bfpoptions.explotype.generic.name		= "Explosion Type:";
	s_bfpoptions.explotype.generic.flags	= QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_bfpoptions.explotype.generic.callback = BFPOptions_Event;
	s_bfpoptions.explotype.generic.id		= ID_EXPLOTYPE;
	s_bfpoptions.explotype.generic.x		= 320;
	s_bfpoptions.explotype.generic.y		= y;
	s_bfpoptions.explotype.itemnames		= explotype_items;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.viewpoint.generic.type		= MTYPE_SPINCONTROL;
	s_bfpoptions.viewpoint.generic.name		= "Viewpoint:";
	s_bfpoptions.viewpoint.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.viewpoint.generic.callback	= BFPOptions_Event;
	s_bfpoptions.viewpoint.generic.id		= ID_VIEWPOINT;
	s_bfpoptions.viewpoint.generic.x		= 320;
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
	s_bfpoptions.particlesfx.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.particlesfx.generic.name		= "Particle Effects:";
	s_bfpoptions.particlesfx.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.particlesfx.generic.callback	= BFPOptions_Event;
	s_bfpoptions.particlesfx.generic.id			= ID_PARTICLESFX;
	s_bfpoptions.particlesfx.generic.x			= BFPOPTIONS_X_POS;
	s_bfpoptions.particlesfx.generic.y			= y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.dynauralight.generic.type        = MTYPE_RADIOBUTTON;
	s_bfpoptions.dynauralight.generic.name	      = "Dynamic Aura Lights:";
	s_bfpoptions.dynauralight.generic.flags	      = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.dynauralight.generic.callback    = BFPOptions_Event;
	s_bfpoptions.dynauralight.generic.id          = ID_DYNAURALIGHT;
	s_bfpoptions.dynauralight.generic.x	          = BFPOPTIONS_X_POS;
	s_bfpoptions.dynauralight.generic.y	          = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.dynexplolights.generic.type        = MTYPE_RADIOBUTTON;
	s_bfpoptions.dynexplolights.generic.name	    = "Dynamic Explosion Lights:";
	s_bfpoptions.dynexplolights.generic.flags	    = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.dynexplolights.generic.callback    = BFPOptions_Event;
	s_bfpoptions.dynexplolights.generic.id          = ID_DYNEXPLOLIGHT;
	s_bfpoptions.dynexplolights.generic.x	        = BFPOPTIONS_X_POS;
	s_bfpoptions.dynexplolights.generic.y	        = y;

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
	s_bfpoptions.ssjglow.generic.type     = MTYPE_RADIOBUTTON;
	s_bfpoptions.ssjglow.generic.name	  = "SSJ Perma-Glow:";
	s_bfpoptions.ssjglow.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.ssjglow.generic.callback = BFPOptions_Event;
	s_bfpoptions.ssjglow.generic.id       = ID_SSJGLOW;
	s_bfpoptions.ssjglow.generic.x	      = BFPOPTIONS_X_POS;
	s_bfpoptions.ssjglow.generic.y	      = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.accucrosshair.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.accucrosshair.generic.name		= "Accurate Crosshair:";
	s_bfpoptions.accucrosshair.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.accucrosshair.generic.callback	= BFPOptions_Event;
	s_bfpoptions.accucrosshair.generic.id		= ID_ACCUCROSSHAIR;
	s_bfpoptions.accucrosshair.generic.x		= BFPOPTIONS_X_POS;
	s_bfpoptions.accucrosshair.generic.y		= y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.simplehud.generic.type			= MTYPE_RADIOBUTTON;
	s_bfpoptions.simplehud.generic.name			= "Simple HUD:";
	s_bfpoptions.simplehud.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.simplehud.generic.callback		= BFPOptions_Event;
	s_bfpoptions.simplehud.generic.id			= ID_SIMPLEHUD;
	s_bfpoptions.simplehud.generic.x			= BFPOPTIONS_X_POS;
	s_bfpoptions.simplehud.generic.y			= y;

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

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.flightilt.generic.type     = MTYPE_RADIOBUTTON;
	s_bfpoptions.flightilt.generic.name		= "Flight Tilt:";
	s_bfpoptions.flightilt.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.flightilt.generic.callback = BFPOptions_Event;
	s_bfpoptions.flightilt.generic.id       = ID_FLIGHTILT;
	s_bfpoptions.flightilt.generic.x	    = BFPOPTIONS_X_POS;
	s_bfpoptions.flightilt.generic.y	    = y;

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

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.stfu.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.stfu.generic.name		= "Disable Voices:";
	s_bfpoptions.stfu.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.stfu.generic.callback	= BFPOptions_Event;
	s_bfpoptions.stfu.generic.id		= ID_STFU;
	s_bfpoptions.stfu.generic.x			= BFPOPTIONS_X_POS;
	s_bfpoptions.stfu.generic.y			= y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfpoptions.lowpolysphere.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.lowpolysphere.generic.name		= "Low Polycount Sphere:";
	s_bfpoptions.lowpolysphere.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.lowpolysphere.generic.callback	= BFPOptions_Event;
	s_bfpoptions.lowpolysphere.generic.id		= ID_LOWPOLYSPHERE;
	s_bfpoptions.lowpolysphere.generic.x		= BFPOPTIONS_X_POS;
	s_bfpoptions.lowpolysphere.generic.y		= y;

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

	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.menubg );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.barlog );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.banner );

	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.auratype );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.explotype);
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.viewpoint );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.fix3person );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.particlesfx );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.dynauralight );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.dynexplolights);
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.kitrailength);
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.beamcmpxy);
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.transaura );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.smallaura );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.ssjglow );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.accucrosshair );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.simplehud );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.chargealert );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.q3hitsfx );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.flightilt );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.bigheads );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.defaultskins );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.stfu );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.lowpolysphere );

//----------------------------Auras-------------------------------------//

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

//----------------------------Explosions-------------------------------------//

	explosionSmoke = trap_Cvar_VariableValue( "cg_explosionSmoke" );
	explosionShell = trap_Cvar_VariableValue( "cg_explosionShell" );
	particles = trap_Cvar_VariableValue( "cg_particles" );
	explosionring = trap_Cvar_VariableValue( "cg_explosionRing" );

	if ( explosionSmoke <= 0 && explosionShell <= 0 && explosionring <= 0 ) {
		s_bfpoptions.explotype.curvalue = WIMPY_EXPLO;
	} else if ( explosionSmoke >= 1 && explosionring >= 1 ) {
		s_bfpoptions.explotype.curvalue = WEAK_EXPLO;
	} else if ( explosionSmoke >= 1 && particles >= 1 && explosionring >= 1 ) {
		s_bfpoptions.explotype.curvalue = SO_SO_EXPLO;
	} else if ( explosionSmoke >= 1 && explosionShell >= 1 && particles >= 1 && explosionring >= 1 ) {
		s_bfpoptions.explotype.curvalue = HARDCORE_EXPLO;
	} else {
		s_bfpoptions.explotype.curvalue = WIMPY_EXPLO;
	}

//----------------------------Camera-------------------------------------//

	thirdperson = trap_Cvar_VariableValue( "cg_thirdPerson" );
	firstpersonvis = trap_Cvar_VariableValue( "cg_drawOwnModel" );

	if ( thirdperson == 1 ) {
		s_bfpoptions.viewpoint.curvalue = 0;
	} else if ( firstpersonvis == 1 ) {
		s_bfpoptions.viewpoint.curvalue = 2;
	} else if ( firstpersonvis == 0 ) {
		s_bfpoptions.viewpoint.curvalue = 1;
	}

//-----------------------------------------------------------------------//

	s_bfpoptions.kitrailength.curvalue  = trap_Cvar_VariableValue( "cg_kiTrail" );
	s_bfpoptions.beamcmpxy.curvalue  = trap_Cvar_VariableValue( "cg_beamTrail" );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.back );

	BFPOptions_SetMenuItems();
}
#undef SPRITE_AURA
#undef SHADER_AURA
#undef LIGHTWEIGHT_AURA
#undef POLYGON_AURA
#undef HIGHPOLYCOUNT_AURA
#undef WIMPY_EXPLO
#undef WEAK_EXPLO
#undef SO_SO_EXPLO
#undef HARDCORE_EXPLO


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
	BFPOptions_MenuInit();
	UI_PushMenu( &s_bfpoptions.menu );
}
