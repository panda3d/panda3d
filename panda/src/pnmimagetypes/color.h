/* Filename: color.h
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

/* SCCSid "@(#)color.h 2.9 9/2/94 LBL" */

/*
 *  color.h - header for routines using pixel color values.
 *
 *     12/31/85
 *
 *  Two color representations are used, one for calculation and
 *  another for storage.  Calculation is done with three floats
 *  for speed.  Stored color values use 4 bytes which contain
 *  three single byte mantissas and a common exponent.
 */

#define  RED            0
#define  GRN            1
#define  BLU            2
#define  EXP            3
#define  COLXS          128     /* excess used for exponent */

typedef unsigned char  BYTE;    /* 8-bit unsigned integer */

typedef BYTE  COLR[4];          /* red, green, blue, exponent */

#define  copycolr(c1,c2)        (c1[0]=c2[0],c1[1]=c2[1], \
                                c1[2]=c2[2],c1[3]=c2[3])

typedef float  COLOR[3];        /* red, green, blue */

#define  colval(col,pri)        ((col)[pri])

#define  setcolor(col,r,g,b)    ((col)[RED]=(r),(col)[GRN]=(g),(col)[BLU]=(b))

#define  copycolor(c1,c2)       ((c1)[0]=(c2)[0],(c1)[1]=(c2)[1],(c1)[2]=(c2)[2])

#define  scalecolor(col,sf)     ((col)[0]*=(sf),(col)[1]*=(sf),(col)[2]*=(sf))

#define  addcolor(c1,c2)        ((c1)[0]+=(c2)[0],(c1)[1]+=(c2)[1],(c1)[2]+=(c2)[2])

#define  multcolor(c1,c2)       ((c1)[0]*=(c2)[0],(c1)[1]*=(c2)[1],(c1)[2]*=(c2)[2])

#ifdef  NTSC
#define  CIE_x_r                0.670           /* standard NTSC primaries */
#define  CIE_y_r                0.330
#define  CIE_x_g                0.210
#define  CIE_y_g                0.710
#define  CIE_x_b                0.140
#define  CIE_y_b                0.080
#define  CIE_x_w                0.3333          /* use true white */
#define  CIE_y_w                0.3333
#else
#define  CIE_x_r                0.640           /* nominal CRT primaries */
#define  CIE_y_r                0.330
#define  CIE_x_g                0.290
#define  CIE_y_g                0.600
#define  CIE_x_b                0.150
#define  CIE_y_b                0.060
#define  CIE_x_w                0.3333          /* use true white */
#define  CIE_y_w                0.3333
#endif

#define CIE_D           (       CIE_x_r*(CIE_y_g - CIE_y_b) + \
                                CIE_x_g*(CIE_y_b - CIE_y_r) + \
                                CIE_x_b*(CIE_y_r - CIE_y_g)     )
#define CIE_C_rD        ( (1./CIE_y_w) * \
                                ( CIE_x_w*(CIE_y_g - CIE_y_b) - \
                                  CIE_y_w*(CIE_x_g - CIE_x_b) + \
                                  CIE_x_g*CIE_y_b - CIE_x_b*CIE_y_g     ) )
#define CIE_C_gD        ( (1./CIE_y_w) * \
                                ( CIE_x_w*(CIE_y_b - CIE_y_r) - \
                                  CIE_y_w*(CIE_x_b - CIE_x_r) - \
                                  CIE_x_r*CIE_y_b + CIE_x_b*CIE_y_r     ) )
#define CIE_C_bD        ( (1./CIE_y_w) * \
                                ( CIE_x_w*(CIE_y_r - CIE_y_g) - \
                                  CIE_y_w*(CIE_x_r - CIE_x_g) + \
                                  CIE_x_r*CIE_y_g - CIE_x_g*CIE_y_r     ) )

#define CIE_rf          (CIE_y_r*CIE_C_rD/CIE_D)
#define CIE_gf          (CIE_y_g*CIE_C_gD/CIE_D)
#define CIE_bf          (CIE_y_b*CIE_C_bD/CIE_D)

/* As of 9-94, CIE_rf=.265074126, CIE_gf=.670114631 and CIE_bf=.064811243 */

#define  bright(col)    (CIE_rf*(col)[RED]+CIE_gf*(col)[GRN]+CIE_bf*(col)[BLU])
#define  normbright(c)  ( ( (long)(CIE_rf*256.+.5)*(c)[RED] + \
                            (long)(CIE_gf*256.+.5)*(c)[GRN] + \
                            (long)(CIE_bf*256.+.5)*(c)[BLU] ) >> 8 )

                                /* luminous efficacies over visible spectrum */
#define  MAXEFFICACY            683.            /* defined maximum at 550 nm */
#define  WHTEFFICACY            179.            /* uniform white light */
#define  D65EFFICACY            203.            /* standard illuminant D65 */
#define  INCEFFICACY            160.            /* illuminant A (incand.) */
#define  SUNEFFICACY            208.            /* illuminant B (solar dir.) */
#define  SKYEFFICACY            D65EFFICACY     /* skylight */
#define  DAYEFFICACY            D65EFFICACY     /* combined sky and solar */

#define  luminance(col)         (WHTEFFICACY * bright(col))

#define  intens(col)            ( (col)[0] > (col)[1] \
                                ? (col)[0] > (col)[2] ? (col)[0] : (col)[2] \
                                : (col)[1] > (col)[2] ? (col)[1] : (col)[2] )

#define  colrval(c,p)           ( (c)[EXP] ? \
                                ldexp((c)[p]+.5,(int)(c)[EXP]-(COLXS+8)) : \
                                0. )

#define  WHTCOLOR               {1.0,1.0,1.0}
#define  BLKCOLOR               {0.0,0.0,0.0}
#define  WHTCOLR                {128,128,128,COLXS+1}
#define  BLKCOLR                {0,0,0,0}

                                /* picture format identifier */
#define  COLRFMT                "32-bit_rle_rgbe"

                                /* macros for exposures */
#define  EXPOSSTR               "EXPOSURE="
#define  LEXPOSSTR              9
#define  isexpos(hl)            (!strncmp(hl,EXPOSSTR,LEXPOSSTR))
#define  exposval(hl)           atof((hl)+LEXPOSSTR)
#define  fputexpos(ex,fp)       fprintf(fp,"%s%e\n",EXPOSSTR,ex)

                                /* macros for pixel aspect ratios */
#define  ASPECTSTR              "PIXASPECT="
#define  LASPECTSTR             10
#define  isaspect(hl)           (!strncmp(hl,ASPECTSTR,LASPECTSTR))
#define  aspectval(hl)          atof((hl)+LASPECTSTR)
#define  fputaspect(pa,fp)      fprintf(fp,"%s%f\n",ASPECTSTR,pa)

                                /* macros for color correction */
#define  COLCORSTR              "COLORCORR="
#define  LCOLCORSTR             10
#define  iscolcor(hl)           (!strncmp(hl,COLCORSTR,LCOLCORSTR))
#define  colcorval(cc,hl)       sscanf(hl+LCOLCORSTR,"%f %f %f", \
                                        &(cc)[RED],&(cc)[GRN],&(cc)[BLU])
#define  fputcolcor(cc,fp)      fprintf(fp,"%s %f %f %f\n",COLCORSTR, \
                                        (cc)[RED],(cc)[GRN],(cc)[BLU])

#ifdef  DCL_ATOF
extern double  atof(), ldexp(), frexp();
#endif
