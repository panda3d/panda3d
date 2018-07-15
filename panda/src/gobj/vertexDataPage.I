/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexDataPage.I
 * @author drose
 * @date 2007-06-04
 */

/**
 * Returns the current ram class of the array.  If this is other than
 * RC_resident, the array data is not resident in memory.
 */
INLINE VertexDataPage::RamClass VertexDataPage::
get_ram_class() const {
  MutexHolder holder(_lock);
  return _ram_class;
}

/**
 * Returns the pending ram class of the array.  If this is different from
 * get_ram_class(), this page has been queued to be processed by the thread.
 * Eventually the page will be set to this ram class.
 */
INLINE VertexDataPage::RamClass VertexDataPage::
get_pending_ram_class() const {
  MutexHolder holder(_lock);
  return _pending_ram_class;
}

/**
 * Ensures that the page will become resident soon.  Future calls to
 * get_page_data() will eventually return non-NULL.
 */
INLINE void VertexDataPage::
request_resident() {
  MutexHolder holder(_lock);
  if (_ram_class != RC_resident) {
    request_ram_class(RC_resident);
  }
}

/**
 * Allocates a new block.  Returns NULL if a block of the requested size
 * cannot be allocated.
 *
 * To free the allocated block, call block->free(), or simply delete the block
 * pointer.
 */
INLINE VertexDataBlock *VertexDataPage::
alloc(size_t size) {
  MutexHolder holder(_lock);
  return do_alloc(size);
}

/**
 * Returns a pointer to the first allocated block, or NULL if there are no
 * allocated blocks.
 */
INLINE VertexDataBlock *VertexDataPage::
get_first_block() const {
  MutexHolder holder(_lock);
  return (VertexDataBlock *)SimpleAllocator::get_first_block();
}

/**
 * Returns a pointer to the book that owns this page.
 */
INLINE VertexDataBook *VertexDataPage::
get_book() const {
  return _book;
}

/**
 * Returns a pointer to the global LRU object that manages the
 * VertexDataPage's with the indicated RamClass.
 */
INLINE SimpleLru *VertexDataPage::
get_global_lru(RamClass rclass) {
  nassertr(rclass >= 0 && rclass < RC_end_of_list, nullptr);
  return _global_lru[rclass];
}

/**
 * Returns a pointer to the global LRU object that manages the
 * VertexDataPage's that are pending processing by the thread.
 */
INLINE SimpleLru *VertexDataPage::
get_pending_lru() {
  return &_pending_lru;
}

/**
 * Returns the global VertexDataSaveFile that will be used to save vertex data
 * buffers to disk when necessary.
 */
INLINE VertexDataSaveFile *VertexDataPage::
get_save_file() {
  if (_save_file == nullptr) {
    make_save_file();
  }
  return _save_file;
}

/**
 * Writes the page to disk, but does not evict it from memory or affect its
 * LRU status.  If it gets evicted later without having been modified, it will
 * not need to write itself to disk again.
 */
INLINE bool VertexDataPage::
save_to_disk() {
  MutexHolder holder(_lock);
  return do_save_to_disk();
}

/**
 * Returns the number of threads that have been spawned to service vertex
 * paging requests, or 0 if no threads have been spawned (which may mean
 * either that all paging requests will be handled by the main thread, or
 * simply that no paging requests have yet been issued).
 */
INLINE int VertexDataPage::
get_num_threads() {
  MutexHolder holder(_tlock);
  if (_thread_mgr == nullptr) {
    return 0;
  }
  return _thread_mgr->get_num_threads();
}

/**
 * Returns the number of read requests that are waiting to be serviced by a
 * thread.
 */
INLINE int VertexDataPage::
get_num_pending_reads() {
  MutexHolder holder(_tlock);
  if (_thread_mgr == nullptr) {
    return 0;
  }
  return _thread_mgr->get_num_pending_reads();
}

/**
 * Returns the number of write requests that are waiting to be serviced by a
 * thread.
 */
INLINE int VertexDataPage::
get_num_pending_writes() {
  MutexHolder holder(_tlock);
  if (_thread_mgr == nullptr) {
    return 0;
  }
  return _thread_mgr->get_num_pending_writes();
}

/**
 * Returns a pointer to the page's data area, or NULL if the page is not
 * currently resident.  If the page is not currently resident, this will
 * implicitly request it to become resident soon.
 *
 * If force is true, this method will never return NULL, but may block until
 * the page is available.
 */
INLINE unsigned char *VertexDataPage::
get_page_data(bool force) {
  MutexHolder holder(_lock);
  if (_ram_class != RC_resident || _pending_ram_class != RC_resident) {
    if (force) {
      make_resident_now();
    } else {
      request_ram_class(RC_resident);
      if (_ram_class != RC_resident) {
        return nullptr;
      }
    }
  }

  mark_used_lru();
  nassertr(_size == _uncompressed_size, _page_data);
  return _page_data;
}

/**
 * This comparison method is used to order pages within a book.
 */
INLINE bool VertexDataPage::
operator < (const VertexDataPage &other) const {
  // We sort pages so that the pages with the smallest number of available
  // contiguous bytes come up first.  We store our best estimate of
  // continguous bytes here.
  if (_book_size != other._book_size) {
    return _book_size < other._book_size;
  }

  // For pages of equal size, we sort based on pointers, to make it easy to
  // quickly find a specific page.
  return this < &other;
}

/**
 * Puts the data in a new ram class.  Assumes the page lock is already held.
 */
INLINE void VertexDataPage::
set_ram_class(RamClass rclass) {
  _ram_class = rclass;
  mark_used_lru(_global_lru[rclass]);

  // Changing the ram class might make our effective available space 0 and
  // thereby change the placement within the book.
  adjust_book_size();
}

/**
 * Round page_size up to the next multiple of _block_size.
 */
INLINE size_t VertexDataPage::
round_up(size_t page_size) const {
  return ((page_size + _block_size - 1) / _block_size) * _block_size;
}
