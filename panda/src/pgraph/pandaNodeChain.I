/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandaNodeChain.I
 * @author drose
 * @date 2006-04-21
 */

/**
 *
 */
INLINE PandaNodeChain::
PandaNodeChain(const char *lock_name) :
  LinkedListNode(true),  // This object is the root of a list of PandaNodes.
 _lock(lock_name)
{
}

/**
 *
 */
INLINE PandaNodeChain::
~PandaNodeChain() {
  _next = nullptr;
  _prev = nullptr;
}
