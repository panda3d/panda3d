// Filename: eggPrimitive.I
// Created by:  drose (16Jan99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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
  _connected_shading = S_unknown;
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
  _textures(copy._textures),
  _material(copy._material),
  _bface(copy._bface)
{
  copy_vertices(copy);
  _connected_shading = S_unknown;
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
  _textures = copy._textures;
  _material = copy._material;
  _bface = copy._bface;
  _connected_shading = S_unknown;
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
//     Function: EggPrimitive::get_sort_name
//       Access: Published
//  Description: Returns the name of the primitive for the purposes of
//               sorting primitives into different groups, if there is
//               one.
//
//               Presently, this is defined as the primitive name
//               itself, unless it begins with a digit.
////////////////////////////////////////////////////////////////////
INLINE string EggPrimitive::
get_sort_name() const {
  const string &name = get_name();
  if (!name.empty() && !isdigit(name[0])) {
    return name;
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::clear_connected_shading
//       Access: Published
//  Description: Resets the connected_shading member in this
//               primitive, so that get_connected_shading() will
//               recompute a new value.
////////////////////////////////////////////////////////////////////
INLINE void EggPrimitive::
clear_connected_shading() {
  _connected_shading = S_unknown;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::get_connected_shading
//       Access: Published
//  Description: Determines what sort of shading properties this
//               primitive's connected neighbors have.
//
//               To get the most accurate results, you should first
//               call clear_connected_shading() on all connected
//               primitives (or on all primitives in the egg file).
//               It might also be a good idea to call
//               remove_unused_vertices() to ensure proper
//               connectivity.
//
//               You may find it easiest to call these other methods
//               on the EggData root node (they are defined on
//               EggGroupNode).
////////////////////////////////////////////////////////////////////
INLINE EggPrimitive::Shading EggPrimitive::
get_connected_shading() const {
  if (_connected_shading == S_unknown) {
    ((EggPrimitive *)this)->set_connected_shading(S_unknown, this);
  }

  return _connected_shading;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::set_texture
//       Access: Published
//  Description: Replaces the current list of textures with the
//               indicated texture.
//
//               This method is deprecated and is used in support of
//               single-texturing only.  Please use the multitexture
//               variant add_texture instead.
////////////////////////////////////////////////////////////////////
INLINE void EggPrimitive::
set_texture(EggTexture *texture) {
  clear_texture();
  add_texture(texture);
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::has_texture
//       Access: Published
//  Description: Returns true if the primitive has any textures
//               specified, false otherwise.
//
//               This method is deprecated and is used in support of
//               single-texturing only.  New code should be written to
//               use the multitexture variants instead.
////////////////////////////////////////////////////////////////////
INLINE bool EggPrimitive::
has_texture() const {
  return get_num_textures() > 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::has_texture
//       Access: Published
//  Description: Returns true if the primitive has the particular
//               indicated texture, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggPrimitive::
has_texture(EggTexture *texture) const {
  PT_EggTexture t = texture;
  return (::find(_textures.begin(), _textures.end(), t) != _textures.end());
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::get_texture
//       Access: Published
//  Description: Returns the first texture on the primitive, if any,
//               or NULL if there are no textures on the primitive.
//
//               This method is deprecated and is used in support of
//               single-texturing only.  New code should be written to
//               use the multitexture variants instead.
////////////////////////////////////////////////////////////////////
INLINE EggTexture *EggPrimitive::
get_texture() const {
  return has_texture() ? get_texture(0) : (EggTexture *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::add_texture
//       Access: Published
//  Description: Applies the indicated texture to the primitive.
//
//               Note that, in the case of multiple textures being
//               applied to a single primitive, the order in which the
//               textures are applied does not affect the rendering
//               order; use EggTexture::set_sort() to specify that.
////////////////////////////////////////////////////////////////////
INLINE void EggPrimitive::
add_texture(EggTexture *texture) {
  _textures.push_back(texture);
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::clear_texture
//       Access: Published
//  Description: Removes any texturing from the primitive.
////////////////////////////////////////////////////////////////////
INLINE void EggPrimitive::
clear_texture() {
  _textures.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::get_num_textures
//       Access: Published
//  Description: Returns the number of textures applied to the
//               primitive.
////////////////////////////////////////////////////////////////////
INLINE int EggPrimitive::
get_num_textures() const {
  return _textures.size();
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::get_texture
//       Access: Published
//  Description: Returns the nth texture that has been applied to the
//               primitive.
////////////////////////////////////////////////////////////////////
INLINE EggTexture *EggPrimitive::
get_texture(int n) const {
  nassertr(n >= 0 && n < (int)_textures.size(), NULL);
  return _textures[n];
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
  prepare_add_vertex(x, position - _vertices.begin(), _vertices.size() + 1);
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
  prepare_remove_vertex(*position, position - _vertices.begin(), _vertices.size());
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

  // We pass -1 for i and n so that EggCompositePrimitive won't try to
  // adjust its _components list.
  prepare_remove_vertex(*position, -1, -1);
  prepare_add_vertex(x, -1, -1);
  *(Vertices::iterator &)position = x;

  x->test_pref_integrity();
  test_vref_integrity();
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::clear
//       Access: Published
//  Description: Removes all of the vertices from the primitive.
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
  return empty() ? (EggVertexPool *)NULL : _vertices.front()->get_pool();
}
