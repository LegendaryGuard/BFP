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
// cg_players.c -- handle the media and animation for player entities
#include "cg_local.h"

char	*cg_customSoundNames[MAX_CUSTOM_SOUNDS] = {
	"*death1.wav",
	"*death2.wav",
	"*death3.wav",
	"*jump1.wav",
	"*pain25_1.wav",
	"*pain50_1.wav",
	"*pain75_1.wav",
	"*pain100_1.wav",
	"*falling1.wav",
	"*gasp.wav",
	// "*drown.wav", // BFP - No drowning
	"*fall1.wav",
	"*taunt.wav"
};


/*
================
CG_CustomSound

================
*/
sfxHandle_t	CG_CustomSound( int clientNum, const char *soundName ) {
	clientInfo_t *ci;
	int			i;

	if ( soundName[0] != '*' ) {
		return trap_S_RegisterSound( soundName, qfalse );
	}

	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}
	ci = &cgs.clientinfo[ clientNum ];

	for ( i = 0 ; i < MAX_CUSTOM_SOUNDS && cg_customSoundNames[i] ; i++ ) {
		if ( !strcmp( soundName, cg_customSoundNames[i] ) ) {
			return ci->sounds[i];
		}
	}

	CG_Error( "Unknown custom sound: %s", soundName );
	return 0;
}



/*
=============================================================================

CLIENT INFO

=============================================================================
*/

/*
======================
CG_ParseAnimationFile

Read a configuration file containing animation coutns and rates
models/players/visor/animation.cfg, etc
======================
*/
static qboolean	CG_ParseAnimationFile( const char *filename, clientInfo_t *ci ) {
	char		*text_p, *prev;
	int			len;
	int			i;
	char		*token;
	float		fps;
	int			skip;
	char		text[20000];
	fileHandle_t	f;
	animation_t *animations;

	animations = ci->animations;

	// load the file
	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( f == FS_INVALID_HANDLE ) {
		return qfalse;
	}
	if ( len <= 0 ) {
		trap_FS_FCloseFile( f );
		return qfalse;
	}
	if ( len >= sizeof( text ) - 1 ) {
		CG_Printf( "File %s too long\n", filename );
		return qfalse;
	}
	trap_FS_Read( text, len, f );
	text[len] = '\0';
	trap_FS_FCloseFile( f );

	// parse the text
	text_p = text;
	skip = 0;	// quite the compiler warning

	ci->footsteps = FOOTSTEP_NORMAL;
	VectorClear( ci->headOffset );
	ci->gender = GENDER_MALE;
	ci->fixedlegs = qfalse;
	ci->fixedtorso = qfalse;

	// read optional parameters
	while ( 1 ) {
		prev = text_p;	// so we can unget
		token = COM_Parse( &text_p );
		if ( !token[0] ) {
			break;
		}
		if ( !Q_stricmp( token, "footsteps" ) ) {
			token = COM_Parse( &text_p );
			if ( !token[0] ) {
				break;
			}
			if ( !Q_stricmp( token, "default" ) || !Q_stricmp( token, "normal" ) ) {
				ci->footsteps = FOOTSTEP_NORMAL;
			} else if ( !Q_stricmp( token, "boot" ) ) {
				ci->footsteps = FOOTSTEP_BOOT;
			} else if ( !Q_stricmp( token, "flesh" ) ) {
				ci->footsteps = FOOTSTEP_FLESH;
			} else if ( !Q_stricmp( token, "mech" ) ) {
				ci->footsteps = FOOTSTEP_MECH;
			} else if ( !Q_stricmp( token, "energy" ) ) {
				ci->footsteps = FOOTSTEP_ENERGY;
			} else {
				CG_Printf( "Bad footsteps parm in %s: %s\n", filename, token );
			}
			continue;
		} else if ( !Q_stricmp( token, "headoffset" ) ) {
			for ( i = 0 ; i < 3 ; i++ ) {
				token = COM_Parse( &text_p );
				if ( !token[0] ) {
					break;
				}
				ci->headOffset[i] = atof( token );
			}
			continue;
		} else if ( !Q_stricmp( token, "sex" ) ) {
			token = COM_Parse( &text_p );
			if ( !token[0] ) {
				break;
			}
			if ( token[0] == 'f' || token[0] == 'F' ) {
				ci->gender = GENDER_FEMALE;
			} else if ( token[0] == 'n' || token[0] == 'N' ) {
				ci->gender = GENDER_NEUTER;
			} else {
				ci->gender = GENDER_MALE;
			}
			continue;
		} else if ( !Q_stricmp( token, "fixedlegs" ) ) {
			ci->fixedlegs = qtrue;
			continue;
		} else if ( !Q_stricmp( token, "fixedtorso" ) ) {
			ci->fixedtorso = qtrue;
			continue;
		}

		// if it is a number, start parsing animations
		if ( token[0] >= '0' && token[0] <= '9' ) {
			text_p = prev;	// unget the token
			break;
		}
		Com_Printf( "unknown token '%s' in %s\n", token, filename );
	}

	// read information for each frame
	for ( i = 0 ; i < MAX_ANIMATIONS ; i++ ) {

		token = COM_Parse( &text_p );
		if ( !token[0] ) {
			if( i >= TORSO_GETFLAG && i <= TORSO_NEGATIVE ) {
				animations[i].firstFrame = animations[TORSO_GESTURE].firstFrame;
				animations[i].frameLerp = animations[TORSO_GESTURE].frameLerp;
				animations[i].initialLerp = animations[TORSO_GESTURE].initialLerp;
				animations[i].loopFrames = animations[TORSO_GESTURE].loopFrames;
				animations[i].numFrames = animations[TORSO_GESTURE].numFrames;
				animations[i].reversed = qfalse;
				animations[i].flipflop = qfalse;
				continue;
			}
			break;
		}
		animations[i].firstFrame = atoi( token );
		// leg only frames are adjusted to not count the upper body only frames
		if ( i == LEGS_WALKCR ) {
			skip = animations[LEGS_WALKCR].firstFrame - animations[TORSO_GESTURE].firstFrame;
		}
		if ( i >= LEGS_WALKCR && i<TORSO_GETFLAG) {
			animations[i].firstFrame -= skip;
		}

		token = COM_Parse( &text_p );
		if ( !token[0] ) {
			break;
		}
		animations[i].numFrames = atoi( token );

		animations[i].reversed = qfalse;
		animations[i].flipflop = qfalse;
		// if numFrames is negative the animation is reversed
		if (animations[i].numFrames < 0) {
			animations[i].numFrames = -animations[i].numFrames;
			animations[i].reversed = qtrue;
		}

		token = COM_Parse( &text_p );
		if ( !token[0] ) {
			break;
		}
		animations[i].loopFrames = atoi( token );

		token = COM_Parse( &text_p );
		if ( !token[0] ) {
			break;
		}
		fps = atof( token );
		if ( fps == 0 ) {
			fps = 1;
		}
		animations[i].frameLerp = 1000 / fps;
		animations[i].initialLerp = 1000 / fps;
	}

	if ( i != MAX_ANIMATIONS ) {
		CG_Printf( "Error parsing animation file: %s\n", filename );
		return qfalse;
	}

	// BFP - memcpy is not necessary, that can be removed
	// crouch backward animation
	// memcpy(&animations[LEGS_WALKCR], &animations[LEGS_WALKCR], sizeof(animation_t)); // BFP - Crouch backwards animation tweak
	animations[LEGS_WALKCR].reversed = qfalse; // BFP - Make the duck walking forward only
	// walk backward animation
	// memcpy(&animations[LEGS_WALK], &animations[LEGS_WALK], sizeof(animation_t)); // BFP - Walk backwards animation tweak
	animations[LEGS_WALK].reversed = qfalse; // BFP - Make the walk moving forward only
	// flag moving fast
	animations[FLAG_RUN].firstFrame = 0;
	animations[FLAG_RUN].numFrames = 16;
	animations[FLAG_RUN].loopFrames = 16;
	animations[FLAG_RUN].frameLerp = 1000 / 15;
	animations[FLAG_RUN].initialLerp = 1000 / 15;
	animations[FLAG_RUN].reversed = qfalse;
	// flag not moving or moving slowly
	animations[FLAG_STAND].firstFrame = 16;
	animations[FLAG_STAND].numFrames = 5;
	animations[FLAG_STAND].loopFrames = 0;
	animations[FLAG_STAND].frameLerp = 1000 / 20;
	animations[FLAG_STAND].initialLerp = 1000 / 20;
	animations[FLAG_STAND].reversed = qfalse;
	// flag speeding up
	animations[FLAG_STAND2RUN].firstFrame = 16;
	animations[FLAG_STAND2RUN].numFrames = 5;
	animations[FLAG_STAND2RUN].loopFrames = 1;
	animations[FLAG_STAND2RUN].frameLerp = 1000 / 15;
	animations[FLAG_STAND2RUN].initialLerp = 1000 / 15;
	animations[FLAG_STAND2RUN].reversed = qtrue;
	//
	// new anims changes
	//
//	animations[TORSO_GETFLAG].flipflop = qtrue;
//	animations[TORSO_GUARDBASE].flipflop = qtrue;
//	animations[TORSO_PATROL].flipflop = qtrue;
//	animations[TORSO_AFFIRMATIVE].flipflop = qtrue;
//	animations[TORSO_NEGATIVE].flipflop = qtrue;
	//
	return qtrue;
}

/*
==========================
CG_FileExists
==========================
*/
static qboolean	CG_FileExists( const char *filename ) {
	int len;
	fileHandle_t	f;

	len = trap_FS_FOpenFile( filename, &f, FS_READ );

	if ( f != FS_INVALID_HANDLE ) {
		trap_FS_FCloseFile( f );
	}

	if ( len > 0 ) {
		return qtrue;
	}

	return qfalse;
}

// BFP - No CG_FindClientModelFile, remove in the future?
#if 0
/*
==========================
CG_FindClientModelFile
==========================
*/
static qboolean	CG_FindClientModelFile( char *filename, int length, clientInfo_t *ci, const char *teamName, const char *modelName, const char *skinName, const char *base, const char *ext ) {
	char *team;
	int i;

	if ( cgs.gametype >= GT_TEAM ) {
		switch ( ci->team ) {
			case TEAM_BLUE: {
				team = "blue";
				break;
			}
			default: {
				team = "red";
				break;
			}
		}
	}
	else {
		team = "default";
	}
	while(1) {
		for ( i = 0; i < 2; i++ ) {
			if ( i == 0 && teamName && *teamName ) {
				//								"models/players/james/stroggs/lower_lily_red.skin"
				Com_sprintf( filename, length, "models/players/%s/%s%s_%s_%s.%s", modelName, teamName, base, skinName, team, ext );
			}
			else {
				//								"models/players/james/lower_lily_red.skin"
				Com_sprintf( filename, length, "models/players/%s/%s_%s_%s.%s", modelName, base, skinName, team, ext );
			}
			if ( CG_FileExists( filename ) ) {
				return qtrue;
			}

// BFP - Avoid loading team skins (causes crash)
#if 0
			if ( cgs.gametype >= GT_TEAM ) {
				if ( i == 0 && teamName && *teamName ) {
					//								"models/players/james/stroggs/lower_red.skin"
					Com_sprintf( filename, length, "models/players/%s/%s%s_%s.%s", modelName, teamName, base, team, ext );
				}
				else {
					//								"models/players/james/lower_red.skin"
					Com_sprintf( filename, length, "models/players/%s/%s_%s.%s", modelName, base, team, ext );
				}
			}
			else {
				if ( i == 0 && teamName && *teamName ) {
					//								"models/players/james/stroggs/lower_lily.skin"
					Com_sprintf( filename, length, "models/players/%s/%s%s_%s.%s", modelName, teamName, base, skinName, ext );
				}
				else {
					//								"models/players/james/lower_lily.skin"
					Com_sprintf( filename, length, "models/players/%s/%s_%s.%s", modelName, base, skinName, ext );
				}
			}
#endif

			Com_sprintf( filename, length, "models/players/%s/%s_%s.%s", modelName, base, skinName, ext );
			if ( CG_FileExists( filename ) ) {
				return qtrue;
			}
			if ( !teamName || !*teamName ) {
				break;
			}
		}
	}

	return qfalse;
}
#endif

/*
==========================
CG_FindClientHeadFile
==========================
*/
static qboolean	CG_FindClientHeadFile( char *filename, int length, clientInfo_t *ci, const char *teamName, const char *headModelName, const char *headSkinName, const char *base, const char *ext ) {
	char *team, *headsFolder;
	int i;

	if ( cgs.gametype >= GT_TEAM ) {
		switch ( ci->team ) {
			case TEAM_BLUE: {
				team = "blue";
				break;
			}
			default: {
				team = "red";
				break;
			}
		}
	}
	else {
		team = "default";
	}

	if ( headModelName[0] == '*' ) {
		headsFolder = "heads/";
		headModelName++;
	}
	else {
		headsFolder = "";
	}
	while(1) {
		for ( i = 0; i < 2; i++ ) {
			if ( i == 0 && teamName && *teamName ) {
				Com_sprintf( filename, length, "models/players/%s%s/%s/%s%s_%s.%s", headsFolder, headModelName, headSkinName, teamName, base, team, ext );
			}
			else {
				Com_sprintf( filename, length, "models/players/%s%s/%s/%s_%s.%s", headsFolder, headModelName, headSkinName, base, team, ext );
			}
			if ( CG_FileExists( filename ) ) {
				return qtrue;
			}

// BFP - Avoid loading team skins (causes crash)
#if 0
			if ( cgs.gametype >= GT_TEAM ) {
				if ( i == 0 &&  teamName && *teamName ) {
					Com_sprintf( filename, length, "models/players/%s%s/%s%s_%s.%s", headsFolder, headModelName, teamName, base, team, ext );
				}
				else {
					Com_sprintf( filename, length, "models/players/%s%s/%s_%s.%s", headsFolder, headModelName, base, team, ext );
				}
			}
			else {
				if ( i == 0 && teamName && *teamName ) {
					Com_sprintf( filename, length, "models/players/%s%s/%s%s_%s.%s", headsFolder, headModelName, teamName, base, headSkinName, ext );
				}
				else {
					Com_sprintf( filename, length, "models/players/%s%s/%s_%s.%s", headsFolder, headModelName, base, headSkinName, ext );
				}
			}
#endif

			Com_sprintf( filename, length, "models/players/%s%s/%s_%s.%s", headsFolder, headModelName, base, headSkinName, ext );
			if ( CG_FileExists( filename ) ) {
				return qtrue;
			}
			if ( !teamName || !*teamName ) {
				break;
			}
		}
		// if tried the heads folder first
		if ( headsFolder[0] ) {
			break;
		}
		headsFolder = "heads/";
	}

	return qfalse;
}

