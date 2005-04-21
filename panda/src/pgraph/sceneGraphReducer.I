// Filename: sceneGraphReducer.I
// Created by:  drose (14Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SceneGraphReducer::
SceneGraphReducer() {
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SceneGraphReducer::
~SceneGraphReducer() {
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::set_usage_hint
//       Access: Published
//  Description: Specifies the ceiling UsageHint that will be applied
//               to transformed geometry.  If the source geometry's
//               usage hint is greater (more static) than this, it
//               will be reduced to this level.  If the source
//               geometry's usage hit is less (more dynamic) than
//               this, it will be preserved at its current level.
////////////////////////////////////////////////////////////////////
INLINE void SceneGraphReducer::
set_usage_hint(qpGeom::UsageHint usage_hint) {
  _transformer.set_usage_hint(usage_hint);
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::get_usage_hint
//       Access: Published
//  Description: Returns the UsageHint that will be applied to
//               generated geometry.  See set_usage_hint().
////////////////////////////////////////////////////////////////////
INLINE qpGeom::UsageHint SceneGraphReducer::
get_usage_hint() const {
  return _transformer.get_usage_hint();
}


////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::apply_attribs
//       Access: Published
//  Description: Walks the scene graph, accumulating attribs of
//               the indicated types, applying them to the vertices,
//               and removing them from the scene graph.  This has a
//               performance optimization benefit in itself, but is
//               especially useful to pave the way for a call to
//               flatten() and greatly improve the effectiveness of
//               the flattening operation.
//
//               Multiply instanced geometry is duplicated before the
//               attribs are applied.
//
//               Of course, this operation does make certain dynamic
//               operations impossible.
////////////////////////////////////////////////////////////////////
INLINE void SceneGraphReducer::
apply_attribs(PandaNode *node, int attrib_types) {
  AccumulatedAttribs attribs;
  r_apply_attribs(node, attribs, attrib_types, _transformer);
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::apply_attribs
//       Access: Published
//  Description: This flavor of apply_attribs() can be called
//               recursively from within another flatten process
//               (e.g. from PandaNode::apply_attribs_to_vertices()).
//               The parameters were presumably received from a parent
//               SceneGraphReducer object.
////////////////////////////////////////////////////////////////////
INLINE void SceneGraphReducer::
apply_attribs(PandaNode *node, const AccumulatedAttribs &attribs,
              int attrib_types, GeomTransformer &transformer) {
  r_apply_attribs(node, attribs, attrib_types, transformer);
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::collect_vertex_data
//       Access: Published
//  Description: Collects all different GeomVertexData blocks that
//               have compatible formats at this node and below into a
//               single, unified block (or at least multiple larger
//               blocks).  This is intended to reduce rendering
//               overhead incurred by switching vertex buffers.
//
//               The set of bits passed in collect_bits indicates
//               which properties are used to differentiate
//               GeomVertexData blocks.  If it is 0, then more blocks
//               will be combined together than if it is nonzero.
////////////////////////////////////////////////////////////////////
INLINE int SceneGraphReducer::
collect_vertex_data(PandaNode *root, int collect_bits) {
  return r_collect_vertex_data(root, collect_bits, _transformer);
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::make_nonindexed
//       Access: Published
//  Description: Converts indexed geometry to nonindexed geometry at
//               the indicated node and below, by duplicating vertices
//               where necessary.  The parameter nonindexed_bits is a
//               union of bits defined in
//               SceneGraphReducer::MakeNonindexed, which specifes
//               which types of geometry to avoid making nonindexed.
////////////////////////////////////////////////////////////////////
INLINE int SceneGraphReducer::
make_nonindexed(PandaNode *root, int nonindexed_bits) {
  return r_make_nonindexed(root, nonindexed_bits);
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::unify
//       Access: Published
//  Description: Calls unify() on every GeomNode at this level and
//               below.
////////////////////////////////////////////////////////////////////
INLINE void SceneGraphReducer::
unify(PandaNode *root) {
  r_unify(root);
}
