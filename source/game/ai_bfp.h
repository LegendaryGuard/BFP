/*
===========================================================================

BFP BOT AI

===========================================================================
*/

#define	BFP_BOT_KI_CRITICAL_PCT					0.10f
void BotBFPCombatAI( bot_state_t *bs );
void BotBFPBeginFrame( bot_state_t *bs );
void BotBFPApplyButtons( bot_state_t *bs, usercmd_t *ucmd );
void BotBFPUniversalCheck( bot_state_t *bs );
void BotBFPResetState( bot_state_t *bs );
