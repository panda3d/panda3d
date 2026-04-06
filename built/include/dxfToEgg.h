/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxfToEgg.h
 * @author drose
 * @date 2004-05-04
 */

#ifndef DXFTOEGG_H
#define DXFTOEGG_H

#include "pandatoolbase.h"

#include "somethingToEgg.h"
#include "dxfToEggConverter.h"

/**
 * A program to read a DXF file and generate an egg file.
 */
class DXFToEgg : public SomethingToEgg {
public:
  DXFToEgg();

  void run();
};

#endif
