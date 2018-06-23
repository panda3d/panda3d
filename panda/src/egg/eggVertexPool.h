/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggVertexPool.h
 * @author drose
 * @date 1999-01-16
 */

#ifndef EGGVERTEXPOOL_H
#define EGGVERTEXPOOL_H

#include "pandabase.h"

#include "eggVertex.h"
#include "eggNode.h"
#include "pt_EggVertex.h"

#include "pointerTo.h"
#include "pset.h"
#include "pvector.h"
#include "pmap.h"
#include "lmatrix.h"
#include "iterator_types.h"

/**
 * A collection of vertices.  There may be any number of vertex pools in a
 * single egg structure.  The vertices in a single pool need not necessarily
 * have any connection to each other, but it is necessary that any one
 * primitive (e.g.  a polygon) must pull all its vertices from the same pool.
 *
 * An EggVertexPool is an STL-style container of pointers to EggVertex's.
 * Functions add_vertex() and remove_vertex() are provided to manipulate the
 * list.  The list may also be operated on (read-only) via iterators and
 * begin()/end().
 */
class EXPCL_PANDA_EGG EggVertexPool : public EggNode {

  // This is a bit of private interface stuff that must be here as a forward
  // reference.  This allows us to define the EggVertexPool as an STL
  // container.

private:
  // IndexVertices is the main storage mechanism of the vertex pool.  It
  // stores a reference-counting pointer to each vertex, ordered by vertex
  // index number.
  typedef pmap<int, PT_EggVertex> IndexVertices;

  // UniqueVertices is an auxiliary indexing mechanism.  It stores the same
  // vertex pointers as IndexVertices (although these pointers are not
  // reference-counted), this time ordered by vertex properties.  This makes
  // it easy to determine when one or more vertices already exist in the pool
  // with identical properties.
  typedef pmultiset<EggVertex *, UniqueEggVertices> UniqueVertices;

public:
  typedef second_of_pair_iterator<IndexVertices::const_iterator> iterator;
  typedef iterator const_iterator;
  typedef IndexVertices::size_type size_type;

  // Here begins the actual public interface to EggVertexPool.

PUBLISHED:
  explicit EggVertexPool(const std::string &name);
  EggVertexPool(const EggVertexPool &copy);
  ~EggVertexPool();

  INLINE bool has_vertex(int index) const;

  bool has_forward_vertices() const;
  bool has_defined_vertices() const;

  // Returns NULL if there is no such vertex.
  EggVertex *get_vertex(int index) const;
  INLINE EggVertex *operator [](int index) const;

  // Returns a forward reference if there is no such vertex.
  EggVertex *get_forward_vertex(int index);

  // Returns 0 if the pool is empty.
  int get_highest_index() const;
  void set_highest_index(int highest_index);

  int get_num_dimensions() const;
  bool has_normals() const;
  bool has_colors() const;
  bool has_nonwhite_colors() const;
  void check_overall_color(bool &has_overall_color, LColor &overall_color) const;
  bool has_uvs() const;
  bool has_aux() const;
  void get_uv_names(vector_string &uv_names, vector_string &uvw_names,
                    vector_string &tbn_names) const;
  void get_aux_names(vector_string &aux_names) const;

public:
  // Can be used to traverse all the vertices in index number order.
  iterator begin() const;
  iterator end() const;
  bool empty() const;

PUBLISHED:
  size_type size() const;

  // add_vertex() adds a freshly-allocated vertex.  It is up to the user to
  // allocate the vertex.
  EggVertex *add_vertex(EggVertex *vertex, int index = -1);

  // make_new_vertex() allocates and returns a new vertex from the pool.
  INLINE EggVertex *make_new_vertex();
  INLINE EggVertex *make_new_vertex(double pos);
  INLINE EggVertex *make_new_vertex(const LPoint2d &pos);
  INLINE EggVertex *make_new_vertex(const LPoint3d &pos);
  INLINE EggVertex *make_new_vertex(const LPoint4d &pos);

  // create_unique_vertex() creates a new vertex if there is not already one
  // identical to the indicated vertex, or returns the existing one if there
  // is.
  EggVertex *create_unique_vertex(const EggVertex &copy);
  EggVertex *find_matching_vertex(const EggVertex &copy);

  void remove_vertex(EggVertex *vertex);
  int remove_unused_vertices();
  void add_unused_vertices_to_prim(EggPrimitive *prim);

  void transform(const LMatrix4d &mat);
  void sort_by_external_index();

  void write(std::ostream &out, int indent_level) const;

protected:
  virtual void r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
                           CoordinateSystem to_cs);
  virtual void r_transform_vertices(const LMatrix4d &mat);

private:
  UniqueVertices _unique_vertices;
  IndexVertices _index_vertices;
  int _highest_index;


public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNode::init_type();
    register_type(_type_handle, "EggVertexPool",
                  EggNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

friend class EggVertex;
};

typedef pvector< PT(EggVertexPool) > EggVertexPools;

#include "eggVertexPool.I"

#endif