/*
==========================
CG_RegisterClientSkin
==========================
*/
static qboolean	CG_RegisterClientSkin( clientInfo_t *ci, const char *teamName, const char *modelName, const char *skinName, const char *headModelName, const char *headSkinName ) {
	char filename[MAX_QPATH];

	// BFP - That was a way to load the skins, so CG_FindClientModelFile function does the job, 
	// but it tends to have more memory load. Also the characters folder check is disabled, BFP doesn't have any check of this

	// BFP - Legs skin
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/lower_%s.skin", modelName, skinName );
	ci->legsSkin = trap_R_RegisterSkin( filename );
	if (!ci->legsSkin) {
		// Com_sprintf( filename, sizeof( filename ), "models/players/characters/%s/lower_%s.skin", modelName, skinName );
		// ci->legsSkin = trap_R_RegisterSkin( filename );
		// if (!ci->legsSkin) {
			Com_Printf( "Leg skin load failure: %s\n", filename );
		// }
	}

	// BFP - Ultimate tier legs skin
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/ssjlower.skin", modelName, skinName );
	ci->ultTierLegsSkin = trap_R_RegisterSkin( filename );
	// if (!ci->ultTierLegsSkin) {
		// Com_sprintf( filename, sizeof( filename ), "models/players/characters/%s/ssjlower.skin", modelName, skinName );
		// ci->ultTierLegsSkin = trap_R_RegisterSkin( filename );
		// if (!ci->ultTierLegsSkin) {
		//	Com_Printf( "Ultimate tier leg skin load failure: %s\n", filename );
		// }
	// }

	// BFP - Torso skin
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/upper_%s.skin", modelName, skinName );
	ci->torsoSkin = trap_R_RegisterSkin( filename );
	if (!ci->torsoSkin) {
		// Com_sprintf( filename, sizeof( filename ), "models/players/characters/%s/upper_%s.skin", modelName, skinName );
		// ci->torsoSkin = trap_R_RegisterSkin( filename );
		// if (!ci->torsoSkin) {
			Com_Printf( "Torso skin load failure: %s\n", filename );
		// }
	}

	// BFP - Ultimate tier torso skin
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/ssjtorso.skin", modelName, skinName );
	ci->ultTierTorsoSkin = trap_R_RegisterSkin( filename );
	// if (!ci->ultTierTorsoSkin) {
		// Com_sprintf( filename, sizeof( filename ), "models/players/characters/%s/ssjtorso.skin", modelName, skinName );
		// ci->ultTierTorsoSkin = trap_R_RegisterSkin( filename );
		// if (!ci->ultTierTorsoSkin) {
		//	Com_Printf( "Ultimate tier torso skin load failure: %s\n", filename );
		// }
	// }

	// BFP - Head skin
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/head_%s.skin", modelName, skinName );
	ci->headSkin = trap_R_RegisterSkin( filename );
	if (!ci->headSkin) {
		// Com_sprintf( filename, sizeof( filename ), "models/players/characters/%s/head_%s.skin", modelName, skinName );
		// ci->headSkin = trap_R_RegisterSkin( filename );
		// if (!ci->headSkin) {
			Com_Printf( "Head skin load failure: %s\n", filename );
		// }
	}

	// BFP - Ultimate tier head skin
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/ssjhead.skin", modelName, skinName );
	ci->ultTierHeadSkin = trap_R_RegisterSkin( filename );
	// if (!ci->ultTierHeadSkin) {
		// Com_sprintf( filename, sizeof( filename ), "models/players/characters/%s/ssjhead.skin", modelName, skinName );
		// ci->ultTierHeadSkin = trap_R_RegisterSkin( filename );
		// if (!ci->ultTierHeadSkin) {
		//	Com_Printf( "Ultimate tier head skin load failure: %s\n", filename );
		// }
	// }

	// BFP - Uses more memory load, not recommended, remove in the future?
#if 0
	if ( CG_FindClientModelFile( filename, sizeof(filename), ci, teamName, modelName, skinName, "lower", "skin" ) ) {
		ci->legsSkin = trap_R_RegisterSkin( filename );
	}
	if (!ci->legsSkin) {
		Com_Printf( "Leg skin load failure: %s\n", filename );
	}

	if ( CG_FindClientModelFile( filename, sizeof(filename), ci, teamName, modelName, skinName, "upper", "skin" ) ) {
		ci->torsoSkin = trap_R_RegisterSkin( filename );
	}
	if (!ci->torsoSkin) {
		Com_Printf( "Torso skin load failure: %s\n", filename );
	}

	if ( CG_FindClientHeadFile( filename, sizeof(filename), ci, teamName, headModelName, headSkinName, "head", "skin" ) ) {
		ci->headSkin = trap_R_RegisterSkin( filename );
	}
	if (!ci->headSkin) {
		Com_Printf( "Head skin load failure: %s\n", filename );
	}
#endif

	// if any skins failed to load
	if ( !ci->legsSkin || !ci->torsoSkin || !ci->headSkin ) {
		return qfalse;
	}
	return qtrue;
}

/*
==========================
CG_RegisterClientModelname
==========================
*/
static qboolean CG_RegisterClientModelname( clientInfo_t *ci, const char *modelName, const char *skinName, const char *headModelName, const char *headSkinName, const char *teamName ) {
	char	filename[MAX_QPATH*2];
	const char		*headName;
	char newTeamName[MAX_QPATH*2];

	// BFP - Most stuff is modified here to reduce memory load, 
	// anyone can modify and insert their own customized player folder/load error messages

	if ( headModelName[0] == '\0' ) {
		headName = modelName;
	}
	else {
		headName = headModelName;
	}
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/lower.md3", modelName );
	ci->legsModel = trap_R_RegisterModel( filename );
	if ( !ci->legsModel ) {
		// Com_sprintf( filename, sizeof( filename ), "models/players/characters/%s/lower.md3", modelName );
		// ci->legsModel = trap_R_RegisterModel( filename );
		// if ( !ci->legsModel ) {
			Com_Printf( "Failed to load model file %s\n", filename );
			return qfalse;
		// }
	}

	// BFP - Ultimate tier legs model
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/ssjlegs.md3", modelName );
	ci->ultTierLegsModel = trap_R_RegisterModel( filename );
	/*if ( !ci->ultTierLegsModel ) {
		Com_sprintf( filename, sizeof( filename ), "models/players/characters/%s/ssjlegs.md3", modelName );
		ci->ultTierLegsModel = trap_R_RegisterModel( filename );
		if ( !ci->ultTierLegsModel ) {
			Com_Printf( "Failed to load ultimate tier legs model file %s\n", filename );
			return qfalse;
		}
	}*/

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/upper.md3", modelName );
	ci->torsoModel = trap_R_RegisterModel( filename );
	if ( !ci->torsoModel ) {
		// Com_sprintf( filename, sizeof( filename ), "models/players/characters/%s/upper.md3", modelName );
		// ci->torsoModel = trap_R_RegisterModel( filename );
		// if ( !ci->torsoModel ) {
			Com_Printf( "Failed to load model file %s\n", filename );
			return qfalse;
		// }
	}

	// BFP - Ultimate tier torso model
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/ssjtorso.md3", modelName );
	ci->ultTierTorsoModel = trap_R_RegisterModel( filename );
	/*if ( !ci->ultTierTorsoModel ) {
		Com_sprintf( filename, sizeof( filename ), "models/players/characters/%s/ssjtorso.md3", modelName );
		ci->ultTierTorsoModel = trap_R_RegisterModel( filename );
		if ( !ci->ultTierTorsoModel ) {
			Com_Printf( "Failed to load ultimate tier torso model file %s\n", filename );
			return qfalse;
		}
	}*/

	// if( headName[0] == '*' ) {
		// Com_sprintf( filename, sizeof( filename ), "models/players/heads/%s/%s.md3", &headModelName[1], &headModelName[1] );
	// }
	// else {
		Com_sprintf( filename, sizeof( filename ), "models/players/%s/head.md3", headName );
	// }
	ci->headModel = trap_R_RegisterModel( filename );
	/*
	// if the head model could not be found and we didn't load from the heads folder try to load from there
	if ( !ci->headModel && headName[0] != '*' ) {
		Com_sprintf( filename, sizeof( filename ), "models/players/heads/%s/%s.md3", headModelName, headModelName );
		ci->headModel = trap_R_RegisterModel( filename );
	}
	*/
	if ( !ci->headModel ) {
		Com_Printf( "Failed to load model file %s\n", filename );
		return qfalse;
	}

	// BFP - Ultimate tier head model
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/ssjhead.md3", modelName );
	ci->ultTierHeadModel = trap_R_RegisterModel( filename );
	/*if ( !ci->ultTierHeadModel ) {
		Com_sprintf( filename, sizeof( filename ), "models/players/characters/%s/ssjhead.md3", modelName );
		ci->ultTierHeadModel = trap_R_RegisterModel( filename );
		if ( !ci->ultTierHeadModel ) {
			Com_Printf( "Failed to load ultimate tier head model file %s\n", filename );
			return qfalse;
		}
	}*/

	// if any skins failed to load, return failure
	if ( !CG_RegisterClientSkin( ci, teamName, modelName, skinName, headName, headSkinName ) ) {
		if ( teamName && *teamName) {
			Com_Printf( "Failed to load skin file: %s : %s : %s, %s : %s\n", teamName, modelName, skinName, headName, headSkinName );
			if( ci->team == TEAM_BLUE ) {
				Com_sprintf(newTeamName, sizeof(newTeamName), "%s/", DEFAULT_BLUETEAM_NAME);
			}
			else {
				Com_sprintf(newTeamName, sizeof(newTeamName), "%s/", DEFAULT_REDTEAM_NAME);
			}
			if ( !CG_RegisterClientSkin( ci, newTeamName, modelName, skinName, headName, headSkinName ) ) {
				Com_Printf( "Failed to load skin file: %s : %s : %s, %s : %s\n", newTeamName, modelName, skinName, headName, headSkinName );
				return qfalse;
			}
		} else {
			Com_Printf( "Failed to load skin file: %s : %s, %s : %s\n", modelName, skinName, headName, headSkinName );
			return qfalse;
		}
	}

	// load the animations
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/animation.cfg", modelName );
	if ( !CG_ParseAnimationFile( filename, ci ) ) {
		Com_sprintf( filename, sizeof( filename ), "models/players/characters/%s/animation.cfg", modelName );
		if ( !CG_ParseAnimationFile( filename, ci ) ) {
			Com_Printf( "Failed to load animation file %s\n", filename );
			return qfalse;
		}
	}

	if ( CG_FindClientHeadFile( filename, sizeof(filename), ci, teamName, headName, headSkinName, "icon", "skin" ) ) {
		ci->modelIcon = trap_R_RegisterShaderNoMip( filename );
	}
	else if ( CG_FindClientHeadFile( filename, sizeof(filename), ci, teamName, headName, headSkinName, "icon", "tga" ) ) {
		ci->modelIcon = trap_R_RegisterShaderNoMip( filename );
	}

	if ( !ci->modelIcon ) {
		return qfalse;
	}

	return qtrue;
}

	// BFP - No color1
#if 0
/*
====================
CG_ColorFromString
====================
*/
static void CG_ColorFromString( const char *v, vec3_t color ) {
	int val;

	VectorClear( color );

	val = atoi( v );

	if ( val < 1 || val > 7 ) {
		VectorSet( color, 1, 1, 1 );
		return;
	}

	if ( val & 1 ) {
		color[2] = 1.0f;
	}
	if ( val & 2 ) {
		color[1] = 1.0f;
	}
	if ( val & 4 ) {
		color[0] = 1.0f;
	}
}
#endif

/*
===================
CG_LoadClientInfo

Load it now, taking the disk hits.
This will usually be deferred to a safe time
===================
*/
static void CG_LoadClientInfo( clientInfo_t *ci ) {
	const char	*dir, *fallback;
	int			i, modelloaded;
	const char	*s;
	int			clientNum;
	char		teamname[MAX_QPATH];
	// BFP - Monster gamemode, slash to truncate the skinName in modelName
	char		*slash;

	teamname[0] = 0;
	modelloaded = qtrue;
	if ( !CG_RegisterClientModelname( ci, ci->modelName, ci->skinName, ci->headModelName, ci->headSkinName, teamname ) ) {
		if ( cg_buildScript.integer ) {
			CG_Error( "CG_RegisterClientModelname( %s, %s, %s, %s %s ) failed", ci->modelName, ci->skinName, ci->headModelName, ci->headSkinName, teamname );
			modelloaded = qfalse; // BFP - if the model didn't load
		}

// BFP - no team skin and model
#if 0
		// fall back to default team name
		if( cgs.gametype >= GT_TEAM) {
			// keep skin name
			if( ci->team == TEAM_BLUE ) {
				Q_strncpyz(teamname, DEFAULT_BLUETEAM_NAME, sizeof(teamname) );
			} else {
				Q_strncpyz(teamname, DEFAULT_REDTEAM_NAME, sizeof(teamname) );
			}
			if ( !CG_RegisterClientModelname( ci, DEFAULT_TEAM_MODEL, ci->skinName, DEFAULT_TEAM_HEAD, ci->skinName, teamname ) ) {
				CG_Error( "DEFAULT_TEAM_MODEL / skin (%s/%s) failed to register", DEFAULT_TEAM_MODEL, ci->skinName );
			}
		} else {
			if ( !CG_RegisterClientModelname( ci, DEFAULT_MODEL, "default", DEFAULT_MODEL, "default", teamname ) ) {
				CG_Error( "DEFAULT_MODEL (%s) failed to register", DEFAULT_MODEL );
			}
		}
#endif
	}

	ci->newAnims = qfalse;
	if ( ci->torsoModel ) {
		orientation_t tag;
		// if the torso model has the "tag_flag"
		if ( trap_R_LerpTag( &tag, ci->torsoModel, 0, 0, 1, "tag_flag" ) ) {
			ci->newAnims = qtrue;
		}
	}

	// sounds
	dir = ci->modelName;
	// BFP - Monster gamemode, get original model name to keep the player sounds
	if ( cgs.gametype == GT_MONSTER && cgs.monster > 0 ) {
		dir = ci->originalModelName;
		// truncate skinName (e.g. modelName/red --> modelName), otherwise it will get loading errors
		slash = strchr( dir, '/' );
		if ( slash ) {
			*slash = '\0';
		}
	}
	fallback = (cgs.gametype >= GT_TEAM) ? DEFAULT_TEAM_MODEL : DEFAULT_MODEL;

	for ( i = 0 ; i < MAX_CUSTOM_SOUNDS ; i++ ) {
		s = cg_customSoundNames[i];
		if ( !s ) {
			break;
		}
		ci->sounds[i] = 0;
		// if the model didn't load use the sounds of the default model
		if (modelloaded) {
			ci->sounds[i] = trap_S_RegisterSound( va("sound/player/%s/%s", dir, s + 1), qfalse );
		}
		if ( !ci->sounds[i] ) {
			ci->sounds[i] = trap_S_RegisterSound( va("sound/player/%s/%s", fallback, s + 1), qfalse );
		}
	}

	ci->deferred = qfalse;

	// reset any existing players and bodies, because they might be in bad
	// frames for this new model
	clientNum = ci - cgs.clientinfo;
	for ( i = 0 ; i < MAX_GENTITIES ; i++ ) {
		if ( cg_entities[i].currentState.clientNum == clientNum
			&& cg_entities[i].currentState.eType == ET_PLAYER ) {
			CG_ResetPlayerEntity( &cg_entities[i] );
		}
	}
}

