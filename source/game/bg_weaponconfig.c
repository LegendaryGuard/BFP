/*
===========================================================================

BFP WEAPON CONFIG AND ATTACKSETS

===========================================================================
*/


#include "q_shared.h"
#include "bg_public.h"

int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void	trap_FS_Read( void *buffer, int len, fileHandle_t f );
void	trap_FS_FCloseFile( fileHandle_t f );

typedef struct {
	bfpWeaponDef_t	defs[MAX_BFP_WEAPON_DEFS];
	int				numDefs;
} bfpWeaponList_t;

static bfpWeaponList_t	bfpWeapons;

// BFP - attackType string -> ATK_* value
typedef struct {
	const char	*name;
	int			value;
} attackTypeToken_t;

static const attackTypeToken_t	attackTypeTokens[] = {
	{ "missile",	ATK_MISSILE },
	{ "rdmissile",	ATK_RDMISSILE },
	{ "beam",		ATK_BEAM },
	{ "sbeam",		ATK_SBEAM },
	{ "hitscan",	ATK_HITSCAN },
	{ "forcefield",	ATK_FORCEFIELD },
};
#define	NUM_ATTACKTYPE_TOKENS	( sizeof( attackTypeTokens ) / sizeof( attackTypeTokens[0] ) )

/*
================
BG_FindBFPWeaponDef

Looks up a parsed attack definition by weaponNum. Returns NULL if not found
(caller should fall back to safe defaults rather than crash)

weaponNum -> bfpWeaponDef_t lookup
================
*/
bfpWeaponDef_t *BG_FindBFPWeaponDef( int weaponNum ) {
	int	i;

	for ( i = 0; i < bfpWeapons.numDefs; i++ ) {
		if ( bfpWeapons.defs[i].inuse && bfpWeapons.defs[i].weaponNum == weaponNum ) {
			return &bfpWeapons.defs[i];
		}
	}

	return NULL;
}

/*
================
BG_SetDefaultWeaponDef

Sets default weapon properties. A regular missile attack type
================
*/
bfpWeaponDef_t *BG_SetDefaultWeaponDef( void ) {
	static bfpWeaponDef_t	def;

	def.inuse = qtrue;
	Q_strncpyz( def.attackName, "ki_blast", sizeof(def.attackName) );
	def.weaponNum = 21;
	def.attackType = ATK_MISSILE;
	def.weaponTime = 1000;
	def.randomWeaponTime = 0;
	def.kiCostAsPct = qfalse;
	def.kiPct = 0;
	def.kiCost = 10;
	def.chargeAttack = qfalse;
	def.chargeAutoFire = qfalse;
	def.minCharge = 0;
	def.maxCharge = 0;
	def.damage = 20;
	def.splashDamage = 20;
	def.chargeDamageMult = 0;
	def.maxDamage = 0;
	def.radius = 20;
	def.explosionRadius = 125;
	def.chargeRadiusMult = 0;
	def.chargeExpRadiusMult = 0;
	def.maxRadius = 0;
	def.maxExpRadius = 0;
	def.missileSpeed = 5000;
	def.homing = 0;
	def.homingRange = 0;
	def.homingAcceleration = 0;
	def.range = 0;
	def.loopingAnim = qfalse;
	def.noAttackAnim = qfalse;
	def.alternatingXOffset = 0;
	def.randYOffset = 0;
	def.randXOffset = 0;
	def.coneOfFireX = 0;
	def.coneOfFireY = 0;
	def.piercing = qfalse;
	def.reflective = qfalse;
	def.priority = 0;
	def.blinding = qfalse;
	def.extraKnockback = 0;
	def.railTrail = qfalse;
	def.movementPenalty = 0;
	def.missileGravity = 0;
	def.missileAcceleration = 0;
	def.multishot = 0;
	def.bounces = qfalse;
	def.noZBounce = qfalse;
	def.bounceFriction = 0;
	def.missileDuration = 0;
	def.explosionSpawn = 0;

	return &def;
}

/*
================
BG_SetMonsterDefaultWeaponDef

Sets default monster weapon properties. A sbeam attack type
================
*/
bfpWeaponDef_t *BG_SetMonsterDefaultWeaponDef( void ) {
	static bfpWeaponDef_t	def;

	def.inuse = qtrue;
	Q_strncpyz( def.attackName, "mouthbeam", sizeof(def.attackName) );
	def.weaponNum = 29;
	def.attackType = ATK_SBEAM;
	def.weaponTime = 250;
	def.randomWeaponTime = 0;
	def.kiCostAsPct = qfalse;
	def.kiPct = 0;
	def.kiCost = 250;
	def.chargeAttack = qfalse;
	def.chargeAutoFire = qfalse;
	def.minCharge = 0;
	def.maxCharge = 0;
	def.damage = 15;
	def.splashDamage = 15;
	def.chargeDamageMult = 0;
	def.maxDamage = 15;
	def.radius = 50;
	def.explosionRadius = 200;
	def.chargeRadiusMult = 0;
	def.chargeExpRadiusMult = 0;
	def.maxRadius = 0;
	def.maxExpRadius = 0;
	def.missileSpeed = 1000;
	def.homing = 0;
	def.homingRange = 0;
	def.homingAcceleration = 0;
	def.range = 0;
	def.loopingAnim = qfalse;
	def.noAttackAnim = qfalse;
	def.alternatingXOffset = 0;
	def.randYOffset = 0;
	def.randXOffset = 0;
	def.coneOfFireX = 0;
	def.coneOfFireY = 0;
	def.piercing = qfalse;
	def.reflective = qfalse;
	def.priority = 1;
	def.blinding = qfalse;
	def.extraKnockback = 0;
	def.railTrail = qfalse;
	def.movementPenalty = 0;
	def.missileGravity = 0;
	def.missileAcceleration = 0;
	def.multishot = 0;
	def.bounces = qfalse;
	def.noZBounce = qfalse;
	def.bounceFriction = 0;
	def.missileDuration = 10000;
	def.explosionSpawn = 0;

	return &def;
}

