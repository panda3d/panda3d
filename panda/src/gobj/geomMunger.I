/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomMunger.I
 * @author drose
 * @date 2005-03-10
 */

/**
 * Returns a pointer to the GSG that created this munger.
 */
INLINE GraphicsStateGuardianBase *GeomMunger::
get_gsg() const {
  return _gsg;
}

/**
 * Returns true if this munger has been registered, false if it has not.  It
 * may not be used for a Geom until it has been registered, but once
 * registered, it may no longer be modified.
 */
INLINE bool GeomMunger::
is_registered() const {
  return _is_registered;
}

/**
 * Adds the indicated munger to the registry, if there is not an equivalent
 * munger already there; in either case, returns the pointer to the equivalent
 * munger now in the registry.
 *
 * This must be called before a munger may be used in a Geom.  After this
 * call, you should discard the original pointer you passed in (which may or
 * may not now be invalid) and let its reference count decrement normally; you
 * should use only the returned value from this point on.
 */
INLINE PT(GeomMunger) GeomMunger::
register_munger(GeomMunger *munger, Thread *current_thread) {
  return get_registry()->register_munger(munger, current_thread);
}

/**
 * Removes all the mungers from the registry that are associated with the
 * indicated GSG.
 */
INLINE void GeomMunger::
unregister_mungers_for_gsg(GraphicsStateGuardianBase *gsg) {
  get_registry()->unregister_mungers_for_gsg(gsg);
}

/**
 * Given a source GeomVertexFormat, converts it if necessary to the
 * appropriate format for rendering.
 *
 * If the GeomVertexAnimationSpec so indicates, then the format will be chosen
 * to convert CPU-based animation tables to HW-based animation tables,
 * reserving space for the specified number of transforms per vertex.
 */
INLINE CPT(GeomVertexFormat) GeomMunger::
munge_format(const GeomVertexFormat *format,
             const GeomVertexAnimationSpec &animation) const {
  // We cast away the const pointer, because do_munge_format() needs to update
  // caches and stuff, but we trust it not to change any user-definable
  // parameters.
  return ((GeomMunger *)this)->do_munge_format(format, animation);
}

/**
 * Given a source GeomVertexData, converts it if necessary to the appropriate
 * data for rendering.
 */
INLINE CPT(GeomVertexData) GeomMunger::
munge_data(const GeomVertexData *data) const {
  // We cast away the const pointer, because do_munge_data() needs to update
  // caches and stuff, but we trust it not to change any user-definable
  // parameters.
  return ((GeomMunger *)this)->munge_data_impl(data);
}

/**
 * This is similar to munge_format(), but it is done at load time, to optimize
 * a model for eventual rendering on a particular GSG.  At this point, we do
 * not necessarily know the final render state that will be applied, so we
 * cannot make any destructive changes to the geom, its data, or its format.
 */
INLINE CPT(GeomVertexFormat) GeomMunger::
premunge_format(const GeomVertexFormat *format) const {
  return ((GeomMunger *)this)->do_premunge_format(format);
}

/**
 * This is similar to munge_data(), but it is done at load time, to optimize a
 * model for eventual rendering on a particular GSG.  At this point, we do not
 * necessarily know the final render state that will be applied, so we cannot
 * make any destructive changes to the geom, its data, or its format.
 */
INLINE CPT(GeomVertexData) GeomMunger::
premunge_data(const GeomVertexData *data) const {
  return ((GeomMunger *)this)->premunge_data_impl(data);
}

/**
 * This is similar to munge_geom(), but it is done at load time, to optimize a
 * model for eventual rendering on a particular GSG.  At this point, we do not
 * necessarily know the final render state that will be applied, so we cannot
 * make any destructive changes to the geom, its data, or its format.
 *
 * Unlike munge_geom(), this result is not cached, since the assumption is
 * that this operation is performed at load time once for each model.
 */
INLINE void GeomMunger::
premunge_geom(CPT(Geom) &geom, CPT(GeomVertexData) &data) const {
  ((GeomMunger *)this)->premunge_geom_impl(geom, data);
}

/**
 * Provides an arbitrary ordering among all unique GeomMungers, so we can
 * store the essentially different ones in a big set and throw away the rest.
 */
INLINE int GeomMunger::
compare_to(const GeomMunger &other) const {
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
 * Compares two GeomMungers, considering only whether they would produce a
 * different answer to munge_format(), munge_data(), or munge_geom().  (They
 * still might be different in other ways, but if they would produce the same
 * answer, this function consider them to be the same.)
 */
INLINE int GeomMunger::
geom_compare_to(const GeomMunger &other) const {
  // First, we compare the types; if they are of different types then they
  // sort differently.
  TypeHandle type = get_type();
  TypeHandle other_type = other.get_type();
  if (type != other_type) {
    return type.get_index() - other_type.get_index();
  }

  // We only call compare_to_impl() if they have the same type.
  return geom_compare_to_impl(&other);
}

/**
 * Unregisters the GeomMunger, for instance when it is being destructed, or
 * whenever it has become invalid for some reason.  This removes it from the
 * registry so that it will no longer be available to be returned by
 * register_munger().
 *
 * It is not an error to call this if the munger has already been
 * unregistered.
 */
INLINE void GeomMunger::
unregister_myself() {
  if (is_registered()) {
    get_registry()->unregister_munger(this);
  }
}

/**
 * Returns the global registry object.
 */
INLINE GeomMunger::Registry *GeomMunger::
get_registry() {
  if (_registry == nullptr) {
    make_registry();
  }
  return _registry;
}
