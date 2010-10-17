// Filename: physxSoftBodyNode.I
// Created by:  enn0x (13Sep10)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxSoftBodyNode::
PhysxSoftBodyNode(const char *name) : GeomNode(name) {

  _vdata = new GeomVertexData("", GeomVertexFormat::get_v3n3t2(), Geom::UH_stream);

  _prim = new GeomTriangles(Geom::UH_stream);
  _prim->set_shade_model(Geom::SM_uniform);

  _geom = new Geom(_vdata);
  _geom->add_primitive(_prim);

  this->add_geom(_geom);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyNode::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxSoftBodyNode::
~PhysxSoftBodyNode() {

}

