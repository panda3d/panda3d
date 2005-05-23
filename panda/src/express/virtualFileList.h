// Filename: virtualFileList.h
// Created by:  drose (03Aug02)
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
