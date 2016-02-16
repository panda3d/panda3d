/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgButtonNotify.h
 * @author drose
 * @date 2005-08-18
 */

#ifndef PGBUTTONNOTIFY_H
#define PGBUTTONNOTIFY_H

#include "pandabase.h"
#include "pgItemNotify.h"

class PGButton;

/**
 * Objects that inherit from this class can receive notify messages when a
 * slider bar moves or otherwise is reconfigured.
 */
class EXPCL_PANDA_PGUI PGButtonNotify : public PGItemNotify {
public:
  INLINE PGButtonNotify();

protected:
  virtual void button_click(PGButton *button, const MouseWatcherParameter &param);

  friend class PGButton;
};

#include "pgButtonNotify.I"

#endif
