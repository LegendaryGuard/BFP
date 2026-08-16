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
// bg_public.h -- definitions shared by both the server game and client game modules

// because games can change separately from the main system version, we need a
// second version that must match between game and cgame

#define	GAME_VERSION		"bfp10" // BFP - Game version for BFP, before: baseq3-1

#define	DEFAULT_GRAVITY		800
#define	GIB_HEALTH			-40
#define	ARMOR_PROTECTION	0.66

#define	MAX_ITEMS			256

#define	RANK_TIED_FLAG		0x4000

#define DEFAULT_SHOTGUN_SPREAD	700
#define DEFAULT_SHOTGUN_COUNT	11

#define	ITEM_RADIUS			15		// item sizes are needed for client side pickup detection

#define	SCORE_NOT_PRESENT	-9999	// for the CS_SCORES[12] when only one player is present

#define	VOTE_TIME			30000	// 30 seconds before vote times out

#define	MINS_Z				-24
#define	DEFAULT_VIEWHEIGHT	26
#define CROUCH_VIEWHEIGHT	12
#define	DEAD_VIEWHEIGHT		-16

//
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.
//

// CS_SERVERINFO and CS_SYSTEMINFO are defined in q_shared.h
#define	CS_MUSIC				2
#define	CS_MESSAGE				3		// from the map worldspawn's message field
#define	CS_MOTD					4		// g_motd string for server message of the day
#define	CS_WARMUP				5		// server time when the match will be restarted
#define	CS_SCORES1				6
#define	CS_SCORES2				7
#define CS_VOTE_TIME			8
#define CS_VOTE_STRING			9
#define	CS_VOTE_YES				10
#define	CS_VOTE_NO				11

#define CS_TEAMVOTE_TIME		12
#define CS_TEAMVOTE_STRING		14
#define	CS_TEAMVOTE_YES			16
#define	CS_TEAMVOTE_NO			18

#define	CS_GAME_VERSION			20
#define	CS_LEVEL_START_TIME		21		// so the timer only shows the current level
#define	CS_INTERMISSION			22		// when 1, fraglimit/timelimit has been hit and intermission will start in a second or two
#define CS_FLAGSTATUS			23		// string indicating flag status in CTF
#define CS_SHADERSTATE			24
#define CS_BOTINFO				25

#define	CS_ITEMS				27		// string of 0's and 1's that tell which items are present

#define	CS_MODELS				32
#define	CS_SOUNDS				(CS_MODELS+MAX_MODELS)
#define	CS_PLAYERS				(CS_SOUNDS+MAX_SOUNDS)
#define CS_LOCATIONS			(CS_PLAYERS+MAX_CLIENTS)
#define CS_PARTICLES			(CS_LOCATIONS+MAX_LOCATIONS) 

#define CS_MAX					(CS_PARTICLES+MAX_LOCATIONS)

#if (CS_MAX) > MAX_CONFIGSTRINGS
#error overflow: (CS_MAX) > MAX_CONFIGSTRINGS
#endif

typedef enum {
	GT_FFA,				// free for all
	GT_TOURNAMENT,		// one on one tournament
	GT_SINGLE_PLAYER,	// single player ffa

	GT_SURVIVAL,		// BFP - Survival "survival":					g_gametype 3
	GT_MONSTER,			// BFP - Monster "monster" / "oozaru":			g_gametype 4

	//-- team games go after this --

	GT_TEAM,			// team deathmatch
	GT_TLMS,			// BFP - Team Last Man Standing "lms" / "tlms":	g_gametype 6
	GT_CTF,				// capture the flag
	GT_MAX_GAME_TYPE
} gametype_t;

typedef enum { GENDER_MALE, GENDER_FEMALE, GENDER_NEUTER } gender_t;

/*
===================================================================================

PMOVE MODULE

The pmove code takes a player_state_t and a usercmd_t and generates a new player_state_t
and some other output data.  Used for local prediction on the client game and true
movement on the server game.
===================================================================================
*/

typedef enum {
	PM_NORMAL,		// can accelerate and turn
	PM_NOCLIP,		// noclip movement
	PM_SPECTATOR,	// still run into walls
	PM_DEAD,		// no acceleration or turning, but free falling
	PM_FREEZE,		// stuck in place with no control
	PM_INTERMISSION,	// no movement or status bar
	PM_SPINTERMISSION	// no movement or status bar
} pmtype_t;

