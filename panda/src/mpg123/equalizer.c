/* Filename: equalizer.c
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

#include "mpg123.h"

real equalizer[2][32];
real equalizer_sum[2][32];
int equalizer_cnt;

real equalizerband[2][SBLIMIT*SSLIMIT];

void do_equalizer(real *bandPtr,int channel)
{
        int i;

        if(param.enable_equalizer) {
                for(i=0;i<32;i++)
                        bandPtr[i] *= equalizer[channel][i];
        }

/*      if(param.equalizer & 0x2) {

                for(i=0;i<32;i++)
                        equalizer_sum[channel][i] += bandPtr[i];
        }
*/
}

void do_equalizerband(real *bandPtr,int channel)
{
  int i;
  for(i=0;i<576;i++) {
    bandPtr[i] *= equalizerband[channel][i];
  }
}

