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
  MutexHolder holder(_lock);
  return _ram_class;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::get_pending_ram_class
//       Access: Published
//  Description: Returns the pending ram class of the array.  If this
//               is different from get_ram_class(), this page has been
//               queued to be processed by the thread.  Eventually the
//               page will be set to this ram class.
////////////////////////////////////////////////////////////////////
INLINE VertexDataPage::RamClass VertexDataPage::
get_pending_ram_class() const {
  MutexHolder holder(_lock);
  return _pending_ram_class;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::request_resident
//       Access: Published
//  Description: Ensures that the page will become resident soon.
//               Future calls to get_page_data() will eventually
//               return non-NULL.
////////////////////////////////////////////////////////////////////
INLINE void VertexDataPage::
request_resident() {
  MutexHolder holder(_lock);
  if (_ram_class != RC_resident) {
    request_ram_class(RC_resident);
  }
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
  return (VertexDataBlock *)SimpleAllocator::get_first_block();
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::get_book
//       Access: Published
//  Description: Returns a pointer to the book that owns this page.
////////////////////////////////////////////////////////////////////
INLINE VertexDataBook *VertexDataPage::
get_book() const {
  return _book;
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
//     Function: VertexDataPage::get_pending_lru
//       Access: Published, Static
//  Description: Returns a pointer to the global LRU object that
//               manages the VertexDataPage's that are pending
//               processing by the thread.
////////////////////////////////////////////////////////////////////
INLINE SimpleLru *VertexDataPage::
get_pending_lru() {
  return &_pending_lru;
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
//     Function: VertexDataPage::get_page_data
//       Access: Public
//  Description: Returns a pointer to the page's data area, or NULL if
//               the page is not currently resident.  If the page is
//               not currently resident, this will implicitly request
//               it to become resident soon.
//
//               If force is true, this method will never return NULL,
//               but may block until the page is available.
////////////////////////////////////////////////////////////////////
INLINE unsigned char *VertexDataPage::
get_page_data(bool force) {
  MutexHolder holder(_lock);
  if (_ram_class != RC_resident) {
    if (force) {
      make_resident_now();
    } else {
      request_ram_class(RC_resident);
      if (_ram_class != RC_resident) {
        return NULL;
      }
    }
  }

  mark_used_lru();
  nassertr(_size == _uncompressed_size, _page_data);
  return _page_data;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::set_ram_class
//       Access: Private
//  Description: Puts the data in a new ram class.  Assumes the page
//               lock is already held.
////////////////////////////////////////////////////////////////////
INLINE void VertexDataPage::
set_ram_class(RamClass rclass) {
  _ram_class = rclass;
  mark_used_lru(_global_lru[rclass]);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::PageThread::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VertexDataPage::PageThread::
PageThread() : 
  Thread("VertexDataPage", "VertexDataPage"),
  _shutdown(false),
  _working_cvar(_tlock),
  _pending_cvar(_tlock)
{
}
