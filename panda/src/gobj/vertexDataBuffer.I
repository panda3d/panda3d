// Filename: vertexDataBuffer.I
// Created by:  drose (14May07)
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
//     Function: VertexDataBuffer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VertexDataBuffer::
VertexDataBuffer() :
  _resident_data(NULL),
  _size(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VertexDataBuffer::
VertexDataBuffer(size_t size) :
  _resident_data(NULL),
  _size(0)
{
  unclean_realloc(size);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VertexDataBuffer::
VertexDataBuffer(const VertexDataBuffer &copy) :
  _resident_data(NULL),
  _size(0)
{
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void VertexDataBuffer::
operator = (const VertexDataBuffer &copy) {
  unclean_realloc(copy.get_size());
  memcpy(_resident_data, copy.get_read_pointer(), _size);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VertexDataBuffer::
~VertexDataBuffer() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::get_read_pointer
//       Access: Public
//  Description: Returns a read-only pointer to the raw data.
////////////////////////////////////////////////////////////////////
INLINE const unsigned char *VertexDataBuffer::
get_read_pointer() const {
  if (_block != (VertexDataBlock *)NULL) {
    // We don't necessarily need to page the buffer all the way into
    // independent status; it's sufficient just to return the block's
    // pointer, which will force its page to resident status.
    return _block->get_pointer();
  }
  return _resident_data;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::get_write_pointer
//       Access: Public
//  Description: Returns a writable pointer to the raw data.
////////////////////////////////////////////////////////////////////
INLINE unsigned char *VertexDataBuffer::
get_write_pointer() {
  if (_block != (VertexDataBlock *)NULL) {
    page_in();
  }
  return _resident_data;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::get_size
//       Access: Public
//  Description: Returns the number of bytes in the buffer.
////////////////////////////////////////////////////////////////////
INLINE size_t VertexDataBuffer::
get_size() const {
  return _size;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::unclean_realloc
//       Access: Public
//  Description: Changes the size of the buffer, without regard to
//               preserving its data.  The buffer may contain random
//               data after this call.
////////////////////////////////////////////////////////////////////
INLINE void VertexDataBuffer::
unclean_realloc(size_t size) {
  // At the moment, this has no distinct definition, since the system
  // realloc() call doesn't have an unclean variant.
  clean_realloc(size);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::clear
//       Access: Public
//  Description: Empties the buffer and sets its size to 0.
////////////////////////////////////////////////////////////////////
INLINE void VertexDataBuffer::
clear() {
  unclean_realloc(0);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::swap
//       Access: Public
//  Description: Swaps the data buffers between this one and the other
//               one.
////////////////////////////////////////////////////////////////////
INLINE void VertexDataBuffer::
swap(VertexDataBuffer &other) {
  unsigned char *resident_data = _resident_data;
  size_t size = _size;
  PT(VertexDataBlock) block = _block;

  _resident_data = other._resident_data;
  _size = other._size;
  _block = other._block;

  other._resident_data = resident_data;
  other._size = size;
  other._block = block;
}
