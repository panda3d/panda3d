// Filename: eggVertexPool.h
// Created by:  drose (16Jan99)
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

#ifndef EGGVERTEXPOOL_H
#define EGGVERTEXPOOL_H

#include "pandabase.h"

#include "eggVertex.h"
#include "eggNode.h"
#include "pt_EggVertex.h"

#include "pointerTo.h"
#include "pset.h"
#include "pmap.h"
#include "lmatrix.h"
#include "iterator_types.h"

////////////////////////////////////////////////////////////////////
//       Class : EggVertexPool
// Description : A collection of vertices.  There may be any number of
//               vertex pools in a single egg structure.  The vertices
//               in a single pool need not necessarily have any
//               connection to each other, but it is necessary that
//               any one primitive (e.g. a polygon) must pull all its
//               vertices from the same pool.
//
//               An EggVertexPool is an STL-style container of
//               pointers to EggVertex's.  Functions add_vertex() and
//               remove_vertex() are provided to manipulate the list.
//               The list may also be operated on (read-only) via
//               iterators and begin()/end().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggVertexPool : public EggNode {

  // This is a bit of private interface stuff that must be here as a
  // forward reference.  This allows us to define the EggVertexPool as
  // an STL container.

private:
  // IndexVertices is the main storage mechanism of the vertex pool.
  // It stores a reference-counting pointer to each vertex, ordered by
  // vertex index number.
  typedef pmap<int, PT_EggVertex> IndexVertices;

  // UniqueVertices is an auxiliary indexing mechanism.  It stores the
  // same vertex pointers as IndexVertices (although these pointers
  // are not reference-counted), this time ordered by vertex
  // properties.  This makes it easy to determine when one or more
  // vertices already exist in the pool with identical properties.
  typedef pmultiset<EggVertex *, UniqueEggVertices> UniqueVertices;

public:
  typedef second_of_pair_iterator<IndexVertices::const_iterator> iterator;
  typedef iterator const_iterator;
  typedef IndexVertices::size_type size_type;

  // Here begins the actual public interface to EggVertexPool.

PUBLISHED:
  EggVertexPool(const string &name);
  EggVertexPool(const EggVertexPool &copy);
  ~EggVertexPool();

  // Returns NULL if there is no such vertex.
  EggVertex *get_vertex(int index) const;
  INLINE EggVertex *operator [](int index) const;

  // Returns 0 if the pool is empty.
  int get_highest_index() const;

public:
  // Can be used to traverse all the vertices in index number order.
  iterator begin() const;
  iterator end() const;
  bool empty() const;
  size_type size() const;

PUBLISHED:
  // add_vertex() adds a freshly-allocated vertex.  It is up to the
  // user to allocate the vertex.
  void add_vertex(EggVertex *vertex, int index = -1);

  // make_new_vertex() allocates and returns a new vertex from the
  // pool.
  INLINE EggVertex *make_new_vertex();
  INLINE EggVertex *make_new_vertex(double pos);
  INLINE EggVertex *make_new_vertex(const LPoint2d &pos);
  INLINE EggVertex *make_new_vertex(const LPoint3d &pos);
  INLINE EggVertex *make_new_vertex(const LPoint4d &pos);

  // create_unique_vertex() creates a new vertex if there is not
  // already one identical to the indicated vertex, or returns the
  // existing one if there is.
  EggVertex *create_unique_vertex(const EggVertex &copy);

  void remove_vertex(EggVertex *vertex);
  int remove_unused_vertices();

  void transform(const LMatrix4d &mat);

  void write(ostream &out, int indent_level) const;

protected:
  virtual void r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
                           CoordinateSystem to_cs);
  virtual void r_transform_vertices(const LMatrix4d &mat);

private:
  UniqueVertices _unique_vertices;
  IndexVertices _index_vertices;


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

#include "eggVertexPool.I"

#endif
