/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSoftBodyNode.h
 * @author enn0x
 * @date 2010-12-27
 */

#ifndef __BULLET_SOFT_BODY_NODE_H__
#define __BULLET_SOFT_BODY_NODE_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"
#include "bulletBodyNode.h"

#include "collideMask.h"
#include "geom.h"
#include "geomNode.h"
#include "geomVertexFormat.h"
#include "boundingBox.h"
#include "nurbsCurveEvaluator.h"
#include "nurbsSurfaceEvaluator.h"
#include "pta_LVecBase3.h"

class BulletRigidBodyNode;
class BulletSoftBodyConfig;
class BulletSoftBodyControl;
class BulletSoftBodyMaterial;
class BulletSoftBodyWorldInfo;

/**
 *
 */
class EXPCL_PANDABULLET BulletSoftBodyNodeElement {

PUBLISHED:
  INLINE ~BulletSoftBodyNodeElement();
  INLINE static BulletSoftBodyNodeElement empty();

  LPoint3 get_pos() const;
  LVector3 get_velocity() const;
  LVector3 get_normal() const;
  PN_stdfloat get_inv_mass() const;
  PN_stdfloat get_area() const;
  int is_attached() const;

  MAKE_PROPERTY(pos, get_pos);
  MAKE_PROPERTY(velocity, get_velocity);
  MAKE_PROPERTY(normal, get_normal);
  MAKE_PROPERTY(inv_mass, get_inv_mass);
  MAKE_PROPERTY(area, get_area);
  MAKE_PROPERTY(attached, is_attached);

public:
  BulletSoftBodyNodeElement(btSoftBody::Node &node);

private:
  btSoftBody::Node &_node;
};

/**
 *
 */
class EXPCL_PANDABULLET BulletSoftBodyNode : public BulletBodyNode {

public:
  BulletSoftBodyNode(btSoftBody *body, const char *name="softbody");

PUBLISHED:
  INLINE ~BulletSoftBodyNode();

  BulletSoftBodyConfig get_cfg();
  BulletSoftBodyWorldInfo get_world_info();

  void generate_bending_constraints(int distance, BulletSoftBodyMaterial *material=nullptr);
  void randomize_constraints();

  // Mass, volume, density
  void set_volume_mass(PN_stdfloat mass);
  void set_volume_density(PN_stdfloat density);
  void set_total_mass(PN_stdfloat mass, bool fromfaces=false);
  void set_total_density(PN_stdfloat density);
  void set_mass(int node, PN_stdfloat mass);

  PN_stdfloat get_mass(int node) const;
  PN_stdfloat get_total_mass() const;
  PN_stdfloat get_volume() const;

  // Force
  void add_force(const LVector3 &force);
  void add_force(const LVector3 &force, int node);

  void set_velocity(const LVector3 &velocity);
  void add_velocity(const LVector3 &velocity);
  void add_velocity(const LVector3 &velocity, int node);

  void set_wind_velocity(const LVector3 &velocity);
  LVector3 get_wind_velocity() const;

  void set_pose(bool bvolume, bool bframe);

  BoundingBox get_aabb() const;

  // Cluster
  void generate_clusters(int k, int maxiterations=8192);
  void release_cluster(int index);
  void release_clusters();
  int get_num_clusters() const;
  LVecBase3 cluster_com(int cluster) const;

  // Rendering
  void link_geom(Geom *geom);
  void unlink_geom();

  void link_curve(NurbsCurveEvaluator *curve);
  void unlink_curve();

  void link_surface(NurbsSurfaceEvaluator *surface);
  void unlink_surface();

  // Anchors
  void append_anchor(int node, BulletRigidBodyNode *body,
      bool disable=false);
  void append_anchor(int node, BulletRigidBodyNode *body,
      const LVector3 &pivot,
      bool disable=false);

  // Links
  void append_linear_joint(BulletBodyNode *body, int cluster,
    PN_stdfloat erp=1.0,
    PN_stdfloat cfm=1.0,
    PN_stdfloat split=1.0);

