/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderAttribRegistry.I
 * @author drose
 * @date 2008-11-13
 */

/**
 * Returns the slot number assigned to the indicated TypeHandle, or 0 if no
 * slot number has been assigned.
 */
INLINE int RenderAttribRegistry::
get_slot(TypeHandle type_handle) const {
  int type_index = type_handle.get_index();
  if (type_index >= (int)_slots_by_type.size()) {
    return 0;
  }
  return _slots_by_type[(size_t)type_index];
}

/**
 * Returns the number of RenderAttrib slots that have been allocated.  This is
 * one more than the highest slot number in use.
 */
INLINE int RenderAttribRegistry::
get_num_slots() const {
  return _registry.size();
}

/**
 * Returns the TypeHandle associated with slot n.
 */
INLINE TypeHandle RenderAttribRegistry::
get_slot_type(int slot) const {
  nassertr(slot >= 0 && slot < (int)_registry.size(), TypeHandle::none());
  return _registry[slot]._type;
}

/**
 * Returns the sort number associated with slot n.
 */
INLINE int RenderAttribRegistry::
get_slot_sort(int slot) const {
  nassertr(slot >= 0 && slot < (int)_registry.size(), 0);
  return _registry[slot]._sort;
}

/**
 * Returns the default RenderAttrib object associated with slot n.  This is
 * the attrib that should be applied in the absence of any other attrib of
 * this type.
 */
INLINE const RenderAttrib *RenderAttribRegistry::
get_slot_default(int slot) const {
  nassertr(slot >= 0 && slot < (int)_registry.size(), 0);
  return _registry[slot]._default_attrib;
}

/**
 * Returns the number of entries in the sorted_slots list.
 */
INLINE int RenderAttribRegistry::
get_num_sorted_slots() const {
  return _sorted_slots.size();
}

/**
 * Returns the nth slot in sorted order.  By traversing this list, you will
 * retrieve all the slot numbers in order according to their registered sort
 * value.
 */
INLINE int RenderAttribRegistry::
get_sorted_slot(int n) const {
  nassertr(n >= 0 && n < (int)_sorted_slots.size(), 0);
  return _sorted_slots[n];
}

/**
 *
 */
INLINE RenderAttribRegistry *RenderAttribRegistry::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    init_global_ptr();
  }
  return _global_ptr;
}

/**
 * Returns the global_ptr without first ensuring it has been initialized.
 * Only safe for code that knows it has already been initialized.
 */
INLINE RenderAttribRegistry *RenderAttribRegistry::
quick_get_global_ptr() {
  return _global_ptr;
}

/**
 * This is an STL function object for sorting the _sorted_slots list into
 * order by slot sort number.
 */
INLINE RenderAttribRegistry::SortSlots::
SortSlots(RenderAttribRegistry *reg) : _reg(reg) {
}

/**
 *
 */
INLINE bool RenderAttribRegistry::SortSlots::
operator () (int a, int b) const {
  return _reg->get_slot_sort(a) < _reg->get_slot_sort(b);
}

/**
 *
 */
INLINE RenderAttribRegistry::RegistryNode::
RegistryNode(TypeHandle type, int sort, const RenderAttrib *default_attrib) :
  _type(type),
  _sort(sort),
  _default_attrib(default_attrib) {
}
