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

// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"

// BFPR - Max length of a ban reason string stored in a ban-list entry
#define MAX_ADMIN_BAN_REASON_LEN	64

/*
==================
G_RealTimeSeconds

Wall-clock time in seconds since epoch, via trap_RealTime. 
Used for ban expirations
==================
*/
static int G_RealTimeSeconds( void ) { // BFPR - Wall-clock time for ban expirations
	qtime_t	qt;
	return trap_RealTime( &qt );
}


/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and you can use '*' to match any value
so you can specify an entire class C network with "addip 192.246.40.*"

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

g_filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.

TTimo NOTE: for persistence, bans are stored in g_banIPs cvar MAX_CVAR_VALUE_STRING
The size of the cvar string buffer is limiting the banning to around 20 masks
this could be improved by putting some g_banIPs2 g_banIps3 etc. maybe
still, you should rely on PB for banning instead

==============================================================================
*/

typedef struct ipFilter_s
{
	unsigned	mask;
	unsigned	compare;
} ipFilter_t;

#define	MAX_IPFILTERS	1024

static ipFilter_t	ipFilters[MAX_IPFILTERS];
static int			numIPFilters;

/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter (char *s, ipFilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];
	
	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}
	
	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			if (*s == '*') // 'match any'
			{
				// b[i] and m[i] to 0
				s++;
				if (!*s)
					break;
				s++;
				continue;
			}
			G_Printf( "Bad filter address: %s\n", s );
			return qfalse;
		}
		
		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		m[i] = 255;

		if (!*s)
			break;
		s++;
	}
	
	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;
	
	return qtrue;
}

/*
=================
UpdateIPBans
=================
*/
static void UpdateIPBans (void)
{
	byte	b[4];
	byte	m[4];
	int		i,j;
	char	iplist_final[MAX_CVAR_VALUE_STRING];
	char	ip[64];

	*iplist_final = 0;
	for (i = 0 ; i < numIPFilters ; i++)
	{
		if (ipFilters[i].compare == 0xffffffff)
			continue;

		*(unsigned *)b = ipFilters[i].compare;
		*(unsigned *)m = ipFilters[i].mask;
		*ip = 0;
		for (j = 0 ; j < 4 ; j++)
		{
			if (m[j]!=255)
				Q_strcat(ip, sizeof(ip), "*");
			else
				Q_strcat(ip, sizeof(ip), va("%i", b[j]));
			Q_strcat(ip, sizeof(ip), (j<3) ? "." : " ");
		}		
		if (strlen(iplist_final)+strlen(ip) < MAX_CVAR_VALUE_STRING)
		{
			Q_strcat( iplist_final, sizeof(iplist_final), ip);
		}
		else
		{
			Com_Printf("g_banIPs overflowed at MAX_CVAR_VALUE_STRING\n");
			break;
		}
	}

	trap_Cvar_Set( "g_banIPs", iplist_final );
}

/*
=================
G_FilterPacket
=================
*/
qboolean G_FilterPacket (char *from)
{
	int		i;
	unsigned	in;
	byte m[4] = { 0 };
	char *p;

	i = 0;
	p = from;
	while (*p && i < 4) {
		m[i] = 0;
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}
	
	in = *(unsigned *)m;

	for (i=0 ; i<numIPFilters ; i++)
		if ( (in & ipFilters[i].mask) == ipFilters[i].compare)
			return g_filterBan.integer != 0;

	return g_filterBan.integer == 0;
}

/*
=================
AddIP
=================
*/
static void AddIP( char *str )
{
	int		i;

	for (i = 0 ; i < numIPFilters ; i++)
		if (ipFilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numIPFilters)
	{
		if (numIPFilters == MAX_IPFILTERS)
		{
			G_Printf ("IP filter list is full\n");
			return;
		}
		numIPFilters++;
	}
	
	if (!StringToFilter (str, &ipFilters[i]))
		ipFilters[i].compare = 0xffffffffu;

	UpdateIPBans();
}

/*
=================
G_ProcessIPBans
=================
*/
void G_ProcessIPBans(void) 
{
	char *s, *t;
	char		str[MAX_CVAR_VALUE_STRING];

	Q_strncpyz( str, g_banIPs.string, sizeof(str) );

	for (t = s = g_banIPs.string; *t; /* */ ) {
		s = strchr(s, ' ');
		if (!s)
			break;
		while (*s == ' ')
			*s++ = 0;
		if (*t)
			AddIP( t );
		t = s;
	}
}


/*
==============================================================================

BFPR BAN FEATURES

==============================================================================
*/

/*
==================
G_BanListParseEntry

Splits a single "ip|guid|expires|reason" list entry into its parts
==================
*/
static void G_BanListParseEntry( char *buf, char **ip, char **guid, int *expires, char **reason ) { // BFPR - Parse ban entry
	char	*p;

	*ip = buf;
	*guid = "";
	*expires = 0;
	*reason = "";

	p = strchr( buf, '|' );
	if ( !p ) {
		return;
	}
	*p = '\0';
	*guid = p + 1;

	p = strchr( *guid, '|' );
	if ( !p ) {
		return;
	}
	*p = '\0';
	*expires = atoi( p + 1 );

	p = strchr( p + 1, '|' );
	if ( !p ) {
		return;
	}
	*p = '\0';
	*reason = p + 1;
}

