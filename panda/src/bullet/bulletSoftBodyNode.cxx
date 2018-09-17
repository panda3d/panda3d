/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSoftBodyNode.cxx
 * @author enn0x
 * @date 2010-12-27
 */

#include "bulletSoftBodyNode.h"

#include "bulletSoftBodyConfig.h"
#include "bulletSoftBodyControl.h"
#include "bulletSoftBodyMaterial.h"
#include "bulletSoftBodyShape.h"
#include "bulletSoftBodyWorldInfo.h"
#include "bulletHelper.h"
#include "bulletWorld.h"

#include "geomVertexRewriter.h"
#include "geomVertexReader.h"

TypeHandle BulletSoftBodyNode::_type_handle;

/**
 *
 */
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

  nassertv(shape_ptr != nullptr);
  nassertv(shape_ptr->getShapeType() == SOFTBODY_SHAPE_PROXYTYPE);

  _shapes.push_back(new BulletSoftBodyShape((btSoftBodyCollisionShape *)shape_ptr));

  // Rendering
  _geom = nullptr;
  _curve = nullptr;
  _surface = nullptr;
}

/**
 *
 */
btCollisionObject *BulletSoftBodyNode::
get_object() const {

  return _soft;
}

/**
 *
 */
BulletSoftBodyConfig BulletSoftBodyNode::
get_cfg() {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return BulletSoftBodyConfig(_soft->m_cfg);
}

/**
 *
 */
BulletSoftBodyWorldInfo BulletSoftBodyNode::
get_world_info() {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return BulletSoftBodyWorldInfo(*(_soft->m_worldInfo));
}

/**
 *
 */
int BulletSoftBodyNode::
get_num_materials() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _soft->m_materials.size();
}

/**
 *
 */
BulletSoftBodyMaterial BulletSoftBodyNode::
get_material(int idx) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertr(idx >= 0 && idx < _soft->m_materials.size(), BulletSoftBodyMaterial::empty());

  btSoftBody::Material *material = _soft->m_materials[idx];
  return BulletSoftBodyMaterial(*material);
}

/**
 *
 */
BulletSoftBodyMaterial BulletSoftBodyNode::
append_material() {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  btSoftBody::Material *material = _soft->appendMaterial();
  nassertr(material, BulletSoftBodyMaterial::empty());

  return BulletSoftBodyMaterial(*material);
}

/**
 *
 */
int BulletSoftBodyNode::
get_num_nodes() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _soft->m_nodes.size();
}

/**
 *
 */
BulletSoftBodyNodeElement BulletSoftBodyNode::
get_node(int idx) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertr(idx >= 0 && idx < _soft->m_nodes.size(), BulletSoftBodyNodeElement::empty());
  return BulletSoftBodyNodeElement(_soft->m_nodes[idx]);
}

/**
 *
 */
void BulletSoftBodyNode::
generate_bending_constraints(int distance, BulletSoftBodyMaterial *material) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  if (material) {
    _soft->generateBendingConstraints(distance, &(material->get_material()));
  }
  else {
    _soft->generateBendingConstraints(distance);
  }
}

/**
 *
 */
void BulletSoftBodyNode::
randomize_constraints() {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _soft->randomizeConstraints();
}

/**
 *
 */
void BulletSoftBodyNode::
transform_changed() {
  if (_sync_disable) return;

  LightMutexHolder holder(BulletWorld::get_global_lock());

  NodePath np = NodePath::any_path((PandaNode *)this);
  CPT(TransformState) ts = np.get_net_transform();

  LMatrix4 m_sync = _sync->get_mat();
  LMatrix4 m_ts = ts->get_mat();

  if (!m_sync.almost_equal(m_ts)) {

    // New transform for the center
    btTransform trans = TransformState_to_btTrans(ts);

    // Offset between current approx center and current initial transform
    btVector3 pos = LVecBase3_to_btVector3(this->do_get_aabb().get_approx_center());
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

    _sync = std::move(ts);
  }
}

/**
 *
 */
void BulletSoftBodyNode::
do_sync_p2b() {

  // transform_changed(); Disabled for now...
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletSoftBodyNode::
do_sync_b2p() {

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

  // Update the synchronized transform with the current approximate center of
  // the soft body
  btVector3 pMin, pMax;
  _soft->getAabb(pMin, pMax);
  LPoint3 pos = (btVector3_to_LPoint3(pMin) + btVector3_to_LPoint3(pMax)) * 0.5;
  CPT(TransformState) ts;
  if (!pos.is_nan()) {
    ts = TransformState::make_pos(pos);
  } else {
    ts = TransformState::make_identity();
  }

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

/**
 * Returns the index of the node which is closest to the given point.  The
 * distance between each node and the given point is computed in world space
 * if local=false, and in local space if local=true.
 */
int BulletSoftBodyNode::
get_closest_node_index(LVecBase3 point, bool local) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return do_get_closest_node_index(point, local);
}

/**
 * Returns the index of the node which is closest to the given point.  The
 * distance between each node and the given point is computed in world space
 * if local=false, and in local space if local=true.
 * Assumes the lock(bullet global lock) is held by the caller
 */
int BulletSoftBodyNode::
do_get_closest_node_index(LVecBase3 point, bool local) {

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

/**
 *
 */
void BulletSoftBodyNode::
link_geom(Geom *geom) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(geom->get_vertex_data()->has_column(InternalName::get_vertex()));
  nassertv(geom->get_vertex_data()->has_column(InternalName::get_normal()));

  do_sync_p2b();

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
    int node_idx = do_get_closest_node_index(point, true);
    indices.set_data1i(node_idx);
  }
}

