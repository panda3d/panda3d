/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file load_collada_file.h
 * @author rdb
 * @date 2011-03-16
 */

#ifndef LOAD_COLLADA_FILE_H
#define LOAD_COLLADA_FILE_H

#include "pandabase.h"

#include "pandaNode.h"
#include "filename.h"
#include "coordinateSystem.h"

class BamCacheRecord;

BEGIN_PUBLISH
/**
 * A convenience function; the primary interface to this package.  Loads up
 * the indicated DAE file, and returns the root of a scene graph.  Returns
 * NULL if the file cannot be read for some reason.
 */
EXPCL_COLLADA PT(PandaNode)
load_collada_file(const Filename &filename, CoordinateSystem cs = CS_default,
                  BamCacheRecord *record = nullptr);
END_PUBLISH

#endif
