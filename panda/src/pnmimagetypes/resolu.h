/* Filename: resolu.h
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

/* SCCSid "@(#)resolu.h 2.2 6/4/93 LBL" */

/*
 * Definitions for resolution line in image file.
 *
 * True image orientation is defined by an xy coordinate system
 * whose origin is at the lower left corner of the image, with
 * x increasing to the right and y increasing in the upward direction.
 * This true orientation is independent of how the pixels are actually
 * ordered in the file, which is indicated by the resolution line.
 * This line is of the form "{+-}{XY} xyres {+-}{YX} yxres\n".
 * A typical line for a 1024x600 image might be "-Y 600 +X 1024\n",
 * indicating that the scanlines are in English text order (PIXSTANDARD).
 */

                        /* flags for scanline ordering */
#define  XDECR                  1
#define  YDECR                  2
#define  YMAJOR                 4

                        /* standard scanline ordering */
#define  PIXSTANDARD            (YMAJOR|YDECR)
#define  PIXSTDFMT              "-Y %d +X %d\n"

                        /* structure for image dimensions */
typedef struct {
        int     orient;         /* orientation (from flags above) */
        int     xr, yr;         /* x and y resolution */
} RESOLU;

                        /* macros to get scanline length and number */
#define  scanlen(rs)            ((rs)->or & YMAJOR ? (rs)->xr : (rs)->yr)
#define  numscans(rs)           ((rs)->or & YMAJOR ? (rs)->yr : (rs)->xr)

                        /* resolution string buffer and its size */
#define  RESOLU_BUFLEN          32
extern char  resolu_buf[RESOLU_BUFLEN];

                        /* macros for reading/writing resolution struct */
#define  fputsresolu(rs,fp)     fputs(resolu2str(resolu_buf,rs),fp)
#define  fgetsresolu(rs,fp)     str2resolu(rs, \
                                        fgets(resolu_buf,RESOLU_BUFLEN,fp))

                        /* reading/writing of standard ordering */
#define  fprtresolu(sl,ns,fp)   fprintf(fp,PIXSTDFMT,ns,sl)
#define  fscnresolu(sl,ns,fp)   (fscanf(fp,PIXSTDFMT,ns,sl)==2)

extern char  *resolu2str();
