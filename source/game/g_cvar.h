#ifdef EXTERN_G_CVAR
	#define G_CVAR( vmCvar, cvarName, defaultString, cvarFlags, modificationCount, trackChange ) extern vmCvar_t vmCvar;
#endif

#ifdef DECLARE_G_CVAR
	#define G_CVAR( vmCvar, cvarName, defaultString, cvarFlags, modificationCount, trackChange ) vmCvar_t vmCvar;
#endif

#ifdef G_CVAR_LIST
	#define G_CVAR( vmCvar, cvarName, defaultString, cvarFlags, modificationCount, trackChange ) { & vmCvar, cvarName, defaultString, cvarFlags, modificationCount, trackChange },
#endif

// don't override the cheat state set by the system
G_CVAR( g_cheats, "sv_cheats", "", 0, 0, qfalse )

G_CVAR( g_restarted, "g_restarted", "0", CVAR_ROM, 0, qfalse )

// latched vars
G_CVAR( g_gametype, "g_gametype", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_LATCH, 0, qfalse )

G_CVAR( g_maxclients, "sv_maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse ) // allow this many total, including spectators
G_CVAR( g_maxGameClients, "g_maxGameClients", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse ) // allow this many active

// change anytime vars
G_CVAR( g_dmflags, "dmflags", "0", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qtrue )
G_CVAR( g_fraglimit, "fraglimit", "20", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue )
G_CVAR( g_timelimit, "timelimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue )
G_CVAR( g_capturelimit, "capturelimit", "8", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue )

G_CVAR( g_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO, 0, qfalse )

G_CVAR( g_friendlyFire, "g_friendlyFire", "0", CVAR_ARCHIVE, 0, qtrue )

G_CVAR( g_teamAutoJoin, "g_teamAutoJoin", "0", CVAR_ARCHIVE, 0, qfalse )
G_CVAR( g_teamForceBalance, "g_teamForceBalance", "0", CVAR_ARCHIVE, 0, qfalse )

G_CVAR( g_warmup, "g_warmup", "20", CVAR_ARCHIVE, 0, qtrue )
G_CVAR( g_doWarmup, "g_doWarmup", "0", 0, 0, qtrue )
G_CVAR( g_log, "g_log", "games.log", CVAR_ARCHIVE, 0, qfalse )
G_CVAR( g_logSync, "g_logSync", "0", CVAR_ARCHIVE, 0, qfalse )

G_CVAR( g_password, "g_password", "", CVAR_USERINFO, 0, qfalse )

G_CVAR( g_banIPs, "g_banIPs", "", CVAR_ARCHIVE, 0, qfalse )
G_CVAR( g_filterBan, "g_filterBan", "1", CVAR_ARCHIVE, 0, qfalse )

G_CVAR( g_needpass, "g_needpass", "0", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse )

G_CVAR( g_dedicated, "dedicated", "0", 0, 0, qfalse )

G_CVAR( g_speed, "g_speed", "320", 0, 0, qtrue )
G_CVAR( g_gravity, "g_gravity", "800", 0, 0, qtrue )
G_CVAR( g_knockback, "g_knockback", "1000", 0, 0, qtrue )
G_CVAR( g_quadfactor, "g_quadfactor", "3", 0, 0, qtrue )
G_CVAR( g_weaponRespawn, "g_weaponrespawn", "5", 0, 0, qtrue )
G_CVAR( g_weaponTeamRespawn, "g_weaponTeamRespawn", "30", 0, 0, qtrue )
G_CVAR( g_forcerespawn, "g_forcerespawn", "20", 0, 0, qtrue )
G_CVAR( g_inactivity, "g_inactivity", "0", 0, 0, qtrue )
G_CVAR( g_debugMove, "g_debugMove", "0", 0, 0, qfalse )
G_CVAR( g_debugDamage, "g_debugDamage", "0", 0, 0, qfalse )
G_CVAR( g_debugAlloc, "g_debugAlloc", "0", 0, 0, qfalse )
G_CVAR( g_motd, "g_motd", "", 0, 0, qfalse )
G_CVAR( g_blood, "com_blood", "1", 0, 0, qfalse )

G_CVAR( g_basePL, "g_basePL", "150", 0, 0, qtrue ) // BFP - Base powerlevel
G_CVAR( g_allowSpectatorChat, "g_allowSpectatorChat", "", 0, 0, qtrue ) // BFP - Allow spectator chat
G_CVAR( g_meleeDamage, "g_meleeDamage", "10", 0, 0, qtrue ) // BFP - Melee damage
G_CVAR( g_meleeDiveRange, "g_meleeDiveRange", "700", 0, 0, qtrue ) // BFP - Melee dive range
G_CVAR( g_meleeRange, "g_meleeRange", "32", 0, 0, qtrue ) // BFP - Melee range
G_CVAR( g_chargeDelay, "g_chargeDelay", "750", 0, 0, qtrue ) // BFP - Charge delay
G_CVAR( g_hitStun, "g_hitStun", "", 0, 0, qtrue ) // BFP - Hit stun
G_CVAR( g_meleeOnly, "g_meleeOnly", "", 0, 0, qtrue ) // BFP - Melee only
G_CVAR( g_noFlight, "g_noFlight", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qtrue ) // BFP - No flight
G_CVAR( g_plKillBonusPct, "g_plKillBonusPct", ".1", 0, 0, qtrue ) // BFP - Kill bonus percentage
G_CVAR( g_maxSpawnPL, "g_maxSpawnPL", "999", 0, 0, qtrue ) // BFP - Max spawn powerlevel
G_CVAR( g_flightCost, "g_flightCost", "50", 0, 0, qtrue ) // BFP - Flight cost
G_CVAR( g_flightCostPct, "g_flightCostPct", "0", 0, 0, qtrue ) // BFP - Flight cost percentage
G_CVAR( g_boostCost, "g_boostCost", "350", 0, 0, qtrue ) // BFP - Boost cost
G_CVAR( g_boostCostPct, "g_boostCostPct", "0", 0, 0, qtrue ) // BFP - Boost cost percentage

G_CVAR( g_blockCost, "g_blockCost", "2", CVAR_ARCHIVE, 0, qtrue ) // BFP - Block cost
G_CVAR( g_blockCostPct, "g_blockCostPct", "0", CVAR_ARCHIVE, 0, qtrue ) // BFP - Block cost percentage
G_CVAR( g_blockDelay, "g_blockDelay", "2", CVAR_ARCHIVE, 0, qtrue ) // BFP - Block delay
G_CVAR( g_blockLength, "g_blockLength", "2", CVAR_ARCHIVE, 0, qtrue ) // BFP - Block length

G_CVAR( g_kiRegen, "g_kiRegen", "0", 0, 0, qtrue ) // BFP - Ki regeneration
G_CVAR( g_kiRegenPct, "g_kiRegenPct", "0.6", 0, 0, qtrue ) // BFP - Ki regeneration percentage
G_CVAR( g_kiCharge, "g_kiCharge", "0", 0, 0, qtrue ) // BFP - Ki charge
G_CVAR( g_kiChargePct, "g_kiChargePct", "15", 0, 0, qtrue ) // BFP - Ki charge percentage

G_CVAR( g_podiumDist, "g_podiumDist", "80", 0, 0, qfalse )
G_CVAR( g_podiumDrop, "g_podiumDrop", "70", 0, 0, qfalse )

G_CVAR( g_allowVote, "g_allowVote", "1", CVAR_ARCHIVE, 0, qfalse )
G_CVAR( g_listEntity, "g_listEntity", "0", 0, 0, qfalse )

G_CVAR( g_smoothClients, "g_smoothClients", "1", 0, 0, qfalse )
G_CVAR( pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO, 0, qfalse )
G_CVAR( pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, 0, qfalse )

G_CVAR( g_rankings, "g_rankings", "0", 0, 0, qfalse )

#undef G_CVAR
