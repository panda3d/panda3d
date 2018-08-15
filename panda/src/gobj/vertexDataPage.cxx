/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexDataPage.cxx
 * @author drose
 * @date 2007-06-04
 */

#include "vertexDataPage.h"
#include "configVariableInt.h"
#include "vertexDataSaveFile.h"
#include "vertexDataBook.h"
#include "vertexDataBlock.h"
#include "pStatTimer.h"
#include "memoryHook.h"
#include "config_gobj.h"
#include <algorithm>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

ConfigVariableInt max_resident_vertex_data
("max-resident-vertex-data", -1,
 PRC_DESC("Specifies the maximum number of bytes of all vertex data "
          "that is allowed to remain resident in system RAM at one time. "
          "If more than this number of bytes of vertices are created, "
          "the least-recently-used ones will be temporarily compressed in "
          "system RAM until they are needed.  Set it to -1 for no limit."));

ConfigVariableInt max_compressed_vertex_data
("max-compressed-vertex-data", 0,
 PRC_DESC("Specifies the maximum number of bytes of all vertex data "
          "that is allowed to remain compressed in system RAM at one time. "
          "If more than this number of bytes of vertices are created, "
          "the least-recently-used ones will be temporarily flushed to "
          "disk until they are needed.  Set it to -1 for no limit."));

ConfigVariableInt vertex_data_compression_level
("vertex-data-compression-level", 1,
 PRC_DESC("Specifies the zlib compression level to use when compressing "
          "vertex data.  The number should be in the range 1 to 9, where "
          "larger values are slower but give better compression."));

ConfigVariableInt max_disk_vertex_data
("max-disk-vertex-data", -1,
 PRC_DESC("Specifies the maximum number of bytes of vertex data "
          "that is allowed to be written to disk.  Set it to -1 for no "
          "limit."));

PT(VertexDataPage::PageThreadManager) VertexDataPage::_thread_mgr;

// This is a reference to an allocated Mutex, instead of just a static Mutex,
// to protect against ordering issues when the application shuts down.
Mutex &VertexDataPage::_tlock = *(new Mutex("VertexDataPage::_tlock"));

SimpleLru VertexDataPage::_resident_lru("resident", max_resident_vertex_data);
SimpleLru VertexDataPage::_compressed_lru("compressed", max_compressed_vertex_data);
SimpleLru VertexDataPage::_disk_lru("disk", 0);
SimpleLru VertexDataPage::_pending_lru("pending", 0);

SimpleLru *VertexDataPage::_global_lru[RC_end_of_list] = {
  &VertexDataPage::_resident_lru,
  &VertexDataPage::_compressed_lru,
  &VertexDataPage::_disk_lru,
};

VertexDataSaveFile *VertexDataPage::_save_file;

// This mutex is (mostly) unused.  We just need a Mutex to pass to the Book
// Constructor, below.
Mutex VertexDataPage::_unused_mutex;

PStatCollector VertexDataPage::_vdata_compress_pcollector("*:Vertex Data:Compress");
PStatCollector VertexDataPage::_vdata_decompress_pcollector("*:Vertex Data:Decompress");
PStatCollector VertexDataPage::_vdata_save_pcollector("*:Vertex Data:Save");
PStatCollector VertexDataPage::_vdata_restore_pcollector("*:Vertex Data:Restore");
PStatCollector VertexDataPage::_thread_wait_pcollector("Wait:Idle");
PStatCollector VertexDataPage::_alloc_pages_pcollector("System memory:MMap:Vertex data");

TypeHandle VertexDataPage::_type_handle;
TypeHandle VertexDataPage::DeflatePage::_type_handle;

#if defined(HAVE_ZLIB) && !defined(USE_MEMORY_NOWRAPPERS)
// Define functions that hook zlib into panda's memory allocation system.
static void *
do_zlib_alloc(voidpf opaque, uInt items, uInt size) {
  return PANDA_MALLOC_ARRAY(items * size);
}
static void
do_zlib_free(voidpf opaque, voidpf address) {
  PANDA_FREE_ARRAY(address);
}
#endif  // HAVE_ZLIB && !USE_MEMORY_NOWRAPPERS


