// Filename: eggPrimitive.I
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


////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggPrimitive::
EggPrimitive(const string &name): EggNode(name) {
  _bface = false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::Copy constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggPrimitive::
EggPrimitive(const EggPrimitive &copy) :
  EggNode(copy),
  EggAttributes(copy),
  _texture(copy._texture),
  _material(copy._material),
  _bface(copy._bface)
{
  copy_vertices(copy);
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::Copy assignment operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggPrimitive &EggPrimitive::
operator = (const EggPrimitive &copy) {
  EggNode::operator = (copy);
  EggAttributes::operator = (copy);
  copy_vertices(copy);
  _texture = copy._texture;
  _material = copy._material;
  _bface = copy._bface;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggPrimitive::
~EggPrimitive() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::set_texture
//       Access: Published
//  Description: Applies the indicated texture to the primitive.
////////////////////////////////////////////////////////////////////
INLINE void EggPrimitive::
set_texture(EggTexture *texture) {
  _texture = texture;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::clear_texture
//       Access: Published
//  Description: Removes any texturing from the primitive.
////////////////////////////////////////////////////////////////////
INLINE void EggPrimitive::
clear_texture() {
  _texture = (EggTexture *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::get_texture
//       Access: Published
//  Description: Returns a pointer to the applied texture, or NULL if
//               there is no texture applied.
////////////////////////////////////////////////////////////////////
INLINE EggTexture *EggPrimitive::
get_texture() const {
  return _texture;
}


////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::has_texture
//       Access: Published
//  Description: Returns true if the primitive is textured (and
//               get_texture() will return a real pointer), false
//               otherwise (and get_texture() will return NULL).
////////////////////////////////////////////////////////////////////
INLINE bool EggPrimitive::
has_texture() const {
  return _texture != (EggTexture*)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::set_material
//       Access: Published
//  Description: Applies the indicated material to the primitive.
////////////////////////////////////////////////////////////////////
INLINE void EggPrimitive::
set_material(EggMaterial *material) {
  _material = material;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::clear_material
//       Access: Published
//  Description: Removes any material from the primitive.
////////////////////////////////////////////////////////////////////
INLINE void EggPrimitive::
clear_material() {
  _material = (EggMaterial *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::get_material
//       Access: Published
//  Description: Returns a pointer to the applied material, or NULL if
//               there is no material applied.
////////////////////////////////////////////////////////////////////
INLINE EggMaterial *EggPrimitive::
get_material() const {
  return _material;
}


////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::has_material
//       Access: Published
//  Description: Returns true if the primitive is materiald (and
//               get_material() will return a real pointer), false
//               otherwise (and get_material() will return NULL).
////////////////////////////////////////////////////////////////////
INLINE bool EggPrimitive::
has_material() const {
  return _material != (EggMaterial *)NULL;
}


////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::set_bface_flag
//       Access: Published
//  Description: Sets the backfacing flag of the polygon.  If this is
//               true, the polygon will be rendered so that both faces
//               are visible; if it is false, only the front face of
//               the polygon will be visible.
////////////////////////////////////////////////////////////////////
INLINE void EggPrimitive::
set_bface_flag(bool flag) {
  _bface = flag;
}


////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::get_bface_flag
//       Access: Published
//  Description: Retrieves the backfacing flag of the polygon.  See
//               set_bface_flag().
////////////////////////////////////////////////////////////////////
INLINE bool EggPrimitive::
get_bface_flag() const {
  return _bface;
}


////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::begin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggPrimitive::iterator EggPrimitive::
begin() const {
  return _vertices.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::end
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggPrimitive::iterator EggPrimitive::
end() const {
  return _vertices.end();
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::rbegin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggPrimitive::reverse_iterator EggPrimitive::
rbegin() const {
  return _vertices.rbegin();
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::rend
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggPrimitive::reverse_iterator EggPrimitive::
rend() const {
  return _vertices.rend();
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::empty
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggPrimitive::
empty() const {
  return _vertices.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggPrimitive::size_type EggPrimitive::
size() const {
  return _vertices.size();
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::Indexing operator
//       Access: Public
//  Description: This is read-only: you can't assign directly to an
//               indexed vertex.  See set_vertex() instead.
////////////////////////////////////////////////////////////////////
INLINE EggVertex *EggPrimitive::
operator [] (int index) const {
  nassertr(index >= 0 && index < (int)size(), NULL);
  return *(begin() + index);
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::insert
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggPrimitive::iterator EggPrimitive::
insert(iterator position, EggVertex *x) {
  prepare_add_vertex(x);
  iterator i = _vertices.insert((Vertices::iterator &)position, x);
  x->test_pref_integrity();
  test_vref_integrity();
  return i;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::erase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggPrimitive::iterator EggPrimitive::
erase(iterator position) {
  prepare_remove_vertex(*position);
  iterator i = _vertices.erase((Vertices::iterator &)position);
  test_vref_integrity();
  return i;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::replace
//       Access: Public
//  Description: Replaces the vertex at the indicated position with
//               the indicated vertex.  It is an error to call this
//               with an invalid position iterator (e.g. end()).
////////////////////////////////////////////////////////////////////
INLINE void EggPrimitive::
replace(iterator position, EggVertex *x) {
  nassertv(position != end());

  prepare_remove_vertex(*position);
  prepare_add_vertex(x);
  *(Vertices::iterator &)position = x;

  x->test_pref_integrity();
  test_vref_integrity();
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::clear
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggPrimitive::
clear() {
  erase(begin(), end());
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::get_num_vertices
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int EggPrimitive::
get_num_vertices() const {
  return size();
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::set_vertex
//       Access: Published
//  Description: Replaces a particular vertex based on its index
//               number in the list of vertices.  This is just a
//               convenience function for people who don't want to
//               mess with the iterators.
////////////////////////////////////////////////////////////////////
INLINE void EggPrimitive::
set_vertex(int index, EggVertex *vertex) {
  nassertv(index >= 0 && index < (int)size());
  replace(begin() + index, vertex);
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::get_vertex
//       Access: Published
//  Description: Returns a particular index based on its index number.
////////////////////////////////////////////////////////////////////
INLINE EggVertex *EggPrimitive::
get_vertex(int index) const {
  nassertr(index >= 0 && index < (int)size(), NULL);
  return *(begin() + index);
}


////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::get_pool
//       Access: Published
//  Description: Returns the vertex pool associated with the vertices
//               of the primitive, or NULL if the primitive has no
//               vertices.
////////////////////////////////////////////////////////////////////
INLINE EggVertexPool *EggPrimitive::
get_pool() const {
  return empty() ? (EggVertexPool *)0L : _vertices.front()->get_pool();
}
