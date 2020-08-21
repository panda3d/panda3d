/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggNode.h
 * @author drose
 * @date 1999-01-16
 */

#ifndef EGGNODE_H
#define EGGNODE_H

#include "pandabase.h"

#include "eggNamedObject.h"

#include "typedObject.h"
#include "lmatrix.h"
#include "pointerTo.h"
#include "referenceCount.h"

class EggGroupNode;
class EggRenderMode;
class EggTextureCollection;

/**
 * A base class for things that may be directly added into the egg hierarchy.
 * This includes groups, joints, polygons, vertex pools, etc., but does not
 * include things like vertices.
 */
class EXPCL_PANDA_EGG EggNode : public EggNamedObject {
PUBLISHED:
  INLINE explicit EggNode(const std::string &name = "");
  INLINE EggNode(const EggNode &copy);
  INLINE EggNode &operator = (const EggNode &copy);

  INLINE EggGroupNode *get_parent() const;
  INLINE int get_depth() const;
  INLINE bool is_under_instance() const;
  INLINE bool is_under_transform() const;
  INLINE bool is_local_coord() const;

  MAKE_PROPERTY(parent, get_parent);
  MAKE_PROPERTY(depth, get_depth);

  INLINE const LMatrix4d &get_vertex_frame() const;
  INLINE const LMatrix4d &get_node_frame() const;
  INLINE const LMatrix4d &get_vertex_frame_inv() const;
  INLINE const LMatrix4d &get_node_frame_inv() const;
  INLINE const LMatrix4d &get_vertex_to_node() const;
  INLINE const LMatrix4d &get_node_to_vertex() const;

  INLINE const LMatrix4d *get_vertex_frame_ptr() const;
  INLINE const LMatrix4d *get_node_frame_ptr() const;
  INLINE const LMatrix4d *get_vertex_frame_inv_ptr() const;
  INLINE const LMatrix4d *get_node_frame_inv_ptr() const;
  INLINE const LMatrix4d *get_vertex_to_node_ptr() const;
  INLINE const LMatrix4d *get_node_to_vertex_ptr() const;

  INLINE void transform(const LMatrix4d &mat);
  INLINE void transform_vertices_only(const LMatrix4d &mat);
  INLINE void flatten_transforms();
  void apply_texmats();

  int rename_node(vector_string strip_prefix);

  virtual bool is_joint() const;
  virtual bool is_anim_matrix() const;

  virtual EggRenderMode *determine_alpha_mode();
  virtual EggRenderMode *determine_depth_write_mode();
  virtual EggRenderMode *determine_depth_test_mode();
  virtual EggRenderMode *determine_visibility_mode();
  virtual EggRenderMode *determine_depth_offset();
  virtual EggRenderMode *determine_draw_order();
  virtual EggRenderMode *determine_bin();
  virtual bool determine_indexed();
  virtual bool determine_decal();

  virtual void write(std::ostream &out, int indent_level) const=0;
  bool parse_egg(const std::string &egg_syntax);

#ifdef _DEBUG
  void test_under_integrity() const;
#else
  void test_under_integrity() const { }
#endif  // _DEBUG


protected:
  enum UnderFlags {
    UF_under_instance  = 0x001,
    UF_under_transform = 0x002,
    UF_local_coord     = 0x004,
  };

  virtual bool egg_start_parse_body();

  virtual void update_under(int depth_offset);
  virtual void adjust_under();
  virtual bool has_primitives() const;
  virtual bool joint_has_primitives() const;
  virtual bool has_normals() const;

  virtual void r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
                           CoordinateSystem to_cs);
  virtual void r_transform_vertices(const LMatrix4d &mat);
  virtual void r_mark_coordsys(CoordinateSystem cs);
  virtual void r_flatten_transforms();
  virtual void r_apply_texmats(EggTextureCollection &textures);

  // These members are updated automatically by prepare_add_child(),
  // prepare_remove_child(), and update_under().  Other functions shouldn't be
  // fiddling with them.

  EggGroupNode *_parent;
  int _depth;
  int _under_flags;

  typedef RefCountObj<LMatrix4d> MatrixFrame;

  PT(MatrixFrame) _vertex_frame;
  PT(MatrixFrame) _node_frame;
  PT(MatrixFrame) _vertex_frame_inv;
  PT(MatrixFrame) _node_frame_inv;
  PT(MatrixFrame) _vertex_to_node;
  PT(MatrixFrame) _node_to_vertex;


public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNamedObject::init_type();
    register_type(_type_handle, "EggNode",
                  EggNamedObject::get_class_type());
    MatrixFrame::init_type();
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class EggGroupNode;
  friend class EggTable;
};

#include "eggNode.I"

#endif
