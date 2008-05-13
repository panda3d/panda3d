// Filename: store_pixel.cxx
// Created by:  drose (12May08)
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

#include <stdlib.h>
#include <stdio.h>
#include "zbuffer.h"

/* Pick up all of the generated code references to store_pixel.h. */

#define STORE_PIX_CLAMP(x) (min((x), (unsigned int)0xffff))

#include "store_pixel_table.h"
#include "store_pixel_code.h"
