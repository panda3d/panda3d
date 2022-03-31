/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file callbackNode.I
 * @author drose
 * @date 2009-03-13
 */

/**
 * Sets the CallbackObject that will be notified when this node is visited
 * during the cull traversal.  This callback will be made during the cull
 * thread.
 *
 * The cull traversal is responsible for determining which nodes are visible
 * and within the view frustum, and for accumulating state and transform, and
 * generally building up the list of CullableObjects that are to be eventually
 * passed to the draw traversal for rendering.
 *
 * At the time the cull traversal callback is made, the node has been
 * determined to be visible and it has passed the bounding-volume test, so it
 * lies within the view frustum.
 *
 * The callback is passed an instance of a NodeCullCallbackData, which
 * contains pointers to the CullTraverser and CullTraverserData--enough data
 * to examine the current node and its place within the scene graph.  The
 * callback *replaces* the normal cull behavior, so if your callback does
 * nothing, the cull traversal will not continue below this node.  If you wish
 * the cull traversal to continue to visit this node and below, you must call
 * cbdata->upcall() from your callback.
 */
INLINE void CallbackNode::
set_cull_callback(CallbackObject *object) {
  CDWriter cdata(_cycler);
  cdata->_cull_callback = object;
}

/**
 * Removes the callback set by an earlier call to set_cull_callback().
 */
INLINE void CallbackNode::
clear_cull_callback() {
  set_cull_callback(nullptr);
}

/**
 * Returns the CallbackObject set by set_cull_callback().
 */
INLINE CallbackObject *CallbackNode::
get_cull_callback() const {
  CDReader cdata(_cycler);
  return cdata->_cull_callback;
}

/**
 * Sets the CallbackObject that will be notified when this node is visited
 * during the draw traversal.  This callback will be made during the draw
 * thread.
 *
 * The draw traversal is responsible for actually issuing the commands to the
 * graphics engine to draw primitives.  Its job is to walk through the list of
 * CullableObjects build up by the cull traversal, as quickly as possible,
 * issuing the appropriate commands to draw each one.
 *
 * At the time the draw traversal callback is made, the graphics state has
 * been loaded with the correct modelview transform and render state, and the
 * primitives (if any) in this node are ready to be drawn.
 *
 * The callback is passed an instance of a GeomDrawCallbackData, which
 * contains pointers to the current state and transform, as well as the
 * current GSG.  There is a Geom pointer as well, but it will always be NULL
 * to this callback, since the CallbackNode does not itself contain any Geoms.
 */
INLINE void CallbackNode::
set_draw_callback(CallbackObject *object) {
  CDWriter cdata(_cycler);
  cdata->_draw_callback = object;
}

/**
 * Removes the callback set by an earlier call to set_draw_callback().
 */
INLINE void CallbackNode::
clear_draw_callback() {
  set_draw_callback(nullptr);
}

/**
 * Returns the CallbackObject set by set_draw_callback().
 */
INLINE CallbackObject *CallbackNode::
get_draw_callback() const {
  CDReader cdata(_cycler);
  return cdata->_draw_callback;
}

/**
 *
 */
INLINE CallbackNode::CData::
CData() {
}

/**
 *
 */
INLINE CallbackNode::CData::
CData(const CallbackNode::CData &copy) :
  _cull_callback(copy._cull_callback),
  _draw_callback(copy._draw_callback)
{
}