/*
================
BG_ReadConfigToken

Token reader shared by every directive below.
Copies the next whitespace-delimited token from *ptr into value,
advances *ptr past it (leaving *ptr at the following whitespace/EOL)
================
*/
static void BG_ReadConfigToken( char **ptr, char *value, int valueSize ) {
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
BG_ParseAttackTypeToken

Maps an attackType string token (e.g. "hitscan") to its ATK_* value.
Returns qtrue and fills *outType if recognized, qfalse otherwise (cur
keeps whatever attackType it had, i.e. ATK_MISSILE from the memset)

attackType string -> ATK_* value
================
*/
static qboolean BG_ParseAttackTypeToken( const char *value, int *outType ) {
	int	i;

	for ( i = 0; i < NUM_ATTACKTYPE_TOKENS; i++ ) {
		if ( !Q_stricmp( value, attackTypeTokens[i].name ) ) {
			*outType = attackTypeTokens[i].value;
			return qtrue;
		}
	}

	return qfalse;
}

/*
================
BG_ParseBFPWeaponConfigFile

Parses a loaded bfp_weapon.cfg-style buffer into bfpWeapons,
appending to whatever was already loaded (used to 
chain bfp_weapon.cfg then bfp_weapon2.cfg into the same table)
================
*/
static void BG_ParseBFPWeaponConfigFile( char *buf ) {
	char			*ptr;
	bfpWeaponDef_t	*cur;

	cur = NULL;
	ptr = buf;

	while ( *ptr ) {
		// skip whitespace at line start
		while ( *ptr && ( *ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n' ) ) {
			ptr++;
		}

		if ( !*ptr ) {
			break;
		}

		if ( !Q_stricmpn( ptr, "end", 3 ) ) { // "end" marks the end of file
			break;
		}

		// every recognized directive is "token value", where value is one whitespace-delimited word
		if ( !Q_stricmpn( ptr, "weaponNum", 9 ) && ( ptr[9] == ' ' || ptr[9] == '\t' ) ) {
			char	value[32];
			ptr += 9;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->weaponNum = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "attackType", 10 ) && ( ptr[10] == ' ' || ptr[10] == '\t' ) ) {
			char	value[32];
			ptr += 10;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				BG_ParseAttackTypeToken( value, &cur->attackType );
			}
		} else if ( !Q_stricmpn( ptr, "weaponTime", 10 ) && ( ptr[10] == ' ' || ptr[10] == '\t' ) ) {
			char	value[32];
			ptr += 10;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->weaponTime = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "randomWeaponTime", 16 ) && ( ptr[16] == ' ' || ptr[16] == '\t' ) ) {
			char	value[32];
			ptr += 16;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->randomWeaponTime = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "kiCostAsPct", 11 ) && ( ptr[11] == ' ' || ptr[11] == '\t' ) ) {
			char	value[32];
			ptr += 11;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->kiCostAsPct = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "kiPct", 5 ) && ( ptr[5] == ' ' || ptr[5] == '\t' ) ) {
			char	value[32];
			ptr += 5;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->kiPct = (float)atof( value );
			}
		} else if ( !Q_stricmpn( ptr, "kiCost", 6 ) && ( ptr[6] == ' ' || ptr[6] == '\t' ) ) {
			char	value[32];
			ptr += 6;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->kiCost = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "chargeAttack", 12 ) && ( ptr[12] == ' ' || ptr[12] == '\t' ) ) {
			char	value[32];
			ptr += 12;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->chargeAttack = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "chargeAutoFire", 14 ) && ( ptr[14] == ' ' || ptr[14] == '\t' ) ) {
			char	value[32];
			ptr += 14;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->chargeAutoFire = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "minCharge", 9 ) && ( ptr[9] == ' ' || ptr[9] == '\t' ) ) {
			char	value[32];
			ptr += 9;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->minCharge = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "maxCharge", 9 ) && ( ptr[9] == ' ' || ptr[9] == '\t' ) ) {
			char	value[32];
			ptr += 9;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->maxCharge = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "chargeDamageMult", 16 ) && ( ptr[16] == ' ' || ptr[16] == '\t' ) ) {
			char	value[32];
			ptr += 16;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->chargeDamageMult = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "maxDamage", 9 ) && ( ptr[9] == ' ' || ptr[9] == '\t' ) ) {
			char	value[32];
			ptr += 9;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->maxDamage = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "damage", 6 ) && ( ptr[6] == ' ' || ptr[6] == '\t' ) ) {
			char	value[32];
			ptr += 6;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->damage = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "splashDamage", 12 ) && ( ptr[12] == ' ' || ptr[12] == '\t' ) ) {
			char	value[32];
			ptr += 12;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->splashDamage = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "explosionRadius", 15 ) && ( ptr[15] == ' ' || ptr[15] == '\t' ) ) {
			char	value[32];
			ptr += 15;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->explosionRadius = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "chargeRadiusMult", 16 ) && ( ptr[16] == ' ' || ptr[16] == '\t' ) ) {
			char	value[32];
			ptr += 16;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->chargeRadiusMult = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "chargeExpRadiusMult", 19 ) && ( ptr[19] == ' ' || ptr[19] == '\t' ) ) {
			char	value[32];
			ptr += 19;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->chargeExpRadiusMult = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "maxExpRadius", 12 ) && ( ptr[12] == ' ' || ptr[12] == '\t' ) ) {
			char	value[32];
			ptr += 12;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->maxExpRadius = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "maxRadius", 9 ) && ( ptr[9] == ' ' || ptr[9] == '\t' ) ) {
			char	value[32];
			ptr += 9;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->maxRadius = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "radius", 6 ) && ( ptr[6] == ' ' || ptr[6] == '\t' ) ) {
			char	value[32];
			ptr += 6;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->radius = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "missileSpeed", 12 ) && ( ptr[12] == ' ' || ptr[12] == '\t' ) ) {
			char	value[32];
			ptr += 12;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->missileSpeed = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "homing", 6 ) && ( ptr[6] == ' ' || ptr[6] == '\t' ) ) {
			char	value[32];
			ptr += 6;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->homing = (float)atof( value );
			}
		} else if ( !Q_stricmpn( ptr, "homingRange", 11 ) && ( ptr[11] == ' ' || ptr[11] == '\t' ) ) {
			char	value[32];
			ptr += 11;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->homingRange = (float)atof( value );
			}
		} else if ( !Q_stricmpn( ptr, "homingAcceleration", 18 ) && ( ptr[18] == ' ' || ptr[18] == '\t' ) ) {
			char	value[32];
			ptr += 18;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->homingAcceleration = (float)atof( value );
			}
		} else if ( !Q_stricmpn( ptr, "range", 5 ) && ( ptr[5] == ' ' || ptr[5] == '\t' ) ) {
			char	value[32];
			ptr += 5;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->range = (float)atof( value );
			}
		} else if ( !Q_stricmpn( ptr, "loopingAnim", 11 ) && ( ptr[11] == ' ' || ptr[11] == '\t' ) ) {
			char	value[32];
			ptr += 11;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->loopingAnim = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "noAttackAnim", 12 ) && ( ptr[12] == ' ' || ptr[12] == '\t' ) ) {
			char	value[32];
			ptr += 12;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->noAttackAnim = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "alternatingXOffset", 18 ) && ( ptr[18] == ' ' || ptr[18] == '\t' ) ) {
			char	value[32];
			ptr += 18;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->alternatingXOffset = (float)atof( value );
			}
		} else if ( !Q_stricmpn( ptr, "randYOffset", 11 ) && ( ptr[11] == ' ' || ptr[11] == '\t' ) ) {
			char	value[32];
			ptr += 11;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->randYOffset = (float)atof( value );
			}
		} else if ( !Q_stricmpn( ptr, "randXOffset", 11 ) && ( ptr[11] == ' ' || ptr[11] == '\t' ) ) {
			char	value[32];
			ptr += 11;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->randXOffset = (float)atof( value );
			}
		} else if ( !Q_stricmpn( ptr, "coneOfFireX", 11 ) && ( ptr[11] == ' ' || ptr[11] == '\t' ) ) {
			char	value[32];
			ptr += 11;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->coneOfFireX = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "coneOfFireY", 11 ) && ( ptr[11] == ' ' || ptr[11] == '\t' ) ) {
			char	value[32];
			ptr += 11;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->coneOfFireY = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "piercing", 8 ) && ( ptr[8] == ' ' || ptr[8] == '\t' ) ) {
			char	value[32];
			ptr += 8;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->piercing = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "reflective", 10 ) && ( ptr[10] == ' ' || ptr[10] == '\t' ) ) {
			char	value[32];
			ptr += 10;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->reflective = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "priority", 8 ) && ( ptr[8] == ' ' || ptr[8] == '\t' ) ) {
			char	value[32];
			ptr += 8;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->priority = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "blinding", 8 ) && ( ptr[8] == ' ' || ptr[8] == '\t' ) ) {
			char	value[32];
			ptr += 8;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->blinding = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "missileGravity", 14 ) && ( ptr[14] == ' ' || ptr[14] == '\t' ) ) {
			char	value[32];
			ptr += 14;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->missileGravity = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "missileAcceleration", 19 ) && ( ptr[19] == ' ' || ptr[19] == '\t' ) ) {
			char	value[32];
			ptr += 19;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->missileAcceleration = (float)atof( value );
			}
		} else if ( !Q_stricmpn( ptr, "missileDuration", 15 ) && ( ptr[15] == ' ' || ptr[15] == '\t' ) ) {
			char	value[32];
			ptr += 15;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->missileDuration = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "multishot", 9 ) && ( ptr[9] == ' ' || ptr[9] == '\t' ) ) {
			char	value[32];
			ptr += 9;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->multishot = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "bounceFriction", 14 ) && ( ptr[14] == ' ' || ptr[14] == '\t' ) ) {
			char	value[32];
			ptr += 14;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->bounceFriction = (float)atof( value );
			}
		} else if ( !Q_stricmpn( ptr, "bounces", 7 ) && ( ptr[7] == ' ' || ptr[7] == '\t' ) ) {
			char	value[32];
			ptr += 7;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->bounces = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "noZBounce", 9 ) && ( ptr[9] == ' ' || ptr[9] == '\t' ) ) {
			char	value[32];
			ptr += 9;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->noZBounce = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "extraKnockback", 14 ) && ( ptr[14] == ' ' || ptr[14] == '\t' ) ) {
			char	value[32];
			ptr += 14;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->extraKnockback = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "railTrail", 9 ) && ( ptr[9] == ' ' || ptr[9] == '\t' ) ) {
			char	value[32];
			ptr += 9;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->railTrail = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "movementPenalty", 15 ) && ( ptr[15] == ' ' || ptr[15] == '\t' ) ) {
			char	value[32];
			ptr += 15;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->movementPenalty = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "explosionSpawn", 14 ) && ( ptr[14] == ' ' || ptr[14] == '\t' ) ) {
			char	value[32];
			ptr += 14;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
			if ( cur ) {
				cur->explosionSpawn = atoi( value );
			}
		} else if ( !Q_stricmpn( ptr, "usesGravity", 11 ) && ( ptr[11] == ' ' || ptr[11] == '\t' ) ) {
			// NOTE: usesGravity is unused, skip its value
			char	value[32];
			ptr += 11;
			BG_ReadConfigToken( &ptr, value, sizeof( value ) );
		} else {
			// not a known keyword: treat the whole line as an (attack_name) tag,
			// which opens a new attack block
			char	value[MAX_QPATH];
			char	cleanedName[MAX_QPATH];
			int		i, len;

			i = 0;
			while ( ptr[i] && ptr[i] != ' ' && ptr[i] != '\t' && ptr[i] != '\n' && ptr[i] != '\r' && i < sizeof( value ) - 1 ) {
				value[i] = ptr[i];
				i++;
			}
			value[i] = 0;
			ptr += i;

			// extract the name in the parentheses if exist
			if ( value[0] == '(' ) {
				char	*open = strchr( value, '(' );
				char	*close = Q_strrchr( value, ')' );
				if ( open && close && close > open ) {
					len = close - open - 1;
					if ( len > 0 && len < sizeof(cleanedName) ) {
						char	*start, *end;
						strncpy( cleanedName, open + 1, len );
						cleanedName[len] = 0;
						// remove spaces at the start and end
						start = cleanedName;
						while ( *start && *start <= ' ' ) {
							start++;
						}
						end = start + strlen( start ) - 1;
						while ( end > start && *end <= ' ' ) {
							end--;
						}
						if ( end > start ) {
							*(end + 1) = 0;
							if ( start != cleanedName ) {
								memmove( cleanedName, start, strlen( start ) + 1 );
							}
						} else {
							cleanedName[0] = 0;
						}
						Q_strncpyz( value, cleanedName, sizeof(value) );
					}
				}
			}

			if ( value[0] && bfpWeapons.numDefs < MAX_BFP_WEAPON_DEFS ) {
				cur = &bfpWeapons.defs[bfpWeapons.numDefs];
				memset( cur, 0, sizeof( bfpWeaponDef_t ) );
				cur->inuse = qtrue;
				cur->attackType = ATK_MISSILE; // default attackType
				Q_strncpyz( cur->attackName, value, sizeof( cur->attackName ) );
				bfpWeapons.numDefs++;
			} else if ( value[0] ) {
				Com_Printf( S_COLOR_YELLOW "WARNING: Maximum BFP weapon defs reached, ignoring further attacks in bfp_weapon.cfg\n" );
				cur = NULL;
			}
			// if value[0] is empty here, the line was blank/unparseable; skip it silently
		}

		// skip to next line (handle both \n and \r\n)
		while ( *ptr && ( *ptr == '\n' || *ptr == '\r' ) ) {
			ptr++;
		}
	}
}

