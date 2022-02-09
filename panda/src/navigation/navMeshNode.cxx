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
#include "omniBoundingVolume.h"
#include "cullTraverserData.h"
#include "cullTraverser.h"
#include <iostream>

TypeHandle NavMeshNode::_type_handle;

/**
 * NavMeshNode contructor which stores the NavMesh object as _nav_mesh
 */
NavMeshNode::NavMeshNode(const std::string &name, PT(NavMesh) nav_mesh):
  PandaNode(name)
{
  _nav_mesh = nav_mesh;
  set_cull_callback();
  // CollisionNodes are hidden by default.
  set_overall_hidden(true);
  set_renderable();
}

NavMeshNode::NavMeshNode(const std::string &name):
 PandaNode(name) {}

NavMeshNode::~NavMeshNode() {}

/**
 * This function will be called during the cull traversal to perform any
 * additional operations that should be performed at cull time.  This may
 * include additional manipulation of render state or additional
 * visible/invisible decisions, or any other arbitrary operation.
 *
 * Note that this function will *not* be called unless set_cull_callback() is
 * called in the constructor of the derived class.  It is necessary to call
 * set_cull_callback() to indicated that we require cull_callback() to be
 * called.
 *
 * By the time this function is called, the node has already passed the
 * bounding-volume test for the viewing frustum, and the node's transform and
 * state have already been applied to the indicated CullTraverserData object.
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool NavMeshNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  // Append our navigation mesh vizzes to the drawing, even though they're not
  // actually part of the scene graph.
  PT(PandaNode) node = _nav_mesh->draw_nav_mesh_geom();
  
  if(node != nullptr) {
    trav->traverse_down(data, node, data._net_transform, RenderState::make_empty());
  }

  // Now carry on to render our child nodes.
  return true;
}

/**
 * Called when needed to recompute the node's _internal_bound object.  Nodes
 * that contain anything of substance should redefine this to do the right
 * thing.
 */
void NavMeshNode::
compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                        int &internal_vertices,
                        int pipeline_stage,
                        Thread *current_thread) const {
  internal_bounds = new OmniBoundingVolume;
  internal_vertices = 0;
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

/**
 * This function is called by the BamReader's factory when a new object of
 * type BulletShape is encountered in the Bam file.  It should create the
 * BulletShape and extract its information from the file.
 */
TypedWritable *NavMeshNode::
make_from_bam(const FactoryParams &params) {
  std::string name = "FromBam";
  NavMeshNode *param = new NavMeshNode(name);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new BulletTriangleMeshShape.
 */
void NavMeshNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  manager->read_pointer(scan);
}


