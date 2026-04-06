/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file meshDrawer.h
 * @author treeform
 * @date 2008-12-19
 * @author treeform
 * @date 2010-01-12
 */

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

/**
 * Mesh drawer creates a single geom object that can be shaped with different
 * draw commands.  This is an efficient way to render bunch of billboards,
 * particles, fast changing triangles.  Its implemented by recycling same geom
 * over and over again.  Max budget specifies how many triangles are allowed.
 * Some uses of this class can be : particle system, radar icons, health bars,
 * 2d icons, 2d ui, bullets, missile trails.  Any that can be drawn with
 * triangles can be drawn with this class.  At the low level this uses the
 * GeomVertexRewriter's.  The internal geom consists of vertex, normal, uv and
 * color channels.
 */
class EXPCL_PANDA_GRUTIL MeshDrawer : public TypedObject {
PUBLISHED:
  INLINE MeshDrawer();
  INLINE ~MeshDrawer();

  INLINE void set_budget(int budget);
  INLINE int get_budget();

  INLINE NodePath get_root();

  void begin(NodePath camera, NodePath render);
  INLINE void tri(const LVector3 &v1, const LVector4 &c1, const LVector2 &uv1,
                  const LVector3 &v2, const LVector4 &c2, const LVector2 &uv2,
                  const LVector3 &v3, const LVector4 &c3, const LVector2 &uv3);

  void particle(const LVector3 &pos, const LVector4 &frame, PN_stdfloat size, const LVector4 &color, PN_stdfloat rotation);
  void blended_particle(const LVector3 &pos, const LVector4 &frame1, const LVector4 &frame2,
    PN_stdfloat blend, PN_stdfloat size, const LVector4 &color, PN_stdfloat rotation);
  void billboard(const LVector3 &pos, const LVector4 &frame, PN_stdfloat size, const LVector4 &color);
  void segment(const LVector3 &start, const LVector3 &stop, const LVector4 &frame, PN_stdfloat thickness, const LVector4 &color);
  void cross_segment(const LVector3 &start, const LVector3 &stop, const LVector4 &frame, PN_stdfloat thickness, const LVector4 &color);
  void uneven_segment(const LVector3 &start, const LVector3 &stop,
    const LVector4 &frame, PN_stdfloat thickness_start, const LVector4 &color_start,
    PN_stdfloat thickness_stop, const LVector4 &color_stop);

  void link_segment(const LVector3 &pos, const LVector4 &frame, PN_stdfloat thickness, const LVector4 &color);
  void link_segment_end(const LVector4 &frame, const LVector4 &color);

  void explosion(const LVector3 &pos, const LVector4 &frame, PN_stdfloat size, const LVector4 &color,
    int seed, int number, PN_stdfloat distance);
  void stream(const LVector3 &start, const LVector3 &stop, const LVector4 &frame, PN_stdfloat size, const LVector4 &color,
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
