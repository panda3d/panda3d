// Filename: virtualFileList.h
// Created by:  drose (03Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef VIRTUALFILELIST_H
#define VIRTUALFILELIST_H

#include "pandabase.h"

#include "virtualFile.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : VirtualFileList
// Description : A list of VirtualFiles, as returned by 
//               VirtualDirectory::scan().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS VirtualFileList : public ReferenceCount {
public:
  INLINE VirtualFileList();

PUBLISHED:
  INLINE ~VirtualFileList();

public:
  INLINE void add_file(VirtualFile *file);

PUBLISHED:
  INLINE int get_num_files() const;
  INLINE VirtualFile *get_file(int n) const;

private:
  typedef pvector< PT(VirtualFile) > Files;
  Files _files;
};


#include "virtualFileList.I"

#endif
