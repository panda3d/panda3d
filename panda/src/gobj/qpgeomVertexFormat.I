// Filename: qpgeomVertexFormat.I
// Created by:  drose (07Mar05)
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


INLINE ostream &
operator << (ostream &out, const qpGeomVertexFormat &obj) {
  obj.output(out);
  return out;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::is_registered
//       Access: Published
//  Description: Returns true if this format has been registered,
//               false if it has not.  It may not be used for a Geom
//               until it has been registered, but once registered, it
//               may no longer be modified.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexFormat::
is_registered() const {
  return _is_registered;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::register_format
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
INLINE CPT(qpGeomVertexFormat) qpGeomVertexFormat::
register_format(qpGeomVertexFormat *format) {
  return get_registry()->register_format(format);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::register_format
//       Access: Published, Static
//  Description: This flavor of register_format() implicitly creates a
//               one-array vertex format from the array definition.
////////////////////////////////////////////////////////////////////
INLINE CPT(qpGeomVertexFormat) qpGeomVertexFormat::
register_format(qpGeomVertexArrayFormat *format) {
  return get_registry()->register_format(format);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_animation
//       Access: Published
//  Description: Returns the GeomVertexAnimationSpec that indicates
//               how this format's vertices are set up for animation.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexAnimationSpec &qpGeomVertexFormat::
get_animation() const {
  return _animation;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::set_animation
//       Access: Published
//  Description: Resets the GeomVertexAnimationSpec that indicates
//               how this format's vertices are set up for animation.
//               You should also, of course, change the columns in the
//               tables accordingly.
//
//               This may not be called once the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexFormat::
set_animation(const qpGeomVertexAnimationSpec &animation) {
  nassertv(!_is_registered);
  _animation = animation;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_num_arrays
//       Access: Published
//  Description: Returns the number of individual arrays required by
//               the format.  If the array data is completely
//               interleaved, this will be 1; if it is completely
//               parallel, this will be the same as the number of data
//               types.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexFormat::
get_num_arrays() const {
  return _arrays.size();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_array
//       Access: Published
//  Description: Returns the description of the nth array used by the
//               format.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexArrayFormat *qpGeomVertexFormat::
get_array(int array) const {
  nassertr(array >= 0 && array < (int)_arrays.size(), NULL);
  return _arrays[array];
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::has_data_type
//       Access: Published
//  Description: Returns true if the format has the named data type,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexFormat::
has_data_type(const InternalName *name) const {
  return (get_data_type(name) != (qpGeomVertexDataType *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_num_morphs
//       Access: Published
//  Description: Returns the number of data types within the format
//               that represent morph deltas.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexFormat::
get_num_morphs() const {
  nassertr(_is_registered, 0);
  
  return _morphs.size();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_morph_slider
//       Access: Published
//  Description: Returns the slider name associated with the nth morph
//               data type.  This is the name of the slider that will
//               control the morph, and should be defined within the
//               SliderTable associated with the GeomVertexData.
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE const InternalName *qpGeomVertexFormat::
get_morph_slider(int n) const {
  nassertr(_is_registered, NULL);
  nassertr(n >= 0 && n < (int)_morphs.size(), NULL);

  return _morphs[n]._slider;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_morph_base
//       Access: Published
//  Description: Returns the name of the base data type that the nth
//               morph modifies.  This data type will also be defined
//               within the format, and can be retrieved via
//               get_array_with() and/or get_data_type().
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE const InternalName *qpGeomVertexFormat::
get_morph_base(int n) const {
  nassertr(_is_registered, NULL);
  nassertr(n >= 0 && n < (int)_morphs.size(), NULL);

  return _morphs[n]._base;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_morph_delta
//       Access: Published
//  Description: Returns the name of the data type that defines the
//               nth morph.  This contains the delta offsets that are
//               to be applied to the data type defined by
//               get_morph_base().  This data type will be defined
//               within the format, and can be retrieved via
//               get_array_with() and/or get_data_type().
//
//               This may only be called after the format has been
//               registered.
////////////////////////////////////////////////////////////////////
INLINE const InternalName *qpGeomVertexFormat::
get_morph_delta(int n) const {
  nassertr(_is_registered, NULL);
  nassertr(n >= 0 && n < (int)_morphs.size(), NULL);

  return _morphs[n]._delta;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_v3
//       Access: Published, Static
//  Description: Returns a standard vertex format with just a
//               3-component vertex position.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexFormat *qpGeomVertexFormat::
get_v3() {
  return get_registry()->_v3;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_v3n3
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 3-component
//               normal and a 3-component vertex position.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexFormat *qpGeomVertexFormat::
get_v3n3() {
  return get_registry()->_v3n3;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_v3t2
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 2-component
//               texture coordinate pair and a 3-component vertex
//               position.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexFormat *qpGeomVertexFormat::
get_v3t2() {
  return get_registry()->_v3t2;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_v3n3t2
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 2-component
//               texture coordinate pair, a 3-component normal, and a
//               3-component vertex position.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexFormat *qpGeomVertexFormat::
get_v3n3t2() {
  return get_registry()->_v3n3t2;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_v3cp
//       Access: Published, Static
//  Description: Returns a standard vertex format with a packed
//               color and a 3-component vertex position.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexFormat *qpGeomVertexFormat::
get_v3cp() {
  return get_registry()->_v3cp;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_v3n3cp
//       Access: Published, Static
//  Description: Returns a standard vertex format with a packed
//               color, a 3-component normal, and a 3-component vertex
//               position.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexFormat *qpGeomVertexFormat::
get_v3n3cp() {
  return get_registry()->_v3n3cp;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_v3cpt2
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 2-component
//               texture coordinate pair, a packed color, and a
//               3-component vertex position.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexFormat *qpGeomVertexFormat::
get_v3cpt2() {
  return get_registry()->_v3cpt2;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_v3n3cpt2
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 2-component
//               texture coordinate pair, a packed color, a
//               3-component normal, and a 3-component vertex
//               position.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexFormat *qpGeomVertexFormat::
get_v3n3cpt2() {
  return get_registry()->_v3n3cpt2;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_v3c4
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 4-component
//               color and a 3-component vertex position.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexFormat *qpGeomVertexFormat::
get_v3c4() {
  return get_registry()->_v3c4;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_v3n3c4
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 4-component
//               color, a 3-component normal, and a 3-component vertex
//               position.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexFormat *qpGeomVertexFormat::
get_v3n3c4() {
  return get_registry()->_v3n3c4;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_v3c4t2
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 2-component
//               texture coordinate pair, a 4-component color, and a
//               3-component vertex position.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexFormat *qpGeomVertexFormat::
get_v3c4t2() {
  return get_registry()->_v3c4t2;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_v3n3c4t2
//       Access: Published, Static
//  Description: Returns a standard vertex format with a 2-component
//               texture coordinate pair, a 4-component color, a
//               3-component normal, and a 3-component vertex
//               position.
////////////////////////////////////////////////////////////////////
INLINE const qpGeomVertexFormat *qpGeomVertexFormat::
get_v3n3c4t2() {
  return get_registry()->_v3n3c4t2;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::get_registry
//       Access: Private
//  Description: Returns the global registry object.
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexFormat::Registry *qpGeomVertexFormat::
get_registry() {
  if (_registry == (Registry *)NULL) {
    make_registry();
  }
  return _registry;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexFormat::Registry::register_format
//       Access: Public
//  Description: This flavor of register_format() implicitly creates a
//               one-array vertex format from the array definition.
////////////////////////////////////////////////////////////////////
INLINE CPT(qpGeomVertexFormat) qpGeomVertexFormat::Registry::
register_format(qpGeomVertexArrayFormat *format) {
  return register_format(new qpGeomVertexFormat(format));
}
