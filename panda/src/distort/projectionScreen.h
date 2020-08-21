/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file projectionScreen.h
 * @author drose
 * @date 2001-12-11
 */

#ifndef PROJECTIONSCREEN_H
#define PROJECTIONSCREEN_H

#include "pandabase.h"

#include "pandaNode.h"
#include "lensNode.h"
#include "geomNode.h"
#include "nodePath.h"
#include "internalName.h"
#include "pointerTo.h"
#include "pfmFile.h"

class Geom;
class WorkingNodePath;

/**
 * A ProjectionScreen implements a simple system for projective texturing.
 * The ProjectionScreen node is the parent of a hierarchy of geometry that is
 * considered a "screen"; the ProjectionScreen will automatically recompute
 * all the UV's (for a particular texture stage) on its subordinate geometry
 * according to the relative position and lens parameters of the indicated
 * LensNode.
 *
 * All this does is recompute UV's; the caller is responsible for applying the
 * appropriate texture(s) to the geometry.
 *
 * This does not take advantage of any hardware-assisted projective texturing;
 * all of the UV's are computed in the CPU.  (Use NodePath::project_texture()
 * to enable hardware-assisted projective texturing.)  However, the
 * ProjectionScreen interface does support any kind of lens, linear or
 * nonlinear, that might be defined using the Lens interface, including
 * fisheye and cylindrical lenses.
 */
class EXPCL_PANDAFX ProjectionScreen : public PandaNode {
PUBLISHED:
  explicit ProjectionScreen(const std::string &name = "");
  virtual ~ProjectionScreen();

protected:
  ProjectionScreen(const ProjectionScreen &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

PUBLISHED:
  void set_projector(const NodePath &projector);
  INLINE const NodePath &get_projector() const;

  INLINE void clear_undist_lut();
  INLINE void set_undist_lut(const PfmFile &undist_lut);
  INLINE bool has_undist_lut() const;
  INLINE const PfmFile &get_undist_lut() const;

  PT(GeomNode) generate_screen(const NodePath &projector,
                               const std::string &screen_name,
                               int num_x_verts, int num_y_verts,
                               PN_stdfloat distance, PN_stdfloat fill_ratio);
  void regenerate_screen(const NodePath &projector, const std::string &screen_name,
                         int num_x_verts, int num_y_verts, PN_stdfloat distance,
                         PN_stdfloat fill_ratio);
  PT(PandaNode) make_flat_mesh(const NodePath &this_np, const NodePath &camera);

  INLINE void set_texcoord_name(const std::string &texcoord_name);
  INLINE std::string get_texcoord_name() const;

  INLINE void set_invert_uvs(bool invert_uvs);
  INLINE bool get_invert_uvs() const;

  INLINE void set_texcoord_3d(bool texcoord_3d);
  INLINE bool get_texcoord_3d() const;

  INLINE void set_vignette_on(bool vignette_on);
  INLINE bool get_vignette_on() const;

  INLINE void set_vignette_color(const LColor &vignette_color);
  INLINE const LColor &get_vignette_color() const;
  INLINE void set_frame_color(const LColor &frame_color);
  INLINE const LColor &get_frame_color() const;

  INLINE void set_auto_recompute(bool auto_recompute);
  INLINE bool get_auto_recompute() const;

  void recompute();
  INLINE const UpdateSeq &get_last_screen() const;
  bool recompute_if_stale();
  bool recompute_if_stale(const NodePath &this_np);

private:
  void do_recompute(const NodePath &this_np);
  void recompute_node(const WorkingNodePath &np, LMatrix4 &rel_mat, bool &computed_rel_mat);
  void recompute_child(const WorkingNodePath &np, LMatrix4 &rel_mat, bool &computed_rel_mat);
  void recompute_geom_node(const WorkingNodePath &np, LMatrix4 &rel_mat, bool &computed_rel_mat);
  void recompute_geom(Geom *geom, const LMatrix4 &rel_mat);

  PandaNode *
  make_mesh_node(PandaNode *result_parent, const WorkingNodePath &np,
                 const NodePath &camera,
                 LMatrix4 &rel_mat, bool &computed_rel_mat);
  void make_mesh_children(PandaNode *new_node, const WorkingNodePath &np,
                          const NodePath &camera,
                          LMatrix4 &rel_mat, bool &computed_rel_mat);
  PT(GeomNode) make_mesh_geom_node(const WorkingNodePath &np,
                                   const NodePath &camera,
                                   LMatrix4 &rel_mat,
                                   bool &computed_rel_mat);
  PT(Geom) make_mesh_geom(const Geom *geom, Lens *lens, LMatrix4 &rel_mat);


  NodePath _projector;
  PT(LensNode) _projector_node;
  bool _has_undist_lut;
  PfmFile _undist_lut;
  PT(InternalName) _texcoord_name;
  bool _invert_uvs;
  bool _texcoord_3d;
  bool _vignette_on;
  LColor _vignette_color;
  LColor _frame_color;

  LMatrix4 _rel_top_mat;
  bool _computed_rel_top_mat;
  bool _stale;
  UpdateSeq _projector_lens_change;
  UpdateSeq _last_screen;
  bool _auto_recompute;

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
