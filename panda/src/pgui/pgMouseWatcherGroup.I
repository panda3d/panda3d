/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgMouseWatcherGroup.I
 * @author drose
 * @date 2001-07-09
 */

/**
 *
 */
INLINE PGMouseWatcherGroup::
PGMouseWatcherGroup(PGTop *top) : _top(top) {
}

/**
 * Called by the PGTop object to indicate that it is no longer keeping the
 * pointer to the PGMouseWatcherGroup object.
 */
INLINE void PGMouseWatcherGroup::
clear_top(PGTop *top) {
  nassertv(_top == top);
  _top = nullptr;
}
