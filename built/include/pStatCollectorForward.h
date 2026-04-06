/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatCollectorForward.h
 * @author drose
 * @date 2006-10-30
 */

#ifndef PSTATCOLLECTORFORWARD_H
#define PSTATCOLLECTORFORWARD_H

#include "pandabase.h"
#include "pStatCollectorForwardBase.h"
#include "pStatCollector.h"

/**
 * This class serves as a cheap forward reference to a PStatCollector, so that
 * classes that are defined before the pstats module may access the
 * PStatCollector.
 */
class EXPCL_PANDA_PSTATCLIENT PStatCollectorForward : public PStatCollectorForwardBase {
PUBLISHED:
  INLINE PStatCollectorForward(const PStatCollector &col);

  virtual void add_level(double level);

#ifdef DO_PSTATS
private:
  PStatCollector _col;
#endif
};

#include "pStatCollectorForward.I"

#endif
