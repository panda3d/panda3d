/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navObstacleCylinderNode.h
 * @author Maxwell175
 * @date 2022-12-15
 */

#ifndef NAVOBSTACLECYLINDERNODE_H
#define NAVOBSTACLECYLINDERNODE_H

#include <recastnavigation/DetourTileCache.h>
#include "navObstacleNode.h"
#include "navMeshBuilder.h"

/**
 * The NavObstacleCylinderNode is a navmesh obstacle in the form of a cylinder.
 */
class EXPCL_NAVIGATION NavObstacleCylinderNode : public NavObstacleNode
{
PUBLISHED:
  explicit NavObstacleCylinderNode(float radius, float height, const std::string &name);

  INLINE float get_radius() const;
  INLINE void set_radius(float radius);
  MAKE_PROPERTY(radius, get_radius, set_radius);

  INLINE float get_height() const;
  INLINE void set_height(float height);
  MAKE_PROPERTY(height, get_height, set_height);

protected:
  PT(GeomNode) get_debug_geom() override;

  friend class NavMesh;

private:
  float _radius;
  float _height;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "NavObstacleCylinderNode",
                  NavObstacleNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() { init_type(); return get_class_type(); }

  NavMeshBuilder::ObstacleData get_obstacle_data(const LMatrix4 &transform) override;
  void add_obstacle(std::shared_ptr<dtTileCache> tileCache, const LMatrix4 &transform) override;

private:
  static TypeHandle _type_handle;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

};

#include "navObstacleCylinderNode.I"

#endif // NAVOBSTACLECYLINDERNODE_H
