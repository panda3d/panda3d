/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file subfileInfo.h
 * @author drose
 * @date 2011-06-20
 */

#ifndef SUBFILEINFO_H
#define SUBFILEINFO_H

#include "pandabase.h"
#include "fileReference.h"
#include "pointerTo.h"

/**
 * This class records a particular byte sub-range within an existing file on
 * disk.  Generally, the filename is understood as a physical file on disk,
 * and not to be looked up via the vfs.
 */
class EXPCL_PANDA_EXPRESS SubfileInfo {
PUBLISHED:
  INLINE SubfileInfo();
  INLINE explicit SubfileInfo(const FileReference *file, std::streampos start, std::streamsize size);
  INLINE explicit SubfileInfo(const Filename &filename, std::streampos start, std::streamsize size);
  INLINE SubfileInfo(const SubfileInfo &copy);
  INLINE void operator = (const SubfileInfo &copy);

  INLINE bool is_empty() const;

  INLINE const FileReference *get_file() const;
  INLINE const Filename &get_filename() const;
  INLINE std::streampos get_start() const;
  INLINE std::streamsize get_size() const;

  void output(std::ostream &out) const;

private:
  CPT(FileReference) _file;
  std::streampos _start;
  std::streamsize _size;
};

INLINE std::ostream &operator << (std::ostream &out, const SubfileInfo &info);

#include "subfileInfo.I"

#endif
