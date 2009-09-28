// Filename: portalClipper.h
// Created by:  masad (4May04)
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

#ifndef PORTALCLIPPER_H
#define PORTALCLIPPER_H

#include "pandabase.h"

#include "geom.h"
#include "geomVertexData.h"
#include "sceneSetup.h"
#include "renderState.h"
#include "portalNode.h"
#include "transformState.h"
#include "geometricBoundingVolume.h"
#include "boundingHexahedron.h"
#include "pointerTo.h"
#include "drawMask.h"
#include "typedObject.h"
#include "pStatCollector.h"
#include "config_pgraph.h"

#include "geom.h"
#include "geomNode.h"

class PandaNode;
class PortalNode;
class CullHandler;
class CullTraverserData;
class CullableObject;
class NodePath;

#define _FACING_THRESHOLD 0.0  //about 90 degrees with the camera

////////////////////////////////////////////////////////////////////
//       Class : PortalClipper
// Description : This object performs a depth-first traversal of the
//               scene graph, with optional view-frustum culling,
//               collecting CullState and searching for GeomNodes.
//               Each renderable Geom encountered is passed along with
//               its associated RenderState to the CullHandler object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH PortalClipper : public TypedObject {
public:
  PortalClipper(GeometricBoundingVolume *frustum, SceneSetup *scene_setup);
  ~PortalClipper();

  INLINE bool is_partial_portal_in_view();
  INLINE bool is_facing_view(Planef portal_plane);
  INLINE bool is_whole_portal_in_view(LMatrix4f cmat);

  bool prepare_portal(const NodePath &node_path);
  
  void draw_lines();
  INLINE void draw_camera_frustum();
  void draw_hexahedron(BoundingHexahedron *frustum);

  INLINE void move_to(float x, float y, float z);
  void move_to(const LVecBase3f &v);

  INLINE void draw_to(float x, float y, float z);
  void draw_to(const LVecBase3f &v);

  void draw_current_portal();

  INLINE BoundingHexahedron *get_reduced_frustum() const;
  INLINE void set_reduced_frustum(BoundingHexahedron *bh);
  INLINE void get_reduced_viewport(LPoint2f& min, LPoint2f& max) const; 
  INLINE void set_reduced_viewport(const LPoint2f& min, const LPoint2f& max);
  INLINE const RenderState* get_clip_state() const; 
  INLINE void set_clip_state(const RenderState* clip_state);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "PortalClipper",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

private:
  class Point {
  public:
    INLINE Point();
    INLINE Point(const LVecBase3f &point, const Colorf &color);
    INLINE Point(const Point &copy);
    INLINE void operator = (const Point &copy);

    Vertexf _point;
    Colorf _color;
  };

  typedef pvector<Point> SegmentList;
  typedef pvector<SegmentList> LineList;

  LineList _list;
  Colorf _color;

  PT(GeomVertexData) _created_data;

  BoundingHexahedron *_view_frustum;
  BoundingHexahedron *_reduced_frustum;
  LPoint2f _reduced_viewport_min;
  LPoint2f _reduced_viewport_max;
  CPT(RenderState) _clip_state; // each portal node needs to know the clip state of its "parent" portal Node

  PortalNode *_portal_node;  // current working portal for dereference ease

  //int _num_vert;
  //Vertexf _coords[4];

public:
  PT(GeomNode) _previous;
  SceneSetup *_scene_setup;
};

#include "portalClipper.I"

#endif


  
