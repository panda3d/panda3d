/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file membuffer.h
 */

#ifndef __MEMBUFFER_GM_H__
#define __MEMBUFFER_GM_H__

// RHH
/**
 * This a base class designed to be used to for items that will share portions
 * of a memory buffer and want to avoid copying the data.  Use if the class
 * wants to allow for reference in place of data arrays.  ** be careful could be
 * dangerous **  GmCoreMessage GmRingBuffer
 */
class EXPCL_PANDA_NATIVENET MemBuffer {
public:
  inline      MemBuffer(void);
  inline      MemBuffer(size_t len);
  inline      MemBuffer(char * data, size_t len);
  virtual ~MemBuffer();
  inline void SetBuffer(char * data, size_t len);
  inline void GrowBuffer(size_t len);
  inline size_t  GetBufferSize(void ) const;
  inline char * GetBuffer(void);
  inline const char * GetBuffer(void) const;
  inline bool InBufferRange(char * );

protected:
  bool        _BufferLocal;  // indicates responsibility of managment of the data
  size_t          _BufferLen; // the length of the data
  char    *   _Buffer;        // the data

  inline void ClearBuffer(void);
  inline void AllocBuffer(size_t len);
};

#include "membuffer.I"

#endif //__MEMBUFFER_GM_H__
