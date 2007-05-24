// Filename: vertexDataSaveFile.I
// Created by:  drose (12May07)
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


////////////////////////////////////////////////////////////////////
//     Function: VertexDataSaveFile::is_valid
//       Access: Public
//  Description: Returns true if the save file was successfully
//               created and is ready for use, false if there was an
//               error.
////////////////////////////////////////////////////////////////////
INLINE bool VertexDataSaveFile::
is_valid() const {
  return _is_valid;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataSaveFile::get_total_file_size
//       Access: Public
//  Description: Returns the amount of space consumed by the save
//               file, including unused portions.
////////////////////////////////////////////////////////////////////
INLINE size_t VertexDataSaveFile::
get_total_file_size() const {
  return _total_file_size;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataSaveFile::get_used_file_size
//       Access: Public
//  Description: Returns the amount of space within the save file that
//               is currently in use.
////////////////////////////////////////////////////////////////////
INLINE size_t VertexDataSaveFile::
get_used_file_size() const {
  return SimpleAllocator::get_total_size();
}


////////////////////////////////////////////////////////////////////
//     Function: VertexDataSaveBlock::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VertexDataSaveBlock::
VertexDataSaveBlock(VertexDataSaveFile *file, size_t start, size_t size) :
  SimpleAllocatorBlock(file, start, size)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataSaveBlock::set_compressed
//       Access: Public
//  Description: Sets the compressed flag.  This is true to indicate
//               the data is written in zlib-compressed form to the
//               save file; false to indicate the data is
//               uncompressed.
////////////////////////////////////////////////////////////////////
INLINE void VertexDataSaveBlock::
set_compressed(bool compressed) {
  _compressed = compressed;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataSaveBlock::get_compressed
//       Access: Public
//  Description: Returns the compressed flag.  This is true to
//               indicate the data is written in zlib-compressed form
//               to the save file; false to indicate the data is
//               uncompressed.
////////////////////////////////////////////////////////////////////
INLINE bool VertexDataSaveBlock::
get_compressed() const {
  return _compressed;
}
