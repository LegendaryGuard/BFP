#ifdef EXTERN_CG_CVAR
	#define CG_CVAR( vmCvar, cvarName, defaultString, cvarFlags ) extern vmCvar_t vmCvar;
#endif

#ifdef DECLARE_CG_CVAR
	#define CG_CVAR( vmCvar, cvarName, defaultString, cvarFlags ) vmCvar_t vmCvar;
#endif

#ifdef CG_CVAR_LIST
	#define CG_CVAR( vmCvar, cvarName, defaultString, cvarFlags ) { & vmCvar, cvarName, defaultString, cvarFlags },
#endif

CG_CVAR( cg_ignore, "cg_ignore", "0", 0 )	// used for debugging
CG_CVAR( cg_autoswitch, "cg_autoswitch", "1", CVAR_ARCHIVE )
CG_CVAR( cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE )
CG_CVAR( cg_zoomFov, "cg_zoomfov", "22.5", CVAR_ARCHIVE )
CG_CVAR( cg_fov, "cg_fov", "90", CVAR_ARCHIVE )
CG_CVAR( cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE )
CG_CVAR( cg_stereoSeparation, "cg_stereoSeparation", "0.4", CVAR_ARCHIVE )
CG_CVAR( cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE )
CG_CVAR( cg_gibs, "cg_gibs", "1", CVAR_ARCHIVE )
CG_CVAR( cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE )
CG_CVAR( cg_drawStatus, "cg_drawStatus", "1", CVAR_ARCHIVE )
CG_CVAR( cg_drawTimer, "cg_drawTimer", "0", CVAR_ARCHIVE )
CG_CVAR( cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE )
CG_CVAR( cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE )
CG_CVAR( cg_draw3dIcons, "cg_draw3dIcons", "1", CVAR_ARCHIVE )
CG_CVAR( cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE )
CG_CVAR( cg_drawAmmoWarning, "cg_drawAmmoWarning", "1", CVAR_ARCHIVE )
CG_CVAR( cg_drawAttacker, "cg_drawAttacker", "1", CVAR_ARCHIVE )
CG_CVAR( cg_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE )
CG_CVAR( cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE )
CG_CVAR( cg_drawRewards, "cg_drawRewards", "1", CVAR_ARCHIVE )
CG_CVAR( cg_chargeupAlert, "cg_chargeupAlert", "1", CVAR_ARCHIVE ) // BFP - Ready message when charging attacks
CG_CVAR( cg_crosshairColor, "cg_crosshairColor", "7", CVAR_ARCHIVE ) // BFP - Crosshair color
CG_CVAR( cg_crosshairSize, "cg_crosshairSize", "24", CVAR_ARCHIVE )
CG_CVAR( cg_crosshairHealth, "cg_crosshairHealth", "1", CVAR_ARCHIVE )
CG_CVAR( cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE )
CG_CVAR( cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE )
CG_CVAR( cg_brassTime, "cg_brassTime", "2500", CVAR_ARCHIVE )
CG_CVAR( cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE )
CG_CVAR( cg_addMarks, "cg_marks", "1", CVAR_ARCHIVE )
CG_CVAR( cg_particles, "cg_particles", "1", CVAR_ARCHIVE ) // BFP - Particles
CG_CVAR( cg_lagometer, "cg_lagometer", "1", CVAR_ARCHIVE )
CG_CVAR( cg_railTrailTime, "cg_railTrailTime", "400", CVAR_ARCHIVE )
CG_CVAR( cg_gun_frame, "cg_gun_frame", "", CVAR_ROM )
CG_CVAR( cg_gun_x, "cg_gunX", "0", CVAR_CHEAT )
CG_CVAR( cg_gun_y, "cg_gunY", "0", CVAR_CHEAT )
CG_CVAR( cg_gun_z, "cg_gunZ", "0", CVAR_CHEAT )
CG_CVAR( cg_centertime, "cg_centertime", "3", CVAR_CHEAT )
CG_CVAR( cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE )
CG_CVAR( cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE )
CG_CVAR( cg_bobup , "cg_bobup", "0.005", CVAR_CHEAT )
CG_CVAR( cg_bobpitch, "cg_bobpitch", "0.002", CVAR_ARCHIVE )
CG_CVAR( cg_bobroll, "cg_bobroll", "0.002", CVAR_ARCHIVE )
CG_CVAR( cg_swingSpeed, "cg_swingSpeed", "0.3", CVAR_CHEAT )
CG_CVAR( cg_animSpeed, "cg_animspeed", "1", CVAR_CHEAT )
CG_CVAR( cg_debugAnim, "cg_debuganim", "0", CVAR_CHEAT )
CG_CVAR( cg_debugPosition, "cg_debugposition", "0", CVAR_CHEAT )
CG_CVAR( cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT )
CG_CVAR( cg_errorDecay, "cg_errordecay", "100", 0 )
CG_CVAR( cg_nopredict, "cg_nopredict", "0", 0 )
CG_CVAR( cg_noPlayerAnims, "cg_noplayeranims", "0", CVAR_CHEAT )
CG_CVAR( cg_showmiss, "cg_showmiss", "0", 0 )
CG_CVAR( cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT )
CG_CVAR( cg_tracerChance, "cg_tracerchance", "0.4", CVAR_CHEAT )
CG_CVAR( cg_tracerWidth, "cg_tracerwidth", "1", CVAR_CHEAT )
CG_CVAR( cg_tracerLength, "cg_tracerlength", "100", CVAR_CHEAT )
CG_CVAR( cg_flytilt, "cg_flytilt", "1", CVAR_ARCHIVE ) // BFP - Fly tilt
CG_CVAR( cg_kiTrail, "cg_kiTrail", "50", CVAR_ARCHIVE ) // BFP - Ki trail length
CG_CVAR( cg_playHitSound, "cg_playHitSound", "0", CVAR_ARCHIVE ) // BFP - Play hit sound
CG_CVAR( cg_thirdPersonRange, "cg_thirdPersonRange", "110", CVAR_ARCHIVE ) // BFP
CG_CVAR( cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_ARCHIVE ) // BFP
CG_CVAR( cg_thirdPersonHeight, "cg_thirdPersonHeight", "-60", CVAR_ARCHIVE ) // BFP - Camera height
CG_CVAR( cg_fixedThirdPerson, "cg_fixedThirdPerson", "1", CVAR_ARCHIVE ) // BFP - Fixed third person camera
CG_CVAR( cg_drawOwnModel, "cg_drawOwnModel", "1", CVAR_ARCHIVE ) // BFP - toggle first person between traditional and vis modes
CG_CVAR( cg_drawKiWarning, "cg_drawKiWarning", "1", CVAR_ARCHIVE ) // BFP - Ki warning
CG_CVAR( cg_stableCrosshair, "cg_stableCrosshair", "0", CVAR_ARCHIVE ) // BFP - Accurate crosshair
CG_CVAR( cg_spriteAura, "cg_spriteAura", "0", CVAR_ARCHIVE ) // BFP - Sprite aura
CG_CVAR( cg_particleAura, "cg_particleAura", "0", CVAR_ARCHIVE ) // BFP - Particle aura
CG_CVAR( cg_lightAuras, "cg_lightAuras", "1", CVAR_ARCHIVE ) // BFP - Light auras
CG_CVAR( cg_smallOwnAura, "cg_smallOwnAura", "0", CVAR_ARCHIVE ) // BFP - Small own aura
CG_CVAR( cg_lightweightAuras, "cg_lightweightAuras", "0", CVAR_ARCHIVE ) // BFP - Lightweight auras
CG_CVAR( cg_polygonAura, "cg_polygonAura", "1", CVAR_ARCHIVE ) // BFP - Polygonal aura
CG_CVAR( cg_highPolyAura, "cg_highPolyAura", "1", CVAR_ARCHIVE ) // BFP - High polycount aura
CG_CVAR( cg_thirdPerson, "cg_thirdPerson", "1", 0 ) // BFP
CG_CVAR( cg_superdeformed, "cg_superdeformed", "0", CVAR_ARCHIVE ) // BFP - Super Deformed (Chibi style) easter egg
CG_CVAR( cg_yrgolroxor, "cg_yrgolroxor", "0", 0 ) // BFP - Yrgol Roxor easter egg
CG_CVAR( cg_teamChatTime, "cg_teamChatTime", "3000", CVAR_ARCHIVE )
CG_CVAR( cg_teamChatHeight, "cg_teamChatHeight", "0", CVAR_ARCHIVE )
CG_CVAR( cg_forceModel, "cg_forceModel", "0", CVAR_ARCHIVE ) // BFP - TODO: In the future, remove cg_forceModel, which wasn't removed originally?
CG_CVAR( cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE )
CG_CVAR( cg_deferPlayers, "cg_deferPlayers", "1", CVAR_ARCHIVE ) // BFP - TODO: In the future, remove cg_deferPlayers, which wasn't removed originally?
CG_CVAR( cg_drawTeamOverlay, "cg_drawTeamOverlay", "0", CVAR_ARCHIVE )
CG_CVAR( cg_teamOverlayUserinfo, "teamoverlay", "0", CVAR_ROM | CVAR_USERINFO )
CG_CVAR( cg_stats, "cg_stats", "0", 0 )
CG_CVAR( cg_drawFriend, "cg_drawFriend", "1", CVAR_ARCHIVE )
CG_CVAR( cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE )
CG_CVAR( cg_noVoiceChats, "cg_noVoiceChats", "0", CVAR_ARCHIVE )
CG_CVAR( cg_noVoiceText, "cg_noVoiceText", "0", CVAR_ARCHIVE )
CG_CVAR( cg_musicUnpacked, "cg_musicUnpacked", "0", CVAR_ARCHIVE ) // BFP - Unpack music
// the following variables are created in other parts of the system,
// but we also reference them here
CG_CVAR( cg_buildScript, "com_buildScript", "0", 0 )	// force loading of all possible data amd error on failures
CG_CVAR( cg_paused, "cl_paused", "0", CVAR_ROM )
CG_CVAR( cg_blood, "com_blood", "1", CVAR_ARCHIVE )
CG_CVAR( cg_synchronousClients, "g_synchronousClients", "0", 0 )	// communicated by systeminfo
CG_CVAR( cg_cameraOrbit, "cg_cameraOrbit", "0", CVAR_CHEAT )
CG_CVAR( cg_cameraOrbitDelay, "cg_cameraOrbitDelay", "50", CVAR_ARCHIVE )
CG_CVAR( cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0 )
CG_CVAR( cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0 )
CG_CVAR( cg_timescale, "timescale", "1", 0 )
CG_CVAR( cg_scorePlum, "cg_scorePlums", "1", CVAR_USERINFO | CVAR_ARCHIVE )
CG_CVAR( cg_smoothClients, "cg_smoothClients", "0", CVAR_USERINFO | CVAR_ARCHIVE )
// CG_CVAR( cg_cameraMode, "com_cameraMode", "0", CVAR_CHEAT ) // BFP - cg_cameraMode cvar doesn't exist

CG_CVAR( pmove_fixed, "pmove_fixed", "0", 0 )
CG_CVAR( pmove_msec, "pmove_msec", "8", 0 )
CG_CVAR( cg_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE )
CG_CVAR( cg_noProjectileTrail, "cg_noProjectileTrail", "0", CVAR_ARCHIVE )
CG_CVAR( cg_smallFont, "ui_smallFont", "0.25", CVAR_ARCHIVE )
CG_CVAR( cg_bigFont, "ui_bigFont", "0.4", CVAR_ARCHIVE )
CG_CVAR( cg_oldRail, "cg_oldRail", "1", CVAR_ARCHIVE )
CG_CVAR( cg_oldRocket, "cg_oldRocket", "1", CVAR_ARCHIVE )
CG_CVAR( cg_oldPlasma, "cg_oldPlasma", "1", CVAR_ARCHIVE )
CG_CVAR( cg_trueLightning, "cg_trueLightning", "0.0", CVAR_ARCHIVE )
//CG_CVAR( cg_pmove_fixed, "cg_pmove_fixed", "0", CVAR_USERINFO | CVAR_ARCHIVE )

#undef CG_CVAR