// BFP - Unused static functions, remove? I think so
#if 0
/*
======================
CG_CopyClientInfoModel
======================
*/
static void CG_CopyClientInfoModel( clientInfo_t *from, clientInfo_t *to ) {
	VectorCopy( from->headOffset, to->headOffset );
	to->footsteps = from->footsteps;
	to->gender = from->gender;

	to->legsModel = from->legsModel;
	to->legsSkin = from->legsSkin;
	to->torsoModel = from->torsoModel;
	to->torsoSkin = from->torsoSkin;
	to->headModel = from->headModel;
	to->headSkin = from->headSkin;
	to->modelIcon = from->modelIcon;

	to->newAnims = from->newAnims;

	memcpy( to->animations, from->animations, sizeof( to->animations ) );
	memcpy( to->sounds, from->sounds, sizeof( to->sounds ) );
}

/*
======================
CG_ScanForExistingClientInfo
======================
*/
static qboolean CG_ScanForExistingClientInfo( clientInfo_t *ci ) {
	int		i;
	clientInfo_t	*match;

	for ( i = 0 ; i < cgs.maxclients ; i++ ) {
		match = &cgs.clientinfo[ i ];
		if ( !match->infoValid ) {
			continue;
		}
		if ( match->deferred ) {
			continue;
		}
		if ( !Q_stricmp( ci->modelName, match->modelName )
			&& !Q_stricmp( ci->skinName, match->skinName )
			&& !Q_stricmp( ci->headModelName, match->headModelName )
			&& !Q_stricmp( ci->headSkinName, match->headSkinName ) 
			&& !Q_stricmp( ci->blueTeam, match->blueTeam ) 
			&& !Q_stricmp( ci->redTeam, match->redTeam )
			&& (cgs.gametype < GT_TEAM || ci->team == match->team) ) {
			// this clientinfo is identical, so use it's handles

			ci->deferred = qfalse;

			CG_CopyClientInfoModel( match, ci );

			return qtrue;
		}
	}

	// nothing matches, so defer the load
	return qfalse;
}

/*
======================
CG_SetDeferredClientInfo

We aren't going to load it now, so grab some other
client's info to use until we have some spare time.
======================
*/
static void CG_SetDeferredClientInfo( clientInfo_t *ci ) {
	int		i;
	clientInfo_t	*match;

	// if someone else is already the same models and skins we
	// can just load the client info
	for ( i = 0 ; i < cgs.maxclients ; i++ ) {
		match = &cgs.clientinfo[ i ];
		if ( !match->infoValid || match->deferred ) {
			continue;
		}
		if ( Q_stricmp( ci->skinName, match->skinName ) ||
			 Q_stricmp( ci->modelName, match->modelName ) ||
//			 Q_stricmp( ci->headModelName, match->headModelName ) ||
//			 Q_stricmp( ci->headSkinName, match->headSkinName ) ||
			 (cgs.gametype >= GT_TEAM && ci->team != match->team) ) {
			continue;
		}
		// just load the real info cause it uses the same models and skins
		CG_LoadClientInfo( ci );
		return;
	}

	// if we are in teamplay, only grab a model if the skin is correct
	if ( cgs.gametype >= GT_TEAM ) {
		for ( i = 0 ; i < cgs.maxclients ; i++ ) {
			match = &cgs.clientinfo[ i ];
			if ( !match->infoValid || match->deferred ) {
				continue;
			}
			if ( Q_stricmp( ci->skinName, match->skinName ) ||
				(cgs.gametype >= GT_TEAM && ci->team != match->team) ) {
				continue;
			}
			ci->deferred = qtrue;
			CG_CopyClientInfoModel( match, ci );
			return;
		}
		// load the full model, because we don't ever want to show
		// an improper team skin.  This will cause a hitch for the first
		// player, when the second enters.  Combat shouldn't be going on
		// yet, so it shouldn't matter
		CG_LoadClientInfo( ci );
		return;
	}

	// find the first valid clientinfo and grab its stuff
	for ( i = 0 ; i < cgs.maxclients ; i++ ) {
		match = &cgs.clientinfo[ i ];
		if ( !match->infoValid ) {
			continue;
		}

		ci->deferred = qtrue;
		CG_CopyClientInfoModel( match, ci );
		return;
	}

	// we should never get here...
	CG_Printf( "CG_SetDeferredClientInfo: no valid clients!\n" );

	CG_LoadClientInfo( ci );
}
#endif


/*
======================
CG_NewClientInfo
======================
*/
void CG_NewClientInfo( int clientNum ) {
	clientInfo_t *ci;
	clientInfo_t newInfo;
	const char	*configstring;
	const char	*v;
	char		*slash;

	ci = &cgs.clientinfo[clientNum];

	configstring = CG_ConfigString( clientNum + CS_PLAYERS );
	if ( !configstring[0] ) {
		memset( ci, 0, sizeof( *ci ) );
		return;		// player just left
	}

	// build into a temp buffer so the defer checks can use
	// the old value
	memset( &newInfo, 0, sizeof( newInfo ) );

	// isolate the player's name
	v = Info_ValueForKey(configstring, "n");
	Q_strncpyz( newInfo.name, v, sizeof( newInfo.name ) );

	// BFP - No color1
#if 0
	// colors
	v = Info_ValueForKey( configstring, "c1" );
	CG_ColorFromString( v, newInfo.color1 );
#endif

	// bot skill
	v = Info_ValueForKey( configstring, "skill" );
	newInfo.botSkill = atoi( v );

	// BFP - No handicap
#if 0
	// handicap
	v = Info_ValueForKey( configstring, "hc" );
	newInfo.handicap = atoi( v );
#endif

	// wins
	v = Info_ValueForKey( configstring, "w" );
	newInfo.wins = atoi( v );

	// losses
	v = Info_ValueForKey( configstring, "l" );
	newInfo.losses = atoi( v );

	// team
	v = Info_ValueForKey( configstring, "t" );
	newInfo.team = atoi( v );

	// model
	v = Info_ValueForKey( configstring, "model" );
	// BFP - No force model (In the future, remove cg_forceModel, which wasn't removed originally?)
#if 0
	if ( cg_forceModel.integer ) {
		// forcemodel makes everyone use a single model
		// to prevent load hitches
		char modelStr[MAX_QPATH];
		char *skin;

		if( cgs.gametype >= GT_TEAM ) {
			Q_strncpyz( newInfo.modelName, DEFAULT_TEAM_MODEL, sizeof( newInfo.modelName ) );
			Q_strncpyz( newInfo.skinName, "default", sizeof( newInfo.skinName ) );
		} else {
			trap_Cvar_VariableStringBuffer( "model", modelStr, sizeof( modelStr ) );
			if ( ( skin = strchr( modelStr, '/' ) ) == NULL) {
				skin = "default";
			} else {
				*skin++ = 0;
			}

			Q_strncpyz( newInfo.skinName, skin, sizeof( newInfo.skinName ) );
			Q_strncpyz( newInfo.modelName, modelStr, sizeof( newInfo.modelName ) );
		}

		if ( cgs.gametype >= GT_TEAM ) {
			// keep skin name
			slash = strchr( v, '/' );
			if ( slash ) {
				Q_strncpyz( newInfo.skinName, slash + 1, sizeof( newInfo.skinName ) );
			}
		}
	} else 
#endif
	Q_strncpyz( newInfo.modelName, v, sizeof( newInfo.modelName ) );

	slash = strchr( newInfo.modelName, '/' );
	if ( !slash ) {
		// modelName didn not include a skin name
		Q_strncpyz( newInfo.skinName, "default", sizeof( newInfo.skinName ) );
	} else {
		Q_strncpyz( newInfo.skinName, slash + 1, sizeof( newInfo.skinName ) );
		// truncate modelName
		*slash = 0;
	}

	// BFP - Monster gamemode, get original model name with "omodel" to keep the player sounds
	if ( cgs.gametype == GT_MONSTER && cgs.monster > 0 ) {
		v = Info_ValueForKey( configstring, "omodel" );
		Q_strncpyz( newInfo.originalModelName, v, sizeof( newInfo.originalModelName ) );
		slash = strchr( newInfo.originalModelName, '/' );
		if ( !slash || !Q_stricmp( newInfo.modelName, MONSTER_NAME ) ) {
			// modelName didn not include a skin name
			Q_strncpyz( newInfo.skinName, "default", sizeof( newInfo.skinName ) );
		} else {
			Q_strncpyz( newInfo.skinName, slash + 1, sizeof( newInfo.skinName ) );
			// truncate modelName
			*slash = 0;
		}
	}

	// BFP - No force model (In the future, remove cg_forceModel, which wasn't removed originally?)
#if 0
	if ( cg_forceModel.integer ) {
		// forcemodel makes everyone use a single model
		// to prevent load hitches
		char modelStr[MAX_QPATH];
		char *skin;

		if( cgs.gametype >= GT_TEAM ) {
			Q_strncpyz( newInfo.headModelName, DEFAULT_TEAM_MODEL, sizeof( newInfo.headModelName ) );
			Q_strncpyz( newInfo.headSkinName, "default", sizeof( newInfo.headSkinName ) );
		} else {
			trap_Cvar_VariableStringBuffer( "headmodel", modelStr, sizeof( modelStr ) );
			if ( ( skin = strchr( modelStr, '/' ) ) == NULL) {
				skin = "default";
			} else {
				*skin++ = 0;
			}

			Q_strncpyz( newInfo.headSkinName, skin, sizeof( newInfo.headSkinName ) );
			Q_strncpyz( newInfo.headModelName, modelStr, sizeof( newInfo.headModelName ) );
		}

		if ( cgs.gametype >= GT_TEAM ) {
			// keep skin name
			slash = strchr( v, '/' );
			if ( slash ) {
				Q_strncpyz( newInfo.headSkinName, slash + 1, sizeof( newInfo.headSkinName ) );
			}
		}
	} else 
#endif

	// BFP - Set the head model name as the same as model name
	Q_strncpyz( newInfo.headModelName, newInfo.modelName, sizeof( newInfo.headModelName ) );
	slash = strchr( newInfo.headModelName, '/' );
	if ( !slash ) {
		// modelName didn not include a skin name
		Q_strncpyz( newInfo.headSkinName, "default", sizeof( newInfo.headSkinName ) );
	} else {
		Q_strncpyz( newInfo.headSkinName, slash + 1, sizeof( newInfo.headSkinName ) );
		// truncate modelName
		*slash = 0;
	}

	// BFP - Change the model without impeding with defer
	// BFP - TODO: Remove cg_deferPlayers cvar and its unnecessary code in the future
#if 0
	// scan for an existing clientinfo that matches this modelname
	// so we can avoid loading checks if possible
	// if ( !CG_ScanForExistingClientInfo( &newInfo ) ) 
	// {
		qboolean	forceDefer;

		forceDefer = trap_MemoryRemaining() < 4000000;

		// if we are defering loads, just have it pick the first valid
		if ( forceDefer || (cg_deferPlayers.integer && !cg_buildScript.integer && !cg.loading ) ) {
			// keep whatever they had if it won't violate team skins
			CG_SetDeferredClientInfo( &newInfo );
			// if we are low on memory, leave them with this model
			if ( forceDefer ) {
				CG_Printf( "Memory is low.  Using deferred model.\n" );
				newInfo.deferred = qfalse;
			}
		} else {
#endif
			CG_LoadClientInfo( &newInfo );
		// }
	// }

	// replace whatever was there with the new one
	newInfo.infoValid = qtrue;
	*ci = newInfo;
}



/*
======================
CG_LoadDeferredPlayers

Called each frame when a player is dead
and the scoreboard is up
so deferred players can be loaded
======================
*/
void CG_LoadDeferredPlayers( void ) {
	int		i;
	clientInfo_t	*ci;

	// scan for a deferred player to load
	for ( i = 0, ci = cgs.clientinfo ; i < cgs.maxclients ; i++, ci++ ) {
		if ( ci->infoValid && ci->deferred ) {
			// if we are low on memory, leave it deferred
			if ( trap_MemoryRemaining() < 4000000 ) {
				CG_Printf( "Memory is low.  Using deferred model.\n" );
				ci->deferred = qfalse;
				continue;
			}
			CG_LoadClientInfo( ci );
//			break;
		}
	}
}

/*
=============================================================================

PLAYER ANIMATION

=============================================================================
*/


/*
===============
CG_SetLerpFrameAnimation

may include ANIM_TOGGLEBIT
===============
*/
static void CG_SetLerpFrameAnimation( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation ) {
	animation_t	*anim;

	lf->animationNumber = newAnimation;
	newAnimation &= ~ANIM_TOGGLEBIT;

	if ( newAnimation < 0 || newAnimation >= MAX_TOTALANIMATIONS ) {
		CG_Error( "Bad animation number: %i", newAnimation );
	}

	anim = &ci->animations[ newAnimation ];

	lf->animation = anim;
	lf->animationTime = lf->frameTime + anim->initialLerp;

	if ( cg_debugAnim.integer ) {
		CG_Printf( "Anim: %i\n", newAnimation );
	}
}

/*
===============
CG_RunLerpFrame

Sets cg.snap, cg.oldFrame, and cg.backlerp
cg.time should be between oldFrameTime and frameTime after exit
===============
*/
static void CG_RunLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation, float speedScale ) {
	int			f, numFrames;
	animation_t	*anim;

	// debugging tool to get no animations
	if ( cg_animSpeed.integer == 0 ) {
		lf->oldFrame = lf->frame = lf->backlerp = 0;
		return;
	}

	// see if the animation sequence is switching
	if ( newAnimation != lf->animationNumber || !lf->animation ) {
		CG_SetLerpFrameAnimation( ci, lf, newAnimation );
	}

	// if we have passed the current frame, move it to
	// oldFrame and calculate a new frame
	if ( cg.time >= lf->frameTime ) {
		lf->oldFrame = lf->frame;
		lf->oldFrameTime = lf->frameTime;

		// get the next frame based on the animation
		anim = lf->animation;
		if ( !anim->frameLerp ) {
			return;		// shouldn't happen
		}
		if ( cg.time < lf->animationTime ) {
			lf->frameTime = lf->animationTime;		// initial lerp
		} else {
			lf->frameTime = lf->oldFrameTime + anim->frameLerp;
		}
		f = ( lf->frameTime - lf->animationTime ) / anim->frameLerp;
		f *= speedScale;		// adjust for haste, etc

		numFrames = anim->numFrames;
		if (anim->flipflop) {
			numFrames *= 2;
		}
		if ( f >= numFrames ) {
			f -= numFrames;
			if ( anim->loopFrames ) {
				f %= anim->loopFrames;
				f += anim->numFrames - anim->loopFrames;
			} else {
				f = numFrames - 1;
				// the animation is stuck at the end, so it
				// can immediately transition to another sequence
				lf->frameTime = cg.time;
			}
		}
		if ( anim->reversed ) {
			lf->frame = anim->firstFrame + anim->numFrames - 1 - f;
		}
		else if (anim->flipflop && f>=anim->numFrames) {
			lf->frame = anim->firstFrame + anim->numFrames - 1 - (f%anim->numFrames);
		}
		else {
			lf->frame = anim->firstFrame + f;
		}
		if ( cg.time > lf->frameTime ) {
			lf->frameTime = cg.time;
			if ( cg_debugAnim.integer ) {
				CG_Printf( "Clamp lf->frameTime\n");
			}
		}
	}

	if ( lf->frameTime > cg.time + 200 ) {
		lf->frameTime = cg.time;
	}

	if ( lf->oldFrameTime > cg.time ) {
		lf->oldFrameTime = cg.time;
	}
	// calculate current lerp value
	if ( lf->frameTime == lf->oldFrameTime ) {
		lf->backlerp = 0;
	} else {
		lf->backlerp = 1.0 - (float)( cg.time - lf->oldFrameTime ) / ( lf->frameTime - lf->oldFrameTime );
	}
}


