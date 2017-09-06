/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderAttrib.I
 * @author drose
 * @date 2002-02-21
 */

/**
 * Returns a new RenderAttrib object that represents the composition of this
 * attrib with the other attrib.  In most cases, this is the same as the other
 * attrib; a compose b produces b.  Some kinds of attributes, like a
 * TextureTransform, for instance, might produce a new result: a compose b
 * produces c.
 */
INLINE CPT(RenderAttrib) RenderAttrib::
compose(const RenderAttrib *other) const {
  return compose_impl(other);
}

/**
 * Returns a new RenderAttrib object that represents the composition of the
 * inverse of this attrib with the other attrib.  In most cases, this is the
 * same as the other attrib; !a compose b produces b.  Some kinds of
 * attributes, like a TextureTransform, for instance, might produce a new
 * result: !a compose b produces c.
 *
 * This is similar to compose() except that the source attrib is inverted
 * first.  This is used to compute the relative attribute for one node as
 * viewed from some other node, which is especially useful for transform-type
 * attributes.
 */
INLINE CPT(RenderAttrib) RenderAttrib::
invert_compose(const RenderAttrib *other) const {
  return invert_compose_impl(other);
}

/**
 * Provides an arbitrary ordering among all unique RenderAttribs, so we can
 * store the essentially different ones in a big set and throw away the rest.
 *
 * This method is not needed outside of the RenderAttrib class because all
 * equivalent RenderAttrib objects are guaranteed to share the same pointer;
 * thus, a pointer comparison is always sufficient.
 */
INLINE int RenderAttrib::
compare_to(const RenderAttrib &other) const {
  // First, we compare the types; if they are of different types then they
  // sort differently.
  TypeHandle type = get_type();
  TypeHandle other_type = other.get_type();
  if (type != other_type) {
    return type.get_index() - other_type.get_index();
  }

  // We only call compare_to_impl() if they have the same type.
  return compare_to_impl(&other);
}

/**
 * Returns a suitable hash value for phash_map.
 */
INLINE size_t RenderAttrib::
get_hash() const {
  return _hash;
}

/**
 * Returns the pointer to the unique RenderAttrib in the cache that is
 * equivalent to this one.  This may be the same pointer as this object, or it
 * may be a different pointer; but it will be an equivalent object, and it
 * will be a shared pointer.  This may be called from time to time to improve
 * cache benefits.
 */
INLINE CPT(RenderAttrib) RenderAttrib::
get_unique() const {
  return return_unique((RenderAttrib *)this);
}

/**
 * Calculates a suitable hash value for phash_map.
 */
INLINE void RenderAttrib::
calc_hash() {
  size_t hash = get_hash_impl();

  // The type is also added to the hash.
  _hash = int_hash::add_hash(hash, get_type().get_index());
}

/**
 * Adds the indicated TypeHandle to the registry, if it is not there already,
 * and returns a unique slot number.  See RenderAttribRegistry.
 */
INLINE int RenderAttrib::
register_slot(TypeHandle type_handle, int sort, RenderAttrib *default_attrib) {
  RenderAttribRegistry *reg = RenderAttribRegistry::get_global_ptr();
  return reg->register_slot(type_handle, sort, default_attrib);
}
