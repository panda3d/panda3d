/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file load_egg_file.h
 * @author drose
 * @date 2002-02-26
 */

#ifndef LOAD_EGG_FILE_H
#define LOAD_EGG_FILE_H

#include "pandabase.h"

#include "pandaNode.h"
#include "coordinateSystem.h"
#include "eggData.h"

class BamCacheRecord;

BEGIN_PUBLISH
/**
 * A convenience function; the primary interface to this package.  Loads up
 * the indicated egg file, and returns the root of a scene graph.  Returns
 * NULL if the file cannot be read for some reason.
 *
 * Also see the EggLoader class, which can exercise a bit more manual control
 * over the loading process.
 */
EXPCL_PANDA_EGG2PG PT(PandaNode)
load_egg_file(const Filename &filename, CoordinateSystem cs = CS_default,
              BamCacheRecord *record = nullptr);

/**
 * Another convenience function; works like load_egg_file() but starts from an
 * already-filled EggData structure.  The structure is destroyed in the
 * loading.
 */
EXPCL_PANDA_EGG2PG PT(PandaNode)
load_egg_data(EggData *data, CoordinateSystem cs = CS_default);
END_PUBLISH

#endif
