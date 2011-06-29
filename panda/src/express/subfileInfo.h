// Filename: subfileInfo.h
// Created by:  drose (20Jun11)
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

#ifndef SUBFILEINFO_H
#define SUBFILEINFO_H

#include "pandabase.h"
#include "fileReference.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : SubfileInfo
// Description : This class records a particular byte sub-range within
//               an existing file on disk.  Generally, the filename is
//               understood as a physical file on disk, and not to be
//               looked up via the vfs.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS SubfileInfo {
PUBLISHED:
  INLINE SubfileInfo();
  INLINE SubfileInfo(const FileReference *file, streampos start, streamsize size);
  INLINE SubfileInfo(const Filename &filename, streampos start, streamsize size);
  INLINE SubfileInfo(const SubfileInfo &copy);
  INLINE void operator = (const SubfileInfo &copy);

  INLINE bool is_empty() const;

  INLINE const FileReference *get_file() const;
  INLINE const Filename &get_filename() const;
  INLINE streampos get_start() const;
  INLINE streamsize get_size() const;

  void output(ostream &out) const;

private:
  CPT(FileReference) _file;
  streampos _start;
  streamsize _size;
};

INLINE ostream &operator << (ostream &out, const SubfileInfo &info);

#include "subfileInfo.I"

#endif
