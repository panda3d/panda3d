/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVUsageAnalysis.I
 * @author rdb
 * @date 2026-07-07
 */

/**
 * Constructs a usage analysis pre-allocated for the given id bound.
 */
INLINE SpirVUsageAnalysis::
SpirVUsageAnalysis(uint32_t id_bound) :
  _flags(id_bound, 0u),
  _origins(id_bound, Id()) {
}

/**
 * Returns the id of the variable or function parameter that the given pointer
 * or image value originates from, chased through loads, access chains, copies
 * and casts.  A variable originates from itself.  Returns 0 if the value does
 * not trace back to one.
 */
INLINE SpirVId SpirVUsageAnalysis::
get_origin(Id id) const {
  return (id < _origins.size()) ? _origins[id] : Id();
}

/**
 * Returns true if the given variable, parameter or function is used.  For a
 * value it is "used" if it is loaded, stored, passed to a function or
 * otherwise consumed.  For a function, it is "used" if it is called at least
 * once.
 */
INLINE bool SpirVUsageAnalysis::
is_used(Id id) const {
  return has_flag(id, UF_used);
}

/**
 * Returns true if a depth-comparison sample traces back to this variable.
 */
INLINE bool SpirVUsageAnalysis::
is_dref_sampled(Id var_id) const {
  return has_flag(var_id, UF_dref_sampled);
}

/**
 * Returns true if a regular (non-depth-comparison) sample traces back to
 * this variable.
 */
INLINE bool SpirVUsageAnalysis::
is_non_dref_sampled(Id var_id) const {
  return has_flag(var_id, UF_non_dref_sampled);
}

/**
 * Returns true if this variable is indexed with a non-constant index.
 */
INLINE bool SpirVUsageAnalysis::
is_dynamically_indexed(Id var_id) const {
  return has_flag(var_id, UF_dynamically_indexed);
}

/**
 * Returns true if the size or level count of an image loaded from this
 * variable was queried.
 */
INLINE bool SpirVUsageAnalysis::
was_size_or_levels_queried(Id var_id) const {
  return has_flag(var_id, UF_queried_size_levels);
}

/**
 * Returns true if the given value was extracted from a sampled image.
 */
INLINE bool SpirVUsageAnalysis::
is_sampled_image_value(Id id) const {
  return has_flag(id, UF_sampled_image);
}

/**
 * Returns true if the given value can be evaluated at compile time.
 */
INLINE bool SpirVUsageAnalysis::
is_constant_expression(Id id) const {
  return has_flag(id, UF_constant_expression);
}

/**
 *
 */
INLINE bool SpirVUsageAnalysis::
has_flag(Id id, uint8_t flag) const {
  return id < _flags.size() && (_flags[id] & flag) != 0;
}

/**
 *
 */
INLINE void SpirVUsageAnalysis::
set_flag(Id id, uint8_t flag) {
  if (id < _flags.size()) {
    _flags[id] |= (uint8_t)flag;
  }
}

/**
 *
 */
INLINE void SpirVUsageAnalysis::
set_origin(Id id, Id origin) {
  if (id < _origins.size()) {
    _origins[id] = origin;
  }
}

/**
 * Marks the variable or parameter the given value originates from as used.
 */
INLINE void SpirVUsageAnalysis::
mark_used(Id id) {
  Id origin = get_origin(id);
  if (origin != 0) {
    set_flag(origin, UF_used);
  }
}

/**
 * Sets the given flag on the variable the given value originates from.
 */
INLINE void SpirVUsageAnalysis::
set_origin_flag(Id id, uint8_t flag) {
  Id origin = get_origin(id);
  if (origin != 0) {
    set_flag(origin, flag);
  }
}
