// Filename: geomVertexFormat.I
// Created by:  drose (07Mar05)
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


INLINE ostream &
operator << (ostream &out, const GeomVertexFormat &obj) {
  obj.output(out);
  return out;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::is_registered
//       Access: Published
//  Description: Returns true if this format has been registered,
//               false if it has not.  It may not be used for a Geom
//               until it has been registered, but once registered, it
//               may no longer be modified.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexFormat::
is_registered() const {
  return _is_registered;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::register_format
//       Access: Published, Static
//  Description: Adds the indicated format to the registry, if there
//               is not an equivalent format already there; in either
//               case, returns the pointer to the equivalent format
//               now in the registry.
//
//               This must be called before a format may be used in a
//               Geom.  After this call, you should discard the
//               original pointer you passed in (which may or may not
//               now be invalid) and let its reference count decrement
//               normally; you should use only the returned value from
//               this point on.
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomVertexFormat) GeomVertexFormat::
register_format(const GeomVertexFormat *format) {
  return get_registry()->register_format((GeomVertexFormat *)format);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::register_format
//       Access: Published, Static
//  Description: This flavor of register_format() implicitly creates a
//               one-array vertex format from the array definition.
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomVertexFormat) GeomVertexFormat::
register_format(const GeomVertexArrayFormat *format) {
  return register_format(new GeomVertexFormat(format));
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_animation
//       Access: Published
//  Description: Returns the GeomVertexAnimationSpec that indicates
//               how this format's vertices are set up for animation.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexAnimationSpec &GeomVertexFormat::
get_animation() const {
  return _animation;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::set_animation
//       Access: Published
//  Description: Resets the GeomVertexAnimationSpec that indicates
//               how this format's vertices are set up for animation.
//               You should also, of course, change the columns in the
//               tables accordingly.
//
//               This may not be called once the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE void GeomVertexFormat::
set_animation(const GeomVertexAnimationSpec &animation) {
  nassertv(!_is_registered);
  _animation = animation;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_num_arrays
//       Access: Published
//  Description: Returns the number of individual arrays required by
//               the format.  If the array data is completely
//               interleaved, this will be 1; if it is completely
//               parallel, this will be the same as the number of data
//               types.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexFormat::
get_num_arrays() const {
  return _arrays.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_array
//       Access: Published
//  Description: Returns the description of the nth array used by the
//               format.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexArrayFormat *GeomVertexFormat::
get_array(int array) const {
  nassertr(array >= 0 && array < (int)_arrays.size(), NULL);
  return _arrays[array];
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::has_column
//       Access: Published
//  Description: Returns true if the format has the named column,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GeomVertexFormat::
has_column(const InternalName *name) const {
  return (get_column(name) != (GeomVertexColumn *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_num_points
//       Access: Published
//  Description: Returns the number of columns within the format
//               that represent points in space.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexFormat::
get_num_points() const {
  nassertr(_is_registered, 0);
  return _points.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_point
//       Access: Published
//  Description: Returns the name of the nth point column.  This
//               represents a point in space, which should be
//               transformed by any spatial transform matrix.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE const InternalName *GeomVertexFormat::
get_point(int n) const {
  nassertr(_is_registered, NULL);
  nassertr(n >= 0 && n < (int)_points.size(), NULL);
  return _points[n];
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_num_vectors
//       Access: Published
//  Description: Returns the number of columns within the format
//               that represent directional vectors.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexFormat::
get_num_vectors() const {
  nassertr(_is_registered, 0);
  return _vectors.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_vector
//       Access: Published
//  Description: Returns the name of the nth vector column.  This
//               represents a directional vector, which should be
//               transformed by any spatial transform matrix as a
//               vector.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE const InternalName *GeomVertexFormat::
get_vector(int n) const {
  nassertr(_is_registered, NULL);
  nassertr(n >= 0 && n < (int)_vectors.size(), NULL);
  return _vectors[n];
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_num_texcoords
//       Access: Published
//  Description: Returns the number of columns within the format
//               that represent texture coordinates.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexFormat::
get_num_texcoords() const {
  nassertr(_is_registered, 0);
  return _texcoords.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_texcoord
//       Access: Published
//  Description: Returns the name of the nth texcoord column.  This
//               represents a texture coordinate.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE const InternalName *GeomVertexFormat::
get_texcoord(int n) const {
  nassertr(_is_registered, NULL);
  nassertr(n >= 0 && n < (int)_texcoords.size(), NULL);
  return _texcoords[n];
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_num_morphs
//       Access: Published
//  Description: Returns the number of columns within the format
//               that represent morph deltas.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexFormat::
get_num_morphs() const {
  nassertr(_is_registered, 0);

  return _morphs.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_morph_slider
//       Access: Published
//  Description: Returns the slider name associated with the nth morph
//               column.  This is the name of the slider that will
//               control the morph, and should be defined within the
//               SliderTable associated with the GeomVertexData.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE const InternalName *GeomVertexFormat::
get_morph_slider(int n) const {
  nassertr(_is_registered, NULL);
  nassertr(n >= 0 && n < (int)_morphs.size(), NULL);

  return _morphs[n]._slider;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_morph_base
//       Access: Published
//  Description: Returns the name of the base column that the nth
//               morph modifies.  This column will also be defined
//               within the format, and can be retrieved via
//               get_array_with() and/or get_column().
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE const InternalName *GeomVertexFormat::
get_morph_base(int n) const {
  nassertr(_is_registered, NULL);
  nassertr(n >= 0 && n < (int)_morphs.size(), NULL);

  return _morphs[n]._base;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_morph_delta
//       Access: Published
//  Description: Returns the name of the column that defines the
//               nth morph.  This contains the delta offsets that are
//               to be applied to the column defined by
//               get_morph_base().  This column will be defined
//               within the format, and can be retrieved via
//               get_array_with() and/or get_column().
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE const InternalName *GeomVertexFormat::
get_morph_delta(int n) const {
  nassertr(_is_registered, NULL);
  nassertr(n >= 0 && n < (int)_morphs.size(), NULL);

  return _morphs[n]._delta;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_v3
//       Access: Published, Static
//  Description: Returns a standard vertex format with just a
//               3-component vertex position.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3() {
  return get_registry()->_v3;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_v3n3
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 3-component
//               normal and a 3-component vertex position.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3n3() {
  return get_registry()->_v3n3;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_v3t2
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 2-component
//               texture coordinate pair and a 3-component vertex
//               position.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3t2() {
  return get_registry()->_v3t2;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_v3n3t2
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 2-component
//               texture coordinate pair, a 3-component normal, and a
//               3-component vertex position.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3n3t2() {
  return get_registry()->_v3n3t2;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_v3cp
//       Access: Published, Static
//  Description: Returns a standard vertex format with a packed
//               color and a 3-component vertex position.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3cp() {
  return get_registry()->_v3cp;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_v3n3cp
//       Access: Published, Static
//  Description: Returns a standard vertex format with a packed
//               color, a 3-component normal, and a 3-component vertex
//               position.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3n3cp() {
  return get_registry()->_v3n3cp;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_v3cpt2
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 2-component
//               texture coordinate pair, a packed color, and a
//               3-component vertex position.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3cpt2() {
  return get_registry()->_v3cpt2;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_v3n3cpt2
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 2-component
//               texture coordinate pair, a packed color, a
//               3-component normal, and a 3-component vertex
//               position.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3n3cpt2() {
  return get_registry()->_v3n3cpt2;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_v3c4
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 4-component
//               color and a 3-component vertex position.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3c4() {
  return get_registry()->_v3c4;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_v3n3c4
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 4-component
//               color, a 3-component normal, and a 3-component vertex
//               position.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3n3c4() {
  return get_registry()->_v3n3c4;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_v3c4t2
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 2-component
//               texture coordinate pair, a 4-component color, and a
//               3-component vertex position.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3c4t2() {
  return get_registry()->_v3c4t2;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_v3n3c4t2
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 2-component
//               texture coordinate pair, a 4-component color, a
//               3-component normal, and a 3-component vertex
//               position.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3n3c4t2() {
  return get_registry()->_v3n3c4t2;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_vertex_array_index
//       Access: Public
//  Description: Returns the array index of the array including the
//               "vertex" column, or -1 if there is no such array.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexFormat::
get_vertex_array_index() const {
  nassertr(_is_registered, -1);
  return _vertex_array_index;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_vertex_column
//       Access: Public
//  Description: Returns the column definition of the "vertex" column,
//               or NULL if there is no such column.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexColumn *GeomVertexFormat::
get_vertex_column() const {
  nassertr(_is_registered, NULL);
  return _vertex_column;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_normal_array_index
//       Access: Public
//  Description: Returns the array index of the array including the
//               "normal" column, or -1 if there is no such array.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexFormat::
get_normal_array_index() const {
  nassertr(_is_registered, -1);
  return _normal_array_index;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_normal_column
//       Access: Public
//  Description: Returns the column definition of the "normal" column,
//               or NULL if there is no such column.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexColumn *GeomVertexFormat::
get_normal_column() const {
  nassertr(_is_registered, NULL);
  return _normal_column;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_color_array_index
//       Access: Public
//  Description: Returns the array index of the array including the
//               "color" column, or -1 if there is no such array.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE int GeomVertexFormat::
get_color_array_index() const {
  nassertr(_is_registered, -1);
  return _color_array_index;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_color_column
//       Access: Public
//  Description: Returns the column definition of the "color" column,
//               or NULL if there is no such column.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexColumn *GeomVertexFormat::
get_color_column() const {
  nassertr(_is_registered, NULL);
  return _color_column;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::get_registry
//       Access: Private
//  Description: Returns the global registry object.
////////////////////////////////////////////////////////////////////
INLINE GeomVertexFormat::Registry *GeomVertexFormat::
get_registry() {
  if (_registry == (Registry *)NULL) {
    make_registry();
  }
  return _registry;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomVertexFormat::Registry::register_format
//       Access: Public
//  Description: This flavor of register_format() implicitly creates a
//               one-array vertex format from the array definition.
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomVertexFormat) GeomVertexFormat::Registry::
register_format(GeomVertexArrayFormat *format) {
  return register_format(new GeomVertexFormat(format));
}
