// Filename: vertexDataPage.cxx
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

#include "vertexDataPage.h"
#include "configVariableInt.h"
#include "vertexDataSaveFile.h"
#include "vertexDataBook.h"
#include "pStatTimer.h"
#include "mutexHolder.h"

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

PT(VertexDataPage::PageThread) VertexDataPage::_thread;
Mutex VertexDataPage::_tlock;

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

// This mutex is (mostly) unused.  We just need a Mutex to pass to the
// Book Constructor, below.
Mutex VertexDataPage::_unused_mutex;

PStatCollector VertexDataPage::_vdata_compress_pcollector("*:Vertex Data:Compress");
PStatCollector VertexDataPage::_vdata_decompress_pcollector("*:Vertex Data:Decompress");
PStatCollector VertexDataPage::_vdata_save_pcollector("*:Vertex Data:Save");
PStatCollector VertexDataPage::_vdata_restore_pcollector("*:Vertex Data:Restore");
PStatCollector VertexDataPage::_thread_wait_pcollector("*:Wait:Idle");

TypeHandle VertexDataPage::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::Book Constructor
//       Access: Public
//  Description: This constructor is used only by VertexDataBook, to
//               create a mostly-empty object that can be used to
//               search for a particular page size in the set.
////////////////////////////////////////////////////////////////////
VertexDataPage::
VertexDataPage(size_t book_size) : 
  SimpleAllocator(book_size, _unused_mutex), 
  SimpleLruPage(book_size),
  _book_size(book_size),
  _book(NULL)
{
  _page_data = NULL;
  _size = 0;
  _uncompressed_size = 0;
  _ram_class = RC_resident;
  _pending_ram_class = RC_resident;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
VertexDataPage::
VertexDataPage(VertexDataBook *book, size_t page_size) : 
  SimpleAllocator(page_size, book->_lock), 
  SimpleLruPage(page_size),
  _book_size(page_size),
  _book(book)
{
  get_class_type().inc_memory_usage(TypeHandle::MC_array, (int)page_size);
  _page_data = new unsigned char[page_size];
  _size = page_size;

  _uncompressed_size = _size;
  _pending_ram_class = RC_resident;
  set_ram_class(RC_resident);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
VertexDataPage::
~VertexDataPage() {

  // Since the only way to delete a page is via the
  // changed_contiguous() method, the lock will already be held.
  // MutexHolder holder(_lock);

  {
    MutexHolder holder2(_tlock);
    if (_pending_ram_class != _ram_class) {
      nassertv(_thread != (PageThread *)NULL);
      _thread->remove_page(this);
    }
  }

  if (_page_data != NULL) {
    get_class_type().dec_memory_usage(TypeHandle::MC_array, (int)_size);
    delete[] _page_data;
    _size = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::stop_thread
//       Access: Published, Static
//  Description: Call this to stop the paging thread, if it was
//               started.  This may block until all of the thread's
//               pending tasks have been completed.
////////////////////////////////////////////////////////////////////
void VertexDataPage::
stop_thread() {
  PT(PageThread) thread;
  {
    MutexHolder holder(_tlock);
    thread = _thread;
    _thread.clear();
  }

  if (thread != (PageThread *)NULL) {
    gobj_cat.info()
      << "Stopping vertex paging thread.\n";
    thread->stop_thread();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::make_block
//       Access: Protected, Virtual
//  Description: Creates a new SimpleAllocatorBlock object.  Override
//               this function to specialize the block type returned.
////////////////////////////////////////////////////////////////////
SimpleAllocatorBlock *VertexDataPage::
make_block(size_t start, size_t size) {
  return new VertexDataBlock(this, start, size);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::changed_contiguous
//       Access: Protected, Virtual
//  Description: This callback function is made whenever the estimate
//               of contiguous available space changes, either through
//               an alloc or free.  The lock will be held.
////////////////////////////////////////////////////////////////////
void VertexDataPage::
changed_contiguous() {
  if (do_is_empty()) {
    // If the page is now empty, delete it.
    VertexDataBook::Pages::iterator pi = _book->_pages.find(this);
    nassertv(pi != _book->_pages.end());
    _book->_pages.erase(pi);
    delete this;
    return;
  }

  adjust_book_size();
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::evict_lru
//       Access: Public, Virtual
//  Description: Evicts the page from the LRU.  Called internally when
//               the LRU determines that it is full.  May also be
//               called externally when necessary to explicitly evict
//               the page.
//
//               It is legal for this method to either evict the page
//               as requested, do nothing (in which case the eviction
//               will be requested again at the next epoch), or
//               requeue itself on the tail of the queue (in which
//               case the eviction will be requested again much
//               later).
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::do_alloc
//       Access: Private
//  Description: Allocates a new block.  Returns NULL if a block of the
//               requested size cannot be allocated.
//
//               To free the allocated block, call block->free(), or
//               simply delete the block pointer.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
VertexDataBlock *VertexDataPage::
do_alloc(size_t size) {
  VertexDataBlock *block = (VertexDataBlock *)SimpleAllocator::do_alloc(size);

  if (block != (VertexDataBlock *)NULL && _ram_class != RC_disk) {
    // When we allocate a new block within a resident page, we have to
    // clear the disk cache (since we have just invalidated it).
    _saved_block.clear();
  }

  return block;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::make_resident_now
//       Access: Private
//  Description: Short-circuits the thread and forces the page into
//               resident status immediately.
//
//               Intended to be called from the main thread.  Assumes
//               the lock is already held.
////////////////////////////////////////////////////////////////////
void VertexDataPage::
make_resident_now() {
  MutexHolder holder(_tlock);
  if (_pending_ram_class != _ram_class) {
    nassertv(_thread != (PageThread *)NULL);
    _thread->remove_page(this);
  }

  make_resident();
  _pending_ram_class = RC_resident;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::make_resident
//       Access: Private
//  Description: Moves the page to fully resident status by
//               expanding it or reading it from disk as necessary.
//
//               Intended to be called from the sub-thread.  Assumes
//               the lock is already held.
////////////////////////////////////////////////////////////////////
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
    unsigned char *new_data = new unsigned char[_uncompressed_size];
    uLongf dest_len = _uncompressed_size;
    int result = uncompress(new_data, &dest_len, _page_data, _size);
    if (result != Z_OK) {
      gobj_cat.error()
        << "Couldn't expand: zlib error " << result << "\n";
      nassert_raise("zlib error");
    }
    nassertv(dest_len == _uncompressed_size);

    get_class_type().inc_memory_usage(TypeHandle::MC_array, (int)_uncompressed_size - (int)_size);
    delete[] _page_data;
    _page_data = new_data;
    _size = _uncompressed_size;
#endif

    set_lru_size(_size);
    set_ram_class(RC_resident);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::make_compressed
//       Access: Private
//  Description: Moves the page to compressed status by
//               compressing it or reading it from disk as necessary.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
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

    // According to the zlib manual, we need to provide this much
    // buffer to the compress algorithm: 0.1% bigger plus twelve
    // bytes.
    uLongf buffer_size = _uncompressed_size + ((_uncompressed_size + 999) / 1000) + 12;
    Bytef *buffer = (Bytef *)alloca(buffer_size);

    int result = compress2(buffer, &buffer_size,
                           _page_data, _uncompressed_size,
                           vertex_data_compression_level);
    if (result != Z_OK) {
      gobj_cat.error()
        << "Couldn't compress: zlib error " << result << "\n";
      nassert_raise("zlib error");
    }
    
    unsigned char *new_data = new unsigned char[buffer_size];
    memcpy(new_data, buffer, buffer_size);

    get_class_type().inc_memory_usage(TypeHandle::MC_array, (int)buffer_size - (int)_size);
    delete[] _page_data;
    _page_data = new_data;
    _size = buffer_size;

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

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::make_disk
//       Access: Private
//  Description: Moves the page to disk status by writing it to disk
//               as necessary.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
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

    get_class_type().dec_memory_usage(TypeHandle::MC_array, (int)_size);
    delete[] _page_data;
    _page_data = NULL;
    _size = 0;
    
    set_ram_class(RC_disk);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::do_save_to_disk
//       Access: Private
//  Description: Writes the page to disk, but does not evict it from
//               memory or affect its LRU status.  If it gets evicted
//               later without having been modified, it will not need
//               to write itself to disk again.
//
//               Returns true on success, false on failure.  Assumes
//               the lock is already held.
////////////////////////////////////////////////////////////////////
bool VertexDataPage::
do_save_to_disk() {
  if (_ram_class == RC_resident || _ram_class == RC_compressed) {
    PStatTimer timer(_vdata_save_pcollector);

    if (_saved_block == (VertexDataSaveBlock *)NULL) {
      if (gobj_cat.is_debug()) {
        gobj_cat.debug()
          << "Storing page, " << _size << " bytes, to disk\n";
      }

      bool compressed = (_ram_class == RC_compressed);
      
      _saved_block = get_save_file()->write_data(_page_data, _size, compressed);
      if (_saved_block == (VertexDataSaveBlock *)NULL) {
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

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::do_restore_from_disk
//       Access: Private
//  Description: Restores the page from disk and makes it
//               either compressed or resident (according to whether
//               it was stored compressed on disk).
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void VertexDataPage::
do_restore_from_disk() {
  if (_ram_class == RC_disk) {
    nassertv(_saved_block != (VertexDataSaveBlock *)NULL);
    nassertv(_page_data == (unsigned char *)NULL && _size == 0);

    PStatTimer timer(_vdata_restore_pcollector);

    size_t buffer_size = _saved_block->get_size();
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Restoring page, " << buffer_size << " bytes, from disk\n";
    }

    unsigned char *new_data = new unsigned char[buffer_size];
    if (!get_save_file()->read_data(new_data, buffer_size, _saved_block)) {
      nassert_raise("read error");
    }

    get_class_type().inc_memory_usage(TypeHandle::MC_array, (int)buffer_size);
    nassertv(_page_data == (unsigned char *)NULL);
    _page_data = new_data;
    _size = buffer_size;

    set_lru_size(_size);
    if (_saved_block->get_compressed()) {
      set_ram_class(RC_compressed);
    } else {
      set_ram_class(RC_resident);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::adjust_book_size
//       Access: Private
//  Description: Called when the "book size"--the size of the page as
//               recorded in its book's table--has changed for some
//               reason.  Assumes the lock is held.
////////////////////////////////////////////////////////////////////
void VertexDataPage::
adjust_book_size() {
  size_t new_size = _contiguous;
  if (_ram_class != RC_resident) {
    // Let's not attempt to allocate new buffers from non-resident
    // pages.
    new_size = 0;
  }

  if (new_size != _book_size) {
    VertexDataBook::Pages::iterator pi = _book->_pages.find(this);
    nassertv(pi != _book->_pages.end());
    _book->_pages.erase(pi);

    _book_size = new_size;
    bool inserted = _book->_pages.insert(this).second;
    nassertv(inserted);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::request_ram_class
//       Access: Private
//  Description: Requests the thread set the page to the indicated ram
//               class (if we are using threading).  The page will be
//               enqueued in the thread, which will eventually be
//               responsible for setting the requested ram class.
//
//               Assumes the page's lock is already held.
////////////////////////////////////////////////////////////////////
void VertexDataPage::
request_ram_class(RamClass ram_class) {
  if (!vertex_data_threaded_paging || !Thread::is_threading_supported()) {
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
  if (_thread == (PageThread *)NULL) {
    // Allocate and start a new global thread.
    gobj_cat.info()
      << "Spawning vertex paging thread.\n";
    _thread = new PageThread;
    _thread->start(TP_low, true);
  }

  _thread->add_page(this, ram_class);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::make_save_file
//       Access: Private, Static
//  Description: Creates the global VertexDataSaveFile that will be
//               used to save vertex data buffers to disk when
//               necessary.
////////////////////////////////////////////////////////////////////
void VertexDataPage::
make_save_file() {
  size_t max_size = (size_t)max_disk_vertex_data;

  _save_file = new VertexDataSaveFile(vertex_save_file_directory,
                                      vertex_save_file_prefix, max_size);
}
 
////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::PageThread::add_page
//       Access: Public
//  Description: Enqueues the indicated page on the thread to convert
//               it to the specified ram class.  
//
//               It is assumed the page's lock is already held, and
//               the thread's tlock is already held.
////////////////////////////////////////////////////////////////////
void VertexDataPage::PageThread::
add_page(VertexDataPage *page, RamClass ram_class) {
  if (page->_pending_ram_class == ram_class) {
    // It's already queued.
    nassertv(page->get_lru() == &_pending_lru);
    return;
  }
  
  if (page->_pending_ram_class != page->_ram_class) {
    // It's already queued, but for a different ram class.  Dequeue it
    // so we can requeue it.
    remove_page(page);
  }

  if (page->_pending_ram_class != ram_class) {
    // First, move the page to the "pending" LRU.  When it eventually
    // gets its requested ram class set, it will be requeued on the
    // appropriate live LRU.
    page->mark_used_lru(&_pending_lru);

    page->_pending_ram_class = ram_class;
    if (ram_class == RC_resident) {
      _pending_reads.push_back(page);
    } else {
      _pending_writes.push_back(page);
    }
    _pending_cvar.signal();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::PageThread::remove_page
//       Access: Public
//  Description: Dequeues the indicated page and removes it from the
//               pending task list.
//
//               It is assumed the page's lock is already held, and
//               the thread's tlock is already held.
////////////////////////////////////////////////////////////////////
void VertexDataPage::PageThread::
remove_page(VertexDataPage *page) {
  nassertv(page != (VertexDataPage *)NULL);
  if (page == _working_page) {
    // Oops, the thread is currently working on this one.  We'll have
    // to wait for the thread to finish.
    page->_lock.release();
    while (page == _working_page) {
      _working_cvar.wait();
    }
    page->_lock.lock();
    return;
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

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::PageThread::thread_main
//       Access: Protected, Virtual
//  Description: The main processing loop for the sub-thread.
////////////////////////////////////////////////////////////////////
void VertexDataPage::PageThread::
thread_main() {
  _tlock.lock();

  while (true) {
    PStatClient::thread_tick(get_sync_name());

    while (_pending_reads.empty() && _pending_writes.empty()) {
      if (_shutdown) {
        _tlock.release();
        return;
      }
      PStatTimer timer(_thread_wait_pcollector);
      _pending_cvar.wait();
    }

    // Reads always have priority.
    if (!_pending_reads.empty()) {
      _working_page = _pending_reads.front();
      _pending_reads.pop_front();
    } else {
      _working_page = _pending_writes.front();
      _pending_writes.pop_front();
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
    
    _tlock.lock();

    _working_page = NULL;
    _working_cvar.signal();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::PageThread::stop_thread
//       Access: Public
//  Description: Signals the thread to stop and waits for it.  Does
//               not return until the thread has finished.
////////////////////////////////////////////////////////////////////
void VertexDataPage::PageThread::
stop_thread() {
  {
    MutexHolder holder(_tlock);
    _shutdown = true;
    _pending_cvar.signal();
  }
  join();
}