// BFP - Cannot be more than 16, weaponstate is a 4-bit integer size
typedef enum {
	WEAPON_READY, 
	WEAPON_RAISING,
	WEAPON_DROPPING,
	WEAPON_FIRING,

	// BFP - The following weapon states are to handle the movements 
	// (originally BFP didn't use that because of their abuse of WP_, PW_ and STAT_ stuff in their networking):
	WEAPON_ACTIVE, // BFP - Active fire
	WEAPON_BEAMSTRUGGLE, // BFP - Beam struggle
	WEAPON_STUN // BFP - Weapon stun status (not hit stun)
} weaponstate_t;

// BFP - Attack types
#define	ATK_MISSILE			0
#define	ATK_RDMISSILE		1
#define	ATK_BEAM			2
#define	ATK_SBEAM			3
#define	ATK_HITSCAN			4
#define	ATK_FORCEFIELD		5

// BFP - Charge limit
#define	ATTACK_CHARGE_LIMIT	6

// BFP - If you want to keep demo networking, change the way to use the PMF_ flags
// pmove->pm_flags
#define	PMF_DUCKED			1
#define	PMF_JUMP_HELD		2
#define	PMF_BLOCK			4		// BFP - Block
// BFP - PMF_BACKWARDS_JUMP is unused
//#define	PMF_BACKWARDS_JUMP		8		// go into backwards land
// BFP - PMF_BACKWARDS_RUN is renamed	// coast down to backwards run
#define	PMF_MELEE			16		// BFP - Melee
// BFP - PMF_TIME_LAND is unused
//#define PMF_TIME_LAND			32	// pm_time is time before rejump
#define	PMF_TIME_KNOCKBACK	64		// pm_time is an air-accelerate only time
#define PMF_KI_CHARGE		128		// BFP - Ki charge
// BFP - PMF_TIME_WATERJUMP is renamed	// pm_time is waterjump
#define	PMF_AIR_GRAVITY		256		// BFP - Air gravity check
#define	PMF_RESPAWNED		512		// clear after attack and jump buttons come up
#define	PMF_USE_ITEM_HELD	1024
// BFP - PMF_GRAPPLE_PULL is unused
//#define	PMF_GRAPPLE_PULL	2048	// pull towards grapple location
#define PMF_FOLLOW			4096	// spectate following another player
// BFP - PMF_SCOREBOARD is renamed	// spectate as a scoreboard
#define PMF_ULTIMATE_TIER	8192	// BFP - Ultimate tier status
#define	PMF_FLIGHT_LATCH	16384	// BFP - Flight latch toggling
// #define	PMF_UNUSED_FLAG		32768	// BFP - Some unused pm_flag
// BFP - Last pm_flag after 32768. That's the limit of pm_flags, it can't reach more
// #define PMF_SOMEFLAG		65536	// some pm_flag

// BFP - That combination of PMF_TIME_* flags is unused
#define	PMF_ALL_TIMES	PMF_TIME_KNOCKBACK //(PMF_TIME_WATERJUMP|PMF_TIME_LAND|PMF_TIME_KNOCKBACK)

#define	MAXTOUCH	32
typedef struct {
	// state (in / out)
	playerState_t	*ps;

	// command (in)
	usercmd_t	cmd;
	int			tracemask;			// collide against these types of surfaces
	int			debugLevel;			// if set, diagnostic output will be printed
	qboolean	noFootsteps;		// if the game is setup for no footsteps by the server
	qboolean	meleeHit;			// BFP - before: gauntletHit - true if a melee attack would actually hit something

	qboolean	noFlight;			// BFP - No flight
	qboolean	meleeOnly;			// BFP - Melee only

	int			framecount;

	// results (out)
	int			numtouch;
	int			touchents[MAXTOUCH];

	vec3_t		mins, maxs;			// bounding box size

	int			watertype;
	int			waterlevel;

	float		xyspeed;

	// for fixed msec Pmove
	int			pmove_fixed;
	int			pmove_msec;

	// callbacks to test the world
	// these will be different functions during game and cgame
	void		(*trace)( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask );
	int			(*pointcontents)( const vec3_t point, int passEntityNum );
} pmove_t;

// if a full pmove isn't done on the client, you can just update the angles
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd );
void Pmove (pmove_t *pmove);

//===================================================================================


