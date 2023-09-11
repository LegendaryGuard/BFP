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
#define ID_TRANSAURA        147
#define ID_SMALLAURA        148
#define ID_SSJGLOW          149
#define ID_ACCUCROSSHAIR    150
#define ID_SIMPLEHUD        151
#define ID_CHARGEALERT      152
#define ID_Q3HITSFX         153
#define ID_FLIGHTILT        154
#define ID_BIGHEADS         155
#define ID_DEFAULTSKINS    	156
#define ID_DISABLEVOICES   	157
#define ID_LOWPOLYSPHERE 	158
#define ID_BACK				159

static const char *auratype_items[] = {
	"High Polycount Aura",
	"Polygonal Aura",
	"Shader Aura",
	"Lightweight Aura",
	NULL
};

static const char *viewpoint_items[] = {
	"Third Person",
	"First Person",
	"First Person Vis",
	NULL
};

static const char* explotype_items[] = {
	"So-So",
	"Hardcore",
	"Wimpy",
	"Weak",
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
	menuradiobutton_s	disablevoices;
	menuradiobutton_s	lowpolysphere;
	menubitmap_s		back;

} bfpoptions_t;

static bfpoptions_t s_bfpoptions;

static void BFPOptions_SetMenuItems( void ) {
	s_bfpoptions.fix3person.curvalue		= trap_Cvar_VariableValue( "cg_fixedThirdPerson" ) != 0;
	s_bfpoptions.particlesfx.curvalue		= trap_Cvar_VariableValue( "cg_particles" ) != 0;
	s_bfpoptions.dynauralight.curvalue		= trap_Cvar_VariableValue( "cg_lightAuras" ) != 0;
	s_bfpoptions.dynexplolights.curvalue	= trap_Cvar_VariableValue( "cg_lightExplosions" ) != 0;
	s_bfpoptions.transaura.curvalue			= trap_Cvar_VariableValue( "cg_transformationAura" ) != 0;
	s_bfpoptions.smallaura.curvalue			= trap_Cvar_VariableValue( "cg_smallOwnAura" ) != 0;
	s_bfpoptions.ssjglow.curvalue			= trap_Cvar_VariableValue( "cg_permaglowUltimate" ) != 0;
	s_bfpoptions.accucrosshair.curvalue		= trap_Cvar_VariableValue( "cg_stableCrosshair" ) != 0;
	s_bfpoptions.simplehud.curvalue			= trap_Cvar_VariableValue( "cg_simpleHUD" ) != 0;
	s_bfpoptions.chargealert.curvalue		= trap_Cvar_VariableValue( "cg_chargeupAlert" ) != 0;
	s_bfpoptions.q3hitsfx.curvalue			= trap_Cvar_VariableValue( "cg_playHitSound" ) != 0;
	s_bfpoptions.flightilt.curvalue			= trap_Cvar_VariableValue( "cg_flytilt" ) != 0;
	s_bfpoptions.bigheads.curvalue			= trap_Cvar_VariableValue( "cg_superdeformed" ) != 0;
	s_bfpoptions.defaultskins.curvalue		= trap_Cvar_VariableValue( "cg_forceSkin" ) != 0;
	s_bfpoptions.disablevoices.curvalue		= trap_Cvar_VariableValue( "cg_stfu" ) != 0;
	s_bfpoptions.lowpolysphere.curvalue		= trap_Cvar_VariableValue( "cg_lowpolysphere" ) != 0;
}


