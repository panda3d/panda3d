// Filename: vertexDataPage.I
// Created by:  drose (04Jun07)
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


////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::get_ram_class
//       Access: Published
//  Description: Returns the current ram class of the array.  If this
//               is other than RC_resident, the array data is not
//               resident in memory.
////////////////////////////////////////////////////////////////////
INLINE VertexDataPage::RamClass VertexDataPage::
get_ram_class() const {
  return _ram_class;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::get_first_block
//       Access: Published
//  Description: Returns a pointer to the first allocated block, or
//               NULL if there are no allocated blocks.
////////////////////////////////////////////////////////////////////
INLINE VertexDataBlock *VertexDataPage::
get_first_block() const {
  MutexHolder holder(_lock);
  check_resident();
  return (VertexDataBlock *)SimpleAllocator::get_first_block();
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::get_total_page_size
//       Access: Published, Static
//  Description: Returns the byte count allocated to all
//               VertexDataPages currently in existance.
////////////////////////////////////////////////////////////////////
INLINE size_t VertexDataPage::
get_total_page_size() {
  return _total_page_size;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::get_global_lru
//       Access: Published, Static
//  Description: Returns a pointer to the global LRU object that
//               manages the VertexDataPage's with the indicated
//               RamClass.
////////////////////////////////////////////////////////////////////
INLINE SimpleLru *VertexDataPage::
get_global_lru(RamClass rclass) {
  nassertr(rclass >= 0 && rclass < RC_end_of_list, NULL);
  return _global_lru[rclass];
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::get_save_file
//       Access: Published, Static
//  Description: Returns the global VertexDataSaveFile that will be
//               used to save vertex data buffers to disk when
//               necessary.
////////////////////////////////////////////////////////////////////
INLINE VertexDataSaveFile *VertexDataPage::
get_save_file() {
  if (_save_file == (VertexDataSaveFile *)NULL) {
    make_save_file();
  }
  return _save_file;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::save_to_disk
//       Access: Published
//  Description: Writes the page to disk, but does not evict it from
//               memory or affect its LRU status.  If it gets evicted
//               later without having been modified, it will not need
//               to write itself to disk again.
////////////////////////////////////////////////////////////////////
INLINE bool VertexDataPage::
save_to_disk() {
  MutexHolder holder(_lock);
  return do_save_to_disk();
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::restore_from_disk
//       Access: Published
//  Description: Restores the page from disk and makes it
//               either compressed or resident (according to whether
//               it was stored compressed on disk).
////////////////////////////////////////////////////////////////////
INLINE void VertexDataPage::
restore_from_disk() {
  MutexHolder holder(_lock);
  do_restore_from_disk();
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::get_page_data
//       Access: Public
//  Description: Returns a pointer to the page's data area.
////////////////////////////////////////////////////////////////////
INLINE unsigned char *VertexDataPage::
get_page_data() const {
  MutexHolder holder(_lock);
  check_resident();
  return _page_data;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::check_resident
//       Access: Private
//  Description: Forces the vertex data into system RAM, if it is not
//               already there; also, marks it recently-used.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
INLINE void VertexDataPage::
check_resident() const {
  if (_ram_class != RC_resident) {
    ((VertexDataPage *)this)->make_resident();
  } else {
    ((VertexDataPage *)this)->mark_used_lru();
  }
  nassertv(_size == _uncompressed_size);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::set_ram_class
//       Access: Private
//  Description: Puts the data in a new ram class.
////////////////////////////////////////////////////////////////////
INLINE void VertexDataPage::
set_ram_class(RamClass rclass) {
  _ram_class = rclass;
  mark_used_lru(_global_lru[rclass]);
}