/*
================
BG_LoadBFPWeaponConfigFile

Loads one named file into a local buffer and hands it to
BG_ParseBFPWeaponConfigFile. Returns qtrue if the file existed and was
read, qfalse = not found
================
*/
static qboolean BG_LoadBFPWeaponConfigFile( const char *fileName ) {
	fileHandle_t	f;
	int				len;
	static char		buf[BFP_CFG_BUFFER_SIZE];

	len = trap_FS_FOpenFile( fileName, &f, FS_READ );
	if ( !f ) {
		return qfalse;
	}

	if ( len >= sizeof( buf ) ) {
		trap_FS_FCloseFile( f );
		Com_Printf( "BFP weapon config file too long: %s\n", fileName );
		return qfalse;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	//Com_Printf( "reading %s\n", fileName );

	BG_ParseBFPWeaponConfigFile( buf );

	return qtrue;
}

/*
======================
BG_LoadBFPWeaponConfig

Loads and parses bfp_weapon.cfg, then bfp_weapon2.cfg if it exists,
appending both into the same weaponNum-indexed table
======================
*/
void BG_LoadBFPWeaponConfig( void ) {
	bfpWeapons.numDefs = 0;

	BG_LoadBFPWeaponConfigFile( "bfp_weapon.cfg" );
	BG_LoadBFPWeaponConfigFile( "bfp_weapon2.cfg" );

	// to debug the loaded weapon defs
#if 0
{
	int	i;
	for ( i = 0; i < bfpWeapons.numDefs; i++ ) {
		const bfpWeaponDef_t *d = &bfpWeapons.defs[i];
		Com_Printf( "^6[%d] ^3%s ^6(weaponNum=^3%d^6)^7\n", i, d->attackName, d->weaponNum );
		Com_Printf( "  ^2attackType: ^3%d ^7(%s)\n", d->attackType,
			( d->attackType == ATK_MISSILE ) ? "missile" :
			( d->attackType == ATK_RDMISSILE ) ? "rdmissile" :
			( d->attackType == ATK_BEAM ) ? "beam" :
			( d->attackType == ATK_SBEAM ) ? "sbeam" :
			( d->attackType == ATK_HITSCAN ) ? "hitscan" :
			( d->attackType == ATK_FORCEFIELD ) ? "forcefield" : "UNKNOWN" );
		Com_Printf( "  ^2weaponTime: ^3%d  ^2randomWeaponTime: ^3%d\n", d->weaponTime, d->randomWeaponTime );
		Com_Printf( "  ^2kiCostAsPct: ^3%d  ^2kiPct: ^3%f  ^2kiCost: ^3%d\n", d->kiCostAsPct, d->kiPct, d->kiCost );
		Com_Printf( "  ^2chargeAttack: ^3%d  ^2chargeAutoFire: ^3%d  ^2minCharge: ^3%d  ^2maxCharge: ^3%d\n",
			d->chargeAttack, d->chargeAutoFire, d->minCharge, d->maxCharge );
		Com_Printf( "  ^2damage: ^3%d  ^2splashDamage: ^3%d  ^2chargeDamageMult: ^3%d  ^2maxDamage: ^3%d\n",
			d->damage, d->splashDamage, d->chargeDamageMult, d->maxDamage );
		Com_Printf( "  ^2radius: ^3%d  ^2explosionRadius: ^3%d  ^2chargeRadiusMult: ^3%d  ^2chargeExpRadiusMult: ^3%d\n",
			d->radius, d->explosionRadius, d->chargeRadiusMult, d->chargeExpRadiusMult );
		Com_Printf( "  ^2maxRadius: ^3%d  ^2maxExpRadius: ^3%d\n", d->maxRadius, d->maxExpRadius );
		Com_Printf( "  ^2missileSpeed: ^3%d  ^2homing: ^3%f  ^2homingRange: ^3%f  ^2homingAcceleration: ^3%f\n",
			d->missileSpeed, d->homing, d->homingRange, d->homingAcceleration );
		Com_Printf( "  ^2range: ^3%f\n", d->range );
		Com_Printf( "  ^2loopingAnim: ^3%d  ^2noAttackAnim: ^3%d\n", d->loopingAnim, d->noAttackAnim );
		Com_Printf( "  ^2alternatingXOffset: ^3%f  ^2randYOffset: ^3%f  ^2randXOffset: ^3%f\n",
			d->alternatingXOffset, d->randYOffset, d->randXOffset );
		Com_Printf( "  ^2coneOfFireX: ^3%d  ^2coneOfFireY: ^3%d\n", d->coneOfFireX, d->coneOfFireY );
		Com_Printf( "  ^2piercing: ^3%d  ^2reflective: ^3%d  ^2priority: ^3%d  ^2blinding: ^3%d\n",
			d->piercing, d->reflective, d->priority, d->blinding );
		Com_Printf( "  ^2missileGravity: ^3%d  ^2missileAcceleration: ^3%f  ^2missileDuration: ^3%d\n",
			d->missileGravity, d->missileAcceleration, d->missileDuration );
		Com_Printf( "  ^2multishot: ^3%d  ^2bounces: ^3%d  ^2bounceFriction: ^3%f  ^2noZBounce: ^3%d\n",
			d->multishot, d->bounces, d->bounceFriction, d->noZBounce );
		Com_Printf( "  ^2extraKnockback: ^3%d  ^2railTrail: ^3%d  ^2movementPenalty: ^3%d  ^2explosionSpawn: ^3%d\n",
			d->extraKnockback, d->railTrail, d->movementPenalty, d->explosionSpawn );
	}
	Com_Printf( "^2LOADED BFP WEAPON DEFS: ^3%d\n", bfpWeapons.numDefs );
}
#endif
}

/*
===========================================================================

ATTACKSETS

Maps each player model prefix group (e.g. "bfp1-") to 5 weaponNum entries,
one per attack slot (WP_ATTACK_0..WP_ATTACK_4). Moved here (from g_utils.c)
so bg_pmove.c can resolve pm->ps->weapon (a slot 0-4) straight into a
weaponNum via BG_GetWeaponNumForSlot(), without any extra playerState_t stat
===========================================================================
*/

typedef struct {
	bfpAttacksetGroup_t	sets[MAX_BFP_ATTACKSETS];
	int					numSets;
} bfpAttacksetList_t;

static bfpAttacksetList_t	bfpAttacksets;

/*
================
BG_ModelMatchesAnyAttacksetPrefix

Returns qtrue if "modelName" starts with (case-insensitive) one of the
modelPrefix values loaded from bfp_attacksets.cfg. Used by g_utils.c to
filter models/players entries so only models covered by an attackset are
considered valid, regardless of whether the prefix itself ends in '-'
(e.g. "bfp1-" vs "a17")
================
*/
qboolean BG_ModelMatchesAnyAttacksetPrefix( const char *modelName ) {
	int	i, prefixLen;

	for ( i = 0; i < bfpAttacksets.numSets; ++i ) {
		prefixLen = (int)strlen( bfpAttacksets.sets[i].modelPrefix );
		if ( prefixLen == 0 ) {
			continue;
		}
		if ( !Q_stricmpn( modelName, bfpAttacksets.sets[i].modelPrefix, prefixLen ) ) {
			return qtrue;
		}
	}

	return qfalse;
}

/*
==================
BG_FindAttacksetDefaultModel

Looks up the attackset group whose modelPrefix matches "prefix" (e.g. "bfp1-")
and returns its configured defaultModel, or NULL if no attackset matches the 
prefix, or a matching attackset has no defaultModel set
==================
*/
const char *BG_FindAttacksetDefaultModel( const char *prefix ) {
	int	i;

	for ( i = 0; i < bfpAttacksets.numSets; i++ ) {
		if ( !Q_stricmp( bfpAttacksets.sets[i].modelPrefix, prefix ) ) {
			if ( !bfpAttacksets.sets[i].defaultModel[0] ) {
				return NULL; // matched prefix but no defaultModel was set for it
			}
			return bfpAttacksets.sets[i].defaultModel;
		}
	}

	return NULL;
}

/*
==================
BG_GetWeaponNumForSlot

Resolves (player model name, attack slot 0-4) -> weaponNum, by finding
which attackset group's modelPrefix the model name starts with, then
reading that group's attack[attackSlot] entry.

Returns -1 if no attackset group matches the model, or attackSlot is out
of range; callers (BG_FindBFPWeaponDef via pm->ps->weapon) should treat
that as "no attack definition available" rather than crash

(playerModel, slot 0-4) -> weaponNum
==================
*/
int BG_GetWeaponNumForSlot( const char *modelName, int attackSlot ) {
	int	i, prefixLen;

	if ( attackSlot < 0 || attackSlot >= BFP_NUM_WEAPONS ) {
		return -1;
	}

	for ( i = 0; i < bfpAttacksets.numSets; ++i ) {
		prefixLen = (int)strlen( bfpAttacksets.sets[i].modelPrefix );
		if ( prefixLen == 0 ) {
			continue;
		}
		if ( !Q_stricmpn( modelName, bfpAttacksets.sets[i].modelPrefix, prefixLen ) ) {
			return bfpAttacksets.sets[i].attack[attackSlot];
		}
	}

	return -1;
}

/*
=========================================
Per-client attack slot -> weaponNum cache
=========================================
*/
static bfpWeaponDef_t *bfpClientAttackWeaponDefs[MAX_CLIENTS][BFP_NUM_WEAPONS];
static qboolean bfpClientAttackWeaponDefsSet[MAX_CLIENTS];

/*
==================
BG_SetClientAttackWeaponNums

Resolves and caches all 5 attack slots -> weaponNum for one client's
current model. Call this whenever a client's model changes; it does the
BG_GetWeaponNumForSlot lookup once per slot right away so later
BG_GetClientWeaponNumForSlot calls are a plain array read
==================
*/
void BG_SetClientAttackWeaponNums( int clientNum, const char *modelName ) {
	int	slot;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		return;
	}

	for ( slot = 0; slot < BFP_NUM_WEAPONS; slot++ ) {
		int	wn = BG_GetWeaponNumForSlot( modelName, slot );
		bfpClientAttackWeaponDefs[clientNum][slot] = ( wn != -1 ) ? BG_FindBFPWeaponDef( wn ) : NULL;
	}
	bfpClientAttackWeaponDefsSet[clientNum] = qtrue;
}

