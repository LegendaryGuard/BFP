/*
 * Copyright (C) 2000-2001 Tim Angus, 1995-1997 Michael Hipp,
 *               1993 Sun Microsystems, 1999 Aaron Holtzman.
 *
 * Since I'll probably get whined at if I LGPL this, I won't.
 * All I ask if that if you use this due credit is given to
 * the above persons and parties and that this message
 * remains intact.
 */

/*
 *  Portions Copyright (C) 2000-2001 Tim Angus
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the OSML - Open Source Modification License v1.0 as
 *  described in the file COPYING which is distributed with this source
 *  code.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "ui_mp3decoder.h"

mpegPacket_t *gmp;

static float ispow[ 8207 ];
static float aa_ca[ 8 ], aa_cs[ 8 ];
static float COS1[ 12 ][ 6 ];
static float win[ 4 ][ 36 ];
static float win1[ 4 ][ 36 ];
static float gainpow2[ 256 + 118 + 4 ];
static float COS9[ 9 ];
static float COS6_1, COS6_2;
static float tfcos36[ 9 ];
static float tfcos12[ 3 ];

static int longLimit[ 9 ][ 23 ];
static int shortLimit[ 9 ][ 14 ];

                  
static int bitindex;
static unsigned char *wordpointer;

static int mapbuf0[ 9 ][ 152 ];
static int mapbuf1[ 9 ][ 156 ];
static int mapbuf2[ 9 ][ 44 ];
static int *map[ 9 ][ 3 ];
static int *mapend[ 9 ][ 3 ];

static unsigned int n_slen2[ 512 ]; /* MPEG 2.0 slen for 'normal' mode */
static unsigned int i_slen2[ 256 ]; /* MPEG 2.0 slen for intensity stereo */

static float tan1_1[ 16 ], tan2_1[ 16 ], tan1_2[ 16 ], tan2_2[ 16 ];
static float pow1_1[ 2 ][ 16 ], pow2_1[ 2 ][ 16 ], pow1_2[ 2 ][ 16 ], pow2_2[ 2 ][ 16 ];

static float decwin[ 512 + 32 ];
static float cos64[ 16 ], cos32[ 8], cos16[ 4 ], cos8[ 2 ], cos4[ 1 ];
static float *pnts[] = { cos64, cos32, cos16, cos8, cos4 };

#define CBUFFER_SIZE 16384
#define OBUFFER_SIZE 8192
static char conversionBuffer[ CBUFFER_SIZE ];
static char outputBuffer[ OBUFFER_SIZE ];

/*
==================
makeDecodeTables
==================
*/
static void makeDecodeTables( long scaleval )
{
  int i, j, k, kr, divv;
  float *table, *costab;

  
  for( i = 0; i < 5; i++ )
  {
    kr = 0x10 >> i;
    divv = 0x40 >> i;
    costab = pnts[ i ];
    
    for( k = 0; k < kr; k++)
      costab[ k ] = 1.0 / ( 2.0 * cos( M_PI * ( (float)k * 2.0 + 1.0 ) / (float)divv ) );
  }

  table = decwin;
  scaleval = -scaleval;
  
  for( i = 0, j = 0; i < 256; i++, j++, table += 32 )
  {
    if( table < decwin + 512 + 16 )
      table[ 16 ] = table[ 0 ] = (float)intwinbase[ j ] / 65536.0 * (float)scaleval;
    if( i % 32 == 31 )
      table -= 1023;
    if( i % 64 == 63 )
      scaleval = - scaleval;
  }

  for( /* i=256 */ ; i < 512; i++, j--, table += 32 )
  {
    if( table < decwin + 512 + 16 )
      table[ 16 ] = table[ 0 ] = (float)intwinbase[ j ] / 65536.0 * (float)scaleval;
    if( i % 32 == 31 )
      table -= 1023;
    if( i % 64 == 63 )
      scaleval = - scaleval;
  }
}

/*
==================
initLayer3
==================
*/
void initLayer3( int down_sample_sblimit )
{
  int i, j, k, l;

  for( i = -256; i < 118 + 4 ; i++ )
    gainpow2[ i + 256 ] = pow( (float)2.0, -0.25 * (float)( i + 210 ) );

  for( i = 0; i < 8207; i++ )
    ispow[ i ] = pow( (float)i, (float)4.0 / 3.0 );

  for ( i = 0; i < 8; i++ )
  {
    static float Ci[ 8 ] = { -0.6, -0.535, -0.33, -0.185, -0.095, -0.041, -0.0142, -0.0037 };
    float sq = sqrt( 1.0 + Ci[ i ] * Ci[ i ] );
    aa_cs[ i ] = 1.0 / sq;
    aa_ca[ i ] = Ci[ i ] / sq;
  }

  for( i = 0; i < 18; i++ )
  {
    win[ 0 ][ i ] = win[ 1 ][ i ] = 0.5 * sin( M_PI / 72.0 * (float)( 2 * ( i + 0 ) + 1 ) ) /
                                    cos( M_PI * (float)( 2 * ( i + 0 ) + 19 ) / 72.0 );
    win[ 0 ][ i + 18 ] = win[ 3 ][ i + 18 ] = 0.5 * sin( M_PI / 72.0 * (float)( 2 * ( i + 18 ) + 1 ) ) /
                                    cos( M_PI * (float)( 2 * ( i + 18 ) + 19 ) / 72.0 );
  }
  
  for( i = 0; i < 6; i++ )
  {
    win[ 1 ][ i + 18 ] = 0.5 / cos( M_PI * (float)( 2 * ( i + 18 ) + 19 ) / 72.0 );
    win[ 3 ][ i + 12 ] = 0.5 / cos( M_PI * (float)( 2 * ( i + 12 ) + 19 ) / 72.0 );
    win[ 1 ][ i + 24 ] = 0.5 * sin( M_PI / 24.0 * (float)( 2 * i + 13 ) ) /
                                    cos( M_PI * (float)( 2 * ( i + 24 ) + 19 ) / 72.0 );
    win[ 1 ][ i + 30 ] = win[ 3 ][ i ] = 0.0;
    win[ 3 ][ i + 6 ] = 0.5 * sin( M_PI / 24.0 * (float)( 2 * i + 1 ) ) /
                                    cos( M_PI * (float)( 2 * ( i + 6 ) + 19 ) / 72.0 );
  }

  for( i = 0; i < 9; i++ )
    COS9[ i ] = cos( M_PI / 18.0 * (float)i );

  for( i = 0; i < 9; i++ )
    tfcos36[ i ] = 0.5 / cos( M_PI * (float)( i * 2 + 1 ) / 36.0 );
  for( i = 0; i < 3; i++ )
    tfcos12[ i ] = 0.5 / cos( M_PI * (float)( i * 2 + 1 ) / 12.0 );

  COS6_1 = cos( M_PI / 6.0 * (float)1 );
  COS6_2 = cos( M_PI / 6.0 * (float)2 );

  for( i = 0; i < 12; i++ )
  {
    win[ 2 ][ i ]  = 0.5 * sin( M_PI / 24.0 * (float)( 2 * i + 1 ) ) /
                         cos ( M_PI * (float) ( 2 * i + 7 ) / 24.0 );
    for( j = 0; j < 6; j++ )
      COS1[ i ][ j ] = cos( M_PI / 24.0 * (float)( ( 2 * i + 7 ) * ( 2 * j + 1) ) );
  }

  for( j = 0; j < 4; j++ )
  {
    static int len[ 4 ] = { 36, 36, 12, 36 };
    
    for( i = 0; i < len[ j ]; i += 2 )
      win1[ j ][ i ] = + win[ j ][ i ];
    for( i = 1; i < len[ j ]; i += 2 )
      win1[ j ][ i ] = - win[ j ][ i ];
  }

  for( i = 0; i < 16; i++ )
  {
    float t = tan( (float)i * M_PI / 12.0 );
    tan1_1[ i ] = t / ( 1.0 + t );
    tan2_1[ i ] = 1.0 / ( 1.0 + t );
    tan1_2[ i ] = M_SQRT2 * t / ( 1.0 + t );
    tan2_2[ i ] = M_SQRT2 / ( 1.0 + t );

    for( j = 0 ; j < 2; j++ )
    {
      float base = pow( 2.0, -0.25 * ( j + 1.0 ) );
      float p1 = 1.0, p2 = 1.0;
      
      if( i > 0 )
      {
        if( i & 1 )
          p1 = pow( base, ( i + 1.0 ) * 0.5 );
        else
          p2 = pow( base, i * 0.5 );
      }
      pow1_1[ j ][ i ] = p1;
      pow2_1[ j ][ i ] = p2;
      pow1_2[ j ][ i ] = M_SQRT2 * p1;
      pow2_2[ j ][ i ] = M_SQRT2 * p2;
    }
  }

  for( j = 0; j < 9; j++ )
  {
    bandInfo_t *bi = &bandInfo[ j ];
    int *mp;
    int cb, lwin;
    int *bdf;
 
    mp = map[ j ][ 0 ] = mapbuf0[ j ];
    bdf = bi->longDiff;
    
    for( i = 0, cb = 0; cb < 8; cb++, i += *bdf++ )
    {
      *mp++ = ( *bdf ) >> 1;
      *mp++ = i;
      *mp++ = 3;
      *mp++ = cb;
    }
    
    bdf = bi->shortDiff + 3;
    
    for( cb = 3; cb < 13; cb++ )
    {
      int l = ( *bdf++ ) >> 1;
      
      for( lwin = 0; lwin < 3; lwin++ )
      {
        *mp++ = l;
        *mp++ = i + lwin;
        *mp++ = lwin;
        *mp++ = cb;
      }
      i += 6 * l;
    }
    mapend[ j ][ 0 ] = mp;
 
    mp = map[ j ][ 1 ] = mapbuf1[ j ];
    bdf = bi->shortDiff + 0;
    for( i = 0, cb = 0; cb < 13; cb++ )
    {
      int l = ( *bdf++ ) >> 1;
      
      for( lwin = 0; lwin < 3; lwin++ )
      {
        *mp++ = l;
        *mp++ = i + lwin;
        *mp++ = lwin;
        *mp++ = cb;
      }
      i += 6 * l;
    }
    mapend[ j ][ 1 ] = mp;
 
    mp = map[ j ][ 2 ] = mapbuf2[ j ];
    bdf = bi->longDiff;
    for( cb = 0; cb < 22 ; cb++ )
    {
      *mp++ = ( *bdf++ ) >> 1;
      *mp++ = cb;
    }
    mapend[ j ][ 2 ] = mp;
  }

  for( j = 0; j < 9; j++ )
  {
    for( i = 0; i < 23; i++ )
    {
      longLimit[ j ][ i ] = ( bandInfo[ j ].longIdx[ i ] - 1 + 8 ) / 18 + 1;
      if(longLimit[ j ][ i ] > (down_sample_sblimit) )
        longLimit[ j ][ i ] = down_sample_sblimit;
    }
    
    for( i = 0; i < 14; i++ )
    {
      shortLimit[ j ][ i ] = ( bandInfo[ j ].shortIdx[ i ] - 1 ) / 18 + 1;
      if(shortLimit[ j ][ i ] > (down_sample_sblimit) )
        shortLimit[ j ][ i ] = down_sample_sblimit;
    }
  }

  for( i = 0; i < 5; i++ )
  {
    for( j = 0; j < 6; j++ )
    {
      for( k = 0; k < 6; k++ )
      {
        int n = k + j * 6 + i * 36;
        i_slen2[ n ] = i | ( j << 3 ) | ( k << 6 ) | ( 3 << 12 );
      }
    }
  }
  
  for( i = 0; i < 4; i++ )
  {
    for( j = 0; j < 4; j++ )
    {
      for( k = 0; k < 4; k++ )
      {
        int n = k + j * 4 + i * 16;
        i_slen2[ n + 180 ] = i | ( j << 3 ) | ( k << 6 ) | ( 4 << 12 );
      }
    }
  }
  
  for( i = 0; i < 4; i++ )
  {
    for( j = 0; j < 3; j++ )
    {
      int n = j + i * 3;
      i_slen2[ n + 244 ] = i | ( j << 3 ) | ( 5 << 12 );
      n_slen2[ n + 500 ] = i | ( j << 3 ) | ( 2 << 12 ) | ( 1 << 15);
    }
  }

  for( i = 0; i < 5; i++ )
  {
    for( j = 0; j < 5; j++ )
    {
      for( k = 0; k < 4; k++ )
      {
        for( l = 0; l < 4; l++ )
        {
          int n = l + k * 4 + j * 16 + i * 80;
          n_slen2[ n ] = i | ( j << 3 ) | ( k << 6 ) | ( l << 9 ) | ( 0 << 12 );
        }
      }
    }
  }
  
  for( i = 0; i < 5; i++ )
  {
    for( j = 0; j < 5; j++ )
    {
      for( k = 0; k < 4; k++ )
      {
        int n = k + j * 4 + i * 20;
        n_slen2[ n + 400 ] = i | ( j << 3 ) | ( k << 6 ) | ( 1 << 12 );
      }
    }
  }
}

