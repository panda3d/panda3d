/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file membuffer.I
 */


#define MEMBUF_THRASH_SIZE  25

/**
 * Releases all resources(Memory USed) is locally allocated
 */
inline void MemBuffer::
ClearBuffer(void) {
  if (_BufferLocal == true) {
    if (_Buffer != NULL) {
      delete[] _Buffer;
    }

    _Buffer = NULL;
  }
}

/**
 * Locally allocate a new buffer
 */
inline void MemBuffer::
AllocBuffer(size_t len) {
  _Buffer = new char[len];
  _BufferLocal = true;
  _BufferLen = len;
}

/**
 * default constructor
 */
inline MemBuffer::
MemBuffer(void) {
  _Buffer = NULL;
  _BufferLocal = false;
  _BufferLen = 0;
}

/**
 * Constructure to locall allocate a buffer
 */
inline MemBuffer::
MemBuffer(size_t len) {
  AllocBuffer(len);
}

/**
 * Constructure to use an external buffer
 */
inline MemBuffer::
MemBuffer(char *data, size_t len) {
  _BufferLocal = false;
  _BufferLen = len;
  _Buffer = data;
}

/**
 * CLean UP a mess on Deletion
 */
inline MemBuffer::
~MemBuffer() {
  ClearBuffer();
}

/**
 * Assigns a buffer
 */
inline void MemBuffer::
SetBuffer(char * data, size_t len) {
  if (_BufferLocal == true) {
    ClearBuffer();
  }

  _BufferLocal = false;
  _BufferLen = len;
  _Buffer = data;
}

/**
 * Grow a buffer is needed to get to a sertion size No care is made here to
 * preserve convtent unlike a vector of chars
 */
inline void MemBuffer::
GrowBuffer(size_t new_len) {
  if (new_len >= _BufferLen) {
    size_t len = new_len + MEMBUF_THRASH_SIZE;
    len = len +len;

    char *tmp = new char[len];

    if (_Buffer != NULL) {
      memcpy(tmp,_Buffer,_BufferLen);
    }

    ClearBuffer();

    _Buffer = tmp;
    _BufferLocal = true;
    _BufferLen = len;
  }
}

/**
 * Access to the BUffer Size Information
 */
inline size_t MemBuffer::
GetBufferSize(void) const {
  return  _BufferLen;
}

/**
 * Access to the actual BUffer
 */
inline char *MemBuffer::
GetBuffer(void) {
  return  _Buffer;
}

inline const char *MemBuffer::
GetBuffer(void) const {
  return  _Buffer;
}

/**

 */
inline bool MemBuffer::
InBufferRange(char *inpos) {
  return (inpos >= _Buffer && inpos <= (_Buffer + _BufferLen));
}