/*
==================
BG_GetClientWeaponNumForSlot

Reads back a value cached by BG_SetClientAttackWeaponNums: which
weaponNum this client's current model has assigned to the given attack
slot. Returns -1 if the client index is out of range, the slot is out of
range, or nothing has been cached for this client yet (e.g. before its
first ClientUserinfoChanged on the server, or before cgame has learned
this client's model)
==================
*/
bfpWeaponDef_t *BG_GetClientWeaponDefForSlot( int clientNum, int attackSlot ) {
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		return NULL;
	}
	if ( attackSlot < 0 || attackSlot >= BFP_NUM_WEAPONS ) {
		return NULL;
	}
	if ( !bfpClientAttackWeaponDefsSet[clientNum] ) {
		return NULL;
	}
	return bfpClientAttackWeaponDefs[clientNum][attackSlot];
}

/*
======================
BG_LoadBFPAttacksetsConfig

Loads and parses bfp_attacksets.cfg file
======================
*/
void BG_LoadBFPAttacksetsConfig( void ) {
	fileHandle_t			f;
	int						len;
	static char				buf[BFP_CFG_BUFFER_SIZE];
	char					*ptr;
	bfpAttacksetGroup_t		*cur;
	const char				*ATTACKSETS_FILENAME = "bfp_attacksets.cfg";

	bfpAttacksets.numSets = 0;

	len = trap_FS_FOpenFile( ATTACKSETS_FILENAME, &f, FS_READ );
	if ( !f ) {
		return;
	}

	if ( len >= sizeof( buf ) ) {
		trap_FS_FCloseFile( f );
		Com_Printf( "BFP attacksets config file too long\n" );
		return;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	Com_Printf( "reading %s\n", ATTACKSETS_FILENAME );

	cur = NULL;
	ptr = buf;

	while ( *ptr ) {
		// skip whitespace at line start
		while ( *ptr && ( *ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n' ) ) {
			ptr++;
		}

		if ( !*ptr ) {
			break;
		}

		if ( !Q_stricmpn( ptr, "end", 3 ) ) { // "end" marks the end of file
			break;
		}

		if ( !Q_stricmpn( ptr, "attackset", 9 ) && ( ptr[9] == ' ' || ptr[9] == '\t' ) ) {
			char	value[32];
			int		i;

			ptr += 9;
			while ( *ptr && ( *ptr == ' ' || *ptr == '\t' ) ) {
				ptr++;
			}

			i = 0;
			while ( *ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != '\r' && i < sizeof( value ) - 1 ) {
				value[i++] = *ptr++;
			}
			value[i] = 0;

			if ( bfpAttacksets.numSets < MAX_BFP_ATTACKSETS ) {
				cur = &bfpAttacksets.sets[bfpAttacksets.numSets];
				memset( cur, 0, sizeof( bfpAttacksetGroup_t ) );
				cur->inuse = qtrue;
				cur->attacksetId = atoi( value );
				bfpAttacksets.numSets++;
			} else {
				Com_Printf( S_COLOR_YELLOW "WARNING: Maximum BFP attacksets reached, ignoring further attacksets in %s\n", ATTACKSETS_FILENAME );
				cur = NULL;
			}
		} else if ( !Q_stricmpn( ptr, "attack", 6 ) && ( ptr[6] == ' ' || ptr[6] == '\t' ) ) {
			char	idxValue[32], weaponValue[32];
			int		i, attackIdx;

			ptr += 6;
			while ( *ptr && ( *ptr == ' ' || *ptr == '\t' ) ) {
				ptr++;
			}

			i = 0;
			while ( *ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != '\r' && i < sizeof( idxValue ) - 1 ) {
				idxValue[i++] = *ptr++;
			}
			idxValue[i] = 0;

			while ( *ptr && ( *ptr == ' ' || *ptr == '\t' ) ) {
				ptr++;
			}

			i = 0;
			while ( *ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != '\r' && i < sizeof( weaponValue ) - 1 ) {
				weaponValue[i++] = *ptr++;
			}
			weaponValue[i] = 0;

			attackIdx = atoi( idxValue );
			if ( cur && attackIdx >= 0 && attackIdx < BFP_NUM_WEAPONS ) {
				cur->attack[attackIdx] = atoi( weaponValue );
			}
		} else if ( !Q_stricmpn( ptr, "modelPrefix", 11 ) && ( ptr[11] == ' ' || ptr[11] == '\t' ) ) {
			char	value[MAX_QPATH];
			int		i;

			ptr += 11;
			while ( *ptr && ( *ptr == ' ' || *ptr == '\t' ) ) {
				ptr++;
			}

			i = 0;
			while ( *ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != '\r' && i < sizeof( value ) - 1 ) {
				value[i++] = *ptr++;
			}
			value[i] = 0;

			if ( cur ) {
				Q_strncpyz( cur->modelPrefix, value, sizeof( cur->modelPrefix ) );
			}
		} else if ( !Q_stricmpn( ptr, "defaultModel", 12 ) && ( ptr[12] == ' ' || ptr[12] == '\t' ) ) {
			char	value[MAX_QPATH];
			int		i;

			ptr += 12;
			while ( *ptr && ( *ptr == ' ' || *ptr == '\t' ) ) {
				ptr++;
			}

			i = 0;
			while ( *ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != '\r' && i < sizeof( value ) - 1 ) {
				value[i++] = *ptr++;
			}
			value[i] = 0;

			if ( cur ) {
				Q_strncpyz( cur->defaultModel, value, sizeof( cur->defaultModel ) );
			}
		} else {
			// skip unrecognized lines
			while ( *ptr && *ptr != '\n' && *ptr != '\r' ) {
				ptr++;
			}
		}

		// skip to next line (handle both \n and \r\n)
		while ( *ptr && ( *ptr == '\n' || *ptr == '\r' ) ) {
			ptr++;
		}
	}

	// to debug the loaded attacksets with weapon defs
#if 0
{
	int	i, j;
	for ( i = 0; i < bfpAttacksets.numSets; i++ ) {
		const bfpAttacksetGroup_t	*g = &bfpAttacksets.sets[i];
		Com_Printf( "^6[%d] ^3attackset %d ^6(modelPrefix=^3'%s'^6, defaultModel=^3'%s'^6)^7\n",
			i, g->attacksetId, g->modelPrefix, g->defaultModel );
		for ( j = 0; j < BFP_NUM_WEAPONS; j++ ) {
			int	wn = g->attack[j];
			Com_Printf( "  ^2attack[%d] -> weaponNum ^3%d", j, wn );
	// to debug with weapon defs
#if 0
			if ( wn != 0 ) {
				bfpWeaponDef_t	*def = BG_FindBFPWeaponDef( wn );
				if ( def ) {
					Com_Printf( " ^6(^3%s^6)", def->attackName );
					// print all weapon properties
					Com_Printf( "\n      ^2attackType: ^3%d ^7%s", def->attackType,
						( def->attackType == ATK_MISSILE ) ? "missile" :
						( def->attackType == ATK_RDMISSILE ) ? "rdmissile" :
						( def->attackType == ATK_BEAM ) ? "beam" :
						( def->attackType == ATK_SBEAM ) ? "sbeam" :
						( def->attackType == ATK_HITSCAN ) ? "hitscan" :
						( def->attackType == ATK_FORCEFIELD ) ? "forcefield" : "UNKNOWN" );
					Com_Printf( "\n      ^2weaponTime: ^3%d  ^2randomWeaponTime: ^3%d", def->weaponTime, def->randomWeaponTime );
					Com_Printf( "\n      ^2kiCostAsPct: ^3%d  ^2kiPct: ^3%f  ^2kiCost: ^3%d", def->kiCostAsPct, def->kiPct, def->kiCost );
					Com_Printf( "\n      ^2chargeAttack: ^3%d  ^2chargeAutoFire: ^3%d  ^2minCharge: ^3%d  ^2maxCharge: ^3%d",
						def->chargeAttack, def->chargeAutoFire, def->minCharge, def->maxCharge );
					Com_Printf( "\n      ^2damage: ^3%d  ^2splashDamage: ^3%d  ^2chargeDamageMult: ^3%d  ^2maxDamage: ^3%d",
						def->damage, def->splashDamage, def->chargeDamageMult, def->maxDamage );
					Com_Printf( "\n      ^2radius: ^3%d  ^2explosionRadius: ^3%d  ^2chargeRadiusMult: ^3%d  ^2chargeExpRadiusMult: ^3%d",
						def->radius, def->explosionRadius, def->chargeRadiusMult, def->chargeExpRadiusMult );
					Com_Printf( "\n      ^2maxRadius: ^3%d  ^2maxExpRadius: ^3%d", def->maxRadius, def->maxExpRadius );
					Com_Printf( "\n      ^2missileSpeed: ^3%d  ^2homing: ^3%f  ^2homingRange: ^3%f  ^2homingAcceleration: ^3%f",
						def->missileSpeed, def->homing, def->homingRange, def->homingAcceleration );
					Com_Printf( "\n      ^2range: ^3%f", def->range );
					Com_Printf( "\n      ^2loopingAnim: ^3%d  ^2noAttackAnim: ^3%d", def->loopingAnim, def->noAttackAnim );
					Com_Printf( "\n      ^2alternatingXOffset: ^3%f  ^2randYOffset: ^3%f  ^2randXOffset: ^3%f",
						def->alternatingXOffset, def->randYOffset, def->randXOffset );
					Com_Printf( "\n      ^2coneOfFireX: ^3%d  ^2coneOfFireY: ^3%d", def->coneOfFireX, def->coneOfFireY );
					Com_Printf( "\n      ^2piercing: ^3%d  ^2reflective: ^3%d  ^2priority: ^3%d  ^2blinding: ^3%d",
						def->piercing, def->reflective, def->priority, def->blinding );
					Com_Printf( "\n      ^2missileGravity: ^3%d  ^2missileAcceleration: ^3%f  ^2missileDuration: ^3%d",
						def->missileGravity, def->missileAcceleration, def->missileDuration );
					Com_Printf( "\n      ^2multishot: ^3%d  ^2bounces: ^3%d  ^2bounceFriction: ^3%f  ^2noZBounce: ^3%d",
						def->multishot, def->bounces, def->bounceFriction, def->noZBounce );
					Com_Printf( "\n      ^2extraKnockback: ^3%d  ^2railTrail: ^3%d  ^2movementPenalty: ^3%d  ^2explosionSpawn: ^3%d",
						def->extraKnockback, def->railTrail, def->movementPenalty, def->explosionSpawn );
				} else {
					Com_Printf( " ^1(WARNING: weaponNum %d not found in weapon definitions!)", wn );
				}
			}
#endif
			Com_Printf( "\n" );
		}
	}
	Com_Printf( "^2LOADED BFP ATTACKSETS: ^3%d\n", bfpAttacksets.numSets );
}
#endif
}
