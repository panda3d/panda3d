/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file store_pixel.cxx
 * @author drose
 * @date 2008-05-12
 */

#include <stdlib.h>
#include <stdio.h>
#include "zbuffer.h"

/* Pick up all of the generated code references to store_pixel.h. */

#define STORE_PIX_CLAMP(x) (std::min((x), (unsigned int)0xffff))

#include "store_pixel_table.h"
#include "store_pixel_code.h"
