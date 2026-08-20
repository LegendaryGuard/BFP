/*
===========================================================================

BFP SKIN CONFIG

===========================================================================
*/


#include "cg_local.h"

static qboolean CG_LoadSkinConfigFile( const char *fileName, bfpSkinConfig_t *config );

/*
================
CG_SkinConfig_ReadToken

Copies the next whitespace-delimited token from *ptr into value,
advances *ptr past it
================
*/
static void CG_SkinConfig_ReadToken( char **ptr, char *value, int valueSize ) {
	int	i;

	while ( **ptr && ( **ptr == ' ' || **ptr == '\t' ) ) {
		( *ptr )++;
	}

	i = 0;
	while ( **ptr && **ptr != ' ' && **ptr != '\t' && **ptr != '\n' && **ptr != '\r' && i < valueSize - 1 ) {
		value[i++] = *( *ptr )++;
	}
	value[i] = 0;
}

/*
================
CG_SkinConfig_ReadQuotedOrToken

Like CG_SkinConfig_ReadToken, but if the next non-whitespace char is a
quote, reads until the matching closing quote instead of stopping at
whitespace (so "path/with spaces.wav" style values work, even though none
of BFP's actual paths use spaces)
================
*/
static void CG_SkinConfig_ReadQuotedOrToken( char **ptr, char *value, int valueSize ) {
	int	i;

	while ( **ptr && ( **ptr == ' ' || **ptr == '\t' ) ) {
		( *ptr )++;
	}

	if ( **ptr == '"' ) {
		( *ptr )++;
		i = 0;
		while ( **ptr && **ptr != '"' && **ptr != '\n' && **ptr != '\r' && i < valueSize - 1 ) {
			value[i++] = *( *ptr )++;
		}
		value[i] = 0;
		if ( **ptr == '"' ) {
			( *ptr )++;
		}
		return;
	}

	CG_SkinConfig_ReadToken( ptr, value, valueSize );
}

/*
================
CG_SkinConfig_ReadAttackIndex

Reads the leading [attack index] argument that every directive in this file takes
================
*/
static qboolean CG_SkinConfig_ReadAttackIndex( char **ptr, int *outIndex ) {
	char	value[16];

	CG_SkinConfig_ReadToken( ptr, value, sizeof( value ) );
	*outIndex = atoi( value );

	if ( *outIndex < 0 || *outIndex >= BFP_NUM_WEAPONS ) {
		return qfalse;
	}
	return qtrue;
}

/*
================
CG_SkinConfig_TrailFuncFromString

Maps a missileTrailFunc string token to its MISSILE_TRAIL_FUNC_* value

missileTrailFunc string -> MISSILE_TRAIL_FUNC_* value
================
*/
static int CG_SkinConfig_TrailFuncFromString( const char *value ) {
	if ( !Q_stricmpn( value, "beam", 4 ) ) {
		return MISSILE_TRAIL_FUNC_BEAM;
	}
	if ( !Q_stricmpn( value, "rocket", 6 ) ) {
		return MISSILE_TRAIL_FUNC_ROCKET;
	}
	if ( !Q_stricmpn( value, "spiralbeam", 10 ) ) {
		return MISSILE_TRAIL_FUNC_SPIRALBEAM;
	}
	return 0; // "none" or any unknown string
}