/*
==================
getbits
==================
*/
static unsigned int getbits( int number_of_bits )
{
  unsigned long rval;

  if( !number_of_bits )
    return 0;

  {
    rval = wordpointer[ 0 ];
    rval <<= 8;
    rval |= wordpointer[ 1 ];
    rval <<= 8;
    rval |= wordpointer[ 2 ];
    rval <<= bitindex;
    rval &= 0xffffff;

    bitindex += number_of_bits;

    rval >>= ( 24 - number_of_bits );

    wordpointer += ( bitindex >> 3 );
    bitindex &= 7;
  }
  
  return rval;
}

/*
==================
getbits_fast
==================
*/
static unsigned int getbits_fast( int number_of_bits )
{
  unsigned long rval;

  {
    rval = wordpointer[ 0 ];
    rval <<= 8; 
    rval |= wordpointer[ 1 ];
    rval <<= bitindex;
    rval &= 0xffff;
    bitindex += number_of_bits;

    rval >>= ( 16 - number_of_bits );

    wordpointer += ( bitindex >> 3 );
    bitindex &= 7;
  }
  
  return rval;
}

/*
==================
get1bit
==================
*/
static unsigned int get1bit( void )
{
  unsigned char rval;
  rval = *wordpointer << bitindex;

  bitindex++;
  wordpointer += ( bitindex >> 3 );
  bitindex &= 7;

  return rval >> 7;
}

/*
==================
III_get_side_info_1
==================
*/
static decoderError_t III_get_side_info_1( III_sideinfo_t *si, int stereo,
 int ms_stereo, long sfreq, int single)
{
   int ch, gr;
   int powdiff = ( single == 3 ) ? 4 : 0;

   si->main_data_begin = getbits( 9 );
   
   if( stereo == 1 )
     si->private_bits = getbits_fast( 5 );
   else 
     si->private_bits = getbits_fast( 3 );

   for( ch = 0; ch < stereo; ch++ )
   {
       si->ch[ch].gr[ 0 ].scfsi = -1;
       si->ch[ch].gr[ 1 ].scfsi = getbits_fast( 4 );
   }

   for( gr = 0; gr < 2; gr++ ) 
   {
     for( ch = 0; ch < stereo; ch++ ) 
     {
       register gr_info_t *gr_info = &( si->ch[ ch ].gr[ gr ] );

       gr_info->part2_3_length = getbits( 12 );
       gr_info->big_values = getbits_fast( 9 );
       
       if( gr_info->big_values > 288 )
       {
          Com_Printf( "mp3dec: big_values too large\n" );
          gr_info->big_values = 288;
       }
       
       gr_info->pow2gain = gainpow2 + 256 - getbits_fast( 8 ) + powdiff;
       
       if( ms_stereo )
         gr_info->pow2gain += 2;
         
       gr_info->scalefac_compress = getbits_fast( 4 );
       
       if( get1bit( ) ) 
       {
         int i;
         gr_info->block_type = getbits_fast( 2 );
         gr_info->mixed_block_flag = get1bit( );
         gr_info->table_select[ 0 ] = getbits_fast( 5 );
         gr_info->table_select[ 1 ] = getbits_fast( 5 );
         /*
          * table_select[2] not needed, because there is no region2,
          * but to satisfy some verifications tools we set it either.
          */
         gr_info->table_select[ 2 ] = 0;
         
         for( i = 0; i < 3; i++ )
           gr_info->full_gain[ i ] = gr_info->pow2gain + ( getbits_fast( 3 ) << 3 );

         if( gr_info->block_type == 0 )
         {
           Com_Printf( "mp3dec: Blocktype == 0 and window-switching == 1 not allowed\n" );
           return DE_ERR;
         }
         /* region_count/start parameters are implicit in this case. */       
         gr_info->region1start = 36 >> 1;
         gr_info->region2start = 576 >> 1;
       }
       else 
       {
         int i, r0c, r1c;
         
         for( i = 0; i < 3; i++ )
           gr_info->table_select[ i ] = getbits_fast( 5 );
           
         r0c = getbits_fast( 4 );
         r1c = getbits_fast( 3 );
         gr_info->region1start = bandInfo[ sfreq ].longIdx[ r0c + 1 ] >> 1 ;
         gr_info->region2start = bandInfo[ sfreq ].longIdx[ r0c + 1 + r1c + 1 ] >> 1;
         gr_info->block_type = 0;
         gr_info->mixed_block_flag = 0;
       }

       gr_info->preflag = get1bit( );
       gr_info->scalefac_scale = get1bit( );
       gr_info->count1table_select = get1bit( );
     }
   }

   return DE_OK;
}

/*
==================
III_get_side_info_2
==================
*/
static decoderError_t III_get_side_info_2( III_sideinfo_t *si, int stereo,
 int ms_stereo, long sfreq, int single)
{
  int ch;
  int powdiff = ( single == 3 ) ? 4 : 0;

  si->main_data_begin = getbits( 8 );
  if( stereo == 1 )
    si->private_bits = get1bit( );
  else 
    si->private_bits = getbits_fast( 2 );

  for(ch = 0; ch < stereo; ch++ ) 
  {
    register gr_info_t *gr_info = &( si->ch[ ch ].gr[ 0 ] );

    gr_info->part2_3_length = getbits( 12 );
    gr_info->big_values = getbits_fast( 9 );
    
    if( gr_info->big_values > 288 )
    {
      Com_Printf( "mp3dec: big_values too large\n" );
      gr_info->big_values = 288;
    }
    
    gr_info->pow2gain = gainpow2 + 256 - getbits_fast( 8 ) + powdiff;
    
    if( ms_stereo )
      gr_info->pow2gain += 2;
      
    gr_info->scalefac_compress = getbits( 9 );
    
    if( get1bit( ) ) 
    {
      int i;
      gr_info->block_type = getbits_fast( 2 );
      gr_info->mixed_block_flag = get1bit( );
      gr_info->table_select[ 0 ] = getbits_fast( 5 );
      gr_info->table_select[ 1 ] = getbits_fast( 5 );
      /*
       * table_select[2] not needed, because there is no region2,
       * but to satisfy some verifications tools we set it either.
       */
      gr_info->table_select[ 2 ] = 0;
      for( i = 0; i < 3; i++ )
        gr_info->full_gain[ i ] = gr_info->pow2gain + ( getbits_fast( 3 ) << 3 );

      if( gr_info->block_type == 0 )
      {
        Com_Printf( "mp3dec: Blocktype == 0 and window-switching == 1 not allowed\n" );
        return DE_ERR;
      }
      /* region_count/start parameters are implicit in this case. */       
      /* check this again! */
      if( gr_info->block_type == 2 )
        gr_info->region1start = 36 >> 1;
      else if( sfreq == 8 )
        gr_info->region1start = 108 >> 1;
      else
        gr_info->region1start = 54 >> 1;
        
      gr_info->region2start = 576 >> 1;
    }
    else 
    {
      int i, r0c, r1c;
      
      for( i = 0; i < 3; i++ )
        gr_info->table_select[ i ] = getbits_fast( 5 );
        
      r0c = getbits_fast( 4 );
      r1c = getbits_fast( 3 );
      gr_info->region1start = bandInfo[ sfreq ].longIdx[ r0c + 1 ] >> 1;
      gr_info->region2start = bandInfo[ sfreq ].longIdx[ r0c + 1 + r1c + 1 ] >> 1;
      gr_info->block_type = 0;
      gr_info->mixed_block_flag = 0;
    }
    gr_info->scalefac_scale = get1bit( );
    gr_info->count1table_select = get1bit( );
  }

  return DE_OK;
}

/*
==================
III_get_scale_factors_1
==================
*/
static int III_get_scale_factors_1( int *scf, gr_info_t *gr_info )
{
   static unsigned char slen[ 2 ][ 16 ] =
   {
     { 0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4 },
     { 0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3 }
   };
   int numbits;
   int num0 = slen[ 0 ][ gr_info->scalefac_compress ];
   int num1 = slen[ 1 ][ gr_info->scalefac_compress ];

    if( gr_info->block_type == 2 ) 
    {
      int i = 18;
      numbits = ( num0 + num1 ) * 18;

      if( gr_info->mixed_block_flag )
      {
         for( i = 8; i; i-- )
           *scf++ = getbits_fast( num0 );
           
         i = 9;
         numbits -= num0; /* num0 * 17 + num1 * 18 */
      }

      for( ; i; i-- )
        *scf++ = getbits_fast( num0 );
        
      for( i = 18; i; i-- )
        *scf++ = getbits_fast( num1 );
        
      *scf++ = 0; *scf++ = 0; *scf++ = 0; /* short[13][0..2] = 0 */
    }
    else 
    {
      int i;
      int scfsi = gr_info->scfsi;

      if( scfsi < 0 )
      { /* scfsi < 0 => granule == 0 */
         for( i = 11; i; i-- )
           *scf++ = getbits_fast( num0 );
           
         for( i = 10; i; i-- )
           *scf++ = getbits_fast( num1 );
           
         numbits = ( num0 + num1 ) * 10 + num0;
      }
      else
      {
        numbits = 0;
        
        if( !( scfsi & 0x8 ) )
        {
          for( i = 6; i; i-- )
            *scf++ = getbits_fast( num0 );
          numbits += num0 * 6;
        }
        else
        {
          *scf++ = 0; *scf++ = 0; *scf++ = 0;  /* set to ZERO necessary? */
          *scf++ = 0; *scf++ = 0; *scf++ = 0;
        }

        if( !( scfsi & 0x4 ) )
        {
          for( i = 5; i; i-- )
            *scf++ = getbits_fast( num0 );
          numbits += num0 * 5;
        }
        else
        {
          *scf++ = 0; *scf++ = 0; *scf++ = 0;  /* set to ZERO necessary? */
          *scf++ = 0; *scf++ = 0;
        }

        if( !( scfsi & 0x2 ) )
        {
          for( i = 5; i; i-- )
            *scf++ = getbits_fast( num1 );
          numbits += num1 * 5;
        }
        else
        {
          *scf++ = 0; *scf++ = 0; *scf++ = 0;  /* set to ZERO necessary? */
          *scf++ = 0; *scf++ = 0;
        }

        if( !( scfsi & 0x1 ) )
        {
          for( i = 5; i; i-- )
            *scf++ = getbits_fast( num1 );
          numbits += num1 * 5;
        }
        else
        {
          *scf++ = 0; *scf++ = 0; *scf++ = 0;  /* set to ZERO necessary? */
          *scf++ = 0; *scf++ = 0;
        }
      }

      *scf++ = 0;  /* no l[21] in original sources */
    }
    return numbits;
}

/*
==================
III_get_scale_factors_2
==================
*/
static int III_get_scale_factors_2( int *scf, gr_info_t *gr_info, int i_stereo )
{
  unsigned char *pnt;
  int i,j;
  unsigned int slen;
  int n = 0;
  int numbits = 0;

  static unsigned char stab[3][6][4] =
  {
    {
      { 6, 5, 5,5 } , { 6, 5, 7,3 } , { 11,10,0,0},
      { 7, 7, 7,0 } , { 6, 6, 6,3 } , {  8, 8,5,0}
    },
    {
      { 9, 9, 9,9 } , { 9, 9,12,6 } , { 18,18,0,0},
      {12,12,12,0 } , {12, 9, 9,6 } , { 15,12,9,0}
    },
    {
      { 6, 9, 9,9 } , { 6, 9,12,6 } , { 15,18,0,0},
      { 6,15,12,0 } , { 6,12, 9,6 } , {  6,18,9,0}
    }
  }; 

  if( i_stereo ) /* i_stereo AND second channel -> doLayer3() checks this */
    slen = i_slen2[ gr_info->scalefac_compress >> 1 ];
  else
    slen = n_slen2[ gr_info->scalefac_compress ];

  gr_info->preflag = ( slen >> 15 ) & 0x1;

  n = 0;  
  if( gr_info->block_type == 2 )
  {
    n++;
    if( gr_info->mixed_block_flag )
      n++;
  }

  pnt = stab[ n ][ ( slen >> 12 ) & 0x7 ];

  for( i = 0; i < 4; i++ )
  {
    int num = slen & 0x7;
    slen >>= 3;
    if( num )
    {
      for( j = 0; j < (int)( pnt[ i ] ); j++ )
        *scf++ = getbits_fast( num );
        
      numbits += pnt[ i ] * num;
    }
    else
    {
      for( j = 0; j < (int)( pnt[ i ] ); j++ )
        *scf++ = 0;
    }
  }
  
  n = ( n << 1 ) + 1;
  for( i = 0; i < n; i++ )
    *scf++ = 0;

  return numbits;
}