/*
==================
G_BanListIsMatch

Checks a single "ip|guid|expires|reason" list entry against 
the IP/GUID being tested. An expired entry never matches
==================
*/
static qboolean G_BanListIsMatch( const char *entry, const char *ip, const char *guid ) { // BFPR - Ban entry match
	char	buf[256];
	char	*entryIp, *entryGuid, *entryReason;
	int		entryExpires;

	Q_strncpyz( buf, entry, sizeof( buf ) );
	G_BanListParseEntry( buf, &entryIp, &entryGuid, &entryExpires, &entryReason );

	if ( entryExpires != 0 && entryExpires <= G_RealTimeSeconds() ) {
		return qfalse; // expired
	}

	if ( ip && ip[0] && entryIp[0] && !Q_stricmp( entryIp, ip ) ) {
		return qtrue;
	}
	if ( guid && guid[0] && entryGuid[0] && !Q_stricmp( entryGuid, guid ) ) {
		return qtrue;
	}
	return qfalse;
}

/*
==================
G_BanListContains

Checks whether IP or GUID matches any non-expired entry of the given ban-list cvar
==================
*/
static qboolean G_BanListContains( vmCvar_t *list, const char *ip, const char *guid ) { // BFPR - Check IP/GUID in ban list
	char	buf[MAX_CVAR_VALUE_STRING];
	char	*token, *p;

	if ( !list->string[0] ) {
		return qfalse;
	}

	Q_strncpyz( buf, list->string, sizeof( buf ) );
	p = buf;
	while ( ( token = strchr( p, ';' ) ) ) {
		*token = '\0';
		if ( p[0] && G_BanListIsMatch( p, ip, guid ) ) {
			return qtrue;
		}
		p = token + 1;
	}
	if ( p[0] && G_BanListIsMatch( p, ip, guid ) ) {
		return qtrue;
	}
	return qfalse;
}

/*
==================
G_BanListFind
==================
*/
static qboolean G_BanListFind( vmCvar_t *list, const char *ip, const char *guid, int *expiresOut, char *reasonOut, int reasonOutSize ) { // BFPR - Find matching ban entry
	char	buf[MAX_CVAR_VALUE_STRING];
	char	parseBuf[256];
	char	*token, *p;
	char	*entryIp, *entryGuid, *entryReason;
	int		entryExpires;

	if ( !list->string[0] ) {
		return qfalse;
	}

	Q_strncpyz( buf, list->string, sizeof( buf ) );
	p = buf;
	while ( ( token = strchr( p, ';' ) ) ) {
		*token = '\0';
		if ( p[0] && G_BanListIsMatch( p, ip, guid ) ) {
			Q_strncpyz( parseBuf, p, sizeof( parseBuf ) );
			G_BanListParseEntry( parseBuf, &entryIp, &entryGuid, &entryExpires, &entryReason );
			if ( expiresOut ) {
				*expiresOut = entryExpires;
			}
			if ( reasonOut ) {
				Q_strncpyz( reasonOut, entryReason[0] ? entryReason : "No_reason_given", reasonOutSize );
			}
			return qtrue;
		}
		p = token + 1;
	}
	if ( p[0] && G_BanListIsMatch( p, ip, guid ) ) {
		Q_strncpyz( parseBuf, p, sizeof( parseBuf ) );
		G_BanListParseEntry( parseBuf, &entryIp, &entryGuid, &entryExpires, &entryReason );
		if ( expiresOut ) {
			*expiresOut = entryExpires;
		}
		if ( reasonOut ) {
			Q_strncpyz( reasonOut, entryReason[0] ? entryReason : "No_reason_given", reasonOutSize );
		}
		return qtrue;
	}
	return qfalse;
}

/*
==================
G_BanListPrune

Rebuilds the given ban-list cvar dropping any expired entry
==================
*/
static void G_BanListPrune( vmCvar_t *list, const char *cvarName ) { // BFPR - Drop expired ban entries
	char	buf[MAX_CVAR_VALUE_STRING];
	char	newList[MAX_CVAR_VALUE_STRING];
	char	parseBuf[256];
	char	*token, *p;
	char	*entryIp, *entryGuid, *entryReason;
	int		entryExpires;
	qboolean	first = qtrue;

	if ( !list->string[0] ) {
		return;
	}

	Q_strncpyz( buf, list->string, sizeof( buf ) );
	newList[0] = '\0';
	p = buf;
	while ( ( token = strchr( p, ';' ) ) ) {
		*token = '\0';
		if ( p[0] ) {
			Q_strncpyz( parseBuf, p, sizeof( parseBuf ) );
			G_BanListParseEntry( parseBuf, &entryIp, &entryGuid, &entryExpires, &entryReason );
			if ( !( entryExpires != 0 && entryExpires <= G_RealTimeSeconds() ) ) {
				if ( !first ) {
					Q_strcat( newList, sizeof( newList ), ";" );
				}
				Q_strcat( newList, sizeof( newList ), p );
				first = qfalse;
			}
		}
		p = token + 1;
	}

	if ( p[0] ) {
		Q_strncpyz( parseBuf, p, sizeof( parseBuf ) );
		G_BanListParseEntry( parseBuf, &entryIp, &entryGuid, &entryExpires, &entryReason );
		if ( !( entryExpires != 0 && entryExpires <= G_RealTimeSeconds() ) ) {
			if ( !first ) {
				Q_strcat( newList, sizeof( newList ), ";" );
			}
			Q_strcat( newList, sizeof( newList ), p );
		}
	}
	trap_Cvar_Set( cvarName, newList );
	trap_Cvar_Update( list );
}

