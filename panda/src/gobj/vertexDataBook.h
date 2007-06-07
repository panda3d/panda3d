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

#ifndef VERTEXDATABOOK_H
#define VERTEXDATABOOK_H

#include "pandabase.h"
#include "pmutex.h"
#include "mutexHolder.h"
#include "vertexDataPage.h"
#include "indirectLess.h"
#include "plist.h"

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

  INLINE VertexDataBlock *alloc(size_t size);

  INLINE int get_num_pages() const;

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