static int pretab1[22] = {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,3,3,3,2,0};
static int pretab2[22] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/*
==================
III_dequantize_sample
==================
*/
static decoderError_t III_dequantize_sample( float xr[ SBLIMIT ][ SSLIMIT ], int *scf,
   gr_info_t *gr_info, int sfreq, int part2bits )
{
  int shift = 1 + gr_info->scalefac_scale;
  float *xrpnt = (float *)xr;
  int l[ 3 ], l3;
  int part2remain = gr_info->part2_3_length - part2bits;
  int *me;

  {
    int bv       = gr_info->big_values;
    int region1  = gr_info->region1start;
    int region2  = gr_info->region2start;

    l3 = ( ( 576 >> 1 ) - bv ) >> 1;   
/*
 * we may lose the 'odd' bit here !! 
 * check this later again 
 */
    if( bv <= region1 )
    {
      l[ 0 ] = bv;
      l[ 1 ] = 0;
      l[ 2 ] = 0;
    }
    else
    {
      l[ 0 ] = region1;
      
      if( bv <= region2 )
      {
        l[ 1 ] = bv - l[ 0 ];
        l[ 2 ] = 0;
      }
      else
      {
        l[ 1 ] = region2 - l[ 0 ];
        l[ 2 ] = bv - region2;
      }
    }
  }
 
  if( gr_info->block_type == 2 )
  {
    /*
     * decoding with short or mixed mode BandIndex table 
     */
    int i, max[ 4 ];
    int step = 0, lwin = 0, cb = 0;
    register float v = 0.0;
    register int *m, mc;

    if( gr_info->mixed_block_flag )
    {
      max[ 3 ] = -1;
      max[ 0 ] = max[ 1 ] = max[ 2 ] = 2;
      m = map[ sfreq ][ 0 ];
      me = mapend[ sfreq ][ 0 ];
    }
    else
    {
      max[ 0 ] = max[ 1 ] = max[ 2 ] = max[ 3 ] = -1;
      /* max[3] not floatly needed in this case */
      m = map[ sfreq ][ 1 ];
      me = mapend[ sfreq ][ 1 ];
    }

    mc = 0;
    for( i = 0; i < 2; i++ )
    {
      int lp = l[ i ];
      newHuff_t *h = ht + gr_info->table_select[ i ];
      
      for( ; lp; lp--, mc--)
      {
        register int x, y;
        if( !mc )
        {
          mc = *m++;
          xrpnt = ( (float *)xr ) + ( *m++ );
          lwin = *m++;
          cb = *m++;
          
          if( lwin == 3 )
          {
            v = gr_info->pow2gain[ (*scf++) << shift ];
            step = 1;
          }
          else
          {
            v = gr_info->full_gain[ lwin ][ (*scf++) << shift ];
            step = 3;
          }
        }
        
        {
          register int *val = h->table;
          while( ( y = *val++ ) < 0 )
          {
            if ( get1bit( ) )
              val -= y;
              
            part2remain--;
          }
          
          x = y >> 4;
          y &= 0xf;
        }
        
        if( x == 15 )
        {
          max[ lwin ] = cb;
          part2remain -= h->linbits + 1;
          x += getbits( h->linbits );
          if( get1bit() )
            *xrpnt = -ispow[ x ] * v;
          else
            *xrpnt =  ispow[ x ] * v;
        }
        else if( x )
        {
          max[ lwin ] = cb;
          if( get1bit() )
            *xrpnt = -ispow[ x ] * v;
          else
            *xrpnt =  ispow[ x ] * v;
            
          part2remain--;
        }
        else
          *xrpnt = 0.0;
          
        xrpnt += step;
        
        if( y == 15 )
        {
          max[ lwin ] = cb;
          part2remain -= h->linbits + 1;
          y += getbits( h->linbits );
          
          if( get1bit( ) )
            *xrpnt = -ispow[ y ] * v;
          else
            *xrpnt =  ispow[ y ] * v;
        }
        else if( y )
        {
          max[ lwin ] = cb;
          
          if( get1bit( ) )
            *xrpnt = -ispow[ y ] * v;
          else
            *xrpnt =  ispow[ y ] * v;
            
          part2remain--;
        }
        else
          *xrpnt = 0.0;
          
        xrpnt += step;
      }
    }
    
    for( ; l3 && ( part2remain > 0 ); l3-- )
    {
      newHuff_t *h = htc + gr_info->count1table_select;
      register int *val = h->table, a;

      while( ( a = *val++ ) < 0 )
      {
        part2remain--;
        
        if( part2remain < 0)
        {
          part2remain++;
          a = 0;
          break;
        }
        
        if( get1bit( ) )
          val -= a;
      }

      for( i = 0; i < 4; i++ )
      {
        if( !( i & 1 ) )
        {
          if( !mc )
          {
            mc = *m++;
            xrpnt = ( (float *)xr ) + ( *m++ );
            lwin = *m++;
            cb = *m++;
            
            if( lwin == 3 )
            {
              v = gr_info->pow2gain[ (*scf++) << shift ];
              step = 1;
            }
            else
            {
              v = gr_info->full_gain[ lwin ][ (*scf++) << shift ];
              step = 3;
            }
          }
          mc--;
        }
        
        if( ( a & ( 0x8 >> i )) )
        {
          max[ lwin ] = cb;
          part2remain--;
          
          if( part2remain < 0 )
          {
            part2remain++;
            break;
          }
          
          if( get1bit( ) ) 
            *xrpnt = -v;
          else
            *xrpnt = v;
        }
        else
          *xrpnt = 0.0;
          
        xrpnt += step;
      }
    }
 
    while( m < me )
    {
      if( !mc )
      {
        mc = *m++;
        xrpnt = ( (float *)xr ) + *m++;
        
        if( ( *m++ ) == 3 )
          step = 1;
        else
          step = 3;
        m++; /* cb */
      }
      
      mc--;
      *xrpnt = 0.0;
      xrpnt += step;
      *xrpnt = 0.0;
      xrpnt += step;
/* we could add a little opt. here:
 * if we finished a band for window 3 or a long band
 * further bands could copied in a simple loop without a
 * special 'map' decoding
 */
    }

    gr_info->maxband[ 0 ] = max[ 0 ] + 1;
    gr_info->maxband[ 1 ] = max[ 1 ] + 1;
    gr_info->maxband[ 2 ] = max[ 2 ] + 1;
    gr_info->maxbandl = max[ 3 ] + 1;

    {
      int rmax = max[ 0 ] > max[ 1 ] ? max[ 0 ] : max[ 1 ];
      rmax = ( rmax > max[ 2 ] ? rmax : max[ 2 ] ) + 1;
      gr_info->maxb = rmax ? shortLimit[ sfreq ][ rmax ] : longLimit[ sfreq ][ max[ 3 ] + 1 ];
    }

  }
  else
  {
  /*
     * decoding with 'long' BandIndex table (block_type != 2)
     */
    int *pretab = gr_info->preflag ? pretab1 : pretab2;
    int i, max = -1;
    int cb = 0;
    register int *m = map[ sfreq ][ 2 ];
    register float v = 0.0;
    register int mc = 0;

    /*
     * long hash table values
     */
    for( i = 0; i < 3; i++ )
    {
      int lp = l[ i ];
      newHuff_t *h = ht + gr_info->table_select[ i ];

      for( ; lp; lp--, mc--)
      {
        int x, y;

        if( !mc )
        {
          mc = *m++;
          v = gr_info->pow2gain[ ( (*scf++) + (*pretab++) ) << shift ];
          cb = *m++;
        }
        
        {
          register int *val = h->table;
          
          while( ( y = *val++ ) < 0 )
          {
            if ( get1bit( ) )
              val -= y;
              
            part2remain--;
          }
          
          x = y >> 4;
          y &= 0xf;
        }
        
        if( x == 15 )
        {
          max = cb;
          part2remain -= h->linbits + 1;
          x += getbits( h->linbits );
          
          if( get1bit( ) )
            *xrpnt++ = -ispow[ x ] * v;
          else
            *xrpnt++ =  ispow[ x ] * v;
        }
        else if( x )
        {
          max = cb;
          if( get1bit( ) )
            *xrpnt++ = -ispow[ x ] * v;
          else
            *xrpnt++ =  ispow[ x ] * v;
            
          part2remain--;
        }
        else
          *xrpnt++ = 0.0;

        if( y == 15 )
        {
          max = cb;
          part2remain -= h->linbits + 1;
          
          y += getbits( h->linbits );
          
          if( get1bit( ) )
            *xrpnt++ = -ispow[ y ] * v;
          else
            *xrpnt++ =  ispow[ y ] * v;
        }
        else if( y )
        {
          max = cb;
          if( get1bit( ) )
            *xrpnt++ = -ispow[ y ] * v;
          else
            *xrpnt++ =  ispow[ y ] * v;
            
          part2remain--;
        }
        else
          *xrpnt++ = 0.0;
      }
    }

    /*
     * short (count1table) values
     */
    for( ; l3 && ( part2remain > 0 ); l3-- )
    {
      newHuff_t *h = htc + gr_info->count1table_select;
      register int *val = h->table, a;

      while( ( a = *val++ ) < 0 )
      {
        part2remain--;
        
        if( part2remain < 0 )
        {
          part2remain++;
          a = 0;
          break;
        }
        if( get1bit( ) )
          val -= a;
      }

      for( i = 0; i < 4; i++ )
      {
        if( !( i & 1 ) )
        {
          if( !mc )
          {
            mc = *m++;
            cb = *m++;
            v = gr_info->pow2gain[ ( ( *scf++ ) + ( *pretab++ ) ) << shift ];
          }
          
          mc--;
        }
        if( ( a & ( 0x8 >> i ) ) )
        {
          max = cb;
          part2remain--;
          
          if( part2remain < 0 )
          {
            part2remain++;
            break;
          }
          
          if( get1bit( ) )
            *xrpnt++ = -v;
          else
            *xrpnt++ = v;
        }
        else
          *xrpnt++ = 0.0;
      }
    }

    /* 
     * zero part
     */
    for( i = ( &xr[ SBLIMIT ][ 0 ] - xrpnt ) >> 1; i; i-- )
    {
      *xrpnt++ = 0.0;
      *xrpnt++ = 0.0;
    }

    gr_info->maxbandl = max + 1;
    gr_info->maxb = longLimit[ sfreq ][ gr_info->maxbandl ];
  }

  while( part2remain > 16 )
  {
    getbits( 16 ); /* Dismiss stuffing Bits */
    part2remain -= 16;
  }
  
  if( part2remain > 0 )
    getbits( part2remain );
  else if( part2remain < 0 )
  {
    Com_Printf( "mp3dec: can't rewind stream by %d bits\n", -part2remain );
    return DE_ERR; /* -> error */
  }
  return DE_OK;
}

/*
==================
III_i_stereo
==================
*/
static void III_i_stereo( float xr_buf[ 2 ][ SBLIMIT ][ SSLIMIT ], int *scalefac,
   gr_info_t *gr_info, int sfreq, int ms_stereo, int lsf )
{
      float (*xr)[ SBLIMIT * SSLIMIT ] = ( float (*)[ SBLIMIT * SSLIMIT ] )xr_buf;
      bandInfo_t *bi = &bandInfo[ sfreq ];
      float *tab1, *tab2;

      if( lsf )
      {
        int p = gr_info->scalefac_compress & 0x1;
        
        if( ms_stereo )
        {
          tab1 = pow1_2[ p ];
          tab2 = pow2_2[ p ];
        }
        else
        {
          tab1 = pow1_1[ p ];
          tab2 = pow2_1[ p ];
        }
      }
      else
      {
        if( ms_stereo )
        {
          tab1 = tan1_2;
          tab2 = tan2_2;
        }
        else
        {
          tab1 = tan1_1;
          tab2 = tan2_1;
        }
      }

      if( gr_info->block_type == 2 )
      {
         int lwin, do_l = 0;
         if( gr_info->mixed_block_flag )
           do_l = 1;

         for( lwin = 0; lwin < 3; lwin++ ) /* process each window */
         {
           /* get first band with zero values */
           int is_p, sb, idx, sfb = gr_info->maxband[ lwin ];  /* sfb is minimal 3 for mixed mode */
           if( sfb > 3 )
             do_l = 0;

           for( ; sfb<12; sfb++ )
           {
             is_p = scalefac[ sfb * 3 + lwin - gr_info->mixed_block_flag ]; /* scale: 0-15 */ 
             if( is_p != 7 )
             {
               float t1, t2;
               sb = bi->shortDiff[ sfb ];
               idx = bi->shortIdx[ sfb ] + lwin;
               t1 = tab1[ is_p ]; t2 = tab2[ is_p ];
               
               for( ; sb > 0; sb--, idx += 3 )
               {
                 float v = xr[ 0 ][ idx ];
                 xr[ 0 ][ idx ] = v * t1;
                 xr[ 1 ][ idx ] = v * t2;
               }
             }
           }

           is_p = scalefac[ 11 * 3 + lwin - gr_info->mixed_block_flag ]; /* scale: 0-15 */
           sb = bi->shortDiff[ 12 ];
           idx = bi->shortIdx[ 12 ] + lwin;
           
           if( is_p != 7 )
           {
             float t1, t2;
             t1 = tab1[ is_p ]; t2 = tab2[ is_p ];
             for ( ; sb > 0; sb--, idx += 3 )
             {  
               float v = xr[ 0 ][ idx ];
               xr[ 0 ][ idx ] = v * t1;
               xr[ 1 ][ idx ] = v * t2;
             }
           }
         } /* end for(lwin; .. ; . ) */

         if( do_l )
         {
/* also check l-part, if ALL bands in the three windows are 'empty'
 * and mode = mixed_mode 
 */
           int sfb = gr_info->maxbandl;
           int idx = bi->longIdx[ sfb ];

           for( ; sfb < 8; sfb++ )
           {
             int sb = bi->longDiff[ sfb ];
             int is_p = scalefac[ sfb ]; /* scale: 0-15 */
             if( is_p != 7 )
             {
               float t1, t2;
               t1 = tab1[ is_p ]; t2 = tab2[ is_p ];
               
               for( ; sb > 0; sb--, idx++ )
               {
                 float v = xr[ 0 ][ idx ];
                 xr[ 0 ][ idx ] = v * t1;
                 xr[ 1 ][ idx ] = v * t2;
               }
             }
             else 
               idx += sb;
           }
         }     
      } 
      else /* ((gr_info->block_type != 2)) */
      {
        int sfb = gr_info->maxbandl;
        int is_p, idx = bi->longIdx[ sfb ];
        for( ; sfb < 21; sfb++ )
        {
          int sb = bi->longDiff[ sfb ];
          is_p = scalefac[ sfb ]; /* scale: 0-15 */
          
          if( is_p != 7 )
          {
            float t1, t2;
            t1 = tab1[ is_p ]; t2 = tab2[ is_p ];
            
            for( ; sb > 0; sb--, idx++ )
            {
               float v = xr[ 0 ][ idx ];
               xr[ 0 ][ idx ] = v * t1;
               xr[ 1 ][ idx ] = v * t2;
            }
          }
          else
            idx += sb;
        }

        is_p = scalefac[ 20 ]; /* copy l-band 20 to l-band 21 */
        if( is_p != 7 )
        {
          int sb;
          float t1 = tab1[ is_p ],t2 = tab2[ is_p ]; 

          for( sb = bi->longDiff[ 21 ]; sb > 0; sb--, idx++ )
          {
            float v = xr[ 0 ][ idx ];
            xr[ 0 ][ idx ] = v * t1;
            xr[ 1 ][ idx ] = v * t2;
          }
        }
      } /* ... */
}

