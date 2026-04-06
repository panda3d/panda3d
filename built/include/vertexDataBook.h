/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexDataBook.h
 * @author drose
 * @date 2007-05-16
 */

#ifndef VERTEXDATABOOK_H
#define VERTEXDATABOOK_H

#include "pandabase.h"
#include "pmutex.h"
#include "mutexHolder.h"
#include "vertexDataPage.h"
#include "indirectLess.h"
#include "plist.h"

class VertexDataBlock;

/**
 * A collection of VertexDataPages, which can be used to allocate new
 * VertexDataBlock objects.
 */
class EXPCL_PANDA_GOBJ VertexDataBook {
PUBLISHED:
  explicit VertexDataBook(size_t block_size);
  ~VertexDataBook();

  INLINE VertexDataBlock *alloc(size_t size);

  INLINE size_t get_num_pages() const;

  size_t count_total_page_size() const;
  size_t count_total_page_size(VertexDataPage::RamClass ram_class) const;
  size_t count_allocated_size() const;
  size_t count_allocated_size(VertexDataPage::RamClass ram_class) const;

  void save_to_disk();

public:
  void reorder_page(VertexDataPage *page);

private:
  INLINE VertexDataPage *create_new_page(size_t size);
  VertexDataBlock *do_alloc(size_t size);

private:
  size_t _block_size;

  typedef pset<VertexDataPage *, IndirectLess<VertexDataPage> > Pages;
  Pages _pages;

  Mutex _lock;
  friend class VertexDataPage;
};

#include "vertexDataBook.I"

#endif
