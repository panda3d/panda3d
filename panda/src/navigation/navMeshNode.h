/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshNode.h
 * @author ashwini
 * @date 2020-060-21
 */

#ifndef NAVMESHNODE_H
#define NAVMESHNODE_H

#include <string>
#include "geom.h"
#include "pandaFramework.h"
#include "pandaSystem.h"
#include "navMesh.h"
#include <string>

class EXPCL_NAVIGATION NavMeshNode: public PandaNode
{
PUBLISHED:
  NavMeshNode(const std::string &name, PT(NavMesh) nav_mesh);
private:
  PT(NavMesh) _nav_mesh;

public:
  NavMeshNode(const std::string &name);
  ~NavMeshNode();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "NavMeshNode",
      PandaNode::get_class_type());
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
  virtual int complete_pointers(TypedWritable **plist,
                                BamReader *manager);

protected:
  //static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

};

#endif // NAVMESHNODE_H
