/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file partBundleNode.I
 * @author drose
 * @date 2002-03-06
 */

/**
 * The PartBundle and its node should be constructed together.  Generally, the
 * derived classes of PartBundleNode will automatically create a PartBundle of
 * the appropriate type, and pass it up to this constructor.
 */
INLINE PartBundleNode::
PartBundleNode(const std::string &name, PartBundle *bundle) :
  PandaNode(name)
{
  add_bundle(bundle);
}

/**
 * For internal use only.
 */
INLINE PartBundleNode::
PartBundleNode() : PandaNode("") {
}

/**
 * Use make_copy() or copy_subgraph() to copy one of these.
 *
 * This constructor does not copy the bundle pointers.
 */
INLINE PartBundleNode::
PartBundleNode(const PartBundleNode &copy) :
  PandaNode(copy)
{
}

/**
 *
 */
INLINE int PartBundleNode::
get_num_bundles() const {
  return _bundles.size();
}

/**
 *
 */
INLINE PartBundle *PartBundleNode::
get_bundle(int n) const {
  nassertr(n >= 0 && n < (int)_bundles.size(), nullptr);
  return _bundles[n]->get_bundle();
}

/**
 * Returns the PartBundleHandle that wraps around the actual nth PartBundle.
 * While the PartBundle pointer might later change due to a future flatten
 * operation, the PartBundleHandle will not.
 */
INLINE PartBundleHandle *PartBundleNode::
get_bundle_handle(int n) const {
  nassertr(n >= 0 && n < (int)_bundles.size(), nullptr);
  return _bundles[n];
}
