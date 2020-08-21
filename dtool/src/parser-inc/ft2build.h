/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ft2build.h
 * @author drose
 * @date 2002-02-08
 */

// This file, and all the other files in this directory, aren't
// intended to be compiled--they're just parsed by CPPParser (and
// interrogate) in lieu of the actual system headers, to generate the
// interrogate database.

#ifndef FT2BUILD_H
#define FT2BUILD_H

// This definition is intentionally recursive.  Why complicate things
// with multiple files?
#define FT_FREETYPE_H <ft2build.h>
#define FT_OUTLINE_H <ft2build.h>

class FT_Face;
class FT_Library;
class FT_Bitmap;
class FT_Vector;
class FT_Span;
class FT_Outline;

#endif

