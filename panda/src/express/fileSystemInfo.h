// Filename: fileSystemInfo.h
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

#ifndef FILESYSTEMINFO_H
#define FILESYSTEMINFO_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : FileSystemInfo
// Description : This class is used to return data about an actual
//               file on disk by VirtualFile::get_system_info().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS FileSystemInfo {
PUBLISHED:
  INLINE FileSystemInfo();
  INLINE FileSystemInfo(const string &os_file_name, streampos file_start, streamsize file_size);
  INLINE FileSystemInfo(const FileSystemInfo &copy);
  INLINE void operator = (const FileSystemInfo &copy);

  INLINE const string &get_os_file_name() const;
  INLINE streampos get_file_start() const;
  INLINE streamsize get_file_size() const;

  void output(ostream &out) const;

private:
  string _os_file_name;
  streampos _file_start;
  streamsize _file_size;
};

INLINE ostream &operator << (ostream &out, const FileSystemInfo &info);

#include "fileSystemInfo.I"

#endif
