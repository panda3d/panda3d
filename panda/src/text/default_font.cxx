// Filename: default_font.cxx
// Created by:  drose (31Jan03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "default_font.h"

// The binary data included here was generated from an existing font
// file via the utility program bin2c (defined in pandatool).  It is
// used as the default font when no font file is specified by the
// user.

// The particular font we use may come from either of two possible
// sources:

// If we have the Freetype library available, we use uhvr8ac.pfb, a
// PostScript Type1 font installed along with TeX (and it was probably
// converted there from some other format).  It defines the font
// "Nimbus Sans L Regular Condensed", a nice simple sans-serif font.

// If Freetype is not available, we use cmss12.bam, which was in turn
// generated from cmss12.720pk, and defines "Computer Modern Sans
// Serif", a basic Metafont-generated font supplied with TeX.  This
// egg file, by the way, is also distributed with Panda in the models
// tree.

#if defined(COMPILE_IN_DEFAULT_FONT) && !defined(CPPPARSER)

#ifdef HAVE_FREETYPE

// If we have FreeType available, include the pfb font; it's superior
// because it's dynamic.
#include "uhvr8ac.pfb.c"

#elif defined(HAVE_ZLIB)

// If we don't have FreeType, we have to include the bam font, which
// is kind of bulky but at least we can compress it if we have zlib.

// Regenerate this file with (cmss12.egg can be loaded from the models tree):

// egg2bam -rawtex -o cmss12.bam cmss12.egg
// pcompress cmss12.bam
// bin2c -n default_font_data -o cmss12.bam.pz.c cmss12.bam.pz

#include "cmss12.bam.pz.c"

#else

// If we don't even have zlib, just include the whole uncompressed bam
// file.

// Regenerate this file with (cmss12.egg can be loaded from the models tree):

// egg2bam -rawtex -o cmss12.bam cmss12.egg
// bin2c -n default_font_data -o cmss12.bam.c cmss12.bam

#include "cmss12.bam.c"

#endif

const int default_font_size = sizeof(default_font_data);

#endif  // COMPILE_IN_DEFAULT_FONT && !CPPPARSER
