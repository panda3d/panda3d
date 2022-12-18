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
#include "geomTriangles.h"

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
  transform.xform_point_in_place(pos);
  // Apply the transforms to the radius and height.
  LPoint3 radius_height_transformed(_radius, _radius, _height);
  transform.xform_point_in_place(radius_height_transformed);
  // Counteract the translation
  LVector3 radius_height = radius_height_transformed - transform.get_row3(3);
  // Undo the yup conversion that is baked into the incoming mat.
  LMatrix4::convert_mat(CS_yup_right, CS_zup_right).xform_point_in_place(radius_height);
  return {DT_OBSTACLE_CYLINDER, pos, LPoint3(), std::min(radius_height[0], radius_height[1]), radius_height[2]};
}

void NavObstacleCylinderNode::
add_obstacle(dtTileCache *tileCache, const LMatrix4 &transform) {
  LPoint3 pos(0, 0, 0);
  transform.xform_point_in_place(pos);
  float posArr[3] = {pos[0], pos[1], pos[2]};
  // Apply the transforms to the radius and height.
  LPoint3 radius_height_transformed(_radius, _radius, _height);
  transform.xform_point_in_place(radius_height_transformed);
  // Counteract the translation
  LVector3 radius_height = radius_height_transformed - transform.get_row3(3);
  // Undo the yup conversion that is baked into the incoming mat.
  LMatrix4::convert_mat(CS_yup_right, CS_zup_right).xform_point_in_place(radius_height);
  tileCache->addObstacle(reinterpret_cast<const float *>(&posArr), std::min(radius_height[0], radius_height[1]), radius_height[2], nullptr);
}

/**
 * Generates a visualization of the cylinder representing this node.
 */
PT(GeomNode) NavObstacleCylinderNode::
get_debug_geom() {
  PT(GeomVertexData) vdata = new GeomVertexData
      ("obstacle", GeomVertexFormat::get_v3c4(),
       Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter color(vdata, InternalName::get_color());

  PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_static);

  static const int num_slices = 8;
  vertex.reserve_num_rows(num_slices * 2 + 2);
  for (int si = 0; si <= num_slices; si++) {
    vertex.add_data3(LPoint3::rfu(_radius * ccos(((PN_stdfloat)si / num_slices) * 2.0 * MathNumbers::pi),
                             _radius * csin(((PN_stdfloat)si / num_slices) * 2.0 * MathNumbers::pi),
                             0));
    color.add_data4(1, 0, 0, 1);
    vertex.add_data3(LPoint3::rfu(_radius * ccos(((PN_stdfloat)si / num_slices) * 2.0 * MathNumbers::pi),
                             _radius * csin(((PN_stdfloat)si / num_slices) * 2.0 * MathNumbers::pi),
                             _height));
    color.add_data4(1, 0, 0, 1);
  }
  vertex.add_data3(LPoint3::rfu(0, 0, 0));
  color.add_data4(1, 0, 0, 1);
  vertex.add_data3(LPoint3::rfu(0, 0, _height));
  color.add_data4(1, 0, 0, 1);

  for (int si = 0; si <= num_slices; si++) {
    int top_left = si * 2 + 1;
    int bottom_left = si * 2;
    int bottom_right;
    int top_right;
    if (si == num_slices - 1) {
      bottom_right = 0;
      top_right = 1;
    } else {
      bottom_right = si * 2 + 2;
      top_right = si * 2 + 3;
    }
    // Top left tri
    tris->add_vertices(top_left, bottom_left, bottom_right);
    // Bottom right tri
    tris->add_vertices(bottom_right, top_right, top_left);

    // Slice of bottom cap
    tris->add_vertices(num_slices * 2, bottom_right, bottom_left);
    // Slice of top cap
    tris->add_vertices(top_left, top_right, num_slices * 2 + 1);
  }

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(tris);

  PT(GeomNode) geomNode = new GeomNode(get_name() + "-debug");
  geomNode->add_geom(geom);

  return geomNode;
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
