/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file binCullHandler.h
 * @author drose
 * @date 2002-02-28
 */

#ifndef BINCULLHANDLER_H
#define BINCULLHANDLER_H

#include "pandabase.h"
#include "cullHandler.h"
#include "cullResult.h"
#include "pointerTo.h"

/**
 * This CullHandler sends all of the geoms it receives into a CullResult
 * object, for binning (and later drawing).  This is the kind of CullHandler
 * to use for most normal rendering needs.
 */
class EXPCL_PANDA_CULL BinCullHandler : public CullHandler {
public:
  INLINE BinCullHandler(CullResult *cull_result);

  virtual void record_object(CullableObject *object,
                             const CullTraverser *traverser);

private:
  PT(CullResult) _cull_result;
};

#include "binCullHandler.I"

#endif
