// Filename: portalClipper.h
// Created by:  masad (4May04)
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

#ifndef PORTALCLIPPER_H
#define PORTALCLIPPER_H

#include "pandabase.h"

#include "geom.h"
#include "sceneSetup.h"
#include "renderState.h"
#include "transformState.h"
#include "geometricBoundingVolume.h"
#include "boundingHexahedron.h"
#include "pointerTo.h"
#include "drawMask.h"
#include "typedObject.h"
#include "pStatCollector.h"

#include "geom.h"
#include "geomPoint.h"
#include "geomLine.h"
#include "geomLinestrip.h"
#include "geomNode.h"

class PandaNode;
class CullHandler;
class CullTraverserData;
class CullableObject;
class NodePath;

////////////////////////////////////////////////////////////////////
//       Class : PortalClipper
// Description : This object performs a depth-first traversal of the
//               scene graph, with optional view-frustum culling,
//               collecting CullState and searching for GeomNodes.
//               Each renderable Geom encountered is passed along with
//               its associated RenderState to the CullHandler object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PortalClipper : public TypedObject {
public:
  PortalClipper(GeometricBoundingVolume *frustum, SceneSetup *scene_setup);
  ~PortalClipper();

  INLINE bool is_facing_camera();
  void prepare_portal(int idx);

  void clip_portal(int idx);

  PT(BoundingVolume) get_reduced_frustum(int idx);

  void draw_lines();
  INLINE void draw_camera_frustum();
  void draw_hexahedron(BoundingHexahedron *frustum);

  INLINE void move_to(float x, float y, float z);
  void move_to(const LVecBase3f &v);

  INLINE void draw_to(float x, float y, float z);
  void draw_to(const LVecBase3f &v);


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
  float _thick;

  PTA_Vertexf _created_verts;
  PTA_Colorf _created_colors;

  PT(GeomLine) _geom_line;
  PT(GeomPoint) _geom_point;
  PT(GeomLinestrip) _geom_linestrip;

  BoundingHexahedron *_hex_frustum;
  SceneSetup *_scene_setup;

  int _num_vert;
  Vertexf _coords[4];

public:
  PT(GeomNode) _previous;
};

#include "portalClipper.I"

#endif


  
