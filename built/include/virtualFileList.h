/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileList.h
 * @author drose
 * @date 2002-08-03
 */

#ifndef VIRTUALFILELIST_H
#define VIRTUALFILELIST_H

#include "pandabase.h"

#include "virtualFile.h"
#include "pointerTo.h"

/**
 * A list of VirtualFiles, as returned by VirtualFile::scan_directory().
 */
class EXPCL_PANDA_EXPRESS VirtualFileList : public ReferenceCount {
public:
  INLINE VirtualFileList();

PUBLISHED:
  INLINE ~VirtualFileList();

public:
  INLINE void add_file(VirtualFile *file);

PUBLISHED:
  INLINE size_t get_num_files() const;
  INLINE VirtualFile *get_file(size_t n) const;
  MAKE_SEQ(get_files, get_num_files, get_file);

  INLINE VirtualFile *operator [](size_t n) const;
  INLINE size_t size() const;
  INLINE void operator += (const VirtualFileList &other);
  INLINE VirtualFileList operator + (const VirtualFileList &other) const;

private:
  typedef pvector< PT(VirtualFile) > Files;
  Files _files;
};


#include "virtualFileList.I"

#endif