/**
 * This constructor is used only by VertexDataBook, to create a mostly-empty
 * object that can be used to search for a particular page size in the set.
 */
VertexDataPage::
VertexDataPage(size_t book_size) :
  SimpleAllocator(book_size, _unused_mutex),
  SimpleLruPage(book_size),
  _book_size(book_size),
  _block_size(0),
  _book(nullptr)
{
  _page_data = nullptr;
  _size = 0;
  _uncompressed_size = 0;
  _ram_class = RC_resident;
  _pending_ram_class = RC_resident;
}

/**
 *
 */
VertexDataPage::
VertexDataPage(VertexDataBook *book, size_t page_size, size_t block_size) :
  SimpleAllocator(page_size, book->_lock),
  SimpleLruPage(page_size),
  _book_size(page_size),
  _block_size(block_size),
  _book(book)
{
  _allocated_size = round_up(page_size);
  _page_data = alloc_page_data(_allocated_size);
  _size = page_size;

  _uncompressed_size = _size;
  _pending_ram_class = RC_resident;
  set_ram_class(RC_resident);
}

/**
 *
 */
VertexDataPage::
~VertexDataPage() {

  // Since the only way to delete a page is via the changed_contiguous()
  // method, the lock will already be held.  MutexHolder holder(_lock);

  {
    MutexHolder holder2(_tlock);
    if (_pending_ram_class != _ram_class) {
      nassertv(_thread_mgr != nullptr);
      _thread_mgr->remove_page(this);
    }
  }

  if (_page_data != nullptr) {
    free_page_data(_page_data, _allocated_size);
    _size = 0;
  }

  nassertv(_book == nullptr);
}

/**
 * Call this to stop the paging threads, if they were started.  This may block
 * until all of the pending tasks have been completed.
 */
void VertexDataPage::
stop_threads() {
  PT(PageThreadManager) thread_mgr;
  {
    MutexHolder holder(_tlock);
    thread_mgr = _thread_mgr;
    _thread_mgr.clear();
  }

  if (thread_mgr != nullptr) {
    gobj_cat.info()
      << "Stopping vertex paging threads.\n";
    thread_mgr->stop_threads();
  }
}

/**
 * Waits for all of the pending thread tasks to finish before returning.
 */
void VertexDataPage::
flush_threads() {
  int num_threads = vertex_data_page_threads;
  if (num_threads == 0) {
    stop_threads();
    return;
  }

  PT(PageThreadManager) thread_mgr;
  {
    MutexHolder holder(_tlock);
    thread_mgr = _thread_mgr;
  }

  if (thread_mgr != nullptr) {
    thread_mgr->stop_threads();
    MutexHolder holder(_tlock);
    thread_mgr->start_threads(num_threads);
  }
}

/**
 *
 */
void VertexDataPage::
output(std::ostream &out) const {
  SimpleAllocator::output(out);
}

/**
 *
 */
void VertexDataPage::
write(std::ostream &out, int indent_level) const {
  SimpleAllocator::write(out);
}

/**
 * Creates a new SimpleAllocatorBlock object.  Override this function to
 * specialize the block type returned.
 */
SimpleAllocatorBlock *VertexDataPage::
make_block(size_t start, size_t size) {
  return new VertexDataBlock(this, start, size);
}

/**
 * This callback function is made whenever the estimate of contiguous
 * available space changes, either through an alloc or free.  The lock will be
 * held.
 */
void VertexDataPage::
changed_contiguous() {
  if (do_is_empty()) {
    // If the page is now empty, delete it.
    VertexDataBook::Pages::iterator pi = _book->_pages.find(this);
    nassertv(pi != _book->_pages.end());
    _book->_pages.erase(pi);
    _book = nullptr;
    delete this;
    return;
  }

  adjust_book_size();
}

/**
 * Evicts the page from the LRU.  Called internally when the LRU determines
 * that it is full.  May also be called externally when necessary to
 * explicitly evict the page.
 *
 * It is legal for this method to either evict the page as requested, do
 * nothing (in which case the eviction will be requested again at the next
 * epoch), or requeue itself on the tail of the queue (in which case the
 * eviction will be requested again much later).
 */
