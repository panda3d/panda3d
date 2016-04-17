/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file panda3dMac.h
 * @author drose
 * @date 2009-10-23
 */

#ifndef PANDA3DMAC_H
#define PANDA3DMAC_H

#include "panda3d.h"

#include <Carbon/Carbon.h>

/**
 * A specialization of Panda3D for running as a Carbon application on OS X.
 * Instead of taking input from the command line, this program waits quietly
 * for an "open documents" Apple event.
 */
class Panda3DMac : public Panda3D {
public:
  Panda3DMac();

  void open_p3d_file(FSRef *ref);
};

#endif
