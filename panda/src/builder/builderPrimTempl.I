// Filename: builderPrimTempl.I
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

#include "notify.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE BuilderPrimTempl<VTX>::
BuilderPrimTempl() {
  clear();
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE BuilderPrimTempl<VTX>::
BuilderPrimTempl(const BuilderPrimTempl<VTX> &copy) {
  (*this) = copy;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE BuilderPrimTempl<VTX> &BuilderPrimTempl<VTX>::
operator = (const BuilderPrimTempl<VTX> &copy) {
  DAttrib::operator = (copy);
  _verts = copy._verts;
  _components = copy._components;
  _type = copy._type;
  _overall = copy._overall;

  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::has_normal
//       Access: Public
//  Description: Returns true if set_normal() has been called on the
//               primitive.  This is unrelated to the normal values
//               that may or may not override from the vertices.  Also
//               see has_overall_normal().
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
has_normal() const {
  return DAttrib::has_normal();
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::has_color
//       Access: Public
//  Description: Returns true if set_color() has been called on the
//               primitive.  This is unrelated to the color values
//               that may or may not override from the vertices.  Also
//               see has_overall_color().
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
has_color() const {
  return DAttrib::has_color();
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::has_pixel_size
//       Access: Public
//  Description: Returns true if set_pixel_size() has been called on
//               the primitive.  This is unrelated to the pixel_size
//               values that may or may not override from the
//               vertices.  Also see has_overall_pixel_size().
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
has_pixel_size() const {
  return DAttrib::has_pixel_size();
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::has_overall_normal
//       Access: Public
//  Description: Returns true if the primitive has a single, overall
//               normal value.  This can happen because of one of: (a)
//               the primitive had a normal value assigned to it
//               directly, and its individual vertices and components
//               did not; (b) each vertex was assigned the same normal
//               value; (c) each component was assigned the same normal
//               value.
//
//               If has_overall_normal() returns true, then get_normal()
//               will return the overall normal value.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
has_overall_normal() const {
  if ((_overall & BAF_overall_updated) == 0) {
    ((BuilderPrimTempl<VTX> *)this)->update_overall_attrib();
  }
  return (_overall & BAF_overall_normal)!=0;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::has_overall_color
//       Access: Public
//  Description: Returns true if the primitive has a single, overall
//               color value.  This can happen because of one of: (a)
//               the primitive had a color value assigned to it
//               directly, and its individual vertices and components
//               did not; (b) each vertex was assigned the same color
//               value; (c) each component was assigned the same color
//               value.
//
//               If has_overall_color() returns true, then get_color()
//               will return the overall color value.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
has_overall_color() const {
  if ((_overall & BAF_overall_updated) == 0) {
    ((BuilderPrimTempl<VTX> *)this)->update_overall_attrib();
  }
  return (_overall & BAF_overall_color)!=0;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::has_overall_pixel_size
//       Access: Public
//  Description: Returns true if the primitive has a single, overall
//               pixel_size value.  This can happen because of one of:
//               (a) the primitive had a pixel_size value assigned to
//               it directly, and its individual vertices and
//               components did not; (b) each vertex was assigned the
//               same pixel_size value; (c) each component was
//               assigned the same pixel_size value.
//
//               If has_overall_pixel_size() returns true, then
//               get_pixel_size() will return the overall pixel_size
//               value.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
has_overall_pixel_size() const {
  if ((_overall & BAF_overall_updated) == 0) {
    ((BuilderPrimTempl<VTX> *)this)->update_overall_attrib();
  }
  return (_overall & BAF_overall_pixel_size)!=0;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::has_vertex_normal
//       Access: Public
//  Description: Returns true if each of the primitive's vertices has
//               a different normal value set.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
has_vertex_normal() const {
  if ((_overall & BAF_overall_updated) == 0) {
    ((BuilderPrimTempl<VTX> *)this)->update_overall_attrib();
  }
  return (_overall & BAF_vertex_normal)!=0;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::has_vertex_color
//       Access: Public
//  Description: Returns true if each of the primitive's vertices has
//               a different color value set.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
has_vertex_color() const {
  if ((_overall & BAF_overall_updated) == 0) {
    ((BuilderPrimTempl<VTX> *)this)->update_overall_attrib();
  }
  return (_overall & BAF_vertex_color)!=0;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::has_vertex_texcoord
//       Access: Public
//  Description: Returns true if each of the primitive's vertices has
//               a texcoord value set.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
has_vertex_texcoord() const {
  if ((_overall & BAF_overall_updated) == 0) {
    ((BuilderPrimTempl<VTX> *)this)->update_overall_attrib();
  }
  return (_overall & BAF_vertex_texcoord)!=0;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::has_vertex_pixel_size
//       Access: Public
//  Description: Returns true if each of the primitive's vertices has
//               a different pixel_size value set.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
has_vertex_pixel_size() const {
  if ((_overall & BAF_overall_updated) == 0) {
    ((BuilderPrimTempl<VTX> *)this)->update_overall_attrib();
  }
  return (_overall & BAF_vertex_pixel_size)!=0;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::has_component_normal
//       Access: Public
//  Description: Returns true if the prim is a composite primitive
//               like a tristrip, and its individual components each
//               have a different normal value set.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
has_component_normal() const {
  if ((_overall & BAF_overall_updated) == 0) {
    ((BuilderPrimTempl<VTX> *)this)->update_overall_attrib();
  }
  return (_overall & BAF_component_normal)!=0;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::has_component_color
//       Access: Public
//  Description: Returns true if the prim is a composite primitive
//               like a tristrip, and its individual components each
//               have a different color value set.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
has_component_color() const {
  if ((_overall & BAF_overall_updated) == 0) {
    ((BuilderPrimTempl<VTX> *)this)->update_overall_attrib();
  }
  return (_overall & BAF_component_color)!=0;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::has_component_pixel_size
//       Access: Public
//  Description: Returns true if the prim is a composite primitive
//               like a tristrip, and its individual components each
//               have a different pixel_size value set.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
has_component_pixel_size() const {
  if ((_overall & BAF_overall_updated) == 0) {
    ((BuilderPrimTempl<VTX> *)this)->update_overall_attrib();
  }
  return (_overall & BAF_component_pixel_size)!=0;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::has_any_normal
//       Access: Public
//  Description: Returns true if the prim has a normal value set
//               at any level.  That is, it returns true if any of
//               has_overall_normal(), has_vertex_normal(), or
//               has_component_normal() is true.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
has_any_normal() const {
  return has_overall_normal() || has_vertex_normal() || has_component_normal();
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::has_any_color
//       Access: Public
//  Description: Returns true if the prim has a color value set at any
//               level.  That is, it returns true if any of
//               has_overall_color(), has_vertex_color(), or
//               has_component_color() is true.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
has_any_color() const {
  return has_overall_color() || has_vertex_color() || has_component_color();
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::has_any_texcoord
//       Access: Public
//  Description: Returns true if the prim has a texcoord value set
//               at any level.  Since texture coordinates can only be
//               set on vertices, this is the same thing as
//               has_vertex_texcoord().
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
has_any_texcoord() const {
  return has_vertex_texcoord();
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::has_any_pixel_size
//       Access: Public
//  Description: Returns true if the prim has a pixel_size value set
//               at any level.  That is, it returns true if any of
//               has_overall_pixel_size(), has_vertex_pixel_size(), or
//               has_component_pixel_size() is true.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
has_any_pixel_size() const {
  return has_overall_pixel_size() || has_vertex_pixel_size() ||
    has_component_pixel_size();
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::clear
//       Access: Public
//  Description: Resets the BuilderPrim to its initial, default state.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE BuilderPrimTempl<VTX> &BuilderPrimTempl<VTX>::
clear() {
  DAttrib::clear();
  _type = BPT_poly;
  return clear_vertices();
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::clear_vertices
//       Access: Public
//  Description: Removes all the vertices that have been added to the
//               BuilderPrim, without otherwise affecting its
//               properties.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE BuilderPrimTempl<VTX> &BuilderPrimTempl<VTX>::
clear_vertices() {
  _verts.clear();
  _components.clear();
  _overall = 0;

  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::set_attrib
//       Access: Public
//  Description: Copies the polygon attributes, color and normal, from
//               the indicated attribute structure (which might be
//               another BuilderPrim).
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE BuilderPrimTempl<VTX> &BuilderPrimTempl<VTX>::
set_attrib(const DAttrib &attrib) {
  DAttrib::operator = (attrib);
  _overall = 0;

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::get_type
//       Access: Public
//  Description: Indicates the type of primitive represented by the
//               BuilderPrim.  See set_type().
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE BuilderPrimType BuilderPrimTempl<VTX>::
get_type() const {
  return _type;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::set_type
//       Access: Public
//  Description: Indicates the type of primitive represented by the
//               BuilderPrim.  Normally, the user should set this to
//               one of BPT_poly, BPT_point, or BPT_line.  However,
//               other types are possible, and are created by the
//               mesher, including triangle strips, quad strips, and
//               triangle fans.
//
//               Setting this value changes the interpretation of the
//               vertices, for instance, a triangle strip is defined
//               by vertices that zigzag back and forth between the
//               triangles.  Note that when defining a composite
//               primitive such as a triangle strip, it is also
//               necessary to call add_component() the correct number
//               of times to match the calls to add_vertex(),
//               according to the primitive type.  See
//               add_component().
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE BuilderPrimTempl<VTX> &BuilderPrimTempl<VTX>::
set_type(BuilderPrimType t) {
  _type = t;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::get_normal
//       Access: Public
//  Description: Returns the value previously set by set_normal(), or
//               the overall normal value common to all vertices.  It
//               is an error to call this without first testing that
//               one of has_normal() or has_overall_normal() is true.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE TYPENAME BuilderPrimTempl<VTX>::NType BuilderPrimTempl<VTX>::
get_normal() const {
  return DAttrib::get_normal();
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::set_normal
//       Access: Public
//  Description: Sets the flat normal associated with the primitive.
//               If each of the primitive's vertices also has a normal
//               set, the vertex normals will override.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE BuilderPrimTempl<VTX> &BuilderPrimTempl<VTX>::
set_normal(const NType &n) {
  DAttrib::set_normal(n);
  _overall = 0;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::get_color
//       Access: Public
//  Description: Returns the value previously set by set_color(), or
//               the overall color value common to all vertices.  It
//               is an error to call this without first testing that
//               one of has_color() or has_overall_color() is true.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE TYPENAME BuilderPrimTempl<VTX>::CType BuilderPrimTempl<VTX>::
get_color() const {
  return DAttrib::get_color();
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::set_color
//       Access: Public
//  Description: Sets the flat color associated with the primitive.
//               If each of the primitive's vertices also has a color
//               set, the vertex colors will override.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE BuilderPrimTempl<VTX> &BuilderPrimTempl<VTX>::
set_color(const CType &c) {
  DAttrib::set_color(c);
  _overall = 0;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::get_pixel_size
//       Access: Public
//  Description: Returns the value previously set by set_pixel_size(),
//               or the overall pixel_size value common to all
//               vertices.  It is an error to call this without first
//               testing that one of has_pixel_size() or
//               has_overall_pixel_size() is true.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE float BuilderPrimTempl<VTX>::
get_pixel_size() const {
  return DAttrib::get_pixel_size();
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::set_pixel_size
//       Access: Public
//  Description: Sets the line thickness (for a polygon or line) or
//               size (for a point) in pixels.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE BuilderPrimTempl<VTX> &BuilderPrimTempl<VTX>::
set_pixel_size(float s) {
  DAttrib::set_pixel_size(s);
  _overall = 0;
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::add_vertex
//       Access: Public
//  Description: Adds a new vertex to the BuilderPrim.  The vertex
//               data is copied into the prim's internal structure.
//               Vertices should be added in counterclockwise order,
//               when viewed from the front.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE BuilderPrimTempl<VTX> &BuilderPrimTempl<VTX>::
add_vertex(const Vertex &v) {
  _overall = 0;
  _verts.push_back(v);
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::get_num_verts
//       Access: Public
//  Description: Returns the number of vertices in the primitive.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE int BuilderPrimTempl<VTX>::
get_num_verts() const {
  return _verts.size();
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::get_vertex
//       Access: Public
//  Description: Returns a reference to the nth vertex of the
//               primitive, where 0 <= n < get_num_verts().  This
//               reference may be modified directly, e.g. to set a
//               vertex normal on a vertex after it has been added to
//               the primitive.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE TYPENAME BuilderPrimTempl<VTX>::Vertex &BuilderPrimTempl<VTX>::
get_vertex(int n) {
  nassertr(n >= 0 && n < (int)_verts.size(), *(new Vertex));
  return _verts[n];
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::get_vertex (const)
//       Access: Public
//  Description: Returns the nth vertex of the primitive, where 0 <= n
//               < get_num_verts().
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE const TYPENAME BuilderPrimTempl<VTX>::Vertex &BuilderPrimTempl<VTX>::
get_vertex(int n) const {
  nassertr(n >= 0 && n < (int)_verts.size(), *(new Vertex));
  return _verts[n];
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::add_component
//       Access: Public
//  Description: Normally, this function is only used by the mesher
//               code; user code generally should not need to call it.
//
//               In addition to storing a simple polygon, a
//               BuilderPrim can store a composite primitive, like a
//               triangle strip.  In this case, it is useful to store
//               the attributes (like color and normal) associated
//               with each component.
//
//               When get_type() is one of the composite types, it is
//               the responsibility of the caller to call
//               add_component() once for each component, where the
//               number of components is defined by the type and the
//               number of vertices (e.g. for a triangle strip, the
//               number of components is the number of vertices - 2).
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE BuilderPrimTempl<VTX> &BuilderPrimTempl<VTX>::
add_component(const DAttrib &attrib) {
  _overall = 0;
  _components.push_back(attrib);
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::get_num_components
//       Access: Public
//  Description: Returns the number of times add_component() has been
//               called.  This should be, but is not necessarily, the
//               same as the number of components in the composite
//               primitive (e.g. a triangle strip).  See
//               add_component().
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE int BuilderPrimTempl<VTX>::
get_num_components() const {
  return _components.size();
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::get_component
//       Access: Public
//  Description: Returns a refernece to the attribute structure
//               associated with the nth component, where 0 <= n <
//               get_num_components().  This reference may be modified
//               directly, e.g. to change the color of a polygon after
//               it has been added to the tristrip.  See
//               add_component() and get_num_components().
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE TYPENAME BuilderPrimTempl<VTX>::DAttrib &BuilderPrimTempl<VTX>::
get_component(int n) {
  nassertr(n >= 0 && n < (int)_components.size(), *(new DAttrib));
  return _components[n];
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::get_component (const)
//       Access: Public
//  Description: Returns the attributes associated with the nth
//               component, where 0 <= n < get_num_components().  See
//               add_component() and get_num_components().
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE const TYPENAME BuilderPrimTempl<VTX>::DAttrib &BuilderPrimTempl<VTX>::
get_component(int n) const {
  nassertr(n >= 0 && n < (int)_components.size(), *(new DAttrib));
  return _components[n];
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::Ordering operator
//       Access: Public
//  Description: Returns true if this primitive should come before the
//               other one, in the bin ordering.  This is used to sort
//               polygons into groups which can be tristripped
//               together, similar to the same operation on the
//               BuilderBucket, except that this works at a finer
//               level of detail (i.e. on prims within the same
//               bucket).
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE bool BuilderPrimTempl<VTX>::
operator < (const BuilderPrimTempl<VTX> &other) const {
  int sv1 = sort_value();
  int sv2 = other.sort_value();

  // Normally, we compare only based on the integer sort_value().
  // However, other things being equal, if two prims have a different
  // pixel_size then they sort differently.

  // We can ignore per-vertex pixel_size, since there's no way to
  // render that anyway.

  return sv1 < sv2 ||
    (sv1==sv2 && has_pixel_size() && get_pixel_size() < other.get_pixel_size());
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::sort_value
//       Access: Protected
//  Description: Returns a number for grouping primitives, such that
//               only primitives that share the same number can be
//               tristripped together.
////////////////////////////////////////////////////////////////////
template <class VTX>
INLINE int BuilderPrimTempl<VTX>::
sort_value() const {
  // If a polygon is missing a normal, color, or uv, it shouldn't be
  // stripped along with polygons that have normals, colors, or uv's.
  // Furthermore, if one polygon has vertex normals and another is
  // flat shaded, they shouldn't be stripped together.
  return
    ((has_vertex_normal() ? 1:0) << 5) |
    ((has_vertex_color() ? 1:0) << 4) |
    ((has_any_texcoord() ? 1:0) << 3) |
    ((has_any_normal() ? 1:0) << 2) |
    ((has_any_color() ? 1:0) << 1) |
    ((has_pixel_size() ? 1:0) << 0);
}


////////////////////////////////////////////////////////////////////
//        Class: SameCoord
//  Description: An STL function object that identifies vertices as
//               equivalent when they have the same coordinate value,
//               regardless of their attribute values like texture
//               coordinates.  This is used in remove_doubled_verts().
////////////////////////////////////////////////////////////////////
template <class VTX>
class SameCoord {
public:
  bool operator () (const VTX &a, const VTX &b) const {
    return a.get_coord() == b.get_coord();
  }
};

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::remove_doubled_verts
//       Access: Public
//  Description: Removes consecutive identical vertices from the prim
//               definition.  These are meaningless and only confuse
//               polygon subdividing and meshing.
//
//               If closed is true, this also removes vertices doubled
//               at the beginning and end (as if the list of vertices
//               were implicitly closed, as it is for a polygon).
////////////////////////////////////////////////////////////////////
template <class VTX>
void BuilderPrimTempl<VTX>::
remove_doubled_verts(int closed) {
  // For some reason, writing this variable declaration this way
  // prevents VC++ from generating dozens of spurious "local variable
  // 'sc' used without having been initialized" warning messages.
  SameCoord<Vertex> sc = SameCoord<Vertex>();

  TYPENAME Verts::iterator new_end = unique(_verts.begin(), _verts.end(), sc);
  _verts.erase(new_end, _verts.end());

  if (closed) {
    // Then, if this is a polygon (which will be closed anyway),
    // remove the vertex from the end if it's a repeat of the
    // beginning.
    while (!_verts.empty() && sc(_verts.back(), _verts.front())) {
      _verts.pop_back();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::is_valid
//       Access: Public
//  Description: Returns true if the primitive is well-defined and has
//               the right number of vertices for its primitive type,
//               and all of its vertices are valid.
////////////////////////////////////////////////////////////////////
template <class VTX>
bool BuilderPrimTempl<VTX>::
is_valid() const {
  int num_verts = get_num_verts();
  for (int i = 0; i < num_verts; i++) {
    if (!get_vertex(i).is_valid()) {
      return false;
    }
  }

  switch (get_type()) {
  case BPT_poly:
    return num_verts >= 3;

  case BPT_point:
    return num_verts >= 1;

  case BPT_line:
    return num_verts == 2;

  case BPT_linestrip:
    return num_verts >= 2;

  case BPT_tri:
    return num_verts == 3;

  case BPT_tristrip:
  case BPT_trifan:
    return num_verts >= 3;

  case BPT_quad:
    return num_verts == 4;

  default:
    return false;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::output
//       Access: Public
//  Description: Formats the prim for output in some sensible way.
////////////////////////////////////////////////////////////////////
template <class VTX>
ostream &BuilderPrimTempl<VTX>::
output(ostream &out) const {
  int num_verts = get_num_verts();

  out << get_type() << ", " << num_verts << " vertices:\n";

  for (int i = 0; i < num_verts; i++) {
    out << "  " << i << ". " << get_vertex(i) << "\n";
  }

  if (has_overall_normal()) {
    out << "normal = " << get_normal() << "\n";
  }
  if (has_overall_color()) {
    out << "color = " << get_color() << "\n";
  }
  if (has_overall_pixel_size()) {
    out << "pixel_size = " << get_pixel_size() << "\n";
  }

  return out;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::update_overall_attrib
//       Access: Protected
//  Description: Examines the primitive and all of its vertices and
//               components to determine the amount of commonality of
//               each attribute, and updates the bits in _overall to
//               indicate this.  If an overall attribute is found, the
//               primitive attribute value is set to that common
//               value.
////////////////////////////////////////////////////////////////////
template <class VTX>
void BuilderPrimTempl<VTX>::
update_overall_attrib() {
  int num_verts = get_num_verts();
  int num_components = get_num_components();

  NType common_normal;
  CType common_color;
  float common_pixel_size = 0.0;

  bool has_common_normal = false;
  bool has_common_color = false;
  bool has_common_pixel_size = false;

  bool has_component_normal = false;
  bool has_component_color = false;
  bool has_component_pixel_size = false;

  bool has_vertex_normal = false;
  bool has_vertex_color = false;
  bool has_vertex_texcoord = false;
  bool has_vertex_pixel_size = false;

  int i;
  if (num_verts > 0) {
    has_vertex_texcoord = true;
    for (i = 0; i < num_verts && has_vertex_texcoord; i++) {
      has_vertex_texcoord = get_vertex(i).has_texcoord();
    }

    if (get_vertex(0).has_normal()) {
      common_normal = get_vertex(0).get_normal();
      has_common_normal = true;
      has_vertex_normal = true;

      for (i = 1; i < num_verts; i++) {
        Vertex &vertex = get_vertex(i);
        has_vertex_normal =
          (has_vertex_normal && vertex.has_normal());
        if (!vertex.has_normal() ||
            !(vertex.get_normal() == common_normal)) {
          has_common_normal = false;
        }
      }
    }
    if (get_vertex(0).has_color()) {
      common_color = get_vertex(0).get_color();
      has_common_color = true;
      has_vertex_color = true;

      for (i = 1; i < num_verts; i++) {
        Vertex &vertex = get_vertex(i);
        has_vertex_color =
          (has_vertex_color && vertex.has_color());
        if (!vertex.has_color() ||
            !(vertex.get_color() == common_color)) {
          has_common_color = false;
        }
      }
    }
    if (get_vertex(0).has_pixel_size()) {
      common_pixel_size = get_vertex(0).get_pixel_size();
      has_common_pixel_size = true;
      has_vertex_pixel_size = true;

      for (i = 1; i < num_verts; i++) {
        Vertex &vertex = get_vertex(i);
        has_vertex_pixel_size =
          (has_vertex_pixel_size && vertex.has_pixel_size());
        if (!vertex.has_pixel_size() ||
            !(vertex.get_pixel_size() == common_pixel_size)) {
          has_common_pixel_size = false;
        }
      }
    }
  }

  if (num_components > 0) {
    if (!has_vertex_normal && get_component(0).has_normal()) {
      common_normal = get_component(0).get_normal();
      has_common_normal = true;
      has_component_normal = true;

      for (i = 1; i < num_components; i++) {
        DAttrib &component = get_component(i);
        has_component_normal =
          (has_component_normal && component.has_normal());
        if (!component.has_normal() ||
            !(component.get_normal() == common_normal)) {
          has_common_normal = false;
        }
      }
    }
    if (!has_vertex_color && get_component(0).has_color()) {
      common_color = get_component(0).get_color();
      has_common_color = true;
      has_component_color = true;

      for (i = 1; i < num_components; i++) {
        DAttrib &component = get_component(i);
        has_component_color =
          (has_component_color && component.has_color());
        if (!component.has_color() ||
            !(component.get_color() == common_color)) {
          has_common_color = false;
        }
      }
    }
    if (!has_vertex_pixel_size && get_component(0).has_pixel_size()) {
      common_pixel_size = get_component(0).get_pixel_size();
      has_common_pixel_size = true;
      has_component_pixel_size = true;

      for (i = 1; i < num_components; i++) {
        DAttrib &component = get_component(i);
        has_component_pixel_size =
          (has_component_pixel_size && component.has_pixel_size());
        if (!component.has_pixel_size() ||
            !(component.get_pixel_size() == common_pixel_size)) {
          has_common_pixel_size = false;
        }
      }
    }
  }

  _overall = BAF_overall_updated;

  if (has_common_normal) {
    // The primitive has one overall normal, or each of the vertices
    // has the same normal.
    _overall |= BAF_overall_normal;
    DAttrib::set_normal(common_normal);

  } else if (has_component_normal) {
    // Each component primitive has a different normal.
    _overall |= BAF_component_normal;

  } else if (has_vertex_normal) {
    // Each vertex has a different normal.
    _overall |= BAF_vertex_normal;

  } else if (has_normal()) {
    // Well, none of the above, but the prim itself has a normal.
    _overall |= BAF_overall_normal;
  }


  if (has_common_color) {
    // The primitive has one overall color, or each of the vertices
    // has the same color.
    _overall |= BAF_overall_color;
    DAttrib::set_color(common_color);

  } else if (has_component_color) {
    // Each component primitive has a different color.
    _overall |= BAF_component_color;

  } else if (has_vertex_color) {
    // Each vertex has a different color.
    _overall |= BAF_vertex_color;

  } else if (has_color()) {
    // None of the above, but the prim itself has a color.
    _overall |= BAF_overall_color;
  }

  if (has_vertex_texcoord) {
    // Each vertex has a texture coordinate.
    _overall |= BAF_vertex_texcoord;
  }

  if (has_common_pixel_size) {
    // The primitive has one overall pixel size, or each of the vertices
    // has the same pixel size.
    _overall |= BAF_overall_pixel_size;
    DAttrib::set_pixel_size(common_pixel_size);

  } else if (has_component_pixel_size) {
    // Each component primitive has a different pixel size.
    _overall |= BAF_component_pixel_size;

  } else if (has_vertex_pixel_size) {
    // Each vertex has a different pixel size.
    _overall |= BAF_vertex_pixel_size;

  } else if (has_pixel_size()) {
    // The prim itself has a pixel size.
    _overall |= BAF_overall_pixel_size;
  }
}
