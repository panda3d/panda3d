/* Filename: color.c
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

/*
 *  color.c - routines for color calculations.
 *
 *     10/10/85
 */

#include  <stdio.h>

#include  <math.h>

#include  "color.h"

#ifdef WIN32_VC
#include <malloc.h>
#endif

#define  MINELEN        8       /* minimum scanline length for encoding */
#define  MAXELEN        0x7fff  /* maximum scanline length for encoding */
#define  MINRUN         4       /* minimum run length */

void setcolr(register COLR, double, double, double);
void colr_color(register COLOR, register COLR);


char *
tempbuffer(unsigned len)                /* get a temporary buffer */
{
#ifndef WIN32_VC
        extern char  *malloc(size_t), *realloc(void *, size_t);
#endif
        static char  *tempbuf = NULL;
        static unsigned  tempbuflen = 0;

        if (len > tempbuflen) {
                if (tempbuflen > 0)
                        tempbuf = (char *)realloc(tempbuf, len);
                else
                        tempbuf = (char *)malloc(len);
                tempbuflen = tempbuf==NULL ? 0 : len;
        }
        return(tempbuf);
}


int fwritecolrs(register COLR *scanline, unsigned len, register FILE *fp)
                /* write out a colr scanline */
{
        register int  i, j, beg, cnt;
        int  c2;

        if (len < MINELEN || len > MAXELEN)     /* OOBs, write out flat */
                return(fwrite((char *)scanline,sizeof(COLR),len,fp) - len);
                                        /* put magic header */
        putc(2, fp);
        putc(2, fp);
        putc(len>>8, fp);
        putc(len&255, fp);
                                        /* put components seperately */
        for (i = 0; i < 4; i++) {
            for (j = 0; j < (int)len; j += cnt) {    /* find next run */
                for (beg = j; beg < (int)len; beg += cnt) {
                    for (cnt = 1; cnt < 127 && ((beg+cnt) < (int)len) &&
                            (scanline[beg+cnt][i] == scanline[beg][i]); cnt++)
                        ;
                    if (cnt >= MINRUN)
                        break;                  /* long enough */
                }
                if (beg-j > 1 && beg-j < MINRUN) {
                    c2 = j+1;
                    while (scanline[c2++][i] == scanline[j][i])
                        if (c2 == beg) {        /* short run */
                            putc(128+beg-j, fp);
                            putc(scanline[j][i], fp);
                            j = beg;
                            break;
                        }
                }
                while (j < beg) {               /* write out non-run */
                    if ((c2 = beg-j) > 128) c2 = 128;
                    putc(c2, fp);
                    while (c2--)
                        putc(scanline[j++][i], fp);
                }
                if (cnt >= MINRUN) {            /* write out run */
                    putc(128+cnt, fp);
                    putc(scanline[beg][i], fp);
                } else
                    cnt = 0;
            }
        }
        return(ferror(fp) ? -1 : 0);
}

int oldreadcolrs(register COLR *scanline, int len, register FILE *fp);

int freadcolrs(register COLR *scanline, int len, register FILE *fp)
                /* read in an encoded colr scanline */
{
        register int  i, j;
        int  code, val;
                                        /* determine scanline type */
        if (len < MINELEN || len > MAXELEN)
                return(oldreadcolrs(scanline, len, fp));
        if ((i = getc(fp)) == EOF)
                return(-1);
        if (i != 2) {
                ungetc(i, fp);
                return(oldreadcolrs(scanline, len, fp));
        }
        scanline[0][GRN] = getc(fp);
        scanline[0][BLU] = getc(fp);
        if ((i = getc(fp)) == EOF)
                return(-1);
        if (scanline[0][GRN] != 2 || scanline[0][BLU] & 128) {
                scanline[0][RED] = 2;
                scanline[0][EXP] = i;
                return(oldreadcolrs(scanline+1, len-1, fp));
        }
        if ((scanline[0][BLU]<<8 | i) != len)
                return(-1);             /* length mismatch! */
                                        /* read each component */
        for (i = 0; i < 4; i++)
            for (j = 0; j < len; ) {
                if ((code = getc(fp)) == EOF)
                    return(-1);
                if (code > 128) {       /* run */
                    code &= 127;
                    val = getc(fp);
                    while (code--)
                        scanline[j++][i] = val;
                } else                  /* non-run */
                    while (code--)
                        scanline[j++][i] = getc(fp);
            }
        return(feof(fp) ? -1 : 0);
}


                /* read in an old colr scanline */
