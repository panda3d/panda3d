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

#include "pandaNode.h"
#include "lensNode.h"
#include "geomNode.h"
#include "nodePath.h"

class Geom;
class WorkingNodePath;

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
class EXPCL_PANDAFX ProjectionScreen : public PandaNode {
PUBLISHED:
  ProjectionScreen(const string &name = "");
  virtual ~ProjectionScreen();

protected:
  ProjectionScreen(const ProjectionScreen &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

PUBLISHED:
  void set_projector(const NodePath &projector);
  INLINE const NodePath &get_projector() const;

  PT(GeomNode) generate_screen(const NodePath &projector,
                               const string &screen_name,
                               int num_x_verts, int num_y_verts,
                               float distance);
  void regenerate_screen(const NodePath &projector, const string &screen_name,
                         int num_x_verts, int num_y_verts, float distance);
  PT(PandaNode) make_flat_mesh(const NodePath &camera);

  INLINE void set_invert_uvs(bool invert_uvs);
  INLINE bool get_invert_uvs() const;

  INLINE void set_vignette_on(bool vignette_on);
  INLINE bool get_vignette_on() const;

  INLINE void set_vignette_color(const Colorf &vignette_color);
  INLINE const Colorf &get_vignette_color() const;
  INLINE void set_frame_color(const Colorf &frame_color);
  INLINE const Colorf &get_frame_color() const;

  void recompute();

public:
  INLINE const UpdateSeq &get_last_screen() const;
  void recompute_if_stale();

private:
  void do_recompute(const NodePath &this_np);
  void recompute_node(const WorkingNodePath &np, LMatrix4f &rel_mat, bool &computed_rel_mat);
  void recompute_child(const WorkingNodePath &np, LMatrix4f &rel_mat, bool &computed_rel_mat);
  void recompute_geom_node(const WorkingNodePath &np, LMatrix4f &rel_mat, bool &computed_rel_mat);
  void recompute_geom(Geom *geom, const LMatrix4f &rel_mat);

  PandaNode *
  make_mesh_node(PandaNode *result_parent, const WorkingNodePath &np,
                 const NodePath &camera,
                 LMatrix4f &rel_mat, bool &computed_rel_mat);
  void make_mesh_children(PandaNode *new_node, const WorkingNodePath &np,
                          const NodePath &camera,
                          LMatrix4f &rel_mat, bool &computed_rel_mat);
  PT(GeomNode) make_mesh_geom_node(const WorkingNodePath &np, 
                                   const NodePath &camera,
                                   LMatrix4f &rel_mat,
                                   bool &computed_rel_mat);
  PT(Geom) make_mesh_geom(Geom *geom, Lens *lens, LMatrix4f &rel_mat);


  NodePath _projector;
  PT(LensNode) _projector_node;
  bool _invert_uvs;
  bool _vignette_on;
  Colorf _vignette_color;
  Colorf _frame_color;

  PTA_Colorf _colors;
  LMatrix4f _rel_top_mat;
  bool _computed_rel_top_mat;
  bool _stale;
  UpdateSeq _projector_lens_change;
  UpdateSeq _last_screen;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "ProjectionScreen",
                  PandaNode::get_class_type());
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
