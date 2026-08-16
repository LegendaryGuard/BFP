/*
===========================================================================

BFP BOT AI

===========================================================================
*/


#include "g_local.h"
#include "botlib.h"
#include "be_aas.h"
#include "be_ea.h"
#include "be_ai_char.h"
#include "be_ai_chat.h"
#include "be_ai_gen.h"
#include "be_ai_goal.h"
#include "be_ai_move.h"
#include "be_ai_weap.h"
//
#include "ai_main.h"
#include "ai_dmq3.h"
#include "ai_chat.h"
#include "ai_cmd.h"
#include "ai_dmnet.h"
#include "ai_team.h"
#include "ai_bfp.h"
//
#include "chars.h"
#include "inv.h"
#include "syn.h"
#include "match.h"

// thresholds for bot behavior
#define	BFP_BOT_KI_LOW_PCT						0.057f
#define	BFP_BOT_KI_SAFE_PCT						0.705f
#define	BFP_BOT_KI_BOOST_MIN_PCT				0.05f
#define	BFP_BOT_FLIGHT_RANGE					700.0f
#define	BFP_BOT_MELEE_RANGE_PAD					10.0f
#define	BFP_BOT_WEAPON_SWITCH_MINTIME			1.0f
#define	BFP_BOT_WEAPON_SWITCH_MAXTIME			3.0f
#define	BFP_BOT_KI_CHARGE_DANGER_RANGE			100.0f
#define	BFP_BOT_MELEE_REPRESS_DELAY				0.25f
#define	BFP_BOT_MELEE_STANCE_HOLD				2.0f
#define	BFP_BOT_MELEE_STANCE_RANGE_BONUS		40.0f
#define	BFP_BOT_CHASE_TRIGGER_RANGE				1520.0f
#define	BFP_BOT_CHASE_DURATION					5.25f
#define	BFP_BOT_ZANZOKEN_PRESS_MS				120
#define	BFP_BOT_ZANZOKEN_RELEASE_MS				40
#define	BFP_BOT_ZANZOKEN_DANGER_RANGE			300.0f
#define	BFP_BOT_ZANZOKEN_HITSTUN_CHANCE			0.55f
#define	BFP_BOT_ZANZOKEN_HITSTUN_RETRY_MS		400			// how often, in ms, the bot re-rolls its zanzoken-escape chance while hitstun is active - lets it try more than once during a ~3s stun instead of a single roll at the start
#define	BFP_BOT_FIELD_OF_VIEW					180.0f		// very visible, even behind
#define	BFP_BOT_SIXTHSENSE_RANGE				16000.0f

static const float	bfpFlightChanceBySkill[6] = {
	0.26f,	// skill 0
	0.34f,	// skill 1
	0.42f,	// skill 2
	0.50f,	// skill 3
	0.61f,	// skill 4
	0.74f	// skill 5
};

/*
==================
BotBFPResetState
==================
*/
void BotBFPResetState( bot_state_t *bs ) {
	bs->bfpButtons = 0;
    bs->weaponnum = WP_ATTACK_0;
	bs->bfpKiRecharging = qfalse;
	bs->bfpKiRechargeInterrupted_time = 0;
	bs->bfpLastHealth = -1;
	bs->bfpLastMelee_time = 0;
	bs->bfpStrafeFlip_time = 0;
	bs->bfpFlightDecision = qfalse;
	bs->bfpFlightReroll_time = 0;
	bs->bfpZanzokenPhase = 0;
	bs->bfpZanzokenPhaseEnd_time = 0;
	bs->bfpZanzokenDir = 0;
	bs->bfpZanzokenHitstunRetry_time = 0;
	bs->bfpRightmoveOverride = 0;
	bs->bfpRightmoveOverrideActive = qfalse;
	bs->bfpForceAttackOff = qfalse;
	bs->bfpKiCompensate_time = 0;
	bs->bfpSixthSenseStep_time = 0;
	bs->bfpEvadeTime = 0;
	VectorClear( bs->bfpEvadeDir );
}