/**
 *
 */
void BulletSoftBodyNode::
unlink_geom() {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _geom = nullptr;
}

/**
 *
 */
void BulletSoftBodyNode::
link_curve(NurbsCurveEvaluator *curve) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(curve->get_num_vertices() == _soft->m_nodes.size());

  _curve = curve;
}

/**
 *
 */
void BulletSoftBodyNode::
unlink_curve() {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _curve = nullptr;
}

/**
 *
 */
void BulletSoftBodyNode::
link_surface(NurbsSurfaceEvaluator *surface) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(surface->get_num_u_vertices() * surface->get_num_v_vertices() == _soft->m_nodes.size());

  _surface = surface;
}

/**
 *
 */
void BulletSoftBodyNode::
unlink_surface() {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _surface = nullptr;
}

/**
 *
 */
BoundingBox BulletSoftBodyNode::
get_aabb() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return do_get_aabb();
}

/**
 *
 */
BoundingBox BulletSoftBodyNode::
do_get_aabb() const {

  btVector3 pMin;
  btVector3 pMax;

  _soft->getAabb(pMin, pMax);

  return BoundingBox(
    btVector3_to_LPoint3(pMin),
    btVector3_to_LPoint3(pMax)
    );
}

/**
 *
 */
void BulletSoftBodyNode::
set_volume_mass(PN_stdfloat mass) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _soft->setVolumeMass(mass);
}

/**
 *
 */
void BulletSoftBodyNode::
set_total_mass(PN_stdfloat mass, bool fromfaces) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _soft->setTotalMass(mass, fromfaces);
}

/**
 *
 */
void BulletSoftBodyNode::
set_volume_density(PN_stdfloat density) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _soft->setVolumeDensity(density);
}

/**
 *
 */
void BulletSoftBodyNode::
set_total_density(PN_stdfloat density) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _soft->setTotalDensity(density);
}

/**
 *
 */
void BulletSoftBodyNode::
set_mass(int node, PN_stdfloat mass) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _soft->setMass(node, mass);
}

/**
 *
 */
PN_stdfloat BulletSoftBodyNode::
get_mass(int node) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _soft->getMass(node);
}

/**
 *
 */
PN_stdfloat BulletSoftBodyNode::
get_total_mass() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _soft->getTotalMass();
}

/**
 *
 */
PN_stdfloat BulletSoftBodyNode::
get_volume() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _soft->getVolume();
}

/**
 *
 */
void BulletSoftBodyNode::
add_force(const LVector3 &force) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!force.is_nan());
  _soft->addForce(LVecBase3_to_btVector3(force));
}

/**
 *
 */
void BulletSoftBodyNode::
add_force(const LVector3 &force, int node) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!force.is_nan());
  _soft->addForce(LVecBase3_to_btVector3(force), node);
}

/**
 *
 */
void BulletSoftBodyNode::
set_velocity(const LVector3 &velocity) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!velocity.is_nan());
  _soft->setVelocity(LVecBase3_to_btVector3(velocity));
}

/**
 *
 */
void BulletSoftBodyNode::
add_velocity(const LVector3 &velocity) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!velocity.is_nan());
  _soft->addVelocity(LVecBase3_to_btVector3(velocity));
}

/**
 *
 */
void BulletSoftBodyNode::
add_velocity(const LVector3 &velocity, int node) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!velocity.is_nan());
  _soft->addVelocity(LVecBase3_to_btVector3(velocity), node);
}

/**
 *
 */
void BulletSoftBodyNode::
generate_clusters(int k, int maxiterations) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _soft->generateClusters(k, maxiterations);
}

/**
 *
 */
void BulletSoftBodyNode::
release_clusters() {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _soft->releaseClusters();
}

/**
 *
 */
void BulletSoftBodyNode::
release_cluster(int index) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _soft->releaseCluster(index);
}

/**
 *
 */
int BulletSoftBodyNode::
get_num_clusters() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _soft->clusterCount();
}

/**
 *
 */
LVecBase3 BulletSoftBodyNode::
cluster_com(int cluster) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVecBase3(_soft->clusterCom(cluster));
}

/**
 *
 */
void BulletSoftBodyNode::
set_pose(bool bvolume, bool bframe) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _soft->setPose(bvolume, bframe);
}

/**
 *
 */
void BulletSoftBodyNode::
append_anchor(int node, BulletRigidBodyNode *body, bool disable) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(node < _soft->m_nodes.size())
  nassertv(body);

  body->do_sync_p2b();

  btRigidBody *ptr = (btRigidBody *)body->get_object();
  _soft->appendAnchor(node, ptr, disable);
}

