/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxClothNode.I
 * @author enn0x
 * @date 2010-04-05
 */

/**
 *
 */
INLINE PhysxClothNode::
PhysxClothNode(const char *name) : GeomNode(name) {

  _numVertices = 0;

  _vdata = new GeomVertexData("", GeomVertexFormat::get_v3n3t2(), Geom::UH_stream);

  _prim = new GeomTriangles(Geom::UH_stream);
  _prim->set_shade_model(Geom::SM_uniform);

  _geom = new Geom(_vdata);
  _geom->add_primitive(_prim);

  this->add_geom(_geom);

  _numTexcoords = 0;
  _texcoords = nullptr;
}

/**
 *
 */
INLINE PhysxClothNode::
~PhysxClothNode() {

  if (_texcoords) {
    delete [] _texcoords;
  }
}
