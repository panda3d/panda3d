// Filename: bulletSoftBodyNode.cxx
// Created by:  enn0x (27Dec10)
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

#include "bulletSoftBodyNode.h"
#include "bulletSoftBodyConfig.h"
#include "bulletSoftBodyControl.h"
#include "bulletSoftBodyMaterial.h"
#include "bulletSoftBodyShape.h"
#include "bulletSoftBodyWorldInfo.h"
#include "bulletHelper.h"

#include "geomVertexRewriter.h"
#include "geomVertexReader.h"

TypeHandle BulletSoftBodyNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BulletSoftBodyNode::
BulletSoftBodyNode(btSoftBody *body, const char *name) : BulletBodyNode(name) {

  // Synchronised transform
  _sync = TransformState::make_identity();
  _sync_disable = false;

  // Softbody
  _soft = body;
  _soft->setUserPointer(this);

  // Shape
  btCollisionShape *shape_ptr = _soft->getCollisionShape();

  nassertv(shape_ptr != NULL);
  nassertv(shape_ptr->getShapeType() == SOFTBODY_SHAPE_PROXYTYPE);

  _shapes.push_back(new BulletSoftBodyShape((btSoftBodyCollisionShape *)shape_ptr));

  // Rendering
  _geom = NULL;
  _curve = NULL;
  _surface = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::get_object
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btCollisionObject *BulletSoftBodyNode::
get_object() const {

  return _soft;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::get_cfg
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletSoftBodyConfig BulletSoftBodyNode::
get_cfg() {

  return BulletSoftBodyConfig(_soft->m_cfg);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::get_world_info
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletSoftBodyWorldInfo BulletSoftBodyNode::
get_world_info() {

  return BulletSoftBodyWorldInfo(*(_soft->m_worldInfo));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::get_num_materials
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
int BulletSoftBodyNode::
get_num_materials() const {

  return _soft->m_materials.size();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::get_material
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletSoftBodyMaterial BulletSoftBodyNode::
get_material(int idx) const {

  nassertr(idx >= 0 && idx < get_num_materials(), BulletSoftBodyMaterial::empty());

  btSoftBody::Material *material = _soft->m_materials[idx];
  return BulletSoftBodyMaterial(*material);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::append_material
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletSoftBodyMaterial BulletSoftBodyNode::
append_material() {

  btSoftBody::Material *material = _soft->appendMaterial();
  nassertr(material, BulletSoftBodyMaterial::empty());

  return BulletSoftBodyMaterial(*material);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::get_num_nodes
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
int BulletSoftBodyNode::
get_num_nodes() const {

  return _soft->m_nodes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::get_node
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletSoftBodyNodeElement BulletSoftBodyNode::
get_node(int idx) const {

  nassertr(idx >=0 && idx < get_num_nodes(), BulletSoftBodyNodeElement::empty());
  return BulletSoftBodyNodeElement(_soft->m_nodes[idx]);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::generate_bending_constraints
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
generate_bending_constraints(int distance, BulletSoftBodyMaterial *material) {

  if (material) {
    _soft->generateBendingConstraints(distance, &(material->get_material()));
  }
  else {
    _soft->generateBendingConstraints(distance);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::randomize_constraints
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
randomize_constraints() {

  _soft->randomizeConstraints();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::transform_changed
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
transform_changed() {

  if (_sync_disable) return;

  NodePath np = NodePath::any_path((PandaNode *)this);
  CPT(TransformState) ts = np.get_net_transform();

  LMatrix4 m_sync = _sync->get_mat();
  LMatrix4 m_ts = ts->get_mat();

  if (!m_sync.almost_equal(m_ts)) {

    // New transform for the center
    btTransform trans = TransformState_to_btTrans(ts);

    // Offset between current approx center and current initial transform
    btVector3 pos = LVecBase3_to_btVector3(this->get_aabb().get_approx_center());
    btVector3 origin = _soft->m_initialWorldTransform.getOrigin();
    btVector3 offset = pos - origin;

    // Subtract offset to get new transform for the body
    trans.setOrigin(trans.getOrigin() - offset);

    // Now apply the new transform
    _soft->transform(_soft->m_initialWorldTransform.inverse());
    _soft->transform(trans);

    if (ts->has_scale()) {
      btVector3 current_scale = LVecBase3_to_btVector3(_sync->get_scale());
      btVector3 new_scale = LVecBase3_to_btVector3(ts->get_scale());

      current_scale.setX(1.0 / current_scale.getX());
      current_scale.setY(1.0 / current_scale.getY());
      current_scale.setZ(1.0 / current_scale.getZ());

      _soft->scale(current_scale);
      _soft->scale(new_scale);
    }

    _sync = ts;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::sync_p2b
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
sync_p2b() {

  //transform_changed(); Disabled for now...
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::sync_b2p
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
sync_b2p() {

  // Render softbody
  if (_geom) {
    btTransform trans = btTransform::getIdentity();
    get_node_transform(trans, this);

    PT(GeomVertexData) vdata = _geom->modify_vertex_data();

    GeomVertexRewriter vertices(vdata, InternalName::get_vertex());
    GeomVertexRewriter normals(vdata, InternalName::get_normal());
    GeomVertexReader indices(vdata, BulletHelper::get_sb_index());
    GeomVertexReader flips(vdata, BulletHelper::get_sb_flip());

    while (!vertices.is_at_end()) {
      btSoftBody::Node node = _soft->m_nodes[indices.get_data1i()];
      btVector3 v = trans.invXform(node.m_x);
      btVector3 n = node.m_n;

      if (flips.get_data1i() > 0) n *= -1;

      vertices.set_data3((PN_stdfloat)v.getX(), (PN_stdfloat)v.getY(), (PN_stdfloat)v.getZ());
      normals.set_data3((PN_stdfloat)n.getX(), (PN_stdfloat)n.getY(), (PN_stdfloat)n.getZ());
    }
  }

  if (_curve) {
    btSoftBody::tNodeArray &nodes(_soft->m_nodes);

    for (int i=0; i < nodes.size(); i++) {
      btVector3 pos = nodes[i].m_x;
      _curve->set_vertex(i, btVector3_to_LPoint3(pos));
    }
  }

  if (_surface) {
    btSoftBody::tNodeArray &nodes(_soft->m_nodes);

    int num_u = _surface->get_num_u_vertices();
    int num_v = _surface->get_num_v_vertices();
    nassertv(num_u * num_v == nodes.size());

    for (int u=0; u < num_u; u++) {
      for (int v=0; v < num_v; v++) {
        btVector3 pos = nodes[u * num_u + v].m_x;
        _surface->set_vertex(u, v, btVector3_to_LPoint3(pos));
      }
    }
  }

  // Update the synchronized transform with the current
  // approximate center of the soft body
  LVecBase3 pos = this->get_aabb().get_approx_center();
  CPT(TransformState) ts = TransformState::make_pos(pos);

  NodePath np = NodePath::any_path((PandaNode *)this);
  LVecBase3 scale = np.get_net_transform()->get_scale();
  ts = ts->set_scale(scale);

  _sync = ts;
  _sync_disable = true;
  np.set_transform(NodePath(), ts);
  _sync_disable = false;
/*
*/

  Thread *current_thread = Thread::get_current_thread();
  this->r_mark_geom_bounds_stale(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::get_closest_node_index
//       Access: Published
//  Description: Returns the index of the node which is closest
//               to the given point. The distance between each node
//               and the given point is computed in world space
//               if local=false, and in local space if local=true.
////////////////////////////////////////////////////////////////////
int BulletSoftBodyNode::
get_closest_node_index(LVecBase3 point, bool local) {

  btScalar max_dist_sqr = 1e30;
  btVector3 point_x = LVecBase3_to_btVector3(point);

  btTransform trans = btTransform::getIdentity();
  if (local == true) {
    get_node_transform(trans, this);
  }

  btSoftBody::tNodeArray &nodes(_soft->m_nodes);
  int node_idx = 0;

  for (int i=0; i<nodes.size(); ++i) {

    btVector3 node_x = nodes[i].m_x;
    btScalar dist_sqr = (trans.invXform(node_x) - point_x).length2();

    if (dist_sqr < max_dist_sqr) {
      max_dist_sqr = dist_sqr;
      node_idx = i;
    }
  }

  return node_idx;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::link_geom
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
link_geom(Geom *geom) {

  nassertv(geom->get_vertex_data()->has_column(InternalName::get_vertex()));
  nassertv(geom->get_vertex_data()->has_column(InternalName::get_normal()));

  sync_p2b();

  _geom = geom;

  PT(GeomVertexData) vdata = _geom->modify_vertex_data();

  if (!vdata->has_column(BulletHelper::get_sb_index())) {
    CPT(GeomVertexFormat) format = vdata->get_format();
    format = BulletHelper::add_sb_index_column(format);
    vdata->set_format(format);
  }

  if (!vdata->has_column(BulletHelper::get_sb_flip())) {
    CPT(GeomVertexFormat) format = vdata->get_format();
    format = BulletHelper::add_sb_flip_column(format);
    vdata->set_format(format);
  }

  GeomVertexReader vertices(vdata, InternalName::get_vertex());
  GeomVertexRewriter indices(vdata, BulletHelper::get_sb_index());

  while (!vertices.is_at_end()) {
    LVecBase3 point = vertices.get_data3();
    int node_idx = get_closest_node_index(point, true);
    indices.set_data1i(node_idx);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::unlink_geom
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
unlink_geom() {

  _geom = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::link_curve
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
link_curve(NurbsCurveEvaluator *curve) {

  nassertv(curve->get_num_vertices() == _soft->m_nodes.size());

  _curve = curve;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::unlink_curve
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
unlink_curve() {

  _curve = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::link_surface
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
link_surface(NurbsSurfaceEvaluator *surface) {

  nassertv(surface->get_num_u_vertices() * surface->get_num_v_vertices() == _soft->m_nodes.size());

  _surface = surface;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::unlink_surface
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
unlink_surface() {

  _surface = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::get_aabb
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BoundingBox BulletSoftBodyNode::
get_aabb() const {

  btVector3 pMin;
  btVector3 pMax;

  _soft->getAabb(pMin, pMax);

  return BoundingBox(
    btVector3_to_LPoint3(pMin),
    btVector3_to_LPoint3(pMax)
    );
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::set_volume_mass
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
set_volume_mass(PN_stdfloat mass) {

  _soft->setVolumeMass(mass);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::set_total_mass
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
set_total_mass(PN_stdfloat mass, bool fromfaces) {

  _soft->setTotalMass(mass, fromfaces);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::set_volume_density
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
set_volume_density(PN_stdfloat density) {

  _soft->setVolumeDensity(density);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::set_total_density
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
set_total_density(PN_stdfloat density) {

  _soft->setTotalDensity(density);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::set_mass
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
set_mass(int node, PN_stdfloat mass) {

  _soft->setMass(node, mass);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::get_mass
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletSoftBodyNode::
get_mass(int node) const {

  return _soft->getMass(node);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::get_total_mass
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletSoftBodyNode::
get_total_mass() const {

  return _soft->getTotalMass();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::get_volume
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletSoftBodyNode::
get_volume() const {

  return _soft->getVolume();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::add_force
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
add_force(const LVector3 &force) {

  nassertv(!force.is_nan());
  _soft->addForce(LVecBase3_to_btVector3(force));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::add_force
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
add_force(const LVector3 &force, int node) {

  nassertv(!force.is_nan());
  _soft->addForce(LVecBase3_to_btVector3(force), node);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::set_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
set_velocity(const LVector3 &velocity) {

  nassertv(!velocity.is_nan());
  _soft->setVelocity(LVecBase3_to_btVector3(velocity));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::add_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
add_velocity(const LVector3 &velocity) {

  nassertv(!velocity.is_nan());
  _soft->addVelocity(LVecBase3_to_btVector3(velocity));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::add_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
add_velocity(const LVector3 &velocity, int node) {

  nassertv(!velocity.is_nan());
  _soft->addVelocity(LVecBase3_to_btVector3(velocity), node);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::generate_clusters
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
generate_clusters(int k, int maxiterations) {

  _soft->generateClusters(k, maxiterations);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::release_clusters
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
release_clusters() {

  _soft->releaseClusters();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::release_cluster
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
release_cluster(int index) {

  _soft->releaseCluster(index);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::get_num_clusters
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
int BulletSoftBodyNode::
get_num_clusters() const {

  return _soft->clusterCount();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::cluster_com
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVecBase3 BulletSoftBodyNode::
cluster_com(int cluster) const {

  return btVector3_to_LVecBase3(_soft->clusterCom(cluster));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::set_pose
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
set_pose(bool bvolume, bool bframe) {

  _soft->setPose(bvolume, bframe);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::append_anchor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
append_anchor(int node, BulletRigidBodyNode *body, bool disable) {

  nassertv(node < _soft->m_nodes.size())
  nassertv(body);

  body->sync_p2b();

  btRigidBody *ptr = (btRigidBody *)body->get_object();
  _soft->appendAnchor(node, ptr, disable);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::append_anchor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
append_anchor(int node, BulletRigidBodyNode *body, const LVector3 &pivot, bool disable) {

  nassertv(node < _soft->m_nodes.size())
  nassertv(body);
  nassertv(!pivot.is_nan());

  body->sync_p2b();

  btRigidBody *ptr = (btRigidBody *)body->get_object();
  _soft->appendAnchor(node, ptr, LVecBase3_to_btVector3(pivot), disable);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNodeElement::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BulletSoftBodyNodeElement::
BulletSoftBodyNodeElement(btSoftBody::Node &node) : _node(node) {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::get_point_index
//       Access: Private
//  Description: Returns the index of the first point within an
//               array of points which has about the same 
//               coordinates as the given point. If no points
//               is found -1 is returned.
////////////////////////////////////////////////////////////////////
int BulletSoftBodyNode::
get_point_index(LVecBase3 p, PTA_LVecBase3 points) {

  PN_stdfloat eps = 1.0e-6f; // TODO make this a config option

  for (PTA_LVecBase3::size_type i=0; i<points.size(); i++) {
    if (points[i].almost_equal(p, eps)) {
      return i; // Found
    }
  }

  return -1; // Not found
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::next_line
//       Access: Published
//  Description: Read on until the next linebreak is detected, or
//               the end of file has been reached.
////////////////////////////////////////////////////////////////////
int BulletSoftBodyNode::
next_line(const char* buffer) {

  int num_bytes_read = 0;

  while (*buffer != '\n') {
    buffer++;
    num_bytes_read++;
  }
  
  if (buffer[0] == 0x0a) {
    buffer++;
    num_bytes_read++;
  }

  return num_bytes_read;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::make_rope
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PT(BulletSoftBodyNode) BulletSoftBodyNode::
make_rope(BulletSoftBodyWorldInfo &info, const LPoint3 &from, const LPoint3 &to, int res, int fixeds) {

  btSoftBody *body = btSoftBodyHelpers::CreateRope(
    info.get_info(),
    LVecBase3_to_btVector3(from),
    LVecBase3_to_btVector3(to),
    res,
    fixeds);

  PT(BulletSoftBodyNode) node = new BulletSoftBodyNode(body);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::make_patch
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PT(BulletSoftBodyNode) BulletSoftBodyNode::
make_patch(BulletSoftBodyWorldInfo &info, const LPoint3 &corner00, const LPoint3 &corner10, const LPoint3 &corner01, const LPoint3 &corner11, int resx, int resy, int fixeds, bool gendiags) {

  btSoftBody *body = btSoftBodyHelpers::CreatePatch(
    info.get_info(),
    LVecBase3_to_btVector3(corner00),
    LVecBase3_to_btVector3(corner10),
    LVecBase3_to_btVector3(corner01),
    LVecBase3_to_btVector3(corner11),
    resx,
    resy,
    fixeds,
    gendiags);

  PT(BulletSoftBodyNode) node = new BulletSoftBodyNode(body);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::make_ellipsoid
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PT(BulletSoftBodyNode) BulletSoftBodyNode::
make_ellipsoid(BulletSoftBodyWorldInfo &info, const LPoint3 &center, const LVecBase3 &radius, int res) {

  btSoftBody *body = btSoftBodyHelpers::CreateEllipsoid(
    info.get_info(),
    LVecBase3_to_btVector3(center),
    LVecBase3_to_btVector3(radius),
    res);

  PT(BulletSoftBodyNode) node = new BulletSoftBodyNode(body);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::make_tri_mesh
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PT(BulletSoftBodyNode) BulletSoftBodyNode::
make_tri_mesh(BulletSoftBodyWorldInfo &info, PTA_LVecBase3 points, PTA_int indices, bool randomizeConstraints) {

  // Eliminate duplicate vertices
  PTA_LVecBase3 mapped_points;
  PTA_int mapped_indices;

  pmap<int, int> mapping;

  for (PTA_LVecBase3::size_type i=0; i<points.size(); i++) {
    LVecBase3 p = points[i];
    int j = get_point_index(p, mapped_points);
    if (j < 0) {
      mapping[i] = mapped_points.size();
      mapped_points.push_back(p);
    }
    else {
      mapping[i] = j;
    }
  }

  for (PTA_int::size_type i=0; i<indices.size(); i++) {
    int idx = indices[i];
    int mapped_idx = mapping[idx];
    mapped_indices.push_back(mapped_idx);
  }

  points = mapped_points;
  indices = mapped_indices;

  // Convert arrays
  int num_vertices = points.size();
  int num_triangles = indices.size() / 3;

  btScalar *vertices = new btScalar[num_vertices * 3];
  for (int i=0; i < num_vertices; i++) {
    vertices[3*i]   = points[i].get_x();
    vertices[3*i+1] = points[i].get_y();
    vertices[3*i+2] = points[i].get_z();
  }

  int *triangles = new int[num_triangles * 3];
  for (int i=0; i < num_triangles * 3; i++) {
    triangles[i] = indices[i];
  }

  // Create body
  btSoftBody *body = btSoftBodyHelpers::CreateFromTriMesh(
    info.get_info(),
    vertices,
    triangles,
    num_triangles,
    randomizeConstraints);

  nassertr(body, NULL);

  delete[] vertices;
  delete[] triangles;

  PT(BulletSoftBodyNode) node = new BulletSoftBodyNode(body);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::make_tri_mesh
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PT(BulletSoftBodyNode) BulletSoftBodyNode::
make_tri_mesh(BulletSoftBodyWorldInfo &info, const Geom *geom, bool randomizeConstraints) {

  // Read vertex data
  PTA_LVecBase3 points;
  PTA_int indices;

  CPT(GeomVertexData) vdata = geom->get_vertex_data();

  nassertr(vdata->has_column(InternalName::get_vertex()), NULL);

  GeomVertexReader vreader(vdata, InternalName::get_vertex());

  while (!vreader.is_at_end()) {
    LVecBase3 v = vreader.get_data3();
    points.push_back(v);
  }

  // Read indices
  for (int i=0; i<geom->get_num_primitives(); i++) {

    CPT(GeomPrimitive) prim = geom->get_primitive(i);
    prim = prim->decompose();

    for (int j=0; j<prim->get_num_primitives(); j++) {

      int s = prim->get_primitive_start(j);
      int e = prim->get_primitive_end(j);

      for (int k=s; k<e; k++) {
        indices.push_back(prim->get_vertex(k));
      }
    }
  }

  // Create body
  return make_tri_mesh(info, points, indices, randomizeConstraints);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::make_tet_mesh
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PT(BulletSoftBodyNode) BulletSoftBodyNode::
make_tet_mesh(BulletSoftBodyWorldInfo &info, PTA_LVecBase3 points, PTA_int indices, bool tetralinks) {

  // Points
  btAlignedObjectArray<btVector3> pos;
  pos.resize(points.size());
  for (PTA_LVecBase3::size_type i=0; i<points.size(); i++) {
    LVecBase3 point = points[i];
    pos[i] = LVecBase3_to_btVector3(point);
  }

  // Body
  btSoftBody* body = new btSoftBody(&info.get_info(), pos.size(), &pos[0], 0);

  // Indices
  for (PTA_int::size_type i=0; i<indices.size() / 4; i++) {
    int ni[4];

    ni[0] = indices[4*i];
    ni[1] = indices[4*i+1];
    ni[2] = indices[4*i+2];
    ni[3] = indices[4*i+3];

    body->appendTetra(ni[0],ni[1],ni[2],ni[3]);

    if (tetralinks) {
      body->appendLink(ni[0], ni[1], 0, true);
      body->appendLink(ni[1], ni[2], 0, true);
      body->appendLink(ni[2], ni[0], 0, true);
      body->appendLink(ni[0], ni[3], 0, true);
      body->appendLink(ni[1], ni[3], 0, true);
      body->appendLink(ni[2], ni[3], 0, true);
    }
  }

  // Node
  PT(BulletSoftBodyNode) node = new BulletSoftBodyNode(body);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::make_tet_mesh
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PT(BulletSoftBodyNode) BulletSoftBodyNode::
make_tet_mesh(BulletSoftBodyWorldInfo &info, const char *ele, const char *face, const char *node) {

  nassertr(node && node[0], NULL);

  // Nodes
  btAlignedObjectArray<btVector3> pos;

  int npos = 0;
  int ndims = 0; // not used
  int nattrb = 0; // not used
  int hasbounds = 0; // not used

  sscanf(node, "%d %d %d %d", &npos, &ndims, &nattrb, &hasbounds);
  node += next_line(node);

  pos.resize(npos);

  for (int i=0; i<pos.size(); ++i) {
    int index = 0;
    float x, y, z;

    sscanf(node, "%d %f %f %f", &index, &x, &y, &z);
    node += next_line(node);

    pos[index].setX(btScalar(x));
    pos[index].setY(btScalar(y));
    pos[index].setZ(btScalar(z));
  }

  // Body
  btSoftBody *body = new btSoftBody(&info.get_info(), npos, &pos[0], 0);

  // Faces
  if (face && face[0]) {
    int nface = 0;
    int hasbounds = 0; // not used

    sscanf(face, "%d %d", &nface, &hasbounds);
    face += next_line(face);

    for (int i=0; i<nface; ++i) {
      int index = 0;
      int ni[3];

      sscanf(face, "%d %d %d %d", &index, &ni[0], &ni[1], &ni[2]);
      face += next_line(face);

      body->appendFace(ni[0], ni[1], ni[2]);
    }
  }

  // Links
  if (ele && ele[0]) {
    int ntetra = 0;
    int ncorner = 0;
    int neattrb = 0;

    sscanf(ele, "%d %d %d", &ntetra, &ncorner, &neattrb);
    ele += next_line(ele);

    for (int i=0; i<ntetra; ++i) {
      int index = 0;
      int ni[4];

      sscanf(ele, "%d %d %d %d %d", &index, &ni[0], &ni[1], &ni[2], &ni[3]);
      ele += next_line(ele);

      body->appendTetra(ni[0], ni[1], ni[2], ni[3]);

      body->appendLink(ni[0], ni[1], 0, true);
      body->appendLink(ni[1], ni[2], 0, true);
      body->appendLink(ni[2], ni[0], 0, true);
      body->appendLink(ni[0], ni[3], 0, true);
      body->appendLink(ni[1], ni[3], 0, true);
      body->appendLink(ni[2], ni[3], 0, true);
    }
  }

  // Node
  PT(BulletSoftBodyNode) sbnode = new BulletSoftBodyNode(body);

  return sbnode;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::append_linear_joint
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
append_linear_joint(BulletBodyNode *body, int cluster, PN_stdfloat erp, PN_stdfloat cfm, PN_stdfloat split) {

  nassertv(body);

  btCollisionObject *ptr = body->get_object();

  btSoftBody::LJoint::Specs ls;
  ls.erp = erp;
  ls.cfm = cfm;
  ls.split = split;
  ls.position = _soft->clusterCom(cluster);

  _soft->appendLinearJoint(ls, ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::append_linear_joint
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
append_linear_joint(BulletBodyNode *body, const LPoint3 &pos, PN_stdfloat erp, PN_stdfloat cfm, PN_stdfloat split) {

  nassertv(body);

  btCollisionObject *ptr = body->get_object();

  btSoftBody::LJoint::Specs ls;
  ls.erp = erp;
  ls.cfm = cfm;
  ls.split = split;
  ls.position = LVecBase3_to_btVector3(pos);

  _soft->appendLinearJoint(ls, ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::append_angular_joint
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
append_angular_joint(BulletBodyNode *body, const LVector3 &axis, PN_stdfloat erp, PN_stdfloat cfm, PN_stdfloat split, BulletSoftBodyControl *control) {

  nassertv(body);

  btCollisionObject *ptr = body->get_object();

  btSoftBody::AJoint::Specs as;
  as.erp = erp;
  as.cfm = cfm;
  as.split = split;
  as.axis = LVecBase3_to_btVector3(axis);
  as.icontrol = control ? control : btSoftBody::AJoint::IControl::Default();

  _soft->appendAngularJoint(as, ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::set_wind_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletSoftBodyNode::
set_wind_velocity(const LVector3 &velocity) {

  nassertv(!velocity.is_nan());
  _soft->setWindVelocity(LVecBase3_to_btVector3(velocity));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNode::get_wind_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVector3 BulletSoftBodyNode::
get_wind_velocity() const {

  return btVector3_to_LVector3(_soft->getWindVelocity());
}

