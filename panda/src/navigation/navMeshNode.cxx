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
  // Append our collision vizzes to the drawing, even though they're not
  // actually part of the scene graph.
  PT(PandaNode) node = _nav_mesh->draw_nav_mesh_geom();
  if(node != nullptr) {
  	CullTraverserData next_data(data, node);
  	next_data._state = RenderState::make_empty();
  	trav->traverse(next_data);
  }
  // Solids::const_iterator si;
  // for (si = _solids.begin(); si != _solids.end(); ++si) {
  //   CPT(CollisionSolid) solid = (*si).get_read_pointer();
  //   PT(PandaNode) node = solid->get_viz(trav, data, false);
  //   if (node != nullptr) {
  //     CullTraverserData next_data(data, node);

  //     // We don't want to inherit the render state from above for these guys.
  //     next_data._state = RenderState::make_empty();
  //     trav->traverse(next_data);
  //   }
  // }

  

  // Now carry on to render our child nodes.
  return true;
}

/**
 * Returns true if there is some value to visiting this particular node during
 * the cull traversal for any camera, false otherwise.  This will be used to
 * optimize the result of get_net_draw_show_mask(), so that any subtrees that
 * contain only nodes for which is_renderable() is false need not be visited.
 */
bool NavMeshNode::
is_renderable() const {
  return true;
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