void VertexDataPage::
evict_lru() {
  MutexHolder holder(_lock);

  switch (_ram_class) {
  case RC_resident:
    if (_compressed_lru.get_max_size() == 0) {
      request_ram_class(RC_disk);
    } else {
      request_ram_class(RC_compressed);
    }
    break;

  case RC_compressed:
    request_ram_class(RC_disk);
    break;

  case RC_disk:
  case RC_end_of_list:
    gobj_cat.warning()
      << "Internal error: attempt to evict array data " << this
      << " in inappropriate state " << _ram_class << ".\n";
    break;
  }
}

/**
 * Allocates a new block.  Returns NULL if a block of the requested size
 * cannot be allocated.
 *
 * To free the allocated block, call block->free(), or simply delete the block
 * pointer.
 *
 * Assumes the lock is already held.
 */
VertexDataBlock *VertexDataPage::
do_alloc(size_t size) {
  VertexDataBlock *block = (VertexDataBlock *)SimpleAllocator::do_alloc(size);

  if (block != nullptr && _ram_class != RC_disk) {
    // When we allocate a new block within a resident page, we have to clear
    // the disk cache (since we have just invalidated it).
    _saved_block.clear();
  }

  return block;
}

/**
 * Short-circuits the thread and forces the page into resident status
 * immediately.
 *
 * Intended to be called from the main thread.  Assumes the lock is already
 * held.
 */
void VertexDataPage::
make_resident_now() {
  MutexHolder holder(_tlock);
  if (_pending_ram_class != _ram_class) {
    nassertv(_thread_mgr != nullptr);
    _thread_mgr->remove_page(this);
  }

  make_resident();
  _pending_ram_class = RC_resident;
}

/**
 * Moves the page to fully resident status by expanding it or reading it from
 * disk as necessary.
 *
 * Intended to be called from the sub-thread.  Assumes the lock is already
 * held.
 */
void VertexDataPage::
make_resident() {
  if (_ram_class == RC_resident) {
    mark_used_lru();
    return;
  }

  if (_ram_class == RC_disk) {
    do_restore_from_disk();
  }

  if (_ram_class == RC_compressed) {
#ifdef HAVE_ZLIB
    PStatTimer timer(_vdata_decompress_pcollector);

    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Expanding page from " << _size
        << " to " << _uncompressed_size << "\n";
    }
    size_t new_allocated_size = round_up(_uncompressed_size);
    unsigned char *new_data = alloc_page_data(new_allocated_size);
    unsigned char *end_data = new_data + new_allocated_size;

    z_stream z_source;
#ifdef USE_MEMORY_NOWRAPPERS
    z_source.zalloc = Z_NULL;
    z_source.zfree = Z_NULL;
#else
    z_source.zalloc = (alloc_func)&do_zlib_alloc;
    z_source.zfree = (free_func)&do_zlib_free;
#endif

    z_source.opaque = Z_NULL;
    z_source.msg = (char *) "no error message";

    z_source.next_in = (Bytef *)(char *)_page_data;
    z_source.avail_in = _size;
    z_source.next_out = (Bytef *)new_data;
    z_source.avail_out = new_allocated_size;

    int result = inflateInit(&z_source);
    if (result < 0) {
      nassert_raise("zlib error");
      return;
    }
    Thread::consider_yield();

    size_t output_size = 0;

    int flush = 0;
    result = 0;
    while (result != Z_STREAM_END) {
      unsigned char *start_out = (unsigned char *)z_source.next_out;
      nassertv(start_out < end_data);
      z_source.avail_out = std::min((size_t)(end_data - start_out), (size_t)inflate_page_size);
      nassertv(z_source.avail_out != 0);
      result = inflate(&z_source, flush);
      if (result < 0 && result != Z_BUF_ERROR) {
        nassert_raise("zlib error");
        return;
      }
      size_t bytes_produced = (size_t)((unsigned char *)z_source.next_out - start_out);
      output_size += bytes_produced;
      if (bytes_produced == 0) {
        // If we ever produce no bytes, then start flushing the output.
        flush = Z_FINISH;
      }

      Thread::consider_yield();
    }
    nassertv(z_source.avail_in == 0);
    nassertv(output_size == _uncompressed_size);

    result = inflateEnd(&z_source);
    nassertv(result == Z_OK);

    free_page_data(_page_data, _allocated_size);
    _page_data = new_data;
    _size = _uncompressed_size;
    _allocated_size = new_allocated_size;
#endif

    set_lru_size(_size);
    set_ram_class(RC_resident);
  }
}

