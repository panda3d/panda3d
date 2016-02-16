/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltToEgg.h
 * @author drose
 * @date 2001-04-17
 */

#ifndef FLTTOEGG_H
#define FLTTOEGG_H

#include "pandatoolbase.h"

#include "somethingToEgg.h"
#include "fltToEggConverter.h"

#include "dSearchPath.h"

/**
 * A program to read a flt file and generate an egg file.
 */
class FltToEgg : public SomethingToEgg {
public:
  FltToEgg();

  void run();

  bool _compose_transforms;
};

#endif