/*
===============
CG_ClearLerpFrame
===============
*/
static void CG_ClearLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int animationNumber ) {
	lf->frameTime = lf->oldFrameTime = cg.time;
	CG_SetLerpFrameAnimation( ci, lf, animationNumber );
	lf->oldFrame = lf->frame = lf->animation->firstFrame;
}


/*
===============
CG_PlayerAnimation
===============
*/
static void CG_PlayerAnimation( centity_t *cent, int *legsOld, int *legs, float *legsBackLerp,
						int *torsoOld, int *torso, float *torsoBackLerp ) {
	clientInfo_t	*ci;
	int				clientNum;
	float			speedScale;

	clientNum = cent->currentState.clientNum;

	if ( cg_noPlayerAnims.integer ) {
		*legsOld = *legs = *torsoOld = *torso = 0;
		return;
	}

	// if ( cent->currentState.powerups & ( 1 << PW_HASTE ) ) {
	// BFP - When using ki boost use the following speed as haste powerup
	if ( ( cent->currentState.eFlags & EF_AURA )
	&& ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_RUN
		|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_BACK ) ) {
		speedScale = 1.5; // when using ki boost
	} else {
		speedScale = 1;
	}

	ci = &cgs.clientinfo[ clientNum ];

	// do the shuffle turn frames locally
	if ( cent->pe.legs.yawing && ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_IDLE ) {
		CG_RunLerpFrame( ci, &cent->pe.legs, LEGS_TURN, speedScale );
	} else {
		CG_RunLerpFrame( ci, &cent->pe.legs, cent->currentState.legsAnim, speedScale );
	}

	*legsOld = cent->pe.legs.oldFrame;
	*legs = cent->pe.legs.frame;
	*legsBackLerp = cent->pe.legs.backlerp;

	CG_RunLerpFrame( ci, &cent->pe.torso, cent->currentState.torsoAnim, speedScale );

	*torsoOld = cent->pe.torso.oldFrame;
	*torso = cent->pe.torso.frame;
	*torsoBackLerp = cent->pe.torso.backlerp;
}

/*
=============================================================================

PLAYER ANGLES

=============================================================================
*/

/*
==================
CG_SwingAngles
==================
*/
static void CG_SwingAngles( float destination, float swingTolerance, float clampTolerance,
					float speed, float *angle, qboolean *swinging ) {
	float	swing;
	float	move;
	float	scale;

	if ( !*swinging ) {
		// see if a swing should be started
		swing = AngleSubtract( *angle, destination );
		if ( swing > swingTolerance || swing < -swingTolerance ) {
			*swinging = qtrue;
		}
	}

	if ( !*swinging ) {
		return;
	}
	
	// modify the speed depending on the delta
	// so it doesn't seem so linear
	swing = AngleSubtract( destination, *angle );
	scale = fabs( swing );
	if ( scale < swingTolerance * 0.5 ) {
		scale = 0.5;
	} else if ( scale < swingTolerance ) {
		scale = 1.0;
	} else {
		scale = 2.0;
	}

	// swing towards the destination angle
	if ( swing >= 0 ) {
		move = cg.frametime * scale * speed;
		if ( move >= swing ) {
			move = swing;
			*swinging = qfalse;
		}
		*angle = AngleMod( *angle + move );
	} else if ( swing < 0 ) {
		move = cg.frametime * scale * -speed;
		if ( move <= swing ) {
			move = swing;
			*swinging = qfalse;
		}
		*angle = AngleMod( *angle + move );
	}

	// clamp to no more than tolerance
	swing = AngleSubtract( destination, *angle );
	if ( swing > clampTolerance ) {
		*angle = AngleMod( destination - (clampTolerance - 1) );
	} else if ( swing < -clampTolerance ) {
		*angle = AngleMod( destination + (clampTolerance - 1) );
	}
}

/*
=================
CG_AddPainTwitch
=================
*/
static void CG_AddPainTwitch( centity_t *cent, vec3_t torsoAngles ) {
	int		t;
	float	f;

	t = cg.time - cent->pe.painTime;
	if ( t >= PAIN_TWITCH_TIME ) {
		return;
	}

	f = 1.0 - (float)t / PAIN_TWITCH_TIME;

	if ( cent->pe.painDirection ) {
		torsoAngles[ROLL] += 20 * f;
	} else {
		torsoAngles[ROLL] -= 20 * f;
	}
}


/*
===============
CG_PlayerAngles

Handles seperate torso motion

  legs pivot based on direction of movement

  head always looks exactly at cent->lerpAngles

  if motion < 20 degrees, show in head only
  if < 45 degrees, also show in torso
===============
*/
static void CG_PlayerAngles( centity_t *cent, vec3_t legs[3], vec3_t torso[3], vec3_t head[3] ) {
	vec3_t		legsAngles, torsoAngles, headAngles;
	float		dest;
	static	int	movementOffsets[8] = { 0, 22, 45, -22, 0, 22, -45, -22 };
	vec3_t		velocity;
	float		speed;
	int			dir, clientNum;
	clientInfo_t	*ci;

	VectorCopy( cent->lerpAngles, headAngles );
	headAngles[YAW] = AngleMod( headAngles[YAW] );
	VectorClear( legsAngles );
	VectorClear( torsoAngles );

	// --------- yaw -------------

	// BFP - Allow yaw while flying too
	// allow yaw to drift a bit
	if ( ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_IDLE  
		|| ( cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_STAND )
			&& ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_FLYIDLE
			|| ( cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_STAND )
		&& ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_CHARGE
		|| ( cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_CHARGE ) ) {
		// if not standing still, always point all in the same direction
		cent->pe.torso.yawing = qtrue;	// always center
		cent->pe.torso.pitching = qtrue;	// always center
		cent->pe.legs.yawing = qtrue;	// always center
	}

	// adjust legs for movement dir
	if ( cent->currentState.eFlags & EF_DEAD ) {
		// don't let dead bodies twitch
		dir = 0;
	} else {
		dir = cent->currentState.angles2[YAW];
		if ( dir < 0 || dir > 7 ) {
			CG_Error( "Bad player movement angle" );
		}
	}
	legsAngles[YAW] = headAngles[YAW] + movementOffsets[ dir ];
	torsoAngles[YAW] = headAngles[YAW] + 0.25 * movementOffsets[ dir ];

	// BFP - Swing the angles to make the movements look smooth
	// torso
	CG_SwingAngles( torsoAngles[YAW], 40, 90, cg_swingSpeed.value, &cent->pe.torso.yawAngle, &cent->pe.torso.yawing ); // BFP - Before: 25, 90
	CG_SwingAngles( legsAngles[YAW], 90, 90, cg_swingSpeed.value, &cent->pe.legs.yawAngle, &cent->pe.legs.yawing ); // BFP - Before: 40, 90

	torsoAngles[YAW] = cent->pe.torso.yawAngle;
	legsAngles[YAW] = cent->pe.legs.yawAngle;


	// --------- pitch -------------

	// only show a fraction of the pitch angle in the torso
	if ( headAngles[PITCH] > 180 ) {
		dest = (-360 + headAngles[PITCH]) * 0.75f;
	} else {
		dest = headAngles[PITCH] * 0.75f;
	}
	// BFP - When flying, set the legs in the first case
	if ( cent->currentState.eFlags & EF_FLIGHT ) {
		CG_SwingAngles( dest, 15, 30, 0.1f, &cent->pe.legs.pitchAngle, &cent->pe.legs.pitching );
		legsAngles[PITCH] = cent->pe.legs.pitchAngle;
	} else if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_CHARGE ) {}

	// BFP - When flying, set the torso correctly into these angles
	CG_SwingAngles( dest, 30, 30, 0.1f, &cent->pe.torso.pitchAngle, &cent->pe.torso.pitching );
	torsoAngles[PITCH] = cent->pe.torso.pitchAngle;

	//
	clientNum = cent->currentState.clientNum;
	if ( clientNum >= 0 && clientNum < MAX_CLIENTS ) {
		ci = &cgs.clientinfo[ clientNum ];
		if ( ci->fixedtorso ) {
			torsoAngles[PITCH] = 0.0f;
		}
	}

	// --------- roll -------------

	// BFP - Prevent the player's head from tilting
	headAngles[ROLL] = 0;

	// lean towards the direction of travel
	VectorCopy( cent->currentState.pos.trDelta, velocity );
	speed = VectorNormalize( velocity );
	if ( speed ) {
		vec3_t	axis[3];
		float	side;

		// BFP - Speed handling when moving too much the angles
		if ( speed <= -480.0f ) speed = -480.0f;
		if ( speed >=  480.0f ) speed =  480.0f;

		speed *= 0.03f; // BFP - Adjust speed when rotate the angles (not a starting velocity), before: 0.05f

		AnglesToAxis( legsAngles, axis );
		side = speed * DotProduct( velocity, axis[1] );
		legsAngles[ROLL] -= side;

		side = speed * DotProduct( velocity, axis[0] );
		legsAngles[PITCH] += side;

		// BFP - Make the torso move the pitch angle a bit in the flight
		if ( cent->currentState.eFlags & EF_FLIGHT ) {
			AnglesToAxis( torsoAngles, axis );
			side = speed * DotProduct( velocity, axis[0] );
			torsoAngles[PITCH] += side;
		}
	}

	// BFP - Don't make every player forced to see this way with their legs to the others
#if 0
	//
	clientNum = cent->currentState.clientNum;
	if ( clientNum >= 0 && clientNum < MAX_CLIENTS ) {
		ci = &cgs.clientinfo[ clientNum ];
		if ( ci->fixedlegs ) {
			legsAngles[YAW] = torsoAngles[YAW];
			legsAngles[PITCH] = 0.0f;
			legsAngles[ROLL] = 0.0f;
		}
	}
#endif

	// pain twitch
	CG_AddPainTwitch( cent, torsoAngles );

	// pull the angles back out of the hierarchial chain
	AnglesSubtract( headAngles, torsoAngles, headAngles );
	AnglesSubtract( torsoAngles, legsAngles, torsoAngles );
	AnglesToAxis( legsAngles, legs );
	AnglesToAxis( torsoAngles, torso );
	AnglesToAxis( headAngles, head );
}


//==========================================================================

// BFP - No smoke puff effect
#if 0
/*
===============
CG_HasteTrail
===============
*/
static void CG_HasteTrail( centity_t *cent ) {
	localEntity_t	*smoke;
	vec3_t			origin;
	int				anim;

	if ( cent->trailTime > cg.time ) {
		return;
	}

	anim = cent->pe.legs.animationNumber & ~ANIM_TOGGLEBIT;
	if ( anim != LEGS_RUN && anim != LEGS_BACK ) {
		return;
	}

	cent->trailTime += 100;
	if ( cent->trailTime < cg.time ) {
		cent->trailTime = cg.time;
	}

	VectorCopy( cent->lerpOrigin, origin );
	origin[2] -= 16;

	smoke = CG_SmokePuff( origin, vec3_origin, 
				8,
				1, 1, 1, 1,
				500, 
				cg.time,
				0,
				0,
				cgs.media.hastePuffShader );

	// use the optimized local entity add
	smoke->leType = LE_SCALE_FADE;
}
#endif

/*
===============
CG_TrailItem
===============
*/
static void CG_TrailItem( centity_t *cent, qhandle_t hModel ) {
	refEntity_t		ent;
	vec3_t			angles;
	vec3_t			axis[3];

	// BFP - Don't show the model to the player itself
	if ( cent->currentState.clientNum == cg.snap->ps.clientNum ) {
		return;
	}

	VectorCopy( cent->lerpAngles, angles );
	angles[PITCH] = 0;
	angles[ROLL] = 0;
	AnglesToAxis( angles, axis );

	memset( &ent, 0, sizeof( ent ) );
	VectorMA( cent->lerpOrigin, -16, axis[0], ent.origin );
	ent.origin[2] += 16;
	angles[YAW] += 90;
	AnglesToAxis( angles, ent.axis );

	ent.hModel = hModel;
	trap_R_AddRefEntityToScene( &ent );
}


