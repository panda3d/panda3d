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
SceneGraphReducer() :
  _combine_radius(0.0f)
{
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
//     Function: SceneGraphReducer::set_combine_radius
//       Access: Published
//  Description: Specifies the radius that is used in conjunction with
//               CS_within_radius to decide whether a subgraph's
//               siblings should be combined into a single node or
//               not.
//
//               If the CS_within_radius bit is included in the
//               combine_siblings_bits parameter passed to flatten,
//               than any nodes whose bounding volume is smaller than
//               the indicated radius will be combined together (as if
//               CS_other were set).
////////////////////////////////////////////////////////////////////
INLINE void SceneGraphReducer::
set_combine_radius(float combine_radius) {
  _combine_radius = combine_radius;
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::get_combine_radius
//       Access: Published
//  Description: Returns the radius that is used in conjunction with
//               CS_within_radius.  See set_combine_radius().
////////////////////////////////////////////////////////////////////
INLINE float SceneGraphReducer::
get_combine_radius() const {
  return _combine_radius;
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
  PStatTimer timer(_apply_collector);
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
  PStatTimer timer(_collect_collector);
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
  PStatTimer timer(_make_nonindexed_collector);
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
  PStatTimer timer(_unify_collector);
  r_unify(root);
}
