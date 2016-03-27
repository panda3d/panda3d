/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoToEgg.h
 * @author drose
 * @date 2001-04-17
 */

#ifndef LWOTOEGG_H
#define LWOTOEGG_H

#include "pandatoolbase.h"

#include "somethingToEgg.h"
#include "lwoToEggConverter.h"

#include "dSearchPath.h"

/**
 * A program to read a Lightwave file and generate an egg file.
 */
class LwoToEgg : public SomethingToEgg {
public:
  LwoToEgg();

  void run();
};

#endif