/*
==================
III_antialias
==================
*/
static void III_antialias(float xr[SBLIMIT][SSLIMIT],gr_info_t *gr_info)
{
   int sblim;

   if(gr_info->block_type == 2)
   {
      if(!gr_info->mixed_block_flag) 
        return;
      sblim = 1; 
   }
   else {
     sblim = gr_info->maxb-1;
   }

   /* 31 alias-reduction operations between each pair of sub-bands */
   /* with 8 butterflies between each pair                         */

   {
     int sb;
     float *xr1=(float *) xr[1];

     for(sb=sblim;sb;sb--,xr1+=10)
     {
       int ss;
       float *cs=aa_cs,*ca=aa_ca;
       float *xr2 = xr1;

       for(ss=7;ss>=0;ss--)
       {       /* upper and lower butterfly inputs */
         register float bu = *--xr2,bd = *xr1;
         *xr2   = (bu * (*cs)   ) - (bd * (*ca)   );
         *xr1++ = (bd * (*cs++) ) + (bu * (*ca++) );
       }
     }
  }
}

/*
 DCT insipired by Jeff Tsay's DCT from the maplay package
 this is an optimized version with manual unroll.

 References:
 [1] S. Winograd: "On Computing the Discrete Fourier Transform",
     Mathematics of Computation, Volume 32, Number 141, January 1978,
     Pages 175-199
*/

