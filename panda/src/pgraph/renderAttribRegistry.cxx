/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderAttribRegistry.cxx
 * @author drose
 * @date 2008-11-13
 */

#include "renderAttribRegistry.h"
#include "renderAttrib.h"
#include "renderState.h"
#include "deletedChain.h"

RenderAttribRegistry *RenderAttribRegistry::_global_ptr;

/**
 *
 */
RenderAttribRegistry::
RenderAttribRegistry() {
  _registry.reserve(_max_slots);
  _sorted_slots.reserve(_max_slots);

  // Reserve slot 0 for TypeHandle::none(), and for types that exceed
  // max_slots.
  _registry.push_back(RegistryNode(TypeHandle::none(), 0, nullptr));
}

/**
 *
 */
RenderAttribRegistry::
~RenderAttribRegistry() {
}

/**
 * Adds the indicated TypeHandle to the registry, if it is not there already,
 * and returns a unique slot number in the range 0 < slot < get_max_slots().
 *
 * The sort value is an arbitrary integer.  In general, the RenderAttribs will
 * be sorted in order from lowest sort value to highest sort value, when they
 * are traversed via the get_num_sorted_slots() get_sorted_slot() methods.
 * This will be used to sort render states, so that heavier RenderAttribs are
 * changed less frequently.  In general, you should choose sort values such
 * that the heavier RenderAttribs (that is, those which are more expensive to
 * change) have lower sort values.
 *
 * The default_attrib pointer should be a newly created instance of this
 * attribute that represents the default state for this attribute.
 *
 * register_slot() is intended to be called at application start for each
 * different RenderAttrib type in the system, to assign a different integer
 * slot number to each one.
 */
int RenderAttribRegistry::
register_slot(TypeHandle type_handle, int sort, RenderAttrib *default_attrib) {
  // Sanity check; if this triggers, you either passed a wrong argument, or
  // you didn't use the type system correctly.
  nassertr(default_attrib->get_type() == type_handle, 0);

  int type_index = type_handle.get_index();
  while (type_index >= (int)_slots_by_type.size()) {
    _slots_by_type.push_back(0);
  }

  if (_slots_by_type[type_index] != 0) {
    // This type has already been registered.
    return _slots_by_type[type_index];
  }

  int slot = (int)_registry.size();
  if (slot >= _max_slots) {
    pgraph_cat->error()
      << "Too many registered RenderAttribs; not registering "
      << type_handle << "\n";
    nassert_raise("out of RenderAttrib slots");
    return 0;
  }

  // Register the default attribute.  We don't use return_unique and register
  // it even if the state cache is disabled, because we can't read the
  // state_cache config variable yet at this time.  It probably doesn't hurt
  // to have these 32 entries around in the attrib cache.
  if (default_attrib != nullptr) {
    default_attrib->calc_hash();

    if (default_attrib->_saved_entry == -1) {
      // If this attribute was already registered, something odd is going on.
      nassertr(RenderAttrib::_attribs.find(default_attrib) == -1, 0);
      default_attrib->_saved_entry =
        RenderAttrib::_attribs.store(default_attrib, nullptr);
    }

    // It effectively lives forever.  Might as well make it official.
    default_attrib->local_object();
  }

  _slots_by_type[type_index] = slot;

  _registry.push_back(RegistryNode(type_handle, sort, default_attrib));

  _sorted_slots.push_back(slot);
  std::sort(_sorted_slots.begin(), _sorted_slots.end(), SortSlots(this));

  return slot;
}

/**
 * Changes the sort number associated with slot n.
 */
void RenderAttribRegistry::
set_slot_sort(int slot, int sort) {
  nassertv(slot >= 0 && slot < (int)_registry.size());
  _registry[slot]._sort = sort;

  // Re-sort the slot list.
  _sorted_slots.clear();
  for (int i = 1; i < (int)_registry.size(); ++i) {
    _sorted_slots.push_back(i);
  }
  std::sort(_sorted_slots.begin(), _sorted_slots.end(), SortSlots(this));
}

/**
 *
 */
void RenderAttribRegistry::
init_global_ptr() {
  _global_ptr = new RenderAttribRegistry;
}
