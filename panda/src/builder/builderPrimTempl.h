// Filename: builderPrimTempl.h
// Created by:  drose (11Sep97)
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

#ifndef BUILDERPRIMTEMPL_H
#define BUILDERPRIMTEMPL_H

#include "pandabase.h"

#include "builderVertex.h"
#include "builderAttrib.h"
#include "builderTypes.h"

#include "pvector.h"


/////////////////////////////////////////////////////////////////////
//       Class : BuilderPrimTempl
// Description : The main body of BuilderPrim and BuilderPrimI.  This
//               is a template class on vertex type, which must be
//               either BuilderVertex or BuilderVertexI; these classes
//               are themselves template classes on vertex type,
//               texcoord type, color type, etc.
////////////////////////////////////////////////////////////////////
template <class VTX>
class BuilderPrimTempl : public VTX::Attrib {
public:
  typedef VTX Vertex;
  typedef TYPENAME VTX::VType VType;
  typedef TYPENAME VTX::NType NType;
  typedef TYPENAME VTX::TType TType;
  typedef TYPENAME VTX::CType CType;
  typedef TYPENAME VTX::Attrib DAttrib;

  INLINE BuilderPrimTempl();
  INLINE BuilderPrimTempl(const BuilderPrimTempl &copy);
  INLINE BuilderPrimTempl &operator = (const BuilderPrimTempl &copy);

  void remove_doubled_verts(int closed);
  bool is_valid() const;

  // has_normal() etc. is true if the primitive has a normal, as
  // assigned by the user.  This is unrelated to the vertices or
  // component primitives that may or may not also have normals.
  INLINE bool has_normal() const;
  INLINE bool has_color() const;
  INLINE bool has_pixel_size() const;

  // The following has_* functions are based on information derived
  // from examining the vertices and the component primitives that
  // make up this primitive.

  // has_overall_normal() etc. is true if the primitive has a single,
  // overall normal shared among all vertices and all component
  // primitives, or if its normal was assigned directly via
  // set_normal().  For a polygon, this is the polygon normal.  For a
  // tristrip, this means all triangles share the same normal.
  INLINE bool has_overall_normal() const;
  INLINE bool has_overall_color() const;
  INLINE bool has_overall_pixel_size() const;

  // has_vertex_normal() etc. is true if each vertex in the primitive
  // has its own normal.  It is not true if any vertex does not have a
  // normal.
  INLINE bool has_vertex_normal() const;
  INLINE bool has_vertex_color() const;
  INLINE bool has_vertex_texcoord() const;
  INLINE bool has_vertex_pixel_size() const;

  // has_component_normal() can only be true for aggregate primitive
  // types like tristrips.  In that case, it is true if each
  // individual component (e.g. each triangle of the tristrip) has its
  // own normal.
  INLINE bool has_component_normal() const;
  INLINE bool has_component_color() const;
  INLINE bool has_component_pixel_size() const;

  // In the above, only one of has_overall_normal(),
  // has_vertex_normal(), and has_component_normal() can be true for a
  // given primitive.  For convenience, the following functions return
  // true if any of the above is true:

  INLINE bool has_any_normal() const;
  INLINE bool has_any_color() const;
  INLINE bool has_any_texcoord() const;
  INLINE bool has_any_pixel_size() const;

  INLINE BuilderPrimTempl &clear();
  INLINE BuilderPrimTempl &clear_vertices();

  INLINE BuilderPrimTempl &set_attrib(const DAttrib &attrib);

  INLINE BuilderPrimType get_type() const;
  INLINE BuilderPrimTempl &set_type(BuilderPrimType t);

  INLINE NType get_normal() const;
  INLINE BuilderPrimTempl &set_normal(const NType &n);

  INLINE CType get_color() const;
  INLINE BuilderPrimTempl &set_color(const CType &c);

  INLINE float get_pixel_size() const;
  INLINE BuilderPrimTempl &set_pixel_size(float s);

  INLINE BuilderPrimTempl &add_vertex(const Vertex &v);

  INLINE int get_num_verts() const;
  INLINE Vertex &get_vertex(int n);
  INLINE const Vertex &get_vertex(int n) const;

  INLINE BuilderPrimTempl &add_component(const DAttrib &attrib);
  INLINE int get_num_components() const;
  INLINE DAttrib &get_component(int n);
  INLINE const DAttrib &get_component(int n) const;

  INLINE bool operator < (const BuilderPrimTempl &other) const;

  ostream &output(ostream &out) const;

protected:
  INLINE int sort_value() const;
  void update_overall_attrib();

  typedef pvector<Vertex> Verts;
  typedef pvector<DAttrib> Components;

  Verts _verts;
  Components _components;
  BuilderPrimType _type;
  int _overall;
};

template <class VTX>
INLINE ostream &operator << (ostream &out,
                             const BuilderPrimTempl<VTX> &prim) {
  return prim.output(out);
}


#include "builderPrimTempl.I"

#endif
