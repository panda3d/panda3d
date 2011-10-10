// Filename: meshDrawer.h
// Created by:  treeform (19dec08)
// Changes by:  treeform (12jan10)
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

#ifndef MESHDRAWER_H
#define MESHDRAWER_H

#include "pandabase.h"
#include "luse.h"
#include "pandaNode.h"
#include "pointerTo.h"
#include "lpoint2.h"
#include "lvecBase2.h"
#include "pnmImage.h"
#include "nodePath.h"
#include "texture.h"
#include "geomVertexFormat.h"
#include "geomVertexArrayFormat.h"
#include "geomVertexData.h"
#include "geomVertexWriter.h"
#include "geomVertexRewriter.h"
#include "boundingVolume.h"

#include "nodePathCollection.h"

#include "geomTristrips.h"
#include "geomTriangles.h"
#include "geom.h"
#include "geomNode.h"
#include "nodePath.h"

////////////////////////////////////////////////////////////////////
//       Class : MeshDrawer
// Description : Mesh drawer creates a single geom object that can be
//               shaped with different draw commands.  This is an 
//               efficient way to render bunch of billboards, particles, 
//               fast changing triangles.  Its implemented by recycling 
//               same geom over and over again.  Max budget specifies 
//               how many triangles are allowed.  Some uses of this 
//               class can be : particle system, radar icons, health 
//               bars, 2d icons, 2d ui, bullets, missile trails.  Any 
//               that can be drawn with triangles can be drawn with 
//               this class.  At the low level this uses the 
//               GeomVertexRewriter's.  The internal geom consists of 
//               vertex, normal, uv and color channels.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GRUTIL MeshDrawer : public TypedObject {
PUBLISHED:
  INLINE MeshDrawer();
  INLINE ~MeshDrawer();

  INLINE void set_budget(int budget);
  INLINE int get_budget();

  INLINE NodePath get_root();

  void begin(NodePath camera, NodePath render);
  INLINE void tri(LVector3 v1, LVector4 c1, LVector2 uv1,
                  LVector3 v2, LVector4 c2, LVector2 uv2,
                  LVector3 v3, LVector4 c3, LVector2 uv3);
                  
  void particle(LVector3 pos, LVector4 frame, PN_stdfloat size, LVector4 color, PN_stdfloat rotation);
  void blended_particle(LVector3 pos, LVector4 frame1, LVector4 frame2,
    PN_stdfloat blend, PN_stdfloat size, LVector4 color, PN_stdfloat rotation);
  void billboard(LVector3 pos, LVector4 frame, PN_stdfloat size, LVector4 color);
  void segment(LVector3 start, LVector3 stop, LVector4 frame, PN_stdfloat thickness, LVector4 color);
  void cross_segment(LVector3 start, LVector3 stop, LVector4 frame, PN_stdfloat thickness, LVector4 color);
  void uneven_segment(LVector3 start, LVector3 stop,
    LVector4 frame, PN_stdfloat thickness_start, LVector4 color_start,
    PN_stdfloat thickness_stop, LVector4 color_stop);

  void link_segment(LVector3 pos, LVector4 frame, PN_stdfloat thickness, LVector4 color);
  void link_segment_end(LVector4 frame, LVector4 color);

  void explosion(LVector3 pos, LVector4 frame, PN_stdfloat size, LVector4 color,
    int seed, int number, PN_stdfloat distance);
  void stream(LVector3 start, LVector3 stop, LVector4 frame, PN_stdfloat size, LVector4 color,
    int number, PN_stdfloat offset);
  void geometry(NodePath node);
  void end();

private:

  // use vars
  NodePath _root;
  NodePath _camera, _render;
  int _budget;

  // store regeneration geoms & nodes
  PT(Geom) _geom;
  PT(GeomNode) _geomnode;
  PT(GeomVertexData) _vdata;
  PT(GeomTriangles) _prim;
  CPT(GeomPrimitive) _dprim;

  // writers
  GeomVertexRewriter *_vertex;
  GeomVertexRewriter *_normal;
  GeomVertexRewriter *_uv;
  GeomVertexRewriter *_color;

  // billboard vectors
  LVector4 _colorv;
  LVector3 _normalv;
  LVector3 _eyePos;
  LVector3 _b1, _b2, _b3, _b4;
  LVector3 _up, _right;

  // clear indexes
  int _last_clear_index, _start_clear_index, _end_clear_index, _clear_index;

  // used for curves
  int _at_start;
  LVector3 _last_v1,_last_v2,_last_v3,_last_v4,_last_pos;
  PN_stdfloat _last_thickness;
  LVector4 _last_color;

  // bounding volume
  PT(BoundingVolume) _bv;

  // private create all the needed geoms
  void generator(int budget);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "MeshDrawer",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "meshDrawer.I"

#endif /*MESHDRAWER_H*/
