// Filename: default_font.cxx
// Created by:  drose (31Jan03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "default_font.h"

// The binary data included here was generated from an existing font
// file via the utility program bin2c (defined in pandatool).  It is
// used as the default font when no font file is specified by the
// user.

// The particular font we use may come from either of two possible
// sources:

// If we have the Freetype library available, we use persans.ttf, part
// of the "Perspective Sans" font family downloaded from
// http://www.fontsquirrel.com/fonts/Perspective-Sans, which appears
// to be freeware.
//
// If Freetype is not available, we use cmss12.bam, which was in turn
// generated from cmss12.720pk, and defines "Computer Modern Sans
// Serif", a basic Metafont-generated font supplied with TeX.  This
// egg file, by the way, is also distributed with Panda in the models
// tree.

#if defined(COMPILE_IN_DEFAULT_FONT) && !defined(CPPPARSER)

#ifdef HAVE_FREETYPE

// If we have FreeType available, include the ttf font; it's superior
// because it's dynamic.  See above.
#include "persans.ttf_src.c"

#elif defined(HAVE_ZLIB)

// If we don't have FreeType, we have to include the bam font, which
// is kind of bulky but at least we can compress it if we have zlib.

// Regenerate this file with (cmss12.egg can be loaded from the models tree):

// egg2bam -rawtex -o cmss12.bam cmss12.egg
// pzip cmss12.bam
// bin2c -n default_font_data -o cmss12.bam.pz_src.c cmss12.bam.pz

#include "cmss12.bam.pz_src.c"

#else

// If we don't even have zlib, just include the whole uncompressed bam
// file.

// Regenerate this file with (cmss12.egg can be loaded from the models tree):

// egg2bam -rawtex -o cmss12.bam cmss12.egg
// bin2c -n default_font_data -o cmss12.bam_src.c cmss12.bam

#include "cmss12.bam_src.c"

#endif

const int default_font_size = sizeof(default_font_data);

#endif  // COMPILE_IN_DEFAULT_FONT && !CPPPARSER