/*
===============
CG_PlayerFlag
===============
*/
static void CG_PlayerFlag( centity_t *cent, qhandle_t hSkin, refEntity_t *torso ) {
	clientInfo_t	*ci;
	refEntity_t	pole;
	refEntity_t	flag;
	vec3_t		angles, dir;
	int			legsAnim, flagAnim, updateangles;
	float		angle, d;

	// show the flag pole model
	memset( &pole, 0, sizeof(pole) );
	pole.hModel = cgs.media.flagPoleModel;
	VectorCopy( torso->lightingOrigin, pole.lightingOrigin );
	pole.shadowPlane = torso->shadowPlane;
	pole.renderfx = torso->renderfx;
	CG_PositionEntityOnTag( &pole, torso, torso->hModel, "tag_flag" );
	trap_R_AddRefEntityToScene( &pole );

	// show the flag model
	memset( &flag, 0, sizeof(flag) );
	flag.hModel = cgs.media.flagFlapModel;
	flag.customSkin = hSkin;
	VectorCopy( torso->lightingOrigin, flag.lightingOrigin );
	flag.shadowPlane = torso->shadowPlane;
	flag.renderfx = torso->renderfx;

	VectorClear(angles);

	updateangles = qfalse;
	legsAnim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if( legsAnim == LEGS_IDLE || legsAnim == LEGS_IDLECR ) {
		flagAnim = FLAG_STAND;
	} else if ( legsAnim == LEGS_WALK || legsAnim == LEGS_WALKCR ) {
		flagAnim = FLAG_STAND;
		updateangles = qtrue;
	} else {
		flagAnim = FLAG_RUN;
		updateangles = qtrue;
	}

	if ( updateangles ) {

		VectorCopy( cent->currentState.pos.trDelta, dir );
		// add gravity
		dir[2] += 100;
		VectorNormalize( dir );
		d = DotProduct(pole.axis[2], dir);
		// if there is anough movement orthogonal to the flag pole
		if (fabs(d) < 0.9) {
			//
			d = DotProduct(pole.axis[0], dir);
			if (d > 1.0f) {
				d = 1.0f;
			}
			else if (d < -1.0f) {
				d = -1.0f;
			}
			angle = acos(d);

			d = DotProduct(pole.axis[1], dir);
			if (d < 0) {
				angles[YAW] = 360 - angle * 180 / M_PI;
			}
			else {
				angles[YAW] = angle * 180 / M_PI;
			}
			if (angles[YAW] < 0)
				angles[YAW] += 360;
			if (angles[YAW] > 360)
				angles[YAW] -= 360;

			//vectoangles( cent->currentState.pos.trDelta, tmpangles );
			//angles[YAW] = tmpangles[YAW] + 45 - cent->pe.torso.yawAngle;
			// change the yaw angle
			CG_SwingAngles( angles[YAW], 25, 90, 0.15f, &cent->pe.flag.yawAngle, &cent->pe.flag.yawing );
		}

		/*
		d = DotProduct(pole.axis[2], dir);
		angle = Q_acos(d);

		d = DotProduct(pole.axis[1], dir);
		if (d < 0) {
			angle = 360 - angle * 180 / M_PI;
		}
		else {
			angle = angle * 180 / M_PI;
		}
		if (angle > 340 && angle < 20) {
			flagAnim = FLAG_RUNUP;
		}
		if (angle > 160 && angle < 200) {
			flagAnim = FLAG_RUNDOWN;
		}
		*/
	}

	// set the yaw angle
	angles[YAW] = cent->pe.flag.yawAngle;
	// lerp the flag animation frames
	ci = &cgs.clientinfo[ cent->currentState.clientNum ];
	CG_RunLerpFrame( ci, &cent->pe.flag, flagAnim, 1 );
	flag.oldframe = cent->pe.flag.oldFrame;
	flag.frame = cent->pe.flag.frame;
	flag.backlerp = cent->pe.flag.backlerp;

	AnglesToAxis( angles, flag.axis );
	CG_PositionRotatedEntityOnTag( &flag, &pole, pole.hModel, "tag_flag" );

	trap_R_AddRefEntityToScene( &flag );
}


/*
===============
CG_PlayerPowerups
===============
*/
static void CG_PlayerPowerups( centity_t *cent, refEntity_t *torso ) {
	int		powerups;
	clientInfo_t	*ci;

	powerups = cent->currentState.powerups;
	if ( !powerups ) {
		return;
	}

	// quad gives a dlight
	if ( powerups & ( 1 << PW_QUAD ) ) {
		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 0.2f, 0.2f, 1 );
	}

	// BFP - No flight powerup
#if 0
	// flight plays a looped sound
	if ( powerups & ( 1 << PW_FLIGHT ) ) {
		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.flightSound );
	}
#endif

	ci = &cgs.clientinfo[ cent->currentState.clientNum ];
	// redflag
	if ( powerups & ( 1 << PW_REDFLAG ) ) {
		if (ci->newAnims) {
			CG_PlayerFlag( cent, cgs.media.redFlagFlapSkin, torso );
		}
		else {
			CG_TrailItem( cent, cgs.media.redFlagModel );
		}
		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 1.0, 0.2f, 0.2f );
	}

	// blueflag
	if ( powerups & ( 1 << PW_BLUEFLAG ) ) {
		if (ci->newAnims){
			CG_PlayerFlag( cent, cgs.media.blueFlagFlapSkin, torso );
		}
		else {
			CG_TrailItem( cent, cgs.media.blueFlagModel );
		}
		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 0.2f, 0.2f, 1.0 );
	}

	// BFP - No haste powerup handling
#if 0
	// haste leaves smoke trails
	if ( powerups & ( 1 << PW_HASTE ) ) {
		CG_HasteTrail( cent );
	}
#endif
}


/*
===============
CG_KiAttackSounds
===============
*/
static void CG_KiAttackSounds( centity_t *cent ) { // BFP - Ki attack sounds
	switch ( cg.predictedPlayerState.weaponstate ) {
	case WEAPON_KIEXPLOSIONWAVE:
		trap_S_AddLoopingSound( cent->currentState.clientNum, cent->lerpOrigin, 
			vec3_origin, cgs.media.defaultKiBeamExplosionWaveSound );
		break;
	case WEAPON_BEAMFIRING:
		trap_S_AddLoopingSound( cent->currentState.clientNum, cent->lerpOrigin, 
			vec3_origin, cgs.media.defaultKiBeamExplosionWaveSound );
		break;
	case WEAPON_FIRING:
		// ki attacks like eyebeam shouldn't use that kind of firing sound
		if ( cg.predictedPlayerState.weapon != WP_LIGHTNING ) {
			trap_S_AddLoopingSound( cent->currentState.clientNum, cent->lerpOrigin, 
				vec3_origin, cgs.media.defaultKiFiringAttackSound );
		}
		break;
	case WEAPON_CHARGING:
		trap_S_AddLoopingSound( cent->currentState.clientNum, cent->lerpOrigin, 
			vec3_origin, cgs.media.defaultKiChargingSound );
	}
}


/*
===============
CG_PlayerFloatSprite

Float a sprite over the player's head
===============
*/
static void CG_PlayerFloatSprite( centity_t *cent, qhandle_t shader ) {
	int				rf;
	refEntity_t		ent;

	if ( cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson ) {
		rf = RF_THIRD_PERSON;		// only show in mirrors
	} else {
		rf = 0;
	}

	memset( &ent, 0, sizeof( ent ) );
	VectorCopy( cent->lerpOrigin, ent.origin );
	ent.origin[2] += 70; // BFP - BFP puts the floating sprite a bit up. Q3 default: 48
	ent.reType = RT_SPRITE;
	ent.customShader = shader;
	ent.radius = 10;
	ent.renderfx = rf;
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 255;
	// BFP - Monster gamemode, player monster' floating sprite is bigger and higher than a normal one
	if ( cent->currentState.eFlags & EF_MONSTER ) {
		ent.origin[2] += 250;
		ent.radius = 50;
	}
	trap_R_AddRefEntityToScene( &ent );
}


/*
===============
CG_FloatSpriteCheck

Check if there's some EF flag enabled, also don't show the float sprite to the player itself
===============
*/
static qboolean CG_FloatSpriteCheck( centity_t *cent, int eFlag, qhandle_t shader ) { // BFP - Check EF flag and don't show float sprite to the player itself
	if ( ( cent->currentState.eFlags & eFlag ) && cent->currentState.clientNum != cg.snap->ps.clientNum ) {
		CG_PlayerFloatSprite( cent, shader );
		return qtrue;
	}
	return qfalse;
}


/*
===============
CG_PlayerSprites

Float sprites over the player's head
===============
*/
static void CG_PlayerSprites( centity_t *cent ) {
	int		team;

	if ( CG_FloatSpriteCheck( cent, EF_CONNECTION,       cgs.media.connectionShader ) )	return;
	if ( CG_FloatSpriteCheck( cent, EF_TALK,             cgs.media.balloonShader ) )	return;
	if ( CG_FloatSpriteCheck( cent, EF_AWARD_EXCELLENT,  cgs.media.medalExcellent ) )	return;
	// BFP - No impressive, gauntlet, defend, assist and cap medals
#if 0
	if ( CG_FloatSpriteCheck( cent, EF_AWARD_IMPRESSIVE, cgs.media.medalImpressive ) )	return;
	if ( CG_FloatSpriteCheck( cent, EF_AWARD_GAUNTLET,   cgs.media.medalGauntlet ) )	return;
	if ( CG_FloatSpriteCheck( cent, EF_AWARD_DEFEND,     cgs.media.medalDefend ) )		return;
	if ( CG_FloatSpriteCheck( cent, EF_AWARD_ASSIST,     cgs.media.medalAssist ) )		return;
	if ( CG_FloatSpriteCheck( cent, EF_AWARD_CAP,        cgs.media.medalCapture ) )		return;
#endif

	team = cgs.clientinfo[ cent->currentState.clientNum ].team;
	if ( !(cent->currentState.eFlags & EF_DEAD) && 
		cent->currentState.clientNum != cg.snap->ps.clientNum && // BFP - Don't show the friend team shader to the player itself
		cg.snap->ps.persistant[PERS_TEAM] == team &&
		cgs.gametype >= GT_TEAM) {
		// BFP - BFP doesn't use cg_drawFriend to draw that floating friend sprite, keeps enabled always. 
		// Just wonder if it was some kind of logic reason and looks like that cvar to be removed was also forgotten
		// if (cg_drawFriend.integer) {
			CG_PlayerFloatSprite( cent, cgs.media.friendShader );
		// }
		return;
	}
}

/*
===============
CG_ChargeSmokeBubbles

Spawns charge smoke and bubble particles when being near something solid or water
===============
*/
static void CG_ChargeSmokeBubbles( centity_t *cent, vec3_t mins, vec3_t maxs,
					float chargeSmokeSize, float chargeSmokeRadialVel, 
					float chargeSmokeBaseRadius, float chargeSmokeUpOrigin,
					float bubbleSize, float bubbleRange ) { // BFP - Charging smoke and bubble particles
	int			distance = 300;
	vec3_t		end;
	trace_t		trace;
	vec3_t		chargeSmokePos;
	int			waterContents;

	// BFP - Monster gamemode, increase the distance for the particles if the player is the monster
	if ( cent->currentState.eFlags & EF_MONSTER ) {
		distance = 600;
	}
	
	// send a trace down from the player to the ground
	VectorCopy( cent->lerpOrigin, end );
	end[2] -= distance;

	trap_CM_BoxTrace( &trace, cent->lerpOrigin, end, mins, maxs, 0, MASK_PLAYERSOLID );

	VectorCopy( trace.endpos, chargeSmokePos );
	chargeSmokePos[2] += chargeSmokeUpOrigin; // put a bit above

	waterContents = CG_PointContents( trace.endpos, -1 ); // detect if the player is entirely under water
	if ( !( cent->currentState.eFlags & EF_KI_BOOST )
	&& ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_CHARGE
	&& !( waterContents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) )
#if 0 /* if the player isn't moving */
	&& !cent->currentState.pos.trDelta[0] 
	&& !cent->currentState.pos.trDelta[1] 
	&& !cent->currentState.pos.trDelta[2] 
#endif
	&& ( trace.fraction < 1.0f || cent->currentState.groundEntityNum != ENTITYNUM_NONE ) ) {
		// if stepping a mover
		if ( !( trace.fraction <= 0.75f )
		&& cent->currentState.groundEntityNum != ENTITYNUM_NONE ) {
			chargeSmokePos[2] += 280;
			// BFP - Monster gamemode, increase the charge smoke pos distance if the player is the monster
			if ( cent->currentState.eFlags & EF_MONSTER ) {
				chargeSmokePos[2] += 230;
			}
		}
		CG_ParticleChargeSmoke( cent, cgs.media.particleSmokeShader, chargeSmokePos, chargeSmokeSize, chargeSmokeRadialVel, chargeSmokeBaseRadius );
	}

	// water surface
	if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_RUN
	|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_BACK
	|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_FLYA
	|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_FLYB
	|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_CHARGE ) { // apply on ki charging status too
		trace_t	waterTrace;

		// tracing the water surface
		trap_CM_BoxTrace( &waterTrace, cent->lerpOrigin, end, mins, maxs, 0, CONTENTS_WATER );

		waterTrace.endpos[2] -= 20; // put a bit down to make the bubbles move
		if ( cent->currentState.eFlags & EF_MONSTER ) { // BFP - Monster gamemode, put a bit more down the bubbles
			waterTrace.endpos[2] -= 10;
		}
		if ( ( waterContents & CONTENTS_WATER ) 
		&& waterTrace.fraction >= 0.10f && waterTrace.fraction <= 0.70f ) {
			CG_ParticleBubble( cent, cgs.media.waterBubbleShader, waterTrace.endpos, end, 700, bubbleRange, bubbleSize );
			CG_ParticleBubble( cent, cgs.media.waterBubbleShader, waterTrace.endpos, end, 700, bubbleRange, bubbleSize );
			CG_ParticleBubble( cent, cgs.media.waterBubbleShader, waterTrace.endpos, end, 700, bubbleRange, bubbleSize );
		}
	}
}