/*
==================
dct64_1
==================
*/
static void dct64_1( float *out0, float *out1, float *b1, float *b2, float *samples )
{

 {
  register float *costab = pnts[ 0 ];

  b1[ 0x00 ] = samples[ 0x00 ] + samples[ 0x1F ];
  b1[ 0x1F ] = ( samples[ 0x00 ] - samples[ 0x1F ] ) * costab[ 0x0 ];

  b1[ 0x01 ] = samples[ 0x01 ] + samples[ 0x1E ];
  b1[ 0x1E ] = ( samples[ 0x01 ] - samples[ 0x1E ] ) * costab[ 0x1 ];

  b1[ 0x02 ] = samples[ 0x02 ] + samples[ 0x1D ];
  b1[ 0x1D ] = ( samples[ 0x02 ] - samples[ 0x1D ] ) * costab[ 0x2 ];

  b1[ 0x03 ] = samples[ 0x03 ] + samples[ 0x1C ];
  b1[ 0x1C ] = ( samples[ 0x03 ] - samples[ 0x1C ] ) * costab[ 0x3 ];

  b1[ 0x04 ] = samples[ 0x04 ] + samples[ 0x1B ];
  b1[ 0x1B ] = ( samples[ 0x04 ] - samples[ 0x1B ] ) * costab[ 0x4 ];

  b1[ 0x05 ] = samples[ 0x05 ] + samples[ 0x1A ];
  b1[ 0x1A ] = ( samples[ 0x05 ] - samples[ 0x1A ] ) * costab[ 0x5 ];

  b1[ 0x06 ] = samples[ 0x06 ] + samples[ 0x19 ];
  b1[ 0x19 ] = ( samples[ 0x06 ] - samples[ 0x19 ] ) * costab[ 0x6 ];

  b1[ 0x07 ] = samples[ 0x07 ] + samples[ 0x18 ];
  b1[ 0x18 ] = ( samples[ 0x07 ] - samples[ 0x18 ] ) * costab[ 0x7 ];

  b1[ 0x08 ] = samples[ 0x08 ] + samples[ 0x17 ];
  b1[ 0x17 ] = ( samples[ 0x08 ] - samples[ 0x17 ] ) * costab[ 0x8 ];

  b1[ 0x09 ] = samples[ 0x09 ] + samples[ 0x16 ];
  b1[ 0x16 ] = ( samples[ 0x09 ] - samples[ 0x16 ] ) * costab[ 0x9 ];

  b1[ 0x0A ] = samples[ 0x0A ] + samples[ 0x15 ];
  b1[ 0x15 ] = ( samples[ 0x0A ] - samples[ 0x15 ] ) * costab[ 0xA ];

  b1[ 0x0B ] = samples[ 0x0B ] + samples[ 0x14 ];
  b1[ 0x14 ] = ( samples[ 0x0B ] - samples[ 0x14 ] ) * costab[ 0xB ];

  b1[ 0x0C ] = samples[ 0x0C ] + samples[ 0x13 ];
  b1[ 0x13 ] = ( samples[ 0x0C ] - samples[ 0x13 ] ) * costab[ 0xC ];

  b1[ 0x0D ] = samples[ 0x0D ] + samples[ 0x12 ];
  b1[ 0x12 ] = ( samples[ 0x0D ] - samples[ 0x12 ] ) * costab[ 0xD ];

  b1[ 0x0E ] = samples[ 0x0E ] + samples[ 0x11 ];
  b1[ 0x11 ] = ( samples[ 0x0E ] - samples[ 0x11 ] ) * costab[ 0xE ];

  b1[ 0x0F ] = samples[ 0x0F ] + samples[ 0x10 ];
  b1[ 0x10 ] = ( samples[ 0x0F ] - samples[ 0x10 ] ) * costab[ 0xF ];
 }


 {
  register float *costab = pnts[ 1 ];

  b2[ 0x00 ] = b1[ 0x00 ] + b1[ 0x0F ]; 
  b2[ 0x0F ] = ( b1[ 0x00 ] - b1[ 0x0F ] ) * costab[ 0 ];
  b2[ 0x01 ] = b1[ 0x01 ] + b1[ 0x0E ]; 
  b2[ 0x0E ] = ( b1[ 0x01 ] - b1[ 0x0E ] ) * costab[ 1 ];
  b2[ 0x02 ] = b1[ 0x02 ] + b1[ 0x0D ]; 
  b2[ 0x0D ] = ( b1[ 0x02 ] - b1[ 0x0D ] ) * costab[ 2 ];
  b2[ 0x03 ] = b1[ 0x03 ] + b1[ 0x0C ]; 
  b2[ 0x0C ] = ( b1[ 0x03 ] - b1[ 0x0C ] ) * costab[ 3 ];
  b2[ 0x04 ] = b1[ 0x04 ] + b1[ 0x0B ]; 
  b2[ 0x0B ] = ( b1[ 0x04 ] - b1[ 0x0B ] ) * costab[ 4 ];
  b2[ 0x05 ] = b1[ 0x05 ] + b1[ 0x0A ]; 
  b2[ 0x0A ] = ( b1[ 0x05 ] - b1[ 0x0A ] ) * costab[ 5 ];
  b2[ 0x06 ] = b1[ 0x06 ] + b1[ 0x09 ]; 
  b2[ 0x09 ] = ( b1[ 0x06 ] - b1[ 0x09 ] ) * costab[ 6 ];
  b2[ 0x07 ] = b1[ 0x07 ] + b1[ 0x08 ]; 
  b2[ 0x08 ] = ( b1[ 0x07 ] - b1[ 0x08 ] ) * costab[ 7 ];

  b2[ 0x10 ] = b1[ 0x10 ] + b1[ 0x1F ];
  b2[ 0x1F ] = ( b1[ 0x1F ] - b1[ 0x10 ] ) * costab[ 0 ];
  b2[ 0x11 ] = b1[ 0x11 ] + b1[ 0x1E ];
  b2[ 0x1E ] = ( b1[ 0x1E ] - b1[ 0x11 ] ) * costab[ 1 ];
  b2[ 0x12 ] = b1[ 0x12 ] + b1[ 0x1D ];
  b2[ 0x1D ] = ( b1[ 0x1D ] - b1[ 0x12 ] ) * costab[ 2 ];
  b2[ 0x13 ] = b1[ 0x13 ] + b1[ 0x1C ];
  b2[ 0x1C ] = ( b1[ 0x1C ] - b1[ 0x13 ] ) * costab[ 3 ];
  b2[ 0x14 ] = b1[ 0x14 ] + b1[ 0x1B ];
  b2[ 0x1B ] = ( b1[ 0x1B ] - b1[ 0x14 ] ) * costab[ 4 ];
  b2[ 0x15 ] = b1[ 0x15 ] + b1[ 0x1A ];
  b2[ 0x1A ] = ( b1[ 0x1A ] - b1[ 0x15 ] ) * costab[ 5 ];
  b2[ 0x16 ] = b1[ 0x16 ] + b1[ 0x19 ];
  b2[ 0x19 ] = ( b1[ 0x19 ] - b1[ 0x16 ] ) * costab[ 6 ];
  b2[ 0x17 ] = b1[ 0x17 ] + b1[ 0x18 ];
  b2[ 0x18 ] = ( b1[ 0x18 ] - b1[ 0x17 ] ) * costab[ 7 ];
 }

 {
  register float *costab = pnts[ 2 ];

  b1[ 0x00 ] = b2[ 0x00 ] + b2[ 0x07 ];
  b1[ 0x07 ] = ( b2[ 0x00 ] - b2[ 0x07 ] ) * costab[ 0 ];
  b1[ 0x01 ] = b2[ 0x01 ] + b2[ 0x06 ];
  b1[ 0x06 ] = ( b2[ 0x01 ] - b2[ 0x06 ] ) * costab[ 1 ];
  b1[ 0x02 ] = b2[ 0x02 ] + b2[ 0x05 ];
  b1[ 0x05 ] = ( b2[ 0x02 ] - b2[ 0x05 ] ) * costab[ 2 ];
  b1[ 0x03 ] = b2[ 0x03 ] + b2[ 0x04 ];
  b1[ 0x04 ] = ( b2[ 0x03 ] - b2[ 0x04 ] ) * costab[ 3 ];

  b1[ 0x08 ] = b2[ 0x08 ] + b2[ 0x0F ];
  b1[ 0x0F ] = ( b2[ 0x0F ] - b2[ 0x08 ] ) * costab[ 0 ];
  b1[ 0x09 ] = b2[ 0x09 ] + b2[ 0x0E ];
  b1[ 0x0E ] = ( b2[ 0x0E ] - b2[ 0x09 ] ) * costab[ 1 ];
  b1[ 0x0A ] = b2[ 0x0A ] + b2[ 0x0D ];
  b1[ 0x0D ] = ( b2[ 0x0D ] - b2[ 0x0A ] ) * costab[ 2 ];
  b1[ 0x0B ] = b2[ 0x0B ] + b2[ 0x0C ];
  b1[ 0x0C ] = ( b2[ 0x0C ] - b2[ 0x0B ] ) * costab[ 3 ];

  b1[ 0x10 ] = b2[ 0x10 ] + b2[ 0x17 ];
  b1[ 0x17 ] = ( b2[ 0x10 ] - b2[ 0x17 ] ) * costab[ 0 ];
  b1[ 0x11 ] = b2[ 0x11 ] + b2[ 0x16 ];
  b1[ 0x16 ] = ( b2[ 0x11 ] - b2[ 0x16 ] ) * costab[ 1 ];
  b1[ 0x12 ] = b2[ 0x12 ] + b2[ 0x15 ];
  b1[ 0x15 ] = ( b2[ 0x12 ] - b2[ 0x15 ] ) * costab[ 2 ];
  b1[ 0x13 ] = b2[ 0x13 ] + b2[ 0x14 ];
  b1[ 0x14 ] = ( b2[ 0x13 ] - b2[ 0x14 ] ) * costab[ 3 ];

  b1[ 0x18 ] = b2[ 0x18 ] + b2[ 0x1F ];
  b1[ 0x1F ] = ( b2[ 0x1F ] - b2[ 0x18 ] ) * costab[ 0 ];
  b1[ 0x19 ] = b2[ 0x19 ] + b2[ 0x1E ];
  b1[ 0x1E ] = ( b2[ 0x1E ] - b2[ 0x19 ] ) * costab[ 1 ];
  b1[ 0x1A ] = b2[ 0x1A ] + b2[ 0x1D ];
  b1[ 0x1D ] = ( b2[ 0x1D ] - b2[ 0x1A ] ) * costab[ 2 ];
  b1[ 0x1B ] = b2[ 0x1B ] + b2[ 0x1C ];
  b1[ 0x1C ] = ( b2[ 0x1C ] - b2[ 0x1B ] ) * costab[ 3 ];
 }

 {
  register float const cos0 = pnts[ 3 ][ 0 ];
  register float const cos1 = pnts[ 3 ][ 1 ];

  b2[ 0x00 ] = b1[ 0x00 ] + b1[ 0x03 ];
  b2[ 0x03 ] = ( b1[ 0x00 ] - b1[ 0x03 ] ) * cos0;
  b2[ 0x01 ] = b1[ 0x01 ] + b1[ 0x02 ];
  b2[ 0x02 ] = ( b1[ 0x01 ] - b1[ 0x02 ] ) * cos1;

  b2[ 0x04 ] = b1[ 0x04 ] + b1[ 0x07 ];
  b2[ 0x07 ] = ( b1[ 0x07 ] - b1[ 0x04 ] ) * cos0;
  b2[ 0x05 ] = b1[ 0x05 ] + b1[ 0x06 ];
  b2[ 0x06 ] = ( b1[ 0x06 ] - b1[ 0x05 ] ) * cos1;

  b2[ 0x08 ] = b1[ 0x08 ] + b1[ 0x0B ];
  b2[ 0x0B ] = ( b1[ 0x08 ] - b1[ 0x0B ] ) * cos0;
  b2[ 0x09 ] = b1[ 0x09 ] + b1[ 0x0A ];
  b2[ 0x0A ] = ( b1[ 0x09 ] - b1[ 0x0A ] ) * cos1;
  
  b2[ 0x0C ] = b1[ 0x0C ] + b1[ 0x0F ];
  b2[ 0x0F ] = ( b1[ 0x0F ] - b1[ 0x0C ] ) * cos0;
  b2[ 0x0D ] = b1[ 0x0D ] + b1[ 0x0E ];
  b2[ 0x0E ] = ( b1[ 0x0E ] - b1[ 0x0D ] ) * cos1;

  b2[ 0x10 ] = b1[ 0x10 ] + b1[ 0x13 ];
  b2[ 0x13 ] = ( b1[ 0x10 ] - b1[ 0x13 ] ) * cos0;
  b2[ 0x11 ] = b1[ 0x11 ] + b1[ 0x12 ];
  b2[ 0x12 ] = ( b1[ 0x11 ] - b1[ 0x12 ] ) * cos1;

  b2[ 0x14 ] = b1[ 0x14 ] + b1[ 0x17 ];
  b2[ 0x17 ] = ( b1[ 0x17 ] - b1[ 0x14 ] ) * cos0;
  b2[ 0x15 ] = b1[ 0x15 ] + b1[ 0x16 ];
  b2[ 0x16 ] = ( b1[ 0x16 ] - b1[ 0x15 ] ) * cos1;

  b2[ 0x18 ] = b1[ 0x18 ] + b1[ 0x1B ];
  b2[ 0x1B ] = ( b1[ 0x18 ] - b1[ 0x1B ] ) * cos0;
  b2[ 0x19 ] = b1[ 0x19 ] + b1[ 0x1A ];
  b2[ 0x1A ] = ( b1[ 0x19 ] - b1[ 0x1A ] ) * cos1;

  b2[ 0x1C ] = b1[ 0x1C ] + b1[ 0x1F ];
  b2[ 0x1F ] = ( b1[ 0x1F ] - b1[ 0x1C ] ) * cos0;
  b2[ 0x1D ] = b1[ 0x1D ] + b1[ 0x1E ];
  b2[ 0x1E ] = ( b1[ 0x1E ] - b1[ 0x1D ] ) * cos1;
 }

 {
  register float const cos0 = pnts[ 4 ][ 0 ];

  b1[ 0x00 ] = b2[ 0x00 ] + b2[ 0x01 ];
  b1[ 0x01 ] = ( b2[ 0x00 ] - b2[ 0x01 ] ) * cos0;
  b1[ 0x02 ] = b2[ 0x02 ] + b2[ 0x03 ];
  b1[ 0x03 ] = ( b2[ 0x03 ] - b2[ 0x02 ] ) * cos0;
  b1[ 0x02 ] += b1[ 0x03 ];

  b1[ 0x04 ] = b2[ 0x04 ] + b2[ 0x05 ];
  b1[ 0x05 ] = ( b2[ 0x04 ] - b2[ 0x05 ] ) * cos0;
  b1[ 0x06 ] = b2[ 0x06 ] + b2[ 0x07 ];
  b1[ 0x07 ] = ( b2[ 0x07 ] - b2[ 0x06 ] ) * cos0;
  b1[ 0x06 ] += b1[ 0x07 ];
  b1[ 0x04 ] += b1[ 0x06 ];
  b1[ 0x06 ] += b1[ 0x05 ];
  b1[ 0x05 ] += b1[ 0x07 ];

  b1[ 0x08 ] = b2[ 0x08 ] + b2[ 0x09 ];
  b1[ 0x09 ] = ( b2[ 0x08 ] - b2[ 0x09 ] ) * cos0;
  b1[ 0x0A ] = b2[ 0x0A ] + b2[ 0x0B ];
  b1[ 0x0B ] = ( b2[ 0x0B ] - b2[ 0x0A ] ) * cos0;
  b1[ 0x0A ] += b1[ 0x0B ];

  b1[ 0x0C ] = b2[ 0x0C ] + b2[ 0x0D ];
  b1[ 0x0D ] = ( b2[ 0x0C ] - b2[ 0x0D ] ) * cos0;
  b1[ 0x0E ] = b2[ 0x0E ] + b2[ 0x0F ];
  b1[ 0x0F ] = ( b2[ 0x0F ] - b2[ 0x0E ] ) * cos0;
  b1[ 0x0E ] += b1[ 0x0F ];
  b1[ 0x0C ] += b1[ 0x0E ];
  b1[ 0x0E ] += b1[ 0x0D ];
  b1[ 0x0D ] += b1[ 0x0F ];

  b1[ 0x10 ] = b2[ 0x10 ] + b2[ 0x11 ];
  b1[ 0x11 ] = ( b2[ 0x10 ] - b2[ 0x11 ] ) * cos0;
  b1[ 0x12 ] = b2[ 0x12 ] + b2[ 0x13 ];
  b1[ 0x13 ] = ( b2[ 0x13 ] - b2[ 0x12 ] ) * cos0;
  b1[ 0x12 ] += b1[ 0x13 ];

  b1[ 0x14 ] = b2[ 0x14 ] + b2[ 0x15 ];
  b1[ 0x15 ] = ( b2[ 0x14 ] - b2[ 0x15 ] ) * cos0;
  b1[ 0x16 ] = b2[ 0x16 ] + b2[ 0x17 ];
  b1[ 0x17 ] = ( b2[ 0x17 ] - b2[ 0x16 ] ) * cos0;
  b1[ 0x16 ] += b1[ 0x17 ];
  b1[ 0x14 ] += b1[ 0x16 ];
  b1[ 0x16 ] += b1[ 0x15 ];
  b1[ 0x15 ] += b1[ 0x17 ];

  b1[ 0x18 ] = b2[ 0x18 ] + b2[ 0x19 ];
  b1[ 0x19 ] = ( b2[ 0x18 ] - b2[ 0x19 ] ) * cos0;
  b1[ 0x1A ] = b2[ 0x1A ] + b2[ 0x1B ];
  b1[ 0x1B ] = ( b2[ 0x1B ] - b2[ 0x1A ] ) * cos0;
  b1[ 0x1A ] += b1[ 0x1B ];

  b1[ 0x1C ] = b2[ 0x1C ] + b2[ 0x1D ];
  b1[ 0x1D ] = ( b2[ 0x1C ] - b2[ 0x1D ] ) * cos0;
  b1[ 0x1E ] = b2[ 0x1E ] + b2[ 0x1F ];
  b1[ 0x1F ] = ( b2[ 0x1F ] - b2[ 0x1E ] ) * cos0;
  b1[ 0x1E ] += b1[ 0x1F ];
  b1[ 0x1C ] += b1[ 0x1E ];
  b1[ 0x1E ] += b1[ 0x1D ];
  b1[ 0x1D ] += b1[ 0x1F ];
 }

 out0[ 0x10 * 16 ] = b1[ 0x00 ];
 out0[ 0x10 * 12 ] = b1[ 0x04 ];
 out0[ 0x10 *  8 ] = b1[ 0x02 ];
 out0[ 0x10 *  4 ] = b1[ 0x06 ];
 out0[ 0x10 *  0 ] = b1[ 0x01 ];
 out1[ 0x10 *  0 ] = b1[ 0x01 ];
 out1[ 0x10 *  4 ] = b1[ 0x05 ];
 out1[ 0x10 *  8 ] = b1[ 0x03 ];
 out1[ 0x10 * 12 ] = b1[ 0x07 ];

 b1[ 0x08 ] += b1[ 0x0C ];
 out0[ 0x10 * 14 ] = b1[ 0x08 ];
 b1[ 0x0C ] += b1[ 0x0a ];
 out0[ 0x10 * 10 ] = b1[ 0x0C ];
 b1[ 0x0A ] += b1[ 0x0E ];
 out0[ 0x10 *  6 ] = b1[ 0x0A ];
 b1[ 0x0E ] += b1[ 0x09 ];
 out0[ 0x10 *  2 ] = b1[ 0x0E ];
 b1[ 0x09 ] += b1[ 0x0D ];
 out1[ 0x10 *  2 ] = b1[ 0x09 ];
 b1[ 0x0D ] += b1[ 0x0B ];
 out1[ 0x10 *  6 ] = b1[ 0x0D ];
 b1[ 0x0B ] += b1[ 0x0F ];
 out1[ 0x10 * 10 ] = b1[ 0x0B ];
 out1[ 0x10 * 14 ] = b1[ 0x0F ];

 b1[ 0x18 ] += b1[ 0x1C ];
 out0[ 0x10 * 15 ] = b1[ 0x10 ] + b1[ 0x18 ];
 out0[ 0x10 * 13 ] = b1[ 0x18 ] + b1[ 0x14 ];
 b1[ 0x1C ] += b1[ 0x1a ];
 out0[ 0x10 * 11 ] = b1[ 0x14 ] + b1[ 0x1C ];
 out0[ 0x10 *  9 ] = b1[ 0x1C ] + b1[ 0x12 ];
 b1[ 0x1A ] += b1[ 0x1E ];
 out0[ 0x10 *  7 ] = b1[ 0x12 ] + b1[ 0x1A ];
 out0[ 0x10 *  5 ] = b1[ 0x1A ] + b1[ 0x16 ];
 b1[ 0x1E ] += b1[ 0x19 ];
 out0[ 0x10 *  3 ] = b1[ 0x16 ] + b1[ 0x1E ];
 out0[ 0x10 *  1 ] = b1[ 0x1E ] + b1[ 0x11 ];
 b1[ 0x19 ] += b1[ 0x1D ];
 out1[ 0x10 *  1 ] = b1[ 0x11 ] + b1[ 0x19 ];
 out1[ 0x10 *  3 ] = b1[ 0x19 ] + b1[ 0x15 ];
 b1[ 0x1D ] += b1[ 0x1B ];
 out1[ 0x10 *  5 ] = b1[ 0x15 ] + b1[ 0x1D ];
 out1[ 0x10 *  7 ] = b1[ 0x1D ] + b1[ 0x13 ];
 b1[ 0x1B ] += b1[ 0x1F ];
 out1[ 0x10 *  9 ] = b1[ 0x13 ] + b1[ 0x1B ];
 out1[ 0x10 * 11 ] = b1[ 0x1B ] + b1[ 0x17 ];
 out1[ 0x10 * 13 ] = b1[ 0x17 ] + b1[ 0x1F ];
 out1[ 0x10 * 15 ] = b1[ 0x1F ];
}

/*
==================
dct64_1

the call via dct64 is a trick to force GCC to use
(new) registers for the b1,b2 pointer to the bufs[xx] field
==================
*/
static void dct64(float *a,float *b,float *c)
{
  float bufs[0x40];
  dct64_1(a,b,bufs,bufs+0x20,c);
}

