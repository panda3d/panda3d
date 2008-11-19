// Filename: renderAttribRegistry.cxx
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

#include "renderAttribRegistry.h"
#include "renderAttrib.h"
#include "renderState.h"
#include "deletedChain.h"

RenderAttribRegistry *RenderAttribRegistry::_global_ptr;

////////////////////////////////////////////////////////////////////
//     Function: RenderAttribRegistry::Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
RenderAttribRegistry::
RenderAttribRegistry() {
  ConfigVariableInt max_attribs
    ("max-attribs", SlotMask::get_max_num_bits(),
     PRC_DESC("This specifies the maximum number of different RenderAttrib "
              "types that may be defined at runtime.  Normally you should "
              "never need to change this, but if the default value is too "
              "low for the number of attribs that Panda actually defines, "
              "you may need to raise this number."));

  // Assign this number once, at startup, and never change it again.
  _max_slots = max((int)max_attribs, 1);
  if (_max_slots > SlotMask::get_max_num_bits()) {
    pgraph_cat->warning()
      << "Value for max-attribs too large: cannot exceed " 
      << SlotMask::get_max_num_bits()
      << " in this build.  To raise this limit, change the typedef "
      << "for SlotMask in renderAttribRegistry.h and recompile.\n";
      
    _max_slots = SlotMask::get_max_num_bits();
  }

  // Get a DeletedBufferChain to manage the arrays of RenderAttribs that are
  // allocated within each RenderState object.
  init_memory_hook();
  _array_chain = memory_hook->get_deleted_chain(_max_slots * sizeof(RenderState::Attribute));

  // Reserve slot 0 for TypeHandle::none(), and for types that exceed
  // max_slots.
  RegistryNode node;
  node._sort = 0;
  node._make_default_func = NULL;
  _registry.push_back(node);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttribRegistry::Destructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
RenderAttribRegistry::
~RenderAttribRegistry() {
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttribRegistry::register_slot
//       Access: Public
//  Description: Adds the indicated TypeHandle to the registry, if it
//               is not there already, and returns a unique slot
//               number in the range 0 < slot < get_max_slots().
//
//               The sort value is an arbitrary integer.  In general,
//               the RenderAttribs will be sorted in order from lowest
//               sort value to highest sort value, when they are
//               traversed via the get_num_sorted_slots() /
//               get_sorted_slot() methods.  This will be used to sort
//               render states, so that heavier RenderAttribs are
//               changed less frequently.  In general, you should
//               choose sort values such that the heavier
//               RenderAttribs (that is, those which are more
//               expensive to change) have lower sort values.
//
//               The make_default_func pointer is a function that may
//               be called to generate a default RenderAttrib to apply
//               in the absence of any other attrib of this type.
//
//               register_slot() is intended to be called at
//               application start for each different RenderAttrib
//               type in the system, to assign a different integer
//               slot number to each one.
////////////////////////////////////////////////////////////////////
int RenderAttribRegistry::
register_slot(TypeHandle type_handle, int sort,
              RenderAttribRegistry::MakeDefaultFunc *make_default_func) {
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
    nassertr(false, 0);
    return 0;
  }

  _slots_by_type[type_index] = slot;

  RegistryNode node;
  node._type = type_handle;
  node._sort = sort;
  node._make_default_func = make_default_func;
  _registry.push_back(node);

  _sorted_slots.push_back(slot);
  ::sort(_sorted_slots.begin(), _sorted_slots.end(), SortSlots(this));

  return slot;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttribRegistry::set_slot_sort
//       Access: Published
//  Description: Changes the sort number associated with slot n.
////////////////////////////////////////////////////////////////////
void RenderAttribRegistry::
set_slot_sort(int slot, int sort) {
  nassertv(slot >= 0 && slot < (int)_registry.size());
  _registry[slot]._sort = sort;

  // Re-sort the slot list.
  _sorted_slots.clear();
  _sorted_slots.reserve(_registry.size() - 1);
  for (int i = 1; i < (int)_registry.size(); ++i) {
    _sorted_slots.push_back(i);
  }
  ::sort(_sorted_slots.begin(), _sorted_slots.end(), SortSlots(this));
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttribRegistry::get_slot_default
//       Access: Published
//  Description: Returns the default RenderAttrib object associated
//               with slot n.  This is the attrib that should be
//               applied in the absence of any other attrib of this
//               type.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) RenderAttribRegistry::
get_slot_default(int slot) const {
  nassertr(slot >= 0 && slot < (int)_registry.size(), 0);
  return (*_registry[slot]._make_default_func)();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttribRegistry::init_global_ptr
//       Access: Private, Static
//  Description:
////////////////////////////////////////////////////////////////////
void RenderAttribRegistry::
init_global_ptr() {
  _global_ptr = new RenderAttribRegistry;
}