/*
==================
BotBFPApplyButtons
==================
*/
void BotBFPApplyButtons( bot_state_t *bs, usercmd_t *ucmd ) {
	ucmd->buttons |= bs->bfpButtons;
	if ( bs->bfpForceAttackOff ) {
		ucmd->buttons &= ~BUTTON_ATTACK;
	}
	if ( bs->bfpRightmoveOverrideActive ) {
		ucmd->rightmove = bs->bfpRightmoveOverride;
	}
}

/*
==================
BotBFPWantsToMelee
==================
*/
static qboolean BotBFPWantsToMelee( bot_state_t *bs, aas_entityinfo_t *entinfo ) {
	float	rangeMultiplier, distance;
	vec3_t	dir;

	if ( bs->cur_ps.stats[STAT_HITSTUN_TIME] > 0 ) {
		return qfalse;
	}

	VectorSubtract( entinfo->origin, bs->origin, dir );
	distance = VectorLength( dir );

	rangeMultiplier = g_meleeRange.integer + 45 - BFP_BOT_MELEE_RANGE_PAD;
	if ( bs->cur_ps.eFlags & EF_MONSTER ) {
		rangeMultiplier *= 2.5f;
	}

	if ( bs->bfpLastMelee_time > FloatTime() - BFP_BOT_MELEE_STANCE_HOLD ) {
		rangeMultiplier += BFP_BOT_MELEE_STANCE_RANGE_BONUS;
	}

	if ( distance > rangeMultiplier ) {
		return qfalse;
	}

	if ( !BotEntityVisible( bs->entitynum, bs->eye, bs->viewangles, BFP_BOT_FIELD_OF_VIEW, bs->enemy ) ) {
		return qfalse;
	}

	return qtrue;
}

/*
==================
BotBFPCheckMelee
==================
*/
static void BotBFPCheckMelee( bot_state_t *bs, aas_entityinfo_t *entinfo ) {
	if ( !BotBFPWantsToMelee( bs, entinfo ) ) {
		return;
	}
	if ( bs->bfpLastMelee_time > FloatTime() - BFP_BOT_MELEE_REPRESS_DELAY ) {
		return;
	}
	bs->bfpButtons |= BUTTON_MELEE;
	bs->bfpLastMelee_time = FloatTime();
}

/*
==================
BotBFPCompensateKiCharge
==================
*/
static void BotBFPCompensateKiCharge( bot_state_t *bs ) {
	gentity_t		*botent;
	playerState_t	*ps;
	int				maxKi;
	const float		HUMAN_CMD_INTERVAL = 0.017f;	// 17 ms
	float			now, elapsed, kiPerSecond, kiChargeTotal;

	botent = &g_entities[bs->client];
	if ( !botent->inuse || !botent->client ) {
		return;
	}
	ps = &botent->client->ps;
	now = FloatTime();

	if ( !( ( ps->pm_flags & PMF_KI_CHARGE ) && ( ps->eFlags & EF_AURA ) ) ) {
		bs->bfpKiCompensate_time = now;
		return;
	}

	if ( ps->stats[STAT_HITSTUN_TIME] > 0 ) {
		bs->bfpKiCompensate_time = now;
		return;
	}

	maxKi = ps->stats[STAT_MAX_KI];
	if ( ps->stats[STAT_KI] >= maxKi ) {
		bs->bfpKiCompensate_time = now;
		return;
	}

	if ( bs->bfpKiCompensate_time <= 0 ) {
		bs->bfpKiCompensate_time = now;
		return;
	}

	elapsed = now - bs->bfpKiCompensate_time;
	if ( elapsed <= 0 ) {
		return;
	}
	if ( elapsed > 2.0f ) {
		elapsed = 2.0f;
	}

	kiChargeTotal = ( g_kiCharge.value * 0.01f ) + g_kiChargePct.value * ( maxKi * 0.0001f );
	kiPerSecond = kiChargeTotal / HUMAN_CMD_INTERVAL;
	ps->stats[STAT_KI] += (int)( kiPerSecond * elapsed );
	if ( ps->stats[STAT_KI] > maxKi ) {
		ps->stats[STAT_KI] = maxKi;
	}

	bs->bfpKiCompensate_time = now;
}

