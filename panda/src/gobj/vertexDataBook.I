// Filename: vertexDataBook.I
// Created by:  drose (16May07)
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
//     Function: VertexDataBook::get_num_pages
//       Access: Published
//  Description: Returns the number of pages created for the book.
////////////////////////////////////////////////////////////////////
INLINE int VertexDataBook::
get_num_pages() const {
  return _pages.size();
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBook::get_page
//       Access: Published
//  Description: Returns the nth page created for the book.
////////////////////////////////////////////////////////////////////
INLINE VertexDataPage *VertexDataBook::
get_page(int n) const {
  nassertr(n >= 0 && n < (int)_pages.size(), NULL);
  return _pages[n];
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBook::create_new_page
//       Access: Private
//  Description: Creates a new page of sufficient size to hold the
//               requested block.  The page is not added to the _pages
//               list.
////////////////////////////////////////////////////////////////////
INLINE VertexDataPage *VertexDataBook::
create_new_page(size_t size) {
  size_t page_size = ((size + _block_size - 1) / _block_size) * _block_size;
  return new VertexDataPage(page_size);
}

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
//     Function: VertexDataPage::check_resident
//       Access: Published
//  Description: Forces the vertex data into system RAM, if it is not
//               already there; also, marks it recently-used.
////////////////////////////////////////////////////////////////////
INLINE void VertexDataPage::
check_resident() const {
  if (get_ram_class() != RC_resident) {
    ((VertexDataPage *)this)->make_resident();
  } else {
    ((VertexDataPage *)this)->mark_used_lru();
  }
  nassertv(_size == _uncompressed_size);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::get_first_block
//       Access: Published
//  Description: Returns a pointer to the first allocated block, or
//               NULL if there are no allocated blocks.
////////////////////////////////////////////////////////////////////
INLINE VertexDataBlock *VertexDataPage::
get_first_block() const {
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
//     Function: VertexDataPage::get_page_data
//       Access: Public
//  Description: Returns a pointer to the page's data area.
////////////////////////////////////////////////////////////////////
INLINE unsigned char *VertexDataPage::
get_page_data() const {
  check_resident();
  return _page_data;
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

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBlock::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VertexDataBlock::
VertexDataBlock(VertexDataPage *page, size_t start, size_t size) :
  SimpleAllocatorBlock(page, start, size)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBlock::get_page
//       Access: Published
//  Description: Returns the page from which this buffer was
//               allocated.
////////////////////////////////////////////////////////////////////
INLINE VertexDataPage *VertexDataBlock::
get_page() const {
  return (VertexDataPage *)get_allocator();
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBlock::get_pointer
//       Access: Public
//  Description: Returns a pointer to the start of the allocated
//               memory for this buffer.
////////////////////////////////////////////////////////////////////
INLINE unsigned char *VertexDataBlock::
get_pointer() const {
  nassertr(get_page() != (VertexDataPage *)NULL, NULL);
  return get_page()->get_page_data() + get_start();
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBlock::get_next_block
//       Access: Published
//  Description: Returns a pointer to the next allocated block in the
//               chain, or NULL if there are no more allocated blocks.
////////////////////////////////////////////////////////////////////
INLINE VertexDataBlock *VertexDataBlock::
get_next_block() const {
  return (VertexDataBlock *)SimpleAllocatorBlock::get_next_block();
}
