/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file daeToEgg.h
 * @author rdb
 * @date 2008-05-08
 */

#ifndef DAETOEGG_H
#define DAETOEGG_H

#include "pandatoolbase.h"

#include "somethingToEgg.h"
#include "daeToEggConverter.h"

/**
 * A program to read a DAE file and generate an egg file.
 */
class DAEToEgg : public SomethingToEgg {
public:
  DAEToEgg();

  void run();

private:
  bool _invert_transparency;
};

#endif
