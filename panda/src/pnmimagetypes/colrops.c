/* Filename: colrops.c
 * Created by:  
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Integer operations on COLR scanlines
 */

#include "color.h"

#ifndef NULL
#define NULL            0
#endif

#define bmalloc malloc
#if !defined(WIN32_VC) && !defined(WIN64_VC)
extern char     *bmalloc(int);
#else
#include <malloc.h>
#include <math.h>
#endif

#define MAXGSHIFT       31              /* maximum shift for gamma table */

static BYTE     *g_mant = NULL, *g_nexp = NULL;

static BYTE     (*g_bval)[256] = NULL;

#ifndef pow
#if !defined(WIN32_VC) && !defined(WIN64_VC)
extern double   pow(double, double);
#endif
#endif


int setcolrcor(double (*f)(double, double), double a2)
                /* set brightness correction */
{
        double  mult;
        register int    i, j;
                                        /* allocate tables */
        if (g_bval == NULL && (g_bval =
                        (BYTE (*)[256])bmalloc((MAXGSHIFT+1)*256)) == NULL)
                return(-1);
                                        /* compute colr -> gamb mapping */
        mult = 1.0/256.0;
        for (i = 0; i <= MAXGSHIFT; i++) {
                for (j = 0; j < 256; j++)
                        g_bval[i][j] = (BYTE) (256.0 * (*f)((j+.5)*mult, a2));
                mult *= 0.5;
        }
        return(0);
}


int setcolrinv(double (*f)(double, double), double a2)
                /* set inverse brightness correction */
{
        double  mult;
        register int    i, j;
                                        /* allocate tables */
        if (g_mant == NULL && (g_mant = (BYTE *)bmalloc(256)) == NULL)
                return(-1);
        if (g_nexp == NULL && (g_nexp = (BYTE *)bmalloc(256)) == NULL)
                return(-1);
                                        /* compute gamb -> colr mapping */
        i = 0;
        mult = 256.0;
        for (j = 255; j > 0; j--) {
                while ((g_mant[j] = (BYTE)(mult * (*f)(j/256.0, a2))) < 128) {
                        i++;
                        mult *= 2.0;
                }
                g_nexp[j] = i;
        }
        g_mant[0] = 0;
        g_nexp[0] = COLXS;
        return(0);
}


int setcolrgam(double g)                        /* set gamma conversion */
{
        if (setcolrcor(pow, 1.0/g) < 0)
                return(-1);
        return(setcolrinv(pow, g));
}


int colrs_gambs(register COLR *scan, int len)
                /* convert scanline of colrs to gamma bytes */
{
        register int    i, expo;

        if (g_bval == NULL)
                return(-1);
        while (len-- > 0) {
                expo = scan[0][EXP] - COLXS;
                if (expo < -MAXGSHIFT) {
                        if (expo < -MAXGSHIFT-8) {
                                scan[0][RED] =
                                scan[0][GRN] =
                                scan[0][BLU] = 0;
                        } else {
                                i = (-MAXGSHIFT-1) - expo;
                                scan[0][RED] =
                                g_bval[MAXGSHIFT][((scan[0][RED]>>i)+1)>>1];
                                scan[0][GRN] =
                                g_bval[MAXGSHIFT][((scan[0][GRN]>>i)+1)>>1];
                                scan[0][BLU] =
                                g_bval[MAXGSHIFT][((scan[0][BLU]>>i)+1)>>1];
                        }
                } else if (expo > 0) {
                        if (expo > 8) {
                                scan[0][RED] =
                                scan[0][GRN] =
                                scan[0][BLU] = 255;
                        } else {
                                i = (scan[0][RED]<<1 | 1) << (expo-1);
                                scan[0][RED] = i > 255 ? 255 : g_bval[0][i];
                                i = (scan[0][GRN]<<1 | 1) << (expo-1);
                                scan[0][GRN] = i > 255 ? 255 : g_bval[0][i];
                                i = (scan[0][BLU]<<1 | 1) << (expo-1);
                                scan[0][BLU] = i > 255 ? 255 : g_bval[0][i];
                        }
                } else {
                        scan[0][RED] = g_bval[-expo][scan[0][RED]];
                        scan[0][GRN] = g_bval[-expo][scan[0][GRN]];
                        scan[0][BLU] = g_bval[-expo][scan[0][BLU]];
                }
                scan[0][EXP] = COLXS;
                scan++;
        }
        return(0);
}