/**
 * Moves the page to compressed status by compressing it or reading it from
 * disk as necessary.
 *
 * Assumes the lock is already held.
 */
void VertexDataPage::
make_compressed() {
  if (_ram_class == RC_compressed) {
    // If we're already compressed, just mark the page recently used.
    mark_used_lru();
    return;
  }

  if (_ram_class == RC_disk) {
    do_restore_from_disk();
  }

  if (_ram_class == RC_resident) {
    nassertv(_size == _uncompressed_size);

#ifdef HAVE_ZLIB
    PStatTimer timer(_vdata_compress_pcollector);

    DeflatePage *page = new DeflatePage;
    DeflatePage *head = page;

    z_stream z_dest;
#ifdef USE_MEMORY_NOWRAPPERS
    z_dest.zalloc = Z_NULL;
    z_dest.zfree = Z_NULL;
#else
    z_dest.zalloc = (alloc_func)&do_zlib_alloc;
    z_dest.zfree = (free_func)&do_zlib_free;
#endif

    z_dest.opaque = Z_NULL;
    z_dest.msg = (char *) "no error message";

    int result = deflateInit(&z_dest, vertex_data_compression_level);
    if (result < 0) {
      nassert_raise("zlib error");
      return;
    }
    Thread::consider_yield();

    z_dest.next_in = (Bytef *)(char *)_page_data;
    z_dest.avail_in = _uncompressed_size;
    size_t output_size = 0;

    // Compress the data into one or more individual pages.  We have to
    // compress it page-at-a-time, since we're not really sure how big the
    // result will be (so we can't easily pre-allocate a buffer).
    int flush = 0;
    result = 0;
    while (result != Z_STREAM_END) {
      unsigned char *start_out = (page->_buffer + page->_used_size);
      z_dest.next_out = (Bytef *)start_out;
      z_dest.avail_out = (size_t)deflate_page_size - page->_used_size;
      if (z_dest.avail_out == 0) {
        DeflatePage *new_page = new DeflatePage;
        page->_next = new_page;
        page = new_page;
        start_out = page->_buffer;
        z_dest.next_out = (Bytef *)start_out;
        z_dest.avail_out = deflate_page_size;
      }

      result = deflate(&z_dest, flush);
      if (result < 0 && result != Z_BUF_ERROR) {
        nassert_raise("zlib error");
        return;
      }
      size_t bytes_produced = (size_t)((unsigned char *)z_dest.next_out - start_out);
      page->_used_size += bytes_produced;
      nassertv(page->_used_size <= deflate_page_size);
      output_size += bytes_produced;
      if (bytes_produced == 0) {
        // If we ever produce no bytes, then start flushing the output.
        flush = Z_FINISH;
      }

      Thread::consider_yield();
    }
    nassertv(z_dest.avail_in == 0);

    result = deflateEnd(&z_dest);
    nassertv(result == Z_OK);

    // Now we know how big the result will be.  Allocate a buffer, and copy
    // the data from the various pages.

    size_t new_allocated_size = round_up(output_size);
    unsigned char *new_data = alloc_page_data(new_allocated_size);

    size_t copied_size = 0;
    unsigned char *p = new_data;
    page = head;
    while (page != nullptr) {
      memcpy(p, page->_buffer, page->_used_size);
      copied_size += page->_used_size;
      p += page->_used_size;
      DeflatePage *next = page->_next;
      delete page;
      page = next;
    }
    nassertv(copied_size == output_size);

    // Now free the original, uncompressed data, and put this new compressed
    // buffer in its place.
    free_page_data(_page_data, _allocated_size);
    _page_data = new_data;
    _size = output_size;
    _allocated_size = new_allocated_size;

    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Compressed " << *this << " from " << _uncompressed_size
        << " to " << _size << "\n";
    }
#endif
    set_lru_size(_size);
    set_ram_class(RC_compressed);
  }
}