/*
==================
dct36
==================
*/
static void dct36(float *inbuf,float *o1,float *o2,float *wintab,float *tsbuf)
{
  {
    register float *in = inbuf;

    in[ 17 ]+=in[ 16 ]; in[ 16 ]+=in[ 15 ]; in[ 15 ]+=in[ 14 ];
    in[ 14 ]+=in[ 13 ]; in[ 13 ]+=in[ 12 ]; in[ 12 ]+=in[ 11 ];
    in[ 11 ]+=in[ 10 ]; in[ 10 ]+=in[ 9 ];  in[ 9 ] +=in[ 8 ];
    in[ 8 ] +=in[ 7 ];  in[ 7 ] +=in[ 6 ];  in[ 6 ] +=in[ 5 ];
    in[ 5 ] +=in[ 4 ];  in[ 4 ] +=in[ 3 ];  in[ 3 ] +=in[ 2 ];
    in[ 2 ] +=in[ 1 ];  in[ 1 ] +=in[ 0 ];

    in[ 17 ]+=in[ 15 ]; in[ 15 ]+=in[ 13 ]; in[ 13 ]+=in[ 11 ]; in[ 11 ]+=in[ 9 ];
    in[ 9 ] +=in[ 7 ];  in[ 7 ] +=in[ 5 ];  in[ 5 ] +=in[ 3 ];  in[ 3 ] +=in[ 1 ];


    {

#define MACRO0(v) { \
      float tmp; \
      out2[9+(v)] = (tmp = sum0 + sum1) * w[27+(v)]; \
      out2[8-(v)] = tmp * w[26-(v)];  } \
      sum0 -= sum1; \
      ts[SBLIMIT*(8-(v))] = out1[8-(v)] + sum0 * w[8-(v)]; \
      ts[SBLIMIT*(9+(v))] = out1[9+(v)] + sum0 * w[9+(v)]; 
#define MACRO1(v) { \
      float sum0,sum1; \
      sum0 = tmp1a + tmp2a; \
      sum1 = (tmp1b + tmp2b) * tfcos36[(v)]; \
      MACRO0(v); }
#define MACRO2(v) { \
      float sum0,sum1; \
      sum0 = tmp2a - tmp1a; \
      sum1 = (tmp2b - tmp1b) * tfcos36[(v)]; \
      MACRO0(v); }

      register const float *c = COS9;
      register float *out2 = o2;
      register float *w = wintab;
      register float *out1 = o1;
      register float *ts = tsbuf;

      float ta33, ta66, tb33, tb66;

      ta33 = in[ 2 * 3+0 ]  *  c[ 3 ];
      ta66 = in[ 2 * 6+0 ]  *  c[ 6 ];
      tb33 = in[ 2 * 3+1 ]  *  c[ 3 ];
      tb66 = in[ 2 * 6+1 ]  *  c[ 6 ];

      { 
        float tmp1a, tmp2a, tmp1b, tmp2b;
        tmp1a = in[ 2 * 1 + 0 ] * c[ 1 ] + ta33 + in[ 2 * 5 + 0 ] * c[ 5 ] + in[ 2 * 7 + 0 ] * c[ 7 ];
        tmp1b = in[ 2 * 1 + 1 ] * c[ 1 ] + tb33 + in[ 2 * 5 + 1 ] * c[ 5 ] + in[ 2 * 7 + 1 ] * c[ 7 ];
        tmp2a = in[ 2 * 0 + 0 ] + in[ 2 * 2 + 0 ] * c[ 2 ] + in[ 2 * 4 + 0 ] * c[ 4 ] + ta66 + in[ 2 * 8 + 0 ] * c[ 8 ];
        tmp2b = in[ 2 * 0 + 1 ] + in[ 2 * 2 + 1 ] * c[ 2 ] + in[ 2 * 4 + 1 ] * c[ 4 ] + tb66 + in[ 2 * 8 + 1 ] * c[ 8 ];

        MACRO1( 0 );
        MACRO2( 8 );
      }

      {
        float tmp1a, tmp2a, tmp1b, tmp2b;
        tmp1a = ( in[ 2 * 1 + 0 ] - in[ 2 * 5 + 0 ] - in[ 2 * 7 + 0 ] ) * c[ 3 ];
        tmp1b = ( in[ 2 * 1 + 1 ] - in[ 2 * 5 + 1 ] - in[ 2 * 7 + 1 ] ) * c[ 3 ];
        tmp2a = ( in[ 2 * 2 + 0 ] - in[ 2 * 4 + 0 ] - in[ 2 * 8 + 0 ] ) * c[ 6 ] - in[ 2 * 6 + 0 ] + in[ 2 * 0 + 0 ];
        tmp2b = ( in[ 2 * 2 + 1 ] - in[ 2 * 4 + 1 ] - in[ 2 * 8 + 1 ] ) * c[ 6 ] - in[ 2 * 6 + 1 ] + in[ 2 * 0 + 1 ];

        MACRO1( 1 );
        MACRO2( 7 );
      }

      {
        float tmp1a, tmp2a, tmp1b, tmp2b;
        tmp1a = in[ 2 * 1 + 0 ] * c[ 5 ] - ta33 - in[ 2 * 5 + 0 ] * c[ 7 ] + in[ 2 * 7 + 0 ] * c[ 1 ];
        tmp1b = in[ 2 * 1 + 1 ] * c[ 5 ] - tb33 - in[ 2 * 5 + 1 ] * c[ 7 ] + in[ 2 * 7 + 1 ] * c[ 1 ];
        tmp2a = in[ 2 * 0 + 0 ] - in[ 2 * 2 + 0 ] * c[ 8 ] - in[ 2 * 4 + 0 ] * c[ 2 ] + ta66 + in[ 2 * 8 + 0 ] * c[ 4 ];
        tmp2b = in[ 2 * 0 + 1 ] - in[ 2 * 2 + 1 ] * c[ 8 ] - in[ 2 * 4 + 1 ] * c[ 2 ] + tb66 + in[ 2 * 8 + 1 ] * c[ 4 ];

        MACRO1( 2 );
        MACRO2( 6 );
      }

      {
        float tmp1a, tmp2a, tmp1b, tmp2b;
        tmp1a = in[ 2 * 1 + 0 ] * c[ 7 ] - ta33 + in[ 2 * 5 + 0 ] * c[ 1 ] - in[ 2 * 7 + 0 ] * c[ 5 ];
        tmp1b = in[ 2 * 1 + 1 ] * c[ 7 ] - tb33 + in[ 2 * 5 + 1 ] * c[ 1 ] - in[ 2 * 7 + 1 ] * c[ 5 ];
        tmp2a = in[ 2 * 0 + 0 ] - in[ 2 * 2 + 0 ] * c[ 4 ] + in[ 2 * 4 + 0 ] * c[ 8 ] + ta66 - in[ 2 * 8 + 0 ] * c[ 2 ];
        tmp2b = in[ 2 * 0 + 1 ] - in[ 2 * 2 + 1 ] * c[ 4 ] + in[ 2 * 4 + 1 ] * c[ 8 ] + tb66 - in[ 2 * 8 + 1 ] * c[ 2 ];

        MACRO1( 3 );
        MACRO2( 5 );
      }

      {
        float sum0, sum1;
        sum0 =   in[ 2 * 0 + 0 ] - in[ 2 * 2 + 0 ] + in[ 2 * 4 + 0 ] - in[ 2 * 6 + 0 ] + in[ 2 * 8 + 0 ];
        sum1 = ( in[ 2 * 0 + 1 ] - in[ 2 * 2 + 1 ] + in[ 2 * 4 + 1 ] - in[ 2 * 6 + 1 ] + in[ 2 * 8 + 1 ] ) * tfcos36[ 4 ];
        MACRO0( 4 );
      }
    }

  }
}

/*
==================
dct12
==================
*/
static void dct12( float *in, float *rawout1, float *rawout2, register float *wi, register float *ts )
{
#define DCT12_PART1 \
     in5 = in[5*3];  \
     in5 += (in4 = in[4*3]); \
     in4 += (in3 = in[3*3]); \
     in3 += (in2 = in[2*3]); \
     in2 += (in1 = in[1*3]); \
     in1 += (in0 = in[0*3]); \
                             \
     in5 += in3; in3 += in1; \
                             \
     in2 *= COS6_1; \
     in3 *= COS6_1; \

#define DCT12_PART2 \
     in0 += in4 * COS6_2; \
                          \
     in4 = in0 + in2;     \
     in0 -= in2;          \
                          \
     in1 += in5 * COS6_2; \
                          \
     in5 = (in1 + in3) * tfcos12[0]; \
     in1 = (in1 - in3) * tfcos12[2]; \
                         \
     in3 = in4 + in5;    \
     in4 -= in5;         \
                         \
     in2 = in0 + in1;    \
     in0 -= in1;


   {
     float in0, in1, in2, in3, in4, in5;
     register float *out1 = rawout1;
     ts[ SBLIMIT * 0 ] = out1[ 0 ]; ts[ SBLIMIT * 1 ] = out1[ 1 ]; ts[ SBLIMIT * 2 ] = out1[ 2 ];
     ts[ SBLIMIT * 3 ] = out1[ 3 ]; ts[ SBLIMIT * 4 ] = out1[ 4 ]; ts[ SBLIMIT * 5 ] = out1[ 5 ];
 
     DCT12_PART1

     {
       float tmp0, tmp1 = ( in0 - in4 );
       {
         float tmp2 = ( in1 - in5 ) * tfcos12[ 1 ];
         tmp0 = tmp1 + tmp2;
         tmp1 -= tmp2;
       }
       ts[ ( 17 - 1 ) * SBLIMIT ] = out1[ 17 - 1 ] + tmp0 * wi[ 11 - 1 ];
       ts[ ( 12 + 1 ) * SBLIMIT ] = out1[ 12 + 1 ] + tmp0 * wi[ 6 + 1 ];
       ts[ ( 6  + 1 ) * SBLIMIT ] = out1[ 6  + 1 ] + tmp1 * wi[ 1 ];
       ts[ ( 11 - 1 ) * SBLIMIT ] = out1[ 11 - 1 ] + tmp1 * wi[ 5 - 1 ];
     }

     DCT12_PART2

     ts[ ( 17 - 0 ) * SBLIMIT ] = out1[ 17 - 0 ] + in2 * wi[ 11 - 0 ];
     ts[ ( 12 + 0 ) * SBLIMIT ] = out1[ 12 + 0 ] + in2 * wi[ 6 + 0 ];
     ts[ ( 12 + 2 ) * SBLIMIT ] = out1[ 12 + 2 ] + in3 * wi[ 6 + 2 ];
     ts[ ( 17 - 2 ) * SBLIMIT ] = out1[ 17 - 2 ] + in3 * wi[ 11 - 2 ];

     ts[ ( 6  + 0 ) * SBLIMIT ] = out1[ 6  + 0 ] + in0 * wi[ 0 ];
     ts[ ( 11 - 0 ) * SBLIMIT ] = out1[ 11 - 0 ] + in0 * wi[ 5 - 0 ];
     ts[ ( 6  + 2 ) * SBLIMIT ] = out1[ 6  + 2 ] + in4 * wi[ 2 ];
     ts[ ( 11 - 2 ) * SBLIMIT ] = out1[ 11 - 2 ] + in4 * wi[ 5 - 2 ];
  }

  in++;

  {
     float in0, in1, in2, in3, in4, in5;
     register float *out2 = rawout2;
 
     DCT12_PART1

     {
       float tmp0, tmp1 = ( in0 - in4 );
       {
         float tmp2 = ( in1 - in5 ) * tfcos12[ 1 ];
         tmp0 = tmp1 + tmp2;
         tmp1 -= tmp2;
       }
       out2[ 5 - 1 ] = tmp0 * wi[ 11 - 1 ];
       out2[ 0 + 1 ] = tmp0 * wi[ 6  + 1 ];
       ts[ ( 12 + 1 ) * SBLIMIT ] += tmp1 * wi[ 1 ];
       ts[ ( 17 - 1 ) * SBLIMIT ] += tmp1 * wi[ 5 - 1 ];
     }

     DCT12_PART2

     out2[ 5 - 0 ] = in2 * wi[ 11 - 0 ];
     out2[ 0 + 0 ] = in2 * wi[ 6  + 0 ];
     out2[ 0 + 2 ] = in3 * wi[ 6  + 2 ];
     out2[ 5 - 2 ] = in3 * wi[ 11 - 2 ];
 
     ts[ ( 12 + 0 ) * SBLIMIT ] += in0 * wi[ 0 ];
     ts[ ( 17 - 0 ) * SBLIMIT ] += in0 * wi[ 5 - 0 ];
     ts[ ( 12 + 2 ) * SBLIMIT ] += in4 * wi[ 2 ];
     ts[ ( 17 - 2 ) * SBLIMIT ] += in4 * wi[ 5 - 2 ];
  }

  in++; 

  {
     float in0, in1, in2, in3, in4, in5;
     register float *out2 = rawout2;
     out2[ 12 ] = out2[ 13 ] = out2[ 14 ] = out2[ 15 ] = out2[ 16 ] = out2[ 17 ] = 0.0;

     DCT12_PART1

     {
       float tmp0, tmp1 = ( in0 - in4 );
       {
         float tmp2 = ( in1 - in5 ) * tfcos12[ 1 ];
         tmp0 = tmp1 + tmp2;
         tmp1 -= tmp2;
       }
       out2[ 11 - 1 ]  = tmp0 * wi[ 11 - 1 ];
       out2[ 6  + 1 ]  = tmp0 * wi[ 6  + 1 ];
       out2[ 0  + 1 ] += tmp1 * wi[ 1 ];
       out2[ 5  - 1 ] += tmp1 * wi[ 5  - 1 ];
     }

     DCT12_PART2

     out2[ 11 - 0 ] = in2 * wi[ 11 - 0 ];
     out2[ 6  + 0 ] = in2 * wi[ 6  + 0 ];
     out2[ 6  + 2 ] = in3 * wi[ 6  + 2 ];
     out2[ 11 - 2 ] = in3 * wi[ 11 - 2 ];

     out2[ 0 + 0 ] += in0 * wi[ 0 ];
     out2[ 5 - 0 ] += in0 * wi[ 5 - 0 ];
     out2[ 0 + 2 ] += in4 * wi[ 2 ];
     out2[ 5 - 2 ] += in4 * wi[ 5 - 2 ];
  }
}