/*
==================
G_BanListAdd

Adds an "ip|guid|expires|reason" entry to the given ban-list cvar, unless
a non-expired entry already matches this IP or GUID. 
expires is a Unix timestamp (0 = permanent)
==================
*/
static void G_BanListAdd( vmCvar_t *list, const char *cvarName, const char *ip, const char *guid, int expires, const char *reason ) { // BFPR - Add entry to ban list
	char	newList[MAX_CVAR_VALUE_STRING];
	char	entry[256];
	char	cleanReason[MAX_ADMIN_BAN_REASON_LEN];
	int		i;

	G_BanListPrune( list, cvarName ); // sweep expired entries before checking/adding

	if ( G_BanListContains( list, ip, guid ) ) {
		return; // already banned by this IP or GUID
	}

	if ( !reason || !reason[0] ) {
		reason = "No reason given";
	}
	Q_strncpyz( cleanReason, reason, sizeof( cleanReason ) );
	for ( i = 0; cleanReason[i]; i++ ) {
		if ( cleanReason[i] == '|' || cleanReason[i] == ';'
		|| cleanReason[i] == ' ' || cleanReason[i] == '\t' || cleanReason[i] == '\n' ) {
			cleanReason[i] = '_';
		}
	}

	Com_sprintf( entry, sizeof( entry ), "%s|%s|%i|%s", ip ? ip : "", guid ? guid : "", expires, cleanReason );

	if ( list->string[0] ) {
		Com_sprintf( newList, sizeof( newList ), "%s;%s", list->string, entry );
	} else {
		Q_strncpyz( newList, entry, sizeof( newList ) );
	}
	trap_Cvar_Set( cvarName, newList );
	trap_Cvar_Update( list );
}

/*
==================
G_BanListRemove

Removes every entry matching this IP or GUID from the given ban-list cvar
==================
*/
static void G_BanListRemove( vmCvar_t *list, const char *cvarName, const char *ip, const char *guid ) { // BFPR - Remove IP/GUID from ban list
	char		buf[MAX_CVAR_VALUE_STRING], newList[MAX_CVAR_VALUE_STRING];
	char		*token, *p;
	qboolean	first = qtrue;

	if ( !list->string[0] ) {
		return;
	}

	Q_strncpyz( buf, list->string, sizeof( buf ) );
	newList[0] = '\0';
	p = buf;
	while ( ( token = strchr( p, ';' ) ) ) {
		*token = '\0';
		if ( p[0] && !G_BanListIsMatch( p, ip, guid ) ) {
			if ( !first ) Q_strcat( newList, sizeof( newList ), ";" );
			Q_strcat( newList, sizeof( newList ), p );
			first = qfalse;
		}
		p = token + 1;
	}
	if ( p[0] && !G_BanListIsMatch( p, ip, guid ) ) {
		if ( !first ) Q_strcat( newList, sizeof( newList ), ";" );
		Q_strcat( newList, sizeof( newList ), p );
	}
	trap_Cvar_Set( cvarName, newList );
	trap_Cvar_Update( list );
}

/*
==================
G_BanListRemoveByIndex

Remove the entry at position 'index' (1‑based) from the ban-list cvar
==================
*/
static void G_BanListRemoveByIndex( vmCvar_t *list, const char *cvarName, int index ) { // BFPR - Remove ban from ban list index
	char	buf[MAX_CVAR_VALUE_STRING], newList[MAX_CVAR_VALUE_STRING];
	char	*p, *token;
	int		currentIndex = 0;
	qboolean	first = qtrue;

	if ( !list->string[0] ) {
		G_Printf( "This ban list is empty\n" );
		return;
	}

	Q_strncpyz( buf, list->string, sizeof(buf) );
	newList[0] = '\0';
	p = buf;

	while ( ( token = strchr( p, ';' ) ) ) {
		*token = '\0';
		if ( p[0] ) {
			currentIndex++;
			if ( currentIndex != index ) {
				if ( !first ) {
					Q_strcat( newList, sizeof(newList), ";" );
				}
				Q_strcat( newList, sizeof(newList), p );
				first = qfalse;
			}
		}
		p = token + 1;
	}

	if ( p[0] ) {
		currentIndex++;
		if ( currentIndex != index ) {
			if ( !first ) {
				Q_strcat( newList, sizeof(newList), ";" );
			}
			Q_strcat( newList, sizeof(newList), p );
		}
	}

	if ( currentIndex < index ) {
		G_Printf( "Index %d not found in list\n", index );
		return;
	}

	trap_Cvar_Set( cvarName, newList );
	trap_Cvar_Update( list );
	G_Printf( "Removed entry %d from %s\n", index, cvarName );
}


/*
==================
G_FormatDuration

Formats a duration in seconds as "Xd Xhr Xmin Xsec"
==================
*/
static void G_FormatDuration( int totalSeconds, char *out, int outSize ) { // BFPR - Format seconds as Xd Xhr Xmin Xsec
	int		days, hours, minutes, seconds;
	char	part[16];

	if ( totalSeconds < 0 ) {
		totalSeconds = 0;
	}

	days = totalSeconds / 86400;
	hours = ( totalSeconds % 86400 ) / 3600;
	minutes = ( totalSeconds % 3600 ) / 60;
	seconds = totalSeconds % 60;

	out[0] = '\0';

	if ( days > 0 ) {
		Com_sprintf( part, sizeof(part), "%id ", days );
		Q_strcat( out, outSize, part );
	}
	if ( hours > 0 || days > 0 ) {
		Com_sprintf( part, sizeof(part), "%ihr ", hours );
		Q_strcat( out, outSize, part );
	}
	if ( minutes > 0 || hours > 0 || days > 0 ) {
		Com_sprintf( part, sizeof(part), "%imin ", minutes );
		Q_strcat( out, outSize, part );
	}
	// seconds always shown, so the string is never empty
	Com_sprintf( part, sizeof(part), "%isec", seconds );
	Q_strcat( out, outSize, part );
}

