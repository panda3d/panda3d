#ifndef lint
static char rcsid[] = "$Header$";
#endif

/*
 * Copyright (c) 1992 Sam Leffler
 * Copyright (c) 1992 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 * 
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

/*
 * TIFF Library Machine Dependent Routines.
 */
#include "tiffiop.h"

#ifdef tahoe
typedef struct ieeedouble {
        u_long  sign    : 1,
                exp     : 11,
                mant    : 20;
        u_long  mant2;
} ieeedouble;
typedef struct ieeefloat {
        u_long  sign    : 1,
                exp     : 8,
                mant    : 23;
} ieeefloat;

typedef struct {
        u_long  sign    : 1,
                exp     : 8,
                mant    : 23;
        u_long  mant2;
} nativedouble;
typedef struct {
        u_long  sign    : 1,
                exp     : 8,
                mant    : 23;
} nativefloat;
/*
 * Beware, over/under-flow in conversions will
 * result in garbage values -- handling it would
 * require a subroutine call or lots more code.
 */
#define NATIVE2IEEEFLOAT(fp) { \
    if ((fp)->native.exp) \
        (fp)->ieee.exp = (fp)->native.exp - 129 + 127;  /* alter bias */\
}
#define IEEEFLOAT2NATIVE(fp) { \
    if ((fp)->ieee.exp) \
        (fp)->native.exp = (fp)->ieee.exp - 127 + 129;  /* alter bias */\
}
#define IEEEDOUBLE2NATIVE(dp) { \
    if ((dp)->native.exp = (dp)->ieee.exp) \
        (dp)->native.exp += -1023 + 129; \
    (dp)->native.mant = ((dp)->ieee.mant<<3)|((dp)->native.mant2>>29); \
    (dp)->native.mant2 <<= 3; \
}
#endif /* tahoe */

#ifdef vax
typedef struct ieeedouble {
        u_long  mant    : 20,
                exp     : 11,
                sign    : 1;
        u_long  mant2;
} ieeedouble;
typedef struct ieeefloat {
        u_long  mant    : 23,
                exp     : 8,
                sign    : 1;
} ieeefloat;

typedef struct {
        u_long  mant1   : 7,
                exp     : 8,
                sign    : 1,
                mant2   : 16;
        u_long  mant3;
} nativedouble;
typedef struct {
        u_long  mant1   : 7,
                exp     : 8,
                sign    : 1,
                mant2   : 16;
} nativefloat;
/*
 * Beware, these do not handle over/under-flow
 * during conversion from ieee to native format.
 */
#define NATIVE2IEEEFLOAT(fp) { \
    float_t t; \
    if (t.ieee.exp = (fp)->native.exp) \
        t.ieee.exp += -129 + 127; \
    t.ieee.sign = (fp)->native.sign; \
    t.ieee.mant = ((fp)->native.mant1<<16)|(fp)->native.mant2; \
    *(fp) = t; \
}
#define IEEEFLOAT2NATIVE(fp) { \
    float_t t; int v = (fp)->ieee.exp; \
    if (v) v += -127 + 129;             /* alter bias of exponent */\
    t.native.exp = v;                   /* implicit truncation of exponent */\
    t.native.sign = (fp)->ieee.sign; \
    v = (fp)->ieee.mant; \
    t.native.mant1 = v >> 16; \
    t.native.mant2 = v;\
    *(fp) = t; \
}
#define IEEEDOUBLE2NATIVE(dp) { \
    double_t t; int v = (dp)->ieee.exp; \
    if (v) v += -1023 + 1025;           /* if can alter bias of exponent */\
    t.native.exp = v;                   /* implicit truncation of exponent */\
    v = (dp)->ieee.mant; \
    t.native.sign = (dp)->ieee.sign; \
    t.native.mant1 = v >> 16; \
    t.native.mant2 = v;\
    t.native.mant3 = (dp)->mant2; \
    *(dp) = t; \
}
#endif /* vax */

#if !HAVE_IEEEFP
#if !defined(IEEEFLOAT2NATIVE) || !defined(NATIVE2IEEEFLOAT)
"Help, you've configured the library to not have IEEE floating point,\
but not defined how to convert between IEEE and native formats!"
#endif

/*
 * These unions are used during floating point
 * conversions.  The above macros define the
 * conversion operations.
 */
typedef union {
        ieeedouble      ieee;
        nativedouble    native;
        char            b[8];
        double          d;
} double_t;

typedef union {
        ieeefloat       ieee;
        nativefloat     native;
        char            b[4];
        float           f;
} float_t;

void
TIFFCvtIEEEFloatToNative(TIFF* tif, u_int n, float* f)
{
        float_t *fp = (float_t *)f;

        while (n-- > 0) {
                IEEEFLOAT2NATIVE(fp);
                fp++;
        }
}

void
TIFFCvtNativeToIEEEFloat(TIFF* tif, u_int n, float* f)
{
        float_t *fp = (float_t *)f;

        while (n-- > 0) {
                NATIVE2IEEEFLOAT(fp);
                fp++;
        }
}
#endif
