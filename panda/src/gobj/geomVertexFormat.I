/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexFormat.I
 * @author drose
 * @date 2005-03-07
 */

INLINE std::ostream &
operator << (std::ostream &out, const GeomVertexFormat &obj) {
  obj.output(out);
  return out;
}

/**
 * Returns true if this format has been registered, false if it has not.  It
 * may not be used for a Geom until it has been registered, but once
 * registered, it may no longer be modified.
 */
INLINE bool GeomVertexFormat::
is_registered() const {
  return _is_registered;
}

/**
 * Adds the indicated format to the registry, if there is not an equivalent
 * format already there; in either case, returns the pointer to the equivalent
 * format now in the registry.
 *
 * This must be called before a format may be used in a Geom.  After this
 * call, you should discard the original pointer you passed in (which may or
 * may not now be invalid) and let its reference count decrement normally; you
 * should use only the returned value from this point on.
 */
INLINE CPT(GeomVertexFormat) GeomVertexFormat::
register_format(const GeomVertexFormat *format) {
  return get_registry()->register_format((GeomVertexFormat *)format);
}

/**
 * This flavor of register_format() implicitly creates a one-array vertex
 * format from the array definition.
 */
INLINE CPT(GeomVertexFormat) GeomVertexFormat::
register_format(const GeomVertexArrayFormat *format) {
  return register_format(new GeomVertexFormat(format));
}

/**
 * Returns the GeomVertexAnimationSpec that indicates how this format's
 * vertices are set up for animation.
 */
INLINE const GeomVertexAnimationSpec &GeomVertexFormat::
get_animation() const {
  return _animation;
}

/**
 * Resets the GeomVertexAnimationSpec that indicates how this format's
 * vertices are set up for animation.  You should also, of course, change the
 * columns in the tables accordingly.
 *
 * This may not be called once the format has been registered.
 */
INLINE void GeomVertexFormat::
set_animation(const GeomVertexAnimationSpec &animation) {
  nassertv(!_is_registered);
  _animation = animation;
}

/**
 * Returns the number of individual arrays required by the format.  If the
 * array data is completely interleaved, this will be 1; if it is completely
 * parallel, this will be the same as the number of data types.
 */
INLINE size_t GeomVertexFormat::
get_num_arrays() const {
  return _arrays.size();
}

/**
 * Returns the description of the nth array used by the format.
 */
INLINE const GeomVertexArrayFormat *GeomVertexFormat::
get_array(size_t array) const {
  nassertr(array < _arrays.size(), nullptr);
  return _arrays[array];
}

/**
 * Returns true if the format has the named column, false otherwise.
 */
INLINE bool GeomVertexFormat::
has_column(const InternalName *name) const {
  return (get_column(name) != nullptr);
}

/**
 * Returns the number of columns within the format that represent points in
 * space.
 *
 * This may only be called after the format has been registered.
 */
INLINE size_t GeomVertexFormat::
get_num_points() const {
  nassertr(_is_registered, 0);
  return _points.size();
}

/**
 * Returns the name of the nth point column.  This represents a point in
 * space, which should be transformed by any spatial transform matrix.
 *
 * This may only be called after the format has been registered.
 */
INLINE const InternalName *GeomVertexFormat::
get_point(size_t n) const {
  nassertr(_is_registered, nullptr);
  nassertr(n < _points.size(), nullptr);
  return _points[n];
}

/**
 * Returns the number of columns within the format that represent directional
 * vectors.
 *
 * This may only be called after the format has been registered.
 */
INLINE size_t GeomVertexFormat::
get_num_vectors() const {
  nassertr(_is_registered, 0);
  return _vectors.size();
}

/**
 * Returns the name of the nth vector column.  This represents a directional
 * vector, which should be transformed by any spatial transform matrix as a
 * vector.
 *
 * This may only be called after the format has been registered.
 */
INLINE const InternalName *GeomVertexFormat::
get_vector(size_t n) const {
  nassertr(_is_registered, nullptr);
  nassertr(n < _vectors.size(), nullptr);
  return _vectors[n];
}

/**
 * Returns the number of columns within the format that represent texture
 * coordinates.
 *
 * This may only be called after the format has been registered.
 */
INLINE size_t GeomVertexFormat::
get_num_texcoords() const {
  nassertr(_is_registered, 0);
  return _texcoords.size();
}

/**
 * Returns the name of the nth texcoord column.  This represents a texture
 * coordinate.
 *
 * This may only be called after the format has been registered.
 */
INLINE const InternalName *GeomVertexFormat::
get_texcoord(size_t n) const {
  nassertr(_is_registered, nullptr);
  nassertr(n < _texcoords.size(), nullptr);
  return _texcoords[n];
}

/**
 * Returns the number of columns within the format that represent morph
 * deltas.
 *
 * This may only be called after the format has been registered.
 */
INLINE size_t GeomVertexFormat::
get_num_morphs() const {
  nassertr(_is_registered, 0);

  return _morphs.size();
}

/**
 * Returns the slider name associated with the nth morph column.  This is the
 * name of the slider that will control the morph, and should be defined
 * within the SliderTable associated with the GeomVertexData.
 *
 * This may only be called after the format has been registered.
 */
INLINE const InternalName *GeomVertexFormat::
get_morph_slider(size_t n) const {
  nassertr(_is_registered, nullptr);
  nassertr(n < _morphs.size(), nullptr);

  return _morphs[n]._slider;
}

/**
 * Returns the name of the base column that the nth morph modifies.  This
 * column will also be defined within the format, and can be retrieved via
 * get_array_with() and/or get_column().
 *
 * This may only be called after the format has been registered.
 */
