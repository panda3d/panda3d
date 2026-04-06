/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vector_PT_EggTexture.h
 * @author drose
 * @date 2001-05-01
 */

#ifndef VECTOR_PT_EGGTEXTURE_H
#define VECTOR_PT_EGGTEXTURE_H

#include "pandabase.h"

#include "eggTexture.h"
#include "pt_EggTexture.h"

#include "pvector.h"

/**
 * A vector of PT(EggTexture)'s.  This class is defined once here, and
 * exported to PANDAEGG.DLL; other packages that want to use a vector of this
 * type (whether they need to export it or not) should include this header
 * file, rather than defining the vector again.
 */

#define EXPCL EXPCL_PANDA_EGG
#define EXPTP EXPTP_PANDA_EGG
#define TYPE PT_EggTexture
#define NAME vector_PT_EggTexture

#include "vector_src.h"

#endif
