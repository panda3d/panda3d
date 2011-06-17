// Filename: vertexDataBuffer.I
// Created by:  drose (14May07)
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
//     Function: VertexDataBuffer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VertexDataBuffer::
VertexDataBuffer() :
  _resident_data(NULL),
  _size(0),
  _reserved_size(0)
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
  _size(0),
  _reserved_size(0)
{
  do_unclean_realloc(size);
  _size = size;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VertexDataBuffer::
VertexDataBuffer(const VertexDataBuffer &copy) :
  _resident_data(NULL),
  _size(0),
  _reserved_size(0)
{
  (*this) = copy;
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
//  Description: Returns a read-only pointer to the raw data, or NULL
//               if the data is not currently resident.  If the data
//               is not currently resident, this will implicitly
//               request it to become resident soon.
//
//               If force is true, this method will never return NULL
//               (unless the data is actually empty), but may block
//               until the data is available.
////////////////////////////////////////////////////////////////////
INLINE const unsigned char *VertexDataBuffer::
get_read_pointer(bool force) const {
  LightMutexHolder holder(_lock);

  if (_resident_data != (unsigned char *)NULL || _size == 0) {
    return _resident_data;
  }

  nassertr(_block != (VertexDataBlock *)NULL, NULL);
  nassertr(_reserved_size >= _size, NULL);

  // We don't necessarily need to page the buffer all the way into
  // independent status; it's sufficient just to return the block's
  // pointer, which will force its page to resident status.
  return _block->get_pointer(force);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::get_write_pointer
//       Access: Public
//  Description: Returns a writable pointer to the raw data.
////////////////////////////////////////////////////////////////////
INLINE unsigned char *VertexDataBuffer::
get_write_pointer() { 
  LightMutexHolder holder(_lock);

  if (_resident_data == (unsigned char *)NULL && _reserved_size != 0) {
    do_page_in();
  }
  nassertr(_reserved_size >= _size, NULL);
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
//     Function: VertexDataBuffer::get_reserved_size
//       Access: Public
//  Description: Returns the total number of bytes "reserved" in the
//               buffer.  This may be greater than or equal to
//               get_size().  If it is greater, the additional bytes
//               are extra unused bytes in the buffer, and this
//               indicates the maximum value that may be passed to
//               set_size() without first calling one of the realloc
//               methods.
////////////////////////////////////////////////////////////////////
INLINE size_t VertexDataBuffer::
get_reserved_size() const {
  return _reserved_size;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::set_size
//       Access: Public
//  Description: Changes the size of the buffer.  The new size must be
//               less than or equal to the "reserved" size, which can
//               only be changed via clean_realloc() or
//               unclean_realloc().
////////////////////////////////////////////////////////////////////
INLINE void VertexDataBuffer::
set_size(size_t size) {
  LightMutexHolder holder(_lock);
  nassertv(size <= _reserved_size);

  if (size != _size) {
    if (_resident_data == (unsigned char *)NULL && _reserved_size != 0) {
      do_page_in();
    }

    _size = size;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::clean_realloc
//       Access: Public
//  Description: Changes the "reserved" size of the buffer, preserving
//               its data (except for any data beyond the new end of
//               the buffer, if the buffer is being reduced).  If the
//               buffer is expanded, the new data is uninitialized.
//
//               It is an error to set the reserved size smaller than
//               the size specified with set_size().
////////////////////////////////////////////////////////////////////
INLINE void VertexDataBuffer::
clean_realloc(size_t reserved_size) {
  LightMutexHolder holder(_lock);
  do_clean_realloc(reserved_size);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::unclean_realloc
//       Access: Public
//  Description: Changes the size of the buffer, without regard to
//               preserving its data.  The buffer may contain random
//               data after this call.
//
//               It is an error to set the reserved size smaller than
//               the size specified with set_size().
////////////////////////////////////////////////////////////////////
INLINE void VertexDataBuffer::
unclean_realloc(size_t reserved_size) {
  LightMutexHolder holder(_lock);
  do_unclean_realloc(reserved_size);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::clear
//       Access: Public
//  Description: Empties the buffer and sets its size to 0.
////////////////////////////////////////////////////////////////////
INLINE void VertexDataBuffer::
clear() {
  LightMutexHolder holder(_lock);
  _size = 0;
  do_unclean_realloc(0);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::page_out
//       Access: Public
//  Description: Moves the buffer out of independent memory and puts
//               it on a page in the indicated book.  The buffer may
//               still be directly accessible as long as its page
//               remains resident.  Any subsequent attempt to rewrite
//               the buffer will implicitly move it off of the page
//               and back into independent memory.
////////////////////////////////////////////////////////////////////
INLINE void VertexDataBuffer::
page_out(VertexDataBook &book) {
  LightMutexHolder holder(_lock);
  do_page_out(book);
}