int gambs_colrs(register COLR *scan, int len)
        /* convert gamma bytes to colr scanline */
{
        register int    nexpo;

        if (g_mant == NULL || g_nexp == NULL)
                return(-1);
        while (len-- > 0) {
                nexpo = g_nexp[scan[0][RED]];
                if (g_nexp[scan[0][GRN]] < nexpo)
                        nexpo = g_nexp[scan[0][GRN]];
                if (g_nexp[scan[0][BLU]] < nexpo)
                        nexpo = g_nexp[scan[0][BLU]];
                if (nexpo < g_nexp[scan[0][RED]])
                        scan[0][RED] = g_mant[scan[0][RED]]
                                        >> (g_nexp[scan[0][RED]]-nexpo);
                else
                        scan[0][RED] = g_mant[scan[0][RED]];
                if (nexpo < g_nexp[scan[0][GRN]])
                        scan[0][GRN] = g_mant[scan[0][GRN]]
                                        >> (g_nexp[scan[0][GRN]]-nexpo);
                else
                        scan[0][GRN] = g_mant[scan[0][GRN]];
                if (nexpo < g_nexp[scan[0][BLU]])
                        scan[0][BLU] = g_mant[scan[0][BLU]]
                                        >> (g_nexp[scan[0][BLU]]-nexpo);
                else
                        scan[0][BLU] = g_mant[scan[0][BLU]];
                scan[0][EXP] = COLXS - nexpo;
                scan++;
        }
        return(0);
}


void
shiftcolrs(register COLR *scan, register int len, register int adjust)
        /* shift a scanline of colors by 2^adjust */
{
        int     minexp;

        if (adjust == 0)
                return;
        minexp = adjust < 0 ? -adjust : 0;
        while (len-- > 0) {
                if (scan[0][EXP] <= minexp)
                        scan[0][RED] = scan[0][GRN] = scan[0][BLU] =
                        scan[0][EXP] = 0;
                else
                        scan[0][EXP] += adjust;
                scan++;
        }
}


void
normcolrs(register COLR *scan, int len, int adjust)
/* normalize a scanline of colrs */
{
        register int  c;
        register int  shift;

        while (len-- > 0) {
                shift = scan[0][EXP] + adjust - COLXS;
                if (shift > 0) {
                        if (shift > 8) {
                                scan[0][RED] =
                                scan[0][GRN] =
                                scan[0][BLU] = 255;
                        } else {
                                shift--;
                                c = (scan[0][RED]<<1 | 1) << shift;
                                scan[0][RED] = c > 255 ? 255 : c;
                                c = (scan[0][GRN]<<1 | 1) << shift;
                                scan[0][GRN] = c > 255 ? 255 : c;
                                c = (scan[0][BLU]<<1 | 1) << shift;
                                scan[0][BLU] = c > 255 ? 255 : c;
                        }
                } else if (shift < 0) {
                        if (shift < -8) {
                                scan[0][RED] =
                                scan[0][GRN] =
                                scan[0][BLU] = 0;
                        } else {
                                shift = -1-shift;
                                scan[0][RED] = ((scan[0][RED]>>shift)+1)>>1;
                                scan[0][GRN] = ((scan[0][GRN]>>shift)+1)>>1;
                                scan[0][BLU] = ((scan[0][BLU]>>shift)+1)>>1;
                        }
                }
                scan[0][EXP] = COLXS - adjust;
                scan++;
        }
}
