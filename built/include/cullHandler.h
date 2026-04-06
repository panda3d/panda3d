/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullHandler.h
 * @author drose
 * @date 2002-02-23
 */

#ifndef CULLHANDLER_H
#define CULLHANDLER_H

#include "pandabase.h"
#include "cullableObject.h"
#include "graphicsStateGuardianBase.h"

class CullTraverser;

/**
 * This defines the abstract interface for an object that receives Geoms
 * identified by the CullTraverser.  By itself, it's not a particularly useful
 * class; to use it, derive from it and redefine record_object().
 */
class EXPCL_PANDA_PGRAPH CullHandler {
public:
  CullHandler();
  virtual ~CullHandler();

  virtual void record_object(CullableObject &&object,
                             const CullTraverser *traverser);
  virtual void end_traverse();

  INLINE static void draw(CullableObject *object,
                          GraphicsStateGuardianBase *gsg,
                          bool force, Thread *current_thread);
};

#include "cullHandler.I"

#endif
