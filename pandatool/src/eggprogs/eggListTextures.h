/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggListTextures.h
 * @author drose
 * @date 2005-05-23
 */

#ifndef EGGLISTTEXTURES_H
#define EGGLISTTEXTURES_H

#include "pandatoolbase.h"

#include "eggReader.h"

/**
 * Reads an egg file and outputs the list of textures it uses.
 */
class EggListTextures : public EggReader {
public:
  EggListTextures();

  void run();
};

#endif