static void BFPOptions_Event( void* ptr, int notification ) {
	if( notification != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {

		//-----------------------------Aura list---------------------------------//

	case ID_AURATYPE:
		if( s_bfpoptions.auratype.curvalue == 0 ) {
			trap_Cvar_SetValue( "cg_highPolyAura", 1 );
			trap_Cvar_SetValue( "cg_polygonAura", 1 );
			trap_Cvar_SetValue( "cg_lightweightAuras", 0 );
		}
		else if( s_bfpoptions.auratype.curvalue == 1 ) {
			trap_Cvar_SetValue( "cg_highPolyAura", 0 );
			trap_Cvar_SetValue( "cg_polygonAura", 1 );
			trap_Cvar_SetValue( "cg_lightweightAuras", 0 );
		}
		else if( s_bfpoptions.auratype.curvalue == 2 ) {
			trap_Cvar_SetValue( "cg_highPolyAura", 0 );
			trap_Cvar_SetValue( "cg_polygonAura", 0 );
			trap_Cvar_SetValue( "cg_lightweightAuras", 0 );
		}
		else if( s_bfpoptions.auratype.curvalue == 3 ) {
			trap_Cvar_SetValue( "cg_highPolyAura", 0 );
			trap_Cvar_SetValue( "cg_polygonAura", 0 );
			trap_Cvar_SetValue( "cg_lightweightAuras", 1 );
		}
		else if( s_bfpoptions.auratype.curvalue == 4 ) {
			trap_Cvar_SetValue( "cg_highPolyAura", 0 );
			trap_Cvar_SetValue( "cg_polygonAura", 0 );
			trap_Cvar_SetValue( "cg_lightweightAuras", 0 );
		}
		break;

		//---------------------------View point List---------------------------------------//

	case ID_VIEWPOINT:
		if( s_bfpoptions.viewpoint.curvalue == 0 ) {
			trap_Cvar_SetValue( "cg_thirdPerson", 1 );
			trap_Cvar_SetValue( "cg_drawOwnModel", 1 );
		}
		else if( s_bfpoptions.viewpoint.curvalue == 1 ) {
			trap_Cvar_SetValue( "cg_thirdPerson", 0 );
			trap_Cvar_SetValue( "cg_drawOwnModel", 0 );
		}
		else if( s_bfpoptions.viewpoint.curvalue == 2 ) {
			trap_Cvar_SetValue( "cg_thirdPerson", 0 );
			trap_Cvar_SetValue( "cg_drawOwnModel", 1 );
		}
		break;

		//---------------------------Explosion type list---------------------------------------//

	case ID_EXPLOTYPE:
		if ( s_bfpoptions.explotype.curvalue == 0 ) {
			trap_Cvar_SetValue( "cg_explosionShell", 1 );
			trap_Cvar_SetValue( "cg_explosionSmoke", 0 );
			trap_Cvar_SetValue( "cg_particles", 1 );
			trap_Cvar_SetValue( "cg_explosionRing", 1 );
		}
		else if ( s_bfpoptions.explotype.curvalue == 1 ) {
			trap_Cvar_SetValue( "cg_explosionShell", 1 );
			trap_Cvar_SetValue( "cg_explosionSmoke", 1 );
			trap_Cvar_SetValue( "cg_particles", 1 );
			trap_Cvar_SetValue( "cg_explosionRing", 1 );
		}
		else if ( s_bfpoptions.explotype.curvalue == 2 ) {
			trap_Cvar_SetValue( "cg_explosionShell", 0 );
			trap_Cvar_SetValue( "cg_explosionSmoke", 0 );
			trap_Cvar_SetValue( "cg_particles", 0 );
			trap_Cvar_SetValue( "cg_explosionRing", 0 );
		}
		else if ( s_bfpoptions.explotype.curvalue == 3 ) {
			trap_Cvar_SetValue( "cg_explosionShell", 1 );
			trap_Cvar_SetValue( "cg_explosionSmoke", 0 );
			trap_Cvar_SetValue( "cg_particles", 0 );
			trap_Cvar_SetValue( "cg_explosionRing", 1 );
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


	case ID_TRANSAURA:
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

	case ID_DISABLEVOICES:
		trap_Cvar_SetValue( "cg_stfu", s_bfpoptions.disablevoices.curvalue );
		break;

	case ID_LOWPOLYSPHERE:
		trap_Cvar_SetValue( "cg_lowpolysphere", s_bfpoptions.lowpolysphere.curvalue );
		break;

	case ID_BACK:
		UI_PopMenu();
		break;
	}
}


static void BFPOptions_MenuInit( void ) {
	int		y;
	int		highpolyaura, polygonalaura, lightweightaura;
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
	s_bfpoptions.transaura.generic.id          = ID_TRANSAURA;
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
	s_bfpoptions.disablevoices.generic.type		= MTYPE_RADIOBUTTON;
	s_bfpoptions.disablevoices.generic.name		= "Disable Voices:";
	s_bfpoptions.disablevoices.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfpoptions.disablevoices.generic.callback	= BFPOptions_Event;
	s_bfpoptions.disablevoices.generic.id		= ID_DISABLEVOICES;
	s_bfpoptions.disablevoices.generic.x		= BFPOPTIONS_X_POS;
	s_bfpoptions.disablevoices.generic.y		= y;

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
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.disablevoices );
	Menu_AddItem( &s_bfpoptions.menu, &s_bfpoptions.lowpolysphere );

//----------------------------Auras-------------------------------------//

	highpolyaura = trap_Cvar_VariableValue( "cg_highPolyAura" );
	polygonalaura = trap_Cvar_VariableValue( "cg_polygonAura" );
	lightweightaura = trap_Cvar_VariableValue( "cg_lightweightAuras" );

	if ( highpolyaura == 1 ) {
		s_bfpoptions.auratype.curvalue = 0;
	} else if ( polygonalaura == 1 )  {
		s_bfpoptions.auratype.curvalue = 1;
	} else if ( lightweightaura == 1 ) {
		s_bfpoptions.auratype.curvalue = 3;
	} else {
		s_bfpoptions.auratype.curvalue = 2;
	}

//----------------------------Explosions-------------------------------------//

	explosionSmoke = trap_Cvar_VariableValue( "cg_explosionSmoke" );
	explosionShell = trap_Cvar_VariableValue( "cg_explosionShell" );
	particles = trap_Cvar_VariableValue( "cg_particles" );
	explosionring = trap_Cvar_VariableValue( "cg_explosionRing" );

	if ( explosionSmoke == 1 ) {
		s_bfpoptions.explotype.curvalue = 1;
	} else if ( particles == 1 ) {
		s_bfpoptions.explotype.curvalue = 0;
	} else if ( explosionShell == 1 ) {
		s_bfpoptions.explotype.curvalue = 3;
	} else if ( explosionring == 0 ) {
		s_bfpoptions.explotype.curvalue = 2;
	}

//----------------------------Camera-------------------------------------//

	thirdperson = trap_Cvar_VariableValue( "cg_thirdPerson" );
	firstpersonvis = trap_Cvar_VariableValue( "cg_drawOwnModel" );

	if( thirdperson == 1 ) {
		s_bfpoptions.viewpoint.curvalue = 0;
	} else if( firstpersonvis == 1 ) {
		s_bfpoptions.viewpoint.curvalue = 2;
	} else if( firstpersonvis == 0 ) {
		s_bfpoptions.viewpoint.curvalue = 1;
	}

//-----------------------------------------------------------------------//

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
	BFPOptions_MenuInit();
	UI_PushMenu( &s_bfpoptions.menu );
}
