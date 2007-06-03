// Filename: vertexDataBook.h
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

#ifndef VERTEXDATAPAGE_H
#define VERTEXDATAPAGE_H

#include "pandabase.h"
#include "simpleLru.h"
#include "simpleAllocator.h"
#include "referenceCount.h"
#include "pStatCollector.h"
#include "vertexDataSaveFile.h"
#include "pmutex.h"
#include "mutexHolder.h"

class VertexDataPage;
class VertexDataBlock;

////////////////////////////////////////////////////////////////////
//       Class : VertexDataBook
// Description : A collection of VertexDataPages, which can be used to
//               allocate new VertexDataBlock objects.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA VertexDataBook {
PUBLISHED:
  VertexDataBook(size_t block_size);
  ~VertexDataBook();

  VertexDataBlock *alloc(size_t size);

  INLINE int get_num_pages() const;
  INLINE VertexDataPage *get_page(int n) const;

  void save_to_disk();

private:
  INLINE VertexDataPage *create_new_page(size_t size);

private:
  size_t _block_size;
  typedef pvector<VertexDataPage *> Pages;
  Pages _pages;
  size_t _next_pi;
  Mutex _lock;
};

////////////////////////////////////////////////////////////////////
//       Class : VertexDataPage
// Description : A block of bytes that holds one or more
//               VertexDataBlocks.  The entire page may be paged out,
//               in the form of in-memory compression or to an on-disk
//               cache file, if necessary.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA VertexDataPage : public SimpleAllocator, public SimpleLruPage {
PUBLISHED:
  VertexDataPage(size_t page_size);
  ~VertexDataPage();

  // These are used to indicate the current residency state of the
  // page, which may or may not have been temporarily evicted to
  // satisfy memory requirements.
  enum RamClass {
    RC_resident,
    RC_compressed,
    RC_disk,

    RC_end_of_list,  // list marker; do not use
  };

  INLINE RamClass get_ram_class() const;

  VertexDataBlock *alloc(size_t size);
  INLINE VertexDataBlock *get_first_block() const;

  INLINE static size_t get_total_page_size();
  INLINE static SimpleLru *get_global_lru(RamClass rclass);
  INLINE static VertexDataSaveFile *get_save_file();

  INLINE bool save_to_disk();
  INLINE void restore_from_disk();

public:
  INLINE unsigned char *get_page_data() const;

protected:
  virtual SimpleAllocatorBlock *make_block(size_t start, size_t size);
  virtual void evict_lru();

private:
  INLINE void check_resident() const;

  void make_resident();
  void make_compressed();
  void make_disk();

  bool do_save_to_disk();
  void do_restore_from_disk();

  INLINE void set_ram_class(RamClass ram_class);
  static void make_save_file();

  unsigned char *_page_data;
  size_t _size, _uncompressed_size;
  RamClass _ram_class;
  PT(VertexDataSaveBlock) _saved_block;

  Mutex _lock;

  static SimpleLru _resident_lru;
  static SimpleLru _compressed_lru;
  static SimpleLru _disk_lru;
  static SimpleLru *_global_lru[RC_end_of_list];

  static size_t _total_page_size;
  static VertexDataSaveFile *_save_file;

  static PStatCollector _vdata_compress_pcollector;
  static PStatCollector _vdata_decompress_pcollector;
  static PStatCollector _vdata_save_pcollector;
  static PStatCollector _vdata_restore_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "VertexDataPage");
  }

private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : VertexDataBlock
// Description : A block of bytes that stores the actual raw vertex
//               data referenced by a GeomVertexArrayData object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA VertexDataBlock : public SimpleAllocatorBlock, public ReferenceCount {
protected:
  INLINE VertexDataBlock(VertexDataPage *page,
                         size_t start, size_t size);

PUBLISHED:
  INLINE VertexDataPage *get_page() const;
  INLINE VertexDataBlock *get_next_block() const;

public:
  INLINE unsigned char *get_pointer() const;

  friend class VertexDataPage;
};

#include "vertexDataBook.I"

#endif