/*
==================
BotBFPCheckKiRecharge
==================
*/
static void BotBFPCheckKiRecharge( bot_state_t *bs, aas_entityinfo_t *entinfo ) {
	int			maxKi, curKi, health;
	vec3_t		dir;
	float		enemyDist;
	qboolean	enemyDanger, justHit;

	maxKi = bs->cur_ps.stats[STAT_MAX_KI];
	curKi = bs->cur_ps.stats[STAT_KI];
	health = bs->inventory[INVENTORY_HEALTH];

	if ( maxKi <= 0 ) {
		return;
	}

	enemyDanger = qfalse;
	if ( entinfo != NULL ) {
		VectorSubtract( entinfo->origin, bs->origin, dir );
		enemyDist = VectorLength( dir );
		enemyDanger = ( enemyDist < BFP_BOT_KI_CHARGE_DANGER_RANGE );
	}

	if ( bs->bfpLastHealth < 0 ) {
		bs->bfpLastHealth = health;
	}
	justHit = ( health < bs->bfpLastHealth );
	bs->bfpLastHealth = health;

	if ( ( justHit || enemyDanger ) && bs->bfpKiRecharging ) {
		bs->bfpKiRecharging = qfalse;
		bs->bfpKiRechargeInterrupted_time = FloatTime();
		return;
	}

	if ( curKi < maxKi * BFP_BOT_KI_CRITICAL_PCT ) {
		bs->bfpKiRecharging = qtrue;
		bs->bfpButtons |= BUTTON_KI_CHARGE;
		return;
	}

	if ( bs->bfpKiRechargeInterrupted_time > FloatTime() - 1.5 ) {
		return;
	}

	if ( enemyDanger ) {
		return;
	}

	if ( !bs->bfpKiRecharging ) {
		if ( curKi < maxKi * BFP_BOT_KI_LOW_PCT ) {
			bs->bfpKiRecharging = qtrue;
		}
	} else {
		if ( curKi >= maxKi * BFP_BOT_KI_SAFE_PCT ) {
			bs->bfpKiRecharging = qfalse;
		}
	}

	if ( bs->bfpKiRecharging ) {
		bs->bfpButtons |= BUTTON_KI_CHARGE;
	}
}

/*
==================
BotBFPEnemyAirborne
==================
*/
static qboolean BotBFPEnemyAirborne( aas_entityinfo_t *entinfo ) {
	bsp_trace_t	trace;
	vec3_t		end;

	if ( !entinfo ) {
		return qfalse;
	}

	VectorCopy( entinfo->origin, end );
	end[2] -= 48;
	BotAI_Trace( &trace, entinfo->origin, entinfo->mins, entinfo->maxs, end, -1, MASK_SOLID );
	return ( trace.fraction >= 1.0 );
}

