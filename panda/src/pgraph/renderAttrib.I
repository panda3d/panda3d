// Filename: renderAttrib.I
// Created by:  drose (21Feb02)
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
//     Function: RenderAttrib::compose
//       Access: Published
//  Description: Returns a new RenderAttrib object that represents the
//               composition of this attrib with the other attrib.  In
//               most cases, this is the same as the other attrib; a
//               compose b produces b.  Some kinds of attributes, like
//               a TextureTransform, for instance, might produce a new
//               result: a compose b produces c.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderAttrib) RenderAttrib::
compose(const RenderAttrib *other) const {
  return compose_impl(other);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::invert_compose
//       Access: Published
//  Description: Returns a new RenderAttrib object that represents the
//               composition of the inverse of this attrib with the
//               other attrib.  In most cases, this is the same as the
//               other attrib; !a compose b produces b.  Some kinds of
//               attributes, like a TextureTransform, for instance,
//               might produce a new result: !a compose b produces c.
//
//               This is similar to compose() except that the source
//               attrib is inverted first.  This is used to compute
//               the relative attribute for one node as viewed from
//               some other node, which is especially useful for
//               transform-type attributes.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderAttrib) RenderAttrib::
invert_compose(const RenderAttrib *other) const {
  return invert_compose_impl(other);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::always_reissue
//       Access: Public
//  Description: Returns true if the RenderAttrib should be reissued
//               to the GSG with every state change, even if it is the
//               same pointer as it was before; or false for the
//               normal case, to reissue only when the RenderAttrib
//               pointer changes.
////////////////////////////////////////////////////////////////////
INLINE bool RenderAttrib::
always_reissue() const {
  return _always_reissue;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::compare_to
//       Access: Published
//  Description: Provides an arbitrary ordering among all unique
//               RenderAttribs, so we can store the essentially
//               different ones in a big set and throw away the rest.
//
//               This method is not needed outside of the RenderAttrib
//               class because all equivalent RenderAttrib objects are
//               guaranteed to share the same pointer; thus, a pointer
//               comparison is always sufficient.
////////////////////////////////////////////////////////////////////
INLINE int RenderAttrib::
compare_to(const RenderAttrib &other) const {
  // First, we compare the types; if they are of different types then
  // they sort differently.
  TypeHandle type = get_type();
  TypeHandle other_type = other.get_type();
  if (type != other_type) {
    return type.get_index() - other_type.get_index();
  }

  // We only call compare_to_impl() if they have the same type.
  return compare_to_impl(&other);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::get_hash
//       Access: Published
//  Description: Returns a suitable hash value for phash_map.
////////////////////////////////////////////////////////////////////
INLINE size_t RenderAttrib::
get_hash() const {
  return _hash;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::get_unique
//       Access: Published
//  Description: Returns the pointer to the unique RenderAttrib in
//               the cache that is equivalent to this one.  This may
//               be the same pointer as this object, or it may be a
//               different pointer; but it will be an equivalent
//               object, and it will be a shared pointer.  This may be
//               called from time to time to improve cache benefits.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderAttrib) RenderAttrib::
get_unique() const {
  return return_unique((RenderAttrib *)this);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::get_auto_shader_attrib
//       Access: Published
//  Description: Returns the variant of this RenderAttrib that's most
//               relevant for associating with an auto-generated
//               shader.  This should be a new RenderAttrib of the
//               same type as this one, with any superfluous data set
//               to neutral.  Only the parts of the attrib that
//               contribute to the shader should be reflected in the
//               returned attrib.  The idea is to associate the
//               auto-generated shader with the most neutral form of
//               all states, to allow it to be shared across as many
//               RenderState objects as possible.
//
//               If this RenderAttrib is completely irrelevant to the
//               auto-shader, this should return NULL to indicate that
//               the attrib won't be assocaited with the shader at
//               all.  In this case the attrib does not contribute to
//               the shader meaningfully.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderAttrib) RenderAttrib::
get_auto_shader_attrib(const RenderState *state) const {
  return get_auto_shader_attrib_impl(state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::calc_hash
//       Access: Published
//  Description: Calculates a suitable hash value for phash_map.
////////////////////////////////////////////////////////////////////
INLINE void RenderAttrib::
calc_hash() {
  size_t hash = get_hash_impl();

  // The type is also added to the hash.
  _hash = int_hash::add_hash(hash, get_type().get_index());
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::register_slot
//       Access: Public, Static
//  Description: Adds the indicated TypeHandle to the registry, if it
//               is not there already, and returns a unique slot
//               number.  See RenderAttribRegistry.
////////////////////////////////////////////////////////////////////
INLINE int RenderAttrib::
register_slot(TypeHandle type_handle, int sort,
              RenderAttribRegistry::MakeDefaultFunc *make_default_func) {
  RenderAttribRegistry *reg = RenderAttribRegistry::get_global_ptr();
  return reg->register_slot(type_handle, sort, make_default_func);
}