INLINE const InternalName *GeomVertexFormat::
get_morph_base(size_t n) const {
  nassertr(_is_registered, nullptr);
  nassertr(n < _morphs.size(), nullptr);

  return _morphs[n]._base;
}

/**
 * Returns the name of the column that defines the nth morph.  This contains
 * the delta offsets that are to be applied to the column defined by
 * get_morph_base().  This column will be defined within the format, and can
 * be retrieved via get_array_with() and/or get_column().
 *
 * This may only be called after the format has been registered.
 */
INLINE const InternalName *GeomVertexFormat::
get_morph_delta(size_t n) const {
  nassertr(_is_registered, nullptr);
  nassertr(n < _morphs.size(), nullptr);

  return _morphs[n]._delta;
}

/**
 * Returns a standard vertex format containing no arrays at all, useful for
 * pull-style vertex rendering.
 */
INLINE const GeomVertexFormat *GeomVertexFormat::
get_empty() {
  return get_registry()->_empty;
}

/**
 * Returns a standard vertex format with just a 3-component vertex position.
 */
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3() {
  return get_registry()->_v3;
}

/**
 * Returns a standard vertex format with a 3-component normal and a
 * 3-component vertex position.
 */
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3n3() {
  return get_registry()->_v3n3;
}

/**
 * Returns a standard vertex format with a 2-component texture coordinate pair
 * and a 3-component vertex position.
 */
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3t2() {
  return get_registry()->_v3t2;
}

/**
 * Returns a standard vertex format with a 2-component texture coordinate
 * pair, a 3-component normal, and a 3-component vertex position.
 */
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3n3t2() {
  return get_registry()->_v3n3t2;
}

/**
 * Returns a standard vertex format with a packed color and a 3-component
 * vertex position.
 */
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3cp() {
  return get_registry()->_v3cp;
}

/**
 * Returns a standard vertex format with a packed color, a 3-component normal,
 * and a 3-component vertex position.
 */
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3n3cp() {
  return get_registry()->_v3n3cp;
}

/**
 * Returns a standard vertex format with a 2-component texture coordinate
 * pair, a packed color, and a 3-component vertex position.
 */
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3cpt2() {
  return get_registry()->_v3cpt2;
}

/**
 * Returns a standard vertex format with a 2-component texture coordinate
 * pair, a packed color, a 3-component normal, and a 3-component vertex
 * position.
 */
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3n3cpt2() {
  return get_registry()->_v3n3cpt2;
}

/**
 * Returns a standard vertex format with a 4-component color and a 3-component
 * vertex position.
 */
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3c4() {
  return get_registry()->_v3c4;
}

/**
 * Returns a standard vertex format with a 4-component color, a 3-component
 * normal, and a 3-component vertex position.
 */
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3n3c4() {
  return get_registry()->_v3n3c4;
}

/**
 * Returns a standard vertex format with a 2-component texture coordinate
 * pair, a 4-component color, and a 3-component vertex position.
 */
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3c4t2() {
  return get_registry()->_v3c4t2;
}

/**
 * Returns a standard vertex format with a 2-component texture coordinate
 * pair, a 4-component color, a 3-component normal, and a 3-component vertex
 * position.
 */
INLINE const GeomVertexFormat *GeomVertexFormat::
get_v3n3c4t2() {
  return get_registry()->_v3n3c4t2;
}

/**
 * Returns the array index of the array including the "vertex" column, or -1
 * if there is no such array.
 *
 * This may only be called after the format has been registered.
 */
INLINE int GeomVertexFormat::
get_vertex_array_index() const {
  nassertr(_is_registered, -1);
  return _vertex_array_index;
}

/**
 * Returns the column definition of the "vertex" column, or NULL if there is
 * no such column.
 *
 * This may only be called after the format has been registered.
 */
INLINE const GeomVertexColumn *GeomVertexFormat::
get_vertex_column() const {
  nassertr(_is_registered, nullptr);
  return _vertex_column;
}

/**
 * Returns the array index of the array including the "normal" column, or -1
 * if there is no such array.
 *
 * This may only be called after the format has been registered.
 */
INLINE int GeomVertexFormat::
get_normal_array_index() const {
  nassertr(_is_registered, -1);
  return _normal_array_index;
}

/**
 * Returns the column definition of the "normal" column, or NULL if there is
 * no such column.
 *
 * This may only be called after the format has been registered.
 */
INLINE const GeomVertexColumn *GeomVertexFormat::
get_normal_column() const {
  nassertr(_is_registered, nullptr);
  return _normal_column;
}

/**
 * Returns the array index of the array including the "color" column, or -1 if
 * there is no such array.
 *
 * This may only be called after the format has been registered.
 */
INLINE int GeomVertexFormat::
get_color_array_index() const {
  nassertr(_is_registered, -1);
  return _color_array_index;
}

/**
 * Returns the column definition of the "color" column, or NULL if there is no
 * such column.
 *
 * This may only be called after the format has been registered.
 */
INLINE const GeomVertexColumn *GeomVertexFormat::
get_color_column() const {
  nassertr(_is_registered, nullptr);
  return _color_column;
}

/**
 * Returns the global registry object.
 */
INLINE GeomVertexFormat::Registry *GeomVertexFormat::
get_registry() {
  if (_registry == nullptr) {
    make_registry();
  }
  return _registry;
}

/**
 * This flavor of register_format() implicitly creates a one-array vertex
 * format from the array definition.
 */
INLINE CPT(GeomVertexFormat) GeomVertexFormat::Registry::
register_format(GeomVertexArrayFormat *format) {
  return register_format(new GeomVertexFormat(format));
}
