/* Filename: decode_ntom.c
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

#define NTOM_MUL (32768)
static unsigned long ntom_val[2] = { NTOM_MUL>>1,NTOM_MUL>>1 };
static unsigned long ntom_step = NTOM_MUL;


void synth_ntom_set_step(long m,long n)
{
        if(param.verbose > 1)
                fprintf(stderr,"Init rate converter: %ld->%ld\n",m,n);

        if(n >= 96000 || m >= 96000 || m == 0 || n == 0) {
                fprintf(stderr,"NtoM converter: illegal rates\n");
                exit(1);
        }

        n *= NTOM_MUL;
        ntom_step = n / m;

        if(ntom_step > 8*NTOM_MUL) {
                fprintf(stderr,"max. 1:8 conversion allowed!\n");
                exit(1);
        }

        ntom_val[0] = ntom_val[1] = NTOM_MUL>>1;
}

int synth_ntom_8bit(real *bandPtr,int channel,unsigned char *samples,int *pnt)
{
  short samples_tmp[8*64];
  short *tmp1 = samples_tmp + channel;
  int i,ret;
  int pnt1 = 0;

  ret = synth_ntom(bandPtr,channel,(unsigned char *) samples_tmp,&pnt1);
  samples += channel + *pnt;

  for(i=0;i<(pnt1>>2);i++) {
    *samples = conv16to8[*tmp1>>AUSHIFT];
    samples += 2;
    tmp1 += 2;
  }
  *pnt += pnt1>>1;

  return ret;
}

int synth_ntom_8bit_mono(real *bandPtr,unsigned char *samples,int *pnt)
{
  short samples_tmp[8*64];
  short *tmp1 = samples_tmp;
  int i,ret;
  int pnt1 = 0;

  ret = synth_ntom(bandPtr,0,(unsigned char *) samples_tmp,&pnt1);
  samples += *pnt;

  for(i=0;i<(pnt1>>2);i++) {
    *samples++ = conv16to8[*tmp1>>AUSHIFT];
    tmp1 += 2;
  }
  *pnt += pnt1 >> 2;

  return ret;
}

int synth_ntom_8bit_mono2stereo(real *bandPtr,unsigned char *samples,int *pnt)
{
  short samples_tmp[8*64];
  short *tmp1 = samples_tmp;
  int i,ret;
  int pnt1 = 0;

  ret = synth_ntom(bandPtr,0,(unsigned char *) samples_tmp,&pnt1);
  samples += *pnt;

  for(i=0;i<(pnt1>>2);i++) {
    *samples++ = conv16to8[*tmp1>>AUSHIFT];
    *samples++ = conv16to8[*tmp1>>AUSHIFT];
    tmp1 += 2;
  }
  *pnt += pnt1 >> 1;

  return ret;
}

int synth_ntom_mono(real *bandPtr,unsigned char *samples,int *pnt)
{
  short samples_tmp[8*64];
  short *tmp1 = samples_tmp;
  int i,ret;
  int pnt1 = 0;

  ret = synth_ntom(bandPtr,0,(unsigned char *) samples_tmp,&pnt1);
  samples += *pnt;

  for(i=0;i<(pnt1>>2);i++) {
    *( (short *)samples) = *tmp1;
    samples += 2;
    tmp1 += 2;
  }
  *pnt += pnt1 >> 1;

  return ret;
}


int synth_ntom_mono2stereo(real *bandPtr,unsigned char *samples,int *pnt)
{
  int i,ret;
  int pnt1 = *pnt;

  ret = synth_ntom(bandPtr,0,samples,pnt);
  samples += pnt1;

  for(i=0;i<((*pnt-pnt1)>>2);i++) {
    ((short *)samples)[1] = ((short *)samples)[0];
    samples+=4;
  }

  return ret;
}


int synth_ntom(real *bandPtr,int channel,unsigned char *out,int *pnt)
{
  static real buffs[2][2][0x110];
  static const int step = 2;
  static int bo = 1;
  short *samples = (short *) (out + *pnt);

  real *b0,(*buf)[0x110];
  int clip = 0;
  int bo1;
  int ntom;

  if(param.enable_equalizer)
        do_equalizer(bandPtr,channel);

  if(!channel) {
    bo--;
    bo &= 0xf;
    buf = buffs[0];
    ntom = ntom_val[1] = ntom_val[0];
  }
  else {
    samples++;
    out += 2; /* to compute the right *pnt value */
    buf = buffs[1];
    ntom = ntom_val[1];
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

    for (j=16;j;j--,window+=0x10)
    {
      real sum;

      ntom += ntom_step;
      if(ntom < NTOM_MUL) {
        window += 16;
        b0 += 16;
        continue;
      }

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

      while(ntom >= NTOM_MUL) {
        WRITE_SAMPLE(samples,sum,clip);
        samples += step;
        ntom -= NTOM_MUL;
      }
    }

    ntom += ntom_step;
    if(ntom >= NTOM_MUL)
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

      while(ntom >= NTOM_MUL) {
        WRITE_SAMPLE(samples,sum,clip);
        samples += step;
        ntom -= NTOM_MUL;
      }
    }

    b0-=0x10,window-=0x20;
    window += bo1<<1;

    for (j=15;j;j--,b0-=0x20,window-=0x10)
    {
      real sum;

      ntom += ntom_step;
      if(ntom < NTOM_MUL) {
        window -= 16;
        b0 += 16;
        continue;
      }

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

      while(ntom >= NTOM_MUL) {
        WRITE_SAMPLE(samples,sum,clip);
        samples += step;
        ntom -= NTOM_MUL;
      }
    }
  }

  ntom_val[channel] = ntom;
  *pnt = ((unsigned char *) samples - out);

  return clip;
}


