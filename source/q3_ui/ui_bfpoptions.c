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

#define BFPPREFERENCES_X_POS		360

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

} bfppreferences_t;

static bfppreferences_t s_bfppreferences;

static void BFPPreferences_SetMenuItems( void ) {
	s_bfppreferences.fix3person.curvalue		= trap_Cvar_VariableValue( "cg_fixedThirdPerson" ) != 0;
	s_bfppreferences.particlesfx.curvalue		= trap_Cvar_VariableValue( "cg_particles" ) != 0;
	s_bfppreferences.dynauralight.curvalue		= trap_Cvar_VariableValue( "cg_lightAuras" ) != 0;
	s_bfppreferences.dynexplolights.curvalue	= trap_Cvar_VariableValue( "cg_lightExplosions" ) != 0;
	s_bfppreferences.transaura.curvalue			= trap_Cvar_VariableValue( "cg_transformationAura" ) != 0;
	s_bfppreferences.smallaura.curvalue			= trap_Cvar_VariableValue( "cg_smallOwnAura" ) != 0;
	s_bfppreferences.ssjglow.curvalue			= trap_Cvar_VariableValue( "cg_permaglowUltimate" ) != 0;
	s_bfppreferences.accucrosshair.curvalue		= trap_Cvar_VariableValue( "cg_stableCrosshair" ) != 0;
	s_bfppreferences.simplehud.curvalue			= trap_Cvar_VariableValue( "cg_simpleHUD" ) != 0;
	s_bfppreferences.chargealert.curvalue		= trap_Cvar_VariableValue( "cg_chargeupAlert" ) != 0;
	s_bfppreferences.q3hitsfx.curvalue			= trap_Cvar_VariableValue( "cg_playHitSound" ) != 0;
	s_bfppreferences.flightilt.curvalue			= trap_Cvar_VariableValue( "cg_flytilt" ) != 0;
	s_bfppreferences.bigheads.curvalue			= trap_Cvar_VariableValue( "cg_superdeformed" ) != 0;
	s_bfppreferences.defaultskins.curvalue		= trap_Cvar_VariableValue( "cg_forceSkin" ) != 0;
	s_bfppreferences.disablevoices.curvalue		= trap_Cvar_VariableValue( "cg_stfu" ) != 0;
	s_bfppreferences.lowpolysphere.curvalue		= trap_Cvar_VariableValue( "cg_lowpolysphere" ) != 0;
}