/*
================
CG_ParseSkinConfigBuffer

Parses one already-loaded skin-config-style buffer, applying every directive 
it finds directly onto *config (the caller's in-progress bfpSkinConfig_t) - this 
is what makes the cascade work: call this once per layer, in order, 
on the same config, and each layer's directives simply overwrite whatever the 
previous layer set for that same field.
Fields the file doesn't mention at all are left exactly as they were
================
*/
static void CG_ParseSkinConfigBuffer( char *buf, bfpSkinConfig_t *config ) {
	char	*ptr;
	char	value[MAX_QPATH];
	int		attackIdx;
	bfpAttackSkinConfig_t	*atk;

	ptr = buf;

	while ( *ptr ) {
		while ( *ptr && ( *ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n' ) ) {
			ptr++;
		}
		if ( !*ptr ) {
			break;
		}
		if ( !Q_stricmpn( ptr, "end", 3 ) ) {
			break;
		}

		// longer keywords are checked before any keyword they're a prefix of
		// (e.g. attackTagPart before attackTag), even though the "next char must be
		// whitespace" guard below already prevents a short keyword from matching a
		// longer line by accident - keeping the longer check first avoids relying on
		// that guard alone
		if ( !Q_stricmpn( ptr, "attackTagPart", 13 ) && ( ptr[13] == ' ' || ptr[13] == '\t' ) ) {
			ptr += 13;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				Q_strncpyz( config->attacks[attackIdx].attackTagPart, value, sizeof( config->attacks[attackIdx].attackTagPart ) );
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "attackTag", 9 ) && ( ptr[9] == ' ' || ptr[9] == '\t' ) ) {
			ptr += 9;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				Q_strncpyz( config->attacks[attackIdx].attackTag, value, sizeof( config->attacks[attackIdx].attackTag ) );
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "attackName", 10 ) && ( ptr[10] == ' ' || ptr[10] == '\t' ) ) {
			ptr += 10;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				Q_strncpyz( config->attacks[attackIdx].attackName, value, sizeof( config->attacks[attackIdx].attackName ) );
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "attackIcon", 10 ) && ( ptr[10] == ' ' || ptr[10] == '\t' ) ) {
			ptr += 10;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				atk = &config->attacks[attackIdx];
				atk->attackIcon = value[0] ? trap_R_RegisterShaderNoMip( value ) : 0;
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "constantFireAttack", 18 ) && ( ptr[18] == ' ' || ptr[18] == '\t' ) ) {
			ptr += 18;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].constantFireAttack = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "lightningBolt", 13 ) && ( ptr[13] == ' ' || ptr[13] == '\t' ) ) {
			ptr += 13;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].lightningBolt = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "noExplosionSound", 16 ) && ( ptr[16] == ' ' || ptr[16] == '\t' ) ) {
			ptr += 16;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].noExplosionSound = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "noExplosion", 11 ) && ( ptr[11] == ' ' || ptr[11] == '\t' ) ) {
			ptr += 11;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].noExplosion = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "attackFireVoice", 15 ) && ( ptr[15] == ' ' || ptr[15] == '\t' ) ) {
			ptr += 15;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				atk = &config->attacks[attackIdx];

				// turn off
				if ( Q_stricmp( value, "0" ) ) {
					Q_strncpyz( atk->attackFireVoicePath, value, sizeof( atk->attackFireVoicePath ) );
					atk->attackFireVoice = value[0] ? trap_S_RegisterSound( value, qfalse ) : 0;
				}
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "attackChargeVoice", 17 ) && ( ptr[17] == ' ' || ptr[17] == '\t' ) ) {
			ptr += 17;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				char	chargeValue[16];
				int		chargeIdx;

				CG_SkinConfig_ReadToken( &ptr, chargeValue, sizeof( chargeValue ) );
				chargeIdx = atoi( chargeValue );
				if ( chargeIdx < 0 ) {
					chargeIdx = 0;
				}
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );

				if ( chargeIdx >= 0 && chargeIdx <= ATTACK_CHARGE_LIMIT ) {
					atk = &config->attacks[attackIdx];

					// turn off
					if ( Q_stricmp( value, "0" ) ) {
						Q_strncpyz( atk->attackChargeVoicePath[chargeIdx], value, sizeof( atk->attackChargeVoicePath[chargeIdx] ) );
						atk->attackChargeVoice[chargeIdx] = value[0] ? trap_S_RegisterSound( value, qfalse ) : 0;
					}
				}
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) ); // charge index
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) ); // sound path
			}
		} else if ( !Q_stricmpn( ptr, "missileSound", 12 ) && ( ptr[12] == ' ' || ptr[12] == '\t' ) ) {
			ptr += 12;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				atk = &config->attacks[attackIdx];
				Q_strncpyz( atk->missileSoundPath, value, sizeof( atk->missileSoundPath ) );
				atk->missileSound = value[0] ? trap_S_RegisterSound( value, qfalse ) : 0;
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "chargeSound", 11 ) && ( ptr[11] == ' ' || ptr[11] == '\t' ) ) {
			ptr += 11;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				atk = &config->attacks[attackIdx];
				Q_strncpyz( atk->chargeSoundPath, value, sizeof( atk->chargeSoundPath ) );
				atk->chargeSound = value[0] ? trap_S_RegisterSound( value, qfalse ) : 0;
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "flashSound", 10 ) && ( ptr[10] == ' ' || ptr[10] == '\t' ) ) {
			ptr += 10;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				atk = &config->attacks[attackIdx];
				Q_strncpyz( atk->flashSoundPath, value, sizeof( atk->flashSoundPath ) );
				atk->flashSound = value[0] ? trap_S_RegisterSound( value, qfalse ) : 0;
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "firingSound", 11 ) && ( ptr[11] == ' ' || ptr[11] == '\t' ) ) {
			ptr += 11;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				atk = &config->attacks[attackIdx];
				Q_strncpyz( atk->firingSoundPath, value, sizeof( atk->firingSoundPath ) );
				atk->firingSound = value[0] ? trap_S_RegisterSound( value, qfalse ) : 0;
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "missileDlightColor", 18 ) && ( ptr[18] == ' ' || ptr[18] == '\t' ) ) {
			ptr += 18;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				char	r[16], g[16], b[16];
				CG_SkinConfig_ReadToken( &ptr, r, sizeof( r ) );
				CG_SkinConfig_ReadToken( &ptr, g, sizeof( g ) );
				CG_SkinConfig_ReadToken( &ptr, b, sizeof( b ) );
				atk = &config->attacks[attackIdx];
				atk->missileDlightColor[0] = (float)atof( r );
				atk->missileDlightColor[1] = (float)atof( g );
				atk->missileDlightColor[2] = (float)atof( b );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "missileDlight", 13 ) && ( ptr[13] == ' ' || ptr[13] == '\t' ) ) {
			ptr += 13;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].missileDlight = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "missileTrailFunc", 16 ) && ( ptr[16] == ' ' || ptr[16] == '\t' ) ) {
			ptr += 16;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].missileTrailFunc = CG_SkinConfig_TrailFuncFromString( value );
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "missileTrailTime", 16 ) && ( ptr[16] == ' ' || ptr[16] == '\t' ) ) {
			ptr += 16;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].missileTrailTime = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "missileTrailRadius", 18 ) && ( ptr[18] == ' ' || ptr[18] == '\t' ) ) {
			ptr += 18;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].missileTrailRadius = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "spiralBeamShader", 16 ) && ( ptr[16] == ' ' || ptr[16] == '\t' ) ) {
			ptr += 16;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				atk = &config->attacks[attackIdx];
				Q_strncpyz( atk->spiralBeamShaderName, value, sizeof( atk->spiralBeamShaderName ) );
				atk->spiralBeamShader = value[0] ? trap_R_RegisterShader( value ) : 0;
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "beamShader", 10 ) && ( ptr[10] == ' ' || ptr[10] == '\t' ) ) {
			ptr += 10;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				atk = &config->attacks[attackIdx];
				Q_strncpyz( atk->beamShaderName, value, sizeof( atk->beamShaderName ) );
				atk->beamShader = value[0] ? trap_R_RegisterShader( value ) : 0;
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "flashModel", 10 ) && ( ptr[10] == ' ' || ptr[10] == '\t' ) ) {
			ptr += 10;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				atk = &config->attacks[attackIdx];
				Q_strncpyz( atk->flashModelName, value, sizeof( atk->flashModelName ) );
				atk->flashModel = value[0] ? trap_R_RegisterModel( value ) : 0;
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "flashShader", 11 ) && ( ptr[11] == ' ' || ptr[11] == '\t' ) ) {
			ptr += 11;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				atk = &config->attacks[attackIdx];
				Q_strncpyz( atk->flashShaderName, value, sizeof( atk->flashShaderName ) );
				atk->flashShader = value[0] ? trap_R_RegisterShader( value ) : 0;
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "flashRadius", 11 ) && ( ptr[11] == ' ' || ptr[11] == '\t' ) ) {
			ptr += 11;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].flashRadius = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "flashScaleFactor", 16 ) && ( ptr[16] == ' ' || ptr[16] == '\t' ) ) {
			ptr += 16;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].flashScaleFactor = (float)atof( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "firingFlashRadius", 17 ) && ( ptr[17] == ' ' || ptr[17] == '\t' ) ) {
			ptr += 17;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].firingFlashRadius = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "firingFlashScaleFactor", 22 ) && ( ptr[22] == ' ' || ptr[22] == '\t' ) ) {
			ptr += 22;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].firingFlashScaleFactor = (float)atof( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "missileShader", 13 ) && ( ptr[13] == ' ' || ptr[13] == '\t' ) ) {
			ptr += 13;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				atk = &config->attacks[attackIdx];
				Q_strncpyz( atk->missileShaderName, value, sizeof( atk->missileShaderName ) );
				atk->missileShader = value[0] ? trap_R_RegisterShader( value ) : 0;
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "missileModelRotation", 20 ) && ( ptr[20] == ' ' || ptr[20] == '\t' ) ) {
			ptr += 20;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].missileModelRotation = (float)atof( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "missileModel", 12 ) && ( ptr[12] == ' ' || ptr[12] == '\t' ) ) {
			ptr += 12;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				atk = &config->attacks[attackIdx];
				Q_strncpyz( atk->missileModelName, value, sizeof( atk->missileModelName ) );
				atk->missileModel = value[0] ? trap_R_RegisterModel( value ) : 0;
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "missileRotation", 15 ) && ( ptr[15] == ' ' || ptr[15] == '\t' ) ) {
			ptr += 15;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].missileRotation = atoi( value );
				atk = &config->attacks[attackIdx];
				atk->missileRotation = atoi( value );
				// turn off
				if ( !Q_stricmp( value, "0" ) ) {
					atk->missileModelRotation = 0;
					atk->missileRotation = 0;
				}
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "missileSpinHoriz", 16 ) && ( ptr[16] == ' ' || ptr[16] == '\t' ) ) {
			ptr += 16;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].missileSpinHoriz = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "missileRadiusChargeMult", 23 ) && ( ptr[23] == ' ' || ptr[23] == '\t' ) ) {
			ptr += 23;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].missileRadiusChargeMult = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "missileRadius", 13 ) && ( ptr[13] == ' ' || ptr[13] == '\t' ) ) {
			ptr += 13;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].missileRadius = (float)atof( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "missileScaleFactorChargeMult", 28 ) && ( ptr[28] == ' ' || ptr[28] == '\t' ) ) {
			ptr += 28;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].missileScaleFactorChargeMult = (float)atof( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "missileScaleFactor", 18 ) && ( ptr[18] == ' ' || ptr[18] == '\t' ) ) {
			ptr += 18;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].missileScaleFactor = (float)atof( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "explosionModel", 14 ) && ( ptr[14] == ' ' || ptr[14] == '\t' ) ) {
			ptr += 14;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				atk = &config->attacks[attackIdx];
				Q_strncpyz( atk->explosionModelName, value, sizeof( atk->explosionModelName ) );
				atk->explosionModel = value[0] ? trap_R_RegisterModel( value ) : 0;
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "explosionShader", 15 ) && ( ptr[15] == ' ' || ptr[15] == '\t' ) ) {
			ptr += 15;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
				atk = &config->attacks[attackIdx];
				Q_strncpyz( atk->explosionShaderName, value, sizeof( atk->explosionShaderName ) );
				atk->explosionShader = value[0] ? trap_R_RegisterShader( value ) : 0;
			} else {
				CG_SkinConfig_ReadQuotedOrToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "explosionRingScaleFactorChargeMult", 34 ) && ( ptr[34] == ' ' || ptr[34] == '\t' ) ) {
			ptr += 34;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].explosionRingScaleFactorChargeMult = (float)atof( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "explosionRingScaleFactor", 24 ) && ( ptr[24] == ' ' || ptr[24] == '\t' ) ) {
			ptr += 24;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].explosionRingScaleFactor = (float)atof( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "explosionRing", 13 ) && ( ptr[13] == ' ' || ptr[13] == '\t' ) ) {
			ptr += 13;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].explosionRing = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "explosionShellScaleFactorChargeMult", 36 ) && ( ptr[36] == ' ' || ptr[36] == '\t' ) ) {
			ptr += 36;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].explosionShellScaleFactorChargeMult = (float)atof( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "explosionShellScaleFactor", 25 ) && ( ptr[25] == ' ' || ptr[25] == '\t' ) ) {
			ptr += 25;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].explosionShellScaleFactor = (float)atof( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "explosionShell", 14 ) && ( ptr[14] == ' ' || ptr[14] == '\t' ) ) {
			ptr += 14;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].explosionShell = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "explosionRocks", 14 ) && ( ptr[14] == ' ' || ptr[14] == '\t' ) ) {
			ptr += 14;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].explosionRocks = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "explosionSparks", 15 ) && ( ptr[15] == ' ' || ptr[15] == '\t' ) ) {
			ptr += 15;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].explosionSparks = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "explosionSmokeRadius", 20 ) && ( ptr[20] == ' ' || ptr[20] == '\t' ) ) {
			ptr += 20;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].explosionSmokeRadius = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "explosionSmokeLife", 18 ) && ( ptr[18] == ' ' || ptr[18] == '\t' ) ) {
			ptr += 18;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].explosionSmokeLife = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "explosionSmokeSpeed", 19 ) && ( ptr[19] == ' ' || ptr[19] == '\t' ) ) {
			ptr += 19;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].explosionSmokeSpeed = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "explosionSmoke", 14 ) && ( ptr[14] == ' ' || ptr[14] == '\t' ) ) {
			ptr += 14;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].explosionSmoke = atoi( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "explosionScaleFactorChargeMult", 30 ) && ( ptr[30] == ' ' || ptr[30] == '\t' ) ) {
			ptr += 30;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].explosionScaleFactorChargeMult = (float)atof( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else if ( !Q_stricmpn( ptr, "explosionScaleFactor", 20 ) && ( ptr[20] == ' ' || ptr[20] == '\t' ) ) {
			ptr += 20;
			if ( CG_SkinConfig_ReadAttackIndex( &ptr, &attackIdx ) ) {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
				config->attacks[attackIdx].explosionScaleFactor = (float)atof( value );
			} else {
				CG_SkinConfig_ReadToken( &ptr, value, sizeof( value ) );
			}
		} else {
			// unrecognized directive: skip the rest of the line
			while ( *ptr && *ptr != '\n' && *ptr != '\r' ) {
				ptr++;
			}
		}

		while ( *ptr && ( *ptr == '\n' || *ptr == '\r' ) ) {
			ptr++;
		}
	}
}

