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

#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "typedWritableReferenceCount.h"
#include "pandaFramework.h"
#include "pandaSystem.h"

class EXPCL_NAVIGATION NavMesh: public TypedWritableReferenceCount
{
PUBLISHED:
  NavMesh(dtNavMesh *nav_mesh);
  void set_nav_mesh(dtNavMesh *m) { _nav_mesh = m; }
  NavMesh();

private:
  dtNavMesh *_nav_mesh;
  
public:
  bool init_nav_mesh();
  dtNavMesh *get_nav_mesh() { return _nav_mesh; }
  dtNavMeshCreateParams _params;
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

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);
};



#endif // NAVMESH_H