/*
==================
BotBFPCheckFlight
==================
*/
static void BotBFPCheckFlight( bot_state_t *bs, aas_entityinfo_t *entinfo, bot_goal_t *goal ) {
	int			skillIndex;
	float		flightChance, dist, stopChance;
	vec3_t		dir;
	qboolean	wantsFlight, alreadyFlying;

	if ( bs->cur_ps.stats[STAT_KI] <= 0 ) {
		bs->bfpFlightDecision = qfalse;
		return;
	}

	if ( bs->cur_ps.stats[STAT_KI] < bs->cur_ps.stats[STAT_MAX_KI] * BFP_BOT_KI_CRITICAL_PCT ) {
		return;
	}

	if ( bs->cur_ps.pm_flags & PMF_KI_CHARGE ) {
		return;
	}

	skillIndex = (int)bs->settings.skill;
	if ( skillIndex < 0 ) {
		skillIndex = 0;
	}
	if ( skillIndex > 5 ) {
		skillIndex = 5;
	}
	flightChance = bfpFlightChanceBySkill[skillIndex];

	alreadyFlying = ( bs->cur_ps.eFlags & EF_FLIGHT );

	VectorSubtract( entinfo->origin, bs->origin, dir );
	dist = VectorLength( dir );

	wantsFlight = qfalse;
	if ( entinfo != NULL ) { // combat
		VectorSubtract( entinfo->origin, bs->origin, dir );
		dist = VectorLength( dir );
		if ( BotBFPEnemyAirborne( entinfo ) ) {
			wantsFlight = qtrue;
		} else if ( dist > BFP_BOT_FLIGHT_RANGE ) {
			wantsFlight = qtrue;
		}
	} else if ( goal != NULL ) { // CTF
		VectorSubtract( goal->origin, bs->origin, dir );
		dist = VectorLength( dir );
		if ( dist > BFP_BOT_FLIGHT_RANGE ) {
			wantsFlight = qtrue;
		}
	} else {
		return;
	}

	if ( bs->bfpFlightReroll_time < FloatTime() ) {
		float	windowMin = 1.0f + ( skillIndex / 5.0f ) * 1.0f;
		float	windowMax = 2.0f + ( skillIndex / 5.0f ) * 2.0f;
		float	window = windowMin + random() * ( windowMax - windowMin );

		if ( !alreadyFlying ) {
			bs->bfpFlightDecision = ( wantsFlight && random() < flightChance );
		} else {
			int	maxKi = bs->cur_ps.stats[STAT_MAX_KI];
			int	curKi = bs->cur_ps.stats[STAT_KI];

			stopChance = 0.6f - ( (float)skillIndex / 5.0f ) * 0.55f;
			if ( stopChance < 0.05f ) {
				stopChance = 0.05f;
			}
			if ( stopChance > 0.6f ) {
				stopChance = 0.6f;
			}
			if ( maxKi > 0 && curKi < maxKi * 0.2f ) {
				stopChance += 0.2f;
				if ( stopChance > 0.8f ) {
					stopChance = 0.8f;
				}
			}

			if ( wantsFlight ) {
				bs->bfpFlightDecision = ( random() > stopChance * 0.2f );
			} else {
				bs->bfpFlightDecision = ( random() > stopChance );
			}
		}
		bs->bfpFlightReroll_time = FloatTime() + window;
	}

	if ( bs->bfpFlightDecision != bs->bfpLastFlightDecision ) {
		bs->bfpButtons |= BUTTON_ENABLEFLIGHT;
		bs->bfpLastFlightDecision = bs->bfpFlightDecision;
	}
}

/*
==================
BotBFPCheckKiBoost
==================
*/
static void BotBFPCheckKiBoost( bot_state_t *bs ) {
	int		maxKi = bs->cur_ps.stats[STAT_MAX_KI];
	int		curKi = bs->cur_ps.stats[STAT_KI];
	float	skill = bs->settings.skill;

	if ( maxKi <= 0 || curKi <= 0 ) {
		return;
	}

	if ( bs->cur_ps.stats[STAT_KI] < bs->cur_ps.stats[STAT_MAX_KI] * BFP_BOT_KI_CRITICAL_PCT ) {
		return;
	}

	if ( bs->bfpKiRecharging ) {
		return;
	}

	if ( curKi < maxKi * BFP_BOT_KI_BOOST_MIN_PCT ) {
		return;
	}

	if ( bs->cur_ps.stats[STAT_HITSTUN_TIME] > 0 ) {
		return;
	}

	if ( bs->cur_ps.weaponstate == WEAPON_BEAMSTRUGGLE ) {
		if ( random() < ( 0.3f + 0.06f * skill ) ) {
			bs->bfpButtons |= BUTTON_KI_USE;
		}
		return;
	}

	if ( random() < ( 0.7f + 0.06f * skill ) ) {
		bs->bfpButtons |= BUTTON_KI_USE;
	}
}