/*
==================
III_hybrid
==================
*/
static void III_hybrid( float fsIn[ SBLIMIT ][ SSLIMIT ], float tsOut[ SSLIMIT ][ SBLIMIT ],
   int ch, gr_info_t *gr_info )
{
   float *tspnt = (float *)tsOut;
   float (*block)[ 2 ][ SBLIMIT * SSLIMIT ] = gmp->hybrid_block;
   int *blc = gmp->hybrid_blc;
   float *rawout1, *rawout2;
   int bt;
   int sb = 0;

   {
     int b = blc[ ch ];
     rawout1=block[ b ][ ch ];
     b = -b + 1;
     rawout2 = block[ b ][ ch ];
     blc[ ch ] = b;
   }

  
   if( gr_info->mixed_block_flag )
   {
     sb = 2;
     dct36( fsIn[ 0 ], rawout1, rawout2, win[ 0 ], tspnt );
     dct36( fsIn[ 1 ], rawout1 + 18, rawout2 + 18, win1[ 0 ], tspnt + 1 );
     rawout1 += 36;
     rawout2 += 36;
     tspnt += 2;
   }
 
   bt = gr_info->block_type;
   if( bt == 2 )
   {
     for( ; sb < gr_info->maxb; sb += 2, tspnt += 2, rawout1 += 36, rawout2 += 36 )
     {
       dct12( fsIn[ sb ], rawout1, rawout2, win[ 2 ], tspnt );
       dct12( fsIn[ sb + 1 ], rawout1 + 18, rawout2 + 18, win1[ 2 ], tspnt + 1 );
     }
   }
   else
   {
     for( ; sb < gr_info->maxb; sb += 2, tspnt += 2, rawout1 += 36, rawout2 += 36 )
     {
       dct36( fsIn[ sb ], rawout1, rawout2, win[ bt ], tspnt );
       dct36( fsIn[ sb + 1 ], rawout1 + 18, rawout2 + 18, win1[ bt ], tspnt + 1 );
     }
   }

   for( ; sb < SBLIMIT; sb++, tspnt++ )
   {
     int i;
     for( i = 0; i < SSLIMIT; i++ )
     {
       tspnt[ i * SBLIMIT ] = *rawout1++;
       *rawout2++ = 0.0;
     }
   }
}

/*
==================
setPointer
==================
*/
static int setPointer(long backstep)
{
  unsigned char *bsbufold;
  if( gmp->fsizeold < 0 && backstep > 0 )
  {
    Com_Printf( "mp3dec: can't step back %ld\n", backstep );
    return DE_ERR;
  }
  
  bsbufold = gmp->bsspace[ gmp->bsnum ] + 512;
  wordpointer -= backstep;
  
  if( backstep )
    memcpy( wordpointer, bsbufold + gmp->fsizeold - backstep, backstep );

  bitindex = 0;
  return DE_OK;
}

#define WRITE_SAMPLE(samples,sum,clip) \
  if( (sum) > 32767.0) { *(samples) = 0x7fff; (clip)++; } \
  else if( (sum) < -32768.0) { *(samples) = -0x8000; (clip)++; } \
  else { *(samples) = sum; }

/*
==================
synth1to1
==================
*/
static int synth1to1( float *bandPtr, int channel, unsigned char *out, int *pnt )
{
  static const int step = 2;
  int bo;
  short *samples = (short *)( out + *pnt );

  float *b0, (*buf)[ 0x110 ];
  int clip = 0; 
  int bo1;

  bo = gmp->synth_bo;

  if( !channel )
  {
    bo--;
    bo &= 0xf;
    buf = gmp->synth_buffs[ 0 ];
  }
  else
  {
    samples++;
    buf = gmp->synth_buffs[ 1 ];
  }

  if( bo & 0x1 )
  {
    b0 = buf[ 0 ];
    bo1 = bo;
    dct64( buf[ 1 ] + ( ( bo + 1 ) & 0xf ), buf[ 0 ] + bo, bandPtr );
  }
  else
  {
    b0 = buf[ 1 ];
    bo1 = bo + 1;
    dct64( buf[ 0 ] + bo, buf[ 1 ] + bo + 1, bandPtr );
  }

  gmp->synth_bo = bo;
  
  {
    register int j;
    float *window = decwin + 16 - bo1;

    for ( j = 16; j; j--, b0 += 0x10, window += 0x20, samples += step )
    {
      float sum;
      sum  = window[ 0x0 ] * b0[ 0x0 ];
      sum -= window[ 0x1 ] * b0[ 0x1 ];
      sum += window[ 0x2 ] * b0[ 0x2 ];
      sum -= window[ 0x3 ] * b0[ 0x3 ];
      sum += window[ 0x4 ] * b0[ 0x4 ];
      sum -= window[ 0x5 ] * b0[ 0x5 ];
      sum += window[ 0x6 ] * b0[ 0x6 ];
      sum -= window[ 0x7 ] * b0[ 0x7 ];
      sum += window[ 0x8 ] * b0[ 0x8 ];
      sum -= window[ 0x9 ] * b0[ 0x9 ];
      sum += window[ 0xA ] * b0[ 0xA ];
      sum -= window[ 0xB ] * b0[ 0xB ];
      sum += window[ 0xC ] * b0[ 0xC ];
      sum -= window[ 0xD ] * b0[ 0xD ];
      sum += window[ 0xE ] * b0[ 0xE ];
      sum -= window[ 0xF ] * b0[ 0xF ];

      WRITE_SAMPLE( samples,sum,clip );
    }

    {
      float sum;
      sum  = window[ 0x0 ] * b0[ 0x0 ];
      sum += window[ 0x2 ] * b0[ 0x2 ];
      sum += window[ 0x4 ] * b0[ 0x4 ];
      sum += window[ 0x6 ] * b0[ 0x6 ];
      sum += window[ 0x8 ] * b0[ 0x8 ];
      sum += window[ 0xA ] * b0[ 0xA ];
      sum += window[ 0xC ] * b0[ 0xC ];
      sum += window[ 0xE ] * b0[ 0xE ];
      WRITE_SAMPLE( samples,sum,clip );
      b0 -= 0x10, window -= 0x20, samples += step;
    }
    window += bo1 << 1;

    for ( j = 15; j; j--, b0 -= 0x10, window -= 0x20, samples += step )
    {
      float sum;
      sum = -window[ -0x1 ] * b0[ 0x0 ];
      sum -= window[ -0x2 ] * b0[ 0x1 ];
      sum -= window[ -0x3 ] * b0[ 0x2 ];
      sum -= window[ -0x4 ] * b0[ 0x3 ];
      sum -= window[ -0x5 ] * b0[ 0x4 ];
      sum -= window[ -0x6 ] * b0[ 0x5 ];
      sum -= window[ -0x7 ] * b0[ 0x6 ];
      sum -= window[ -0x8 ] * b0[ 0x7 ];
      sum -= window[ -0x9 ] * b0[ 0x8 ];
      sum -= window[ -0xA ] * b0[ 0x9 ];
      sum -= window[ -0xB ] * b0[ 0xA ];
      sum -= window[ -0xC ] * b0[ 0xB ];
      sum -= window[ -0xD ] * b0[ 0xC ];
      sum -= window[ -0xE ] * b0[ 0xD ];
      sum -= window[ -0xF ] * b0[ 0xE ];
      sum -= window[ -0x0 ] * b0[ 0xF ];

      WRITE_SAMPLE( samples, sum, clip );
    }
  }
  *pnt += 128;

  return clip;
}

/*
==================
synth1to1mono
==================
*/
static int synth1to1mono( float *bandPtr, unsigned char *samples, int *pnt )
{
  short samples_tmp[ 64 ];
  short *tmp1 = samples_tmp;
  int i, ret;
  int pnt1 = 0;

  ret = synth1to1( bandPtr, 0, (unsigned char *)samples_tmp, &pnt1 );
  samples += *pnt;

  for( i = 0; i < 32; i++ )
  {
    *( (short *)samples ) = *tmp1;
    samples += 2;
    tmp1 += 2;
  }
  *pnt += 64;

  return ret;
}

/*
==================
doLayer3
==================
*/
static decoderError_t doLayer3( frame_t *fr, unsigned char *pcm_sample, int *pcm_point )
{
  int gr, ch, ss, clip = 0;
  int scalefacs[ 39 ]; /* max 39 for short[13][3] mode, mixed: 38, long: 22 */
  III_sideinfo_t sideinfo;
  int stereo = fr->stereo;
  int single = fr->single;
  int ms_stereo, i_stereo;
  int sfreq = fr->sampling_frequency;
  int stereo1, granules;

  if( stereo == 1 )
  { /* stream is mono */
    stereo1 = 1;
    single = 0;
  }
  else if( single >= 0 ) /* stream is stereo, but force to mono */
    stereo1 = 1;
  else
    stereo1 = 2;

  if( fr->mode == MPG_MD_JOINT_STEREO )
  {
    ms_stereo = fr->mode_ext & 0x2;
    i_stereo  = fr->mode_ext & 0x1;
  }
  else
    ms_stereo = i_stereo = 0;

  if( fr->lsf )
  {
    granules = 1;
    if( III_get_side_info_2(&sideinfo,stereo,ms_stereo,sfreq,single) != DE_OK )
      return DE_ERR;
  }
  else
  {
    granules = 2;
    if( III_get_side_info_1(&sideinfo,stereo,ms_stereo,sfreq,single) != DE_OK )
      return DE_ERR;
  }

  if( setPointer( sideinfo.main_data_begin ) == DE_ERR )
    return DE_ERR;

  for( gr = 0 ; gr < granules; gr++ ) 
  {
    static float hybridIn[ 2 ][ SBLIMIT ][ SSLIMIT ];
    static float hybridOut[ 2 ][ SSLIMIT ][ SBLIMIT ];

    {
      gr_info_t *gr_info = &( sideinfo.ch[ 0 ].gr[ gr ] );
      long part2bits;
      
      if( fr->lsf )
        part2bits = III_get_scale_factors_2(scalefacs,gr_info,0);
      else
        part2bits = III_get_scale_factors_1(scalefacs,gr_info);
        
      if( III_dequantize_sample( hybridIn[0], scalefacs, gr_info, sfreq, part2bits ) != DE_OK )
        return DE_ERR;
    }
    
    if( stereo == 2 )
    {
      gr_info_t *gr_info = &( sideinfo.ch[ 1 ].gr[ gr ] );
      long part2bits;
      
      if( fr->lsf ) 
        part2bits = III_get_scale_factors_2( scalefacs, gr_info,i_stereo );
      else
        part2bits = III_get_scale_factors_1(scalefacs,gr_info);

      if( III_dequantize_sample( hybridIn[ 1 ], scalefacs, gr_info, sfreq, part2bits ) != DE_OK )
        return DE_ERR;

      if( ms_stereo )
      {
        int i;
        for( i = 0; i < SBLIMIT * SSLIMIT; i++ )
        {
          float tmp0, tmp1;
          tmp0 = ( (float *)hybridIn[ 0 ] )[ i ];
          tmp1 = ( (float *)hybridIn[ 1 ] )[ i ];
          ( (float *)hybridIn[ 1 ] )[ i ] = tmp0 - tmp1;  
          ( (float *)hybridIn[ 0 ] )[ i ] = tmp0 + tmp1;
        }
      }

      if( i_stereo )
        III_i_stereo( hybridIn, scalefacs, gr_info, sfreq, ms_stereo, fr->lsf );

      if( ms_stereo || i_stereo || ( single == 3 ) )
      {
        if( gr_info->maxb > sideinfo.ch[ 0 ].gr[ gr ].maxb ) 
          sideinfo.ch[ 0 ].gr[ gr ].maxb = gr_info->maxb;
        else
          gr_info->maxb = sideinfo.ch[ 0 ].gr[ gr ].maxb;
      }

      switch( single )
      {
        case 3:
          {
            register int i;
            register float *in0 = (float *)hybridIn[ 0 ], *in1 = (float *)hybridIn[ 1 ];
            
            for( i = 0; i < SSLIMIT * gr_info->maxb; i++, in0++ )
              *in0 = ( *in0 + *in1++ ); /* *0.5 done by pow-scale */ 
          }
          break;
        case 1:
          {
            register int i;
            register float *in0 = (float *)hybridIn[ 0 ], *in1 = (float *)hybridIn[ 1 ];
            for( i = 0; i < SSLIMIT * gr_info->maxb; i++ )
              *in0++ = *in1++;
          }
          break;
      }
    }

    for( ch = 0; ch < stereo1; ch++ )
    {
      gr_info_t *gr_info = &( sideinfo.ch[ ch ].gr[ gr ] );
      III_antialias( hybridIn[ ch ], gr_info );
      III_hybrid( hybridIn[ ch ], hybridOut[ ch ], ch, gr_info);
    }

    for( ss = 0; ss < SSLIMIT; ss++)
    {
      if( single >= 0 )
        clip += synth1to1mono( hybridOut[ 0 ][ ss ], pcm_sample, pcm_point );
      else
      {
        int p1 = *pcm_point;
        clip += synth1to1( hybridOut[ 0 ][ ss ], 0, pcm_sample, &p1 );
        clip += synth1to1( hybridOut[ 1 ][ ss ], 1, pcm_sample, pcm_point );
      }
    }
  }
  
  return DE_OK;
}

