// Filename: renderState.I
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
//     Function: RenderState::get_hash
//       Access: Published
//  Description: Returns a suitable hash value for phash_map.
////////////////////////////////////////////////////////////////////
INLINE size_t RenderState::
get_hash() const {
  check_hash();
  return _hash;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::is_empty
//       Access: Published
//  Description: Returns true if the state is empty, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool RenderState::
is_empty() const {
  return _filled_slots.is_zero();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::has_cull_callback
//       Access: Published
//  Description: Returns true if any of the RenderAttribs in this
//               state request a cull_callback(), false if none of
//               them do.
////////////////////////////////////////////////////////////////////
INLINE bool RenderState::
has_cull_callback() const {
  if ((_flags & F_checked_cull_callback) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal shader cache.
    ((RenderState *)this)->determine_cull_callback();
  }
  return (_flags & F_has_cull_callback) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::make_empty
//       Access: Published, Static
//  Description: Returns a RenderState with no attributes set.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderState) RenderState::
make_empty() {
  // The empty state is asked for so often, we make it a special case
  // and store a pointer forever once we find it the first time.
  if (_empty_state == (RenderState *)NULL) {
    RenderState *state = new RenderState;
    _empty_state = return_unique(state);
  }

  return _empty_state;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::make_full_default
//       Access: Published, Static
//  Description: Returns a RenderState with all possible attributes
//               set to their default value.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderState) RenderState::
make_full_default() {
  // The empty state is asked for so often, we make it a special case
  // and store a pointer forever once we find it the first time.
  if (_full_default_state == (RenderState *)NULL) {
    RenderState *state = new RenderState;
    state->fill_default();
    _full_default_state = return_unique(state);
  }

  return _full_default_state;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::remove_attrib
//       Access: Published
//  Description: Returns a new RenderState object that represents the
//               same as the source state, with the indicated
//               RenderAttrib removed.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderState) RenderState::
remove_attrib(TypeHandle type) const {
  RenderAttribRegistry *reg = RenderAttribRegistry::quick_get_global_ptr();
  int slot = reg->get_slot(type);
  return remove_attrib(slot);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::has_attrib
//       Access: Published
//  Description: Returns true if an attrib of the indicated type is
//               present, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool RenderState::
has_attrib(TypeHandle type) const {
  return get_attrib(type) != (RenderAttrib *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::has_attrib
//       Access: Published
//  Description: Returns true if an attrib of the indicated type is
//               present, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool RenderState::
has_attrib(int slot) const {
  return get_attrib(slot) != (RenderAttrib *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_attrib
//       Access: Published
//  Description: Looks for a RenderAttrib of the indicated type in the
//               state, and returns it if it is found, or NULL if it
//               is not.
////////////////////////////////////////////////////////////////////
INLINE const RenderAttrib *RenderState::
get_attrib(TypeHandle type) const {
  RenderAttribRegistry *reg = RenderAttribRegistry::get_global_ptr();
  int slot = reg->get_slot(type);
  return _attributes[slot]._attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_attrib
//       Access: Published
//  Description: Returns the RenderAttrib with the indicated slot
//               index, or NULL if there is no such RenderAttrib in
//               the state.
////////////////////////////////////////////////////////////////////
INLINE const RenderAttrib *RenderState::
get_attrib(int slot) const {
  return _attributes[slot]._attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_attrib_def
//       Access: Published
//  Description: Returns the RenderAttrib with the indicated slot
//               index, or the default attrib for that slot if there
//               is no such RenderAttrib in the state.
////////////////////////////////////////////////////////////////////
INLINE const RenderAttrib *RenderState::
get_attrib_def(int slot) const {
  if (_attributes[slot]._attrib != (RenderAttrib *)NULL) {
    return _attributes[slot]._attrib;
  }
  return make_full_default()->get_attrib(slot);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_override
//       Access: Published
//  Description: Looks for a RenderAttrib of the indicated type in the
//               state, and returns its override value if it is found,
//               or 0 if it is not.
////////////////////////////////////////////////////////////////////
INLINE int RenderState::
get_override(TypeHandle type) const {
  RenderAttribRegistry *reg = RenderAttribRegistry::get_global_ptr();
  int slot = reg->get_slot(type);
  return _attributes[slot]._override;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_override
//       Access: Published
//  Description: Looks for a RenderAttrib of the indicated type in the
//               state, and returns its override value if it is found,
//               or 0 if it is not.
////////////////////////////////////////////////////////////////////
INLINE int RenderState::
get_override(int slot) const {
  return _attributes[slot]._override;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_unique
//       Access: Published
//  Description: Returns the pointer to the unique RenderState in
//               the cache that is equivalent to this one.  This may
//               be the same pointer as this object, or it may be a
//               different pointer; but it will be an equivalent
//               object, and it will be a shared pointer.  This may be
//               called from time to time to improve cache benefits.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderState) RenderState::
get_unique() const {
  return return_unique((RenderState *)this);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::cache_ref
//       Access: Published
//  Description: Overrides this method to update PStats appropriately.
////////////////////////////////////////////////////////////////////
INLINE void RenderState::
cache_ref() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  NodeCachedReferenceCount::cache_ref();
  consider_update_pstats(old_referenced_bits);
#else  // DO_PSTATS
  NodeCachedReferenceCount::cache_ref();
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::cache_unref
//       Access: Published
//  Description: Overrides this method to update PStats appropriately.
////////////////////////////////////////////////////////////////////
INLINE bool RenderState::
cache_unref() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  bool result = do_cache_unref();
  consider_update_pstats(old_referenced_bits);
  return result;
#else  // DO_PSTATS
  return do_cache_unref();
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::node_ref
//       Access: Published
//  Description: Overrides this method to update PStats appropriately.
////////////////////////////////////////////////////////////////////
INLINE void RenderState::
node_ref() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  NodeCachedReferenceCount::node_ref();
  consider_update_pstats(old_referenced_bits);
#else  // DO_PSTATS
  NodeCachedReferenceCount::node_ref();
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::node_unref
//       Access: Published
//  Description: Overrides this method to update PStats appropriately.
////////////////////////////////////////////////////////////////////
INLINE bool RenderState::
node_unref() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  bool result = do_node_unref();
  consider_update_pstats(old_referenced_bits);
  return result;
#else  // DO_PSTATS
  return do_node_unref();
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_composition_cache_num_entries
//       Access: Published
//  Description: Returns the number of entries in the composition
//               cache for this RenderState.  This is the number of
//               other RenderStates whose composition with this one
//               has been cached.  This number is not useful for any
//               practical reason other than performance analysis.
////////////////////////////////////////////////////////////////////
INLINE int RenderState::
get_composition_cache_num_entries() const {
  LightReMutexHolder holder(*_states_lock);
  return _composition_cache.get_num_entries();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_invert_composition_cache_num_entries
//       Access: Published
//  Description: Returns the number of entries in the
//               invert_composition cache for this RenderState.
//               This is similar to the composition cache, but it
//               records cache entries for the invert_compose()
//               operation.  See get_composition_cache_num_entries().
////////////////////////////////////////////////////////////////////
INLINE int RenderState::
get_invert_composition_cache_num_entries() const {
  LightReMutexHolder holder(*_states_lock);
  return _invert_composition_cache.get_num_entries();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_composition_cache_size
//       Access: Published
//  Description: Returns the number of slots in the composition
//               cache for this RenderState.  You may use this as
//               an upper bound when walking through all of the
//               composition cache results via
//               get_composition_cache_source() or result().
//
//               This has no practical value other than for examining
//               the cache for performance analysis.
////////////////////////////////////////////////////////////////////
INLINE int RenderState::
get_composition_cache_size() const {
  LightReMutexHolder holder(*_states_lock);
  return _composition_cache.get_size();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_composition_cache_source
//       Access: Published
//  Description: Returns the source RenderState of the nth element
//               in the composition cache.  Returns NULL if there
//               doesn't happen to be an entry in the nth element.
//               See get_composition_cache_result().
//
//               This has no practical value other than for examining
//               the cache for performance analysis.
////////////////////////////////////////////////////////////////////
INLINE const RenderState *RenderState::
get_composition_cache_source(int n) const {
  LightReMutexHolder holder(*_states_lock);
  if (!_composition_cache.has_element(n)) {
    return NULL;
  }
  return _composition_cache.get_key(n);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_composition_cache_result
//       Access: Published
//  Description: Returns the result RenderState of the nth element
//               in the composition cache.  Returns NULL if there
//               doesn't happen to be an entry in the nth element.
//
//               In general,
//               a->compose(a->get_composition_cache_source(n)) ==
//               a->get_composition_cache_result(n).
//
//               This has no practical value other than for examining
//               the cache for performance analysis.
////////////////////////////////////////////////////////////////////
INLINE const RenderState *RenderState::
get_composition_cache_result(int n) const {
  LightReMutexHolder holder(*_states_lock);
  if (!_composition_cache.has_element(n)) {
    return NULL;
  }
  return _composition_cache.get_data(n)._result;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_invert_composition_cache_size
//       Access: Published
//  Description: Returns the number of slots in the composition
//               cache for this RenderState.  You may use this as
//               an upper bound when walking through all of the
//               composition cache results via
//               get_invert_composition_cache_source() or result().
//
//               This has no practical value other than for examining
//               the cache for performance analysis.
////////////////////////////////////////////////////////////////////
INLINE int RenderState::
get_invert_composition_cache_size() const {
  LightReMutexHolder holder(*_states_lock);
  return _invert_composition_cache.get_size();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_invert_composition_cache_source
//       Access: Published
//  Description: Returns the source RenderState of the nth element
//               in the invert composition cache.  Returns NULL if
//               there doesn't happen to be an entry in the nth
//               element.  See get_invert_composition_cache_result().
//
//               This has no practical value other than for examining
//               the cache for performance analysis.
////////////////////////////////////////////////////////////////////
INLINE const RenderState *RenderState::
get_invert_composition_cache_source(int n) const {
  LightReMutexHolder holder(*_states_lock);
  if (!_invert_composition_cache.has_element(n)) {
    return NULL;
  }
  return _invert_composition_cache.get_key(n);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_invert_composition_cache_result
//       Access: Published
//  Description: Returns the result RenderState of the nth element
//               in the invert composition cache.  Returns NULL if
//               there doesn't happen to be an entry in the nth
//               element.
//
//               In general,
//               a->invert_compose(a->get_invert_composition_cache_source(n))
//               == a->get_invert_composition_cache_result(n).
//
//               This has no practical value other than for examining
//               the cache for performance analysis.
////////////////////////////////////////////////////////////////////
INLINE const RenderState *RenderState::
get_invert_composition_cache_result(int n) const {
  LightReMutexHolder holder(*_states_lock);
  if (!_invert_composition_cache.has_element(n)) {
    return NULL;
  }
  return _invert_composition_cache.get_data(n)._result;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_draw_order
//       Access: Published
//  Description: Returns the draw order indicated by the
//               CullBinAttrib, if any, associated by this state (or 0
//               if there is no CullBinAttrib).  See get_bin_index().
////////////////////////////////////////////////////////////////////
INLINE int RenderState::
get_draw_order() const {
  if ((_flags & F_checked_bin_index) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal draw_order cache.
    ((RenderState *)this)->determine_bin_index();
  }
  return _draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_bin_index
//       Access: Published
//  Description: Returns the bin index indicated by the CullBinAttrib,
//               if any, associated by this state (or the default bin
//               index if there is no CullBinAttrib).  This function
//               is provided as an optimization for determining this
//               at render time.
////////////////////////////////////////////////////////////////////
INLINE int RenderState::
get_bin_index() const {
  if ((_flags & F_checked_bin_index) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal bin_index cache.
    ((RenderState *)this)->determine_bin_index();
  }
  return _bin_index;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::set_destructing
//       Access: Private
//  Description: This function should only be called from the
//               destructor; it indicates that this RenderState
//               object is beginning destruction.  It is only used as
//               a sanity check, and is only meaningful when NDEBUG is
//               not defined.
////////////////////////////////////////////////////////////////////
INLINE void RenderState::
set_destructing() {
#ifndef NDEBUG
  _flags |= F_is_destructing;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::is_destructing
//       Access: Private
//  Description: Returns true if the RenderState object is
//               currently within its destructor
//               (i.e. set_destructing() has been called).  This is
//               only used as a sanity check, and is only meaningful
//               when NDEBUG is not defined.
////////////////////////////////////////////////////////////////////
INLINE bool RenderState::
is_destructing() const {
#ifndef NDEBUG
  return (_flags & F_is_destructing) != 0;
#else
  return false;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::consider_update_pstats
//       Access: Private
//  Description: Calls update_pstats() if the state of the referenced
//               bits has changed from the indicated value.
////////////////////////////////////////////////////////////////////
INLINE void RenderState::
consider_update_pstats(int old_referenced_bits) const {
#ifdef DO_PSTATS
  int new_referenced_bits = get_referenced_bits();
  if (old_referenced_bits != new_referenced_bits) {
    update_pstats(old_referenced_bits, new_referenced_bits);
  }
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Composition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RenderState::Composition::
Composition() {
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Composition::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RenderState::Composition::
Composition(const RenderState::Composition &copy) :
  _result(copy._result)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Attribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RenderState::Attribute::
Attribute(const RenderAttrib *attrib, int override) :
  _attrib(attrib),
  _override(override)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Attribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RenderState::Attribute::
Attribute(int override) :
  _override(override)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Attribute::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RenderState::Attribute::
Attribute(const Attribute &copy) :
  _attrib(copy._attrib),
  _override(copy._override)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Attribute::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void RenderState::Attribute::
operator = (const Attribute &copy) {
  _attrib = copy._attrib;
  _override = copy._override;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Attribute::compare_to
//       Access: Public
//  Description: Provides an indication of whether a particular
//               attribute is equivalent to another one, for purposes
//               of generating unique RenderStates.  This should
//               compare all properties of the Attribute.
////////////////////////////////////////////////////////////////////
INLINE int RenderState::Attribute::
compare_to(const Attribute &other) const {
  if (_attrib != other._attrib) {
    if (_attrib == NULL) {
      return -1;
    } else if (other._attrib == NULL) {
      return 1;
    }

    int c = _attrib->compare_to(*other._attrib);
    if (c != 0) {
      return c;
    }
  }

  return _override - other._override;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Attribute::set
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void RenderState::Attribute::
set(const RenderAttrib *attrib, int override) {
  _attrib = attrib;
  _override = override;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::flush_level
//       Access: Public, Static
//  Description: Flushes the PStatCollectors used during traversal.
////////////////////////////////////////////////////////////////////
INLINE void RenderState::
flush_level() {
  _node_counter.flush_level();
  _cache_counter.flush_level();
}

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_attrib
//       Access: Public
//  Description: Handy templated version of get_attrib that
//               costs to the right type.  Returns true if the
//               attribute was present, false otherwise.
////////////////////////////////////////////////////////////////////
template<class AttribType>
INLINE bool RenderState::
get_attrib(const AttribType *&attrib) const {
  attrib = (const AttribType *)get_attrib((int)AttribType::get_class_slot());
  return (attrib != (const AttribType *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_attrib_def
//       Access: Public
//  Description: Handy templated version of get_attrib_def that
//               costs to the right type.
////////////////////////////////////////////////////////////////////
template<class AttribType>
INLINE void RenderState::
get_attrib_def(const AttribType *&attrib) const {
  attrib = (const AttribType *)get_attrib_def((int)AttribType::get_class_slot());
}
#endif  // CPPPARSER

////////////////////////////////////////////////////////////////////
//     Function: RenderState::check_hash
//       Access: Private
//  Description: Ensures that we know the hash value.
////////////////////////////////////////////////////////////////////
INLINE void RenderState::
check_hash() const {
  // This pretends to be a const function, even though it's not,
  // because it only updates a transparent cache value.
  if ((_flags & F_hash_known) == 0) {
    ((RenderState *)this)->calc_hash();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::do_cache_unref
//       Access: Private
//  Description: Reimplements
//               CachedTypedWritableReferenceCount::cache_unref().  We
//               do this because we have a non-virtual unref() method.
////////////////////////////////////////////////////////////////////
INLINE bool RenderState::
do_cache_unref() const {
  cache_unref_only();
  return unref();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::do_node_unref
//       Access: Private
//  Description: Reimplements NodeReferenceCount::node_unref().  We do
//               this because we have a non-virtual unref() method.
////////////////////////////////////////////////////////////////////
INLINE bool RenderState::
do_node_unref() const {
  node_unref_only();
  return unref();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::calc_hash
//       Access: Private
//  Description: Computes the hash value.
////////////////////////////////////////////////////////////////////
INLINE void RenderState::
calc_hash() {
  LightMutexHolder holder(_lock);
  do_calc_hash();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::CompositionCycleDescEntry::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RenderState::CompositionCycleDescEntry::
CompositionCycleDescEntry(const RenderState *obj,
                          const RenderState *result,
                          bool inverted) :
  _obj(obj),
  _result(result),
  _inverted(inverted)
{
}

