/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMakeSomething.h
 * @author drose
 * @date 2003-10-01
 */

#ifndef EGGMAKESOMETHING_H
#define EGGMAKESOMETHING_H

#include "pandatoolbase.h"

#include "eggWriter.h"

/**
 * A base class for a family of programs that generate egg models of various
 * fundamental shapes.
 */
class EggMakeSomething : public EggWriter {
public:
  EggMakeSomething();
};

#endif
