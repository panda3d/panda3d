/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file save_egg_file.h
 * @author drose
 * @date 2012-12-19
 */

#ifndef SAVE_EGG_FILE_H
#define SAVE_EGG_FILE_H

#include "pandabase.h"

#include "pandaNode.h"
#include "coordinateSystem.h"
#include "eggData.h"

BEGIN_PUBLISH
/**
 * A convenience function; converts the indicated scene graph to an egg file
 * and writes it to disk.
 */
EXPCL_PANDA_EGG2PG bool
save_egg_file(const Filename &filename, PandaNode *node,
              CoordinateSystem cs = CS_default);

/**
 * Another convenience function; works like save_egg_file() but populates an
 * EggData instead of writing the results to disk.
 */
EXPCL_PANDA_EGG2PG bool
save_egg_data(EggData *data, PandaNode *node);
END_PUBLISH

#endif