// player_state->stats[] indexes
// NOTE: may not have more than 16
typedef enum {
	STAT_HEALTH,
	STAT_HOLDABLE_ITEM,
	STAT_WEAPONS,					// 16 bit fields
	STAT_ARMOR,				
	// BFP - Got rid of STAT_DEAD_YAW, now uses ps->damageYaw and ps->damagePitch
	STAT_UNUSED_INDEX4,				// unused stat index (don't remove if you want to keep demo networking!)
	STAT_CLIENTS_READY,				// bit mask of clients wishing to exit the intermission (FIXME: configstring?)

	STAT_UNUSED_INDEX6,				// BFP - Powerlevel
	STAT_UNUSED_INDEX7,				// BFP - Flight jump anim transition seconds, maximum is 21 sec and stops changing to 0, even when stop flying also reproduces this stat index like starting to fly (looks weird)
	STAT_KI,						// BFP - Ki
	STAT_MAX_KI,					// BFP - Maximum ki
	STAT_UNUSED_INDEX10,			// BFP - Melee attack time
	STAT_MAX_HEALTH,				// health / armor limit, changable by handicap
	STAT_UNUSED_INDEX12,			// unused stat index (don't remove if you want to keep demo networking!)
	STAT_UNUSED_INDEX13,			// BFP - Beam firing weapon state
	STAT_UNUSED_INDEX14,			// BFP - Force field weapon state
	//STAT_UNUSED_INDEX15			// BFP - Fly tilt angles (left: moves to -80, right: moves to 80)
	STAT_HITSTUN_TIME				// BFP - Hit stun time
} statIndex_t;

// player_state->persistant[] indexes
// these fields are the only part of player_state that isn't
// cleared on respawn
// NOTE: may not have more than 16
typedef enum {
	PERS_SCORE,						// !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!
	PERS_HITS,						// total points damage inflicted so damage beeps can sound on change
	PERS_RANK,						// player rank or team rank
	PERS_TEAM,						// player team
	PERS_SPAWN_COUNT,				// incremented every respawn
	PERS_PLAYEREVENTS,				// 16 bits that can be flipped for events
	PERS_ATTACKER,					// clientnum of last damage inflicter
	PERS_ATTACKEE_ARMOR,			// health/armor of last person we attacked
	PERS_KILLED,					// count of the number of times you died
	// player awards tracking
	PERS_EXCELLENT_COUNT,			// two successive kills in a short amount of time

	// BFP - NOTE: These indexes are being kept for original BFP networking
	// BFP - No impressive, gauntlet, defend, assist and capture (PERS_CAPTURES is renamed) counters are used. Renamed as PERS_UNUSED_INDEX00
	PERS_UNUSED_INDEX10,			//PERS_IMPRESSIVE_COUNT,			// two railgun hits in a row
	PERS_UNUSED_INDEX11,			//PERS_DEFEND_COUNT,				// defend awards
	PERS_UNUSED_INDEX12,			//PERS_ASSIST_COUNT,				// assist awards
	// BFP - This index is used in original BFP networking, but remains unknown
	PERS_UNUSED_INDEX13,			//PERS_GAUNTLET_FRAG_COUNT,		// kills with the guantlet

	PERS_POWERLEVEL,				// BFP - Powerlevel	(before Q3: //PERS_CAPTURES,					// captures)

	PERS_UNUSED_INDEX15				// BFP - ??? (Original BFP networking says it appears when spawning at the first time of all in-game)
} persEnum_t;

// BFP - If you want to keep demo networking, change the way to use the EF_ flags 
// entityState_t->eFlags
#define	EF_DEAD				0x00000001		// don't draw a foe marker over players with EF_DEAD
#define EF_AURA				0x00000002		// BFP - Aura, used to display players' aura
#define	EF_TELEPORT_BIT		0x00000004		// toggled every time the origin abruptly changes
#define	EF_AWARD_EXCELLENT	0x00000008		// draw an excellent sprite
#define EF_PLAYER_EVENT		0x00000010
// BFP - Unused EF_BOUNCE and EF_BOUNCE_HALF
//#define	EF_BOUNCE			0x00000010		// for missiles
// BFP - This eFlag hack is used for client/player only:
#define	EF_READY_KI_ATTACK	0x00000010		// BFP - Ready ki attack
//#define	EF_BOUNCE_HALF		0x00000020		// for missiles
// BFP - EF_AWARD_GAUNTLET flag is renamed	// draw a gauntlet sprite
#define	EF_FLIGHT			0x00000040		// BFP - Used for flying status
#define	EF_NODRAW			0x00000080		// may have an event, but no model (unspawned items)
#define	EF_FIRING			0x00000100		// for lightning gun
#define	EF_KI_BOOST			0x00000200		// BFP - Used for ki boost status
#define	EF_MOVER_STOP		0x00000400		// will push otherwise
// BFP - This eFlag hack is used for client/player only:
#define EF_AURA_TIER_UP		0x00000400		// BFP - Aura tier up effect when transforms or passes to the next tier
// BFP - EF_AWARD_CAP is unused
//#define	EF_AWARD_CAP		0x00000800		// draw the capture sprite
#define	EF_TALK				0x00001000		// draw a talk balloon
#define	EF_CONNECTION		0x00002000		// draw a connection trouble sprite
#define	EF_VOTED			0x00004000		// already cast a vote
// BFP - EF_AWARD_IMPRESSIVE flag is renamed	// draw an impressive sprite
#define	EF_MONSTER			0x00008000		// BFP - Player marked as monster on monster gametype (g_gametype 4)
// BFP - No EF_AWARD_DEFEND and EF_AWARD_ASSIST flags
// #define	EF_AWARD_DEFEND		0x00010000		// draw a defend sprite
// #define	EF_AWARD_ASSIST		0x00020000		// draw a assist sprite
// BFP - Unused EF flag
// #define EF_AWARD_DENIED		0x00040000		// denied
#define EF_TEAMVOTED		0x00080000		// already cast a team vote

