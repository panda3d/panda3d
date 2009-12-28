// Filename: physxDebugGeomNode.cxx
// Created by:  enn0x (15Sep09)
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

#include "physxDebugGeomNode.h"

TypeHandle PhysxDebugGeomNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::update
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxDebugGeomNode::
update(NxScene *scenePtr) {

  if (get_num_parents() == 0) {
    return;
  }

  const NxDebugRenderable *renderable = scenePtr->getDebugRenderable();
  if (!renderable) {
    remove_all_geoms();
    physx_cat.warning() << "Could no get debug renderable." << endl;
    return;
  }

  const NxDebugLine *lines = renderable->getLines();
  NxU32 n = renderable->getNbLines();

  _segs.reset();

  for (NxU32 i=0; i<n; i++)
  {
    NxF32 b = NxF32((lines[i].color)&0xff) / 255.0f;
    NxF32 g = NxF32((lines[i].color>>8)&0xff) / 255.0f;
    NxF32 r = NxF32((lines[i].color>>16)&0xff) / 255.0f;

    NxVec3 p0 = lines[i].p0;
    NxVec3 p1 = lines[i].p1;

    _segs.set_color(r, g, b);
    _segs.move_to(p0.x, p0.y, p0.z);
    _segs.draw_to(p1.x, p1.y, p1.z);
  }

  GeomNode *node = _segs.create();
  remove_all_geoms();
  add_geoms_from(node);
  delete node;

  physx_cat.spam() << "Updated PhysxDebugGeomNode geometry (" << n << " lines)\n";
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::on
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxDebugGeomNode::
on() {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZATION_SCALE, _scale);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::off
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxDebugGeomNode::
off() {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZATION_SCALE, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::toggle
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxDebugGeomNode::
toggle() {

  if (NxGetPhysicsSDK()->getParameter(NX_VISUALIZATION_SCALE) == 0.0f) {
    on();
  }
  else {
    off();
  }
}