/*
===============
CG_PlayerShadow

Returns the Z component of the surface being shadowed

  should it return a full plane instead of a Z?
===============
*/
static qboolean CG_PlayerShadow( centity_t *cent, float *shadowPlane ) {
	// BFP - Shadow distance variable isn't constant anymore
	int			shadow_distance = 128;
	vec3_t		end, mins = {-15, -15, 0}, maxs = {15, 15, 2};
	trace_t		trace;
	float		alpha;
	int			contents, waterContents; // BFP - To detect if there is water or lava
	float		radius = 24.0f; // BFP - Shadow radius

	// BFP - Bubble particle size and range
	float		bubbleSize = 2;
	float		bubbleRange = 10;

	// BFP - Dash smoke particle size, velocity and acceleration (dispersion)
	float		dashSmokeSize = 25;
	float		dashSmokeVelDisp = 401;
	float		dashSmokeUpVel = 20;
	float		dashSmokeAccel = 10;

	// BFP - Antigrav rock particle size, spawn range and end time
	float		antigravRockSize = 2;
	float		antigravRockSpawnRange = 50;
	float		antigravRockEndTime = 450;

	// BFP - Charge smoke particle size, radial velocity, base radius and up origin
	float		chargeSmokeSize = 40;
	float		chargeSmokeRadialVel = 450;
	float		chargeSmokeBaseRadius = 80;
	float		chargeSmokeUpOrigin = 20;

	// BFP - Monster gamemode, resize shadow and particles if the player is the monster
	if ( cent->currentState.eFlags & EF_MONSTER ) {
		shadow_distance = 400;
		mins[0] *= 2.5;
		mins[1] *= 2.5;

		maxs[0] *= 2.5;
		maxs[1] *= 2.5;
		radius = 180;
		bubbleSize = 8;
		bubbleRange = 150;

		dashSmokeSize = 120;
		dashSmokeVelDisp = 1201;
		dashSmokeUpVel = 1400;
		dashSmokeAccel = 260;

		antigravRockSize = 8;
		antigravRockSpawnRange = 220;
		antigravRockEndTime = 2050;

		chargeSmokeSize = 150;
		chargeSmokeRadialVel = 760;
		chargeSmokeBaseRadius = 300;
		chargeSmokeUpOrigin = 50;
	}

	*shadowPlane = 0;

	// send a trace down from the player to the ground
	VectorCopy( cent->lerpOrigin, end );
	end[2] -= shadow_distance;

	trap_CM_BoxTrace( &trace, cent->lerpOrigin, end, mins, maxs, 0, MASK_PLAYERSOLID );

	// BFP - Dash smoke and bubble particles when using ki boost on the ground or above the water
	contents = CG_PointContents( trace.endpos, -1 );
	if ( ( cent->currentState.eFlags & EF_AURA ) || ( cent->currentState.eFlags & EF_AURA_TIER_UP ) ) {
		if ( ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_RUN
		|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_BACK
		|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_FLYA
		|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_FLYB
		|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_CHARGE ) // apply on ki charging status too
			&& ( !( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) 
			&& ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_CHARGE
		&& ( trace.fraction <= 0.70f
		// If the player is stepping a mover:
		|| cent->currentState.groundEntityNum != ENTITYNUM_NONE ) ) ) {
			// BFP - Apply dash smoke particle for the trail, if the function were used directly, it would generate too many particles than we expected
			vec3_t	dashSmokePos;
			VectorCopy( trace.endpos, dashSmokePos );

			// if stepping a mover
			if ( !( trace.fraction <= 0.70f )
			&& cent->currentState.groundEntityNum != ENTITYNUM_NONE ) {
				dashSmokePos[2] += 100;
			}
			CG_ParticleDashSmoke( cent, cgs.media.particleSmokeShader, dashSmokePos, dashSmokeSize, dashSmokeVelDisp, dashSmokeUpVel, dashSmokeAccel );
		}

		// BFP - Charging smoke and bubble particles
		CG_ChargeSmokeBubbles( cent, mins, maxs, 
					chargeSmokeSize, chargeSmokeRadialVel, 
					chargeSmokeBaseRadius, chargeSmokeUpOrigin, 
					bubbleSize, bubbleRange );

		waterContents = CG_PointContents( cent->lerpOrigin, -1 ); // BFP - Detect if the player is entirely under water
		// BFP - Antigrav rock particles on ki charging status
		if ( !( cent->currentState.eFlags & EF_KI_BOOST )
		&& ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_CHARGE
		&& !( waterContents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) )
#if 0 /* if the player isn't moving */
		&& !cent->currentState.pos.trDelta[0] 
		&& !cent->currentState.pos.trDelta[1] 
		&& !cent->currentState.pos.trDelta[2] 
#endif
		&& ( trace.fraction <= 0.75f
		// If the player is stepping a mover:
		|| cent->currentState.groundEntityNum != ENTITYNUM_NONE )
		&& !( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) ) {
			// BFP - Spawn randomly the antigrav rock shaders with the particles
			int		shaderIndex = rand() % 3;
			vec3_t	antigravRockPos;
			VectorCopy( trace.endpos, antigravRockPos );

			// if stepping a mover
			if ( !( trace.fraction <= 0.75f )
			&& cent->currentState.groundEntityNum != ENTITYNUM_NONE ) {
				antigravRockPos[2] += 100;
			}
			switch ( shaderIndex ) {
				case 0: {
					CG_ParticleAntigravRock( cgs.media.pebbleShader1, cent, cent->currentState.clientNum, antigravRockPos, antigravRockSize, antigravRockSpawnRange, antigravRockEndTime );
					break;
				}
				case 1: {
					CG_ParticleAntigravRock( cgs.media.pebbleShader2, cent, cent->currentState.clientNum, antigravRockPos, antigravRockSize, antigravRockSpawnRange, antigravRockEndTime );
					break;
				}
				default: {
					CG_ParticleAntigravRock( cgs.media.pebbleShader3, cent, cent->currentState.clientNum, antigravRockPos, antigravRockSize, antigravRockSpawnRange, antigravRockEndTime );
				}
			}
		}
	}

	if ( cg_shadows.integer == 0 ) {
		return qfalse;
	}

	// no shadows when invisible
	if ( cent->currentState.powerups & ( 1 << PW_INVIS ) ) {
		return qfalse;
	}

	// no shadow if too high
	if ( trace.fraction == 1.0 || trace.startsolid || trace.allsolid ) {
		return qfalse;
	}

	*shadowPlane = trace.endpos[2] + 1;

	if ( cg_shadows.integer != 1 ) {	// no mark for stencil or projection shadows
		return qtrue;
	}

	// fade the shadow out with height
	alpha = 1.0 - trace.fraction;

	// bk0101022 - hack / FPE - bogus planes?
	//assert( DotProduct( trace.plane.normal, trace.plane.normal ) != 0.0f ) 

	// add the mark as a temporary, so it goes directly to the renderer
	// without taking a spot in the cg_marks array
	CG_ImpactMark( cgs.media.shadowMarkShader, trace.endpos, trace.plane.normal, 
		cent->pe.legs.yawAngle, alpha,alpha,alpha,1, qfalse, radius, qtrue );

	return qtrue;
}


/*
===============
CG_PlayerSplash

Draw a mark at the water surface
===============
*/
static void CG_PlayerSplash( centity_t *cent ) {
	vec3_t		start, end;
	trace_t		trace;
	int			contents;
	polyVert_t	verts[4];
	float		markSize = 32;

	if ( !cg_shadows.integer ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, end );
	end[2] -= 24;

	// BFP - Monster gamemode, player monster water surface mark size and position
	if ( cent->currentState.eFlags & EF_MONSTER ) {
		markSize = 192;
		end[2] -= 114;
	}

	// if the feet aren't in liquid, don't make a mark
	// this won't handle moving water brushes, but they wouldn't draw right anyway...
	contents = trap_CM_PointContents( end, 0 );
	if ( !( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, start );
	start[2] += 32;

	// BFP - Monster gamemode, player monster water surface mark position
	if ( cent->currentState.eFlags & EF_MONSTER ) {
		start[2] += 164;
	}

	// if the head isn't out of liquid, don't make a mark
	contents = trap_CM_PointContents( start, 0 );
	if ( contents & ( CONTENTS_SOLID | CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		return;
	}

	// trace down to find the surface
	trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) );

	if ( trace.fraction == 1.0 ) {
		return;
	}

	// create a mark polygon
	VectorCopy( trace.endpos, verts[0].xyz );
	verts[0].xyz[0] -= markSize;
	verts[0].xyz[1] -= markSize;
	VectorArray2Set( verts[0].st, 0, 0 );
	Vector4Set( verts[0].modulate, 255, 255, 255, 255 );

	VectorCopy( trace.endpos, verts[1].xyz );
	verts[1].xyz[0] -= markSize;
	verts[1].xyz[1] += markSize;
	VectorArray2Set( verts[1].st, 0, 1 );
	Vector4Set( verts[1].modulate, 255, 255, 255, 255 );

	VectorCopy( trace.endpos, verts[2].xyz );
	verts[2].xyz[0] += markSize;
	verts[2].xyz[1] += markSize;
	VectorArray2Set( verts[2].st, 1, 1 );
	Vector4Set( verts[2].modulate, 255, 255, 255, 255 );

	VectorCopy( trace.endpos, verts[3].xyz );
	verts[3].xyz[0] += markSize;
	verts[3].xyz[1] -= markSize;
	VectorArray2Set( verts[3].st, 1, 0 );
	Vector4Set( verts[3].modulate, 255, 255, 255, 255 );

	trap_R_AddPolyToScene( cgs.media.wakeMarkShader, 4, verts );
}



/*
===============
CG_ModelSize

Change/scale model size
===============
*/
static void CG_ModelSize( refEntity_t *model, float size ) { // BFP - Model size
	model->axis[0][0] *= size;
	model->axis[0][1] *= size;
	model->axis[0][2] *= size;

	model->axis[1][0] *= size;
	model->axis[1][1] *= size;
	model->axis[1][2] *= size;

	model->axis[2][0] *= size;
	model->axis[2][1] *= size;
	model->axis[2][2] *= size;
}

/*
==========================
CG_AuraPowerlevelSetShaderColor

Powerlevel is divided into tiers. 
While most powerlevel effects are based on the powerlevel itself, 
aura color is determined by the tier.

- Tier 1:			< 100,000 PL			Blue aura
- Tier 2:			100,000 – 250,000 PL	Red aura
- Tier 3:			250,000 – 500,000 PL	Red aura
- Tier 4:			500,000 – 999,000 PL	Red aura
- Ultimate Tier:	1 mil PL				Yellow aura
==========================
*/
static qhandle_t CG_AuraPowerlevelSetShaderColor( entityState_t *state ) {
	qhandle_t	auraShader = cgs.media.auraRedTinyShader;
	int			powerlevel = state->frame;

	if ( state->clientNum == cg.snap->ps.clientNum ) { // fixes a weird bug when trying to see the powerlevel of itself
		powerlevel = cg.snap->ps.persistant[PERS_POWERLEVEL];
	}

	// red
	if ( cg_lightweightAuras.integer <= 0
	&& cg_polygonAura.integer <= 0
	&& cg_spriteAura.integer <= 0
	&& cg_particleAura.integer <= 0 ) {
		auraShader = cgs.media.auraRedChargeShader;
		if ( ( state->legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_CHARGE
		&& !( state->eFlags & EF_AURA_TIER_UP ) ) {
			auraShader = cgs.media.auraRedUseShader;
		}
	}
	// blue
	if ( powerlevel < 100 
	|| ( cgs.gametype >= GT_TEAM && cgs.clientinfo[ state->clientNum ].team == TEAM_BLUE ) ) {
		auraShader = cgs.media.auraBlueTinyShader;

		if ( cg_lightweightAuras.integer <= 0
		&& cg_polygonAura.integer <= 0
		&& cg_spriteAura.integer <= 0
		&& cg_particleAura.integer <= 0 ) {
			auraShader = cgs.media.auraBlueChargeShader;
			if ( ( state->legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_CHARGE
			&& !( state->eFlags & EF_AURA_TIER_UP ) ) {
				auraShader = cgs.media.auraBlueUseShader;
			}
		}
	}
	// yellow
	if ( powerlevel >= 1000
	&& !( cgs.gametype >= GT_TEAM ) ) {
		auraShader = cgs.media.auraYellowTinyShader;

		if ( cg_lightweightAuras.integer <= 0
		&& cg_polygonAura.integer <= 0
		&& cg_spriteAura.integer <= 0
		&& cg_particleAura.integer <= 0 ) {
			auraShader = cgs.media.auraYellowChargeShader;
			if ( ( state->legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_CHARGE
			&& !( state->eFlags & EF_AURA_TIER_UP ) ) {
				auraShader = cgs.media.auraYellowUseShader;
			}
		}
	}
	return auraShader;
}

/*
===============
CG_AuraAnims

Handle aura animations, when idling it sets the aura vertical rotation, so the aura rotates vertically
===============
*/
static void CG_AuraAnims( centity_t *cent, refEntity_t *aura, qboolean reversed, vec3_t auraInverseRotation ) { // BFP - Aura animations (change model shaders)
	if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_RUN
	|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_JUMP ) {
		aura->hModel = cgs.media.runauraModel;
	} else if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_BACK
	|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_JUMPB
	|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_FLYB ) {
		aura->hModel = cgs.media.backauraModel;
	} else if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_FLYA ) {
		aura->hModel = cgs.media.flyauraModel;
	} else {
		aura->hModel = cgs.media.auraModel;
		if ( reversed ) {
			VectorNegate( cg.autoAngles, auraInverseRotation );
			AnglesToAxis( auraInverseRotation, aura->axis );
		} else {
			AnglesToAxis( cg.autoAngles, aura->axis );
		}
	}
}


/*
===============
CG_DynamicAuraLight

Dynamic aura light, note: when charging it changes the shinning a bit
Aura lights like cg_smallOwnAura only can be shown to itself and not the other clients, 
the other clients only show small lights. 
===============
*/
static void CG_DynamicAuraLight( centity_t *cent, int clientNum, float r, float g, float b ) { // BFP - Dynamic aura light
	int dLightSize = 200;
	int rndDLight = dLightSize * 0.7845;
	int firstRndDlight = dLightSize * 1.26;

	// BFP - Monster gamemode, player monster dynamic light size
	if ( cent->currentState.eFlags & EF_MONSTER ) {
		dLightSize = 1000;
	}
	// BFP - NOTE: Originally, if cg_spriteAura or cg_particleAura is on, the lights aren't displayed. 
	// But in that case, that can displayed, so it makes no sense not being displayed and 
	// maybe these things were broken on original BFP.
	if ( cg_lightAuras.integer > 0 ) {
		if ( clientNum == cg.snap->ps.clientNum && cg_smallOwnAura.integer > 0 ) {
			trap_R_AddLightToScene( cent->lerpOrigin, dLightSize, r, g, b );
			trap_R_AddLightToScene( cent->lerpOrigin, dLightSize, r, g, b );
			if ( !( cg.predictedPlayerState.pm_flags & PMF_KI_CHARGE ) ) {
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&rndDLight), r, g, b );
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&rndDLight), r, g, b );
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&rndDLight), r, g, b );
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&rndDLight), r, g, b );
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&rndDLight), r, g, b );
			} else {
				dLightSize = 100;
				// BFP - Monster gamemode, player monster dynamic light size
				if ( cent->currentState.eFlags & EF_MONSTER ) {
					dLightSize = 500;
				}
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&dLightSize), r, g, b );
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&dLightSize), r, g, b );
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&dLightSize), r, g, b );
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&dLightSize), r, g, b );
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&dLightSize), r, g, b );
			}
		} else if ( clientNum != cg.snap->ps.clientNum || cg_lightweightAuras.integer > 0 || cg_polygonAura.integer > 0 || cg_highPolyAura.integer > 0 ) {
			dLightSize = 50;
			// BFP - Monster gamemode, player monster dynamic light size
			if ( cent->currentState.eFlags & EF_MONSTER ) {
				dLightSize = 250;
			}
			trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&(dLightSize * 2)), r, g, b );
			trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&(dLightSize * 2)), r, g, b );
			trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&(dLightSize * 2)), r, g, b );
		} else {
			trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&firstRndDlight), r, g, b );
			trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&rndDLight), r, g, b );
			trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&rndDLight), r, g, b );
			if ( !( cg.predictedPlayerState.pm_flags & PMF_KI_CHARGE ) ) {
				dLightSize = 100;
				// BFP - Monster gamemode, player monster dynamic light size
				if ( cent->currentState.eFlags & EF_MONSTER ) {
					dLightSize = 500;
				}
				firstRndDlight = dLightSize * 1.5;
				trap_R_AddLightToScene( cent->lerpOrigin, dLightSize + (rand()&firstRndDlight), r, g, b );
			}
		}
	}
}