/*
==================
G_BanListPrintf

Prints every entry of the given ban-list cvar:
ban_id: ip | guid | expires (or "permanent") | reason
==================
*/
static void G_BanListPrintf( vmCvar_t *list, const char *label ) { // BFPR - Print ban list entries
	char	buf[MAX_CVAR_VALUE_STRING];
	char	displayReason[MAX_ADMIN_BAN_REASON_LEN];
	char	durationStr[32];
	char	parseBuf[256];
	char	*token, *p;
	char	*entryIp, *entryGuid, *entryReason;
	int		i, entryExpires, id = 1;

	trap_Cvar_VariableStringBuffer( list->string, buf, sizeof(buf) );
	if ( !list->string[0] ) {
		G_Printf( "%s: none\n", label );
		return;
	}

	G_Printf( "%s:\n", label );
	Q_strncpyz( buf, list->string, sizeof( buf ) );
	p = buf;
	while ( ( token = strchr( p, ';' ) ) ) {
		*token = '\0';
		if ( p[0] ) {
			Q_strncpyz( parseBuf, p, sizeof( parseBuf ) );
			G_BanListParseEntry( parseBuf, &entryIp, &entryGuid, &entryExpires, &entryReason );
			Q_strncpyz( displayReason, entryReason, sizeof( displayReason ) );
			for ( i = 0; displayReason[i]; i++ ) {
				if ( displayReason[i] == '_' ) displayReason[i] = ' ';
			}
			if ( entryExpires == 0 ) {
				G_Printf( "  ban_id=%d: ip=%s guid=%s expires=permanent reason=%s\n",
					id, entryIp[0] ? entryIp : "-", entryGuid[0] ? entryGuid : "-",
					displayReason[0] ? displayReason : "No reason given" );
			} else {
				G_FormatDuration( entryExpires - G_RealTimeSeconds(), durationStr, sizeof(durationStr) );
				G_Printf( "  ban_id=%d: ip=%s guid=%s expires_in=%s reason=%s\n",
					id, entryIp[0] ? entryIp : "-", entryGuid[0] ? entryGuid : "-",
					durationStr,
					displayReason[0] ? displayReason : "No reason given" );
			}
			++id;
		}
		p = token + 1;
	}
	if ( p[0] ) {
		Q_strncpyz( parseBuf, p, sizeof( parseBuf ) );
		G_BanListParseEntry( parseBuf, &entryIp, &entryGuid, &entryExpires, &entryReason );
		Q_strncpyz( displayReason, entryReason, sizeof( displayReason ) );
		for ( i = 0; displayReason[i]; i++ ) {
			if ( displayReason[i] == '_' ) displayReason[i] = ' ';
		}
		if ( entryExpires == 0 ) {
			G_Printf( "  ban_id=%d: ip=%s guid=%s expires=permanent reason=%s\n",
				id, entryIp[0] ? entryIp : "-", entryGuid[0] ? entryGuid : "-",
				displayReason[0] ? displayReason : "No reason given" );
		} else {
			G_FormatDuration( entryExpires - G_RealTimeSeconds(), durationStr, sizeof(durationStr) );
			G_Printf( "  ban_id=%d: ip=%s guid=%s expires_in=%s reason=%s\n",
				id, entryIp[0] ? entryIp : "-", entryGuid[0] ? entryGuid : "-",
				durationStr,
				displayReason[0] ? displayReason : "No reason given" );
		}
		++id;
	}
}

/*
==================
G_ResolveSenderIdentity

Resolves this client's current IP (port stripped) and GUID
==================
*/
static void G_ResolveSenderIdentity( gentity_t *ent, char *ipOut, int ipSize, char *guidOut, int guidSize ) { // BFPR - Resolve IP + GUID for a client
	char	userinfo[MAX_INFO_STRING];
	char	*ip, *colon;

	ipOut[0] = '\0';
	guidOut[0] = '\0';

	if ( !ent || !ent->client ) {
		return;
	}

	trap_GetUserinfo( ent - g_entities, userinfo, sizeof( userinfo ) );
	ip = Info_ValueForKey( userinfo, "ip" );
	colon = strchr( ip, ':' );
	if ( colon ) {
		*colon = '\0';
	}
	Q_strncpyz( ipOut, ip, ipSize );
	Q_strncpyz( guidOut, ent->client->pers.guid, guidSize );
}

/*
==================
G_BanMessageForSender
==================
*/
qboolean G_BanMessageForSender( qboolean cp, gentity_t *ent, vmCvar_t *list, char *out, int outSize ) { // BFPR - Format ban expiration and reason for a client
	char	ip[64], guid[33];
	char	reason[MAX_ADMIN_BAN_REASON_LEN];
	char	durationStr[32];
	int		i, expires, remaining;

	if ( !ent || !ent->client ) {
		return qfalse;
	}

	G_ResolveSenderIdentity( ent, ip, sizeof(ip), guid, sizeof(guid) );
	if ( !G_BanListFind( list, ip, guid, &expires, reason, sizeof(reason) ) ) {
		return qfalse;
	}

	// stored reason uses '_' in place of spaces - convert back for display
	for ( i = 0; reason[i]; i++ ) {
		if ( reason[i] == '_' ) {
			reason[i] = ' ';
		}
	}

	if ( expires == 0 ) {
		if ( cp ) {
			Com_sprintf( out, outSize, "\nBan: permanent\nReason:\n%s", reason );
		} else {
			Com_sprintf( out, outSize, "Ban: permanent | Reason: %s", reason );
		}
	} else {
		remaining = expires - G_RealTimeSeconds();
		G_FormatDuration( remaining, durationStr, sizeof(durationStr) );
		if ( cp ) {
			Com_sprintf( out, outSize, "\nYour ban expires in:\n^3%s^7\nBan reason:\n%s",
				durationStr, reason );
		} else {
			Com_sprintf( out, outSize, "Expires in: %s | Ban reason: %s",
				durationStr, reason );
		}
	}
	return qtrue;
}

