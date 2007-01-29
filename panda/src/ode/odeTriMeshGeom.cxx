// Filename: odeTriMeshGeom.cxx
// Created by:  joswilso (27Dec06)
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

#include "config_ode.h"
#include "odeTriMeshGeom.h"

TypeHandle OdeTriMeshGeom::_type_handle;
TriMeshDataMap OdeTriMeshGeom::MeshData;

OdeTriMeshGeom::
OdeTriMeshGeom(OdeTriMeshData &data) :
  OdeGeom(dCreateTriMesh(0, data.get_id(), 0, 0, 0)) {
  link_data(_id, data);
}

OdeTriMeshGeom::
OdeTriMeshGeom(OdeSpace &space, OdeTriMeshData &data) :
  OdeGeom(dCreateTriMesh(space.get_id(), data.get_id(), 0, 0, 0)) {
  link_data(_id, data);
}

OdeTriMeshGeom::
OdeTriMeshGeom(OdeTriMeshGeom &copy) :
  OdeGeom(dCreateTriMesh(0, copy.get_data_id(), 0, 0, 0)) {
}

OdeTriMeshGeom::
~OdeTriMeshGeom() {
}

void OdeTriMeshGeom::
link_data(dGeomID id, OdeTriMeshData &data) {
  odegeom_cat.debug() << get_class_type() << "(" << id << ") linking to data: " << data.get_id() << "\n";
  MeshData[id] = &data;
}

void OdeTriMeshGeom::
unlink_data(dGeomID id) {
  TriMeshDataMap::iterator iter = MeshData.find(id);
  
  if (iter != MeshData.end()) {
    odegeom_cat.debug() << get_class_type() << "(" << id << ") unlinking from data: " << iter->second->get_id() << "\n";
  } else {
    odegeom_cat.warning() << get_class_type() << "(" << id << ") Unlinking from absent data!\n";
  }

  MeshData.erase(id);
}