/*
==================
BotBFPCheckWeaponSlot
==================
*/
static void BotBFPCheckWeaponSlot( bot_state_t *bs ) {
	int	i, pick;
	int	weaponBits = bs->cur_ps.stats[STAT_WEAPONS];

	if ( bs->cur_ps.weaponstate == WEAPON_RAISING
	|| bs->cur_ps.weaponstate == WEAPON_DROPPING ) {
		return;
	}

	if ( bs->weaponnum < 0 || bs->weaponnum >= BFP_NUM_WEAPONS ) {
		bs->weaponnum = 0;
	}

	if ( !( weaponBits & ( 1 << bs->weaponnum ) ) ) {
		for ( i = 0; i < BFP_NUM_WEAPONS; i++ ) {
			if ( weaponBits & ( 1 << i ) ) {
				bs->weaponnum = i;
				trap_EA_SelectWeapon( bs->client, i );
				return;
			}
		}
		return;
	}

	if ( bs->cur_ps.weaponstate == WEAPON_FIRING
	|| bs->cur_ps.weaponstate == WEAPON_ACTIVE
	|| bs->cur_ps.weaponstate == WEAPON_BEAMSTRUGGLE ) {
		return;
	}

	if ( bs->weaponchange_time > FloatTime() - BFP_BOT_WEAPON_SWITCH_MINTIME ) {
		return;
	}
	if ( bs->weaponchange_time > FloatTime() - BFP_BOT_WEAPON_SWITCH_MAXTIME &&
			random() > 0.65f ) {
		return;
	}

	pick = bs->weaponnum;
	for ( i = 0; i < BFP_NUM_WEAPONS; i++ ) {
		pick = ( pick + 1 ) % BFP_NUM_WEAPONS;
		if ( weaponBits & ( 1 << pick ) ) {
			break;
		}
	}

	if ( pick != bs->weaponnum ) {
		bs->weaponnum = pick;
		bs->weaponchange_time = FloatTime();
		trap_EA_SelectWeapon( bs->client, pick );
	}
}

/*
==================
BotBFPCheckChargedAttack
==================
*/
static void BotBFPCheckChargedAttack( bot_state_t *bs, aas_entityinfo_t *entinfo ) {
	bfpWeaponDef_t	*def;
	qboolean		lineOfFireClear;
	bsp_trace_t		trace;

	bs->bfpForceAttackOff = qfalse;

	def = BG_GetClientWeaponDefForSlot( bs->client, bs->weaponnum );
	if ( !def ) {
		def = BG_SetDefaultWeaponDef();
	}

	if ( ( bs->cur_ps.eFlags & EF_MONSTER ) && g_monster.integer > 0 ) {
		def = BG_SetMonsterDefaultWeaponDef();
	}

	if ( !def ) {
		return;
	}

	// chargeAutoFire
	if ( def->chargeAutoFire ) {
		if ( bs->cur_ps.weaponstate == WEAPON_FIRING || bs->cur_ps.weaponstate == WEAPON_ACTIVE ) {
			if ( bs->bfpChargeAutoFireStartTime == 0 ) {
				bs->bfpChargeAutoFireStartTime = FloatTime();
			}
			if ( FloatTime() - bs->bfpChargeAutoFireStartTime < 4.0f ) {
				bs->bfpButtons |= BUTTON_ATTACK;
			} else {
				bs->bfpButtons &= ~BUTTON_ATTACK;
				bs->bfpForceAttackOff = qtrue;
				bs->bfpChargeAutoFireStartTime = 0;
			}
		} else {
			bs->bfpChargeAutoFireStartTime = 0;
		}
	}

	// chargeAttack
	if ( bs->cur_ps.weaponstate == WEAPON_FIRING && def->chargeAttack ) {
		if ( bs->bfpButtons & BUTTON_KI_CHARGE ) {
			return;
		}
		bs->bfpButtons |= BUTTON_ATTACK;

		if ( bs->cur_ps.eFlags & EF_READY_KI_ATTACK ) {
			BotAI_Trace( &trace, bs->eye, NULL, NULL, bs->aimtarget, bs->client,
					CONTENTS_SOLID | CONTENTS_PLAYERCLIP );
			lineOfFireClear = ( trace.fraction >= 1.0f || trace.ent == bs->enemy );
			if ( lineOfFireClear ) {
				bs->bfpForceAttackOff = qtrue;
			}
		}
		return;
	}

	// sbeam
	if ( def->attackType == ATK_SBEAM ) {
		if ( bs->cur_ps.weaponstate == WEAPON_READY || bs->cur_ps.weaponstate == WEAPON_ACTIVE ) {
			bs->bfpButtons |= BUTTON_ATTACK;
			bs->bfpForceAttackOff = qfalse;
		}
		if ( bs->cur_ps.weaponstate == WEAPON_READY && bs->bfpLastWeaponState == WEAPON_ACTIVE ) {
			bs->bfpForceAttackOff = qfalse;
			bs->bfpButtons |= BUTTON_ATTACK;
		}
		bs->bfpLastWeaponState = bs->cur_ps.weaponstate;
	}

	// beam
	if ( bs->cur_ps.weaponstate == WEAPON_ACTIVE && def->attackType == ATK_BEAM ) {
		bs->bfpForceAttackOff = qtrue;
		return;
	}
	if ( bs->cur_ps.weaponstate == WEAPON_BEAMSTRUGGLE && def->attackType == ATK_BEAM ) {
		bs->bfpForceAttackOff = qtrue;
		return;
	}
}

