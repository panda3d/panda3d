// Filename: sceneGraphReducer.I
// Created by:  drose (14Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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
