// Filename: sliderTable.I
// Created by:  drose (28Mar05)
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
//     Function: SliderTable::is_registered
//       Access: Published
//  Description: Returns true if this table has been registered.
//               Once it has been registered, the set of sliders in
//               a SliderTable may not be further modified; but
//               it must be registered before it can be assigned to a
//               Geom.
////////////////////////////////////////////////////////////////////
INLINE bool SliderTable::
is_registered() const {
  return _is_registered;
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::register_table
//       Access: Published, Static
//  Description: Registers a SliderTable for use.  This is
//               similar to GeomVertexFormat::register_format().  Once
//               registered, a SliderTable may no longer be
//               modified (although the individual VertexSlider
//               objects may modify their reported sliders).
//
//               This must be called before a table may be used in a
//               Geom.  After this call, you should discard the
//               original pointer you passed in (which may or may not
//               now be invalid) and let its reference count decrement
//               normally; you should use only the returned value from
//               this point on.
////////////////////////////////////////////////////////////////////
INLINE CPT(SliderTable) SliderTable::
register_table(const SliderTable *table) {
  // We don't actually bother adding the table object to a registry.
  // This means there may be multiple copies of identical registered
  // SliderTables.  Big deal.  We can always go back and make a
  // registry later if we really need it.
  if (table->is_registered()) {
    return table;
  }

  ((SliderTable *)table)->do_register();
  return table;
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::get_num_sliders
//       Access: Published
//  Description: Returns the number of sliders in the table.
////////////////////////////////////////////////////////////////////
INLINE int SliderTable::
get_num_sliders() const {
  return _sliders.size();
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::get_slider
//       Access: Published
//  Description: Returns the nth slider in the table.
////////////////////////////////////////////////////////////////////
INLINE const VertexSlider *SliderTable::
get_slider(int n) const {
  nassertr(n >= 0 && n < (int)_sliders.size(), NULL);
  return _sliders[n]._slider;
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::get_slider_rows
//       Access: Published
//  Description: Returns the set of rows (vertices) governed by the
//               nth slider in the table.
////////////////////////////////////////////////////////////////////
INLINE const SparseArray &SliderTable::
get_slider_rows(int n) const {
  nassertr(n >= 0 && n < (int)_sliders.size(), _empty_array);
  return _sliders[n]._rows;
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::find_sliders
//       Access: Published
//  Description: Returns a list of slider indices that represent the
//               list of sliders with the indicated name, or an empty
//               SparseArray if no slider in the table has that name.
////////////////////////////////////////////////////////////////////
INLINE const SparseArray &SliderTable::
find_sliders(const InternalName *name) const {
  SlidersByName::const_iterator sni;
  sni = _sliders_by_name.find(name);
  if (sni != _sliders_by_name.end()) {
    return (*sni).second;
  }
  return _empty_array;
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::has_slider
//       Access: Published
//  Description: Returns true if the table has at least one slider by
//               the indicated name, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool SliderTable::
has_slider(const InternalName *name) const {
  return (!find_sliders(name).is_zero());
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::is_empty
//       Access: Published
//  Description: Returns true if the table has no sliders, false if it
//               has at least one.
////////////////////////////////////////////////////////////////////
INLINE bool SliderTable::
is_empty() const {
  return _sliders.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::get_modified
//       Access: Published
//  Description: Returns a sequence number that's guaranteed to change
//               at least when any VertexSliders in the table
//               change.  (However, this is only true for a registered
//               table.  An unregistered table may or may not
//               reflect an update here when a VertexSlider
//               changes.)
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq SliderTable::
get_modified(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::update_modified
//       Access: Private
//  Description: Called internally whenever a nested VertexSlider
//               reports that it has been modified.
////////////////////////////////////////////////////////////////////
INLINE void SliderTable::
update_modified(UpdateSeq modified, Thread *current_thread) {
  CDWriter cdata(_cycler, true, current_thread);
  cdata->_modified = modified;
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SliderTable::CData::
CData() {
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SliderTable::CData::
CData(const SliderTable::CData &copy) :
  _modified(copy._modified)
{
}