/*
==================
BotBFPCheckChase
==================
*/
static void BotBFPCheckChase( bot_state_t *bs, aas_entityinfo_t *entinfo, qboolean forceMelee ) {
	vec3_t			dir, forward, up = { 0, 0, 1 }, sideward;
	float			dist;
	qboolean		visible;
	const float		BFP_BOT_MELEE_STRAFE_RANGE = 220;

	// evasion
	if ( bs->bfpEvadeTime > FloatTime() ) {
		if ( VectorLength( bs->bfpEvadeDir ) > 0.1f ) {
			trap_BotMoveInDirection( bs->ms, bs->bfpEvadeDir, 400, MOVE_WALK );
		}
		return;
	}

	VectorSubtract( entinfo->origin, bs->origin, dir );
	dist = VectorLength( dir );

	visible = BotEntityVisible( bs->entitynum, bs->eye, bs->viewangles, BFP_BOT_FIELD_OF_VIEW, bs->enemy );
	
	if ( visible && dist <= BFP_BOT_MELEE_STRAFE_RANGE ) {
		bs->bfpButtons |= BUTTON_MELEE;
	}

	if ( forceMelee ) {
		if ( visible ) {
			bs->attackchase_time = FloatTime() + 9999.0f;
			dir[2] = 0;
			if ( VectorNormalize( dir ) > 0.1f ) {
				trap_BotMoveInDirection( bs->ms, dir, 400, MOVE_WALK );
			}
		}
	}

	if ( !forceMelee && visible && dist > BFP_BOT_CHASE_TRIGGER_RANGE ) {
		bs->attackchase_time = FloatTime() + BFP_BOT_CHASE_DURATION;
		return;
	}

	if ( visible && dist < BFP_BOT_MELEE_STRAFE_RANGE && bs->attackchase_time <= FloatTime() ) {
		VectorCopy( dir, forward );
		VectorNormalize( forward );
		CrossProduct( forward, up, sideward );
		if ( bs->flags & BFL_STRAFERIGHT ) {
			VectorNegate( sideward, sideward );
		}
		trap_BotMoveInDirection( bs->ms, sideward, 400, MOVE_WALK );
		if ( bs->bfpStrafeFlip_time < FloatTime() ) {
			bs->flags ^= BFL_STRAFERIGHT;
			bs->bfpStrafeFlip_time = FloatTime() + 0.5f + random() * 0.4f;
		}
	}
}

