/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file default_font.h
 * @author drose
 * @date 2003-01-31
 */

#ifndef DEFAULT_FONT_H
#define DEFAULT_FONT_H

#include "pandabase.h"

#if defined(COMPILE_IN_DEFAULT_FONT) && !defined(CPPPARSER)

extern EXPCL_PANDA_TEXT const unsigned char default_font_data[];
extern EXPCL_PANDA_TEXT const int default_font_size;

#endif  // HAVE_FREETYPE && COMPILE_IN_DEFAULT_FONT && !CPPPARSER

#endif
