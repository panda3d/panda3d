// Filename: eggPrimitive.h
// Created by:  drose (16Jan99)
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

#ifndef EGGPRIMITIVE_H
#define EGGPRIMITIVE_H

#include "pandabase.h"

#include "eggNode.h"
#include "eggAttributes.h"
#include "eggVertex.h"
#include "eggTexture.h"
#include "eggMaterial.h"
#include "eggRenderMode.h"
#include "pt_EggTexture.h"
#include "pt_EggMaterial.h"
#include "vector_PT_EggVertex.h"

#include "pointerTo.h"
#include "pvector.h"

#include <algorithm>

class EggVertexPool;

////////////////////////////////////////////////////////////////////
//       Class : EggPrimitive
// Description : A base class for any of a number of kinds of geometry
//               primitives: polygons, point lights, nurbs patches,
//               parametrics curves, etc.  Things with a set of
//               vertices and some rendering properties like color.
//
//               An EggPrimitive is an STL-style container of pointers
//               to EggVertex's.  In fact, it IS a vector, and can be
//               manipulated in all the ways that vectors can.
//               However, it is necessary that all vertices belong to
//               the same vertex pool.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggPrimitive : public EggNode, public EggAttributes,
                     public EggRenderMode
{

  // This is a bit of private interface stuff that must be here as a
  // forward reference.  This allows us to define the EggPrimitive as
  // an STL container.

private:
  typedef vector_PT_EggVertex Vertices;

  // Here begins the actual public interface to EggPrimitive.

PUBLISHED:
  INLINE EggPrimitive(const string &name = "");
  INLINE EggPrimitive(const EggPrimitive &copy);
  INLINE EggPrimitive &operator = (const EggPrimitive &copy);
  INLINE ~EggPrimitive();

  virtual EggRenderMode *determine_alpha_mode();
  virtual EggRenderMode *determine_depth_write_mode();
  virtual EggRenderMode *determine_depth_test_mode();
  virtual EggRenderMode *determine_visibility_mode();
  virtual EggRenderMode *determine_draw_order();
  virtual EggRenderMode *determine_bin();

  INLINE void set_texture(EggTexture *texture);
  INLINE void clear_texture();
  INLINE EggTexture *get_texture() const;
  INLINE bool has_texture() const;

  INLINE void set_material(EggMaterial *material);
  INLINE void clear_material();
  INLINE EggMaterial *get_material() const;
  INLINE bool has_material() const;

  INLINE void set_bface_flag(bool flag);
  INLINE bool get_bface_flag() const;

  void copy_attributes(const EggPrimitive &other);

  bool has_vertex_normal() const;
  bool has_vertex_color() const;

  virtual void reverse_vertex_ordering();
  virtual bool cleanup();

  void remove_doubled_verts(bool closed);
  void remove_nonunique_verts();
  virtual bool has_primitives() const;
  virtual bool joint_has_primitives() const;


  // The EggPrimitive itself appears to be an STL container of
  // pointers to EggVertex objects.  The set of vertices is read-only,
  // however, except through the limited add_vertex/remove_vertex or
  // insert/erase interface.  The following implements this.
public:
#ifdef WIN32_VC
  typedef PT_EggVertex *pointer;
  typedef PT_EggVertex *const_pointer;
#else
  typedef Vertices::const_pointer pointer;
  typedef Vertices::const_pointer const_pointer;
#endif
  typedef Vertices::const_reference reference;
  typedef Vertices::const_reference const_reference;
  typedef Vertices::const_iterator iterator;
  typedef Vertices::const_iterator const_iterator;
  typedef Vertices::const_reverse_iterator reverse_iterator;
  typedef Vertices::const_reverse_iterator const_reverse_iterator;
  typedef Vertices::size_type size_type;
  typedef Vertices::difference_type difference_type;

  INLINE iterator begin() const;
  INLINE iterator end() const;
  INLINE reverse_iterator rbegin() const;
  INLINE reverse_iterator rend() const;
  INLINE bool empty() const;
  INLINE size_type size() const;

  INLINE EggVertex *operator [] (int index) const;

  INLINE iterator insert(iterator position, EggVertex *x);
  INLINE iterator erase(iterator position);
  iterator erase(iterator first, iterator last);
  INLINE void replace(iterator position, EggVertex *vertex);

PUBLISHED:
  INLINE void clear();

  EggVertex *add_vertex(EggVertex *vertex);
  EggVertex *remove_vertex(EggVertex *vertex);
  void copy_vertices(const EggPrimitive &other);

  // These are shorthands if you don't want to use the iterators.
  INLINE int get_num_vertices() const;
  INLINE void set_vertex(int index, EggVertex *vertex);
  INLINE EggVertex *get_vertex(int index) const;

  INLINE EggVertexPool *get_pool() const;

#ifndef NDEBUG
  void test_vref_integrity() const;
#else
  void test_vref_integrity() const { }
#endif  // NDEBUG

private:
  Vertices _vertices;

  // Don't try to use these private functions.  User code should add
  // and remove vertices via add_vertex()/remove_vertex(), or via the
  // STL-like push_back()/pop_back() or insert()/erase(), above.
  void prepare_add_vertex(EggVertex *vertex);
  void prepare_remove_vertex(EggVertex *vertex);


protected:
  void write_body(ostream &out, int indent_level) const;

  virtual bool egg_start_parse_body();
  virtual void r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
                           CoordinateSystem to_cs);
  virtual void r_flatten_transforms();
  virtual void r_apply_texmats(EggTextureCollection &textures);


private:
  PT_EggTexture _texture;
  PT_EggMaterial _material;
  bool _bface;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNode::init_type();
    EggAttributes::init_type();
    EggRenderMode::get_class_type();
    register_type(_type_handle, "EggPrimitive",
                  EggNode::get_class_type(),
                  EggAttributes::get_class_type(),
                  EggRenderMode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggPrimitive.I"

#endif