/*
==================
BotBFPCheckZanzoken
==================
*/
static void BotBFPCheckZanzoken( bot_state_t *bs, aas_entityinfo_t *entinfo ) {
	float		dist;
	vec3_t		dir;
	qboolean	wantsZanzoken;

	if ( bs->bfpZanzokenPhase != 0 ) {
		bs->bfpRightmoveOverrideActive = qtrue;
		switch ( bs->bfpZanzokenPhase ) {
		case 1:
			bs->bfpRightmoveOverride = ( bs->bfpZanzokenDir > 0 ) ? 127 : -127;
			if ( FloatTime() >= bs->bfpZanzokenPhaseEnd_time ) {
				bs->bfpZanzokenPhase = 2;
				bs->bfpZanzokenPhaseEnd_time = FloatTime() + BFP_BOT_ZANZOKEN_RELEASE_MS * 0.001;
			}
			break;
		case 2:
			bs->bfpRightmoveOverride = 0;
			if ( FloatTime() >= bs->bfpZanzokenPhaseEnd_time ) {
				bs->bfpZanzokenPhase = 3;
				bs->bfpZanzokenPhaseEnd_time = FloatTime() + BFP_BOT_ZANZOKEN_PRESS_MS * 0.001;
			}
			break;
		case 3:
			bs->bfpRightmoveOverride = ( bs->bfpZanzokenDir > 0 ) ? 127 : -127;
			if ( FloatTime() >= bs->bfpZanzokenPhaseEnd_time ) {
				bs->bfpZanzokenPhase = 0;
				bs->bfpRightmoveOverrideActive = qfalse;
			}
			break;
		}
		return;
	}

	wantsZanzoken = qfalse;

	if ( bs->cur_ps.stats[STAT_HITSTUN_TIME] > 0
	&& bs->cur_ps.stats[STAT_HITSTUN_TIME] <= 3000 ) {
		if ( bs->bfpZanzokenHitstunRetry_time <= FloatTime() ) {
			bs->bfpZanzokenHitstunRetry_time = FloatTime() + BFP_BOT_ZANZOKEN_HITSTUN_RETRY_MS * 0.001;
			if ( random() < BFP_BOT_ZANZOKEN_HITSTUN_CHANCE ) {
				wantsZanzoken = qtrue;
			}
		}
	}

	if ( bs->cur_ps.stats[STAT_HITSTUN_TIME] <= 0 ) {
		bs->bfpZanzokenHitstunRetry_time = 0;
	}

	if ( !wantsZanzoken ) {
		if ( bs->cur_ps.weaponstate == WEAPON_ACTIVE
		|| bs->cur_ps.weaponstate == WEAPON_BEAMSTRUGGLE
		|| bs->cur_ps.weaponstate == WEAPON_STUN ) {
			return;
		}
		if ( bs->cur_ps.pm_flags & ( PMF_KI_CHARGE | PMF_ULTIMATE_TIER ) ) {
			return;
		}
		if ( bs->cur_ps.stats[STAT_KI] <= ( bs->cur_ps.stats[STAT_MAX_KI] * 0.05f ) ) {
			return;
		}

		if ( entinfo != NULL ) {
			VectorSubtract( entinfo->origin, bs->origin, dir );
			dist = VectorLength( dir );
			if ( dist < BFP_BOT_ZANZOKEN_DANGER_RANGE
			&& BotEntityVisible( bs->entitynum, bs->eye, bs->viewangles, BFP_BOT_FIELD_OF_VIEW, bs->enemy ) ) {
				if ( random() < 0.09f ) {
					wantsZanzoken = qtrue;
				}
			}
		}
	}

	if ( wantsZanzoken ) {
		bs->bfpZanzokenDir = ( random() < 0.5f ) ? -1 : 1;
		bs->bfpZanzokenPhase = 1;
		bs->bfpZanzokenPhaseEnd_time = FloatTime() + BFP_BOT_ZANZOKEN_PRESS_MS * 0.001;
		bs->bfpRightmoveOverrideActive = qtrue;
		bs->bfpRightmoveOverride = ( bs->bfpZanzokenDir > 0 ) ? 127 : -127;
	}
}

/*
==================
BotBFPBeginFrame
==================
*/
void BotBFPBeginFrame( bot_state_t *bs ) {
	bs->bfpButtons = 0;
	bs->bfpForceAttackOff = qfalse;
}

