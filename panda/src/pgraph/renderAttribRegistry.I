// Filename: renderAttribRegistry.I
// Created by:  drose (13Nov08)
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
//     Function: RenderAttribRegistry::get_slot
//       Access: Published
//  Description: Returns the slot number assigned to the indicated
//               TypeHandle, or 0 if no slot number has been assigned.
////////////////////////////////////////////////////////////////////
INLINE int RenderAttribRegistry::
get_slot(TypeHandle type_handle) const {
  int type_index = type_handle.get_index();
  if (type_index >= (int)_slots_by_type.size()) {
    return 0;
  }
  return _slots_by_type[type_index];
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttribRegistry::get_max_slots
//       Access: Published
//  Description: Returns the maximum number that any slot number is
//               allowed to grow.  Actually, this number will be one
//               higher than the highest possible slot number.  This
//               puts an upper bound on the number of RenderAttrib
//               slots that may be allocated, and allows other code to
//               define an array of slots.
//
//               This number will not change during the lifetime of
//               the application.
////////////////////////////////////////////////////////////////////
INLINE int RenderAttribRegistry::
get_max_slots() const {
  return _max_slots;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttribRegistry::get_num_slots
//       Access: Published
//  Description: Returns the number of RenderAttrib slots that have
//               been allocated.  This is one more than the highest
//               slot number in use.
////////////////////////////////////////////////////////////////////
INLINE int RenderAttribRegistry::
get_num_slots() const {
  return _registry.size();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttribRegistry::get_slot_type
//       Access: Published
//  Description: Returns the TypeHandle associated with slot n.
////////////////////////////////////////////////////////////////////
INLINE TypeHandle RenderAttribRegistry::
get_slot_type(int slot) const {
  nassertr(slot >= 0 && slot < (int)_registry.size(), TypeHandle::none());
  return _registry[slot]._type;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttribRegistry::get_slot_sort
//       Access: Published
//  Description: Returns the sort number associated with slot n.
////////////////////////////////////////////////////////////////////
INLINE int RenderAttribRegistry::
get_slot_sort(int slot) const {
  nassertr(slot >= 0 && slot < (int)_registry.size(), 0);
  return _registry[slot]._sort;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttribRegistry::get_num_sorted_slots
//       Access: Published
//  Description: Returns the number of entries in the sorted_slots
//               list.
////////////////////////////////////////////////////////////////////
INLINE int RenderAttribRegistry::
get_num_sorted_slots() const {
  return _sorted_slots.size();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttribRegistry::get_sorted_slot
//       Access: Published
//  Description: Returns the nth slot in sorted order.  By traversing
//               this list, you will retrieve all the slot numbers in
//               order according to their registered sort value.
////////////////////////////////////////////////////////////////////
INLINE int RenderAttribRegistry::
get_sorted_slot(int n) const {
  nassertr(n >= 0 && n < (int)_sorted_slots.size(), 0);
  return _sorted_slots[n];
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttribRegistry::get_array_chain
//       Access: Published
//  Description: Returns the DeletedBufferChain object that may be
//               used to allocated appropriately-sized arrays of
//               RenderState::Attribute objects.
////////////////////////////////////////////////////////////////////
INLINE DeletedBufferChain *RenderAttribRegistry::
get_array_chain() const {
  return _array_chain;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttribRegistry::get_global_ptr
//       Access: Published, Static
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RenderAttribRegistry *RenderAttribRegistry::
get_global_ptr() {
  if (_global_ptr == (RenderAttribRegistry *)NULL) {
    init_global_ptr();
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttribRegistry::quick_get_global_ptr
//       Access: Public, Static
//  Description: Returns the global_ptr without first ensuring it has
//               been initialized.  Only safe for code that knows it
//               has already been initialized.
////////////////////////////////////////////////////////////////////
INLINE RenderAttribRegistry *RenderAttribRegistry::
quick_get_global_ptr() {
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttribRegistry::SortSlots::Constructor
//       Access: Public
//  Description: This is an STL function object for sorting the
//               _sorted_slots list into order by slot sort number.
////////////////////////////////////////////////////////////////////
INLINE RenderAttribRegistry::SortSlots::
SortSlots(RenderAttribRegistry *reg) : _reg(reg) {
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttribRegistry::SortSlots::operator ()
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool RenderAttribRegistry::SortSlots::
operator () (int a, int b) const {
  return _reg->get_slot_sort(a) < _reg->get_slot_sort(b);
}