/*
==================
G_IsSenderMuted

Resolves this client's current IP/GUID and checks them against g_muteban_list
==================
*/
qboolean G_IsSenderMuted( gentity_t *ent ) { // BFPR - Mute the sender
	char	ip[64], guid[33];

	if ( !ent || !ent->client ) {
		return qfalse;
	}

	G_ResolveSenderIdentity( ent, ip, sizeof( ip ), guid, sizeof( guid ) );
	return G_BanListContains( &g_muteban_list, ip, guid );
}

/*
==================
G_IsSenderVotebanned

Resolves this client's current IP/GUID and checks them against g_voteban_list
==================
*/
qboolean G_IsSenderVotebanned( gentity_t *ent ) { // BFPR - Vote-ban the sender
	char	ip[64], guid[33];

	if ( !ent || !ent->client ) {
		return qfalse;
	}

	G_ResolveSenderIdentity( ent, ip, sizeof( ip ), guid, sizeof( guid ) );
	return G_BanListContains( &g_voteban_list, ip, guid );
}

/*
==================
G_IsSenderPlaybanned

Resolves this client's current IP/GUID and checks them against g_playban_list
==================
*/
qboolean G_IsSenderPlaybanned( gentity_t *ent ) { // BFPR - Play-ban the sender
	char	ip[64], guid[33];

	if ( !ent || !ent->client ) {
		return qfalse;
	}

	G_ResolveSenderIdentity( ent, ip, sizeof( ip ), guid, sizeof( guid ) );
	return G_BanListContains( &g_playban_list, ip, guid );
}

/*
=================
Svcmd_AddIP_f
=================
*/
void Svcmd_AddIP_f (void)
{
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage:  addip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	AddIP( str );

}

/*
=================
Svcmd_RemoveIP_f
=================
*/
void Svcmd_RemoveIP_f (void)
{
	ipFilter_t	f;
	int			i;
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage:  sv removeip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	if (!StringToFilter (str, &f))
		return;

	for (i=0 ; i<numIPFilters ; i++) {
		if (ipFilters[i].mask == f.mask	&&
			ipFilters[i].compare == f.compare) {
			ipFilters[i].compare = 0xffffffffu;
			G_Printf ("Removed.\n");

			UpdateIPBans();
			return;
		}
	}

	G_Printf ( "Didn't find %s.\n", str );
}