/*
===============
CG_RemoveKiTrails

Handle aura animations, when idling it sets the aura vertical rotation, so the aura rotates vertically
===============
*/
static void CG_RemoveKiTrails( centity_t *cent, int clientNum, vec3_t kiTrailOrigin, qhandle_t kiTrailShader, qboolean fastRemove ) { // BFP - Remove ki trails
	if ( cg.time > cent->pe.kiTrailTime ) { // reset ki trail position avoid being zeroed
		CG_ResetKiTrail( clientNum, kiTrailOrigin );
	} else { // ki trails keep running in that moment, but their segments are being removed
		CG_KiTrail( clientNum, kiTrailOrigin, fastRemove, kiTrailShader );
	}
}

/*
===============
CG_AddRefEntityWithPowerups

Adds a piece with modifications or duplications for powerups
Also called by CG_Missile for quad rockets, but nobody can tell...
===============
*/
void CG_AddRefEntityWithPowerups( refEntity_t ent, entityState_t *state, int team ) {
	// BFP - Powerlevel
	int powerlevel = state->frame;

	if ( state->clientNum == cg.snap->ps.clientNum ) { // fixes a weird bug when trying to see the powerlevel of itself
		powerlevel = cg.snap->ps.persistant[PERS_POWERLEVEL];
	}

	if ( state->powerups & ( 1 << PW_INVIS ) ) {
		ent.customShader = cgs.media.invisShader;
		trap_R_AddRefEntityToScene( &ent );
		return;
	}

	// render main model
	trap_R_AddRefEntityToScene( &ent );

	// BFP - Render ultimate perma-glow when already transformed
	if ( cg_permaglowUltimate.integer > 0 
	&& powerlevel >= 1000 ) {
		ent.customShader = cgs.media.ultimateAuraShader;
		if ( ent.customShader ) {
			trap_R_AddRefEntityToScene( &ent );
		}
	}
	ent.customShader = CG_AuraPowerlevelSetShaderColor( state );

	if ( ( state->eFlags & EF_AURA ) || ( state->eFlags & EF_AURA_TIER_UP ) ) {
		// BFP - Transformation aura
		if ( ( state->eFlags & EF_AURA_TIER_UP ) && cg_transformationAura.integer <= 0 ) {
			return;
		}
		// BFP - If the player is using lightweight auras or their own small aura
		if ( ( cg_lightweightAuras.integer > 0
		|| ( state->clientNum == cg.snap->ps.clientNum 
			&& cg_smallOwnAura.integer > 0 ) )
		&& cg_spriteAura.integer <= 0
		&& cg_particleAura.integer <= 0 ) {
			trap_R_AddRefEntityToScene( &ent );
		}

		// BFP - Resize shader aura when tier is up
		if ( state->eFlags & EF_AURA_TIER_UP ) {
			CG_ModelSize( &ent, 1.4f );
		}

		// BFP - Shader aura
		if ( cg_lightweightAuras.integer <= 0
		&& cg_polygonAura.integer <= 0
		&& cg_spriteAura.integer <= 0
		&& cg_particleAura.integer <= 0 ) {
			if ( state->clientNum == cg.snap->ps.clientNum 
			&& cg_smallOwnAura.integer > 0 ) {
				trap_R_AddRefEntityToScene( &ent );
				return;
			}
			ent.customShader = CG_AuraPowerlevelSetShaderColor( state );
			trap_R_AddRefEntityToScene( &ent );
		}
	}
}

/*
============
CG_SpriteAura

Adds sprite aura, just one quad
============
*/
static void CG_SpriteAura( refEntity_t aura ) { // BFP - Sprite aura
	// BFP - NOTE: What shader was added?? Originally, BFP didn't finish the shader to attach or they forgot...
	// That radius looks a bit big for an aura, maybe they thought to fit the texture that way or some circular aura?
	// And... What the heck? This sprite view depends of pitch angle until some client connects?
	// Also when cg_smallOwnAura cvar is enabled, it doesn't display any aura to the client itself. 
	// Moreover, the lights are disabled as mentioned previously in CG_DynamicAuraLight function comments
	// In the future, the shader should be added, not sure what kind of aura is this...
	float pitchView = cg.refdefViewAngles[PITCH];
	short i = 0, connectedClients = 1;

	while ( i < MAX_CLIENTS ) {
		if ( cg_entities[i].currentValid ) {
			++connectedClients;
		}
		++i;
	}
	aura.reType = RT_SPRITE;
	aura.customShader = 0;
	aura.radius = 75;
	if ( connectedClients > 1 ) {
		pitchView = -15;
	}
	aura.rotation = pitchView;

	aura.shaderRGBA[0] = 255;
	aura.shaderRGBA[1] = 255;
	aura.shaderRGBA[2] = 255;
	aura.shaderRGBA[3] = 255;
	trap_R_AddRefEntityToScene( &aura );
}


/*
============
CG_Aura

Adds aura and ki trails
============
*/
static void CG_Aura( centity_t *cent, int clientNum, clientInfo_t *ci, int renderfx, refEntity_t legs, qhandle_t kiTrailShader ) { // BFP - Aura and ki trails
	refEntity_t		aura;
	refEntity_t		aura2; // secondary aura
	vec3_t			auraInverseRotation; // for aura inverse rotation
	vec3_t			kiTrailOrigin;
	int				powerlevel = cent->currentState.frame;
	const int		KI_TRAIL_ZPOS = 5;

	if ( clientNum == cg.snap->ps.clientNum ) { // fixes a weird bug when trying to see the powerlevel of itself
		powerlevel = cg.snap->ps.persistant[PERS_POWERLEVEL];
	}

	memset( &aura, 0, sizeof(aura) );
	memset( &aura2, 0, sizeof(aura2) );

	// origin setup for ki trails
	VectorCopy( cent->lerpOrigin, kiTrailOrigin );
	kiTrailOrigin[2] += KI_TRAIL_ZPOS;

	if ( ( cent->currentState.eFlags & EF_AURA ) || ( cent->currentState.eFlags & EF_AURA_TIER_UP ) ) {
		// trace for bubble particles only when moving in the water and charging
		int destContentType = CG_PointContents( legs.origin, -1 );

		// BFP - Ki trail
		if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_CHARGE 
		&& cg_kiTrail.integer > 0 ) {
			// apply time for using ki trail
			cent->pe.kiTrailTime = cg.time + cg_kiTrail.integer*7;

			CG_KiTrail( clientNum, kiTrailOrigin, qfalse, kiTrailShader );
		} else { // handle when the ki trail was being used previously
			CG_RemoveKiTrails( cent, clientNum, kiTrailOrigin, kiTrailShader, qtrue );
		}

		// spawning bubble particles
		if ( destContentType & CONTENTS_WATER ) {
			trace_t trace;
			vec3_t start, bubbleOrigin;
			float bubbleSize = 2;
			float bubbleRange = 10;

			VectorCopy( legs.origin, bubbleOrigin );
			trap_CM_BoxTrace( &trace, start, bubbleOrigin, NULL, NULL, 0, CONTENTS_WATER );

			bubbleOrigin[2] += -17; // put the origin below the character's feet

			// BFP - Monster gamemode, player monster bubble particle size, range and position
			if ( cent->currentState.eFlags & EF_MONSTER ) {
				bubbleSize = 8;
				bubbleRange = 100;
				bubbleOrigin[2] += -85; // put the origin below the character's feet
				trace.endpos[2] += 100;
			}

			if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_FLYA
			|| ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_FLYB ) {
				bubbleOrigin[2] += 6; // put the origin near the player origin point
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, bubbleOrigin, trace.endpos, 700, bubbleRange, bubbleSize );
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, bubbleOrigin, trace.endpos, 700, bubbleRange, bubbleSize );
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, bubbleOrigin, trace.endpos, 700, bubbleRange, bubbleSize );
			} else if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_CHARGE ) {
				bubbleOrigin[2] += -3; // put the origin a little below
				bubbleRange *= 2;

				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, bubbleOrigin, trace.endpos, 0, bubbleRange, bubbleSize );
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, bubbleOrigin, trace.endpos, 0, bubbleRange, bubbleSize );
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, bubbleOrigin, trace.endpos, 0, bubbleRange, bubbleSize );
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, bubbleOrigin, trace.endpos, 0, bubbleRange, bubbleSize );
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, bubbleOrigin, trace.endpos, 0, bubbleRange, bubbleSize );
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, bubbleOrigin, trace.endpos, 0, bubbleRange, bubbleSize );
				CG_ParticleBubble( cent, cgs.media.waterBubbleShader, bubbleOrigin, trace.endpos, 0, bubbleRange, bubbleSize );
			}
		}

		// apply the render type
		aura.reType = aura2.reType = RT_MODEL;

		// clear the axis to keep the position
		AxisClear( aura.axis );
		AxisClear( aura2.axis );

		// if the player is moving like going forward and backwards, then use other aura model
		CG_AuraAnims( cent, &aura, 0, auraInverseRotation );
		CG_AuraAnims( cent, &aura2, 1, auraInverseRotation );

		// resize the aura
		CG_ModelSize( &aura, 1.1565f );
		CG_ModelSize( &aura2, 1.252f );

		// set aura position to the player
		VectorCopy( legs.origin, aura.origin );
		VectorCopy( legs.lightingOrigin, aura.lightingOrigin );
		VectorCopy( legs.origin, aura2.origin );
		VectorCopy( legs.lightingOrigin, aura2.lightingOrigin );

		// apply light blinking
		aura.customShader = aura2.customShader = CG_AuraPowerlevelSetShaderColor( &cent->currentState );
		// blue
		if ( powerlevel < 100 
		|| ( cgs.gametype >= GT_TEAM && ci->team == TEAM_BLUE ) ) {
			CG_DynamicAuraLight( cent, clientNum, 0.2f, 0.2f, 1.0 );
		}
		// yellow
		else if ( powerlevel >= 1000
		&& !( cgs.gametype >= GT_TEAM ) ) {
			//CG_DynamicAuraLight( cent, clientNum, 1.0, 1.0, 0 );
			CG_DynamicAuraLight( cent, clientNum, 1.0, 1.0, 0.2f );
		}
		// red
		else {
			CG_DynamicAuraLight( cent, clientNum, 1.0, 0.2f, 0.2f );
		}

		aura.renderfx = aura2.renderfx = renderfx;
		VectorCopy( aura.origin, aura.oldorigin );	// don't positionally lerp at all
		VectorCopy( aura2.origin, aura2.oldorigin );	// don't positionally lerp at all

		// ki boost and ki charge sounds
		if ( !( cent->currentState.eFlags & EF_AURA_TIER_UP ) ) {
			if ( !( cent->currentState.eFlags & EF_KI_BOOST )
			&& ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_CHARGE ) {
				trap_S_AddLoopingSound( cent->currentState.clientNum, cent->lerpOrigin, 
					vec3_origin, cgs.media.kiChargeSound );
			} else {
				trap_S_AddLoopingSound( cent->currentState.clientNum, cent->lerpOrigin, 
					vec3_origin, cgs.media.kiUseSound );
			}
		}

		// aura tier
		if ( cent->currentState.eFlags & EF_AURA_TIER_UP ) {
			// resize the aura
			CG_ModelSize( &aura, 1.1565f );
			CG_ModelSize( &aura2, 1.252f );
		}

		// keep the aura pivot tagged in tag_torso
		CG_PositionRotatedEntityOnTag( &aura, &legs, ci->legsModel, "tag_torso" );
		CG_PositionRotatedEntityOnTag( &aura2, &legs, ci->legsModel, "tag_torso" );

		// BFP - Sprite aura
		if ( ( cg_spriteAura.integer > 0 && cg_smallOwnAura.integer <= 0 ) 
		|| ( cg_spriteAura.integer > 0 && cg_smallOwnAura.integer > 0 && clientNum != cg.snap->ps.clientNum ) ) {
			CG_SpriteAura( aura );
			return;
		}

		// BFP - Particle aura
		if ( ( cg_particleAura.integer > 0 && cg_smallOwnAura.integer <= 0 ) 
		|| ( cg_particleAura.integer > 0 && cg_smallOwnAura.integer > 0 && clientNum != cg.snap->ps.clientNum ) ) {
			// BFP - NOTE: Particle aura wasn't fully implemented on original BFP.
			// Originally, particle aura wasn't correctly placed on player's origin, it was zeroed and 
			// when the player moves up, the aura was moving to right, and when moves down, it was moving to left; 
			// moreover, spawns too many particles; also the shader uses bubble ones and the particle size is a bit big. 
			// It's unknown what they planned in their future.
			// But this time, it's placed to player's origin like when being underwater, more fading is added, 
			// also it doesn't spawn too many particles.
			// In the future, that should be tweaked, bubble shader doesn't seem to fit well.
			vec3_t pAuraOrigin;
			VectorCopy( legs.origin, pAuraOrigin );

			pAuraOrigin[2] += -18; // put the origin a little below

			CG_ParticleAura( cent, clientNum, cgs.media.waterBubbleShader, pAuraOrigin, NULL, 20 );
			CG_ParticleAura( cent, clientNum, cgs.media.waterBubbleShader, pAuraOrigin, NULL, 20 );
			return;
		}

		// BFP - Small own aura only can be shown to the one who enables it for themself, not everyone
		if ( clientNum != cg.snap->ps.clientNum || cg_smallOwnAura.integer <= 0 ) {
			// BFP - Transformation aura
			if ( ( cent->currentState.eFlags & EF_AURA_TIER_UP ) && cg_transformationAura.integer <= 0 ) {
				return;
			}

			// add aura
			if ( cg_spriteAura.integer <= 0 && cg_particleAura.integer <= 0 
			&& cg_polygonAura.integer > 0 && cg_lightweightAuras.integer <= 0 ) {
				trap_R_AddRefEntityToScene( &aura );
			}

			// add secondary aura to make look cooler, a bit bigger than the other
			if ( cg_spriteAura.integer <= 0 && cg_particleAura.integer <= 0 
			&& cg_polygonAura.integer > 0 && cg_highPolyAura.integer > 0 && cg_lightweightAuras.integer <= 0 ) {
				trap_R_AddRefEntityToScene( &aura2 );
			}
		}
	} else {
		// BFP - Ki trail being removed
		if ( cg_kiTrail.integer > 0 ) {
			CG_RemoveKiTrails( cent, clientNum, kiTrailOrigin, kiTrailShader, qtrue );
		}
	}
}


