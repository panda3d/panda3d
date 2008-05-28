// Filename: odeTriMeshGeom.cxx
// Created by:  joswilso (27Dec06)
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

#include "config_ode.h"
#include "odeTriMeshGeom.h"

TypeHandle OdeTriMeshGeom::_type_handle;

OdeTriMeshGeom::
OdeTriMeshGeom(dGeomID id) :
  OdeGeom(id) {
}

OdeTriMeshGeom::
OdeTriMeshGeom(OdeTriMeshData &data) :
  OdeGeom(dCreateTriMesh(0, data.get_id(), 0, 0, 0)) {
  OdeTriMeshData::link_data(_id, &data);
}

OdeTriMeshGeom::
OdeTriMeshGeom(OdeSpace &space, OdeTriMeshData &data) :
  OdeGeom(dCreateTriMesh(space.get_id(), data.get_id(), 0, 0, 0)) {
  OdeTriMeshData::link_data(_id, &data);
}

OdeTriMeshGeom::
OdeTriMeshGeom(const OdeTriMeshGeom &copy) :
  OdeGeom(dCreateTriMesh(0, copy.get_data_id(), 0, 0, 0)) {
  OdeTriMeshData::link_data(_id, copy.get_data());
}

OdeTriMeshGeom::
~OdeTriMeshGeom() {
}

void OdeTriMeshGeom::
destroy() {
  OdeTriMeshData::unlink_data(_id);
}