/**
 * Moves the page to disk status by writing it to disk as necessary.
 *
 * Assumes the lock is already held.
 */
void VertexDataPage::
make_disk() {
  if (_ram_class == RC_disk) {
    // If we're already on disk, just mark the page recently used.
    mark_used_lru();
    return;
  }

  if (_ram_class == RC_resident || _ram_class == RC_compressed) {
    if (!do_save_to_disk()) {
      // Can't save it to disk for some reason.
      gobj_cat.warning()
        << "Couldn't save page " << this << " to disk.\n";
      mark_used_lru();
      return;
    }

    free_page_data(_page_data, _allocated_size);
    _page_data = nullptr;
    _size = 0;

    set_ram_class(RC_disk);
  }
}

/**
 * Writes the page to disk, but does not evict it from memory or affect its
 * LRU status.  If it gets evicted later without having been modified, it will
 * not need to write itself to disk again.
 *
 * Returns true on success, false on failure.  Assumes the lock is already
 * held.
 */
bool VertexDataPage::
do_save_to_disk() {
  if (_ram_class == RC_resident || _ram_class == RC_compressed) {
    PStatTimer timer(_vdata_save_pcollector);

    if (_saved_block == nullptr) {
      if (gobj_cat.is_debug()) {
        gobj_cat.debug()
          << "Storing page, " << _size << " bytes, to disk\n";
      }

      bool compressed = (_ram_class == RC_compressed);

      _saved_block = get_save_file()->write_data(_page_data, _allocated_size, compressed);
      if (_saved_block == nullptr) {
        // Can't write it to disk.  Too bad.
        return false;
      }
    } else {
      if (gobj_cat.is_debug()) {
        gobj_cat.debug()
          << "Page already stored: " << _size << " bytes\n";
      }
    }
  }

  return true;
}

/**
 * Restores the page from disk and makes it either compressed or resident
 * (according to whether it was stored compressed on disk).
 *
 * Assumes the lock is already held.
 */
void VertexDataPage::
do_restore_from_disk() {
  if (_ram_class == RC_disk) {
    nassertv(_saved_block != nullptr);
    nassertv(_page_data == nullptr && _size == 0);

    PStatTimer timer(_vdata_restore_pcollector);

    size_t buffer_size = _saved_block->get_size();
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Restoring page, " << buffer_size << " bytes, from disk\n";
    }

    size_t new_allocated_size = round_up(buffer_size);
    unsigned char *new_data = alloc_page_data(new_allocated_size);
    if (!get_save_file()->read_data(new_data, new_allocated_size, _saved_block)) {
      nassert_raise("read error");
    }

    nassertv(_page_data == nullptr);
    _page_data = new_data;
    _size = buffer_size;
    _allocated_size = new_allocated_size;

    set_lru_size(_size);
    if (_saved_block->get_compressed()) {
      set_ram_class(RC_compressed);
    } else {
      set_ram_class(RC_resident);
    }
  }
}

/**
 * Called when the "book size"--the size of the page as recorded in its book's
 * table--has changed for some reason.  Assumes the lock is held.
 */
void VertexDataPage::
adjust_book_size() {
  size_t new_size = _contiguous;
  if (_ram_class != RC_resident) {
    // Let's not attempt to allocate new buffers from non-resident pages.
    new_size = 0;
  }

  if (_book != nullptr && new_size != _book_size) {
    VertexDataBook::Pages::iterator pi = _book->_pages.find(this);
    nassertv(pi != _book->_pages.end());
    _book->_pages.erase(pi);

    _book_size = new_size;
    bool inserted = _book->_pages.insert(this).second;
    nassertv(inserted);
  }
}

