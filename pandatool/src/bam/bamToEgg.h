/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamToEgg.h
 * @author drose
 * @date 2001-06-25
 */

#ifndef BAMTOEGG_H
#define BAMTOEGG_H

#include "pandatoolbase.h"

#include "somethingToEgg.h"

/**
 * This program reads a bam file, for instance as written out from a real-time
 * interaction session, and generates a corresponding egg file.
 */
class BamToEgg : public SomethingToEgg {
public:
  BamToEgg();

  void run();

private:
};

#endif