static void BFPPreferences_Event( void* ptr, int notification ) {
	if( notification != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {

		//-----------------------------Aura list---------------------------------//

	case ID_AURATYPE:
		if( s_bfppreferences.auratype.curvalue == 0 ) {
			trap_Cvar_SetValue( "cg_highPolyAura", 1 );
			trap_Cvar_SetValue( "cg_polygonAura", 1 );
			trap_Cvar_SetValue( "cg_lightweightAuras", 0 );
		}
		else if( s_bfppreferences.auratype.curvalue == 1 ) {
			trap_Cvar_SetValue( "cg_highPolyAura", 0 );
			trap_Cvar_SetValue( "cg_polygonAura", 1 );
			trap_Cvar_SetValue( "cg_lightweightAuras", 0 );
		}
		else if( s_bfppreferences.auratype.curvalue == 2 ) {
			trap_Cvar_SetValue( "cg_highPolyAura", 0 );
			trap_Cvar_SetValue( "cg_polygonAura", 0 );
			trap_Cvar_SetValue( "cg_lightweightAuras", 0 );
		}
		else if( s_bfppreferences.auratype.curvalue == 3 ) {
			trap_Cvar_SetValue( "cg_highPolyAura", 0 );
			trap_Cvar_SetValue( "cg_polygonAura", 0 );
			trap_Cvar_SetValue( "cg_lightweightAuras", 1 );
		}
		else if( s_bfppreferences.auratype.curvalue == 4 ) {
			trap_Cvar_SetValue( "cg_highPolyAura", 0 );
			trap_Cvar_SetValue( "cg_polygonAura", 0 );
			trap_Cvar_SetValue( "cg_lightweightAuras", 0 );
		}
		break;

		//---------------------------View point List---------------------------------------//

	case ID_VIEWPOINT:
		if( s_bfppreferences.viewpoint.curvalue == 0 ) {
			trap_Cvar_SetValue( "cg_thirdPerson", 1 );
			trap_Cvar_SetValue( "cg_drawOwnModel", 1 );
		}
		else if( s_bfppreferences.viewpoint.curvalue == 1 ) {
			trap_Cvar_SetValue( "cg_thirdPerson", 0 );
			trap_Cvar_SetValue( "cg_drawOwnModel", 0 );
		}
		else if( s_bfppreferences.viewpoint.curvalue == 2 ) {
			trap_Cvar_SetValue( "cg_thirdPerson", 0 );
			trap_Cvar_SetValue( "cg_drawOwnModel", 1 );
		}
		break;

		//---------------------------Explosion type list---------------------------------------//

	case ID_EXPLOTYPE:
		if (s_bfppreferences.explotype.curvalue == 0) {
			trap_Cvar_SetValue( "cg_explosionShell", 1 );
			trap_Cvar_SetValue( "cg_explosionSmoke", 0 );
			trap_Cvar_SetValue( "cg_particles", 1 );
			trap_Cvar_SetValue( "cg_explosionRing", 1 );
		}
		else if (s_bfppreferences.explotype.curvalue == 1) {
			trap_Cvar_SetValue( "cg_explosionShell", 1 );
			trap_Cvar_SetValue( "cg_explosionSmoke", 1 );
			trap_Cvar_SetValue( "cg_particles", 1 );
			trap_Cvar_SetValue( "cg_explosionRing", 1 );
		}
		else if (s_bfppreferences.explotype.curvalue == 2) {
			trap_Cvar_SetValue( "cg_explosionShell", 0 );
			trap_Cvar_SetValue( "cg_explosionSmoke", 0 );
			trap_Cvar_SetValue( "cg_particles", 0 );
			trap_Cvar_SetValue( "cg_explosionRing", 0 );
		}
		else if (s_bfppreferences.explotype.curvalue == 3) {
			trap_Cvar_SetValue( "cg_explosionShell", 1 );
			trap_Cvar_SetValue( "cg_explosionSmoke", 0 );
			trap_Cvar_SetValue( "cg_particles", 0 );
			trap_Cvar_SetValue( "cg_explosionRing", 1 );
		}
		break;

		//---------------------------------------------------------------------//

	case ID_FIX3PERSON:
		trap_Cvar_SetValue( "cg_fixedThirdPerson", s_bfppreferences.fix3person.curvalue );
		break;

	case ID_PARTICLESFX:
		trap_Cvar_SetValue( "cg_particles", s_bfppreferences.particlesfx.curvalue );
		break;
	
	case ID_DYNAURALIGHT:
		trap_Cvar_SetValue( "cg_lightAuras", s_bfppreferences.dynauralight.curvalue );
		break;
	
	case ID_DYNEXPLOLIGHT:
		trap_Cvar_SetValue( "cg_lightExplosions", s_bfppreferences.dynexplolights.curvalue );
		break;



		//----------------------------Sliders------------------------------------//


	case ID_KITRAILENGTH:
		trap_Cvar_SetValue( "cg_kiTrail", s_bfppreferences.kitrailength.curvalue  );
		break;

	case ID_BEAMCMPXY:
		trap_Cvar_SetValue( "cg_beamTrail", s_bfppreferences.beamcmpxy.curvalue  );
		break;
	
		//-----------------------------------------------------------------------//


	case ID_TRANSAURA:
		trap_Cvar_SetValue( "cg_transformationAura", s_bfppreferences.transaura.curvalue );
		break;
	
	case ID_SMALLAURA:
		trap_Cvar_SetValue( "cg_smallOwnAura", s_bfppreferences.smallaura.curvalue );
		break;

	case ID_SSJGLOW:
		trap_Cvar_SetValue( "cg_permaglowUltimate", s_bfppreferences.ssjglow.curvalue );
		break;

	case ID_ACCUCROSSHAIR:
		trap_Cvar_SetValue( "cg_stableCrosshair", s_bfppreferences.accucrosshair.curvalue );
		break;

	case ID_SIMPLEHUD:
		trap_Cvar_SetValue( "cg_simpleHUD", s_bfppreferences.simplehud.curvalue );
		break;

	case ID_CHARGEALERT:
		trap_Cvar_SetValue( "cg_chargeupAlert", s_bfppreferences.chargealert.curvalue );
		break;

	case ID_Q3HITSFX:
		trap_Cvar_SetValue( "cg_playHitSound", s_bfppreferences.q3hitsfx.curvalue );
		break;

	case ID_FLIGHTILT:
		trap_Cvar_SetValue( "cg_flytilt", s_bfppreferences.flightilt.curvalue );
		break;

	case ID_BIGHEADS:
		trap_Cvar_SetValue( "cg_superdeformed", s_bfppreferences.bigheads.curvalue );
		break;

	case ID_DEFAULTSKINS:
		trap_Cvar_SetValue( "cg_forceSkin", s_bfppreferences.defaultskins.curvalue );
		break;

	case ID_DISABLEVOICES:
		trap_Cvar_SetValue( "cg_stfu", s_bfppreferences.disablevoices.curvalue );
		break;

	case ID_LOWPOLYSPHERE:
		trap_Cvar_SetValue( "cg_lowpolysphere", s_bfppreferences.lowpolysphere.curvalue );
		break;

	case ID_BACK:
		UI_PopMenu();
		break;
	}
}


static void BFPPreferences_MenuInit( void ) {
	int		y;
	int		highpolyaura;
	int		polygonalaura;
	int		lightweightaura;
	int 	thirdperson;
	int 	firstpersonvis;
	int 	explosionShell;
	int 	explosionSmoke;
	int 	particles;
	int		explosionring;

	memset( &s_bfppreferences, 0, sizeof(bfppreferences_t) );

	BFPPreferences_Cache();

	s_bfppreferences.menu.wrapAround = qtrue;
	s_bfppreferences.menu.fullscreen = qtrue;

	s_bfppreferences.menubg.generic.type = MTYPE_BITMAP;
	s_bfppreferences.menubg.generic.name = ART_MENUBG;
	s_bfppreferences.menubg.generic.flags = QMF_LEFT_JUSTIFY | QMF_INACTIVE;
	s_bfppreferences.menubg.generic.x = 0;
	s_bfppreferences.menubg.generic.y = 0;
	s_bfppreferences.menubg.width = 640;
	s_bfppreferences.menubg.height = 480;

	s_bfppreferences.barlog.generic.type = MTYPE_BITMAP;
	s_bfppreferences.barlog.generic.name = ART_BARLOG;
	s_bfppreferences.barlog.generic.flags = QMF_LEFT_JUSTIFY | QMF_INACTIVE;
	s_bfppreferences.barlog.generic.x = 140;
	s_bfppreferences.barlog.generic.y = 5;
	s_bfppreferences.barlog.width = 355;
	s_bfppreferences.barlog.height = 90;

	s_bfppreferences.banner.generic.type = MTYPE_PTEXT;
	s_bfppreferences.banner.generic.flags = QMF_CENTER_JUSTIFY | QMF_INACTIVE;
	s_bfppreferences.banner.generic.x = 320;
	s_bfppreferences.banner.generic.y = 45;
	s_bfppreferences.banner.string = "BFP OPTIONS";
	s_bfppreferences.banner.color = color_white;
	s_bfppreferences.banner.style = UI_CENTER | UI_BIGFONT;

	y = 90;
	s_bfppreferences.auratype.generic.type		= MTYPE_SPINCONTROL;
	s_bfppreferences.auratype.generic.name		= "Aura Type:";
	s_bfppreferences.auratype.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.auratype.generic.callback	= BFPPreferences_Event;
	s_bfppreferences.auratype.generic.id		= ID_AURATYPE;
	s_bfppreferences.auratype.generic.x			= 320;
	s_bfppreferences.auratype.generic.y			= y;
	s_bfppreferences.auratype.itemnames			= auratype_items;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.explotype.generic.type		= MTYPE_SPINCONTROL;
	s_bfppreferences.explotype.generic.name		= "Explosion Type:";
	s_bfppreferences.explotype.generic.flags	= QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_bfppreferences.explotype.generic.callback = BFPPreferences_Event;
	s_bfppreferences.explotype.generic.id		= ID_EXPLOTYPE;
	s_bfppreferences.explotype.generic.x		= 320;
	s_bfppreferences.explotype.generic.y		= y;
	s_bfppreferences.explotype.itemnames		= explotype_items;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.viewpoint.generic.type		= MTYPE_SPINCONTROL;
	s_bfppreferences.viewpoint.generic.name		= "Viewpoint:";
	s_bfppreferences.viewpoint.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.viewpoint.generic.callback	= BFPPreferences_Event;
	s_bfppreferences.viewpoint.generic.id		= ID_VIEWPOINT;
	s_bfppreferences.viewpoint.generic.x		= 320;
	s_bfppreferences.viewpoint.generic.y		= y;
	s_bfppreferences.viewpoint.itemnames		= viewpoint_items;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.fix3person.generic.type        = MTYPE_RADIOBUTTON;
	s_bfppreferences.fix3person.generic.name	    = "Fixed Third Person:";
	s_bfppreferences.fix3person.generic.flags	    = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.fix3person.generic.callback    = BFPPreferences_Event;
	s_bfppreferences.fix3person.generic.id          = ID_FIX3PERSON;
	s_bfppreferences.fix3person.generic.x	        = BFPPREFERENCES_X_POS;
	s_bfppreferences.fix3person.generic.y	        = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.particlesfx.generic.type        = MTYPE_RADIOBUTTON;
	s_bfppreferences.particlesfx.generic.name	      = "Particle Effects:";
	s_bfppreferences.particlesfx.generic.flags	      = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.particlesfx.generic.callback    = BFPPreferences_Event;
	s_bfppreferences.particlesfx.generic.id          = ID_PARTICLESFX;
	s_bfppreferences.particlesfx.generic.x	          = BFPPREFERENCES_X_POS;
	s_bfppreferences.particlesfx.generic.y	          = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.dynauralight.generic.type        = MTYPE_RADIOBUTTON;
	s_bfppreferences.dynauralight.generic.name	      = "Dynamic Aura Lights:";
	s_bfppreferences.dynauralight.generic.flags	      = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.dynauralight.generic.callback    = BFPPreferences_Event;
	s_bfppreferences.dynauralight.generic.id          = ID_DYNAURALIGHT;
	s_bfppreferences.dynauralight.generic.x	          = BFPPREFERENCES_X_POS;
	s_bfppreferences.dynauralight.generic.y	          = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.dynexplolights.generic.type        = MTYPE_RADIOBUTTON;
	s_bfppreferences.dynexplolights.generic.name	    = "Dynamic Explosion Lights:";
	s_bfppreferences.dynexplolights.generic.flags	    = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.dynexplolights.generic.callback    = BFPPreferences_Event;
	s_bfppreferences.dynexplolights.generic.id          = ID_DYNEXPLOLIGHT;
	s_bfppreferences.dynexplolights.generic.x	        = BFPPREFERENCES_X_POS;
	s_bfppreferences.dynexplolights.generic.y	        = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.kitrailength.generic.type		= MTYPE_SLIDER;
	s_bfppreferences.kitrailength.generic.name		= "Ki Trail Length:";
	s_bfppreferences.kitrailength.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.kitrailength.generic.callback	= BFPPreferences_Event;
	s_bfppreferences.kitrailength.generic.id		= ID_KITRAILENGTH;
	s_bfppreferences.kitrailength.generic.x	        = BFPPREFERENCES_X_POS;
	s_bfppreferences.kitrailength.generic.y	        = y;
	s_bfppreferences.kitrailength.minvalue			= 0;
	s_bfppreferences.kitrailength.maxvalue			= 100;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.beamcmpxy.generic.type		= MTYPE_SLIDER;
	s_bfppreferences.beamcmpxy.generic.name		= "Beam Complexity:";
	s_bfppreferences.beamcmpxy.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.beamcmpxy.generic.callback	= BFPPreferences_Event;
	s_bfppreferences.beamcmpxy.generic.id		= ID_BEAMCMPXY;
	s_bfppreferences.beamcmpxy.generic.x	    = BFPPREFERENCES_X_POS;
	s_bfppreferences.beamcmpxy.generic.y	    = y;
	s_bfppreferences.beamcmpxy.minvalue			= 0;
	s_bfppreferences.beamcmpxy.maxvalue			= 100;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.transaura.generic.type        = MTYPE_RADIOBUTTON;
	s_bfppreferences.transaura.generic.name		   = "Transformation Aura:";
	s_bfppreferences.transaura.generic.flags	   = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.transaura.generic.callback    = BFPPreferences_Event;
	s_bfppreferences.transaura.generic.id          = ID_TRANSAURA;
	s_bfppreferences.transaura.generic.x	       = BFPPREFERENCES_X_POS;
	s_bfppreferences.transaura.generic.y	       = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.smallaura.generic.type     = MTYPE_RADIOBUTTON;
	s_bfppreferences.smallaura.generic.name	  	= "Small Own Aura:";
	s_bfppreferences.smallaura.generic.flags    = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.smallaura.generic.callback = BFPPreferences_Event;
	s_bfppreferences.smallaura.generic.id       = ID_SMALLAURA;
	s_bfppreferences.smallaura.generic.x	    = BFPPREFERENCES_X_POS;
	s_bfppreferences.smallaura.generic.y	    = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.ssjglow.generic.type     = MTYPE_RADIOBUTTON;
	s_bfppreferences.ssjglow.generic.name	  = "SSJ Perma-Glow:";
	s_bfppreferences.ssjglow.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.ssjglow.generic.callback = BFPPreferences_Event;
	s_bfppreferences.ssjglow.generic.id       = ID_SSJGLOW;
	s_bfppreferences.ssjglow.generic.x	      = BFPPREFERENCES_X_POS;
	s_bfppreferences.ssjglow.generic.y	      = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.accucrosshair.generic.type     = MTYPE_RADIOBUTTON;
	s_bfppreferences.accucrosshair.generic.name	  = "Accurate Crosshair:";
	s_bfppreferences.accucrosshair.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.accucrosshair.generic.callback = BFPPreferences_Event;
	s_bfppreferences.accucrosshair.generic.id       = ID_ACCUCROSSHAIR;
	s_bfppreferences.accucrosshair.generic.x	      = BFPPREFERENCES_X_POS;
	s_bfppreferences.accucrosshair.generic.y	      = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.simplehud.generic.type     = MTYPE_RADIOBUTTON;
	s_bfppreferences.simplehud.generic.name	  = "Simple HUD:";
	s_bfppreferences.simplehud.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.simplehud.generic.callback = BFPPreferences_Event;
	s_bfppreferences.simplehud.generic.id       = ID_SIMPLEHUD;
	s_bfppreferences.simplehud.generic.x	      = BFPPREFERENCES_X_POS;
	s_bfppreferences.simplehud.generic.y	      = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.chargealert.generic.type     = MTYPE_RADIOBUTTON;
	s_bfppreferences.chargealert.generic.name	  = "Chargeup Alerts:";
	s_bfppreferences.chargealert.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.chargealert.generic.callback = BFPPreferences_Event;
	s_bfppreferences.chargealert.generic.id       = ID_CHARGEALERT;
	s_bfppreferences.chargealert.generic.x	      = BFPPREFERENCES_X_POS;
	s_bfppreferences.chargealert.generic.y	      = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.q3hitsfx.generic.type    	= MTYPE_RADIOBUTTON;
	s_bfppreferences.q3hitsfx.generic.name	  	= "Q3 Hit Sound:";
	s_bfppreferences.q3hitsfx.generic.flags	  	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.q3hitsfx.generic.callback	= BFPPreferences_Event;
	s_bfppreferences.q3hitsfx.generic.id      	= ID_Q3HITSFX;
	s_bfppreferences.q3hitsfx.generic.x	     	= BFPPREFERENCES_X_POS;
	s_bfppreferences.q3hitsfx.generic.y	    	= y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.flightilt.generic.type     = MTYPE_RADIOBUTTON;
	s_bfppreferences.flightilt.generic.name		= "Flight Tilt:";
	s_bfppreferences.flightilt.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.flightilt.generic.callback = BFPPreferences_Event;
	s_bfppreferences.flightilt.generic.id       = ID_FLIGHTILT;
	s_bfppreferences.flightilt.generic.x	    = BFPPREFERENCES_X_POS;
	s_bfppreferences.flightilt.generic.y	    = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.bigheads.generic.type     = MTYPE_RADIOBUTTON;
	s_bfppreferences.bigheads.generic.name	   = "Superdeformed Heads:";
	s_bfppreferences.bigheads.generic.flags	   = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.bigheads.generic.callback = BFPPreferences_Event;
	s_bfppreferences.bigheads.generic.id       = ID_BIGHEADS;
	s_bfppreferences.bigheads.generic.x  	   = BFPPREFERENCES_X_POS;
	s_bfppreferences.bigheads.generic.y	 	   = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.defaultskins.generic.type     = MTYPE_RADIOBUTTON;
	s_bfppreferences.defaultskins.generic.name	  = "Force Default Skins:";
	s_bfppreferences.defaultskins.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.defaultskins.generic.callback = BFPPreferences_Event;
	s_bfppreferences.defaultskins.generic.id       = ID_DEFAULTSKINS;
	s_bfppreferences.defaultskins.generic.x	      = BFPPREFERENCES_X_POS;
	s_bfppreferences.defaultskins.generic.y	      = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.disablevoices.generic.type     = MTYPE_RADIOBUTTON;
	s_bfppreferences.disablevoices.generic.name	  = "Disable Voices:";
	s_bfppreferences.disablevoices.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.disablevoices.generic.callback = BFPPreferences_Event;
	s_bfppreferences.disablevoices.generic.id       = ID_DISABLEVOICES;
	s_bfppreferences.disablevoices.generic.x	      = BFPPREFERENCES_X_POS;
	s_bfppreferences.disablevoices.generic.y	      = y;

	y += BIGCHAR_HEIGHT + 2;
	s_bfppreferences.lowpolysphere.generic.type     = MTYPE_RADIOBUTTON;
	s_bfppreferences.lowpolysphere.generic.name	  = "Low Polycount Sphere:";
	s_bfppreferences.lowpolysphere.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_bfppreferences.lowpolysphere.generic.callback = BFPPreferences_Event;
	s_bfppreferences.lowpolysphere.generic.id       = ID_LOWPOLYSPHERE;
	s_bfppreferences.lowpolysphere.generic.x	      = BFPPREFERENCES_X_POS;
	s_bfppreferences.lowpolysphere.generic.y	      = y;

	s_bfppreferences.back.generic.type	    = MTYPE_BITMAP;
	s_bfppreferences.back.generic.name     = ART_BACK0;
	s_bfppreferences.back.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_bfppreferences.back.generic.callback = BFPPreferences_Event;
	s_bfppreferences.back.generic.id	    = ID_BACK;
	s_bfppreferences.back.generic.x		= 0;
	s_bfppreferences.back.generic.y		= 480-80;
	s_bfppreferences.back.width  		    = 80;
	s_bfppreferences.back.height  		    = 80;
	s_bfppreferences.back.focuspic         = ART_BACK1;

	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.menubg );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.barlog );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.banner );

	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.auratype );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.explotype);
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.viewpoint );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.fix3person );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.particlesfx );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.dynauralight );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.dynexplolights);
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.kitrailength);
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.beamcmpxy);
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.transaura );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.smallaura );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.ssjglow );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.accucrosshair );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.simplehud );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.chargealert );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.q3hitsfx );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.flightilt );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.bigheads );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.defaultskins );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.disablevoices );
	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.lowpolysphere );