/**
 * Requests the thread set the page to the indicated ram class (if we are
 * using threading).  The page will be enqueued in the thread, which will
 * eventually be responsible for setting the requested ram class.
 *
 * Assumes the page's lock is already held.
 */
void VertexDataPage::
request_ram_class(RamClass ram_class) {
  int num_threads = vertex_data_page_threads;
  if (num_threads == 0 || !Thread::is_threading_supported()) {
    // No threads.  Do it immediately.
    switch (ram_class) {
    case RC_resident:
      make_resident();
      break;

    case RC_compressed:
      make_compressed();
      break;

    case RC_disk:
      make_disk();
      break;

    case RC_end_of_list:
      break;
    }
    _pending_ram_class = ram_class;
    return;
  }

  MutexHolder holder(_tlock);
  if (_thread_mgr == nullptr) {
    // Create the thread manager.
    gobj_cat.info()
      << "Spawning " << num_threads << " vertex paging threads.\n";
    _thread_mgr = new PageThreadManager(num_threads);
  }

  _thread_mgr->add_page(this, ram_class);
}

/**
 * Creates the global VertexDataSaveFile that will be used to save vertex data
 * buffers to disk when necessary.
 */
void VertexDataPage::
make_save_file() {
  size_t max_size = (size_t)max_disk_vertex_data;

  _save_file = new VertexDataSaveFile(vertex_save_file_directory,
                                      vertex_save_file_prefix, max_size);
}

/**
 * Allocates and returns a freshly-allocated buffer of at least the indicated
 * size for holding vertex data.
 */
unsigned char *VertexDataPage::
alloc_page_data(size_t page_size) const {
  _alloc_pages_pcollector.add_level_now(page_size);
  return (unsigned char *)memory_hook->mmap_alloc(page_size, false);
}

/**
 * Releases a buffer allocated via alloc_page_data().
 */
void VertexDataPage::
free_page_data(unsigned char *page_data, size_t page_size) const {
  _alloc_pages_pcollector.sub_level_now(page_size);
  memory_hook->mmap_free(page_data, page_size);
}

/**
 * Assumes _tlock is held.
 */
VertexDataPage::PageThreadManager::
PageThreadManager(int num_threads) :
  _shutdown(false),
  _pending_cvar(_tlock)
{
  start_threads(num_threads);
}

/**
 * Enqueues the indicated page on the thread queue to convert it to the
 * specified ram class.
 *
 * It is assumed the page's lock is already held, and that _tlock is already
 * held.
 */
void VertexDataPage::PageThreadManager::
add_page(VertexDataPage *page, RamClass ram_class) {
  nassertv(!_shutdown);

  if (page->_pending_ram_class == ram_class) {
    // It's already queued.
    nassertv(page->get_lru() == &_pending_lru);
    return;
  }

  if (page->_pending_ram_class != page->_ram_class) {
    // It's already queued, but for a different ram class.  Dequeue it so we
    // can requeue it.
    remove_page(page);
  }

  if (page->_pending_ram_class != ram_class) {
    // First, move the page to the "pending" LRU.  When it eventually gets its
    // requested ram class set, it will be requeued on the appropriate live
    // LRU.
    page->mark_used_lru(&_pending_lru);

    page->_pending_ram_class = ram_class;
    if (ram_class == RC_resident) {
      _pending_reads.push_back(page);
    } else {
      _pending_writes.push_back(page);
    }
    _pending_cvar.notify();
  }
}

/**
 * Dequeues the indicated page and removes it from the pending task list.
 *
 * It is assumed the page's lock is already held, and that _tlock is already
 * held.
 */
