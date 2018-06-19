/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexDataSaveFile.h
 * @author drose
 * @date 2007-05-12
 */

#ifndef VERTEXDATASAVEFILE_H
#define VERTEXDATASAVEFILE_H

#include "pandabase.h"
#include "simpleAllocator.h"
#include "filename.h"
#include "pmutex.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

class VertexDataSaveBlock;

/**
 * A temporary file to hold the vertex data that has been evicted from memory
 * and written to disk.  All vertex data arrays are written into one large
 * flat file.
 */
class EXPCL_PANDA_GOBJ VertexDataSaveFile : public SimpleAllocator {
public:
  VertexDataSaveFile(const Filename &directory, const std::string &prefix,
                     size_t max_size);
  ~VertexDataSaveFile();

PUBLISHED:
  INLINE bool is_valid() const;

  INLINE size_t get_total_file_size() const;
  INLINE size_t get_used_file_size() const;

public:
  PT(VertexDataSaveBlock) write_data(const unsigned char *data, size_t size,
                                     bool compressed);
  bool read_data(unsigned char *data, size_t size,
                 VertexDataSaveBlock *block);

protected:
  virtual SimpleAllocatorBlock *make_block(size_t start, size_t size);

private:
  Filename _filename;
  bool _is_valid;
  size_t _total_file_size;
  Mutex _lock;

#ifdef _WIN32
  HANDLE _handle;
#else
  int _fd;  // Posix file descriptor
#endif  // _WIN32
};

/**
 * A block of bytes on the save file.
 */
class EXPCL_PANDA_GOBJ VertexDataSaveBlock : public SimpleAllocatorBlock, public ReferenceCount {
protected:
  INLINE VertexDataSaveBlock(VertexDataSaveFile *file,
                             size_t start, size_t size);

public:
  INLINE void set_compressed(bool compressed);
  INLINE bool get_compressed() const;

private:
  bool _compressed;

public:
  INLINE unsigned char *get_pointer() const;

  friend class VertexDataSaveFile;
};

#include "vertexDataSaveFile.I"

#endif
