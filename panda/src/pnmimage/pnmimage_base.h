/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmimage_base.h
 * @author drose
 * @date 2000-06-14
 */

#ifndef PNMIMAGE_BASE_H
#define PNMIMAGE_BASE_H

// This header file make a few typedefs and other definitions essential to
// everything in the PNMImage package.

#include "pandabase.h"
#include "pnotify.h"

// Since we no longer include pnm.h directly, we have to provide our own
// definitions for xel and xelval.

// For now, we have PGM_BIGGRAYS defined, which gives us 16-bit channels.
// Undefine this if you have memory problems and need to use 8-bit channels
// instead.
#define PGM_BIGGRAYS

#ifdef PGM_BIGGRAYS
typedef unsigned short gray;
#define PGM_MAXMAXVAL 65535
#else  // PGM_BIGGRAYS
typedef unsigned char gray;
#define PGM_MAXMAXVAL 255
#endif  // PGM_BIGGRAYS

#define PNM_MAXMAXVAL PGM_MAXMAXVAL

struct pixel {
PUBLISHED:
  pixel() = default;
  pixel(gray fill) : r(fill), g(fill), b(fill) { }
  pixel(gray r, gray g, gray b) : r(r), g(g), b(b) { }

  gray operator [](int i) const { nassertr(i >= 0 && i < 3, 0); return *(&r + i); }
  gray &operator [](int i) { nassertr(i >= 0 && i < 3, r); return *(&r + i); }
  pixel operator + (const pixel &other) const
    { return pixel(r + other.r, g + other.g, b + other.b); }
  pixel operator - (const pixel &other) const
    { return pixel(r - other.r, g - other.g, b - other.b); }
  pixel operator * (const double mult) const
    { return pixel(r * mult, g * mult, b * mult); }
  void operator += (const pixel &other)
    { r += other.r; g += other.g; b += other.b; }
  void operator -= (const pixel &other)
    { r -= other.r; g -= other.g; b -= other.b; }
  void operator *= (const double mult)
    { r *= mult; g *= mult; b *= mult; }

  bool operator == (const pixel &other) {
    return r == other.r && g == other.g && r == other.r;
  }
  bool operator != (const pixel &other) {
    return r != other.r || g != other.g || r != other.r;
  }
  bool operator < (const pixel &other) const {
    if (r != other.r) {
      return r < other.r;
    }
    if (g != other.g) {
      return g < other.g;
    }
    return b < other.b;
  }

#ifdef HAVE_PYTHON
  static int size() { return 3; }
  void output(std::ostream &out) {
    out << "pixel(r=" << r << ", g=" << g << ", b=" << b << ")";
  }
#endif

  gray r, g, b;
};

typedef gray pixval;
typedef pixel xel;
typedef gray xelval;

// These macros are borrowed from ppm.h.

#define PPM_GETR(p) ((p).r)
#define PPM_GETG(p) ((p).g)
#define PPM_GETB(p) ((p).b)

#define PPM_PUTR(p,red) ((p).r = (red))
#define PPM_PUTG(p,grn) ((p).g = (grn))
#define PPM_PUTB(p,blu) ((p).b = (blu))

#define PPM_ASSIGN(p,red,grn,blu) { (p).r = (red); (p).g = (grn); (p).b = (blu); }
#define PPM_EQUAL(p,q) ( (p).r == (q).r && (p).g == (q).g && (p).b == (q).b )
#define PNM_ASSIGN1(x,v) PPM_ASSIGN(x,0,0,v)

#define PPM_DEPTH(newp,p,oldmaxval,newmaxval) \
    PPM_ASSIGN( (newp), \
        ( (int) PPM_GETR(p) * (newmaxval) + (oldmaxval) / 2 ) / (oldmaxval), \
        ( (int) PPM_GETG(p) * (newmaxval) + (oldmaxval) / 2 ) / (oldmaxval), \
        ( (int) PPM_GETB(p) * (newmaxval) + (oldmaxval) / 2 ) / (oldmaxval) )


// pnm defines these functions, and it's easier to emulate them than to
// rewrite the code that calls them.
EXPCL_PANDA_PNMIMAGE void pm_message(const char *format, ...);
EXPCL_PANDA_PNMIMAGE void pm_error(const char *format, ...);  // doesn't return.

EXPCL_PANDA_PNMIMAGE int pm_maxvaltobits(int maxval);
EXPCL_PANDA_PNMIMAGE int pm_bitstomaxval(int bits);

EXPCL_PANDA_PNMIMAGE char *pm_allocrow(int cols, int size);
EXPCL_PANDA_PNMIMAGE void pm_freerow(char *itrow);

EXPCL_PANDA_PNMIMAGE int pm_readbigshort(std::istream *in, short *sP);
EXPCL_PANDA_PNMIMAGE int pm_writebigshort(std::ostream *out, short s);
EXPCL_PANDA_PNMIMAGE int pm_readbiglong(std::istream *in, long *lP);
EXPCL_PANDA_PNMIMAGE int pm_writebiglong(std::ostream *out, long l);
EXPCL_PANDA_PNMIMAGE int pm_readlittleshort(std::istream *in, short *sP);
EXPCL_PANDA_PNMIMAGE int pm_writelittleshort(std::ostream *out, short s);
EXPCL_PANDA_PNMIMAGE int pm_readlittlelong(std::istream *in, long *lP);
EXPCL_PANDA_PNMIMAGE int pm_writelittlelong(std::ostream *out, long l);


// These ratios are used to compute the brightness of a colored pixel; they
// define the relative contributions of each of the components.
static const float lumin_red = 0.299f;
static const float lumin_grn = 0.587f;
static const float lumin_blu = 0.114f;


#endif