/*
==================
addBuffer
==================
*/
static buffer_t *addBuffer( mpegPacket_t *mp, char *buf, int size )
{
  buffer_t *nbuf;

  nbuf = UI_AllocMem( sizeof( buffer_t ) );
  
  //execution will stop in CG_Alloc if out of memory, but
  //defensive programming says its worth checking anyway
  if( !nbuf )
  {
    Com_Error( ERR_FATAL, "mp3dec: Out of memory!\n" );
    return NULL;
  }
  
  nbuf->pnt = UI_AllocMem( size );
  
  //likewise
  if( !nbuf->pnt )
  {
    UI_FreeMem( nbuf );
    return NULL;
  }
  
  nbuf->size = size;
  memcpy( nbuf->pnt, buf, size );
  nbuf->next = NULL;
  nbuf->prev = mp->head;
  nbuf->pos = 0;

  if( !mp->tail )
    mp->tail = nbuf;
  else
    mp->head->next = nbuf;

  mp->head = nbuf;
  mp->bsize += size;

  return nbuf;
}

/*
==================
removeBuffer
==================
*/
static void removeBuffer( mpegPacket_t *mp )
{
  buffer_t *buf = mp->tail;
  
  mp->tail = buf->next;
  if( mp->tail )
    mp->tail->prev = NULL;
  else
    mp->tail = mp->head = NULL;
  
  UI_FreeMem( buf->pnt );
  UI_FreeMem( buf );

}

/*
==================
readBufferByte
==================
*/
static int readBufferByte(mpegPacket_t *mp)
{
  unsigned int b;

  int pos;

  pos = mp->tail->pos;
  while(pos >= mp->tail->size)
  {
    removeBuffer( mp );
    pos = mp->tail->pos;
    if( !mp->tail )
    {
      Com_Error( ERR_FATAL, "mp3dec: Failed reading buffer\n" );
    }
  }

  b = mp->tail->pnt[ pos ];
  mp->bsize--;
  mp->tail->pos++;
  
  return b;
}

/*
==================
readHead
==================
*/
static void readHead(mpegPacket_t *mp)
{
  unsigned long head;

  head = readBufferByte( mp );
  head <<= 8;
  head |= readBufferByte( mp );
  head <<= 8;
  head |= readBufferByte( mp );
  head <<= 8;
  head |= readBufferByte( mp );

  mp->header = head;
}

/*
==================
readHead

decode a header and write the information
into the frame structure
==================
*/
static decoderError_t decodeHeader( frame_t *fr, unsigned long newhead )
{
    if( newhead & ( 1 << 20 ) )
    {
      fr->lsf = ( newhead & ( 1 << 19 ) ) ? 0x0 : 0x1;
      fr->mpeg25 = 0;
    }
    else
    {
      fr->lsf = 1;
      fr->mpeg25 = 1;
    }
    
    fr->lay = 4 - ( ( newhead >> 17 ) & 3 );
    
    if( ( ( newhead >> 10 ) & 0x3 ) == 0x3 )
    {
      Com_Printf( "mp3dec: Stream error in header\n" );
      return DE_ERR;
    }
    
    if( fr->mpeg25 )
      fr->sampling_frequency = 6 + ( ( newhead >> 10 ) & 0x3 );
    else
      fr->sampling_frequency = ( ( newhead >> 10 ) & 0x3 ) + ( fr->lsf * 3 );
      
    fr->error_protection = ( ( newhead >> 16 ) & 0x1 ) ^ 0x1;

    if( fr->mpeg25 ) /* allow Bitrate change for 2.5 ... */
      fr->bitrate_index = ( ( newhead >> 12 ) & 0xf );

    fr->bitrate_index = ( ( newhead >> 12 ) & 0xf );
    fr->padding       = ( ( newhead >> 9 ) & 0x1 );
    fr->extension     = ( ( newhead >> 8 ) & 0x1 );
    fr->mode          = ( ( newhead >> 6 ) & 0x3 );
    fr->mode_ext      = ( ( newhead >> 4 ) & 0x3 );
    fr->copyright     = ( ( newhead >> 3 ) & 0x1 );
    fr->original      = ( ( newhead >> 2 ) & 0x1 );
    fr->emphasis      = newhead & 0x3;

    fr->stereo        = ( fr->mode == MPG_MD_MONO ) ? 1 : 2;

    if( !fr->bitrate_index )
    {
      Com_Printf( "mp3dec: VBR not supported\n" );
      return DE_ERR;
    }

    switch( fr->lay )
    {
      case 1:
        Com_Printf( "mp3dec: layer 1 not supported\n" );
        return DE_ERR;
        break;
        
      case 2:
        Com_Printf( "mp3dec: layer 2 not supported\n" );
        return DE_ERR;
        break;
        
      case 3:
        fr->framesize = (long)tabsel_123[ fr->lsf ][ 2 ][ fr->bitrate_index ] * 144000;
        fr->framesize /= freqs[ fr->sampling_frequency ] << ( fr->lsf );
        fr->framesize = fr->framesize + fr->padding - 4;
        break;
        
      default:
        Com_Printf( "mp3dec: unknown layer type\n" );
        return DE_ERR;
    }

    return DE_OK;
}

/*
==================
decodeMP3
==================
*/
static decoderError_t decodeMP3( mpegPacket_t *mp, char *in, int isize, char *out, int osize, int *done )
{
  int len;

  gmp = mp;

  if( osize < 4608 )
  {
    Com_Printf( "mp3dec: not enough data to decode\n" );
    return DE_ERR;
  }

  if( in )
  {
    if( addBuffer( mp, in, isize ) == NULL)
      return DE_ERR;
  }

  /* First decode header */
  if( mp->framesize == 0 )
  {
    if( mp->bsize < 4 )
      return DE_NEED_MORE_DATA;
      
    readHead( mp );
    
    if( decodeHeader( &mp->fr, mp->header ) != DE_OK )
      return DE_ERR;
      
    mp->framesize = mp->fr.framesize;
  }

  if( mp->fr.framesize > mp->bsize )
    return DE_NEED_MORE_DATA;

  wordpointer = mp->bsspace[ mp->bsnum ] + 512;
  mp->bsnum = ( mp->bsnum + 1 ) & 0x1;
  bitindex = 0;

  len = 0;
  while( len < mp->framesize )
  {
    int nlen;
    int blen = mp->tail->size - mp->tail->pos;

    if( ( mp->framesize - len ) <= blen )
      nlen = mp->framesize - len;
    else
      nlen = blen;
      
    memcpy( wordpointer + len, mp->tail->pnt + mp->tail->pos, nlen );
    len += nlen;
    mp->tail->pos += nlen;
    mp->bsize -= nlen;
    
    if( mp->tail->pos == mp->tail->size )
      removeBuffer( mp );
  }

  *done = 0;
  if( mp->fr.error_protection )
    getbits( 16 );
    
  if( doLayer3( &mp->fr, (unsigned char *)out, done ) != DE_OK )
    return DE_ERR;

  mp->fsizeold = mp->framesize;
  mp->framesize = 0;

  return DE_OK;
}

/*
==================
initMpegPacket
==================
*/
static void initMpegPacket( mpegPacket_t *mp ) 
{
  memset( mp, 0, sizeof( mpegPacket_t ) );

  mp->framesize = 0;
  mp->fsizeold = -1;
  mp->bsize = 0;
  mp->head = mp->tail = NULL;
  mp->fr.single = -1;
  mp->bsnum = 0;
  mp->synth_bo = 1;

  makeDecodeTables( 32767 );
  initLayer3(  SBLIMIT );
}

/*
==================
wavOpen

creates a new PCM wav and returns a file handle
==================
*/
static fileHandle_t wavOpen( char *wavFile )
{
  unsigned char buf[ WAVE_HEADER_LEN ];
  fileHandle_t  fh;

  memset( buf, 0, WAVE_HEADER_LEN );
  
  //open file
  if( trap_FS_FOpenFile( wavFile, &fh, FS_WRITE ) < 0 )
    return -1;

  //Q3 fs functions are a bit um crap... there is no way of knowing whether
  //this works or not. (and no seek functions either)

  //write out a blank header
  trap_FS_Write( buf, WAVE_HEADER_LEN, fh );

  return fh;
}

/*
==================
wavWriteHeader

closes a previously opened wav file and writes a header
==================
*/
static void wavWriteHeader( fileHandle_t wavFh, long int size, unsigned int bits,
                            unsigned int rate, unsigned int channels )
{
  unsigned char       buf[ WAVE_HEADER_LEN ];
  struct wave_header  wave;
  
  if( size < 0 )
  {
    Com_Printf( "mp3dec: can't open data chunk\n" );
    trap_FS_FCloseFile( wavFh );
    return;
  }

  //wipe the header
  memset( &wave, 0, sizeof( wave ) );
  
  //create a valid header
  wave.common.wChannels = channels;
  wave.common.wBitsPerSample = bits;
  wave.common.dwSamplesPerSec = rate;

  strncpy( (char *)wave.riff.id, "RIFF", 4 );
  wave.riff.len = size - 8;
  strncpy( (char *)wave.riff.wave_id, "WAVE", 4 );

  strncpy( (char *)wave.format.id, "fmt ", 4 );
  wave.format.len = 16;

  wave.common.wFormatTag = WAVE_FORMAT_PCM;
  wave.common.dwAvgBytesPerSec = 
    wave.common.wChannels * wave.common.dwSamplesPerSec *
    ( wave.common.wBitsPerSample >> 3 );

  wave.common.wBlockAlign = wave.common.wChannels * 
    ( wave.common.wBitsPerSample >> 3 );

  strncpy( (char *)wave.data.id, "data", 4 );

  wave.data.len = size - 44;

  strncpy( (char *)buf, (char *)wave.riff.id, 4 );
  WRITE_U32( buf + 4, wave.riff.len );
  strncpy( (char *)buf + 8,  (char *)wave.riff.wave_id, 4 );
  strncpy( (char *)buf + 12, (char *)wave.format.id, 4 );
  WRITE_U32( buf + 16, wave.format.len );
  WRITE_U16( buf + 20, wave.common.wFormatTag );
  WRITE_U16( buf + 22, wave.common.wChannels );
  WRITE_U32( buf + 24, wave.common.dwSamplesPerSec );
  WRITE_U32( buf + 28, wave.common.dwAvgBytesPerSec );
  WRITE_U16( buf + 32, wave.common.wBlockAlign );
  WRITE_U16( buf + 34, wave.common.wBitsPerSample );
  strncpy( (char *)buf + 36, (char *)wave.data.id, 4 );
  WRITE_U32( buf + 40, wave.data.len );

  //seek to beginning of file
  trap_FS_Seek( wavFh, 0, FS_SEEK_SET );
  
  //write the header
  trap_FS_Write( buf, WAVE_HEADER_LEN, wavFh );

  //close the file
  trap_FS_FCloseFile( wavFh );
}

/*
==================
S_decodeMP3

Takes an MP3 file and decodes to a PCM wav. Generally speaking
this is only useful if the destination file has a .cfg or .menu
extension thus exempting it from pure server rules. This file
may then be played back using trap_S_StartBackgroundTrack.

Returns qtrue indicating sucess and qfalse for failure.
==================
*/
qboolean S_decodeMP3( char *mp3File, char *wavFile )
{
  mpegPacket_t    mp;
  int             size, len;
  long int        totalSize = WAVE_HEADER_LEN;
  decoderError_t  error;
  int             pos = CBUFFER_SIZE;
  fileHandle_t    mp3Fh, wavFh;

  //initialise mp struct
  initMpegPacket( &mp );

  //open the input mp3
  len = trap_FS_FOpenFile( mp3File, &mp3Fh, FS_READ );
  
  if( len < 0 )
  {
    Com_Printf( "mp3dec: cannot open %s\n", mp3File );
    return qfalse;
  }

  //open the output wav
  wavFh = wavOpen( wavFile );
  if( wavFh < 0 )
  {
    Com_Printf( "mp3dec: cannot write to %s\n", wavFile );
    return qfalse;
  }
  
  //intially fill the conversion buffer 
  trap_FS_Read( conversionBuffer, CBUFFER_SIZE, mp3Fh );

  //continue reading from the mp3 until out of data
  while( pos < len )
  {
    //decode
    if( decodeMP3( &mp, conversionBuffer, CBUFFER_SIZE, outputBuffer, OBUFFER_SIZE, &size ) == DE_OK )
    {
      //write out decoded pcm to data chunk wav file
      do
      {
        trap_FS_Write( outputBuffer, size, wavFh );
        error = decodeMP3( &mp, NULL, 0, outputBuffer, OBUFFER_SIZE, &size);
        totalSize += size;
      } while( error == DE_OK );

      //decoder failed
      if( error == DE_ERR )
        return qfalse;
    }
    
    //read another 16Kb from the mp3
    trap_FS_Read( conversionBuffer, CBUFFER_SIZE, mp3Fh );
    pos += CBUFFER_SIZE;
  }

  //close the mp3
  trap_FS_FCloseFile( mp3Fh );

  //write wav header and close
  wavWriteHeader( wavFh, totalSize, 16, freqs[ mp.fr.sampling_frequency ], mp.fr.stereo );

  return qtrue;
}