//----------------------------Auras-------------------------------------//

	highpolyaura = trap_Cvar_VariableValue( "cg_highPolyAura" );
	polygonalaura = trap_Cvar_VariableValue( "cg_polygonAura" );
	lightweightaura = trap_Cvar_VariableValue( "cg_lightweightAuras" );

	if ( highpolyaura == 1 ) {
		s_bfppreferences.auratype.curvalue = 0;
	} else if ( polygonalaura == 1 )  {
		s_bfppreferences.auratype.curvalue = 1;
	} else if ( lightweightaura == 1 ) {
		s_bfppreferences.auratype.curvalue = 3;
	} else {
		s_bfppreferences.auratype.curvalue = 2;
	}

//----------------------------Explosions-------------------------------------//

	explosionSmoke = trap_Cvar_VariableValue("cg_explosionSmoke");
	explosionShell = trap_Cvar_VariableValue("cg_explosionShell");
	particles = trap_Cvar_VariableValue("cg_particles");
	explosionring = trap_Cvar_VariableValue("cg_explosionRing");

	if ( explosionSmoke == 1 ) {
		s_bfppreferences.explotype.curvalue = 1;
	} else if ( particles == 1 ) {
		s_bfppreferences.explotype.curvalue = 0;
	} else if ( explosionShell == 1 ) {
		s_bfppreferences.explotype.curvalue = 3;
	} else if ( explosionring == 0 ) {
		s_bfppreferences.explotype.curvalue = 2;
	}

