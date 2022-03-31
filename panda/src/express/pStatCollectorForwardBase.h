/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatCollectorForwardBase.h
 * @author drose
 * @date 2006-10-30
 */

#ifndef PSTATCOLLECTORFORWARDBASE_H
#define PSTATCOLLECTORFORWARDBASE_H

#include "pandabase.h"
#include "referenceCount.h"

/**
 * This class serves as a cheap forward reference to a PStatCollector, which
 * is defined in the pstatclient module (and is not directly accessible here
 * in the express module).
 *
 * This is subclassed as PStatCollectorForward, which defines the actual
 * functionality.
 */
class EXPCL_PANDA_EXPRESS PStatCollectorForwardBase : public ReferenceCount {
PUBLISHED:
  virtual ~PStatCollectorForwardBase();

#ifdef DO_PSTATS
  virtual void add_level(double level)=0;
#else
  // We still need to declare a virtual destructor for ABI compatibility, so
  // that a vtable is created.
  INLINE void add_level(double level) { }
#endif
};

#endif
