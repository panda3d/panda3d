/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navObstacleNode.h
 * @author Maxwell175
 * @date 2022-12-15
 */

#ifndef NAVOBSTACLENODE_H
#define NAVOBSTACLENODE_H

#include <string>
#include "geom.h"
#include "pandaSystem.h"
#include <string>
#include <tuple>
#include <pandaNode.h>


/**
 * The NavObstacleNode class is the base for all the types of NavMesh obstacle shapes.
 */
class EXPCL_NAVIGATION NavObstacleNode : public PandaNode
{
PUBLISHED:
  explicit NavObstacleNode(const std::string &name);

protected:
  virtual PT(GeomNode) get_debug_geom() = 0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "NavObstacleNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() { init_type(); return get_class_type(); }
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

  virtual NavMeshBuilder::ObstacleData get_obstacle_data(const LMatrix4 &transform) = 0;
  virtual void add_obstacle(dtTileCache *tileCache, const LMatrix4 &transform) = 0;

private:
  static TypeHandle _type_handle;

protected:
  virtual void compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                                       int &internal_vertices,
                                       int pipeline_stage,
                                       Thread *current_thread) const;

};

#include "navObstacleNode.I"

#endif // NAVOBSTACLENODE_H
