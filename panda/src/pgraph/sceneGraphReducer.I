/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sceneGraphReducer.I
 * @author drose
 * @date 2002-03-14
 */

/**
 *
 */
INLINE SceneGraphReducer::
SceneGraphReducer(GraphicsStateGuardianBase *gsg) :
  _combine_radius(0.0f)
{
  set_gsg(gsg);
}

/**
 *
 */
INLINE SceneGraphReducer::
~SceneGraphReducer() {
}

/**
 * Returns the particular GraphicsStateGuardian that this object will attempt
 * to optimize to.  See set_gsg().
 */
INLINE GraphicsStateGuardianBase *SceneGraphReducer::
get_gsg() const {
  return _gsg;
}


/**
 * Specifies the radius that is used in conjunction with CS_within_radius to
 * decide whether a subgraph's siblings should be combined into a single node
 * or not.
 *
 * If the CS_within_radius bit is included in the combine_siblings_bits
 * parameter passed to flatten, than any nodes whose bounding volume is
 * smaller than the indicated radius will be combined together (as if CS_other
 * were set).
 */
INLINE void SceneGraphReducer::
set_combine_radius(PN_stdfloat combine_radius) {
  _combine_radius = combine_radius;
}

/**
 * Returns the radius that is used in conjunction with CS_within_radius.  See
 * set_combine_radius().
 */
INLINE PN_stdfloat SceneGraphReducer::
get_combine_radius() const {
  return _combine_radius;
}


/**
 * Walks the scene graph, accumulating attribs of the indicated types,
 * applying them to the vertices, and removing them from the scene graph.
 * This has a performance optimization benefit in itself, but is especially
 * useful to pave the way for a call to flatten() and greatly improve the
 * effectiveness of the flattening operation.
 *
 * Multiply instanced geometry is duplicated before the attribs are applied.
 *
 * Of course, this operation does make certain dynamic operations impossible.
 */
INLINE void SceneGraphReducer::
apply_attribs(PandaNode *node, int attrib_types) {
  nassertv(check_live_flatten(node));
  nassertv(node != nullptr);
  PStatTimer timer(_apply_collector);
  AccumulatedAttribs attribs;
  r_apply_attribs(node, attribs, attrib_types, _transformer);
  _transformer.finish_apply();
}

/**
 * This flavor of apply_attribs() can be called recursively from within
 * another flatten process (e.g.  from
 * PandaNode::apply_attribs_to_vertices()). The parameters were presumably
 * received from a parent SceneGraphReducer object.
 */
INLINE void SceneGraphReducer::
apply_attribs(PandaNode *node, const AccumulatedAttribs &attribs,
              int attrib_types, GeomTransformer &transformer) {
  nassertv(node != nullptr);
  r_apply_attribs(node, attribs, attrib_types, transformer);
}

/**
 * Walks through the tree at this node and below and unifies the
 * GeomVertexFormat for any GeomVertexData objects that are found, so that all
 * eligible vdatas (according to collect_bits; see collect_vertex_data) will
 * share the same vertex format.
 *
 * This will add unused columns where necessary to match formats.  It can
 * result in suboptimal performance if used needlessly.
 *
 * There is usually no reason to call this explicitly, since
 * collect_vertex_data() will do this anyway if it has not been done already.
 * However, calling it ahead of time can make that future call to
 * collect_vertex_data() run a little bit faster.
 *
 * The return value is the number of vertex datas modified.
 */
INLINE int SceneGraphReducer::
make_compatible_format(PandaNode *root, int collect_bits) {
  nassertr(root != nullptr, 0);
  nassertr(check_live_flatten(root), 0);
  PStatTimer timer(_collect_collector);
  int count = 0;
  count += r_collect_vertex_data(root, collect_bits, _transformer, true);
  count += _transformer.finish_collect(true);
  return count;
}

/**
 * Collects all different GeomVertexData blocks that have compatible formats
 * at this node and below into a single, unified block (or at least multiple
 * larger blocks).  This is intended to reduce rendering overhead incurred by
 * switching vertex buffers.  It can also make a subsequent call to unify()
 * much more effective than it would have been otherwise.
 *
 * The set of bits passed in collect_bits indicates which properties are used
 * to differentiate GeomVertexData blocks.  If it is 0, then more blocks will
 * be combined together than if it is nonzero.
 */
INLINE int SceneGraphReducer::
collect_vertex_data(PandaNode *root, int collect_bits) {
  nassertr(root != nullptr, 0);
  nassertr(check_live_flatten(root), 0);
  PStatTimer timer(_collect_collector);
  int count = 0;
  count += r_collect_vertex_data(root, collect_bits, _transformer, false);
  count += _transformer.finish_collect(false);
  return count;
}

/**
 * Converts indexed geometry to nonindexed geometry at the indicated node and
 * below, by duplicating vertices where necessary.  The parameter
 * nonindexed_bits is a union of bits defined in
 * SceneGraphReducer::MakeNonindexed, which specifes which types of geometry
 * to avoid making nonindexed.
 */
INLINE int SceneGraphReducer::
make_nonindexed(PandaNode *root, int nonindexed_bits) {
  nassertr(root != nullptr, 0);
  nassertr(check_live_flatten(root), 0);
  PStatTimer timer(_make_nonindexed_collector);
  return r_make_nonindexed(root, nonindexed_bits);
}

/**
 * Walks the scene graph rooted at this node and below, and uses the indicated
 * GSG to premunge every Geom found to optimize it for eventual rendering on
 * the indicated GSG.  If there is no GSG indicated for the SceneGraphReducer,
 * this is a no-op.
 *
 * This operation will also apply to stashed children.
 */
INLINE void SceneGraphReducer::
premunge(PandaNode *root, const RenderState *initial_state) {
  nassertv(root != nullptr);
  nassertv(check_live_flatten(root));
  if (_gsg != nullptr) {
    PStatTimer timer(_premunge_collector);
    r_premunge(root, initial_state);
  }
}
