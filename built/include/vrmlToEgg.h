/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrmlToEgg.h
 * @author drose
 * @date 2004-10-01
 */

#ifndef VRMLTOEGG_H
#define VRMLTOEGG_H

#include "pandatoolbase.h"

#include "somethingToEgg.h"
#include "vrmlToEggConverter.h"

/**
 * A program to read a VRML file and generate an egg file.
 */
class VRMLToEgg : public SomethingToEgg {
public:
  VRMLToEgg();

  void run();
};

#endif
