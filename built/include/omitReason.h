/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file omitReason.h
 * @author drose
 * @date 2000-11-30
 */

#ifndef OMITREASON_H
#define OMITREASON_H

#include "pandatoolbase.h"

/**
 * This enumerates the reasons why a texture may not have been placed in a
 * palette image.
 */
enum OmitReason {
  OR_none,
  // Not omitted: the texture appears on a palette image.

  OR_working,
  // Still working on placing it.

  OR_omitted,
  // Explicitly omitted by the user via "omit" in .txa file.

  OR_size,
  // Too big to fit on a single palette image.

  OR_solitary,
  // It should be placed, but it's the only one on the palette image so far,
  // so there's no point.

  OR_coverage,
  // The texture repeats.  Specifically, the UV's for the texture exceed the
  // maximum rectangle allowed by coverage_threshold.

  OR_unknown,
  // The texture file cannot be read, so its size can't be determined.

  OR_unused,
  // The texture is no longer used by any of the egg files that formerly
  // referenced it.

  OR_default_omit,
  // The texture is omitted because _omit_everything is set true.
};

std::ostream &operator << (std::ostream &out, OmitReason omit);

#endif