/*
==================
BotBFPCheckSixthSense
==================
*/
static void BotBFPCheckSixthSense( bot_state_t *bs ) {
	int					i;
	vec3_t				dir, angles;
	float				squaredist;
	aas_entityinfo_t	entinfo;

	for ( i = 0; i < level.maxclients; i++ ) {
		if ( i == bs->entitynum ) {
			continue;
		}
		if ( BotSameTeam( bs, i ) ) {
			continue;
		}

		BotEntityInfo( i, &entinfo );
		if ( !entinfo.valid ) {
			continue;
		}
		if ( EntityIsDead( &entinfo ) ) {
			continue;
		}

		VectorSubtract( entinfo.origin, bs->origin, dir );
		squaredist = VectorLengthSquared( dir );
		if ( squaredist > Square( BFP_BOT_SIXTHSENSE_RANGE ) ) {
			continue;
		}

		vectoangles( dir, angles );
		if ( InFieldOfVision( bs->viewangles, BFP_BOT_FIELD_OF_VIEW, angles ) ) {
			continue;
		}

		VectorCopy( angles, bs->ideal_viewangles );
		bs->flags |= BFL_IDEALVIEWSET;

		if ( bs->bfpSixthSenseStep_time < FloatTime() ) {
			vec3_t	away;
			VectorNormalize( dir );
			VectorNegate( dir, away );
			away[2] = 0;
			if ( VectorNormalize( away ) > 0.1f ) {
				trap_BotMoveInDirection( bs->ms, away, 300, MOVE_WALK );
			}
			bs->bfpSixthSenseStep_time = FloatTime() + 0.4f;
		}

		return;
	}
}

/*
==================
BotBFPUniversalCheck
==================
*/
void BotBFPUniversalCheck( bot_state_t *bs ) {
	if ( bs->enemy < 0 ) {
		if ( gametype == GT_CTF ) {
			bot_goal_t	goal;
			if ( trap_BotGetTopGoal( bs->gs, &goal ) ) {
				BotBFPCheckFlight( bs, NULL, &goal );
				BotBFPCheckKiBoost( bs );
			}
		}
		BotBFPCheckZanzoken( bs, NULL );
		BotBFPCheckKiRecharge( bs, NULL );
		BotBFPCompensateKiCharge( bs );
		BotBFPCheckSixthSense( bs );
	}
}

/*
==================
BotBFPCombatAI
==================
*/
void BotBFPCombatAI( bot_state_t *bs ) {
	aas_entityinfo_t	entinfo;
	int					currentHealth = bs->inventory[INVENTORY_HEALTH];

	if ( bs->enemy < 0 ) {
		bs->bfpKiRecharging = qfalse;
		bs->bfpFlightDecision = qfalse;
		return;
	}

	BotEntityInfo( bs->enemy, &entinfo );

	BotBFPCheckMelee( bs, &entinfo );
	BotBFPCheckKiRecharge( bs, &entinfo );
	BotBFPCompensateKiCharge( bs );
	BotBFPCheckFlight( bs, &entinfo, NULL );
	BotBFPCheckKiBoost( bs );

	// use evasion after receiving damage
	if ( bs->bfpLastHealth < 0 ) {
		bs->bfpLastHealth = currentHealth;
	}
	if ( currentHealth < bs->bfpLastHealth && bs->bfpEvadeTime < FloatTime() ) {
		vec3_t	right, up = { 0, 0, 1 };
		vec3_t	dirToEnemy;
		VectorSubtract( entinfo.origin, bs->origin, dirToEnemy );
		VectorNormalize( dirToEnemy );
		CrossProduct( dirToEnemy, up, right );
		VectorNormalize( right );
		if ( random() < 0.5f ) {
			VectorScale( right, ( random() < 0.5f ) ? 1 : -1, bs->bfpEvadeDir );
		} else {
			VectorScale( dirToEnemy, -1, bs->bfpEvadeDir );
		}
		bs->bfpEvadeDir[2] += ( random() - 0.5f ) * 0.2f;
		VectorNormalize( bs->bfpEvadeDir );
		bs->bfpEvadeTime = FloatTime() + 0.5f + random() * 0.3f;
	}
	bs->bfpLastHealth = currentHealth;

	// don't use weapons on melee only
	if ( g_meleeOnly.integer <= 0 ) {
		BotBFPCheckWeaponSlot( bs );
		BotBFPCheckChargedAttack( bs, &entinfo );
	}

	BotBFPCheckChase( bs, &entinfo, ( g_meleeOnly.integer > 0 ) );
	BotBFPCheckZanzoken( bs, &entinfo );
}
