/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderEffects.I
 * @author drose
 * @date 2002-03-14
 */

/**
 *
 */
INLINE RenderEffects::Effect::
Effect(const RenderEffect *effect) :
  _type(effect->get_type()),
  _effect(effect)
{
}

/**
 * This constructor is only used when reading the RenderEffects from a bam
 * file.  At this point, the effect pointer is unknown.
 */
INLINE RenderEffects::Effect::
Effect() {
}

/**
 * This constructor makes an invalid Effect with no RenderEffect pointer; its
 * purpose is just to make an object we can use to look up a particular type
 * in the Effect set.
 */
INLINE RenderEffects::Effect::
Effect(TypeHandle type) :
  _type(type),
  _effect(nullptr)
{
}

/**
 *
 */
INLINE RenderEffects::Effect::
Effect(const Effect &copy) :
  _type(copy._type),
  _effect(copy._effect)
{
}

/**
 *
 */
INLINE void RenderEffects::Effect::
operator = (const Effect &copy) {
  _type = copy._type;
  _effect = copy._effect;
}

/**
 * This is used by the Effects set to uniquify RenderEffects by type.  Only
 * one RenderEffect of a given type is allowed in the set.  This ordering must
 * also match the ordering reported by compare_to().
 */
INLINE bool RenderEffects::Effect::
operator < (const Effect &other) const {
  return _type < other._type;
}

/**
 * Provides an indication of whether a particular effect is equivalent to
 * another one, for purposes of generating unique RenderEffects.  This should
 * compare all properties of the Effect, but it is important that the type is
 * compared first, to be consistent with the ordering defined by operator <.
 */
INLINE int RenderEffects::Effect::
compare_to(const Effect &other) const {
  if (_type != other._type) {
    return _type.get_index() - other._type.get_index();
  }
  if (_effect != other._effect) {
    return _effect < other._effect ? -1 : 1;
  }
  return 0;
}

/**
 * Returns true if the state is empty, false otherwise.
 */
INLINE bool RenderEffects::
is_empty() const {
  return _effects.empty();
}

/**
 * Returns the number of separate effects indicated in the state.
 * @deprecated in Python, use len(effects) instead, or effects.size() in C++.
 */
INLINE size_t RenderEffects::
get_num_effects() const {
  return _effects.size();
}

/**
 * Returns the nth effect in the state.
 */
INLINE const RenderEffect *RenderEffects::
get_effect(size_t n) const {
  nassertr(n < _effects.size(), nullptr);
  return _effects[n]._effect;
}

/**
 * Returns the number of separate effects indicated in the state.
 */
INLINE size_t RenderEffects::
size() const {
  return _effects.size();
}

/**
 * Returns the nth effect in the state.
 */
INLINE const RenderEffect *RenderEffects::
operator [](size_t n) const {
  nassertr(n < _effects.size(), nullptr);
  return _effects[n]._effect;
}

/**
 * Returns the effect in the state with the given type.
 */
INLINE const RenderEffect *RenderEffects::
operator [](TypeHandle type) const {
  return get_effect(type);
}

/**
 * This function is provided as an optimization, to speed up the render-time
 * checking for the existance of a DecalEffect on this state.  It returns true
 * if a DecalEffect exists, false otherwise.  Note that since there is no
 * additional information stored on the DecalEffect, there's no point in
 * returning it if it exists.
 */
INLINE bool RenderEffects::
has_decal() const {
  if ((_flags & F_checked_decal) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal decal cache.
    ((RenderEffects *)this)->determine_decal();
  }
  return ((_flags & F_has_decal) != 0);
}

/**
 * This function is provided as an optimization, to speed up the render-time
 * checking for the existance of a ShowBoundsEffect on this state.  It returns
 * true if a ShowBoundsEffect exists, false otherwise.  Note that since there
 * is no additional information stored on the ShowBoundsEffect, there's no
 * point in returning it if it exists.
 */
INLINE bool RenderEffects::
has_show_bounds() const {
  if ((_flags & F_checked_show_bounds) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal show_bounds cache.
    ((RenderEffects *)this)->determine_show_bounds();
  }
  return ((_flags & F_has_show_bounds) != 0);
}

/**
 * If has_show_bounds() returns true, this will return true if the
 * ShowBoundsEffect in question requests showing a "tight" bound.
 */
INLINE bool RenderEffects::
has_show_tight_bounds() const {
  if ((_flags & F_checked_show_bounds) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal show_bounds cache.
    ((RenderEffects *)this)->determine_show_bounds();
  }
  return ((_flags & F_has_show_tight_bounds) != 0);
}

/**
 * This function is provided as an optimization, to speed up the render-time
 * checking for the existance of an effect with a cull_callback on this state.
 */
INLINE bool RenderEffects::
has_cull_callback() const {
  if ((_flags & F_checked_cull_callback) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal cull_callback cache.
    ((RenderEffects *)this)->determine_cull_callback();
  }
  return ((_flags & F_has_cull_callback) != 0);
}

/**
 * This function is provided as an optimization, to speed up the render-time
 * checking for the existance of an effect with a compute_adjust_transform on
 * this state.
 */
INLINE bool RenderEffects::
has_adjust_transform() const {
  if ((_flags & F_checked_adjust_transform) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal adjust_transform cache.
    ((RenderEffects *)this)->determine_adjust_transform();
  }
  return ((_flags & F_has_adjust_transform) != 0);
}