/*
================
CG_LoadSkinConfigFile

Loads skin config file (if it exists) and merges it onto
*config via CG_ParseSkinConfigBuffer
================
*/
static qboolean CG_LoadSkinConfigFile( const char *fileName, bfpSkinConfig_t *config ) {
	fileHandle_t	f;
	int				len;
	static char		buf[32000];

	len = trap_FS_FOpenFile( fileName, &f, FS_READ );
	if ( !f ) {
		return qfalse;
	}

	if ( len >= sizeof( buf ) ) {
		trap_FS_FCloseFile( f );
		Com_Printf( "BFP skin config file too long: %s\n", fileName );
		return qfalse;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	CG_ParseSkinConfigBuffer( buf, config );

	return qtrue;
}

/*
================
CG_SetMonsterSkinConfig

Sets player monster skin config, a mouth blast
================
*/
void CG_SetMonsterSkinConfig( void ) {
	bfpAttackSkinConfig_t	*a = &cgs.media.monsterAttack;

	if ( cgs.media.monsterAttackLoaded ) {
		return;
	}

	memset( a, 0, sizeof( *a ) );

	Q_strncpyz( a->attackName, "Mouth Blast", sizeof(a->attackName) );
	a->attackIcon = trap_R_RegisterShaderNoMip( "icons/ultimateblast" );
	Q_strncpyz( a->attackTag, "tag_mouth", sizeof(a->attackTag) );
	Q_strncpyz( a->attackTagPart, "head", sizeof(a->attackTagPart) );
	Q_strncpyz( a->flashSoundPath, "sound/bfp/beamflash1.wav", sizeof(a->flashSoundPath) );
	a->flashSound = trap_S_RegisterSound( a->flashSoundPath, qfalse );
	Q_strncpyz( a->firingSoundPath, "sound/weapons/rocket/rockfly.wav", sizeof(a->firingSoundPath) );
	a->firingSound = trap_S_RegisterSound( a->firingSoundPath, qfalse );
	Q_strncpyz( a->chargeSoundPath, "sound/bfp/attackcharge1.wav", sizeof(a->chargeSoundPath) );
	a->chargeSound = trap_S_RegisterSound( a->chargeSoundPath, qfalse );
	Q_strncpyz( a->missileShaderName, "UltimateBlastAttackShader", sizeof(a->missileShaderName) );
	a->missileShader = trap_R_RegisterShader( a->missileShaderName );
	Q_strncpyz( a->missileModelName, "models/weaphits/ffbeamhead.md3", sizeof(a->missileModelName) );
	a->missileModel = trap_R_RegisterModel( a->missileModelName );
	Q_strncpyz( a->beamShaderName, "UltimateBlastBeamShader", sizeof(a->beamShaderName) );
	a->beamShader = trap_R_RegisterShader( a->beamShaderName );
	a->missileScaleFactor = 4.00f;
	a->missileScaleFactorChargeMult = 1.0f;
	a->missileTrailFunc = MISSILE_TRAIL_FUNC_BEAM;
	a->missileDlight = 800;
	a->missileDlightColor[0] = 1.0f; a->missileDlightColor[1] = 0.75f; a->missileDlightColor[2] = 0.0f;
	a->missileModelRotation = 0.2f;
	a->explosionRing = 1;
	a->explosionShell = 1;
	Q_strncpyz( a->flashShaderName, "UltimateBlastFlashShader", sizeof(a->flashShaderName) );
	a->flashShader = trap_R_RegisterShader( a->flashShaderName );
	a->flashRadius = 30;
	a->flashScaleFactor = 0.0f;
	a->firingFlashRadius = 50;
	a->firingFlashScaleFactor = 1.0f;
	Q_strncpyz( a->explosionModelName, "models/weaphits/sphere_hi.md3", sizeof(a->explosionModelName) );
	a->explosionModel = trap_R_RegisterModel( a->explosionModelName );
	Q_strncpyz( a->explosionShaderName, "UltimateBlastExplosionShader", sizeof(a->explosionShaderName) );
	a->explosionShader = trap_R_RegisterShader( a->explosionShaderName );
	a->explosionRocks = 10;
	a->explosionSmoke = 3;
	a->explosionSparks = 20;
	a->explosionSmokeRadius = 200;
	a->explosionSmokeLife = 1500;
	a->explosionSmokeSpeed = 10;
	a->explosionScaleFactor = 3;
	a->explosionScaleFactorChargeMult = 1;
	a->explosionRingScaleFactor = 4;
	a->explosionRingScaleFactorChargeMult = 1;
	a->explosionShellScaleFactor = 6;
	a->explosionShellScaleFactorChargeMult = 1;

	cgs.media.monsterAttackLoaded = qtrue;
}

/*
================
CG_SetDefaultSkinConfig

Sets default skin config, a regular ki blast
================
*/
void CG_SetDefaultSkinConfig( bfpSkinConfig_t *config ) {
	int	i;
	memset( config, 0, sizeof(*config) );

	for ( i = 0; i < BFP_NUM_WEAPONS; i++ ) {
		bfpAttackSkinConfig_t	*a = &config->attacks[i];

		Q_strncpyz( a->attackName, "Ki Blast", sizeof(a->attackName) );
		a->attackIcon = trap_R_RegisterShaderNoMip( "icons/kiblast" );
		Q_strncpyz( a->attackTag, "tag_right", sizeof(a->attackTag) );
		Q_strncpyz( a->attackTagPart, "torso", sizeof(a->attackTagPart) );
		a->constantFireAttack = 0;
		a->lightningBolt = 0;
		a->noExplosion = 0;
		a->noExplosionSound = 0;
		Q_strncpyz( a->missileSoundPath, "sound/weapons/rocket/rockfly.wav", sizeof(a->missileSoundPath) );
		a->missileSound = trap_S_RegisterSound( a->missileSoundPath, qfalse );
		Q_strncpyz( a->flashSoundPath, "sound/weapons/bfg/bfg_fire.wav", sizeof(a->flashSoundPath) );
		a->flashSound = trap_S_RegisterSound( a->flashSoundPath, qfalse );
		Q_strncpyz( a->missileShaderName, "KiBlastAttackShader", sizeof(a->missileShaderName) );
		a->missileShader = trap_R_RegisterShader( a->missileShaderName );
		Q_strncpyz( a->missileModelName, "models/weaphits/kiblast.md3", sizeof(a->missileModelName) );
		a->missileModel = trap_R_RegisterModel( a->missileModelName );
		a->missileScaleFactor = 3.0f;
		a->missileTrailFunc = MISSILE_TRAIL_FUNC_ROCKET;
		a->missileDlight = 200;
		a->missileDlightColor[0] = 1.0f; a->missileDlightColor[1] = 0.75f; a->missileDlightColor[2] = 0.0f;
		a->missileModelRotation = 0.2f;
		a->explosionRing = 1;
		a->explosionShell = 1;
		Q_strncpyz( a->explosionModelName, "models/weaphits/sphere_hi.md3", sizeof(a->explosionModelName) );
		a->explosionModel = trap_R_RegisterModel( a->explosionModelName );
		Q_strncpyz( a->explosionShaderName, "KiBlastExplosionShader", sizeof(a->explosionShaderName) );
		a->explosionShader = trap_R_RegisterShader( a->explosionShaderName );
		a->explosionRocks = 10;
		a->explosionSmoke = 3;
		a->explosionSparks = 20;
		a->explosionSmokeRadius = 200;
		a->explosionSmokeLife = 1500;
		a->explosionSmokeSpeed = 10;
		a->explosionScaleFactor = 0.95f;
		a->explosionRingScaleFactor = 0.95f;
		a->explosionShellScaleFactor = 0.95f;
	}

	config->loaded = qtrue;
}

/*
================
CG_GetAttackConfig

Get saved skin attack config
================
*/
bfpAttackSkinConfig_t *CG_GetAttackConfig( int clientNum, int weaponNum ) {
	clientInfo_t	*ci;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		return NULL;
	}

	ci = &cgs.clientinfo[clientNum];
	if ( !ci->infoValid || weaponNum < 0 || weaponNum >= BFP_NUM_WEAPONS ) {
		return NULL;
	}

	// BFP - Monster gamemode, set player monster skin config
	if ( cgs.gametype == GT_MONSTER && cgs.monster > 0
	&& ( cg_entities[ clientNum ].currentState.eFlags & EF_MONSTER ) ) {
		CG_SetMonsterSkinConfig();
		return &cgs.media.monsterAttack;
	}

	return &ci->skinConfig.attacks[weaponNum];
}