//----------------------------Camera-------------------------------------//

	thirdperson = trap_Cvar_VariableValue( "cg_thirdPerson" );
	firstpersonvis = trap_Cvar_VariableValue( "cg_drawOwnModel" );

	if( thirdperson == 1 ) {
		s_bfppreferences.viewpoint.curvalue = 0;
	} else if( firstpersonvis == 1 ) {
		s_bfppreferences.viewpoint.curvalue = 2;
	} else if( firstpersonvis == 0 ) {
		s_bfppreferences.viewpoint.curvalue = 1;
	}

	//-----------------------------------------------------------------------//

	s_bfppreferences.kitrailength.curvalue  = trap_Cvar_VariableValue( "cg_kiTrail" );
	s_bfppreferences.beamcmpxy.curvalue  = trap_Cvar_VariableValue( "cg_beamTrail" );

	Menu_AddItem( &s_bfppreferences.menu, &s_bfppreferences.back );

	BFPPreferences_SetMenuItems();
}


/*
===============
Preferences_Cache
===============
*/
void BFPPreferences_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_MENUBG );
	trap_R_RegisterShaderNoMip( ART_BARLOG );
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
}


/*
===============
UI_PreferencesMenu
===============
*/
void UI_BFPPreferencesMenu( void ) {
	BFPPreferences_MenuInit();
	UI_PushMenu( &s_bfppreferences.menu );
}
