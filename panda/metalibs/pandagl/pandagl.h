/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandagl.h
 * @author drose
 * @date 2000-05-15
 */

#ifndef PANDAGL_H
#define PANDAGL_H

#include "pandabase.h"

EXPCL_PANDAGL void init_libpandagl();
extern "C" EXPCL_PANDAGL int get_pipe_type_pandagl();

#endif
