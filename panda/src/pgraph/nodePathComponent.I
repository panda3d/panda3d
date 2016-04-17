/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nodePathComponent.I
 * @author drose
 * @date 2002-02-25
 */

/**
 *
 */
INLINE NodePathComponent::CData::
CData() {
  _length = 1;
}

/**
 *
 */
INLINE NodePathComponent::CData::
CData(const NodePathComponent::CData &copy) :
  _next(copy._next),
  _length(copy._length)
{
}

/**
 * NodePathComponents should not be copied.
 */
INLINE NodePathComponent::
NodePathComponent(const NodePathComponent &copy) {
  nassertv(false);
}

/**
 * NodePathComponents should not be copied.
 */
INLINE void NodePathComponent::
operator = (const NodePathComponent &copy) {
  nassertv(false);
}


/**
 *
 */
INLINE NodePathComponent::
~NodePathComponent() {
  nassertv(_node != (PandaNode *)NULL);
  _node->delete_component(this);
}

/**
 * Returns the node referenced by this component.
 */
INLINE PandaNode *NodePathComponent::
get_node() const {
  nassertr(_node != (PandaNode *)NULL, _node);
  return _node;
}

/**
 * Returns true if the key for this component has already been generated,
 * false otherwise.  Even if this returns false, calling get_key() will still
 * return a valid key; that will simply cause the key to be generated on-the-
 * fly.
 */
INLINE bool NodePathComponent::
has_key() const {
  return (_key != 0);
}

INLINE ostream &operator << (ostream &out, const NodePathComponent &comp) {
  comp.output(out);
  return out;
}
