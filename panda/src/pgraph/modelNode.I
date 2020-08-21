/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modelNode.I
 * @author drose
 * @date 2002-03-16
 */

/**
 *
 */
INLINE ModelNode::
ModelNode(const std::string &name) :
  PandaNode(name)
{
  _preserve_transform = PT_none;
  _preserve_attributes = 0;
  _transform_limit = 0;
}

/**
 * Sets the preserve_transform flag.  This restricts the ability of a flatten
 * operation to affect the transform stored on this node, and/or the node
 * itself.  In the order from weakest to strongest restrictions, the possible
 * flags are:
 *
 * PT_drop_node - This node should be removed at the next flatten call.
 *
 * PT_none - The transform may be adjusted at will.  The node itself will not
 * be removed.  This is the default.
 *
 * PT_net - Preserve the net transform from the root, but it's acceptable to
 * modify the local transform stored on this particular node if necessary, so
 * long as the net transform is not changed.  This eliminates the need to drop
 * an extra transform on the node above.
 *
 * PT_local - The local (and net) transform should not be changed in any way.
 * If necessary, an extra transform will be left on the node above to
 * guarantee this.  This is a stronger restriction than PT_net.
 *
 * PT_no_touch - The local transform will not be changed, the node will not be
 * removed, and furthermore any flatten operation will not continue below this
 * node--this node and all descendents are protected from the effects of
 * flatten.
 */
INLINE void ModelNode::
set_preserve_transform(ModelNode::PreserveTransform preserve_transform) {
  _preserve_transform = preserve_transform;
}

/**
 * Returns the current setting of the preserve_transform flag.  See
 * set_preserve_transform().
 */
INLINE ModelNode::PreserveTransform ModelNode::
get_preserve_transform() const {
  return _preserve_transform;
}

/**
 * Sets the preserve_attributes flag.  This restricts the ability of a flatten
 * operation to affect the render attributes stored on this node.
 *
 * The value should be the union of bits from SceneGraphReducer::AttribTypes
 * that represent the attributes that should *not* be changed.
 */
INLINE void ModelNode::
set_preserve_attributes(int preserve_attributes) {
  _preserve_attributes = preserve_attributes;
}

/**
 * Returns the current setting of the preserve_attributes flag.  See
 * set_preserve_attributes().
 */
INLINE int ModelNode::
get_preserve_attributes() const {
  return _preserve_attributes;
}

/**
 *
 */
INLINE ModelNode::
ModelNode(const ModelNode &copy) :
  PandaNode(copy),
  _preserve_transform(copy._preserve_transform),
  _preserve_attributes(copy._preserve_attributes),
  _transform_limit(copy._transform_limit)
{
}
