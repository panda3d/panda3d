/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggPrimitive.h
 * @author drose
 * @date 1999-01-16
 */

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
#include "vector_PT_EggTexture.h"

#include "pointerTo.h"
#include "pvector.h"

#include <algorithm>

class EggVertexPool;

/**
 * A base class for any of a number of kinds of geometry primitives: polygons,
 * point lights, nurbs patches, parametrics curves, etc.  Things with a set of
 * vertices and some rendering properties like color.
 *
 * An EggPrimitive is an STL-style container of pointers to EggVertex's.  In
 * fact, it IS a vector, and can be manipulated in all the ways that vectors
 * can.  However, it is necessary that all vertices belong to the same vertex
 * pool.
 */
class EXPCL_PANDA_EGG EggPrimitive : public EggNode, public EggAttributes,
                     public EggRenderMode
{

  // This is a bit of private interface stuff that must be here as a forward
  // reference.  This allows us to define the EggPrimitive as an STL
  // container.

private:
  typedef vector_PT_EggVertex Vertices;

  // Here begins the actual public interface to EggPrimitive.

PUBLISHED:
  enum Shading {
    // The order here is important.  The later choices are more specific than
    // the earlier ones.
    S_unknown,
    S_overall,
    S_per_face,
    S_per_vertex
  };

  INLINE explicit EggPrimitive(const std::string &name = "");
  INLINE EggPrimitive(const EggPrimitive &copy);
  INLINE EggPrimitive &operator = (const EggPrimitive &copy);
  INLINE ~EggPrimitive();

  virtual EggPrimitive *make_copy() const=0;

  virtual EggRenderMode *determine_alpha_mode();
  virtual EggRenderMode *determine_depth_write_mode();
  virtual EggRenderMode *determine_depth_test_mode();
  virtual EggRenderMode *determine_visibility_mode();
  virtual EggRenderMode *determine_depth_offset();
  virtual EggRenderMode *determine_draw_order();
  virtual EggRenderMode *determine_bin();

  INLINE std::string get_sort_name() const;

  virtual Shading get_shading() const;
  INLINE void clear_connected_shading();
  INLINE Shading get_connected_shading() const;

  INLINE void set_texture(EggTexture *texture);
  INLINE bool has_texture() const;
  INLINE bool has_texture(EggTexture *texture) const;
  INLINE EggTexture *get_texture() const;

  INLINE void add_texture(EggTexture *texture);
  INLINE void clear_texture();
  INLINE int get_num_textures() const;
  INLINE EggTexture *get_texture(int n) const;
  MAKE_SEQ(get_textures, get_num_textures, get_texture);

  INLINE void set_material(EggMaterial *material);
  INLINE void clear_material();
  INLINE EggMaterial *get_material() const;
  INLINE bool has_material() const;

  INLINE void set_bface_flag(bool flag);
  INLINE bool get_bface_flag() const;

  MAKE_PROPERTY(sort_name, get_sort_name);
  MAKE_PROPERTY(shading, get_shading);
  MAKE_PROPERTY(connected_shading, get_connected_shading);

  MAKE_SEQ_PROPERTY(textures, get_num_textures, get_texture);
  MAKE_PROPERTY2(material, has_material, get_material, set_material, clear_material);
  MAKE_PROPERTY(bface_flag, get_bface_flag, set_bface_flag);

  void copy_attributes(const EggAttributes &other);
  void copy_attributes(const EggPrimitive &other);

  bool has_vertex_normal() const;
  bool has_vertex_color() const;

  virtual void unify_attributes(Shading shading);
  virtual void apply_last_attribute();
  virtual void apply_first_attribute();
  virtual void post_apply_flat_attribute();
  virtual void reverse_vertex_ordering();
  virtual bool cleanup();

  void remove_doubled_verts(bool closed);
  void remove_nonunique_verts();
  virtual bool has_primitives() const;
  virtual bool joint_has_primitives() const;
  virtual bool has_normals() const;


  // The EggPrimitive itself appears to be an STL container of pointers to
  // EggVertex objects.  The set of vertices is read-only, however, except
  // through the limited add_vertexremove_vertex or inserterase interface.
  // The following implements this.
public:
#if defined(WIN32_VC) || defined(WIN64_VC)
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
  iterator find(EggVertex *vertex);

PUBLISHED:
  INLINE void clear();

  EggVertex *add_vertex(EggVertex *vertex);
  EggVertex *remove_vertex(EggVertex *vertex);
  void remove_vertex(size_t index);
  void copy_vertices(const EggPrimitive &other);

  // These are shorthands if you don't want to use the iterators.
  INLINE size_t get_num_vertices() const;
  INLINE EggVertex *get_vertex(size_t index) const;
  INLINE void set_vertex(size_t index, EggVertex *vertex);
  INLINE void insert_vertex(size_t index, EggVertex *vertex);
  MAKE_SEQ(get_vertices, get_num_vertices, get_vertex);

  INLINE EggVertexPool *get_pool() const;

  MAKE_SEQ_PROPERTY(vertices, get_num_vertices, get_vertex, set_vertex, remove_vertex, insert_vertex);
  MAKE_PROPERTY(pool, get_pool);

  virtual void write(std::ostream &out, int indent_level) const=0;

#ifdef _DEBUG
  void test_vref_integrity() const;
#else
  void test_vref_integrity() const { }
#endif  // _DEBUG

protected:
  Vertices _vertices;

  // Don't try to use these private functions.  User code should add and
  // remove vertices via add_vertex()remove_vertex(), or via the STL-like
  // push_back()pop_back() or insert()erase(), above.
  virtual void prepare_add_vertex(EggVertex *vertex, int i, int n);
  virtual void prepare_remove_vertex(EggVertex *vertex, int i, int n);

protected:
  void write_body(std::ostream &out, int indent_level) const;

  virtual bool egg_start_parse_body();
  virtual void r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
                           CoordinateSystem to_cs);
  virtual void r_flatten_transforms();
  virtual void r_apply_texmats(EggTextureCollection &textures);

  void do_apply_flat_attribute(int vertex_index, EggAttributes *attrib);

private:
  void set_connected_shading(Shading shading, const EggAttributes *neighbor);

  class ConnectedShadingNode {
  public:
    Shading _shading;
    const EggAttributes *_neighbor;
  };
  typedef pvector<ConnectedShadingNode> ConnectedShadingNodes;

  void r_set_connected_shading(int depth_count,
                               Shading shading, const EggAttributes *neighbor,
                               ConnectedShadingNodes &connected_nodes);

private:
  typedef vector_PT_EggTexture Textures;
  Textures _textures;
  PT_EggMaterial _material;
  bool _bface;
  Shading _connected_shading;

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

  friend class EggTextureCollection;
};

#include "eggPrimitive.I"

#endif
