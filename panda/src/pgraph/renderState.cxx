// Filename: renderState.cxx
// Created by:  drose (21Feb02)
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

#include "renderState.h"
#include "transparencyAttrib.h"
#include "cullBinAttrib.h"
#include "cullBinManager.h"
#include "fogAttrib.h"
#include "clipPlaneAttrib.h"
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

RenderState::States *RenderState::_states = NULL;
CPT(RenderState) RenderState::_empty_state;
UpdateSeq RenderState::_last_cycle_detect;
PStatCollector RenderState::_cache_update_pcollector("*:State Cache:Update");
PStatCollector RenderState::_state_compose_pcollector("*:State Cache:Compose State");
PStatCollector RenderState::_state_invert_pcollector("*:State Cache:Invert State");
PStatCollector RenderState::_node_counter("RenderStates:On nodes");
PStatCollector RenderState::_cache_counter("RenderStates:Cached");

TypeHandle RenderState::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: RenderState::Constructor
//       Access: Protected
//  Description: Actually, this could be a private constructor, since
//               no one inherits from RenderState, but gcc gives us a
//               spurious warning if all constructors are private.
////////////////////////////////////////////////////////////////////
RenderState::
RenderState() {
  if (_states == (States *)NULL) {
    // Make sure the global _states map is allocated.  This only has
    // to be done once.  We could make this map static, but then we
    // run into problems if anyone creates a RenderState object at
    // static init time; it also seems to cause problems when the
    // Panda shared library is unloaded at application exit time.
    _states = new States;
  }
  _saved_entry = _states->end();
  _flags = 0;
  _last_mi = _mungers.end();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Copy Constructor
//       Access: Private
//  Description: RenderStates are not meant to be copied.
////////////////////////////////////////////////////////////////////
RenderState::
RenderState(const RenderState &) {
  nassertv(false);
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

  if (_saved_entry != _states->end()) {
    nassertv(_states->find(this) == _saved_entry);
    _states->erase(_saved_entry);
    _saved_entry = _states->end();
  }

  remove_cache_pointers();

  // If this was true at the beginning of the destructor, but is no
  // longer true now, probably we've been double-deleted.
  nassertv(get_ref_count() == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::operator <
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
bool RenderState::
operator < (const RenderState &other) const {
  // We must compare all the properties of the attributes, not just
  // the type; thus, we compare them one at a time using compare_to().
  return lexicographical_compare(_attributes.begin(), _attributes.end(),
                                 other._attributes.begin(), other._attributes.end(),
                                 CompareTo<Attribute>());
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
  Attributes::const_iterator ai;
  for (ai = _attributes.begin(); ai != _attributes.end(); ++ai) {
    const Attribute &attrib = *ai;
    if (!attrib._attrib->cull_callback(trav, data)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::find_attrib
//       Access: Published
//  Description: Searches for an attribute with the indicated type in
//               the state, and returns its index if it is found, or
//               -1 if it is not.
////////////////////////////////////////////////////////////////////
int RenderState::
find_attrib(TypeHandle type) const {
  Attributes::const_iterator ai = _attributes.find(Attribute(type));
  if (ai == _attributes.end()) {
    return -1;
  }
  return ai - _attributes.begin();
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
    _empty_state = return_new(state);
  }

  return _empty_state;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::make
//       Access: Published, Static
//  Description: Returns a RenderState with one attribute set.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
make(const RenderAttrib *attrib, int override) {
  RenderState *state = new RenderState;
  state->_attributes.reserve(1);
  state->_attributes.insert(Attribute(attrib, override));
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
  state->_attributes.reserve(2);
  state->_attributes.push_back(Attribute(attrib1, override));
  state->_attributes.push_back(Attribute(attrib2, override));
  state->_attributes.sort();
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
  state->_attributes.reserve(3);
  state->_attributes.push_back(Attribute(attrib1, override));
  state->_attributes.push_back(Attribute(attrib2, override));
  state->_attributes.push_back(Attribute(attrib3, override));
  state->_attributes.sort();
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
  state->_attributes.reserve(4);
  state->_attributes.push_back(Attribute(attrib1, override));
  state->_attributes.push_back(Attribute(attrib2, override));
  state->_attributes.push_back(Attribute(attrib3, override));
  state->_attributes.push_back(Attribute(attrib4, override));
  state->_attributes.sort();
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::make
//       Access: Published, Static
//  Description: Returns a RenderState with n attributes set.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
make(const RenderAttrib * const *attrib, int num_attribs, int override) {
  RenderState *state = new RenderState;
  state->_attributes.reserve(num_attribs);
  for (int i = 0; i < num_attribs; i++) {
    state->_attributes.push_back(Attribute(attrib[i], override));
  }
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::make
//       Access: Published, Static
//  Description: Returns a RenderState made from the specified slots.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
make(const AttribSlots *slots, int override) {
  RenderState *state = new RenderState;
  for (int i = 0; i < AttribSlots::slot_count; i++) {
    const RenderAttrib *attrib = slots->get_slot(i);
    if (attrib != 0) {
      state->_attributes.push_back(Attribute(attrib, override));
    }
  }
  state->_attributes.reserve(state->_attributes.size());
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

  // Is this composition already cached?
  CompositionCache::const_iterator ci = _composition_cache.find(other);
  if (ci != _composition_cache.end()) {
    const Composition &comp = (*ci).second;
    if (comp._result == (const RenderState *)NULL) {
      // Well, it wasn't cached already, but we already had an entry
      // (probably created for the reverse direction), so use the same
      // entry to store the new result.
      CPT(RenderState) result = do_compose(other);
      ((Composition &)comp)._result = result;

      if (result != (const RenderState *)this) {
        // See the comments below about the need to up the reference
        // count only when the result is not the same as this.
        result->cache_ref();
      }
    }
    // Here's the cache!
    return comp._result;
  }

  // We need to make a new cache entry, both in this object and in the
  // other object.  We make both records so the other RenderState
  // object will know to delete the entry from this object when it
  // destructs, and vice-versa.

  // The cache entry in this object is the only one that indicates the
  // result; the other will be NULL for now.
  CPT(RenderState) result = do_compose(other);

  // Order is important here, in case other == this.
  ((RenderState *)other)->_composition_cache[this]._result = NULL;
  ((RenderState *)this)->_composition_cache[other]._result = result;

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

  // Is this composition already cached?
  CompositionCache::const_iterator ci = _invert_composition_cache.find(other);
  if (ci != _invert_composition_cache.end()) {
    const Composition &comp = (*ci).second;
    if (comp._result == (const RenderState *)NULL) {
      // Well, it wasn't cached already, but we already had an entry
      // (probably created for the reverse direction), so use the same
      // entry to store the new result.
      CPT(RenderState) result = do_invert_compose(other);
      ((Composition &)comp)._result = result;

      if (result != (const RenderState *)this) {
        // See the comments below about the need to up the reference
        // count only when the result is not the same as this.
        result->cache_ref();
      }
    }
    // Here's the cache!
    return comp._result;
  }

  // We need to make a new cache entry, both in this object and in the
  // other object.  We make both records so the other RenderState
  // object will know to delete the entry from this object when it
  // destructs, and vice-versa.

  // The cache entry in this object is the only one that indicates the
  // result; the other will be NULL for now.
  CPT(RenderState) result = do_invert_compose(other);

  ((RenderState *)other)->_invert_composition_cache[this]._result = NULL;
  ((RenderState *)this)->_invert_composition_cache[other]._result = result;

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
//               same type, it is replaced.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
add_attrib(const RenderAttrib *attrib, int override) const {
  RenderState *new_state = new RenderState;
  back_insert_iterator<Attributes> result = 
    back_inserter(new_state->_attributes);

  Attribute new_attribute(attrib, override);
  Attributes::const_iterator ai = _attributes.begin();

  while (ai != _attributes.end() && (*ai) < new_attribute) {
    *result = *ai;
    ++ai;
    ++result;
  }
  *result = new_attribute;
  ++result;

  if (ai != _attributes.end() && !(new_attribute < (*ai))) {
    // At this point we know:
    // !((*ai) < new_attribute) && !(new_attribute < (*ai))
    // which means (*ai) == new_attribute--so we should leave it out,
    // to avoid duplicating attributes in the set.
    ++ai;
  }

  while (ai != _attributes.end()) {
    *result = *ai;
    ++ai;
    ++result;
  }

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
remove_attrib(TypeHandle type) const {
  RenderState *new_state = new RenderState;
  back_insert_iterator<Attributes> result = 
    back_inserter(new_state->_attributes);

  Attributes::const_iterator ai = _attributes.begin();

  while (ai != _attributes.end()) {
    if ((*ai)._type != type) {
      *result = *ai;
      ++result;
    }
    ++ai;
  }

  return return_new(new_state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::remove_attrib
//       Access: Published
//  Description: Returns a new RenderState object that represents the
//               same as the source state, with all attributes'
//               override values incremented (or decremented, if
//               negative) by the indicated amount.  If the override
//               would drop below zero, it is set to zero.
////////////////////////////////////////////////////////////////////
CPT(RenderState) RenderState::
adjust_all_priorities(int adjustment) const {
  RenderState *new_state = new RenderState;
  new_state->_attributes.reserve(_attributes.size());

  Attributes::const_iterator ai;
  for (ai = _attributes.begin(); ai != _attributes.end(); ++ai) {
    Attribute attrib = *ai;
    attrib._override = max(attrib._override + adjustment, 0);
    new_state->_attributes.push_back(attrib);
  }

  return return_new(new_state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_attrib
//       Access: Published, Virtual
//  Description: Looks for a RenderAttrib of the indicated type in the
//               state, and returns it if it is found, or NULL if it
//               is not.
////////////////////////////////////////////////////////////////////
const RenderAttrib *RenderState::
get_attrib(TypeHandle type) const {
  Attributes::const_iterator ai;
  ai = _attributes.find(Attribute(type));
  if (ai != _attributes.end()) {
    return (*ai)._attrib;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_override
//       Access: Published, Virtual
//  Description: Looks for a RenderAttrib of the indicated type in the
//               state, and returns its override value if it is found,
//               or 0 if it is not.
////////////////////////////////////////////////////////////////////
int RenderState::
get_override(TypeHandle type) const {
  Attributes::const_iterator ai;
  ai = _attributes.find(Attribute(type));
  if (ai != _attributes.end()) {
    return (*ai)._override;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::unref
//       Access: Published
//  Description: This method overrides ReferenceCount::unref() to
//               check whether the remaining reference count is
//               entirely in the cache, and if so, it checks for and
//               breaks a cycle in the cache involving this object.
//               This is designed to prevent leaks from cyclical
//               references within the cache.
//
//               Note that this is not a virtual method, and cannot be
//               because ReferenceCount itself declares no virtual
//               methods (it avoids the overhead of a virtual function
//               pointer).  But this doesn't matter, because
//               PT(TransformState) is a template class, and will call
//               the appropriate method even though it is non-virtual.
////////////////////////////////////////////////////////////////////
int RenderState::
unref() const {
  if (get_cache_ref_count() > 0 &&
      get_ref_count() == get_cache_ref_count() + 1) {
    // If we are about to remove the one reference that is not in the
    // cache, leaving only references in the cache, then we need to
    // check for a cycle involving this RenderState and break it if
    // it exists.

    if (auto_break_cycles) {
      // There might be a tiny race condition if multiple different
      // threads perform cycle detects on related nodes at the same
      // time.  But the cost of failing the race condition is low--we
      // end up with a tiny leak that may eventually be discovered, big
      // deal.
      ++_last_cycle_detect;
      if (r_detect_cycles(this, this, 1, _last_cycle_detect, NULL)) {
        // Ok, we have a cycle.  This will be a leak unless we break the
        // cycle by freeing the cache on this object.
        if (pgraph_cat.is_debug()) {
          pgraph_cat.debug()
            << "Breaking cycle involving " << (*this) << "\n";
        }
        ((RenderState *)this)->remove_cache_pointers();
      }
    }
  }

  return ReferenceCount::unref();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void RenderState::
output(ostream &out) const {
  out << "S:";
  if (_attributes.empty()) {
    out << "(empty)";

  } else {
    Attributes::const_iterator ai = _attributes.begin();
    out << "(" << (*ai)._type;
    ++ai;
    while (ai != _attributes.end()) {
      out << " " << (*ai)._type;
      ++ai;
    }
    out << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void RenderState::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << _attributes.size() << " attribs:\n";
  Attributes::const_iterator ai;
  for (ai = _attributes.begin(); ai != _attributes.end(); ++ai) {
    const Attribute &attribute = (*ai);
    attribute._attrib->write(out, indent_level + 2);
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
  return _states->size();
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

  // First, we need to count the number of times each RenderState
  // object is recorded in the cache.
  typedef pmap<const RenderState *, int> StateCount;
  StateCount state_count;

  States::iterator si;
  for (si = _states->begin(); si != _states->end(); ++si) {
    const RenderState *state = (*si);

    CompositionCache::const_iterator ci;
    for (ci = state->_composition_cache.begin();
         ci != state->_composition_cache.end();
         ++ci) {
      const RenderState *result = (*ci).second._result;
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
    for (ci = state->_invert_composition_cache.begin();
         ci != state->_invert_composition_cache.end();
         ++ci) {
      const RenderState *result = (*ci).second._result;
      if (result != (const RenderState *)NULL && result != state) {
        pair<StateCount::iterator, bool> ir =
          state_count.insert(StateCount::value_type(result, 1));
        if (!ir.second) {
          (*(ir.first)).second++;
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

  PStatTimer timer(_cache_update_pcollector);
  int orig_size = _states->size();

  // First, we need to copy the entire set of states to a temporary
  // vector, reference-counting each object.  That way we can walk
  // through the copy, without fear of dereferencing (and deleting)
  // the objects in the map as we go.
  {
    typedef pvector< CPT(RenderState) > TempStates;
    TempStates temp_states;
    temp_states.reserve(orig_size);

    copy(_states->begin(), _states->end(),
         back_inserter(temp_states));

    // Now it's safe to walk through the list, destroying the cache
    // within each object as we go.  Nothing will be destructed till
    // we're done.
    TempStates::iterator ti;
    for (ti = temp_states.begin(); ti != temp_states.end(); ++ti) {
      RenderState *state = (RenderState *)(*ti).p();

      CompositionCache::const_iterator ci;
      for (ci = state->_composition_cache.begin();
           ci != state->_composition_cache.end();
           ++ci) {
        const RenderState *result = (*ci).second._result;
        if (result != (const RenderState *)NULL && result != state) {
          result->cache_unref();
          nassertr(result->get_ref_count() > 0, 0);
        }
      }
      state->_composition_cache.clear();

      for (ci = state->_invert_composition_cache.begin();
           ci != state->_invert_composition_cache.end();
           ++ci) {
        const RenderState *result = (*ci).second._result;
        if (result != (const RenderState *)NULL && result != state) {
          result->cache_unref();
          nassertr(result->get_ref_count() > 0, 0);
        }
      }
      state->_invert_composition_cache.clear();
    }

    // Once this block closes and the temp_states object goes away,
    // all the destruction will begin.  Anything whose reference was
    // held only within the various objects' caches will go away.
  }

  int new_size = _states->size();
  return orig_size - new_size;
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
  typedef pset<const RenderState *> VisitedStates;
  VisitedStates visited;
  CompositionCycleDesc cycle_desc;

  States::iterator si;
  for (si = _states->begin(); si != _states->end(); ++si) {
    const RenderState *state = (*si);

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
  out << _states->size() << " states:\n";
  States::const_iterator si;
  for (si = _states->begin(); si != _states->end(); ++si) {
    const RenderState *state = (*si);
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
  if (_states->empty()) {
    return true;
  }

  States::const_iterator si = _states->begin();
  States::const_iterator snext = si;
  ++snext;
  nassertr((*si)->get_ref_count() > 0, false);
  while (snext != _states->end()) {
    if (!(*(*si) < *(*snext))) {
      pgraph_cat.error()
        << "RenderStates out of order!\n";
      (*si)->write(pgraph_cat.error(false), 2);
      (*snext)->write(pgraph_cat.error(false), 2);
      return false;
    }
    si = snext;
    ++snext;
    nassertr((*si)->get_ref_count() > 0, false);
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
  if (get_render_mode() != (const RenderModeAttrib *)NULL) {
    geom_rendering = _render_mode->get_geom_rendering(geom_rendering);
  }
  if (get_tex_gen() != (const TexGenAttrib *)NULL) {
    geom_rendering = _tex_gen->get_geom_rendering(geom_rendering);
  }
  if (get_tex_matrix() != (const TexMatrixAttrib *)NULL) {
    geom_rendering = _tex_matrix->get_geom_rendering(geom_rendering);
  }

  return geom_rendering;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::store_into_slots
//       Access: Public
//  Description: Convert the attribute list into an AttribSlots.
////////////////////////////////////////////////////////////////////
void RenderState::
store_into_slots(AttribSlots *output) const {
  Attributes::const_iterator ai;
  for (ai = _attributes.begin(); ai != _attributes.end(); ai++) {
    (*ai)._attrib->store_into_slot(output);
  }
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
//     Function: RenderState::return_new
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
return_new(RenderState *state) {
  nassertr(state != (RenderState *)NULL, state);

  // This should be a newly allocated pointer, not one that was used
  // for anything else.
  nassertr(state->_saved_entry == _states->end(), state);

#ifndef NDEBUG
  if (paranoid_const) {
    nassertr(validate_states(), state);
  }
#endif

  // Save the state in a local PointerTo so that it will be freed at
  // the end of this function if no one else uses it.
  CPT(RenderState) pt_state = state;

  pair<States::iterator, bool> result = _states->insert(state);

  if (result.second) {
    // The state was inserted; save the iterator and return the
    // input state.
    state->_saved_entry = result.first;
    return pt_state;
  }
  
  // The state was not inserted; there must be an equivalent one
  // already in the set.  Return that one.
  return *(result.first);
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

  // First, build a new Attributes member that represents the union of
  // this one and that one.
  Attributes::const_iterator ai = _attributes.begin();
  Attributes::const_iterator bi = other->_attributes.begin();

  // Create a new RenderState that will hold the result.
  RenderState *new_state = new RenderState;
  back_insert_iterator<Attributes> result = 
    back_inserter(new_state->_attributes);

  while (ai != _attributes.end() && bi != other->_attributes.end()) {
    if ((*ai) < (*bi)) {
      // Here is an attribute that we have in the original, which is
      // not present in the secondary.
      *result = *ai;
      ++ai;
      ++result;
    } else if ((*bi) < (*ai)) {
      // Here is a new attribute we have in the secondary, that was
      // not present in the original.
      *result = *bi;
      ++bi;
      ++result;
    } else {
      // Here is an attribute we have in both.  Does A override B?
      const Attribute &a = (*ai);
      const Attribute &b = (*bi);
      if (b._override < a._override) {
        // A, the higher RenderAttrib, overrides.
        *result = *ai;

      } else if (a._override < b._override && 
                 a._attrib->lower_attrib_can_override()) {
        // B, the lower RenderAttrib, overrides.  This is a special
        // case; normally, a lower RenderAttrib does not override a
        // higher one, even if it has a higher override value.  But
        // certain kinds of RenderAttribs redefine
        // lower_attrib_can_override() to return true, allowing this
        // override.
        *result = *bi;

      } else {
        // Either they have the same override value, or B is higher.
        // In either case, the result is the composition of the two,
        // with B's override value.
        *result = Attribute(a._attrib->compose(b._attrib), b._override);
      }
      ++ai;
      ++bi;
      ++result;
    }
  }

  while (ai != _attributes.end()) {
    *result = *ai;
    ++ai;
    ++result;
  }

  while (bi != other->_attributes.end()) {
    *result = *bi;
    ++bi;
    ++result;
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

  Attributes::const_iterator ai = _attributes.begin();
  Attributes::const_iterator bi = other->_attributes.begin();

  // Create a new RenderState that will hold the result.
  RenderState *new_state = new RenderState;
  back_insert_iterator<Attributes> result = 
    back_inserter(new_state->_attributes);

  while (ai != _attributes.end() && bi != other->_attributes.end()) {
    if ((*ai) < (*bi)) {
      // Here is an attribute that we have in the original, which is
      // not present in the secondary.
      *result = Attribute((*ai)._attrib->invert_compose((*ai)._attrib->make_default()), 0);
      ++ai;
      ++result;
    } else if ((*bi) < (*ai)) {
      // Here is a new attribute we have in the secondary, that was
      // not present in the original.
      *result = *bi;
      ++bi;
      ++result;
    } else {
      // Here is an attribute we have in both.  In this case, override
      // is meaningless.
      *result = Attribute((*ai)._attrib->invert_compose((*bi)._attrib), (*bi)._override);
      ++ai;
      ++bi;
      ++result;
    }
  }

  while (ai != _attributes.end()) {
    *result = Attribute((*ai)._attrib->invert_compose((*ai)._attrib->make_default()), 0);
    ++ai;
    ++result;
  }

  while (bi != other->_attributes.end()) {
    *result = *bi;
    ++bi;
    ++result;
  }

  return return_new(new_state);
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
    
  CompositionCache::const_iterator ci;
  for (ci = current_state->_composition_cache.begin();
       ci != current_state->_composition_cache.end();
       ++ci) {
    const RenderState *result = (*ci).second._result;
    if (result != (const RenderState *)NULL) {
      if (r_detect_cycles(start_state, result, length + 1, 
                          this_seq, cycle_desc)) {
        // Cycle detected.
        if (cycle_desc != (CompositionCycleDesc *)NULL) {
          CompositionCycleDescEntry entry((*ci).first, result, false);
          cycle_desc->push_back(entry);
        }
        return true;
      }
    }
  }

  for (ci = current_state->_invert_composition_cache.begin();
       ci != current_state->_invert_composition_cache.end();
       ++ci) {
    const RenderState *result = (*ci).second._result;
    if (result != (const RenderState *)NULL) {
      if (r_detect_cycles(start_state, result, length + 1,
                          this_seq, cycle_desc)) {
        // Cycle detected.
        if (cycle_desc != (CompositionCycleDesc *)NULL) {
          CompositionCycleDescEntry entry((*ci).first, result, true);
          cycle_desc->push_back(entry);
        }
        return true;
      }
    }
  }

  // No cycle detected.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::remove_cache_pointers
//       Access: Private
//  Description: Remove all pointers within the cache from and to this
//               particular RenderState.  The pointers to this
//               object may be scattered around in the various
//               CompositionCaches from other RenderState objects.
////////////////////////////////////////////////////////////////////
void RenderState::
remove_cache_pointers() {
  // Now make sure we clean up all other floating pointers to the
  // RenderState.  These may be scattered around in the various
  // CompositionCaches from other RenderState objects.

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
  if (_composition_cache.empty() && _invert_composition_cache.empty()) {
    return;
  }
  PStatTimer timer(_cache_update_pcollector);
#endif  // DO_PSTATS

  // There are lots of ways to do this loop wrong.  Be very careful if
  // you need to modify it for any reason.
  while (!_composition_cache.empty()) {
    CompositionCache::iterator ci = _composition_cache.begin();

    // It is possible that the "other" RenderState object is
    // currently within its own destructor.  We therefore can't use a
    // PT() to hold its pointer; that could end up calling its
    // destructor twice.  Fortunately, we don't need to hold its
    // reference count to ensure it doesn't destruct while we process
    // this loop; as long as we ensure that no *other* RenderState
    // objects destruct, there will be no reason for that one to.
    RenderState *other = (RenderState *)(*ci).first;

    // We hold a copy of the composition result so we can dereference
    // it later.
    Composition comp = (*ci).second;

    // Now we can remove the element from our cache.  We do this now,
    // rather than later, before any other RenderState objects have
    // had a chance to destruct, so we are confident that our iterator
    // is still valid.
    _composition_cache.erase(ci);

    if (other != this) {
      CompositionCache::iterator oci = other->_composition_cache.find(this);

      // We may or may not still be listed in the other's cache (it
      // might be halfway through pulling entries out, from within its
      // own destructor).
      if (oci != other->_composition_cache.end()) {
        // Hold a copy of the other composition result, too.
        Composition ocomp = (*oci).second;
        
        other->_composition_cache.erase(oci);
        
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
  while (!_invert_composition_cache.empty()) {
    CompositionCache::iterator ci = _invert_composition_cache.begin();
    RenderState *other = (RenderState *)(*ci).first;
    nassertv(other != this);
    Composition comp = (*ci).second;
    _invert_composition_cache.erase(ci);
    if (other != this) {
      CompositionCache::iterator oci = 
        other->_invert_composition_cache.find(this);
      if (oci != other->_invert_composition_cache.end()) {
        Composition ocomp = (*oci).second;
        other->_invert_composition_cache.erase(oci);
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
  string bin_name;
  _draw_order = 0;

  const CullBinAttrib *bin_attrib = get_bin();
  if (bin_attrib != (const CullBinAttrib *)NULL) {
    bin_name = bin_attrib->get_bin_name();
    _draw_order = bin_attrib->get_draw_order();
  }

  if (bin_name.empty()) {
    // No explicit bin is specified; put in the in the default bin,
    // either opaque or transparent, based on the transparency
    // setting.
    bin_name = "opaque";
    const TransparencyAttrib *trans = get_transparency();
    if (trans != (const TransparencyAttrib *)NULL) {
      switch (trans->get_mode()) {
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
//     Function: RenderState::determine_fog
//       Access: Private
//  Description: This is the private implementation of get_fog().
////////////////////////////////////////////////////////////////////
void RenderState::
determine_fog() {
  const RenderAttrib *attrib = get_attrib(FogAttrib::get_class_type());
  _fog = (const FogAttrib *)NULL;
  if (attrib != (const RenderAttrib *)NULL) {
    _fog = DCAST(FogAttrib, attrib);
  }
  _flags |= F_checked_fog;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::determine_bin
//       Access: Private
//  Description: This is the private implementation of get_bin().
////////////////////////////////////////////////////////////////////
void RenderState::
determine_bin() {
  const RenderAttrib *attrib = get_attrib(CullBinAttrib::get_class_type());
  _bin = (const CullBinAttrib *)NULL;
  if (attrib != (const RenderAttrib *)NULL) {
    _bin = DCAST(CullBinAttrib, attrib);
  }
  _flags |= F_checked_bin;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::determine_transparency
//       Access: Private
//  Description: This is the private implementation of get_transparency().
////////////////////////////////////////////////////////////////////
void RenderState::
determine_transparency() {
  const RenderAttrib *attrib = 
    get_attrib(TransparencyAttrib::get_class_type());
  _transparency = (const TransparencyAttrib *)NULL;
  if (attrib != (const RenderAttrib *)NULL) {
    _transparency = DCAST(TransparencyAttrib, attrib);
  }
  _flags |= F_checked_transparency;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::determine_color
//       Access: Private
//  Description: This is the private implementation of get_color().
////////////////////////////////////////////////////////////////////
void RenderState::
determine_color() {
  const RenderAttrib *attrib = get_attrib(ColorAttrib::get_class_type());
  _color = (const ColorAttrib *)NULL;
  if (attrib != (const RenderAttrib *)NULL) {
    _color = DCAST(ColorAttrib, attrib);
  }
  _flags |= F_checked_color;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::determine_color_scale
//       Access: Private
//  Description: This is the private implementation of get_color_scale().
////////////////////////////////////////////////////////////////////
void RenderState::
determine_color_scale() {
  const RenderAttrib *attrib = get_attrib(ColorScaleAttrib::get_class_type());
  _color_scale = (const ColorScaleAttrib *)NULL;
  if (attrib != (const RenderAttrib *)NULL) {
    _color_scale = DCAST(ColorScaleAttrib, attrib);
  }
  _flags |= F_checked_color_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::determine_texture
//       Access: Private
//  Description: This is the private implementation of get_texture().
////////////////////////////////////////////////////////////////////
void RenderState::
determine_texture() {
  const RenderAttrib *attrib = get_attrib(TextureAttrib::get_class_type());
  _texture = (const TextureAttrib *)NULL;
  if (attrib != (const RenderAttrib *)NULL) {
    _texture = DCAST(TextureAttrib, attrib);
  }
  _flags |= F_checked_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::determine_tex_gen
//       Access: Private
//  Description: This is the private implementation of get_tex_gen().
////////////////////////////////////////////////////////////////////
void RenderState::
determine_tex_gen() {
  const RenderAttrib *attrib = get_attrib(TexGenAttrib::get_class_type());
  _tex_gen = (const TexGenAttrib *)NULL;
  if (attrib != (const RenderAttrib *)NULL) {
    _tex_gen = DCAST(TexGenAttrib, attrib);
  }
  _flags |= F_checked_tex_gen;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::determine_tex_matrix
//       Access: Private
//  Description: This is the private implementation of get_tex_matrix().
////////////////////////////////////////////////////////////////////
void RenderState::
determine_tex_matrix() {
  const RenderAttrib *attrib = get_attrib(TexMatrixAttrib::get_class_type());
  _tex_matrix = (const TexMatrixAttrib *)NULL;
  if (attrib != (const RenderAttrib *)NULL) {
    _tex_matrix = DCAST(TexMatrixAttrib, attrib);
  }
  _flags |= F_checked_tex_matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::determine_render_mode
//       Access: Private
//  Description: This is the private implementation of get_render_mode().
////////////////////////////////////////////////////////////////////
void RenderState::
determine_render_mode() {
  const RenderAttrib *attrib = get_attrib(RenderModeAttrib::get_class_type());
  _render_mode = (const RenderModeAttrib *)NULL;
  if (attrib != (const RenderAttrib *)NULL) {
    _render_mode = DCAST(RenderModeAttrib, attrib);
  }
  _flags |= F_checked_render_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::determine_clip_plane
//       Access: Private
//  Description: This is the private implementation of get_clip_plane().
////////////////////////////////////////////////////////////////////
void RenderState::
determine_clip_plane() {
  const RenderAttrib *attrib = get_attrib(ClipPlaneAttrib::get_class_type());
  _clip_plane = (const ClipPlaneAttrib *)NULL;
  if (attrib != (const RenderAttrib *)NULL) {
    _clip_plane = DCAST(ClipPlaneAttrib, attrib);
  }
  _flags |= F_checked_clip_plane;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::determine_shader
//       Access: Private
//  Description: This is the private implementation of get_shader().
////////////////////////////////////////////////////////////////////
void RenderState::
determine_shader() {
  const RenderAttrib *attrib = get_attrib(ShaderAttrib::get_class_type());
  _shader = (const ShaderAttrib *)NULL;
  if (attrib != (const RenderAttrib *)NULL) {
    _shader = DCAST(ShaderAttrib, attrib);
  }
  _flags |= F_checked_shader;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::determine_cull_callback
//       Access: Private
//  Description: This is the private implementation of has_cull_callback().
////////////////////////////////////////////////////////////////////
void RenderState::
determine_cull_callback() {
  Attributes::const_iterator ai;
  for (ai = _attributes.begin(); ai != _attributes.end(); ++ai) {
    const Attribute &attrib = *ai;
    if (attrib._attrib->has_cull_callback()) {
      _flags |= F_has_cull_callback;
      break;
    }
  }

  _flags |= F_checked_cull_callback;
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

  int num_attribs = _attributes.size();
  nassertv(num_attribs == (int)(PN_uint16)num_attribs);
  dg.add_uint16(num_attribs);

  // **** We should smarten up the writing of the override
  // number--most of the time these will all be zero.
  Attributes::const_iterator ai;
  for (ai = _attributes.begin(); ai != _attributes.end(); ++ai) {
    const Attribute &attribute = (*ai);

    manager->write_pointer(dg, attribute._attrib);
    dg.add_int32(attribute._override);
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

  // Get the attribute pointers.
  for (size_t i = 0; i < _attributes.size(); ++i) {
    Attribute &attribute = _attributes[i];

    TypedWritable *ptr = p_list[pi++];
    while (ptr == NULL && i < _attributes.size()) {
      // This is an attribute that we weren't able to load from the
      // bam file.  Remove it.
      _attributes.pop_back();
      ptr = p_list[pi++];
    }
    if (i < _attributes.size()) {
      attribute._attrib = DCAST(RenderAttrib, ptr);
      attribute._type = attribute._attrib->get_type();
    }
  }

  // Now make sure the array is properly sorted.  (It won't
  // necessarily preserve its correct sort after being read from bam,
  // because the sort is based on TypeHandle indices and raw pointers,
  // both of which can change from session to session.)
  _attributes.sort();

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
  CPT(RenderState) pointer = return_new(state);

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

  // Push back a NULL pointer for each attribute for now, until we get
  // the actual list of pointers later in complete_pointers().
  _attributes.reserve(num_attribs);
  for (int i = 0; i < num_attribs; i++) {
    manager->read_pointer(scan);
    int override = scan.get_int32();
    _attributes.push_back(Attribute(override));
  }
}
