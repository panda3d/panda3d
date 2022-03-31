/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcPackData.h
 * @author drose
 * @date 2004-06-15
 */

#ifndef DCPACKDATA_H
#define DCPACKDATA_H

#include "dcbase.h"

/**
 * This is a block of data that receives the results of DCPacker.
 */
class EXPCL_DIRECT_DCPARSER DCPackData {
PUBLISHED:
  INLINE DCPackData();
  INLINE ~DCPackData();

  INLINE void clear();

public:
  INLINE void append_data(const char *buffer, size_t size);
  INLINE char *get_write_pointer(size_t size);
  INLINE void append_junk(size_t size);
  INLINE void rewrite_data(size_t position, const char *buffer, size_t size);
  INLINE char *get_rewrite_pointer(size_t position, size_t size);

PUBLISHED:
  INLINE std::string get_string() const;
  INLINE size_t get_length() const;
public:
  INLINE const char *get_data() const;
  INLINE char *take_data();

private:
  void set_used_length(size_t size);

private:
  char *_buffer;
  size_t _allocated_size;
  size_t _used_length;
};

#include "dcPackData.I"

#endif
