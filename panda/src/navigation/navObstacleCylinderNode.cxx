/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navObstacleCylinderNode.cxx
 * @author Maxwell175
 * @date 2022-12-15
 */

#include "navObstacleCylinderNode.h"

TypeHandle NavObstacleCylinderNode::_type_handle;

/**
 * Initializes the NavObstacleNode with the specified name. Note that this does not add it to any NavMesh.
 * That must be done from a NavMesh object.
 */
NavObstacleCylinderNode::NavObstacleCylinderNode(float radius, float height, const std::string &name) :
    NavObstacleNode(name), _radius(radius), _height(height) { }

NavMeshBuilder::ObstacleData NavObstacleCylinderNode::
get_obstacle_data(const LMatrix4 &transform) {
  LPoint3 pos(0, 0, 0);
  transform.xform_point_general_in_place(pos);
  return {DT_OBSTACLE_CYLINDER, pos, LPoint3(), _radius, _height};
}

void NavObstacleCylinderNode::
add_obstacle(std::shared_ptr<dtTileCache> tileCache, const LMatrix4 &transform) {
  LPoint3 pos(0, 0, 0);
  transform.xform_point_general_in_place(pos);
  float posArr[3] = {pos[0], pos[1], pos[2]};
  tileCache->addObstacle(reinterpret_cast<const float *>(&posArr), _radius, _height, nullptr);
}

/**
 * Generates a visualization of the cylinder representing this node.
 */
PT(GeomNode) NavObstacleCylinderNode::
get_debug_geom() {
  // TODO
  return nullptr;
}

/**
 * Tells the BamReader how to create objects of type NavObstacleCylinderNode.
 */
void NavObstacleCylinderNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void NavObstacleCylinderNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_float32(_radius);
  dg.add_float32(_height);

  PandaNode::write_datagram(manager, dg);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int NavObstacleCylinderNode::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = PandaNode::complete_pointers(p_list, manager);

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type BulletShape is encountered in the Bam file.  It should create the
 * BulletShape and extract its information from the file.
 */
TypedWritable *NavObstacleCylinderNode::
make_from_bam(const FactoryParams &params) {
  std::string name = "FromBam";
  DatagramIterator scan;
  BamReader *manager;

  float radius = scan.get_float32();
  float height = scan.get_float32();

  NavObstacleCylinderNode *param = new NavObstacleCylinderNode(radius, height, name);

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new BulletTriangleMeshShape.
 */
void NavObstacleCylinderNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  manager->read_pointer(scan);
}
