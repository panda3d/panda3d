// Filename: projectionScreen.h
// Created by:  drose (11Dec01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef PROJECTIONSCREEN_H
#define PROJECTIONSCREEN_H

#include "pandabase.h"

#include "namedNode.h"
#include "lensNode.h"

class GeomNode;
class Geom;

////////////////////////////////////////////////////////////////////
//       Class : ProjectionScreen
// Description : A ProjectionScreen implements a simple system for
//               projective texturing.  The ProjectionScreen node is
//               the parent of a hierarchy of geometry that is
//               considered a "screen"; the ProjectionScreen will
//               automatically recompute all the UV's on its
//               subordinate geometry according to the relative
//               position and lens parameters of the indicated
//               LensNode.
//
//               This does not take advantage of any hardware-assisted
//               projective texturing; nor does it presently support
//               multitexturing.  However, it does support any kind of
//               lens, linear or nonlinear, that might be defined
//               using the Lens interface, including fisheye and
//               cylindrical lenses.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAFX ProjectionScreen : public NamedNode {
PUBLISHED:
  ProjectionScreen(const string &name = "");
  virtual ~ProjectionScreen();

public:
  ProjectionScreen(const ProjectionScreen &copy);
  void operator = (const ProjectionScreen &copy);

  virtual Node *make_copy() const;
  virtual void app_traverse(const ArcChain &chain);

PUBLISHED:
  INLINE void set_projector(LensNode *projector);
  INLINE LensNode *get_projector() const;

  PT(GeomNode) generate_screen(LensNode *projector, const string &screen_name,
                               int num_x_verts, int num_y_verts, float distance);
  void regenerate_screen(LensNode *projector, const string &screen_name,
                         int num_x_verts, int num_y_verts, float distance);
  PT_Node make_flat_mesh(LensNode *camera);

  INLINE void set_vignette_on(bool vignette_on);
  INLINE bool get_vignette_on() const;

  INLINE void set_vignette_color(const Colorf &vignette_color);
  INLINE const Colorf &get_vignette_color() const;
  INLINE void set_frame_color(const Colorf &frame_color);
  INLINE const Colorf &get_frame_color() const;

  void recompute();

private:
  void recompute_if_stale();
  void recompute_node(Node *node, LMatrix4f &rel_mat, bool &computed_rel_mat);
  void recompute_geom_node(GeomNode *node, LMatrix4f &rel_mat, bool &computed_rel_mat);
  void recompute_geom(Geom *geom, const LMatrix4f &rel_mat);

  NodeRelation *
  make_mesh_node(Node *result_parent, Node *node, LensNode *camera,
                 LMatrix4f &rel_mat, bool &computed_rel_mat);
  void make_mesh_children(Node *new_node, Node *node, LensNode *camera,
                          LMatrix4f &rel_mat, bool &computed_rel_mat);
  PT(GeomNode) make_mesh_geom_node(GeomNode *node, LensNode *camera,
                                   LMatrix4f &rel_mat, bool &computed_rel_mat);
  PT(dDrawable) make_mesh_geom(Geom *geom, Lens *lens, LMatrix4f &rel_mat);


  PT(LensNode) _projector;
  bool _vignette_on;
  Colorf _vignette_color;
  Colorf _frame_color;

  PTA_Colorf _colors;
  LMatrix4f _rel_top_mat;
  bool _computed_rel_top_mat;
  bool _stale;
  UpdateSeq _projector_lens_change;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NamedNode::init_type();
    register_type(_type_handle, "ProjectionScreen",
                  NamedNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "projectionScreen.I"

#endif