void VertexDataPage::PageThreadManager::
remove_page(VertexDataPage *page) {
  nassertv(page != nullptr);

  PageThreads::iterator ti;
  for (ti = _threads.begin(); ti != _threads.end(); ++ti) {
    PageThread *thread = (*ti);
    if (page == thread->_working_page) {
      // Oops, this thread is currently working on this one.  We'll have to
      // wait for the thread to finish.
      page->_lock.release();
      while (page == thread->_working_page) {
        thread->_working_cvar.wait();
      }
      page->_lock.acquire();
      return;
    }
  }

  if (page->_pending_ram_class == RC_resident) {
    PendingPages::iterator pi =
      find(_pending_reads.begin(), _pending_reads.end(), page);
    nassertv(pi != _pending_reads.end());
    _pending_reads.erase(pi);
  } else {
    PendingPages::iterator pi =
      find(_pending_writes.begin(), _pending_writes.end(), page);
    nassertv(pi != _pending_writes.end());
    _pending_writes.erase(pi);
  }

  page->_pending_ram_class = page->_ram_class;

  // Put the page back on its proper LRU.
  page->mark_used_lru(_global_lru[page->_ram_class]);
}

/**
 * Returns the number of threads active on the thread manager.  Assumes _tlock
 * is held.
 */
int VertexDataPage::PageThreadManager::
get_num_threads() const {
  return (int)_threads.size();
}

/**
 * Returns the number of read requests waiting on the queue.  Assumes _tlock
 * is held.
 */
int VertexDataPage::PageThreadManager::
get_num_pending_reads() const {
  return (int)_pending_reads.size();
}

/**
 * Returns the number of write requests waiting on the queue.  Assumes _tlock
 * is held.
 */
int VertexDataPage::PageThreadManager::
get_num_pending_writes() const {
  return (int)_pending_writes.size();
}

/**
 * Adds the indicated of threads to the list of active threads.  Assumes
 * _tlock is held.
 */
void VertexDataPage::PageThreadManager::
start_threads(int num_threads) {
  _shutdown = false;

  _threads.reserve(num_threads);
  for (int i = 0; i < num_threads; ++i) {
    std::ostringstream name_strm;
    name_strm << "VertexDataPage" << _threads.size();
    PT(PageThread) thread = new PageThread(this, name_strm.str());
    thread->start(TP_low, true);
    _threads.push_back(thread);
  }
}

/**
 * Signals all the threads to stop and waits for them.  Does not return until
 * the threads have finished.  Assumes _tlock is *not* held.
 */
void VertexDataPage::PageThreadManager::
stop_threads() {
  PageThreads threads;
  {
    MutexHolder holder(_tlock);
    _shutdown = true;
    _pending_cvar.notify_all();
    threads.swap(_threads);
  }

  PageThreads::iterator ti;
  for (ti = threads.begin(); ti != threads.end(); ++ti) {
    PageThread *thread = (*ti);
    thread->join();
  }

  nassertv(_pending_reads.empty() && _pending_writes.empty());
}

/**
 *
 */
VertexDataPage::PageThread::
PageThread(PageThreadManager *manager, const std::string &name) :
  Thread(name, name),
  _manager(manager),
  _working_cvar(_tlock)
{
}

/**
 * The main processing loop for each sub-thread.
 */
void VertexDataPage::PageThread::
thread_main() {
  _tlock.acquire();

  while (true) {
    PStatClient::thread_tick(get_sync_name());

    while (_manager->_pending_reads.empty() &&
           _manager->_pending_writes.empty()) {
      if (_manager->_shutdown) {
        _tlock.release();
        return;
      }
      PStatTimer timer(_thread_wait_pcollector);
      _manager->_pending_cvar.wait();
    }

    // Reads always have priority.
    if (!_manager->_pending_reads.empty()) {
      _working_page = _manager->_pending_reads.front();
      _manager->_pending_reads.pop_front();
    } else {
      _working_page = _manager->_pending_writes.front();
      _manager->_pending_writes.pop_front();
    }

    RamClass ram_class = _working_page->_pending_ram_class;
    _tlock.release();

    {
      MutexHolder holder(_working_page->_lock);
      switch (ram_class) {
      case RC_resident:
        _working_page->make_resident();
        break;

      case RC_compressed:
        _working_page->make_compressed();
        break;

      case RC_disk:
        _working_page->make_disk();
        break;

      case RC_end_of_list:
        break;
      }
    }

    _tlock.acquire();

    _working_page = nullptr;
    _working_cvar.notify();

    Thread::consider_yield();
  }
}
