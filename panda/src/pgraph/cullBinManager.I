// Filename: cullBinManager.I
// Created by:  drose (28Feb02)
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
//     Function: CullBinManager::SortBins::Constructor
//       Access: Public
//  Description: This is a function object whose sole purpose is to
//               put the _sorted_bins vector in the proper order for
//               rendering the bins.
////////////////////////////////////////////////////////////////////
INLINE CullBinManager::SortBins::
SortBins(CullBinManager *manager) :
  _manager(manager)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::SortBins::operator ()
//       Access: Public
//  Description: The function call method of the function object.
//               Returns true if the two bin indices are already in
//               sorted order with a < b, or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool CullBinManager::SortBins::
operator () (int a, int b) const {
  return _manager->_bin_definitions[a]._sort < _manager->_bin_definitions[b]._sort;
}



////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::get_num_bins
//       Access: Published
//  Description: Returns the number of bins in the world.
////////////////////////////////////////////////////////////////////
INLINE int CullBinManager::
get_num_bins() const {
  // We quietly sort the bins in order if they are not already sorted.
  // This is a non-const operation, but we pretend it's const because
  // it's intended to be a transparent update.
  if (!_bins_are_sorted) {
    ((CullBinManager *)this)->do_sort_bins();
  }
  return _sorted_bins.size();
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::get_bin
//       Access: Published
//  Description: Returns the bin_index of the nth bin in the set,
//               where n is a number between 0 and get_num_bins().
//               This returns the list of bin_index numbers, in sorted
//               order (that is, in the order in which the bins should
//               be rendered).
////////////////////////////////////////////////////////////////////
INLINE int CullBinManager::
get_bin(int n) const {
  nassertr(n >= 0 && n < (int)_sorted_bins.size(), -1);
  return _sorted_bins[n];
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::get_bin_name
//       Access: Published
//  Description: Returns the name of the bin with the indicated
//               bin_index (where bin_index was retrieved by get_bin()
//               or find_bin()).  The bin's name may not be changed
//               during the life of the bin.
////////////////////////////////////////////////////////////////////
INLINE string CullBinManager::
get_bin_name(int bin_index) const {
  nassertr(bin_index >= 0 && bin_index < (int)_bin_definitions.size(), string());
  nassertr(_bin_definitions[bin_index]._in_use, string());
  return _bin_definitions[bin_index]._name;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::get_bin_type
//       Access: Published
//  Description: Returns the type of the bin with the indicated
//               bin_index (where bin_index was retrieved by get_bin()
//               or find_bin()).
////////////////////////////////////////////////////////////////////
INLINE CullBinManager::BinType CullBinManager::
get_bin_type(int bin_index) const {
  nassertr(bin_index >= 0 && bin_index < (int)_bin_definitions.size(), BT_invalid);
  nassertr(_bin_definitions[bin_index]._in_use, BT_invalid);
  return _bin_definitions[bin_index]._type;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::get_bin_type
//       Access: Published
//  Description: Returns the type of the bin with the indicated
//               name.
////////////////////////////////////////////////////////////////////
INLINE CullBinManager::BinType CullBinManager::
get_bin_type(const string &name) const {
  int bin_index = find_bin(name);
  nassertr(bin_index != -1, BT_invalid);
  return get_bin_type(bin_index);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::set_bin_type
//       Access: Published
//  Description: Changes the type of the bin with the indicated
//               bin_index (where bin_index was retrieved by get_bin()
//               or find_bin()).
//
//               The change might be effective immediately, or it
//               might take place next frame, depending on the bin
//               type.
////////////////////////////////////////////////////////////////////
INLINE void CullBinManager::
set_bin_type(int bin_index, CullBinManager::BinType type) {
  nassertv(bin_index >= 0 && bin_index < (int)_bin_definitions.size());
  nassertv(_bin_definitions[bin_index]._in_use);
  _bin_definitions[bin_index]._type = type;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::set_bin_type
//       Access: Published
//  Description: Changes the type of the bin with the indicated
//               name.
//
//               The change might be effective immediately, or it
//               might take place next frame, depending on the bin
//               type.
////////////////////////////////////////////////////////////////////
INLINE void CullBinManager::
set_bin_type(const string &name, CullBinManager::BinType type) {
  int bin_index = find_bin(name);
  nassertv(bin_index != -1);
  set_bin_type(bin_index, type);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::get_bin_sort
//       Access: Published
//  Description: Returns the sort order of the bin with the indicated
//               bin_index (where bin_index was retrieved by get_bin()
//               or find_bin()).
//
//               The bins are rendered in increasing order by their
//               sort order; this number may be changed from time to
//               time to reorder the bins.
////////////////////////////////////////////////////////////////////
INLINE int CullBinManager::
get_bin_sort(int bin_index) const {
  nassertr(bin_index >= 0 && bin_index < (int)_bin_definitions.size(), 0);
  nassertr(_bin_definitions[bin_index]._in_use, 0);
  return _bin_definitions[bin_index]._sort;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::get_bin_sort
//       Access: Published
//  Description: Returns the sort order of the bin with the indicated
//               name.
//
//               The bins are rendered in increasing order by their
//               sort order; this number may be changed from time to
//               time to reorder the bins.
////////////////////////////////////////////////////////////////////
INLINE int CullBinManager::
get_bin_sort(const string &name) const {
  int bin_index = find_bin(name);
  nassertr(bin_index != -1, 0);
  return get_bin_sort(bin_index);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::set_bin_sort
//       Access: Published
//  Description: Changes the sort order of the bin with the indicated
//               bin_index (where bin_index was retrieved by get_bin()
//               or find_bin()).
//
//               The bins are rendered in increasing order by their
//               sort order; this number may be changed from time to
//               time to reorder the bins.
////////////////////////////////////////////////////////////////////
INLINE void CullBinManager::
set_bin_sort(int bin_index, int sort) {
  nassertv(bin_index >= 0 && bin_index < (int)_bin_definitions.size());
  nassertv(_bin_definitions[bin_index]._in_use);
  _bin_definitions[bin_index]._sort = sort;
  _bins_are_sorted = false;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::set_bin_sort
//       Access: Published
//  Description: Changes the sort order of the bin with the indicated
//               name.
//
//               The bins are rendered in increasing order by their
//               sort order; this number may be changed from time to
//               time to reorder the bins.
////////////////////////////////////////////////////////////////////
INLINE void CullBinManager::
set_bin_sort(const string &name, int sort) {
  int bin_index = find_bin(name);
  nassertv(bin_index != -1);
  set_bin_sort(bin_index, sort);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::get_bin_active
//       Access: Published
//  Description: Returns the active flag of the bin with the indicated
//               bin_index (where bin_index was retrieved by get_bin()
//               or find_bin()).
//
//               When a bin is marked inactive, all geometry assigned
//               to it is not rendered.
////////////////////////////////////////////////////////////////////
INLINE bool CullBinManager::
get_bin_active(int bin_index) const {
  nassertr(bin_index >= 0 && bin_index < (int)_bin_definitions.size(), false);
  nassertr(_bin_definitions[bin_index]._in_use, false);
  return _bin_definitions[bin_index]._active;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::get_bin_active
//       Access: Published
//  Description: Returns the active flag of the bin with the indicated
//               name.
//
//               When a bin is marked inactive, all geometry assigned
//               to it is not rendered.
////////////////////////////////////////////////////////////////////
INLINE bool CullBinManager::
get_bin_active(const string &name) const {
  int bin_index = find_bin(name);
  nassertr(bin_index != -1, false);
  return get_bin_active(bin_index);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::set_bin_active
//       Access: Published
//  Description: Changes the active flag of the bin with the indicated
//               bin_index (where bin_index was retrieved by get_bin()
//               or find_bin()).
//
//               When a bin is marked inactive, all geometry assigned
//               to it is not rendered.
////////////////////////////////////////////////////////////////////
INLINE void CullBinManager::
set_bin_active(int bin_index, bool active) {
  nassertv(bin_index >= 0 && bin_index < (int)_bin_definitions.size());
  nassertv(_bin_definitions[bin_index]._in_use);
  _bin_definitions[bin_index]._active = active;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::set_bin_active
//       Access: Published
//  Description: Changes the active flag of the bin with the indicated
//               name.
//
//               When a bin is marked inactive, all geometry assigned
//               to it is not rendered.
////////////////////////////////////////////////////////////////////
INLINE void CullBinManager::
set_bin_active(const string &name, bool active) {
  int bin_index = find_bin(name);
  nassertv(bin_index != -1);
  set_bin_active(bin_index, active);
}

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::get_bin_flash_active
//       Access: Published
//  Description: Returns true if the bin with the given bin_index is
//               configured to flash at a predetermined color (where
//               bin_index was retrieved by get_bin() or find_bin()).
//
//               This method is not available in release builds.
////////////////////////////////////////////////////////////////////
INLINE bool CullBinManager::
get_bin_flash_active(int bin_index) const {
  nassertr(bin_index >= 0 && bin_index < (int)_bin_definitions.size(), false);
  return _bin_definitions[bin_index]._flash_active;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::get_bin_flash_color
//       Access: Published
//  Description: Returns the color that this bin has been configured
//               to flash to, if configured.
//
//               This method is not available in release builds.
////////////////////////////////////////////////////////////////////
INLINE const LColor &CullBinManager::
get_bin_flash_color(int bin_index) const {
  nassertr(bin_index >= 0 && bin_index < (int)_bin_definitions.size(), LColor::zero());
  return _bin_definitions[bin_index]._flash_color;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::set_bin_flash_active
//       Access: Published
//  Description: When set to true, the given bin_index is configured
//               to flash at a predetermined color (where
//               bin_index was retrieved by get_bin() or find_bin()).
//
//               This method is not available in release builds.
////////////////////////////////////////////////////////////////////
INLINE void CullBinManager::
set_bin_flash_active(int bin_index, bool active) {
  nassertv(bin_index >= 0 && bin_index < (int)_bin_definitions.size());
  _bin_definitions[bin_index]._flash_active = active;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::set_bin_flash_color
//       Access: Published
//  Description: Changes the flash color for the given bin index.

//               This method is not available in release builds.
////////////////////////////////////////////////////////////////////
INLINE void CullBinManager::
set_bin_flash_color(int bin_index, const LColor &color) {
  nassertv(bin_index >= 0 && bin_index < (int)_bin_definitions.size());
  _bin_definitions[bin_index]._flash_color = color;
}
#endif  // NDEBUG

////////////////////////////////////////////////////////////////////
//     Function: CullBinManager::get_global_ptr
//       Access: Published, Static
//  Description: Returns the pointer to the global CullBinManager
//               object.
////////////////////////////////////////////////////////////////////
INLINE CullBinManager *CullBinManager::
get_global_ptr() {
  if (_global_ptr == (CullBinManager *)NULL) {
    _global_ptr = new CullBinManager;
  }
  return _global_ptr;
}
