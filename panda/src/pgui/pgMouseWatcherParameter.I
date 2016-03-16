/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgMouseWatcherParameter.I
 * @author drose
 * @date 2001-07-05
 */

/**
 *
 */
INLINE PGMouseWatcherParameter::
PGMouseWatcherParameter() {
}

/**
 *
 */
INLINE PGMouseWatcherParameter::
PGMouseWatcherParameter(const MouseWatcherParameter &copy) :
  MouseWatcherParameter(copy)
{
}

/**
 *
 */
INLINE void PGMouseWatcherParameter::
operator = (const MouseWatcherParameter &copy) {
  MouseWatcherParameter::operator = (copy);
}
