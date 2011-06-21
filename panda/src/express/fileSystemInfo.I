// Filename: fileSystemInfo.I
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


////////////////////////////////////////////////////////////////////
//     Function: FileSystemInfo::Default Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FileSystemInfo::
FileSystemInfo() :
  _file_start(0),
  _file_size(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: FileSystemInfo::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FileSystemInfo::
FileSystemInfo(const string &os_file_name, streampos file_start, streamsize file_size) :
  _os_file_name(os_file_name),
  _file_start(file_start),
  _file_size(file_size)
{
}

////////////////////////////////////////////////////////////////////
//     Function: FileSystemInfo::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FileSystemInfo::
FileSystemInfo(const FileSystemInfo &copy) :
  _os_file_name(copy._os_file_name),
  _file_start(copy._file_start),
  _file_size(copy._file_size)
{
}

////////////////////////////////////////////////////////////////////
//     Function: FileSystemInfo::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FileSystemInfo::
operator = (const FileSystemInfo &copy) {
  _os_file_name = copy._os_file_name;
  _file_start = copy._file_start;
  _file_size = copy._file_size;
}

////////////////////////////////////////////////////////////////////
//     Function: FileSystemInfo::get_os_file_name
//       Access: Published
//  Description: Returns the os-specific filename that may be used to
//               open this file.
////////////////////////////////////////////////////////////////////
INLINE const string &FileSystemInfo::
get_os_file_name() const {
  return _os_file_name;
}

////////////////////////////////////////////////////////////////////
//     Function: FileSystemInfo::get_file_start
//       Access: Published
//  Description: Returns the offset within the file at which this file
//               data begins.
////////////////////////////////////////////////////////////////////
INLINE streampos FileSystemInfo::
get_file_start() const {
  return _file_start;
}

////////////////////////////////////////////////////////////////////
//     Function: FileSystemInfo::get_file_size
//       Access: Published
//  Description: Returns the number of consecutive bytes, beginning at
//               get_file_start(), that correspond to this file data.
////////////////////////////////////////////////////////////////////
INLINE streamsize FileSystemInfo::
get_file_size() const {
  return _file_size;
}

INLINE ostream &
operator << (ostream &out, const FileSystemInfo &info) {
  info.output(out);
  return out;
}
