// Filename: collisionVisualizer.h
// Created by:  drose (16Apr03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef COLLISIONVISUALIZER_H
#define COLLISIONVISUALIZER_H

#include "pandabase.h"
#include "pandaNode.h"
#include "collisionRecorder.h"
#include "nodePath.h"
#include "pmap.h"

#ifdef DO_COLLISION_RECORDING

////////////////////////////////////////////////////////////////////
//       Class : CollisionVisualizer
// Description : This class is used to help debug the work the
//               collisions system is doing.  It shows the polygons
//               that are detected as collisions, as well as those
//               that are simply considered for collisions.
//
//               It may be parented anywhere in the scene graph where
//               it will be rendered to achieve this.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionVisualizer : public PandaNode, public CollisionRecorder {
PUBLISHED:
  CollisionVisualizer(const string &name);
  virtual ~CollisionVisualizer();

  INLINE void set_viz_scale(float viz_scale);
  INLINE float get_viz_scale() const;

  void clear();

public:
  // from parent class PandaNode.
  virtual PandaNode *make_copy() const;
  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual void output(ostream &out) const;

  // from parent class CollisionRecorder.
  virtual void begin_traversal();
  virtual void collision_tested(const CollisionEntry &entry, bool detected);

private:
  CPT(RenderState) get_viz_state();

private:
  class SolidInfo {
  public:
    INLINE SolidInfo();
    int _detected_count;
    int _missed_count;
  };
  typedef pmap<CPT(CollisionSolid), SolidInfo> Solids;

  class CollisionPoint {
  public:
    LPoint3f _surface_point;
    LVector3f _surface_normal;
    LPoint3f _interior_point;
  };
  typedef pvector<CollisionPoint> Points;

  class VizInfo {
  public:
    Solids _solids;
    Points _points;
  };

  typedef pmap<CPT(TransformState), VizInfo> Data;
  Data _data;

  float _viz_scale;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

  static void init_type() {
    PandaNode::init_type();
    CollisionRecorder::init_type();
    register_type(_type_handle, "CollisionVisualizer",
                  PandaNode::get_class_type(),
                  CollisionRecorder::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionVisualizer.I"

#endif  // DO_COLLISION_RECORDING  

#endif
