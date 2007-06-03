// Filename: vertexDataBook.cxx
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

#include "vertexDataBook.h"
#include "configVariableInt.h"
#include "vertexDataSaveFile.h"
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

SimpleLru VertexDataPage::_resident_lru(max_resident_vertex_data);
SimpleLru VertexDataPage::_compressed_lru(max_compressed_vertex_data);
SimpleLru VertexDataPage::_disk_lru(0);

SimpleLru *VertexDataPage::_global_lru[RC_end_of_list] = {
  &VertexDataPage::_resident_lru,
  &VertexDataPage::_compressed_lru,
  &VertexDataPage::_disk_lru,
};

size_t VertexDataPage::_total_page_size = 0;
VertexDataSaveFile *VertexDataPage::_save_file;

PStatCollector VertexDataPage::_vdata_compress_pcollector("*:Vertex Data:Compress");
PStatCollector VertexDataPage::_vdata_decompress_pcollector("*:Vertex Data:Decompress");
PStatCollector VertexDataPage::_vdata_save_pcollector("*:Vertex Data:Save");
PStatCollector VertexDataPage::_vdata_restore_pcollector("*:Vertex Data:Restore");

TypeHandle VertexDataPage::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBook::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
VertexDataBook::
VertexDataBook(size_t block_size) : _block_size(block_size) {
  _next_pi = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBook::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
VertexDataBook::
~VertexDataBook() {
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBook::alloc
//       Access: Published
//  Description: Allocates and returns a new VertexDataBuffer of the
//               requested size.
////////////////////////////////////////////////////////////////////
VertexDataBlock *VertexDataBook::
alloc(size_t size) {
  MutexHolder holder(_lock);

  // First, try to allocate from the last page that worked; then
  // continue to the end of the list.
  size_t pi = _next_pi;
  while (pi < _pages.size()) {
    VertexDataBlock *block = _pages[pi]->alloc(size);
    if (block != (VertexDataBlock *)NULL) {
      _next_pi = pi;
      return block;
    }
    if (_pages[pi]->is_empty()) {
      // This page is empty, but must have been too small.  Create a
      // new page in its place.
      delete _pages[pi];
      _pages[pi] = create_new_page(size);
      VertexDataBlock *block = _pages[pi]->alloc(size);
      return block;
    }
    ++pi;
  }

  // Then, go back to the beginning and try those pages.
  pi = 0;
  _next_pi = min(_next_pi, _pages.size());
  while (pi < _next_pi) {
    VertexDataBlock *block = _pages[pi]->alloc(size);
    if (block != (VertexDataBlock *)NULL) {
      _next_pi = pi;
      return block;
    }
    if (_pages[pi]->is_empty()) {
      // This page is empty, but must have been too small.  Create a
      // new page in its place.
      delete _pages[pi];
      _pages[pi] = create_new_page(size);
      return _pages[pi]->alloc(size);
    }
    ++pi;
  }

  // No page was good enough.  Create a new page.  Make it at least
  // large enough to hold this requested block.
  VertexDataPage *page = create_new_page(size);
  _pages.push_back(page);
  VertexDataBlock *block = page->alloc(size);
  return block;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBook::save_to_disk
//       Access: Published
//  Description: Writes all pages to disk immediately, just in case
//               they get evicted later.  It makes sense to make this
//               call just before taking down a loading screen, to
//               minimize chugs from saving pages inadvertently later.
////////////////////////////////////////////////////////////////////
void VertexDataBook::
save_to_disk() {
  Pages::iterator pi;
  for (pi = _pages.begin(); pi != _pages.end(); ++pi) {
    (*pi)->save_to_disk();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
VertexDataPage::
VertexDataPage(size_t page_size) : SimpleAllocator(page_size), SimpleLruPage(page_size) {
  _page_data = new unsigned char[get_max_size()];
  _size = page_size;
  _uncompressed_size = _size;
  _total_page_size += _size;
  get_class_type().inc_memory_usage(TypeHandle::MC_array, (int)_size);
  set_ram_class(RC_resident);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
VertexDataPage::
~VertexDataPage() {
  _total_page_size -= _size;
  get_class_type().dec_memory_usage(TypeHandle::MC_array, (int)_size);

  if (_page_data != NULL) {
    delete[] _page_data;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::alloc
//       Access: Published
//  Description: Allocates a new block.  Returns NULL if a block of the
//               requested size cannot be allocated.
//
//               To free the allocated block, call block->free(), or
//               simply delete the block pointer.
////////////////////////////////////////////////////////////////////
VertexDataBlock *VertexDataPage::
alloc(size_t size) {
  MutexHolder holder(_lock);
  check_resident();
  
  VertexDataBlock *block = (VertexDataBlock *)SimpleAllocator::alloc(size);

  if (block != (VertexDataBlock *)NULL) {
    // When we allocate a new block within the page, we have to clear
    // the disk cache (since we have just invalidated it).
    _saved_block.clear();
  }

  return block;
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
      make_disk();
    } else {
      make_compressed();
    }
    break;

  case RC_compressed:
    make_disk();
    break;

  case RC_disk:
    gobj_cat.warning()
      << "Cannot evict array data from disk.\n";
    break;

  case RC_end_of_list:
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataPage::make_resident
//       Access: Private
//  Description: Moves the page to fully resident status by
//               expanding it or reading it from disk as necessary.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void VertexDataPage::
make_resident() {
  if (_ram_class == RC_resident) {
    // If we're already resident, just mark the page recently used.
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

    get_class_type().dec_memory_usage(TypeHandle::MC_array, (int)_size);
    _total_page_size -= _size;

    delete[] _page_data;
    _page_data = new_data;
    _size = _uncompressed_size;

    get_class_type().inc_memory_usage(TypeHandle::MC_array, (int)_size);
    _total_page_size += _size;
  
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

    get_class_type().dec_memory_usage(TypeHandle::MC_array, (int)_size);
    _total_page_size -= _size;

    delete[] _page_data;
    _page_data = new_data;
    _size = buffer_size;

    get_class_type().inc_memory_usage(TypeHandle::MC_array, (int)_size);
    _total_page_size += _size;

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
      mark_used_lru();
      return;
    }

    get_class_type().dec_memory_usage(TypeHandle::MC_array, (int)_size);
    _total_page_size -= _size;

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

    _page_data = new_data;
    _size = buffer_size;

    get_class_type().inc_memory_usage(TypeHandle::MC_array, (int)_size);
    _total_page_size += _size;

    set_lru_size(_size);
    if (_saved_block->get_compressed()) {
      set_ram_class(RC_compressed);
    } else {
      set_ram_class(RC_resident);
    }
  }
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