// BFP - We can use generic1 integer 8-bit fields, but if we use as integer, only can use until 128 ~ 256
// used for ki charge points
// entityState_t->generic1
// #define	GENF_FLAG_1		1
// #define	GENF_FLAG_2		2
// #define	GENF_FLAG_3		4
// #define	GENF_FLAG_4		8
// #define	GENF_FLAG_5		16
// #define	GENF_FLAG_6		32
// #define	GENF_FLAG_7		64
// #define	GENF_FLAG_8		128

// NOTE: may not have more than 16
typedef enum {
	// BFP - NOTE: Modified according to keep original BFP networking
	// PW_REGEN doesn't appear and has been replaced by other feature
	// BFP - that's where PW_HASTE, PW_BATTLESUIT and PW_INVIS are marked as PW_NONE after picking up
	PW_NONE,

	PW_QUAD,

	// BFP - Red and blue flag are replaced from PW_BATTLESUIT and PW_HASTE according to original BFP networking
	PW_REDFLAG, //PW_BATTLESUIT,
	PW_BLUEFLAG, //PW_HASTE,

	PW_UNUSED_INDEX4,	// BFP - Unused powerup index //PW_INVIS
	PW_UNUSED_INDEX5,	// BFP - That would be PW_FLIGHT, used for flying
	PW_UNUSED_INDEX6,	// BFP - Used for ki recharge
	PW_UNUSED_INDEX7,	// BFP - Used for ki use/boost
	PW_UNUSED_INDEX8,	// BFP - Used for blocking seconds (defend from melee, beams, explosions and impacts, and reflect ki attack projectiles)
	PW_UNUSED_INDEX9,	// BFP - Used for melee toggle
	PW_UNUSED_INDEX10,	// BFP - Used for hit stun seconds
	PW_UNUSED_INDEX11,	// BFP - Used for ki attack charge points
	PW_UNUSED_INDEX12,	// BFP - Used for enable/disable monster in monster gamemode (g_gametype 4)
	PW_UNUSED_INDEX13,	// BFP - Used for beam firing state
	PW_UNUSED_INDEX14,	// BFP - Used for jump (looks strange...)

	PW_NUM_POWERUPS

} powerup_t;

typedef enum {
	HI_NONE,

	HI_TELEPORTER,
	HI_MEDKIT,

	HI_NUM_HOLDABLE
} holdable_t;


// BFP - NOTE: According to keep original BFP networking, these are modified for features
typedef enum {
	WP_ATTACK_0,			// BFP - First attack selected
	WP_ATTACK_1,			// BFP - Second attack selected
	WP_ATTACK_2,			// BFP - Third attack selected
	WP_ATTACK_3,			// BFP - Fourth attack selected
	WP_ATTACK_4,			// BFP - Fifth (or last) attack selected, in original BFP is also treated as a timer of 2000 msec when being attacked/damaged

	WP_UNUSED_INDEX5,		// BFP - Original demo networking: Ki recharge delay time (for g_chargeDelay)
	WP_UNUSED_INDEX6,		// BFP - Original demo networking: Hit stun delay after receiving hit stun
	WP_UNUSED_INDEX7,		// BFP - Original demo networking: Block delay
	WP_UNUSED_INDEX8,		// BFP - Original demo networking: Ki use/boost toggle
	WP_UNUSED_INDEX9,		// BFP - Original demo networking: Flight toggle key control
	WP_UNUSED_INDEX10,		// BFP - Original demo networking: Blind seconds

//	WP_UNUSED_INDEX11,		// BFP - Original demo networking: Rapid attacks like ki storm (alternates -1 and 1)
//	WP_UNUSED_INDEX12,		// BFP - Original demo networking: Unknown or unused index
//	WP_UNUSED_INDEX13,		// BFP - Original demo networking: Toggle to use Short-Range Teleport - Zanzoken
//	WP_UNUSED_INDEX14,		// BFP - Original demo networking: Directional left/right keys to move left/right while pressing, adds time msec, looks like a timer to handle for Zanzoken
//	WP_UNUSED_INDEX15,		// BFP - Original demo networking: Enables/disables beam struggle 

	WP_NUM_WEAPONS
} weapon_t;

