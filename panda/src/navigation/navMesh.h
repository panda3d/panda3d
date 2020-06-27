/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMesh.h
 * @author ashwini
 * @date 2020-060-21
 */


#ifndef NAVMESH_H
#define NAVMESH_H

#include "Recast.h"
#include "DetourNavMesh.h"
#include "typedWritableReferenceCount.h"

class EXPCL_NAVIGATION NavMesh: public TypedWritableReferenceCount
{
PUBLISHED:
  NavMesh(dtNavMesh *nav_mesh);
  void set_nav_mesh(dtNavMesh *m) { _nav_mesh = m; }
  NavMesh();

private:
  dtNavMesh *_nav_mesh;
  
public:
  
  
  ~NavMesh();
  

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "NavMesh",
      TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() { init_type(); return get_class_type(); }

private:
  static TypeHandle _type_handle;
};

#endif // NAVMESH_H
