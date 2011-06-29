// Filename: subfileInfo.I
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
//     Function: SubfileInfo::Default Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE SubfileInfo::
SubfileInfo() :
  _start(0),
  _size(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SubfileInfo::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE SubfileInfo::
SubfileInfo(const FileReference *file, streampos start, streamsize size) :
  _file(file),
  _start(start),
  _size(size)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SubfileInfo::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE SubfileInfo::
SubfileInfo(const Filename &filename, streampos start, streamsize size) :
  _file(new FileReference(filename)),
  _start(start),
  _size(size)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SubfileInfo::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE SubfileInfo::
SubfileInfo(const SubfileInfo &copy) :
  _file(copy._file),
  _start(copy._start),
  _size(copy._size)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SubfileInfo::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void SubfileInfo::
operator = (const SubfileInfo &copy) {
  _file = copy._file;
  _start = copy._start;
  _size = copy._size;
}

////////////////////////////////////////////////////////////////////
//     Function: SubfileInfo::is_empty
//       Access: Published
//  Description: Returns true if this SubfileInfo doesn't define any
//               file, false if it has real data.
////////////////////////////////////////////////////////////////////
INLINE bool SubfileInfo::
is_empty() const {
  return _file == (FileReference *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: SubfileInfo::get_file
//       Access: Published
//  Description: Returns the FileReference that represents this file.
////////////////////////////////////////////////////////////////////
INLINE const FileReference *SubfileInfo::
get_file() const {
  return _file;
}

////////////////////////////////////////////////////////////////////
//     Function: SubfileInfo::get_filename
//       Access: Published
//  Description: A shortcut to the filename.
////////////////////////////////////////////////////////////////////
INLINE const Filename &SubfileInfo::
get_filename() const {
  if (_file != (FileReference *)NULL) {
    return _file->get_filename();
  }
  static const Filename empty_filename;
  return empty_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: SubfileInfo::get_start
//       Access: Published
//  Description: Returns the offset within the file at which this file
//               data begins.
////////////////////////////////////////////////////////////////////
INLINE streampos SubfileInfo::
get_start() const {
  return _start;
}

////////////////////////////////////////////////////////////////////
//     Function: SubfileInfo::get_size
//       Access: Published
//  Description: Returns the number of consecutive bytes, beginning at
//               get_start(), that correspond to this file data.
////////////////////////////////////////////////////////////////////
INLINE streamsize SubfileInfo::
get_size() const {
  return _size;
}

INLINE ostream &
operator << (ostream &out, const SubfileInfo &info) {
  info.output(out);
  return out;
}
