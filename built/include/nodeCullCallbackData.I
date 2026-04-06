/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nodeCullCallbackData.I
 * @author drose
 * @date 2009-03-13
 */

/**
 *
 */
INLINE NodeCullCallbackData::
NodeCullCallbackData(CullTraverser *trav, CullTraverserData &data) :
  _trav(trav),
  _data(data)
{
}

/**
 * Returns the CullTraverser in use at the time of the callback.  This object
 * contains data that does not change during the traversal, such as the
 * DisplayRegion and Camera in use.
 */
INLINE CullTraverser *NodeCullCallbackData::
get_trav() const {
  return _trav;
}

/**
 * Returns the CullTraverserData in use at the time of the callback.  This
 * object contains data that changes at each node of the traversal, such as
 * the current node and the current net transform to that node.
 */
INLINE CullTraverserData &NodeCullCallbackData::
get_data() const {
  return _data;
}