  void append_linear_joint(BulletBodyNode *body, const LPoint3 &pos,
    PN_stdfloat erp=1.0,
    PN_stdfloat cfm=1.0,
    PN_stdfloat split=1.0);

  void append_angular_joint(BulletBodyNode *body, const LVector3 &axis,
    PN_stdfloat erp=1.0,
    PN_stdfloat cfm=1.0,
    PN_stdfloat split=1.0,
    BulletSoftBodyControl *control=nullptr);

  // Materials
  int get_num_materials() const;
  BulletSoftBodyMaterial get_material(int idx) const;
  MAKE_SEQ(get_materials, get_num_materials, get_material);

  BulletSoftBodyMaterial append_material();

  // Nodes
  int get_num_nodes() const;
  BulletSoftBodyNodeElement get_node(int idx) const;
  MAKE_SEQ(get_nodes, get_num_nodes, get_node);

  int get_closest_node_index(LVecBase3 point, bool local);

  // Factory
  static PT(BulletSoftBodyNode) make_rope(
      BulletSoftBodyWorldInfo &info,
      const LPoint3 &from,
      const LPoint3 &to,
      int res,
      int fixeds);

  static PT(BulletSoftBodyNode) make_patch(
      BulletSoftBodyWorldInfo &info,
      const LPoint3 &corner00,
      const LPoint3 &corner10,
      const LPoint3 &corner01,
      const LPoint3 &corner11,
      int resx,
      int resy,
      int fixeds,
      bool gendiags);

  static PT(BulletSoftBodyNode) make_ellipsoid(
      BulletSoftBodyWorldInfo &info,
      const LPoint3 &center,
      const LVecBase3 &radius,
      int res);

  static PT(BulletSoftBodyNode) make_tri_mesh(
      BulletSoftBodyWorldInfo &info,
      const Geom *geom,
      bool randomizeConstraints=true);

  static PT(BulletSoftBodyNode) make_tri_mesh(
      BulletSoftBodyWorldInfo &info,
      PTA_LVecBase3 points,
      PTA_int indices,
      bool randomizeConstraints=true);

  static PT(BulletSoftBodyNode) make_tet_mesh(
      BulletSoftBodyWorldInfo &info,
      PTA_LVecBase3 points,
      PTA_int indices,
      bool tetralinks=true);

  static PT(BulletSoftBodyNode) make_tet_mesh(
      BulletSoftBodyWorldInfo &info,
      const char *ele,
      const char *face,
      const char *node);

  MAKE_PROPERTY(cfg, get_cfg);
  MAKE_PROPERTY(world_info, get_world_info);
  MAKE_PROPERTY(wind_velocity, get_wind_velocity, set_wind_velocity);
  MAKE_PROPERTY(aabb, get_aabb);
  MAKE_PROPERTY(num_clusters, get_num_clusters);
  MAKE_SEQ_PROPERTY(materials, get_num_materials, get_material);
  MAKE_SEQ_PROPERTY(nodes, get_num_nodes, get_node);

public:
  virtual btCollisionObject *get_object() const;

  void do_sync_p2b();
  void do_sync_b2p();

protected:
  virtual void transform_changed();

private:
  btSoftBody *_soft;

  CPT(TransformState) _sync;
  bool _sync_disable;

  PT(Geom) _geom;
  PT(NurbsCurveEvaluator) _curve;
  PT(NurbsSurfaceEvaluator) _surface;

  static int get_point_index(LVecBase3 p, PTA_LVecBase3 points);
  static int next_line(const char *buffer);

  BoundingBox do_get_aabb() const;
  int do_get_closest_node_index(LVecBase3 point, bool local);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BulletBodyNode::init_type();
    register_type(_type_handle, "BulletSoftBodyNode",
                  BulletBodyNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "bulletSoftBodyNode.I"

#endif // __BULLET_SOFT_BODY_NODE_H__