// BFP - BFP number of weapon slots to be selected
#define	BFP_NUM_WEAPONS			( WP_ATTACK_4 + 1 )

// BFP - ps->ammo[WP_*] bit flags for ki attacks, used to handle pmove snapshots
// NOTE: may not have more than 16
#define	AMMOF_ACTIVE			1
#define	AMMOF_ATK_BEAM			2
#define	AMMOF_ATK_SBEAM			4
#define	AMMOF_ATK_FORCEFIELD	8
#define	AMMOF_CHARGEATTACK		16
#define	AMMOF_CHARGEAUTOFIRE	32
#define	AMMOF_LOOPINGANIM		64
#define	AMMOF_NOATTACKANIM		128
#define	AMMOF_MOVEMENTPENALTY	256

// BFP - BFP WEAPON CONFIG
/*
===================================================================================

BFP WEAPON CONFIG

===================================================================================
*/

#define	MAX_BFP_ATTACKSETS		3072
#define	MAX_BFP_WEAPON_DEFS		8192

typedef struct {
	qboolean	inuse;						// slot occupied by a parsed (attack_name) block
	char		attackName[MAX_QPATH];		// (attack_name) tag
	int			weaponNum;					// weaponNum

	int			attackType;					// attackType: ATK_MISSILE / ATK_RDMISSILE / ATK_BEAM / ATK_SBEAM / ATK_HITSCAN / ATK_FORCEFIELD

	int			weaponTime;					// weapon time
	int			randomWeaponTime;			// milliseconds of random extra fire delay

	qboolean	kiCostAsPct;				// enables ki cost percentage
	float		kiPct;						// ki cost percentage (0.0 - 1.0)
	int			kiCost;						// ki cost

	qboolean	chargeAttack;				// charge attack
	qboolean	chargeAutoFire;				// charge autofire - whether this projectile/effect's attack definition has chargeAutoFire set (e.g. forcefield's continuous think)
	int			minCharge;					// minimum charge points
	int			maxCharge;					// maximum charge points

	int			damage;						// attack damage
	int			splashDamage;				// attack splash damage
	int			chargeDamageMult;			// damage added per charge level
	int			maxDamage;					// damage cap when charged

	int			radius;						// collision radius
	int			chargeRadiusMult;			// collision radius added per charge level
	int			maxRadius;					// collision radius cap when charged
	int			explosionRadius;			// explosion radius (splashRadius)
	int			chargeExpRadiusMult;		// explosion radius added per charge level
	int			maxExpRadius;				// explosion cap radius when charged

	int			missileSpeed;				// missile speed
	float		homing;						// homing
	float		homingRange;				// homing range
	float		homingAcceleration;			// homing acceleration
	float		range;						// range (hitscan only)

	qboolean	loopingAnim;				// looping ki attack animation
	qboolean	noAttackAnim;				// no prepare ki attack animation

	float		alternatingXOffset;			// alternating X offset
	float		randYOffset;				// random Y offset
	float		randXOffset;				// random X offset
	int			coneOfFireX;				// cone of fire X
	int			coneOfFireY;				// cone of fire Y

	qboolean	piercing;					// pierces any solid entity, it can deal 4 hits
	qboolean	reflective;					// reflects projectiles (hitscan only)
	int			priority;					// projectile priority, higher priority can break the projectile with lower priority
	qboolean	blinding;					// blinds opponents during 6 seconds

	// BFP - NOTE: That weapon property is unused in original BFP, so it won't make any difference
	// just use missileGravity
	// qboolean	usesGravity;				// projectile uses gravity

	int			missileGravity;				// missile gravity
	float		missileAcceleration;		// missile acceleration
	int			missileDuration;			// missile lifetime duration

	int			multishot;					// multishot, number of shots for a weapon
	qboolean	bounces;					// projectile can bounce
	float		bounceFriction;				// adds bounce friction (only 'bounces' enabled)
	qboolean	noZBounce;					// no Z bounce (bounces at the same height from the ground, only 'bounces' enabled)

	int			extraKnockback;				// extra knockback
	qboolean	railTrail;					// rail trail (hitscan only)
	int			movementPenalty;			// seconds of movement penalty (forcefield only)

	int			explosionSpawn;				// explosion spawn (rdmissile only): weaponNum of the split projectile into on detonation
} bfpWeaponDef_t;

