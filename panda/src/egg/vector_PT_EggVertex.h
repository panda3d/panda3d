/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vector_PT_EggVertex.h
 * @author drose
 * @date 2001-02-22
 */

#ifndef VECTOR_PT_EGGVERTEX_H
#define VECTOR_PT_EGGVERTEX_H

#include "pandabase.h"

#include "eggVertex.h"
#include "pt_EggVertex.h"

#include "pvector.h"

/**
 * A vector of PT(EggVertex)'s.  This class is defined once here, and exported
 * to PANDAEGG.DLL; other packages that want to use a vector of this type
 * (whether they need to export it or not) should include this header file,
 * rather than defining the vector again.
 */

#define EXPCL EXPCL_PANDA_EGG
#define EXPTP EXPTP_PANDA_EGG
#define TYPE PT_EggVertex
#define NAME vector_PT_EggVertex

#include "vector_src.h"

#endif
