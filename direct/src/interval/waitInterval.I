/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file waitInterval.I
 * @author drose
 * @date 2002-09-12
 */

/**
 * All Wait intervals have the same name.  No one really cares if their names
 * are unique, after all.
 */
INLINE WaitInterval::
WaitInterval(double duration) :
  CInterval("Wait", duration, true)
{
}
