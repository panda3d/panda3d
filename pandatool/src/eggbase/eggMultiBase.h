/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMultiBase.h
 * @author drose
 * @date 2000-11-02
 */

#ifndef EGGMULTIBASE_H
#define EGGMULTIBASE_H

#include "pandatoolbase.h"

#include "eggBase.h"
#include "coordinateSystem.h"
#include "eggData.h"
#include "pointerTo.h"

class Filename;

/**
 * This specialization of ProgramBase is intended for programs that read
 * and/or write multiple egg files.
 *
 * See also EggMultiFilter, for a class that also knows how to read a bunch of
 * egg files in and write them out again.
 */
class EggMultiBase : public EggBase {
public:
  EggMultiBase();

  void post_process_egg_files();

protected:
  virtual PT(EggData) read_egg(const Filename &filename);

protected:
  typedef pvector< PT(EggData) > Eggs;
  Eggs _eggs;

  bool _force_complete;
};

#endif