/*
===================
Svcmd_EntityList_f
===================
*/
void	Svcmd_EntityList_f (void) {
	int			e;
	gentity_t		*check;

	check = g_entities+1;
	for (e = 1; e < level.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		G_Printf("%3i:", e);
		switch ( check->s.eType ) {
		case ET_GENERAL:
			G_Printf("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			G_Printf("ET_PLAYER           ");
			break;
		case ET_ITEM:
			G_Printf("ET_ITEM             ");
			break;
		case ET_MISSILE:
			G_Printf("ET_MISSILE          ");
			break;
		case ET_MOVER:
			G_Printf("ET_MOVER            ");
			break;
		case ET_BEAM:
			G_Printf("ET_BEAM             ");
			break;
		case ET_PORTAL:
			G_Printf("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			G_Printf("ET_SPEAKER          ");
			break;
		case ET_PUSH_TRIGGER:
			G_Printf("ET_PUSH_TRIGGER     ");
			break;
		case ET_TELEPORT_TRIGGER:
			G_Printf("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			G_Printf("ET_INVISIBLE        ");
			break;
// BFP - no hook
#if 0
		case ET_GRAPPLE:
			G_Printf("ET_GRAPPLE          ");
			break;
#endif
		default:
			G_Printf("%3i                 ", check->s.eType);
			break;
		}

		if ( check->classname ) {
			G_Printf("%s", check->classname);
		}
		G_Printf("\n");
	}
}

/*
===================
Svcmd_PlayerList_f

Lists every connected client's slot number, netname and IP,
so the admin knows which id to pass to mute/unmute, voteban/unvoteban,
playban/unplayban, forceteam, etc
===================
*/
void	Svcmd_PlayerList_f ( void ) { // BFPR - playerlist command
	int			i;
	gclient_t	*cl;
	char		userinfo[MAX_INFO_STRING];
	char		*ip, *colon, *skillStr;
	char		skillDisp[8];
	qboolean	any = qfalse, hasBots = qfalse;
	qboolean	isBot;

	// only show the bot/skill columns if at least one bot is connected
	for ( i = 0, cl = level.clients ; i < level.maxclients ; i++, cl++ ) {
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( g_entities[i].r.svFlags & SVF_BOT ) {
			hasBots = qtrue;
			break;
		}
	}

	if ( hasBots ) {
		G_Printf( "id  name                             ip               bot skill team\n" );
		G_Printf( "--- -------------------------------- ---------------- --- ----- ----------\n" );
	} else {
		G_Printf( "id  name                             ip               team\n" );
		G_Printf( "--- -------------------------------- ---------------- ----------\n" );
	}

	for ( i = 0, cl = level.clients ; i < level.maxclients ; i++, cl++ ) {
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		any = qtrue;

		isBot = ( g_entities[i].r.svFlags & SVF_BOT );

		if ( isBot ) {
			ip = "---";
		} else {
			trap_GetUserinfo( i, userinfo, sizeof( userinfo ) );
			ip = Info_ValueForKey( userinfo, "ip" );
			// strip the port if present, e.g. "1.2.3.4:27960"
			colon = strchr( ip, ':' );
			if ( colon ) {
				*colon = '\0';
			}
		}

		if ( hasBots ) {
			if ( isBot ) {
				trap_GetUserinfo( i, userinfo, sizeof( userinfo ) );
				skillStr = Info_ValueForKey( userinfo, "skill" );
				if ( !skillStr[0] ) {
					skillStr = "?";
				} else { // strip the float string
					Com_sprintf( skillDisp, sizeof( skillDisp ), "%i", (int)( atof( skillStr ) + 0.5f ) );
					skillStr = skillDisp;
				}
			} else {
				skillStr = "-";
			}

			G_Printf( "%-3i %-32s %-16s %-3s %-5s %s\n", i, cl->pers.netname, ip,
				isBot ? "^3yes^7" : "no", skillStr,
				cl->sess.sessionTeam == TEAM_SPECTATOR ? "spectator" :
				cl->sess.sessionTeam == TEAM_RED ? "red" :
				cl->sess.sessionTeam == TEAM_BLUE ? "blue" : "free" );
		} else {
			G_Printf( "%-3i %-32s %-16s %s\n", i, cl->pers.netname, ip,
				cl->sess.sessionTeam == TEAM_SPECTATOR ? "spectator" :
				cl->sess.sessionTeam == TEAM_RED ? "red" :
				cl->sess.sessionTeam == TEAM_BLUE ? "blue" : "free" );
		}
	}

	if ( !any ) {
		G_Printf( "No players connected.\n" );
	}
}

/*
==================
Svcmd_Mute_f

Mutes a currently-connected client by IP and GUID, persists in g_muteban_list.
usage: mute <client id> [minutes] [reason...]
minutes omitted or 0 = permanent. reason omitted = "No reason given"
==================
*/
static void Svcmd_Mute_f( void ) { // BFPR - mute <client id> [minutes] [reason...] command
	char		arg[MAX_TOKEN_CHARS];
	int			targetNum;
	char		userinfo[MAX_INFO_STRING];
	char		*ip, *colon;
	char		*guid;
	int			minutes, expires;
	char		*reason;

	if ( trap_Argc() < 2 ) {
		G_Printf( "usage: mute <client id> [minutes] [reason...]\n" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = atoi( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients
	|| level.clients[targetNum].pers.connected != CON_CONNECTED ) {
		G_Printf( "Client %i is not active\n", targetNum );
		return;
	}

	// bots share the "localhost" IP and have no GUID, so muting one would mute all of them
	if ( g_entities[targetNum].r.svFlags & SVF_BOT ) {
		G_Printf( "Client %i is a bot, mute cannot be applied.\n", targetNum );
		return;
	}

	minutes = 0;
	if ( trap_Argc() >= 3 ) {
		trap_Argv( 2, arg, sizeof( arg ) );
		minutes = atoi( arg );
		if ( minutes < 0 ) {
			minutes = 0;
		}
	}
	expires = minutes > 0 ? G_RealTimeSeconds() + minutes * 60 : 0;
	reason = trap_Argc() >= 4 ? ConcatArgs( 3 ) : "";

	trap_GetUserinfo( targetNum, userinfo, sizeof( userinfo ) );
	ip = Info_ValueForKey( userinfo, "ip" );
	// strip the port if present, e.g. "1.2.3.4:27960"
	colon = strchr( ip, ':' );
	if ( colon ) {
		*colon = '\0';
	}
	guid = level.clients[targetNum].pers.guid;

	G_BanListAdd( &g_muteban_list, "g_muteban_list", ip, guid, expires, reason );
	G_Printf( "Muted %s (%s)%s\n", level.clients[targetNum].pers.netname, ip,
		minutes > 0 ? va( " for %i minutes", minutes ) : " permanently" );
	G_LogPrintf( "mute: %s (%s|%s) expires=%i reason=%s\n",
		level.clients[targetNum].pers.netname, ip, guid, expires, reason[0] ? reason : "No reason given" );
}

/*
==================
Svcmd_Unmute_f
==================
*/
static void Svcmd_Unmute_f( void ) { // BFPR - unmute <ban_id, ip or guid> command
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) {
		G_Printf( "usage: unmute <ban_id, ip or guid>\n" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	if ( arg[0] ) {
		int	i = atoi( arg );
		if ( i > 0 ) {
			G_BanListRemoveByIndex( &g_muteban_list, "g_muteban_list", i );
			G_LogPrintf( "unmute: %s\n", arg );
		} else {
			G_Printf( "Index must be positive.\n" );
		}
	} else {
		G_BanListRemove( &g_muteban_list, "g_muteban_list", arg, arg );
		G_Printf( "Removed %s from mute list\n", arg );
		G_LogPrintf( "unmute: %s\n", arg );
	}
}

/*
==================
Svcmd_MuteBans_f
==================
*/
static void Svcmd_MuteBans_f( void ) { // BFPR - mutebans command
	G_BanListPrune( &g_muteban_list, "g_muteban_list" );
	G_BanListPrintf( &g_muteban_list, "Muted" );
}
 
/*
==================
Svcmd_Playban_f
 
Bans a currently-connected client from playing (forced to spectate), by IP
and GUID, persists in g_playban_list. If the client is on an active team
right now, force them to spectator immediately.
usage: playban <client id> [minutes] [reason...]
minutes omitted or 0 = permanent. reason omitted = "No reason given"
==================
*/
static void Svcmd_Playban_f( void ) { // BFPR - playban <client id> [minutes] [reason...] command
	char		arg[MAX_TOKEN_CHARS];
	int			targetNum;
	char		userinfo[MAX_INFO_STRING];
	char		*ip, *colon;
	char		*guid;
	int			minutes, expires;
	char		*reason;
	gentity_t	*target;

	if ( trap_Argc() < 2 ) {
		G_Printf( "usage: playban <client id> [minutes] [reason...]\n" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = atoi( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients
	|| level.clients[targetNum].pers.connected != CON_CONNECTED ) {
		G_Printf( "Client %i is not active\n", targetNum );
		return;
	}

	// bots share the "localhost" IP and have no GUID, so playbanning one would playban all of them
	if ( g_entities[targetNum].r.svFlags & SVF_BOT ) {
		G_Printf( "Client %i is a bot, playban cannot be applied.\n", targetNum );
		return;
	}

	minutes = 0;
	if ( trap_Argc() >= 3 ) {
		trap_Argv( 2, arg, sizeof( arg ) );
		minutes = atoi( arg );
		if ( minutes < 0 ) {
			minutes = 0;
		}
	}
	expires = minutes > 0 ? G_RealTimeSeconds() + minutes * 60 : 0;
	reason = trap_Argc() >= 4 ? ConcatArgs( 3 ) : "";

	trap_GetUserinfo( targetNum, userinfo, sizeof( userinfo ) );
	ip = Info_ValueForKey( userinfo, "ip" );
	// strip the port if present, e.g. "1.2.3.4:27960"
	colon = strchr( ip, ':' );
	if ( colon ) {
		*colon = '\0';
	}
	guid = level.clients[targetNum].pers.guid;

	G_BanListAdd( &g_playban_list, "g_playban_list", ip, guid, expires, reason );
	G_Printf( "Play-banned %s (%s)%s\n", level.clients[targetNum].pers.netname, ip,
		minutes > 0 ? va( " for %i minutes", minutes ) : " permanently" );
	G_LogPrintf( "playban: %s (%s|%s) expires=%i reason=%s\n",
		level.clients[targetNum].pers.netname, ip, guid, expires, reason[0] ? reason : "No reason given" );

	// if they are on an active team right now, force them to spectator immediately
	target = &g_entities[targetNum];
	if ( target->client && target->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		char	durationStr[32];
		int		remaining = expires - G_RealTimeSeconds();
		G_FormatDuration( remaining, durationStr, sizeof(durationStr) );

		SetTeam( target, "spectator" );
		ClientBegin( targetNum );
		trap_SendServerCommand( targetNum, 
			va( "cp \"^1You are banned from \n^1playing on this server.\n\nYour ban expires in:\n^3%s^7\nBan reason:\n%s\n\"", 
				durationStr, reason[0] ? reason : "No reason given" )
		);
		trap_SendServerCommand( targetNum, 
			va( "print \"You are banned from playing on this server. Expires in: %s | Ban reason: %s\n\"", 
				durationStr, reason[0] ? reason : "No reason given" )
		);
	}
}

/*
==================
Svcmd_Unplayban_f
==================
*/
static void Svcmd_Unplayban_f( void ) { // BFPR - unplayban <ban_id, ip or guid> command
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) {
		G_Printf( "usage: unplayban <ban_id, ip or guid>\n" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	if ( arg[0] ) {
		int	i = atoi( arg );
		if ( i > 0 ) {
			G_BanListRemoveByIndex( &g_playban_list, "g_playban_list", i );
			G_LogPrintf( "unplayban: %s\n", arg );
		} else {
			G_Printf( "Index must be positive.\n" );
		}
	} else {
		G_BanListRemove( &g_playban_list, "g_playban_list", arg, arg );
		G_Printf( "Removed %s from playban list\n", arg );
		G_LogPrintf( "unplayban: %s\n", arg );
	}
}

/*
==================
Svcmd_Playbans_f
==================
*/
static void Svcmd_Playbans_f( void ) { // BFPR - playbans command
	G_BanListPrune( &g_playban_list, "g_playban_list" );
	G_BanListPrintf( &g_playban_list, "Play-banned" );
}

/*
==================
Svcmd_Voteban_f

Prevents a currently-connected client from calling or casting votes,
by IP and GUID, persists in g_voteban_list.
usage: voteban <client id> [minutes] [reason...]
minutes omitted or 0 = permanent. reason omitted = "No reason given"
==================
*/
static void Svcmd_Voteban_f( void ) { // BFPR - voteban <client id> [minutes] [reason...] command
	char		arg[MAX_TOKEN_CHARS];
	int			targetNum;
	char		userinfo[MAX_INFO_STRING];
	char		*ip, *colon;
	char		*guid;
	int			minutes, expires;
	char		*reason;

	if ( trap_Argc() < 2 ) {
		G_Printf( "usage: voteban <client id> [minutes] [reason...]\n" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = atoi( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients
	|| level.clients[targetNum].pers.connected != CON_CONNECTED ) {
		G_Printf( "Client %i is not active\n", targetNum );
		return;
	}

	// bots share the "localhost" IP and have no GUID, so votebanning one would voteban all of them
	if ( g_entities[targetNum].r.svFlags & SVF_BOT ) {
		G_Printf( "Client %i is a bot, voteban cannot be applied.\n", targetNum );
		return;
	}

	minutes = 0;
	if ( trap_Argc() >= 3 ) {
		trap_Argv( 2, arg, sizeof( arg ) );
		minutes = atoi( arg );
		if ( minutes < 0 ) {
			minutes = 0;
		}
	}
	expires = minutes > 0 ? G_RealTimeSeconds() + minutes * 60 : 0;
	reason = trap_Argc() >= 4 ? ConcatArgs( 3 ) : "";

	trap_GetUserinfo( targetNum, userinfo, sizeof( userinfo ) );
	ip = Info_ValueForKey( userinfo, "ip" );
	// strip the port if present, e.g. "1.2.3.4:27960"
	colon = strchr( ip, ':' );
	if ( colon ) {
		*colon = '\0';
	}
	guid = level.clients[targetNum].pers.guid;

	G_BanListAdd( &g_voteban_list, "g_voteban_list", ip, guid, expires, reason );
	G_Printf( "Vote-banned %s (%s)%s\n", level.clients[targetNum].pers.netname, ip,
		minutes > 0 ? va( " for %i minutes", minutes ) : " permanently" );
	G_LogPrintf( "voteban: %s (%s|%s) expires=%i reason=%s\n",
		level.clients[targetNum].pers.netname, ip, guid, expires, reason[0] ? reason : "No reason given" );
}

/*
==================
Svcmd_Unvoteban_f
==================
*/
static void Svcmd_Unvoteban_f( void ) { // BFPR - unvoteban <ban_id, ip or guid> command
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) {
		G_Printf( "usage: unvoteban <ban_id, ip or guid>\n" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	if ( arg[0] ) {
		int	i = atoi( arg );
		if ( i > 0 ) {
			G_BanListRemoveByIndex( &g_voteban_list, "g_voteban_list", i );
			G_LogPrintf( "unvoteban: %s\n", arg );
		} else {
			G_Printf( "Index must be positive.\n" );
		}
	} else {
		G_BanListRemove( &g_voteban_list, "g_voteban_list", arg, arg );
		G_Printf( "Removed %s from voteban list\n", arg );
		G_LogPrintf( "unvoteban: %s\n", arg );
	}
}

/*
==================
Svcmd_Votebans_f
==================
*/
static void Svcmd_Votebans_f( void ) { // BFPR - votebans command
	G_BanListPrune( &g_voteban_list, "g_voteban_list" );
	G_BanListPrintf( &g_voteban_list, "Vote-banned" );
}


gclient_t	*ClientForString( const char *s ) {
	gclient_t	*cl;
	int			i;
	int			idnum;

	// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			Com_Printf( "Bad client slot: %i\n", idnum );
			return NULL;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			G_Printf( "Client %i is not connected\n", idnum );
			return NULL;
		}
		return cl;
	}

	// check for a name match
	for ( i=0 ; i < level.maxclients ; i++ ) {
		cl = &level.clients[i];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( !Q_stricmp( cl->pers.netname, s ) ) {
			return cl;
		}
	}

	G_Printf( "User %s is not on the server\n", s );

	return NULL;
}

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void	Svcmd_ForceTeam_f( void ) {
	gclient_t	*cl;
	char		str[MAX_TOKEN_CHARS];

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}

	// set the team
	trap_Argv( 2, str, sizeof( str ) );
	SetTeam( &g_entities[cl - level.clients], str );
}


/*
=================
ConsoleCommand

=================
*/
qboolean	ConsoleCommand( void ) {
	char	cmd[MAX_TOKEN_CHARS];

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if ( Q_stricmp (cmd, "entitylist") == 0 ) {
		Svcmd_EntityList_f();
		return qtrue;
	}

	if ( Q_stricmp ( cmd, "playerlist" ) == 0 ) { // BFPR - playerlist command
		Svcmd_PlayerList_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "forceteam") == 0 ) {
		Svcmd_ForceTeam_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "game_memory") == 0) {
		Svcmd_GameMem_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addbot") == 0) {
		Svcmd_AddBot_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "botlist") == 0) {
		Svcmd_BotList_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "abort_podium") == 0) {
		Svcmd_AbortPodium_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addip") == 0) {
		Svcmd_AddIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "removeip") == 0) {
		Svcmd_RemoveIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "listip") == 0) {
		trap_SendConsoleCommand( EXEC_NOW, "g_banIPs\n" );
		return qtrue;
	}

	if ( Q_stricmp ( cmd, "mute" ) == 0 ) { // BFPR - mute <client id> command
		Svcmd_Mute_f();
		return qtrue;
	}

	if ( Q_stricmp ( cmd, "unmute" ) == 0 ) { // BFPR - unmute <ban_id, ip or guid> command
		Svcmd_Unmute_f();
		return qtrue;
	}

	if ( Q_stricmp ( cmd, "mutebans" ) == 0 ) { // BFPR - mutebans command
		Svcmd_MuteBans_f();
		return qtrue;
	}

	if ( Q_stricmp ( cmd, "playban" ) == 0 ) { // BFPR - playban <client id> command
		Svcmd_Playban_f();
		return qtrue;
	}

	if ( Q_stricmp ( cmd, "unplayban" ) == 0 ) { // BFPR - unplayban <ban_id, ip or guid> command
		Svcmd_Unplayban_f();
		return qtrue;
	}

	if ( Q_stricmp ( cmd, "playbans" ) == 0 ) { // BFPR - playbans command
		Svcmd_Playbans_f();
		return qtrue;
	}

	if ( Q_stricmp ( cmd, "voteban" ) == 0 ) { // BFPR - voteban <client id> command
		Svcmd_Voteban_f();
		return qtrue;
	}

	if ( Q_stricmp ( cmd, "unvoteban" ) == 0 ) { // BFPR - unvoteban <ban_id, ip or guid> command
		Svcmd_Unvoteban_f();
		return qtrue;
	}

	if ( Q_stricmp ( cmd, "votebans" ) == 0 ) { // BFPR - votebans command
		Svcmd_Votebans_f();
		return qtrue;
	}

	if (g_dedicated.integer) {
		if (Q_stricmp (cmd, "say") == 0) {
			trap_SendServerCommand( -1, va("print \"server: %s\"", ConcatArgs(1) ) );
			return qtrue;
		}
		// everything else will also be printed as a say command
		trap_SendServerCommand( -1, va("print \"server: %s\"", ConcatArgs(0) ) );
		return qtrue;
	}

	return qfalse;
}