void			BG_LoadBFPWeaponConfig( void );
bfpWeaponDef_t	*BG_FindBFPWeaponDef( int weaponNum );
bfpWeaponDef_t	*BG_SetDefaultWeaponDef( void );
bfpWeaponDef_t	*BG_SetMonsterDefaultWeaponDef( void );

// BFP - bfp_attacksets.cfg: maps a player model prefix group to 5 weaponNum entries,
// one per attack slot (WP_ATTACK_0 ... WP_ATTACK_4)
typedef struct {
	qboolean	inuse;
	int			attacksetId;								// attackset [int]
	int			attack[BFP_NUM_WEAPONS];				// attack [slot] [weaponNum]
	char		modelPrefix[MAX_QPATH];						// modelPrefix [string]
	char		defaultModel[MAX_QPATH];					// defaultModel [string]
} bfpAttacksetGroup_t;

void			BG_LoadBFPAttacksetsConfig( void );
const char		*BG_FindAttacksetDefaultModel( const char *modelPrefix );
const char		*BG_FindAttacksetDefaultModelForModel( const char *modelName );
int				BG_GetWeaponNumForSlot( const char *modelName, int attackSlot );
qboolean		BG_ModelMatchesAnyAttacksetPrefix( const char *modelName );
void			BG_SetClientAttackWeaponNums( int clientNum, const char *modelName );
bfpWeaponDef_t	*BG_GetClientWeaponDefForSlot( int clientNum, int attackSlot );

// BFP - End of BFP WEAPON CONFIG


// reward sounds (stored in ps->persistant[PERS_PLAYEREVENTS])
#define	PLAYEREVENT_DENIEDREWARD		0x0001
#define	PLAYEREVENT_GAUNTLETREWARD		0x0002
#define PLAYEREVENT_HOLYSHIT			0x0004

// entityState_t->event values
// entity events are for effects that take place reletive
// to an existing entities origin.  Very network efficient.

// two bits at the top of the entityState->event field
// will be incremented with each change in the event so
// that an identical event started twice in a row can
// be distinguished.  And off the value with ~EV_EVENT_BITS
// to retrieve the actual event number
#define	EV_EVENT_BIT1		0x00000100
#define	EV_EVENT_BIT2		0x00000200
#define	EV_EVENT_BITS		(EV_EVENT_BIT1|EV_EVENT_BIT2)

#define	EVENT_VALID_MSEC	300

typedef enum {
	// BFP - Events are declared in bg_events.h file
#define EVENT_ENUMS
	#include "bg_events.h"
#undef EVENT_ENUMS

} entity_event_t;


typedef enum {
	GTS_RED_CAPTURE,
	GTS_BLUE_CAPTURE,
	GTS_RED_RETURN,
	GTS_BLUE_RETURN,
	GTS_RED_TAKEN,
	GTS_BLUE_TAKEN,
	GTS_REDTEAM_SCORED,
	GTS_BLUETEAM_SCORED,
	GTS_REDTEAM_TOOK_LEAD,
	GTS_BLUETEAM_TOOK_LEAD,
	GTS_TEAMS_ARE_TIED
} global_team_sound_t;

