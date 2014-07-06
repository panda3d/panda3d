// Filename: renderState.cxx
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

#include "renderState.h"
#include "transparencyAttrib.h"
#include "cullBinAttrib.h"
#include "cullBinManager.h"
#include "fogAttrib.h"
#include "clipPlaneAttrib.h"
#include "scissorAttrib.h"
#include "transparencyAttrib.h"
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "textureAttrib.h"
#include "texGenAttrib.h"
#include "shaderAttrib.h"
#include "pStatTimer.h"
#include "config_pgraph.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagramIterator.h"
#include "indent.h"
#include "compareTo.h"
#include "lightReMutexHolder.h"
#include "lightMutexHolder.h"
#include "thread.h"
#include "renderAttribRegistry.h"
#include "py_panda.h"
  
LightReMutex *RenderState::_states_lock = NULL;
RenderState::States *RenderState::_states = NULL;
CPT(RenderState) RenderState::_empty_state;
CPT(RenderState) RenderState::_full_default_state;
UpdateSeq RenderState::_last_cycle_detect;
int RenderState::_garbage_index = 0;

PStatCollector RenderState::_cache_update_pcollector("*:State Cache:Update");
PStatCollector RenderState::_garbage_collect_pcollector("*:State Cache:Garbage Collect");
PStatCollector RenderState::_state_compose_pcollector("*:State Cache:Compose State");
PStatCollector RenderState::_state_invert_pcollector("*:State Cache:Invert State");
PStatCollector RenderState::_node_counter("RenderStates:On nodes");
PStatCollector RenderState::_cache_counter("RenderStates:Cached");
PStatCollector RenderState::_state_break_cycles_pcollector("*:State Cache:Break Cycles");
PStatCollector RenderState::_state_validate_pcollector("*:State Cache:Validate");

CacheStats RenderState::_cache_stats;