/**
 *
 */
void BulletSoftBodyNode::
append_anchor(int node, BulletRigidBodyNode *body, const LVector3 &pivot, bool disable) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(node < _soft->m_nodes.size())
  nassertv(body);
  nassertv(!pivot.is_nan());

  body->do_sync_p2b();

  btRigidBody *ptr = (btRigidBody *)body->get_object();
  _soft->appendAnchor(node, ptr, LVecBase3_to_btVector3(pivot), disable);
}

/**
 *
 */
BulletSoftBodyNodeElement::
BulletSoftBodyNodeElement(btSoftBody::Node &node) : _node(node) {

}

/**
 * Returns the index of the first point within an array of points which has
 * about the same coordinates as the given point.  If no points is found -1 is
 * returned.
 */
int BulletSoftBodyNode::
get_point_index(LVecBase3 p, PTA_LVecBase3 points) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  PN_stdfloat eps = 1.0e-6f; // TODO make this a config option

  for (PTA_LVecBase3::size_type i=0; i<points.size(); i++) {
    if (points[i].almost_equal(p, eps)) {
      return i; // Found
    }
  }

  return -1; // Not found
}

/**
 * Read on until the next linebreak is detected, or the end of file has been
 * reached.
 */
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

/**
 *
 */
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

/**
 *
 */
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

/**
 *
 */
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

/**
 *
 */
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

  nassertr(body, nullptr);

  delete[] vertices;
  delete[] triangles;

  PT(BulletSoftBodyNode) node = new BulletSoftBodyNode(body);

  return node;
}

/**
 *
 */
PT(BulletSoftBodyNode) BulletSoftBodyNode::
make_tri_mesh(BulletSoftBodyWorldInfo &info, const Geom *geom, bool randomizeConstraints) {

  // Read vertex data
  PTA_LVecBase3 points;
  PTA_int indices;

  CPT(GeomVertexData) vdata = geom->get_vertex_data();

  nassertr(vdata->has_column(InternalName::get_vertex()), nullptr);

  GeomVertexReader vreader(vdata, InternalName::get_vertex());

  while (!vreader.is_at_end()) {
    LVecBase3 v = vreader.get_data3();
    points.push_back(v);
  }

  // Read indices
  for (size_t i = 0; i < geom->get_num_primitives(); ++i) {

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

/**
 *
 */
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

/**
 *
 */
PT(BulletSoftBodyNode) BulletSoftBodyNode::
make_tet_mesh(BulletSoftBodyWorldInfo &info, const char *ele, const char *face, const char *node) {

  nassertr(node && node[0], nullptr);

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

/**
 *
 */
void BulletSoftBodyNode::
append_linear_joint(BulletBodyNode *body, int cluster, PN_stdfloat erp, PN_stdfloat cfm, PN_stdfloat split) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(body);

  btCollisionObject *ptr = body->get_object();

  btSoftBody::LJoint::Specs ls;
  ls.erp = erp;
  ls.cfm = cfm;
  ls.split = split;
  ls.position = _soft->clusterCom(cluster);

  _soft->appendLinearJoint(ls, ptr);
}

/**
 *
 */
void BulletSoftBodyNode::
append_linear_joint(BulletBodyNode *body, const LPoint3 &pos, PN_stdfloat erp, PN_stdfloat cfm, PN_stdfloat split) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(body);

  btCollisionObject *ptr = body->get_object();

  btSoftBody::LJoint::Specs ls;
  ls.erp = erp;
  ls.cfm = cfm;
  ls.split = split;
  ls.position = LVecBase3_to_btVector3(pos);

  _soft->appendLinearJoint(ls, ptr);
}

/**
 *
 */
void BulletSoftBodyNode::
append_angular_joint(BulletBodyNode *body, const LVector3 &axis, PN_stdfloat erp, PN_stdfloat cfm, PN_stdfloat split, BulletSoftBodyControl *control) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

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

/**
 *
 */
void BulletSoftBodyNode::
set_wind_velocity(const LVector3 &velocity) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!velocity.is_nan());
  _soft->setWindVelocity(LVecBase3_to_btVector3(velocity));
}

/**
 *
 */
LVector3 BulletSoftBodyNode::
get_wind_velocity() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_soft->getWindVelocity());
}

/**
 *
 */
LPoint3 BulletSoftBodyNodeElement::
get_pos() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LPoint3(_node.m_x);
}

/**
 *
 */
LVector3 BulletSoftBodyNodeElement::
get_normal() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_node.m_n);
}

/**
 *
 */
LVector3 BulletSoftBodyNodeElement::
get_velocity() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_node.m_v);
}

/**
 *
 */
PN_stdfloat BulletSoftBodyNodeElement::
get_inv_mass() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_node.m_im;
}

/**
 *
 */
PN_stdfloat BulletSoftBodyNodeElement::
get_area() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_node.m_area;
}

/**
 *
 */
int BulletSoftBodyNodeElement::
is_attached() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_node.m_battach;
}
