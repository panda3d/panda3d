/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexDataPage.h
 * @author drose
 * @date 2007-06-04
 */

#ifndef VERTEXDATAPAGE_H
#define VERTEXDATAPAGE_H

#include "pandabase.h"
#include "simpleLru.h"
#include "simpleAllocator.h"
#include "pStatCollector.h"
#include "vertexDataSaveFile.h"
#include "pmutex.h"
#include "conditionVar.h"
#include "thread.h"
#include "mutexHolder.h"
#include "pdeque.h"

class VertexDataBook;
class VertexDataBlock;

/**
 * A block of bytes that holds one or more VertexDataBlocks.  The entire page
 * may be paged out, in the form of in-memory compression or to an on-disk
 * cache file, if necessary.
 */
class EXPCL_PANDA_GOBJ VertexDataPage : public SimpleAllocator, public SimpleLruPage {
private:
  VertexDataPage(size_t book_size);
  VertexDataPage(VertexDataBook *book, size_t page_size, size_t block_size);
  virtual ~VertexDataPage();

PUBLISHED:
  // These are used to indicate the current residency state of the page, which
  // may or may not have been temporarily evicted to satisfy memory
  // requirements.
  enum RamClass {
    RC_resident,
    RC_compressed,
    RC_disk,

    RC_end_of_list,  // list marker; do not use
  };

  INLINE RamClass get_ram_class() const;
  INLINE RamClass get_pending_ram_class() const;
  INLINE void request_resident();

  INLINE VertexDataBlock *alloc(size_t size);
  INLINE VertexDataBlock *get_first_block() const;

  INLINE VertexDataBook *get_book() const;

  INLINE static SimpleLru *get_global_lru(RamClass rclass);
  INLINE static SimpleLru *get_pending_lru();
  INLINE static VertexDataSaveFile *get_save_file();
  MAKE_PROPERTY(save_file, get_save_file);

  INLINE bool save_to_disk();

  INLINE static int get_num_threads();
  INLINE static int get_num_pending_reads();
  INLINE static int get_num_pending_writes();
  static void stop_threads();
  static void flush_threads();

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

public:
  INLINE unsigned char *get_page_data(bool force);
  INLINE bool operator < (const VertexDataPage &other) const;

protected:
  virtual SimpleAllocatorBlock *make_block(size_t start, size_t size);
  virtual void changed_contiguous();
  virtual void evict_lru();

private:
  class PageThread;

  VertexDataBlock *do_alloc(size_t size);

  void make_resident_now();
  void make_resident();
  void make_compressed();
  void make_disk();

  bool do_save_to_disk();
  void do_restore_from_disk();

  void adjust_book_size();

  void request_ram_class(RamClass ram_class);
  INLINE void set_ram_class(RamClass ram_class);
  static void make_save_file();

  INLINE size_t round_up(size_t page_size) const;
  unsigned char *alloc_page_data(size_t page_size) const;
  void free_page_data(unsigned char *page_data, size_t page_size) const;

  typedef pdeque<VertexDataPage *> PendingPages;

  class PageThreadManager;
  class EXPCL_PANDA_GOBJ PageThread : public Thread {
  public:
    PageThread(PageThreadManager *manager, const std::string &name);

  protected:
    virtual void thread_main();

  private:
    PageThreadManager *_manager;
    VertexDataPage *_working_page;

    // Signaled when _working_page is set to NULL after finishing a task.
    ConditionVar _working_cvar;
    friend class PageThreadManager;
  };
  typedef pvector<PT(PageThread) > PageThreads;

  class EXPCL_PANDA_GOBJ PageThreadManager : public ReferenceCount {
  public:
    PageThreadManager(int num_threads);
    void add_page(VertexDataPage *page, RamClass ram_class);
    void remove_page(VertexDataPage *page);
    int get_num_threads() const;
    int get_num_pending_reads() const;
    int get_num_pending_writes() const;
    void start_threads(int num_threads);
    void stop_threads();

  private:
    PendingPages _pending_writes;
    PendingPages _pending_reads;
    bool _shutdown;

    // Signaled when anything new is added to either of the above queues, or
    // when _shutdown is set true.  This wakes up any pending thread.
    ConditionVar _pending_cvar;

    PageThreads _threads;
    friend class PageThread;
  };

  static PT(PageThreadManager) _thread_mgr;
  static Mutex &_tlock;  // Protects _thread_mgr and all of its members.

  unsigned char *_page_data;
  size_t _size, _allocated_size, _uncompressed_size;
  RamClass _ram_class;
  PT(VertexDataSaveBlock) _saved_block;
  size_t _book_size;
  size_t _block_size;

  // Mutex _lock;   Inherited from SimpleAllocator.  Protects above members.
  RamClass _pending_ram_class;  // Protected by _tlock.

  VertexDataBook *_book;  // never changes.

  enum { deflate_page_size = 1024, inflate_page_size = 1024 };

  // We build up a temporary linked list of these while deflating
  // (compressing) the vertex data in-memory.
  class EXPCL_PANDA_GOBJ DeflatePage {
  public:
    DeflatePage() {
      _used_size = 0;
      _next = nullptr;
    }
    ALLOC_DELETED_CHAIN(DeflatePage);

    unsigned char _buffer[deflate_page_size];
    size_t _used_size;
    DeflatePage *_next;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      register_type(_type_handle, "VertexDataPage::DeflatePage");
    }

  private:
    static TypeHandle _type_handle;
  };

  static SimpleLru _resident_lru;
  static SimpleLru _compressed_lru;
  static SimpleLru _disk_lru;
  static SimpleLru _pending_lru;
  static SimpleLru *_global_lru[RC_end_of_list];

  static VertexDataSaveFile *_save_file;

  static Mutex _unused_mutex;

  static PStatCollector _vdata_compress_pcollector;
  static PStatCollector _vdata_decompress_pcollector;
  static PStatCollector _vdata_save_pcollector;
  static PStatCollector _vdata_restore_pcollector;
  static PStatCollector _thread_wait_pcollector;
  static PStatCollector _alloc_pages_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "VertexDataPage");
  }

private:
  static TypeHandle _type_handle;

  friend class PageThread;
  friend class VertexDataBook;
};

inline std::ostream &operator << (std::ostream &out, const VertexDataPage &page) {
  page.output(out);
  return out;
}

#include "vertexDataPage.I"

#endif