TypeHandle RenderState::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: RenderState::Constructor
//       Access: Protected
//  Description: Actually, this could be a private constructor, since
//               no one inherits from RenderState, but gcc gives us a
//               spurious warning if all constructors are private.
////////////////////////////////////////////////////////////////////
RenderState::
RenderState() : 
  _flags(0),
  _auto_shader_state(NULL),
  _lock("RenderState")
{
  // Allocate the _attributes array.
  RenderAttribRegistry *reg = RenderAttribRegistry::get_global_ptr();
  _attributes = (Attribute *)reg->get_array_chain()->allocate(reg->get_max_slots() * sizeof(Attribute), get_class_type());

  // Also make sure each element gets initialized.
  for (int i = 0; i < reg->get_max_slots(); ++i) {
    new(&_attributes[i]) Attribute();
  }

  if (_states == (States *)NULL) {
    init_states();
  }
  _saved_entry = -1;
  _last_mi = _mungers.end();
  _cache_stats.add_num_states(1);
  _read_overrides = NULL;
  _generated_shader = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Copy Constructor
//       Access: Private
//  Description: RenderStates are only meant to be copied internally.
////////////////////////////////////////////////////////////////////
RenderState::
RenderState(const RenderState &copy) :
  _filled_slots(copy._filled_slots),
  _flags(0),
  _auto_shader_state(NULL),
  _lock("RenderState")
{
  // Allocate the _attributes array.
  RenderAttribRegistry *reg = RenderAttribRegistry::get_global_ptr();
  _attributes = (Attribute *)reg->get_array_chain()->allocate(reg->get_max_slots() * sizeof(Attribute), get_class_type());

  // Also make sure each element gets initialized, as a copy.
  for (int i = 0; i < reg->get_max_slots(); ++i) {
    new(&_attributes[i]) Attribute(copy._attributes[i]);
  }

  _saved_entry = -1;
  _last_mi = _mungers.end();
  _cache_stats.add_num_states(1);
  _read_overrides = NULL;
  _generated_shader = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Copy Assignment Operator
//       Access: Private
//  Description: RenderStates are not meant to be copied.
////////////////////////////////////////////////////////////////////
void RenderState::
operator = (const RenderState &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Destructor
//       Access: Public, Virtual
//  Description: The destructor is responsible for removing the
//               RenderState from the global set if it is there.
////////////////////////////////////////////////////////////////////
RenderState::
~RenderState() {
  // We'd better not call the destructor twice on a particular object.
  nassertv(!is_destructing());
  set_destructing();

  LightReMutexHolder holder(*_states_lock);

  // unref() should have cleared these.
  nassertv(_saved_entry == -1);
  nassertv(_composition_cache.is_empty() && _invert_composition_cache.is_empty());

  // Make sure the _auto_shader_state cache pointer is cleared.
  if (_auto_shader_state != (const RenderState *)NULL) {
    if (_auto_shader_state != this) {
      cache_unref_delete(_auto_shader_state);
    }
    _auto_shader_state = NULL;
  }

  // If this was true at the beginning of the destructor, but is no
  // longer true now, probably we've been double-deleted.
  nassertv(get_ref_count() == 0);
  _cache_stats.add_num_states(-1);

  // Free the _attributes array.
  RenderAttribRegistry *reg = RenderAttribRegistry::get_global_ptr();
  for (int i = 0; i < reg->get_max_slots(); ++i) {
    _attributes[i].~Attribute();
  }
  reg->get_array_chain()->deallocate(_attributes, get_class_type());
  _attributes = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::compare_to
//       Access: Published
//  Description: Provides an arbitrary ordering among all unique
//               RenderStates, so we can store the essentially
//               different ones in a big set and throw away the rest.
//
//               This method is not needed outside of the RenderState
//               class because all equivalent RenderState objects are
//               guaranteed to share the same pointer; thus, a pointer
//               comparison is always sufficient.
////////////////////////////////////////////////////////////////////
int RenderState::
compare_to(const RenderState &other) const {
  SlotMask mask = _filled_slots | other._filled_slots;
  int slot = mask.get_lowest_on_bit();
  while (slot >= 0) {
    int result = _attributes[slot].compare_to(other._attributes[slot]);
    if (result != 0) {
      return result;
    }
    mask.clear_bit(slot);
    slot = mask.get_lowest_on_bit();
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::compare_sort
//       Access: Published
//  Description: Returns -1, 0, or 1 according to the relative sorting
//               of these two RenderStates, with regards to rendering
//               performance, so that "heavier" RenderAttribs (as
//               defined by RenderAttribRegistry::get_slot_sort()) are
//               more likely to be grouped together.  This is not
//               related to the sorting order defined by compare_to.
////////////////////////////////////////////////////////////////////
int RenderState::
compare_sort(const RenderState &other) const {
  if (this == &other) {
    // Trivial case.
    return 0;
  }

  RenderAttribRegistry *reg = RenderAttribRegistry::quick_get_global_ptr();
  int num_sorted_slots = reg->get_num_sorted_slots();
  for (int n = 0; n < num_sorted_slots; ++n) {
    int slot = reg->get_sorted_slot(n);
    nassertr((_attributes[slot]._attrib != NULL) == _filled_slots.get_bit(slot), 0);

    const RenderAttrib *a = _attributes[slot]._attrib;
    const RenderAttrib *b = other._attributes[slot]._attrib;
    if (a != b) {
      return a < b ? -1 : 1;
    }
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::cull_callback
//       Access: Published
//  Description: Calls cull_callback() on each attrib.  If any attrib
//               returns false, interrupts the list and returns false
//               immediately; otherwise, completes the list and
//               returns true.
////////////////////////////////////////////////////////////////////
bool RenderState::
cull_callback(CullTraverser *trav, const CullTraverserData &data) const {
  SlotMask mask = _filled_slots;
  int slot = mask.get_lowest_on_bit();
  while (slot >= 0) {
    const Attribute &attrib = _attributes[slot];
    nassertr(attrib._attrib != NULL, false);
    if (!attrib._attrib->cull_callback(trav, data)) {
      return false;
    }

    mask.clear_bit(slot);
    slot = mask.get_lowest_on_bit();
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::make_empty
//       Access: Published, Static
//  Description: Returns a RenderState with no attributes set.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
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
CPT(RenderState) RenderState::
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
//     Function: RenderState::make
//       Access: Published, Static
//  Description: Returns a RenderState with one attribute set.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
make(const RenderAttrib *attrib, int override) {
  RenderState *state = new RenderState;
  int slot = attrib->get_slot();
  state->_attributes[slot].set(attrib, override);
  state->_filled_slots.set_bit(slot);
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::make
//       Access: Published, Static
//  Description: Returns a RenderState with two attributes set.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
make(const RenderAttrib *attrib1,
     const RenderAttrib *attrib2, int override) {
  RenderState *state = new RenderState;
  state->_attributes[attrib1->get_slot()].set(attrib1, override);
  state->_attributes[attrib2->get_slot()].set(attrib2, override);
  state->_filled_slots.set_bit(attrib1->get_slot());
  state->_filled_slots.set_bit(attrib2->get_slot());
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::make
//       Access: Published, Static
//  Description: Returns a RenderState with three attributes set.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
make(const RenderAttrib *attrib1,
     const RenderAttrib *attrib2,
     const RenderAttrib *attrib3, int override) {
  RenderState *state = new RenderState;
  state->_attributes[attrib1->get_slot()].set(attrib1, override);
  state->_attributes[attrib2->get_slot()].set(attrib2, override);
  state->_attributes[attrib3->get_slot()].set(attrib3, override);
  state->_filled_slots.set_bit(attrib1->get_slot());
  state->_filled_slots.set_bit(attrib2->get_slot());
  state->_filled_slots.set_bit(attrib3->get_slot());
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::make
//       Access: Published, Static
//  Description: Returns a RenderState with four attributes set.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
make(const RenderAttrib *attrib1,
     const RenderAttrib *attrib2,
     const RenderAttrib *attrib3,
     const RenderAttrib *attrib4, int override) {
  RenderState *state = new RenderState;
  state->_attributes[attrib1->get_slot()].set(attrib1, override);
  state->_attributes[attrib2->get_slot()].set(attrib2, override);
  state->_attributes[attrib3->get_slot()].set(attrib3, override);
  state->_attributes[attrib4->get_slot()].set(attrib4, override);
  state->_filled_slots.set_bit(attrib1->get_slot());
  state->_filled_slots.set_bit(attrib2->get_slot());
  state->_filled_slots.set_bit(attrib3->get_slot());
  state->_filled_slots.set_bit(attrib4->get_slot());
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::make
//       Access: Published, Static
//  Description: Returns a RenderState with n attributes set.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
make(const RenderAttrib * const *attrib, int num_attribs, int override) {
  if (num_attribs == 0) {
    return make_empty();
  }
  RenderState *state = new RenderState;
  for (int i = 0; i < num_attribs; i++) {
    int slot = attrib[i]->get_slot();
    state->_attributes[slot].set(attrib[i], override);
    state->_filled_slots.set_bit(slot);
  }
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::compose
//       Access: Published
//  Description: Returns a new RenderState object that represents the
//               composition of this state with the other state.
//
//               The result of this operation is cached, and will be
//               retained as long as both this RenderState object and
//               the other RenderState object continue to exist.
//               Should one of them destruct, the cached entry will be
//               removed, and its pointer will be allowed to destruct
//               as well.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
compose(const RenderState *other) const {
  // This method isn't strictly const, because it updates the cache,
  // but we pretend that it is because it's only a cache which is
  // transparent to the rest of the interface.

  // We handle empty state (identity) as a trivial special case.
  if (is_empty()) {
    return other;
  }
  if (other->is_empty()) {
    return this;
  }

  if (!state_cache) {
    return do_compose(other);
  }

  LightReMutexHolder holder(*_states_lock);

  // Is this composition already cached?
  int index = _composition_cache.find(other);
  if (index != -1) {
    Composition &comp = ((RenderState *)this)->_composition_cache.modify_data(index);
    if (comp._result == (const RenderState *)NULL) {
      // Well, it wasn't cached already, but we already had an entry
      // (probably created for the reverse direction), so use the same
      // entry to store the new result.
      CPT(RenderState) result = do_compose(other);
      comp._result = result;

      if (result != (const RenderState *)this) {
        // See the comments below about the need to up the reference
        // count only when the result is not the same as this.
        result->cache_ref();
      }
    }
    // Here's the cache!
    _cache_stats.inc_hits();
    return comp._result;
  }
  _cache_stats.inc_misses();

  // We need to make a new cache entry, both in this object and in the
  // other object.  We make both records so the other RenderState
  // object will know to delete the entry from this object when it
  // destructs, and vice-versa.

  // The cache entry in this object is the only one that indicates the
  // result; the other will be NULL for now.
  CPT(RenderState) result = do_compose(other);

  _cache_stats.add_total_size(1);
  _cache_stats.inc_adds(_composition_cache.get_size() == 0);

  ((RenderState *)this)->_composition_cache[other]._result = result;

  if (other != this) {
    _cache_stats.add_total_size(1);
    _cache_stats.inc_adds(other->_composition_cache.get_size() == 0);
    ((RenderState *)other)->_composition_cache[this]._result = NULL;
  }

  if (result != (const RenderState *)this) {
    // If the result of compose() is something other than this,
    // explicitly increment the reference count.  We have to be sure
    // to decrement it again later, when the composition entry is
    // removed from the cache.
    result->cache_ref();
    
    // (If the result was just this again, we still store the
    // result, but we don't increment the reference count, since
    // that would be a self-referential leak.)
  }

  _cache_stats.maybe_report("RenderState");

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::invert_compose
//       Access: Published
//  Description: Returns a new RenderState object that represents the
//               composition of this state's inverse with the other
//               state.
//
//               This is similar to compose(), but is particularly
//               useful for computing the relative state of a node as
//               viewed from some other node.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
invert_compose(const RenderState *other) const {
  // This method isn't strictly const, because it updates the cache,
  // but we pretend that it is because it's only a cache which is
  // transparent to the rest of the interface.

  // We handle empty state (identity) as a trivial special case.
  if (is_empty()) {
    return other;
  }
  // Unlike compose(), the case of other->is_empty() is not quite as
  // trivial for invert_compose().

  if (other == this) {
    // a->invert_compose(a) always produces identity.
    return make_empty();
  }

  if (!state_cache) {
    return do_invert_compose(other);
  }

  LightReMutexHolder holder(*_states_lock);

  // Is this composition already cached?
  int index = _invert_composition_cache.find(other);
  if (index != -1) {
    Composition &comp = ((RenderState *)this)->_invert_composition_cache.modify_data(index);
    if (comp._result == (const RenderState *)NULL) {
      // Well, it wasn't cached already, but we already had an entry
      // (probably created for the reverse direction), so use the same
      // entry to store the new result.
      CPT(RenderState) result = do_invert_compose(other);
      comp._result = result;

      if (result != (const RenderState *)this) {
        // See the comments below about the need to up the reference
        // count only when the result is not the same as this.
        result->cache_ref();
      }
    }
    // Here's the cache!
    _cache_stats.inc_hits();
    return comp._result;
  }
  _cache_stats.inc_misses();

  // We need to make a new cache entry, both in this object and in the
  // other object.  We make both records so the other RenderState
  // object will know to delete the entry from this object when it
  // destructs, and vice-versa.

  // The cache entry in this object is the only one that indicates the
  // result; the other will be NULL for now.
  CPT(RenderState) result = do_invert_compose(other);

  _cache_stats.add_total_size(1);
  _cache_stats.inc_adds(_invert_composition_cache.get_size() == 0);
  ((RenderState *)this)->_invert_composition_cache[other]._result = result;

  if (other != this) {
    _cache_stats.add_total_size(1);
    _cache_stats.inc_adds(other->_invert_composition_cache.get_size() == 0);
    ((RenderState *)other)->_invert_composition_cache[this]._result = NULL;
  }

  if (result != (const RenderState *)this) {
    // If the result of compose() is something other than this,
    // explicitly increment the reference count.  We have to be sure
    // to decrement it again later, when the composition entry is
    // removed from the cache.
    result->cache_ref();
    
    // (If the result was just this again, we still store the
    // result, but we don't increment the reference count, since
    // that would be a self-referential leak.)
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::add_attrib
//       Access: Published
//  Description: Returns a new RenderState object that represents the
//               same as the source state, with the new RenderAttrib
//               added.  If there is already a RenderAttrib with the
//               same type, it is replaced (unless the override is
//               lower).
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
add_attrib(const RenderAttrib *attrib, int override) const {
  int slot = attrib->get_slot();
  if (_filled_slots.get_bit(slot) &&
      _attributes[slot]._override > override) {
    // The existing attribute overrides.
    return this;
  }

  // The new attribute replaces.
  RenderState *new_state = new RenderState(*this);
  new_state->_attributes[slot].set(attrib, override);
  new_state->_filled_slots.set_bit(slot);
  return return_new(new_state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::set_attrib
//       Access: Published
//  Description: Returns a new RenderState object that represents the
//               same as the source state, with the new RenderAttrib
//               added.  If there is already a RenderAttrib with the
//               same type, it is replaced unconditionally.  The
//               override is not changed.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
set_attrib(const RenderAttrib *attrib) const {
  RenderState *new_state = new RenderState(*this);
  int slot = attrib->get_slot();
  new_state->_attributes[slot]._attrib = attrib;
  new_state->_filled_slots.set_bit(slot);
  return return_new(new_state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::set_attrib
//       Access: Published
//  Description: Returns a new RenderState object that represents the
//               same as the source state, with the new RenderAttrib
//               added.  If there is already a RenderAttrib with the
//               same type, it is replaced unconditionally.  The
//               override is also replaced unconditionally.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
set_attrib(const RenderAttrib *attrib, int override) const {
  RenderState *new_state = new RenderState(*this);
  int slot = attrib->get_slot();
  new_state->_attributes[slot].set(attrib, override);
  new_state->_filled_slots.set_bit(slot);
  return return_new(new_state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::remove_attrib
//       Access: Published
//  Description: Returns a new RenderState object that represents the
//               same as the source state, with the indicated
//               RenderAttrib removed.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
remove_attrib(int slot) const {
  if (_attributes[slot]._attrib == NULL) {
    // Already removed.
    return this;
  }

  // Will this bring us down to the empty state?
  if (_filled_slots.get_num_on_bits() == 1) {
    return make_empty();
  }

  RenderState *new_state = new RenderState(*this);
  new_state->_attributes[slot].set(NULL, 0);
  new_state->_filled_slots.clear_bit(slot);
  return return_new(new_state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::adjust_all_priorities
//       Access: Published
//  Description: Returns a new RenderState object that represents the
//               same as the source state, with all attributes'
//               override values incremented (or decremented, if
//               negative) by the indicated amount.  If the override
//               would drop below zero, it is set to zero.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
adjust_all_priorities(int adjustment) const {
  RenderState *new_state = new RenderState(*this);

  SlotMask mask = _filled_slots;
  int slot = mask.get_lowest_on_bit();
  while (slot >= 0) {
    Attribute &attrib = new_state->_attributes[slot];
    nassertr(attrib._attrib != (RenderAttrib *)NULL, this);
    attrib._override = max(attrib._override + adjustment, 0);

    mask.clear_bit(slot);
    slot = mask.get_lowest_on_bit();
  }

  return return_new(new_state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::unref
//       Access: Published, Virtual
//  Description: This method overrides ReferenceCount::unref() to
//               check whether the remaining reference count is
//               entirely in the cache, and if so, it checks for and
//               breaks a cycle in the cache involving this object.
//               This is designed to prevent leaks from cyclical
//               references within the cache.
////////////////////////////////////////////////////////////////////
bool RenderState::
unref() const {
  if (!state_cache || garbage_collect_states) {
    // If we're not using the cache at all, or if we're relying on
    // garbage collection, just allow the pointer to unref normally.
    return ReferenceCount::unref();
  }

  // Here is the normal refcounting case, with a normal cache, and
  // without garbage collection in effect.  In this case we will pull
  // the object out of the cache when its reference count goes to 0.

  // We always have to grab the lock, since we will definitely need to
  // be holding it if we happen to drop the reference count to 0.
  // Having to grab the lock at every call to unref() is a big
  // limiting factor on parallelization.
  LightReMutexHolder holder(*_states_lock);

  if (auto_break_cycles && uniquify_states) {
    if (get_cache_ref_count() > 0 &&
        get_ref_count() == get_cache_ref_count() + 1) {
      // If we are about to remove the one reference that is not in the
      // cache, leaving only references in the cache, then we need to
      // check for a cycle involving this RenderState and break it if
      // it exists.
      ((RenderState *)this)->detect_and_break_cycles();
    }
  }

  if (ReferenceCount::unref()) {
    // The reference count is still nonzero.
    return true;
  }

  // The reference count has just reached zero.  Make sure the object
  // is removed from the global object pool, before anyone else finds
  // it and tries to ref it.
  ((RenderState *)this)->release_new();
  ((RenderState *)this)->remove_cache_pointers();

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_auto_shader_state
//       Access: Published
//  Description: Returns the base RenderState that should have the
//               generated_shader stored within it, for generated
//               shader states.  The returned object might be the same
//               as this object, or it might be a different
//               RenderState with certain attributes removed, or set
//               to their default values.
//
//               The point is to avoid needless regeneration of the
//               shader attrib by storing the generated shader on a
//               common RenderState object, with all irrelevant
//               attributes removed.
////////////////////////////////////////////////////////////////////
const RenderState *RenderState::
get_auto_shader_state() const {
  if (_auto_shader_state == (const RenderState *)NULL) {
    ((RenderState *)this)->assign_auto_shader_state();
  }
  return _auto_shader_state;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void RenderState::
output(ostream &out) const {
  out << "S:";
  if (is_empty()) {
    out << "(empty)";

  } else {
    out << "(";
    const char *sep = "";

    SlotMask mask = _filled_slots;
    int slot = mask.get_lowest_on_bit();
    while (slot >= 0) {
      const Attribute &attrib = _attributes[slot];
      nassertv(attrib._attrib != (RenderAttrib *)NULL);
      out << sep << attrib._attrib->get_type();
      sep = " ";

      mask.clear_bit(slot);
      slot = mask.get_lowest_on_bit();
    }
    out << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void RenderState::
write(ostream &out, int indent_level) const {
  if (is_empty()) {
    indent(out, indent_level) 
      << "(empty)\n";
  }

  SlotMask mask = _filled_slots;
  int slot = mask.get_lowest_on_bit();
  while (slot >= 0) {
    const Attribute &attrib = _attributes[slot];
    nassertv(attrib._attrib != (RenderAttrib *)NULL);
    attrib._attrib->write(out, indent_level);
    
    mask.clear_bit(slot);
    slot = mask.get_lowest_on_bit();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_max_priority
//       Access: Published, Static
//  Description: Returns the maximum priority number (sometimes called
//               override) that may be set on any node.  This may or
//               may not be enforced, but the scene graph code assumes
//               that no priority numbers will be larger than this,
//               and some effects may not work properly if you use a
//               larger number.
////////////////////////////////////////////////////////////////////
int RenderState::
get_max_priority() {
  return 1000000000;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_num_states
//       Access: Published, Static
//  Description: Returns the total number of unique RenderState
//               objects allocated in the world.  This will go up and
//               down during normal operations.
////////////////////////////////////////////////////////////////////
int RenderState::
get_num_states() {
  if (_states == (States *)NULL) {
    return 0;
  }
  LightReMutexHolder holder(*_states_lock);
  return _states->get_num_entries();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_num_unused_states
//       Access: Published, Static
//  Description: Returns the total number of RenderState objects that
//               have been allocated but have no references outside of
//               the internal RenderState cache.
//
//               A nonzero return value is not necessarily indicative
//               of leaked references; it is normal for two
//               RenderState objects, both of which have references
//               held outside the cache, to have to result of their
//               composition stored within the cache.  This result
//               will be retained within the cache until one of the
//               base RenderStates is released.
//
//               Use list_cycles() to get an idea of the number of
//               actual "leaked" RenderState objects.
////////////////////////////////////////////////////////////////////
int RenderState::
get_num_unused_states() {
  if (_states == (States *)NULL) {
    return 0;
  }
  LightReMutexHolder holder(*_states_lock);

  // First, we need to count the number of times each RenderState
  // object is recorded in the cache.
  typedef pmap<const RenderState *, int> StateCount;
  StateCount state_count;

  int size = _states->get_size();
  for (int si = 0; si < size; ++si) {
    if (!_states->has_element(si)) {
      continue;
    }
    const RenderState *state = _states->get_key(si);

    int i;
    int cache_size = state->_composition_cache.get_size();
    for (i = 0; i < cache_size; ++i) {
      if (state->_composition_cache.has_element(i)) {
        const RenderState *result = state->_composition_cache.get_data(i)._result;
        if (result != (const RenderState *)NULL && result != state) {
          // Here's a RenderState that's recorded in the cache.
          // Count it.
          pair<StateCount::iterator, bool> ir =
            state_count.insert(StateCount::value_type(result, 1));
          if (!ir.second) {
            // If the above insert operation fails, then it's already in
            // the cache; increment its value.
            (*(ir.first)).second++;
          }
        }
      }
    }
    cache_size = state->_invert_composition_cache.get_size();
    for (i = 0; i < cache_size; ++i) {
      if (state->_invert_composition_cache.has_element(i)) {
        const RenderState *result = state->_invert_composition_cache.get_data(i)._result;
        if (result != (const RenderState *)NULL && result != state) {
          pair<StateCount::iterator, bool> ir =
            state_count.insert(StateCount::value_type(result, 1));
          if (!ir.second) {
            (*(ir.first)).second++;
          }
        }
      }
    }
  }

  // Now that we have the appearance count of each RenderState
  // object, we can tell which ones are unreferenced outside of the
  // RenderState cache, by comparing these to the reference counts.
  int num_unused = 0;

  StateCount::iterator sci;
  for (sci = state_count.begin(); sci != state_count.end(); ++sci) {
    const RenderState *state = (*sci).first;
    int count = (*sci).second;
    nassertr(count == state->get_cache_ref_count(), num_unused);
    nassertr(count <= state->get_ref_count(), num_unused);
    if (count == state->get_ref_count()) {
      num_unused++;

      if (pgraph_cat.is_debug()) {
        pgraph_cat.debug()
          << "Unused state: " << (void *)state << ":" 
          << state->get_ref_count() << " =\n";
        state->write(pgraph_cat.debug(false), 2);
      }
    }
  }

  return num_unused;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::clear_cache
//       Access: Published, Static
//  Description: Empties the cache of composed RenderStates.  This
//               makes every RenderState forget what results when
//               it is composed with other RenderStates.
//
//               This will eliminate any RenderState objects that
//               have been allocated but have no references outside of
//               the internal RenderState map.  It will not
//               eliminate RenderState objects that are still in
//               use.
//
//               Nowadays, this method should not be necessary, as
//               reference-count cycles in the composition cache
//               should be automatically detected and broken.
//
//               The return value is the number of RenderStates
//               freed by this operation.
////////////////////////////////////////////////////////////////////
int RenderState::
clear_cache() {
  if (_states == (States *)NULL) {
    return 0;
  }
  LightReMutexHolder holder(*_states_lock);

  PStatTimer timer(_cache_update_pcollector);
  int orig_size = _states->get_num_entries();

  // First, we need to copy the entire set of states to a temporary
  // vector, reference-counting each object.  That way we can walk
  // through the copy, without fear of dereferencing (and deleting)
  // the objects in the map as we go.
  {
    typedef pvector< CPT(RenderState) > TempStates;
    TempStates temp_states;
    temp_states.reserve(orig_size);

    int size = _states->get_size();
    for (int si = 0; si < size; ++si) {
      if (!_states->has_element(si)) {
        continue;
      }
      const RenderState *state = _states->get_key(si);
      temp_states.push_back(state);
    }

    // Now it's safe to walk through the list, destroying the cache
    // within each object as we go.  Nothing will be destructed till
    // we're done.
    TempStates::iterator ti;
    for (ti = temp_states.begin(); ti != temp_states.end(); ++ti) {
      RenderState *state = (RenderState *)(*ti).p();

      int i;
      int cache_size = state->_composition_cache.get_size();
      for (i = 0; i < cache_size; ++i) {
        if (state->_composition_cache.has_element(i)) {
          const RenderState *result = state->_composition_cache.get_data(i)._result;
          if (result != (const RenderState *)NULL && result != state) {
            result->cache_unref();
            nassertr(result->get_ref_count() > 0, 0);
          }
        }
      }
      _cache_stats.add_total_size(-state->_composition_cache.get_num_entries());
      state->_composition_cache.clear();

      cache_size = state->_invert_composition_cache.get_size();
      for (i = 0; i < cache_size; ++i) {
        if (state->_invert_composition_cache.has_element(i)) {
          const RenderState *result = state->_invert_composition_cache.get_data(i)._result;
          if (result != (const RenderState *)NULL && result != state) {
            result->cache_unref();
            nassertr(result->get_ref_count() > 0, 0);
          }
        }
      }
      _cache_stats.add_total_size(-state->_invert_composition_cache.get_num_entries());
      state->_invert_composition_cache.clear();
    }

    // Once this block closes and the temp_states object goes away,
    // all the destruction will begin.  Anything whose reference was
    // held only within the various objects' caches will go away.
  }

  int new_size = _states->get_num_entries();
  return orig_size - new_size;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::garbage_collect
//       Access: Published, Static
//  Description: Performs a garbage-collection cycle.  This must be
//               called periodically if garbage-collect-states is true
//               to ensure that RenderStates get cleaned up
//               appropriately.  It does no harm to call it even if
//               this variable is not true, but there is probably no
//               advantage in that case.
//
//               This automatically calls
//               RenderAttrib::garbage_collect() as well.
////////////////////////////////////////////////////////////////////
int RenderState::
garbage_collect() {
  int num_attribs = RenderAttrib::garbage_collect();

  if (_states == (States *)NULL || !garbage_collect_states) {
    return num_attribs;
  }
  LightReMutexHolder holder(*_states_lock);

  PStatTimer timer(_garbage_collect_pcollector);
  int orig_size = _states->get_num_entries();

  // How many elements to process this pass?
  int size = _states->get_size();
  int num_this_pass = int(size * garbage_collect_states_rate);
  if (num_this_pass <= 0) {
    return num_attribs;
  }
  num_this_pass = min(num_this_pass, size);
  int stop_at_element = (_garbage_index + num_this_pass) % size;
  
  int num_elements = 0;
  int si = _garbage_index;
  do {
    if (_states->has_element(si)) {
      ++num_elements;
      RenderState *state = (RenderState *)_states->get_key(si);
      if (auto_break_cycles && uniquify_states) {
        if (state->get_cache_ref_count() > 0 &&
            state->get_ref_count() == state->get_cache_ref_count()) {
          // If we have removed all the references to this state not in
          // the cache, leaving only references in the cache, then we
          // need to check for a cycle involving this RenderState and
          // break it if it exists.
          state->detect_and_break_cycles();
        }
      }

      if (state->get_ref_count() == 1) {
        // This state has recently been unreffed to 1 (the one we
        // added when we stored it in the cache).  Now it's time to
        // delete it.  This is safe, because we're holding the
        // _states_lock, so it's not possible for some other thread to
        // find the state in the cache and ref it while we're doing
        // this.
        state->release_new();
        state->remove_cache_pointers();
        state->cache_unref();
        delete state;
      }
    }      

    si = (si + 1) % size;
  } while (si != stop_at_element);
  _garbage_index = si;
  nassertr(_states->validate(), 0);

  int new_size = _states->get_num_entries();
  return orig_size - new_size + num_attribs;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::clear_munger_cache
//       Access: Published, Static
//  Description: Completely empties the cache of state + gsg ->
//               munger, for all states and all gsg's.  Normally there
//               is no need to empty this cache.
////////////////////////////////////////////////////////////////////
void RenderState::
clear_munger_cache() {
  LightReMutexHolder holder(*_states_lock);

  int size = _states->get_size();
  for (int si = 0; si < size; ++si) {
    if (!_states->has_element(si)) {
      continue;
    }
    RenderState *state = (RenderState *)(_states->get_key(si));
    state->_mungers.clear();
    state->_last_mi = state->_mungers.end();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::list_cycles
//       Access: Published, Static
//  Description: Detects all of the reference-count cycles in the
//               cache and reports them to standard output.
//
//               These cycles may be inadvertently created when state
//               compositions cycle back to a starting point.
//               Nowadays, these cycles should be automatically
//               detected and broken, so this method should never list
//               any cycles unless there is a bug in that detection
//               logic.
//
//               The cycles listed here are not leaks in the strictest
//               sense of the word, since they can be reclaimed by a
//               call to clear_cache(); but they will not be reclaimed
//               automatically.
////////////////////////////////////////////////////////////////////
void RenderState::
list_cycles(ostream &out) {
  if (_states == (States *)NULL) {
    return;
  }
  LightReMutexHolder holder(*_states_lock);

  typedef pset<const RenderState *> VisitedStates;
  VisitedStates visited;
  CompositionCycleDesc cycle_desc;

  int size = _states->get_size();
  for (int si = 0; si < size; ++si) {
    if (!_states->has_element(si)) {
      continue;
    }
    const RenderState *state = _states->get_key(si);

    bool inserted = visited.insert(state).second;
    if (inserted) {
      ++_last_cycle_detect;
      if (r_detect_cycles(state, state, 1, _last_cycle_detect, &cycle_desc)) {
        // This state begins a cycle.
        CompositionCycleDesc::reverse_iterator csi;

        out << "\nCycle detected of length " << cycle_desc.size() + 1 << ":\n"
            << "state " << (void *)state << ":" << state->get_ref_count()
            << " =\n";
        state->write(out, 2);
        for (csi = cycle_desc.rbegin(); csi != cycle_desc.rend(); ++csi) {
          const CompositionCycleDescEntry &entry = (*csi);
          if (entry._inverted) {
            out << "invert composed with ";
          } else {
            out << "composed with ";
          }
          out << (const void *)entry._obj << ":" << entry._obj->get_ref_count()
              << " " << *entry._obj << "\n"
              << "produces " << (const void *)entry._result << ":"
              << entry._result->get_ref_count() << " =\n";
          entry._result->write(out, 2);
          visited.insert(entry._result);
        }

        cycle_desc.clear();
      } else {
        ++_last_cycle_detect;
        if (r_detect_reverse_cycles(state, state, 1, _last_cycle_detect, &cycle_desc)) {
          // This state begins a cycle.
          CompositionCycleDesc::iterator csi;
          
          out << "\nReverse cycle detected of length " << cycle_desc.size() + 1 << ":\n"
              << "state ";
          for (csi = cycle_desc.begin(); csi != cycle_desc.end(); ++csi) {
            const CompositionCycleDescEntry &entry = (*csi);
            out << (const void *)entry._result << ":"
                << entry._result->get_ref_count() << " =\n";
            entry._result->write(out, 2);
            out << (const void *)entry._obj << ":"
                << entry._obj->get_ref_count() << " =\n";
            entry._obj->write(out, 2);
            visited.insert(entry._result);
          }
          out << (void *)state << ":"
              << state->get_ref_count() << " =\n";
          state->write(out, 2);
          
          cycle_desc.clear();
        }
      }
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: RenderState::list_states
//       Access: Published, Static
//  Description: Lists all of the RenderStates in the cache to the
//               output stream, one per line.  This can be quite a lot
//               of output if the cache is large, so be prepared.
////////////////////////////////////////////////////////////////////
void RenderState::
list_states(ostream &out) {
  if (_states == (States *)NULL) {
    out << "0 states:\n";
    return;
  }
  LightReMutexHolder holder(*_states_lock);

  out << _states->get_num_entries() << " states:\n";

  int size = _states->get_size();
  for (int si = 0; si < size; ++si) {
    if (!_states->has_element(si)) {
      continue;
    }
    const RenderState *state = _states->get_key(si);
    state->write(out, 2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::validate_states
//       Access: Published, Static
//  Description: Ensures that the cache is still stored in sorted
//               order, and that none of the cache elements have been
//               inadvertently deleted.  Returns true if so, false if
//               there is a problem (which implies someone has
//               modified one of the supposedly-const RenderState
//               objects).
////////////////////////////////////////////////////////////////////
bool RenderState::
validate_states() {
  if (_states == (States *)NULL) {
    return true;
  }

  PStatTimer timer(_state_validate_pcollector);

  LightReMutexHolder holder(*_states_lock);
  if (_states->is_empty()) {
    return true;
  }

  if (!_states->validate()) {
    pgraph_cat.error()
      << "RenderState::_states cache is invalid!\n";
    return false;
  }    

  int size = _states->get_size();
  int si = 0;
  while (si < size && !_states->has_element(si)) {
    ++si;
  }
  nassertr(si < size, false);
  nassertr(_states->get_key(si)->get_ref_count() >= 0, false);
  int snext = si;
  ++snext;
  while (snext < size && !_states->has_element(snext)) {
    ++snext;
  }
  while (snext < size) {
    nassertr(_states->get_key(snext)->get_ref_count() >= 0, false);
    const RenderState *ssi = _states->get_key(si);
    const RenderState *ssnext = _states->get_key(snext);
    int c = ssi->compare_to(*ssnext);
    int ci = ssnext->compare_to(*ssi);
    if ((ci < 0) != (c > 0) ||
        (ci > 0) != (c < 0) ||
        (ci == 0) != (c == 0)) {
      pgraph_cat.error()
        << "RenderState::compare_to() not defined properly!\n";
      pgraph_cat.error(false)
        << "(a, b): " << c << "\n";
      pgraph_cat.error(false)
        << "(b, a): " << ci << "\n";
      ssi->write(pgraph_cat.error(false), 2);
      ssnext->write(pgraph_cat.error(false), 2);
      return false;
    }
    si = snext;
    ++snext;
    while (snext < size && !_states->has_element(snext)) {
      ++snext;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_geom_rendering
//       Access: Published
//  Description: Returns the union of the Geom::GeomRendering bits
//               that will be required once this RenderState is
//               applied to a geom which includes the indicated
//               geom_rendering bits.
////////////////////////////////////////////////////////////////////
int RenderState::
get_geom_rendering(int geom_rendering) const {
  const RenderModeAttrib *render_mode = DCAST(RenderModeAttrib, get_attrib(RenderModeAttrib::get_class_slot()));
  const TexGenAttrib *tex_gen = DCAST(TexGenAttrib, get_attrib(TexGenAttrib::get_class_slot()));
  const TexMatrixAttrib *tex_matrix = DCAST(TexMatrixAttrib, get_attrib(TexMatrixAttrib::get_class_slot()));

  if (render_mode != (const RenderModeAttrib *)NULL) {
    geom_rendering = render_mode->get_geom_rendering(geom_rendering);
  }
  if (tex_gen != (const TexGenAttrib *)NULL) {
    geom_rendering = tex_gen->get_geom_rendering(geom_rendering);
  }
  if (tex_matrix != (const TexMatrixAttrib *)NULL) {
    geom_rendering = tex_matrix->get_geom_rendering(geom_rendering);
  }

  return geom_rendering;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::bin_removed
//       Access: Public, Static
//  Description: Intended to be called by
//               CullBinManager::remove_bin(), this informs all the
//               RenderStates in the world to remove the indicated
//               bin_index from their cache if it has been cached.
////////////////////////////////////////////////////////////////////
void RenderState::
bin_removed(int bin_index) {
  // Do something here.
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::validate_filled_slots
//       Access: Private
//  Description: Returns true if the _filled_slots bitmask is
//               consistent with the table of RenderAttrib pointers,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool RenderState::
validate_filled_slots() const {
  SlotMask mask;

  RenderAttribRegistry *reg = RenderAttribRegistry::quick_get_global_ptr();
  int max_slots = reg->get_max_slots();
  for (int slot = 1; slot < max_slots; ++slot) {
    const Attribute &attribute = _attributes[slot];
    if (attribute._attrib != (RenderAttrib *)NULL) {
      mask.set_bit(slot);
    }
  }

  return (mask == _filled_slots);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::do_calc_hash
//       Access: Private
//  Description: Computes a suitable hash value for phash_map.
////////////////////////////////////////////////////////////////////
void RenderState::
do_calc_hash() {
  _hash = 0;

  SlotMask mask = _filled_slots;
  int slot = mask.get_lowest_on_bit();
  while (slot >= 0) {
    const Attribute &attrib = _attributes[slot];
    nassertv(attrib._attrib != (RenderAttrib *)NULL);
    _hash = pointer_hash::add_hash(_hash, attrib._attrib);
    _hash = int_hash::add_hash(_hash, attrib._override);

    mask.clear_bit(slot);
    slot = mask.get_lowest_on_bit();
  }

  _flags |= F_hash_known;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::assign_auto_shader_state
//       Access: Private
//  Description: Sets _auto_shader_state to the appropriate
//               RenderState object pointer, either the same pointer
//               as this object, or some other (simpler) RenderState.
////////////////////////////////////////////////////////////////////
void RenderState::
assign_auto_shader_state() {
  CPT(RenderState) state = do_calc_auto_shader_state();

  {
    LightReMutexHolder holder(*_states_lock);
    if (_auto_shader_state == (const RenderState *)NULL) {
      _auto_shader_state = state;
      if (_auto_shader_state != this) {
        _auto_shader_state->cache_ref();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::do_calc_auto_shader_state
//       Access: Private
//  Description: Returns the appropriate RenderState that should be
//               used to store the auto shader pointer for nodes that
//               shader this RenderState.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
do_calc_auto_shader_state() {
  RenderState *state = new RenderState;

  SlotMask mask = _filled_slots;
  int slot = mask.get_lowest_on_bit();
  while (slot >= 0) {
    const Attribute &attrib = _attributes[slot];
    nassertr(attrib._attrib != (RenderAttrib *)NULL, this);
    CPT(RenderAttrib) new_attrib = attrib._attrib->get_auto_shader_attrib(this);
    if (new_attrib != NULL) {
      nassertr(new_attrib->get_slot() == slot, this);
      state->_attributes[slot].set(new_attrib, 0);
      state->_filled_slots.set_bit(slot);
    }
    
    mask.clear_bit(slot);
    slot = mask.get_lowest_on_bit();
  }

  return return_new(state);
}

  
////////////////////////////////////////////////////////////////////
//     Function: RenderState::return_new
//       Access: Private, Static
//  Description: This function is used to share a common RenderState
//               pointer for all equivalent RenderState objects.
//
//               This is different from return_unique() in that it
//               does not actually guarantee a unique pointer, unless
//               uniquify-states is set.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
return_new(RenderState *state) {
  nassertr(state != (RenderState *)NULL, state);

  // Make sure we don't have anything in the 0 slot.  If we did, that
  // would indicate an uninitialized slot number.
#ifndef NDEBUG
  if (state->_attributes[0]._attrib != (RenderAttrib *)NULL) {
    const RenderAttrib *attrib = state->_attributes[0]._attrib;
    if (attrib->get_type() == TypeHandle::none()) {
      ((RenderAttrib *)attrib)->force_init_type();
      pgraph_cat->error()
        << "Uninitialized RenderAttrib type: " << attrib->get_type()
        << "\n";

    } else {
      static pset<TypeHandle> already_reported;
      if (already_reported.insert(attrib->get_type()).second) {
        pgraph_cat->error()
          << attrib->get_type() << " did not initialize its slot number.\n";
      }
    }
  }
#endif
  state->_attributes[0]._attrib = NULL;
  state->_filled_slots.clear_bit(0);

#ifndef NDEBUG
  nassertr(state->validate_filled_slots(), state);
#endif

  if (!uniquify_states && !state->is_empty()) {
    return state;
  }

  return return_unique(state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::return_unique
//       Access: Private, Static
//  Description: This function is used to share a common RenderState
//               pointer for all equivalent RenderState objects.
//
//               See the similar logic in RenderAttrib.  The idea is
//               to create a new RenderState object and pass it
//               through this function, which will share the pointer
//               with a previously-created RenderState object if it is
//               equivalent.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
return_unique(RenderState *state) {
  nassertr(state != (RenderState *)NULL, state);

  if (!state_cache) {
    return state;
  }

#ifndef NDEBUG
  if (paranoid_const) {
    nassertr(validate_states(), state);
  }
#endif

  LightReMutexHolder holder(*_states_lock);

  if (state->_saved_entry != -1) {
    // This state is already in the cache.
    //nassertr(_states->find(state) == state->_saved_entry, state);
    return state;
  }

  // Save the state in a local PointerTo so that it will be freed at
  // the end of this function if no one else uses it.
  CPT(RenderState) pt_state = state;

  // Ensure each of the individual attrib pointers has been uniquified
  // before we add the state to the cache.
  if (!uniquify_attribs && !state->is_empty()) {
    SlotMask mask = state->_filled_slots;
    int slot = mask.get_lowest_on_bit();
    while (slot >= 0) {
      Attribute &attrib = state->_attributes[slot];
      nassertr(attrib._attrib != (RenderAttrib *)NULL, state);
      attrib._attrib = attrib._attrib->get_unique();
      mask.clear_bit(slot);
      slot = mask.get_lowest_on_bit();
    }    
  }

  int si = _states->find(state);
  if (si != -1) {
    // There's an equivalent state already in the set.  Return it.
    return _states->get_key(si);
  }
  
  // Not already in the set; add it.
  if (garbage_collect_states) {
    // If we'll be garbage collecting states explicitly, we'll
    // increment the reference count when we store it in the cache, so
    // that it won't be deleted while it's in it.
    state->cache_ref();
  }
  si = _states->store(state, Empty());

  // Save the index and return the input state.
  state->_saved_entry = si;
  return pt_state;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::do_compose
//       Access: Private
//  Description: The private implemention of compose(); this actually
//               composes two RenderStates, without bothering with the
//               cache.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
do_compose(const RenderState *other) const {
  PStatTimer timer(_state_compose_pcollector);

  RenderState *new_state = new RenderState;

  SlotMask mask = _filled_slots | other->_filled_slots;
  new_state->_filled_slots = mask;

  int slot = mask.get_lowest_on_bit();
  while (slot >= 0) {
    const Attribute &a = _attributes[slot];
    const Attribute &b = other->_attributes[slot];
    Attribute &result = new_state->_attributes[slot];

    if (a._attrib == NULL) {
      nassertr(b._attrib != NULL, this);
      // B wins.
      result = b;

    } else if (b._attrib == NULL) {
      // A wins.
      result = a;

    } else if (b._override < a._override) {
      // A, the higher RenderAttrib, overrides.
      result = a;

    } else if (a._override < b._override &&
               a._attrib->lower_attrib_can_override()) {
      // B, the higher RenderAttrib, overrides.  This is a special
      // case; normally, a lower RenderAttrib does not override a
      // higher one, even if it has a higher override value.  But
      // certain kinds of RenderAttribs redefine
      // lower_attrib_can_override() to return true, allowing this
      // override.
      result = b;

    } else {
      // Either they have the same override value, or B is higher.
      // In either case, the result is the composition of the two,
      // with B's override value.
      result.set(a._attrib->compose(b._attrib), b._override);
    }
    
    mask.clear_bit(slot);
    slot = mask.get_lowest_on_bit();
  }
  
  // If we have any ShaderAttrib with auto-shader enabled,
  // remove any shader inputs on it. This is a workaround for an
  // issue that makes the shader-generator regenerate the shader
  // every time a shader input changes.
  CPT(ShaderAttrib) sattrib = DCAST(ShaderAttrib, new_state->get_attrib_def(ShaderAttrib::get_class_slot()));
  if (sattrib->auto_shader()) {
    sattrib = DCAST(ShaderAttrib, sattrib->clear_all_shader_inputs());
  }
  
  return return_new(new_state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::do_invert_compose
//       Access: Private
//  Description: The private implemention of invert_compose().
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
do_invert_compose(const RenderState *other) const {
  PStatTimer timer(_state_invert_pcollector);

  RenderState *new_state = new RenderState;

  SlotMask mask = _filled_slots | other->_filled_slots;
  new_state->_filled_slots = mask;

  int slot = mask.get_lowest_on_bit();
  while (slot >= 0) {
    const Attribute &a = _attributes[slot];
    const Attribute &b = other->_attributes[slot];
    Attribute &result = new_state->_attributes[slot];

    if (a._attrib == NULL) {
      nassertr(b._attrib != NULL, this);
      // B wins.
      result = b;

    } else if (b._attrib == NULL) {
      // A wins.  Invert it.
      CPT(RenderState) full_default = make_full_default();
      CPT(RenderAttrib) default_attrib = full_default->get_attrib(slot);
      result.set(a._attrib->invert_compose(default_attrib), 0);

    } else {
      // Both are good.  (Overrides are not used in invert_compose.)
      // Compose.
      result.set(a._attrib->invert_compose(b._attrib), 0);
    }
    
    mask.clear_bit(slot);
    slot = mask.get_lowest_on_bit();
  }
  return return_new(new_state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::detect_and_break_cycles
//       Access: Private
//  Description: Detects whether there is a cycle in the cache that
//               begins with this state.  If any are detected, breaks
//               them by removing this state from the cache.
////////////////////////////////////////////////////////////////////
void RenderState::
detect_and_break_cycles() {
  PStatTimer timer(_state_break_cycles_pcollector);
      
  ++_last_cycle_detect;
  if (r_detect_cycles(this, this, 1, _last_cycle_detect, NULL)) {
    // Ok, we have a cycle.  This will be a leak unless we break the
    // cycle by freeing the cache on this object.
    if (pgraph_cat.is_debug()) {
      pgraph_cat.debug()
        << "Breaking cycle involving " << (*this) << "\n";
    }
    
    ((RenderState *)this)->remove_cache_pointers();
  } else {
    ++_last_cycle_detect;
    if (r_detect_reverse_cycles(this, this, 1, _last_cycle_detect, NULL)) {
      if (pgraph_cat.is_debug()) {
        pgraph_cat.debug()
          << "Breaking cycle involving " << (*this) << "\n";
      }
      
      ((RenderState *)this)->remove_cache_pointers();
    }
  }
}
  
////////////////////////////////////////////////////////////////////
//     Function: RenderState::r_detect_cycles
//       Access: Private, Static
//  Description: Detects whether there is a cycle in the cache that
//               begins with the indicated state.  Returns true if at
//               least one cycle is found, false if this state is not
//               part of any cycles.  If a cycle is found and
//               cycle_desc is not NULL, then cycle_desc is filled in
//               with the list of the steps of the cycle, in reverse
//               order.
////////////////////////////////////////////////////////////////////
bool RenderState::
r_detect_cycles(const RenderState *start_state,
                const RenderState *current_state,
                int length, UpdateSeq this_seq,
                RenderState::CompositionCycleDesc *cycle_desc) {
  if (current_state->_cycle_detect == this_seq) {
    // We've already seen this state; therefore, we've found a cycle.

    // However, we only care about cycles that return to the starting
    // state and involve more than two steps.  If only one or two
    // nodes are involved, it doesn't represent a memory leak, so no
    // problem there.
    return (current_state == start_state && length > 2);
  }
  ((RenderState *)current_state)->_cycle_detect = this_seq;
    
  int i;
  int cache_size = current_state->_composition_cache.get_size();
  for (i = 0; i < cache_size; ++i) {
    if (current_state->_composition_cache.has_element(i)) {
      const RenderState *result = current_state->_composition_cache.get_data(i)._result;
      if (result != (const RenderState *)NULL) {
        if (r_detect_cycles(start_state, result, length + 1, 
                            this_seq, cycle_desc)) {
          // Cycle detected.
          if (cycle_desc != (CompositionCycleDesc *)NULL) {
            const RenderState *other = current_state->_composition_cache.get_key(i);
            CompositionCycleDescEntry entry(other, result, false);
            cycle_desc->push_back(entry);
          }
          return true;
        }
      }
    }
  }

  cache_size = current_state->_invert_composition_cache.get_size();
  for (i = 0; i < cache_size; ++i) {
    if (current_state->_invert_composition_cache.has_element(i)) {
      const RenderState *result = current_state->_invert_composition_cache.get_data(i)._result;
      if (result != (const RenderState *)NULL) {
        if (r_detect_cycles(start_state, result, length + 1,
                            this_seq, cycle_desc)) {
          // Cycle detected.
          if (cycle_desc != (CompositionCycleDesc *)NULL) {
            const RenderState *other = current_state->_invert_composition_cache.get_key(i);
            CompositionCycleDescEntry entry(other, result, true);
            cycle_desc->push_back(entry);
          }
          return true;
        }
      }
    }
  }

  // No cycle detected.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::r_detect_reverse_cycles
//       Access: Private, Static
//  Description: Works the same as r_detect_cycles, but checks for
//               cycles in the reverse direction along the cache
//               chain.  (A cycle may appear in either direction, and
//               we must check both.)
////////////////////////////////////////////////////////////////////
bool RenderState::
r_detect_reverse_cycles(const RenderState *start_state,
                        const RenderState *current_state,
                        int length, UpdateSeq this_seq,
                        RenderState::CompositionCycleDesc *cycle_desc) {
  if (current_state->_cycle_detect == this_seq) {
    // We've already seen this state; therefore, we've found a cycle.

    // However, we only care about cycles that return to the starting
    // state and involve more than two steps.  If only one or two
    // nodes are involved, it doesn't represent a memory leak, so no
    // problem there.
    return (current_state == start_state && length > 2);
  }
  ((RenderState *)current_state)->_cycle_detect = this_seq;

  int i;
  int cache_size = current_state->_composition_cache.get_size();
  for (i = 0; i < cache_size; ++i) {
    if (current_state->_composition_cache.has_element(i)) {
      const RenderState *other = current_state->_composition_cache.get_key(i);
      if (other != current_state) {
        int oi = other->_composition_cache.find(current_state);
        nassertr(oi != -1, false);

        const RenderState *result = other->_composition_cache.get_data(oi)._result;
        if (result != (const RenderState *)NULL) {
          if (r_detect_reverse_cycles(start_state, result, length + 1, 
                                      this_seq, cycle_desc)) {
            // Cycle detected.
            if (cycle_desc != (CompositionCycleDesc *)NULL) {
              const RenderState *other = current_state->_composition_cache.get_key(i);
              CompositionCycleDescEntry entry(other, result, false);
              cycle_desc->push_back(entry);
            }
            return true;
          }
        }
      }
    }
  }

  cache_size = current_state->_invert_composition_cache.get_size();
  for (i = 0; i < cache_size; ++i) {
    if (current_state->_invert_composition_cache.has_element(i)) {
      const RenderState *other = current_state->_invert_composition_cache.get_key(i);
      if (other != current_state) {
        int oi = other->_invert_composition_cache.find(current_state);
        nassertr(oi != -1, false);

        const RenderState *result = other->_invert_composition_cache.get_data(oi)._result;
        if (result != (const RenderState *)NULL) {
          if (r_detect_reverse_cycles(start_state, result, length + 1, 
                                      this_seq, cycle_desc)) {
            // Cycle detected.
            if (cycle_desc != (CompositionCycleDesc *)NULL) {
              const RenderState *other = current_state->_invert_composition_cache.get_key(i);
              CompositionCycleDescEntry entry(other, result, false);
              cycle_desc->push_back(entry);
            }
            return true;
          }
        }
      }
    }
  }

  // No cycle detected.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::release_new
//       Access: Private
//  Description: This inverse of return_new, this releases this object
//               from the global RenderState table.
//
//               You must already be holding _states_lock before you
//               call this method.
////////////////////////////////////////////////////////////////////
void RenderState::
release_new() {
  nassertv(_states_lock->debug_is_locked());

  if (_saved_entry != -1) {
    //nassertv(_states->find(this) == _saved_entry);
    _saved_entry = _states->find(this);
    _states->remove_element(_saved_entry);
    _saved_entry = -1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::remove_cache_pointers
//       Access: Private
//  Description: Remove all pointers within the cache from and to this
//               particular RenderState.  The pointers to this
//               object may be scattered around in the various
//               CompositionCaches from other RenderState objects.
//
//               You must already be holding _states_lock before you
//               call this method.
////////////////////////////////////////////////////////////////////
void RenderState::
remove_cache_pointers() {
  nassertv(_states_lock->debug_is_locked());

  // First, make sure the _auto_shader_state cache pointer is cleared.
  if (_auto_shader_state != (const RenderState *)NULL) {
    if (_auto_shader_state != this) {
      cache_unref_delete(_auto_shader_state);
    }
    _auto_shader_state = NULL;
  }

  // Fortunately, since we added CompositionCache records in pairs, we
  // know exactly the set of RenderState objects that have us in their
  // cache: it's the same set of RenderState objects that we have in
  // our own cache.

  // We do need to put considerable thought into this loop, because as
  // we clear out cache entries we'll cause other RenderState
  // objects to destruct, which could cause things to get pulled out
  // of our own _composition_cache map.  We want to allow this (so
  // that we don't encounter any just-destructed pointers in our
  // cache), but we don't want to get bitten by this cascading effect.
  // Instead of walking through the map from beginning to end,
  // therefore, we just pull out the first one each time, and erase
  // it.

#ifdef DO_PSTATS
  if (_composition_cache.is_empty() && _invert_composition_cache.is_empty()) {
    return;
  }
  PStatTimer timer(_cache_update_pcollector);
#endif  // DO_PSTATS

  // There are lots of ways to do this loop wrong.  Be very careful if
  // you need to modify it for any reason.
  int i = 0;
  while (!_composition_cache.is_empty()) {
    // Scan for the next used slot in the table.
    while (!_composition_cache.has_element(i)) {
      ++i;
    }

    // It is possible that the "other" RenderState object is
    // currently within its own destructor.  We therefore can't use a
    // PT() to hold its pointer; that could end up calling its
    // destructor twice.  Fortunately, we don't need to hold its
    // reference count to ensure it doesn't destruct while we process
    // this loop; as long as we ensure that no *other* RenderState
    // objects destruct, there will be no reason for that one to.
    RenderState *other = (RenderState *)_composition_cache.get_key(i);

    // We hold a copy of the composition result so we can dereference
    // it later.
    Composition comp = _composition_cache.get_data(i);

    // Now we can remove the element from our cache.  We do this now,
    // rather than later, before any other RenderState objects have
    // had a chance to destruct, so we are confident that our iterator
    // is still valid.
    _composition_cache.remove_element(i);
    _cache_stats.add_total_size(-1);
    _cache_stats.inc_dels();

    if (other != this) {
      int oi = other->_composition_cache.find(this);

      // We may or may not still be listed in the other's cache (it
      // might be halfway through pulling entries out, from within its
      // own destructor).
      if (oi != -1) {
        // Hold a copy of the other composition result, too.
        Composition ocomp = other->_composition_cache.get_data(oi);
        
        other->_composition_cache.remove_element(oi);
        _cache_stats.add_total_size(-1);
        _cache_stats.inc_dels();
        
        // It's finally safe to let our held pointers go away.  This may
        // have cascading effects as other RenderState objects are
        // destructed, but there will be no harm done if they destruct
        // now.
        if (ocomp._result != (const RenderState *)NULL && ocomp._result != other) {
          cache_unref_delete(ocomp._result);
        }
      }
    }

    // It's finally safe to let our held pointers go away.  (See
    // comment above.)
    if (comp._result != (const RenderState *)NULL && comp._result != this) {
      cache_unref_delete(comp._result);
    }
  }

  // A similar bit of code for the invert cache.
  i = 0;
  while (!_invert_composition_cache.is_empty()) {
    while (!_invert_composition_cache.has_element(i)) {
      ++i;
    }

    RenderState *other = (RenderState *)_invert_composition_cache.get_key(i);
    nassertv(other != this);
    Composition comp = _invert_composition_cache.get_data(i);
    _invert_composition_cache.remove_element(i);
    _cache_stats.add_total_size(-1);
    _cache_stats.inc_dels();
    if (other != this) {
      int oi = other->_invert_composition_cache.find(this);
      if (oi != -1) {
        Composition ocomp = other->_invert_composition_cache.get_data(oi);
        other->_invert_composition_cache.remove_element(oi);
        _cache_stats.add_total_size(-1);
        _cache_stats.inc_dels();
        if (ocomp._result != (const RenderState *)NULL && ocomp._result != other) {
          cache_unref_delete(ocomp._result);
        }
      }
    }
    if (comp._result != (const RenderState *)NULL && comp._result != this) {
      cache_unref_delete(comp._result);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::determine_bin_index
//       Access: Private
//  Description: This is the private implementation of
//               get_bin_index() and get_draw_order().
////////////////////////////////////////////////////////////////////
void RenderState::
determine_bin_index() {
  LightMutexHolder holder(_lock);
  if ((_flags & F_checked_bin_index) != 0) {
    // Someone else checked it first.
    return;
  }

  string bin_name;
  _draw_order = 0;

  const CullBinAttrib *bin = DCAST(CullBinAttrib, get_attrib(CullBinAttrib::get_class_slot()));
  if (bin != (const CullBinAttrib *)NULL) {
    bin_name = bin->get_bin_name();
    _draw_order = bin->get_draw_order();
  }

  if (bin_name.empty()) {
    // No explicit bin is specified; put in the in the default bin,
    // either opaque or transparent, based on the transparency
    // setting.
    bin_name = "opaque";

    const TransparencyAttrib *transparency = DCAST(TransparencyAttrib, get_attrib(TransparencyAttrib::get_class_slot()));
    if (transparency != (const TransparencyAttrib *)NULL) {
      switch (transparency->get_mode()) {
      case TransparencyAttrib::M_alpha:
      case TransparencyAttrib::M_dual:
        // These transparency modes require special back-to-front sorting.
        bin_name = "transparent";
        break;

      default:
        break;
      }
    }
  }
  
  CullBinManager *bin_manager = CullBinManager::get_global_ptr();
  _bin_index = bin_manager->find_bin(bin_name);
  if (_bin_index == -1) {
    pgraph_cat.warning()
      << "No bin named " << bin_name << "; creating default bin.\n";
    _bin_index = bin_manager->add_bin(bin_name, CullBinManager::BT_unsorted, 0);
  }
  _flags |= F_checked_bin_index;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::determine_cull_callback
//       Access: Private
//  Description: This is the private implementation of has_cull_callback().
////////////////////////////////////////////////////////////////////
void RenderState::
determine_cull_callback() {
  LightMutexHolder holder(_lock);
  if ((_flags & F_checked_cull_callback) != 0) {
    // Someone else checked it first.
    return;
  }

  SlotMask mask = _filled_slots;
  int slot = mask.get_lowest_on_bit();
  while (slot >= 0) {
    const Attribute &attrib = _attributes[slot];
    nassertv(attrib._attrib != (RenderAttrib *)NULL);
    if (attrib._attrib->has_cull_callback()) {
      _flags |= F_has_cull_callback;
      break;
    }
    
    mask.clear_bit(slot);
    slot = mask.get_lowest_on_bit();
  }

  _flags |= F_checked_cull_callback;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::fill_default
//       Access: Private
//  Description: Fills up the state with all of the default attribs.
////////////////////////////////////////////////////////////////////
void RenderState::
fill_default() {
  RenderAttribRegistry *reg = RenderAttribRegistry::quick_get_global_ptr();
  int num_slots = reg->get_num_slots();
  for (int slot = 1; slot < num_slots; ++slot) {
    _attributes[slot].set(reg->get_slot_default(slot), 0);
    _filled_slots.set_bit(slot);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::update_pstats
//       Access: Private
//  Description: Moves the RenderState object from one PStats category
//               to another, so that we can track in PStats how many
//               pointers are held by nodes, and how many are held in
//               the cache only.
////////////////////////////////////////////////////////////////////
void RenderState::
update_pstats(int old_referenced_bits, int new_referenced_bits) {
#ifdef DO_PSTATS
  if ((old_referenced_bits & R_node) != 0) {
    _node_counter.sub_level(1);
  } else if ((old_referenced_bits & R_cache) != 0) {
    _cache_counter.sub_level(1);
  }
  if ((new_referenced_bits & R_node) != 0) {
    _node_counter.add_level(1);
  } else if ((new_referenced_bits & R_cache) != 0) {
    _cache_counter.add_level(1);
  }
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::init_states
//       Access: Public, Static
//  Description: Make sure the global _states map is allocated.  This
//               only has to be done once.  We could make this map
//               static, but then we run into problems if anyone
//               creates a RenderState object at static init time;
//               it also seems to cause problems when the Panda shared
//               library is unloaded at application exit time.
////////////////////////////////////////////////////////////////////
void RenderState::
init_states() {
  _states = new States;

  // TODO: we should have a global Panda mutex to allow us to safely
  // create _states_lock without a startup race condition.  For the
  // meantime, this is OK because we guarantee that this method is
  // called at static init time, presumably when there is still only
  // one thread in the world.
  _states_lock = new LightReMutex("RenderState::_states_lock");
  _cache_stats.init();
  nassertv(Thread::get_current_thread() == Thread::get_main_thread());
}


////////////////////////////////////////////////////////////////////
//     Function: RenderState::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               RenderState.
////////////////////////////////////////////////////////////////////
void RenderState::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void RenderState::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  int num_attribs = _filled_slots.get_num_on_bits();
  nassertv(num_attribs == (int)(PN_uint16)num_attribs);
  dg.add_uint16(num_attribs);

  // **** We should smarten up the writing of the override
  // number--most of the time these will all be zero.
  SlotMask mask = _filled_slots;
  int slot = mask.get_lowest_on_bit();
  while (slot >= 0) {
    const Attribute &attrib = _attributes[slot];
    nassertv(attrib._attrib != (RenderAttrib *)NULL);
    manager->write_pointer(dg, attrib._attrib);
    dg.add_int32(attrib._override);
    
    mask.clear_bit(slot);
    slot = mask.get_lowest_on_bit();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int RenderState::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritable::complete_pointers(p_list, manager);

  int num_attribs = 0;

  RenderAttribRegistry *reg = RenderAttribRegistry::quick_get_global_ptr();
  for (size_t i = 0; i < (*_read_overrides).size(); ++i) {
    int override = (*_read_overrides)[i];

    RenderAttrib *attrib = DCAST(RenderAttrib, p_list[pi++]);
    if (attrib != (RenderAttrib *)NULL) {
      int slot = attrib->get_slot();
      if (slot > 0 && slot < reg->get_max_slots()) {
        _attributes[slot].set(attrib, override);
        _filled_slots.set_bit(slot);
        ++num_attribs;
      }
    }
  }

  delete _read_overrides;
  _read_overrides = NULL;

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::change_this
//       Access: Public, Static
//  Description: Called immediately after complete_pointers(), this
//               gives the object a chance to adjust its own pointer
//               if desired.  Most objects don't change pointers after
//               completion, but some need to.
//
//               Once this function has been called, the old pointer
//               will no longer be accessed.
////////////////////////////////////////////////////////////////////
TypedWritable *RenderState::
change_this(TypedWritable *old_ptr, BamReader *manager) {
  // First, uniquify the pointer.
  RenderState *state = DCAST(RenderState, old_ptr);
  CPT(RenderState) pointer = return_unique(state);

  // But now we have a problem, since we have to hold the reference
  // count and there's no way to return a TypedWritable while still
  // holding the reference count!  We work around this by explicitly
  // upping the count, and also setting a finalize() callback to down
  // it later.
  if (pointer == state) {
    pointer->ref();
    manager->register_finalize(state);
  }
  
  // We have to cast the pointer back to non-const, because the bam
  // reader expects that.
  return (RenderState *)pointer.p();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::finalize
//       Access: Public, Virtual
//  Description: Called by the BamReader to perform any final actions
//               needed for setting up the object after all objects
//               have been read and all pointers have been completed.
////////////////////////////////////////////////////////////////////
void RenderState::
finalize(BamReader *) {
  // Unref the pointer that we explicitly reffed in change_this().
  unref();

  // We should never get back to zero after unreffing our own count,
  // because we expect to have been stored in a pointer somewhere.  If
  // we do get to zero, it's a memory leak; the way to avoid this is
  // to call unref_delete() above instead of unref(), but this is
  // dangerous to do from within a virtual function.
  nassertv(get_ref_count() != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type RenderState is encountered
//               in the Bam file.  It should create the RenderState
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *RenderState::
make_from_bam(const FactoryParams &params) {
  RenderState *state = new RenderState;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  state->fillin(scan, manager);
  manager->register_change_this(change_this, state);

  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new RenderState.
////////////////////////////////////////////////////////////////////
void RenderState::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  int num_attribs = scan.get_uint16();
  _read_overrides = new vector_int;
  (*_read_overrides).reserve(num_attribs);

  for (int i = 0; i < num_attribs; ++i) {
    manager->read_pointer(scan);
    int override = scan.get_int32();
    (*_read_overrides).push_back(override);
  }
}

