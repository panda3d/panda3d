/* Filename: decode_4to1.c
 * Created by:  
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
 *
 * All use of this software is subject to the terms of the Panda 3d
 * Software license.  You should have received a copy of this license
 * along with this source code; you will also find a current copy of
 * the license at http://www.panda3d.org/license.txt .
 *
 * To contact the maintainers of this program write to
 * panda3d@yahoogroups.com .
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "mpg123.h"

#define WRITE_SAMPLE(samples,sum,clip) \
  if( (sum) > 32767.0) { *(samples) = 0x7fff; (clip)++; } \
  else if( (sum) < -32768.0) { *(samples) = -0x8000; (clip)++; } \
  else { *(samples) = sum; }

int synth_4to1_8bit(real *bandPtr,int channel,unsigned char *samples,int *pnt)
{
  short samples_tmp[16];
  short *tmp1 = samples_tmp + channel;
  int i,ret;
  int pnt1 = 0;

  ret = synth_4to1(bandPtr,channel,(unsigned char *) samples_tmp,&pnt1);
  samples += channel + *pnt;

  for(i=0;i<8;i++) {
    *samples = conv16to8[*tmp1>>AUSHIFT];
    samples += 2;
    tmp1 += 2;
  }
  *pnt += 16;

  return ret;
}

int synth_4to1_8bit_mono(real *bandPtr,unsigned char *samples,int *pnt)
{
  short samples_tmp[16];
  short *tmp1 = samples_tmp;
  int i,ret;
  int pnt1 = 0;

  ret = synth_4to1(bandPtr,0,(unsigned char *) samples_tmp,&pnt1);
  samples += *pnt;

  for(i=0;i<8;i++) {
    *samples++ = conv16to8[*tmp1>>AUSHIFT];
    tmp1 += 2;
  }
  *pnt += 8;

  return ret;
}


int synth_4to1_8bit_mono2stereo(real *bandPtr,unsigned char *samples,int *pnt)
{
  short samples_tmp[16];
  short *tmp1 = samples_tmp;
  int i,ret;
  int pnt1 = 0;

  ret = synth_4to1(bandPtr,0,(unsigned char *) samples_tmp,&pnt1);
  samples += *pnt;

  for(i=0;i<8;i++) {
    *samples++ = conv16to8[*tmp1>>AUSHIFT];
    *samples++ = conv16to8[*tmp1>>AUSHIFT];
    tmp1 += 2;
  }
  *pnt += 16;

  return ret;
}

int synth_4to1_mono(real *bandPtr,unsigned char *samples,int *pnt)
{
  short samples_tmp[16];
  short *tmp1 = samples_tmp;
  int i,ret;
  int pnt1 = 0;

  ret = synth_4to1(bandPtr,0,(unsigned char *) samples_tmp,&pnt1);
  samples += *pnt;

  for(i=0;i<8;i++) {
    *( (short *)samples) = *tmp1;
    samples += 2;
    tmp1 += 2;
  }
  *pnt += 16;

  return ret;
}

int synth_4to1_mono2stereo(real *bandPtr,unsigned char *samples,int *pnt)
{
  int i,ret;

  ret = synth_4to1(bandPtr,0,samples,pnt);
  samples = samples + *pnt - 32;

  for(i=0;i<8;i++) {
    ((short *)samples)[1] = ((short *)samples)[0];
    samples+=4;
  }

  return ret;
}

int synth_4to1(real *bandPtr,int channel,unsigned char *out,int *pnt)
{
  static real buffs[2][2][0x110];
  static const int step = 2;
  static int bo = 1;
  short *samples = (short *) (out + *pnt);

  real *b0,(*buf)[0x110];
  int clip = 0;
  int bo1;

  if(param.enable_equalizer)
    do_equalizer(bandPtr,channel);

  if(!channel) {
    bo--;
    bo &= 0xf;
    buf = buffs[0];
  }
  else {
    samples++;
    buf = buffs[1];
  }

  if(bo & 0x1) {
    b0 = buf[0];
    bo1 = bo;
    dct64(buf[1]+((bo+1)&0xf),buf[0]+bo,bandPtr);
  }
  else {
    b0 = buf[1];
    bo1 = bo+1;
    dct64(buf[0]+bo,buf[1]+bo+1,bandPtr);
  }

  {
    register int j;
    real *window = decwin + 16 - bo1;

    for (j=4;j;j--,b0+=0x30,window+=0x70)
    {
      real sum;
      sum  = *window++ * *b0++;
      sum -= *window++ * *b0++;
      sum += *window++ * *b0++;
      sum -= *window++ * *b0++;
      sum += *window++ * *b0++;
      sum -= *window++ * *b0++;
      sum += *window++ * *b0++;
      sum -= *window++ * *b0++;
      sum += *window++ * *b0++;
      sum -= *window++ * *b0++;
      sum += *window++ * *b0++;
      sum -= *window++ * *b0++;
      sum += *window++ * *b0++;
      sum -= *window++ * *b0++;
      sum += *window++ * *b0++;
      sum -= *window++ * *b0++;

      WRITE_SAMPLE(samples,sum,clip); samples += step;
#if 0
      WRITE_SAMPLE(samples,sum,clip); samples += step;
      WRITE_SAMPLE(samples,sum,clip); samples += step;
      WRITE_SAMPLE(samples,sum,clip); samples += step;
#endif
    }

    {
      real sum;
      sum  = window[0x0] * b0[0x0];
      sum += window[0x2] * b0[0x2];
      sum += window[0x4] * b0[0x4];
      sum += window[0x6] * b0[0x6];
      sum += window[0x8] * b0[0x8];
      sum += window[0xA] * b0[0xA];
      sum += window[0xC] * b0[0xC];
      sum += window[0xE] * b0[0xE];
      WRITE_SAMPLE(samples,sum,clip); samples += step;
#if 0
      WRITE_SAMPLE(samples,sum,clip); samples += step;
      WRITE_SAMPLE(samples,sum,clip); samples += step;
      WRITE_SAMPLE(samples,sum,clip); samples += step;
#endif
      b0-=0x40,window-=0x80;
    }
    window += bo1<<1;

    for (j=3;j;j--,b0-=0x50,window-=0x70)
    {
      real sum;
      sum = -*(--window) * *b0++;
      sum -= *(--window) * *b0++;
      sum -= *(--window) * *b0++;
      sum -= *(--window) * *b0++;
      sum -= *(--window) * *b0++;
      sum -= *(--window) * *b0++;
      sum -= *(--window) * *b0++;
      sum -= *(--window) * *b0++;
      sum -= *(--window) * *b0++;
      sum -= *(--window) * *b0++;
      sum -= *(--window) * *b0++;
      sum -= *(--window) * *b0++;
      sum -= *(--window) * *b0++;
      sum -= *(--window) * *b0++;
      sum -= *(--window) * *b0++;
      sum -= *(--window) * *b0++;

      WRITE_SAMPLE(samples,sum,clip); samples += step;
#if 0
      WRITE_SAMPLE(samples,sum,clip); samples += step;
      WRITE_SAMPLE(samples,sum,clip); samples += step;
      WRITE_SAMPLE(samples,sum,clip); samples += step;
#endif
    }
  }

  *pnt += 32;

  return clip;
}


