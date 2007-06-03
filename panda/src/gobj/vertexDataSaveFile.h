// Filename: vertexDataSaveFile.h
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

#ifndef VERTEXDATASAVEFILE_H
#define VERTEXDATASAVEFILE_H

#include "pandabase.h"
#include "simpleAllocator.h"
#include "filename.h"
#include "pmutex.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

class VertexDataSaveBlock;

////////////////////////////////////////////////////////////////////
//       Class : VertexDataSaveFile
// Description : A temporary file to hold the vertex data that has
//               been evicted from memory and written to disk.  All
//               vertex data arrays are written into one large flat
//               file.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA VertexDataSaveFile : public SimpleAllocator {
public:
  VertexDataSaveFile(const Filename &directory, const string &prefix,
                     size_t max_size);
  ~VertexDataSaveFile();

  INLINE bool is_valid() const;

  INLINE size_t get_total_file_size() const;
  INLINE size_t get_used_file_size() const;
  
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

////////////////////////////////////////////////////////////////////
//       Class : VertexDataSaveBlock
// Description : A block of bytes on the save file.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA VertexDataSaveBlock : public SimpleAllocatorBlock, public ReferenceCount {
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