int oldreadcolrs(register COLR *scanline, int len, register FILE *fp)
{
        int  rshift;
        register int  i;

        rshift = 0;

        while (len > 0) {
                scanline[0][RED] = getc(fp);
                scanline[0][GRN] = getc(fp);
                scanline[0][BLU] = getc(fp);
                scanline[0][EXP] = getc(fp);
                if (feof(fp) || ferror(fp))
                        return(-1);
                if (scanline[0][RED] == 1 &&
                                scanline[0][GRN] == 1 &&
                                scanline[0][BLU] == 1) {
                        for (i = scanline[0][EXP] << rshift; i > 0; i--) {
                                copycolr(scanline[0], scanline[-1]);
                                scanline++;
                                len--;
                        }
                        rshift += 8;
                } else {
                        scanline++;
                        len--;
                        rshift = 0;
                }
        }
        return(0);
}


        /* write out a scanline */
int fwritescan(register COLOR *scanline, int len, FILE *fp)
{
        COLR  *clrscan;
        int  n;
        register COLR  *sp;
                                        /* get scanline buffer */
        if ((sp = (COLR *)tempbuffer(len*sizeof(COLR))) == NULL)
                return(-1);
        clrscan = sp;
                                        /* convert scanline */
        n = len;
        while (n-- > 0) {
                setcolr(sp[0], scanline[0][RED],
                                  scanline[0][GRN],
                                  scanline[0][BLU]);
                scanline++;
                sp++;
        }
        return(fwritecolrs(clrscan, len, fp));
}


int freadscan(register COLOR *scanline, int len, FILE *fp)
                        /* read in a scanline */
{
        register COLR  *clrscan;

        if ((clrscan = (COLR *)tempbuffer(len*sizeof(COLR))) == NULL)
                return(-1);
        if (freadcolrs(clrscan, len, fp) < 0)
                return(-1);
                                        /* convert scanline */
        colr_color(scanline[0], clrscan[0]);
        while (--len > 0) {
                scanline++; clrscan++;
                if (clrscan[0][RED] == clrscan[-1][RED] &&
                            clrscan[0][GRN] == clrscan[-1][GRN] &&
                            clrscan[0][BLU] == clrscan[-1][BLU] &&
                            clrscan[0][EXP] == clrscan[-1][EXP])
                        copycolor(scanline[0], scanline[-1]);
                else
                        colr_color(scanline[0], clrscan[0]);
        }
        return(0);
}


void
setcolr(register COLR clr, double r, double g, double b)
                        /* assign a short color value */
{
        double  d;
        int  e;

        d = r > g ? r : g;
        if (b > d) d = b;

        if (d <= 1e-32) {
                clr[RED] = clr[GRN] = clr[BLU] = 0;
                clr[EXP] = 0;
                return;
        }

        d = frexp(d, &e) * 255.9999 / d;

        clr[RED] = (BYTE) (r * d);
        clr[GRN] = (BYTE) (g * d);
        clr[BLU] = (BYTE) (b * d);
        clr[EXP] = (BYTE) (e) + COLXS;
}


void
colr_color(register COLOR col, register COLR clr)
                        /* convert short to float color */
{
        double  f;

        if (clr[EXP] == 0)
                col[RED] = col[GRN] = col[BLU] = 0.0;
        else {
                f = ldexp(1.0, (int)clr[EXP]-(COLXS+8));
                col[RED] = (float) ((clr[RED] + 0.5)*f);
                col[GRN] = (float) ((clr[GRN] + 0.5)*f);
                col[BLU] = (float) ((clr[BLU] + 0.5)*f);
        }
}


int
bigdiff(register COLOR c1, register COLOR c2, double md)
                        /* c1 delta c2 > md? */
{
        register int  i;

        for (i = 0; i < 3; i++)
                if (colval(c1,i)-colval(c2,i) > md*colval(c2,i) ||
                        colval(c2,i)-colval(c1,i) > md*colval(c1,i))
                        return(1);
        return(0);
}