/*
================
CG_GetBFPWeaponForSlot
================
*/
bfpWeapon_t *CG_GetBFPWeaponForSlot( int clientNum, int slot ) {
	char	*modelName;
	int		weaponNum = -1;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		return NULL;
	}
	modelName = cgs.clientinfo[ clientNum ].modelName;
	if ( !modelName[0] ) {
		return NULL;
	}
	weaponNum = BG_GetWeaponNumForSlot( modelName, slot );
	if ( weaponNum == -1 ) {
		return NULL;
	}
	return BG_FindBFPBFPWeapon( weaponNum );
}

/*
======================
CG_LoadSkinConfig

Loads and merges the skin config cascade for (modelName, skinName)
into ci->skinConfig, per 3-layer cascade:

1. <attackset defaultModel>/default.cfg (e.g. bfp3-ryuujin/default.cfg)
2. <modelName>/default.cfg (e.g. bfp3-moi/default.cfg)
3. <modelName>/<skinName>.cfg, if skinName isn't "default"

Each layer only overrides the specific fields it sets; anything a layer
doesn't mention (including an attackFireVoice/attackChargeVoice "0",
which Create_Custom_Models.md defines as explicitly "turning off" - i.e.
not touching - a value) keeps whatever the previous layer left there.
This is why layer 1 has to be loaded first and separately from layer 2,
even though both are named "default.cfg": layer 2 is what's allowed to
selectively fall back to layer 1's values via "0", so layer 1 must
already be in ci->skinConfig before layer 2 is parsed.

ci->skinConfig is zeroed first, so a totally missing cascade (e.g. no
bfp_attacksets.cfg match at all) just leaves everything at safe zero
defaults rather than stale data from whichever client previously used
this clientInfo_t slot
======================
*/
void CG_LoadSkinConfig( clientInfo_t *ci ) {
	char		filename[MAX_QPATH*2];
	qboolean	loadedAnyFile = qfalse;
	const char	*attacksetDefaultModel;
	int			i;

	memset( &ci->skinConfig, 0, sizeof( ci->skinConfig ) );

	// set default properties, just in case, that applies 
	// if there's like no 'missileModelRotation [attackIndex] "0"' set
	for ( i = 0; i < BFP_NUM_WEAPONS; i++ ) {
		ci->skinConfig.attacks[i].missileModelRotation = .25f;
		ci->skinConfig.attacks[i].missileTrailRadius = 15;
		ci->skinConfig.attacks[i].missileTrailTime = 2000;
	}

	// layer 1: <attackset defaultModel>/default.cfg
	// skipped when modelName itself already IS the attackset's default
	// model - that model's own default.cfg is about to be loaded as
	// layer 2 anyway, so loading it twice here would be redundant (and
	// any "0" in its own file has nothing earlier to fall back to)
	attacksetDefaultModel = BG_FindAttacksetDefaultModelForModel( ci->modelName );
	if ( attacksetDefaultModel && attacksetDefaultModel[0]
	&& Q_stricmp( attacksetDefaultModel, ci->modelName ) ) {
		Com_sprintf( filename, sizeof(filename), "models/players/%s/default.cfg", attacksetDefaultModel );
		if ( CG_LoadSkinConfigFile( filename, &ci->skinConfig ) ) {
			loadedAnyFile = qtrue;
		}
	}

	// layer 2: modelName/default.cfg
	Com_sprintf( filename, sizeof(filename), "models/players/%s/default.cfg", ci->modelName );
	if ( CG_LoadSkinConfigFile( filename, &ci->skinConfig ) ) {
		loadedAnyFile = qtrue;
	}

	// layer 3: modelName/[skinName.cfg]
	if ( ci->skinName[0] && Q_stricmp( ci->skinName, "default" ) ) {
		Com_sprintf( filename, sizeof(filename), "models/players/%s/%s.cfg", ci->modelName, ci->skinName );
		if ( CG_LoadSkinConfigFile( filename, &ci->skinConfig ) ) {
			loadedAnyFile = qtrue;
		}
	}

	// to debug the loaded skin config file
#if 0
{
	int	i, j;
	Com_Printf( "^2=== Skin config for model ^3'%s'^2, skin ^3'%s'^2 ===\n", ci->modelName, ci->skinName );
	for ( i = 0; i < BFP_NUM_WEAPONS; i++ ) {
		bfpAttackSkinConfig_t	*a = &ci->skinConfig.attacks[i];
		Com_Printf( "^3Slot %d:\n", i );
		Com_Printf( "  ^6name^7=^3'%s'^7, ^6icon^7=^3%d\n", a->attackName, a->attackIcon );
		Com_Printf( "  ^6attackTag^7=^3'%s'^7, ^6attackTagPart^7=^3%s\n", a->attackTag, a->attackTagPart );
		Com_Printf( "  ^6constantFireAttack^7=^3%d^7, ^6lightningBolt^7=^3%d^7, ^6noExplosion^7=^3%d^7, ^6noExplosionSound^7=^3%d\n",
			a->constantFireAttack, a->lightningBolt, a->noExplosion, a->noExplosionSound );
		Com_Printf( "  ^6attackFireVoicePath^7=^3'%s'^7 (sfx^7=^3%d^7)\n", a->attackFireVoicePath, a->attackFireVoice );
		for ( j = 0; j < 5; j++ ) {
			if ( a->attackChargeVoicePath[j][0] ) {
				Com_Printf( "  ^6attackChargeVoicePath[%d]^7=^3'%s'^7 (sfx^7=^3%d^7)\n", j, a->attackChargeVoicePath[j], a->attackChargeVoice[j] );
			}
		}
		Com_Printf( "  ^6missileSound^7=^3'%s'^7 (sfx^7=^3%d^7) ^6chargeSound^7=^3'%s'^7 (sfx^7=^3%d^7)\n",
			a->missileSoundPath, a->missileSound, a->chargeSoundPath, a->chargeSound );
		Com_Printf( "  ^6flashSound^7=^3'%s'^7 (sfx^7=^3%d^7) ^6firingSound^7=^3'%s'^7 (sfx^7=^3%d^7)\n",
			a->flashSoundPath, a->flashSound, a->firingSoundPath, a->firingSound );
		Com_Printf( "  ^6missileDlight^7=^3%d^7, ^6color^7=(^3%f^7,^3%f^7,^3%f^7)\n",
			a->missileDlight, a->missileDlightColor[0], a->missileDlightColor[1], a->missileDlightColor[2] );
		Com_Printf( "  ^6missileTrailFunc^7=^3%d^7, ^6trailTime^7=^3%d^7, ^6trailRadius^7=^3%d\n",
			a->missileTrailFunc, a->missileTrailTime, a->missileTrailRadius );
		Com_Printf( "  ^6beamShader^7=^3'%s'^7 (shader^7=^3%d^7) ^6spiralBeamShader^7=^3'%s'^7 (shader^7=^3%d^7)\n",
			a->beamShaderName, a->beamShader, a->spiralBeamShaderName, a->spiralBeamShader );
		Com_Printf( "  ^6flashModel^7=^3'%s'^7 (model^7=^3%d^7) ^6flashShader^7=^3'%s'^7 (shader^7=^3%d^7)\n",
			a->flashModelName, a->flashModel, a->flashShaderName, a->flashShader );
		Com_Printf( "  ^6flashRadius^7=^3%d^7, ^6flashScaleFactor^7=^3%f^7, ^6firingFlashRadius^7=^3%d^7, ^6firingFlashScaleFactor^7=^3%f\n",
			a->flashRadius, a->flashScaleFactor, a->firingFlashRadius, a->firingFlashScaleFactor );
		Com_Printf( "  ^6missileShader^7=^3'%s'^7 (shader^7=^3%d^7) ^6missileModel^7=^3'%s'^7 (model^7=^3%d^7)\n",
			a->missileShaderName, a->missileShader, a->missileModelName, a->missileModel );
		Com_Printf( "  ^6missileRotation^7=^3%d^7, ^6missileModelRotation^7=^3%f^7, ^6missileSpinHoriz^7=^3%d\n",
			a->missileRotation, a->missileModelRotation, a->missileSpinHoriz );
		Com_Printf( "  ^6missileRadius^7=^3%f^7, ^6missileRadiusChargeMult^7=^3%d^7, ^6missileScaleFactor^7=^3%f^7, ^6missileScaleFactorChargeMult^7=^3%f\n",
			a->missileRadius, a->missileRadiusChargeMult, a->missileScaleFactor, a->missileScaleFactorChargeMult );
		Com_Printf( "  ^6explosionModel^7=^3'%s'^7 (model^7=^3%d^7) ^6explosionShader^7=^3'%s'^7 (shader^7=^3%d^7)\n",
			a->explosionModelName, a->explosionModel, a->explosionShaderName, a->explosionShader );
		Com_Printf( "  ^6explosionRing^7=^3%d^7, ^6explosionShell^7=^3%d^7, ^6explosionRocks^7=^3%d^7, ^6explosionSparks^7=^3%d\n",
			a->explosionRing, a->explosionShell, a->explosionRocks, a->explosionSparks );
		Com_Printf( "  ^6explosionSmoke^7=^3%d^7, ^6smokeRadius^7=^3%d^7, ^6smokeLife^7=^3%d^7, ^6smokeSpeed^7=^3%d\n",
			a->explosionSmoke, a->explosionSmokeRadius, a->explosionSmokeLife, a->explosionSmokeSpeed );
		Com_Printf( "  ^6explosionScaleFactor^7=^3%f^7, ^6explosionScaleFactorChargeMult^7=^3%f\n",
			a->explosionScaleFactor, a->explosionScaleFactorChargeMult );
		Com_Printf( "  ^6explosionRingScaleFactor^7=^3%f^7, ^6explosionRingScaleFactorChargeMult^7=^3%f\n",
			a->explosionRingScaleFactor, a->explosionRingScaleFactorChargeMult );
		Com_Printf( "  ^6explosionShellScaleFactor^7=^3%f^7, ^6explosionShellScaleFactorChargeMult^7=^3%f\n",
			a->explosionShellScaleFactor, a->explosionShellScaleFactorChargeMult );
	}
	Com_Printf( "^2=== End skin config debug ===\n" );
}
#endif

	if ( !loadedAnyFile ) {
		CG_SetDefaultSkinConfig( &ci->skinConfig );
		ci->skinConfig.loaded = qfalse;
		return;
	}

	ci->skinConfig.loaded = qtrue;
}