// animations
typedef enum {

	// BFP - The animations must be set in this order

	BOTH_DEATH1,			// BFP uses this animation (fall chest) (fall spinning)
	BOTH_DEAD1,				// BFP uses this animation
	BOTH_DEATH2,			// BFP uses this animation (fall back) (fall summersault)
	BOTH_DEAD2,				// BFP uses this animation
	BOTH_DEATH3,			// BFP uses this animation (fall back hard)
	BOTH_DEAD3,				// BFP uses this animation

	TORSO_GESTURE,			// BFP uses this animation
	
	TORSO_STAND,			// BFP uses this animation

	TORSO_RUN,				// BFP

	TORSO_BLOCK,			// BFP

	TORSO_STUN,				// BFP

	TORSO_FLYA,				// BFP
	TORSO_FLYB,				// BFP

	TORSO_CHARGE,			// BFP

	TORSO_MELEE_READY,		// BFP
	TORSO_MELEE,			// BFP
	TORSO_MELEE_STRIKE,		// BFP
	TORSO_MELEE_AXEHANDLE,	// BFP

	LEGS_WALKCR,			// BFP uses this animation
	LEGS_WALK,				// BFP uses this animation
	LEGS_RUN,				// BFP uses this animation
	LEGS_BACK,				// BFP uses this animation
	LEGS_SWIM,				// BFP uses this animation

	LEGS_JUMP,				// BFP uses this animation (1 leg up) (2 rolls forward)
	LEGS_JUMPB,				// BFP uses this animation (scissor) (2 rolls) (1 roll straight legs)

	LEGS_IDLE,				// BFP uses this animation
	LEGS_IDLECR,			// BFP uses this animation

	LEGS_TURN,				// BFP uses this animation

	LEGS_FLYIDLE,			// BFP
	LEGS_FLYA,				// BFP
	LEGS_FLYB,				// BFP

	LEGS_CHARGE,			// BFP

	LEGS_MELEE,				// BFP
	LEGS_MELEE_STRIKE,		// BFP


	// BFP - the following attack animations will be used for attacksets and new config stuff

	// BFP - (push right hand)
	TORSO_ATTACK0_PREPARE,	// BFP
	TORSO_ATTACK0_STRIKE,	// BFP

	// BFP - (throw right hand)
	TORSO_ATTACK1_PREPARE,	// BFP
	TORSO_ATTACK1_STRIKE,	// BFP

	// BFP - (both hands ki attack)
	TORSO_ATTACK2_PREPARE,	// BFP
	TORSO_ATTACK2_STRIKE,	// BFP

	// BFP - (eye laser)
	TORSO_ATTACK3_PREPARE,	// BFP
	TORSO_ATTACK3_STRIKE,	// BFP

	// BFP - (soul ball)
	TORSO_ATTACK4_PREPARE,	// BFP
	TORSO_ATTACK4_STRIKE,	// BFP

	// BFP - The following attackset animations are just reminders, these aren't used as variables
/*
	// BFP - (point finger right hand)
	TORSO_ATTACK5_PREPARE,	// BFP
	TORSO_ATTACK5_STRIKE,	// BFP

	// BFP - (angry expulsion)
	TORSO_ATTACK6_PREPARE,	// BFP
	TORSO_ATTACK6_STRIKE,	// BFP

	// BFP - (big disc hold)
	TORSO_ATTACK7_PREPARE,	// BFP
	TORSO_ATTACK7_STRIKE,	// BFP

	// BFP - (triangle attack)
	TORSO_ATTACK8_PREPARE,	// BFP
	TORSO_ATTACK8_STRIKE,	// BFP

	// BFP - (ending flash attack)
	TORSO_ATTACK9_PREPARE,	// BFP
	TORSO_ATTACK9_STRIKE,	// BFP

	// BFP - (2 hands forehead attack)
	TORSO_ATTACK10_PREPARE,	// BFP
	TORSO_ATTACK10_STRIKE,	// BFP

	// BFP - (big gnab attack)
	TORSO_ATTACK11_PREPARE,	// BFP
	TORSO_ATTACK11_STRIKE,	// BFP

	// BFP - (1 hand on forehead attack)
	TORSO_ATTACK12_PREPARE,	// BFP
	TORSO_ATTACK12_STRIKE,	// BFP

	// BFP - (charge 2 hands center)
	TORSO_ATTACK13_PREPARE,	// BFP
	TORSO_ATTACK13_STRIKE,	// BFP

	// BFP - (controlled sphere attack)
	TORSO_ATTACK14_PREPARE,	// BFP
	TORSO_ATTACK14_STRIKE,	// BFP

	// BFP - (hold sphere w/finger attack)
	TORSO_ATTACK15_PREPARE,	// BFP
	TORSO_ATTACK15_STRIKE,	// BFP

	// BFP - (ken and ryu fireball)
	TORSO_ATTACK16_PREPARE,	// BFP
	TORSO_ATTACK16_STRIKE,	// BFP
*/

	// BFP - The following animations are useless, 
	// possibly can be removed only if the game works as should
	TORSO_GETFLAG,
	// TORSO_GUARDBASE, 	// BFP - No longer used
	TORSO_PATROL,
	TORSO_FOLLOWME,
	TORSO_AFFIRMATIVE,
	TORSO_NEGATIVE,

	MAX_ANIMATIONS,			// BFP - important index, don't remove!

	// LEGS_BACKCR,			// BFP - No longer used
	// LEGS_BACKWALK,		// BFP - No longer used
	FLAG_RUN,
	FLAG_STAND,
	FLAG_STAND2RUN,

	MAX_TOTALANIMATIONS		// BFP - important index, don't remove!
} animNumber_t;

typedef struct animation_s {
	int		firstFrame;
	int		numFrames;
	int		loopFrames;			// 0 to numFrames
	int		frameLerp;			// msec between frames
	int		initialLerp;		// msec to get to first frame
	int		reversed;			// true if animation is reversed
	int		flipflop;			// true if animation should flipflop back to base
} animation_t;


// flip the togglebit every time an animation
// changes so a restart of the same anim can be detected
#define	ANIM_TOGGLEBIT		128


typedef enum {
	TEAM_FREE,
	TEAM_RED,
	TEAM_BLUE,
	TEAM_SPECTATOR,

	TEAM_NUM_TEAMS
} team_t;