/*
=================
CG_LightVerts
=================
*/
int CG_LightVerts( vec3_t normal, int numVerts, polyVert_t *verts )
{
	int				i, j;
	float			incoming;
	vec3_t			ambientLight;
	vec3_t			lightDir;
	vec3_t			directedLight;

	trap_R_LightForPoint( verts[0].xyz, ambientLight, directedLight, lightDir );

	for (i = 0; i < numVerts; i++) {
		incoming = DotProduct (normal, lightDir);
		if ( incoming <= 0 ) {
			Vector4Set( verts[i].modulate, ambientLight[0], ambientLight[1], ambientLight[2], 255 );
			continue;
		} 
		j = ( ambientLight[0] + incoming * directedLight[0] );
		if ( j > 255 ) {
			j = 255;
		}
		verts[i].modulate[0] = j;

		j = ( ambientLight[1] + incoming * directedLight[1] );
		if ( j > 255 ) {
			j = 255;
		}
		verts[i].modulate[1] = j;

		j = ( ambientLight[2] + incoming * directedLight[2] );
		if ( j > 255 ) {
			j = 255;
		}
		verts[i].modulate[2] = j;

		verts[i].modulate[3] = 255;
	}
	return qtrue;
}

/*
===============
CG_Player
===============
*/
void CG_Player( centity_t *cent ) {
	clientInfo_t	*ci;
	refEntity_t		legs;
	refEntity_t		torso;
	refEntity_t		head;
	int				clientNum;
	int				renderfx;
	qboolean		shadow;
	float			shadowPlane;
	qhandle_t		kiTrailShader;
	// BFP - Powerlevel for the aura
	int				powerlevel = -1;
	// BFP - Save head for first person vis mode
	refEntity_t savedHead;

	// the client number is stored in clientNum.  It can't be derived
	// from the entity number, because a single client may have
	// multiple corpses on the level using the same clientinfo
	clientNum = cent->currentState.clientNum;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		CG_Error( "Bad clientNum on player entity");
	}
	ci = &cgs.clientinfo[ clientNum ];
	
	// BFP - Powerlevel for the aura
	powerlevel = cent->currentState.frame;
	if ( clientNum == cg.snap->ps.clientNum ) { // fixes a weird bug when trying to see the powerlevel of itself
		powerlevel = cg.snap->ps.persistant[PERS_POWERLEVEL];
	}

	// BFP - Ki trail shader set
	// red
	kiTrailShader = cgs.media.kiTrailRedShader;
	// blue
	if ( powerlevel < 100 
	|| ( cgs.gametype >= GT_TEAM && ci->team == TEAM_BLUE ) ) {
		kiTrailShader = cgs.media.kiTrailBlueShader;
	}
	// yellow
	if ( powerlevel >= 1000
	&& !( cgs.gametype >= GT_TEAM ) ) {
		kiTrailShader = cgs.media.kiTrailYellowShader;
	}

	// it is possible to see corpses from disconnected players that may
	// not have valid clientinfo
	if ( !ci->infoValid ) {
		return;
	}

	// get the player model information
	renderfx = 0;
	if ( clientNum == cg.snap->ps.clientNum ) {
		if (!cg.renderingThirdPerson) {
			renderfx = RF_THIRD_PERSON;			// only draw in mirrors
		} /*else { // BFP - cg_cameraMode cvar doesn't exist
			if (cg_cameraMode.integer) {
				return;
			}
		}*/
	}


	memset( &legs, 0, sizeof(legs) );
	memset( &torso, 0, sizeof(torso) );
	memset( &head, 0, sizeof(head) );

	// get the rotation information
	CG_PlayerAngles( cent, legs.axis, torso.axis, head.axis );
	
	// get the animation state (after rotation, to allow feet shuffle)
	CG_PlayerAnimation( cent, &legs.oldframe, &legs.frame, &legs.backlerp,
		 &torso.oldframe, &torso.frame, &torso.backlerp );

	// add the talk baloon or disconnect icon
	CG_PlayerSprites( cent );

	// add the shadow
	shadow = CG_PlayerShadow( cent, &shadowPlane );

	// BFP - Handle the antigrav rock particles when the player is charging
	CG_AntigravRockHandling( cent );

	// BFP - Handle particle aura
	CG_ParticleAuraHandling( cent );

	// add a water splash if partially in and out of water
	CG_PlayerSplash( cent );

	if ( cg_shadows.integer == 3 && shadow ) {
		renderfx |= RF_SHADOW_PLANE;
	}
	renderfx |= RF_LIGHTING_ORIGIN;			// use the same origin for all

	//
	// add the legs
	//
	legs.hModel = ci->legsModel;
	legs.customSkin = ci->legsSkin;

	// BFP - Ultimate tier legs model and skin
	if ( powerlevel >= 1000 && cg.time > cent->pe.ultTierTransformTime - 800 ) {
		if ( ci->ultTierLegsModel ) {
			legs.hModel = ci->ultTierLegsModel;
		}
		if ( ci->ultTierLegsSkin ) {
			legs.customSkin = ci->ultTierLegsSkin;
		}
	}

	// BFP - Super Deformed (Chibi style) easter egg for the base model (the legs apply all parts of the body)
	if ( cg_superdeformed.integer > 0 ) {
		CG_ModelSize( &legs, 0.8f );
	}

	VectorCopy( cent->lerpOrigin, legs.origin );

	VectorCopy( cent->lerpOrigin, legs.lightingOrigin );

	// BFP - Monster gamemode, the player monster is bigger than a normal player
	if ( cent->currentState.eFlags & EF_MONSTER ) {
		// monster model position
		if ( cgs.monster > 0 ) {
			CG_ModelSize( &legs, 4.75 );
		} else { // tweak a bit because of the hit box upside... huh... :/
			CG_ModelSize( &legs, 7.9 );
		}
		// adjust the model a bit up to see the legs correctly
		legs.origin[2] -= 22;
	}

	legs.shadowPlane = shadowPlane;
	legs.renderfx = renderfx;
	VectorCopy (legs.origin, legs.oldorigin);	// don't positionally lerp at all

	CG_AddRefEntityWithPowerups( legs, &cent->currentState, ci->team );

	// if the model failed, allow the default nullmodel to be displayed
	if (!legs.hModel) {
		return;
	}

	//
	// add the torso
	//
	torso.hModel = ci->torsoModel;
	if (!torso.hModel) {
		return;
	}

	torso.customSkin = ci->torsoSkin;

	// BFP - Ultimate tier torso model and skin
	if ( powerlevel >= 1000 && cg.time > cent->pe.ultTierTransformTime - 800 ) {
		if ( ci->ultTierTorsoModel ) {
			torso.hModel = ci->ultTierTorsoModel;
		}
		if ( ci->ultTierTorsoSkin ) {
			torso.customSkin = ci->ultTierTorsoSkin;
		}
	}

	VectorCopy( cent->lerpOrigin, torso.lightingOrigin );

	CG_PositionRotatedEntityOnTag( &torso, &legs, ci->legsModel, "tag_torso");

	torso.shadowPlane = shadowPlane;
	torso.renderfx = renderfx;

	CG_AddRefEntityWithPowerups( torso, &cent->currentState, ci->team );

	//
	// add the head
	//
	head.hModel = ci->headModel;
	if ( !head.hModel ) {
		return;
	}

	head.customSkin = ci->headSkin;

	// BFP - Ultimate tier head model and skin
	if ( powerlevel >= 1000 && cg.time > cent->pe.ultTierTransformTime - 800 ) {
		if ( ci->ultTierHeadModel ) {
			head.hModel = ci->ultTierHeadModel;
		}
		if ( ci->ultTierHeadSkin ) {
			head.customSkin = ci->ultTierHeadSkin;
		}
	}

	// BFP - Make a model changing effect when aura tier is up
	if ( ( cent->currentState.eFlags & EF_AURA_TIER_UP )
	&& cg.time > cent->pe.ultTierTransformTime ) {
		cent->pe.ultTierTransformTime = cg.time + 1100;
	}
	if ( !( cent->currentState.eFlags & EF_AURA_TIER_UP ) ) {
		cent->pe.ultTierTransformTime = 0;
	}
	
	if ( cg_yrgolroxor.integer > 0 ) { // BFP - Yrgol Roxor easter egg
		head.hModel = 0; // 0: no head model display, display pivot only
	}

	// BFP - Super Deformed (Chibi style) easter egg for the head model
	if ( cg_superdeformed.integer > 0 ) {
		CG_ModelSize( &head, 3.0f );
	}

	VectorCopy( cent->lerpOrigin, head.lightingOrigin );

	CG_PositionRotatedEntityOnTag( &head, &torso, ci->torsoModel, "tag_head");

	head.shadowPlane = shadowPlane;
	head.renderfx = renderfx;

	// BFP - First person vis mode doesn't have head model to be displayed
	savedHead = head;
	if ( cg_drawOwnModel.integer >= 1 && cg_thirdPerson.integer <= 0
	&& clientNum == cg.snap->ps.clientNum
	&& !( cent->currentState.eFlags & EF_DEAD ) ) {
		memset( &head, 0, sizeof(head) );
	}

	CG_AddRefEntityWithPowerups( head, &cent->currentState, ci->team );

	// BFP - If the entity is a corpse, avoid drawing ki trails to the dead
	if ( cent->currentState.eFlags & EF_DEAD ) {
		return;
	}

	//
	// BFP - Aura and ki trails
	//
	CG_Aura( cent, clientNum, ci, renderfx, legs, kiTrailShader );

	// BFP - Ki attack sounds
	CG_KiAttackSounds( cent );

	// BFP - Render ultimate perma-glow dynamic lights when already transformed
	if ( cg_lightAuras.integer > 0 && cg_permaglowUltimate.integer > 0 && powerlevel >= 1000 ) {
		trap_R_AddLightToScene( cent->lerpOrigin, 50 + (rand()&80), 1.0, 1.0, 0.2f );
		trap_R_AddLightToScene( cent->lerpOrigin, 50 + (rand()&80), 1.0, 1.0, 0.2f );
	}

	// BFP - First person camera setup
	if ( cg_thirdPerson.integer <= 0 
	&& clientNum == cg.snap->ps.clientNum ) { // BFP - Avoid every time some player/bot enters in the server and changes the view into the other player
		static vec3_t	deadOriginDrawOwnModel;

		if ( !( cent->currentState.eFlags & EF_DEAD ) ) {
			// BFP - Set dead origin where the player was alive when First person vis mode is being used
			VectorCopy( cg.refdef.vieworg, deadOriginDrawOwnModel );
			CG_PositionRotatedEntityOnTag( &savedHead, &savedHead, ci->headModel, "tag_eyes");
			CG_OffsetFirstPersonView( cent, &savedHead, ci->headModel );
		} else if ( cg.snap->ps.stats[STAT_HEALTH] <= 0
		&& ( cent->currentState.eFlags & EF_DEAD )
		&& cg_drawOwnModel.integer >= 1 ) { // BFP - Death camera only for First person vis
			VectorCopy( deadOriginDrawOwnModel, cg.refdef.vieworg );
			cg.refdefViewAngles[YAW] = cg.snap->ps.stats[STAT_DEAD_YAW];
		}
	}

	//
	// add the gun / barrel / flash
	//
	CG_AddPlayerWeapon( &torso, NULL, cent, ci->team );

	// add powerups floating behind the player
	CG_PlayerPowerups( cent, &torso );
}

/*
===============
CG_GetTagOrientationFromPlayerEntityParentModel
===============
*/
qboolean CG_GetTagOrientationFromPlayerEntityParentModel( centity_t *cent, refEntity_t *parent, 
					qhandle_t parentModel, char *tagName, orientation_t *tagOrient ) { // BFP - Parent model tag orientation, used for first person vis mode
	int				i;
	orientation_t	lerped;
	vec3_t			tempAxis[3];

	if ( cent->currentState.eType != ET_PLAYER || !tagName[0] ) {
		return qfalse;
	}
	
	// Prepare the destination orientation_t
	AxisClear( tagOrient->axis );

	// Try to find the tag and return its coordinates
	if ( trap_R_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame, 1.0 - parent->backlerp, tagName ) ) {
        VectorCopy( parent->origin, tagOrient->origin );
        for ( i = 0 ; i < 3 ; i++ ) {
            VectorMA( tagOrient->origin, lerped.origin[i], parent->axis[i], tagOrient->origin );
        }
        MatrixMultiply( tagOrient->axis, lerped.axis, tempAxis );
        MatrixMultiply( tempAxis, parent->axis, tagOrient->axis );
        return qtrue;
    }
	return qfalse;
}

//=====================================================================

/*
===============
CG_ResetPlayerEntity

A player just came into view or teleported, so reset all animation info
===============
*/
void CG_ResetPlayerEntity( centity_t *cent ) {
	cent->errorTime = -99999;		// guarantee no error decay added
	cent->extrapolated = qfalse;	

	CG_ClearLerpFrame( &cgs.clientinfo[ cent->currentState.clientNum ], &cent->pe.legs, cent->currentState.legsAnim );
	CG_ClearLerpFrame( &cgs.clientinfo[ cent->currentState.clientNum ], &cent->pe.torso, cent->currentState.torsoAnim );

	BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );

	VectorCopy( cent->lerpOrigin, cent->rawOrigin );
	VectorCopy( cent->lerpAngles, cent->rawAngles );

	memset( &cent->pe.legs, 0, sizeof( cent->pe.legs ) );
	cent->pe.legs.yawAngle = cent->rawAngles[YAW];
	cent->pe.legs.yawing = qfalse;
	cent->pe.legs.pitchAngle = 0;
	cent->pe.legs.pitching = qfalse;

	memset( &cent->pe.torso, 0, sizeof( cent->pe.legs ) );
	cent->pe.torso.yawAngle = cent->rawAngles[YAW];
	cent->pe.torso.yawing = qfalse;
	cent->pe.torso.pitchAngle = cent->rawAngles[PITCH];
	cent->pe.torso.pitching = qfalse;

	if ( cg_debugPosition.integer ) {
		CG_Printf("%i ResetPlayerEntity yaw=%i\n", cent->currentState.number, cent->pe.torso.yawAngle );
	}
}

