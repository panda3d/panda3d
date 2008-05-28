/* Filename: bmp.h
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

#ifndef _BMP_H_
#define _BMP_H_

#include "pandabase.h"
#include "pnmimage_base.h"

/* prototypes */
static unsigned long BMPlenfileheader(int classv);
static unsigned long BMPleninfoheader(int classv);
static unsigned long BMPlenrgbtable(int classv, unsigned long bitcount);
static unsigned long BMPlenline(int classv, unsigned long bitcount, unsigned long x);
static unsigned long BMPlenbits(int classv, unsigned long bitcount, unsigned long x, unsigned long y);
static unsigned long BMPlenfile(int classv, unsigned long bitcount, unsigned long x, unsigned long y);
static unsigned long BMPoffbits(int classv, unsigned long bitcount);
/*
 * Classves of BMP files
 */

#define C_WIN   1
#define C_OS2   2

static char     er_internal[] = "%s: internal error!";

static unsigned long
BMPlenfileheader(int classv)
{
        switch (classv)
        {
        case C_WIN:
                return 14;
        case C_OS2:
                return 14;
        default:
                pm_error(er_internal, "BMPlenfileheader");
                return 0;
        }
}

static unsigned long
BMPleninfoheader(int classv)
{
        switch (classv)
        {
        case C_WIN:
                return 40;
        case C_OS2:
                return 12;
        default:
                pm_error(er_internal, "BMPleninfoheader");
                return 0;
        }
}

static unsigned long
BMPlenrgbtable(int classv, unsigned long bitcount)
{
        unsigned long   lenrgb;

        if (bitcount > 8) {
          return 0;
        }

        if (bitcount < 1)
        {
                pm_error(er_internal, "BMPlenrgbtable");
                return 0;
        }
        switch (classv)
        {
        case C_WIN:
                lenrgb = 4;
                break;
        case C_OS2:
                lenrgb = 3;
                break;
        default:
                pm_error(er_internal, "BMPlenrgbtable");
                return 0;
        }

        return (1 << bitcount) * lenrgb;
}

/*
 * length, in bytes, of a line of the image
 *
 * Evidently each row is padded on the right as needed to make it a
 * multiple of 4 bytes long.  This appears to be true of both
 * OS/2 and Windows BMP files.
 */
static unsigned long
BMPlenline(int classv, unsigned long bitcount, unsigned long x)
{
        unsigned long   bitsperline;

        switch (classv)
        {
        case C_WIN:
                break;
        case C_OS2:
                break;
        default:
                pm_error(er_internal, "BMPlenline");
                return 0;
        }

        bitsperline = x * bitcount;

        /*
         * if bitsperline is not a multiple of 32, then round
         * bitsperline up to the next multiple of 32.
         */
        if ((bitsperline % 32) != 0)
        {
                bitsperline += (32 - (bitsperline % 32));
        }

        if ((bitsperline % 32) != 0)
        {
                pm_error(er_internal, "BMPlenline");
                return 0;
        }

        /* number of bytes per line == bitsperline/8 */
        return bitsperline >> 3;
}

/* return the number of bytes used to store the image bits */
static unsigned long
BMPlenbits(
        int             classv,
        unsigned long   bitcount,
        unsigned long   x,
        unsigned long   y)
{
        return y * BMPlenline(classv, bitcount, x);
}

/* return the offset to the BMP image bits */
static unsigned long
BMPoffbits(
        int             classv,
        unsigned long   bitcount)
{
        return BMPlenfileheader(classv)
                + BMPleninfoheader(classv)
                + BMPlenrgbtable(classv, bitcount);
}

/* return the size of the BMP file in bytes */
static unsigned long
BMPlenfile(
        int             classv,
        unsigned long   bitcount,
        unsigned long   x,
        unsigned long   y)
{
        return BMPoffbits(classv, bitcount)
                + BMPlenbits(classv, bitcount, x, y);
}

#endif /* _BMP_H_ */

