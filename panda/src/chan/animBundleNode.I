/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animBundleNode.I
 * @author drose
 * @date 2002-03-06
 */

/**
 * The AnimBundle and its node should be constructed together.  Generally, the
 * derived classes of AnimBundleNode will automatically create a AnimBundle of
 * the appropriate type, and pass it up to this constructor.
 */
INLINE AnimBundleNode::
AnimBundleNode(const std::string &name, AnimBundle *bundle) :
  PandaNode(name),
  _bundle(bundle)
{
}

/**
 * For internal use only.
 */
INLINE AnimBundleNode::
AnimBundleNode() : PandaNode("") {
}

/**
 * Use make_copy() or copy_subgraph() to copy one of these.
 */
INLINE AnimBundleNode::
AnimBundleNode(const AnimBundleNode &copy) :
  PandaNode(copy),
  _bundle(copy._bundle)
{
}

/**
 *
 */
INLINE AnimBundle *AnimBundleNode::
get_bundle() const {
  return _bundle;
}
