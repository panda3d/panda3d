/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsServer.cxx
 * @author drose
 * @date 2006-01-16
 */

#include "gtkStatsServer.h"
#include "gtkStatsMonitor.h"

/**

 */
PStatMonitor *GtkStatsServer::
make_monitor() {
  return new GtkStatsMonitor(this);
}