// Time between location updates
#define TEAM_LOCATION_UPDATE_TIME		1000

// How many players on the overlay
#define TEAM_MAXOVERLAY		32

//team task
typedef enum {
	TEAMTASK_NONE,
	TEAMTASK_OFFENSE, 
	TEAMTASK_DEFENSE,
	TEAMTASK_PATROL,
	TEAMTASK_FOLLOW,
	TEAMTASK_RETRIEVE,
	TEAMTASK_ESCORT,
	TEAMTASK_CAMP
} teamtask_t;

// means of death
typedef enum {
	// BFP - Means of death are declared in bg_meansofdeath.h file
#define MOD_ENUMS
	#include "bg_meansofdeath.h"
#undef MOD_ENUMS
} meansOfDeath_t;


//---------------------------------------------------------

// gitem_t->type
typedef enum {
	IT_BAD,
	IT_WEAPON,				// EFX: rotate + upscale + minlight
	IT_AMMO,				// EFX: rotate
	IT_ARMOR,				// EFX: rotate + minlight
	IT_HEALTH,				// EFX: static external sphere + rotating internal
	IT_POWERUP,				// instant on, timer based
							// EFX: rotate + external ring that rotates
	IT_HOLDABLE,			// single use, holdable item
							// EFX: rotate + bob
	IT_PERSISTANT_POWERUP,
	IT_TEAM
} itemType_t;

#define MAX_ITEM_MODELS 4

typedef struct gitem_s {
	char		*classname;	// spawning name
	char		*pickup_sound;
	char		*world_model[MAX_ITEM_MODELS];

	char		*icon;
	char		*pickup_name;	// for printing on pickup

	int			quantity;		// for ammo how much, or duration of powerup
	itemType_t  giType;			// IT_* flags

	int			giTag;

	char		*precaches;		// string of all models and images this item will use
	char		*sounds;		// string of all sounds this item will use
} gitem_t;

// included in both the game dll and the client
extern	gitem_t	bg_itemlist[];
extern	int		bg_numItems;

gitem_t	*BG_FindItem( const char *pickupName );
gitem_t	*BG_FindItemForWeapon( weapon_t weapon );
gitem_t	*BG_FindItemForPowerup( powerup_t pw );
gitem_t	*BG_FindItemForHoldable( holdable_t pw );
#define	ITEM_INDEX(x) ((x)-bg_itemlist)

qboolean	BG_CanItemBeGrabbed( int gametype, const entityState_t *ent, const playerState_t *ps );


// g_dmflags->integer flags
#define	DF_NO_FALLING			8
#define DF_FIXED_FOV			16
#define	DF_NO_FOOTSTEPS			32

// content masks
#define	MASK_ALL				(-1)
#define	MASK_SOLID				(CONTENTS_SOLID)
#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY)
#define	MASK_DEADSOLID			(CONTENTS_SOLID|CONTENTS_PLAYERCLIP)
#define	MASK_WATER				(CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)
#define	MASK_OPAQUE				(CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA)
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE)


//
// entityState_t->eType
//
typedef enum {
	ET_GENERAL,
	ET_PLAYER,
	ET_ITEM,
	ET_MISSILE,
	ET_MOVER,
	ET_BEAM,
	ET_PORTAL,
	ET_SPEAKER,
	ET_PUSH_TRIGGER,
	ET_TELEPORT_TRIGGER,
	ET_INVISIBLE,
	// BFP - no hook
	// ET_GRAPPLE,				// grapple hooked on wall
	ET_TEAM,

	ET_EVENTS				// any of the EV_* events can be added freestanding
							// by setting eType to ET_EVENTS + eventNum
							// this avoids having to set eFlags and eventNum
} entityType_t;



void	BG_EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result );
void	BG_EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result );

void	BG_AddPredictableEventToPlayerstate( entity_event_t newEvent, int eventParm, playerState_t *ps, int entityNum );

void	BG_TouchJumpPad( playerState_t *ps, entityState_t *jumppad );

void	BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, qboolean snap );
void	BG_PlayerStateToEntityStateExtraPolate( playerState_t *ps, entityState_t *s, int time, qboolean snap );

qboolean	BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime );


#define ARENAS_PER_TIER		4
#define MAX_ARENAS			1024
#define	MAX_ARENAS_TEXT		8192

#define MAX_BOTS			1024
#define MAX_BOTS_TEXT		8192


// calculated by modulus.c for appropriate dividers:
#define TMOD_004	4272943
#define TMOD_075	2292106
#define TMOD_1000	5730265
#define TMOD_2000	5730265
