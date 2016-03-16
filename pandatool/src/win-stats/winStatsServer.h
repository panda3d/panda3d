/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsServer.h
 * @author drose
 * @date 2003-12-02
 */

#ifndef WINSTATSSERVER_H
#define WINSTATSSERVER_H

#include "pandatoolbase.h"
#include "pStatServer.h"

/**
 * The class that owns the main loop, waiting for client connections.
 */
class WinStatsServer : public PStatServer {
public:
  virtual PStatMonitor *make_monitor();
};

#endif
