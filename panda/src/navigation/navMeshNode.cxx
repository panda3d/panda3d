/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshNode.cxx
 * @author ashwini
 * @date 2020-060-21
 */


#include "navMeshNode.h"

TypeHandle NavMeshNode::_type_handle;

NavMeshNode::NavMeshNode(const std::string &name, PT(NavMesh) nav_mesh):
  PandaNode(name)
{
  _nav_mesh = nav_mesh;
}

NavMeshNode::NavMeshNode(const std::string &name):
 PandaNode(name) {}

NavMeshNode::~NavMeshNode() {
  
}


/**
 * Tells the BamReader how to create objects of type BulletTriangleMeshShape.
 */
void NavMeshNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void NavMeshNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);

  manager->write_pointer(dg, _nav_mesh);
  // Write NULL pointer to indicate the end of the list.
  manager->write_pointer(dg, nullptr);

}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int NavMeshNode::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = PandaNode::complete_pointers(p_list, manager);

  _nav_mesh = DCAST(NavMesh, p_list[pi++]);

  return pi;
}

// /**
//  * This function is called by the BamReader's factory when a new object of
//  * type BulletShape is encountered in the Bam file.  It should create the
//  * BulletShape and extract its information from the file.
//  */
// TypedWritable *NavMeshNode::
// make_from_bam(const FactoryParams &params) {
//   string s = "FromBam"
//   NavMeshNode *param = new NavMeshNode(s);
//   DatagramIterator scan;
//   BamReader *manager;

//   parse_params(params, scan, manager);
//   param->fillin(scan, manager);

//   return param;
// }

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new BulletTriangleMeshShape.
 */
void NavMeshNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);

 

  manager->read_pointer(scan);

  
}


