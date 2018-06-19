/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rocketFileInterface.h
 * @author rdb
 * @date 2011-11-03
 */

#ifndef ROCKET_FILE_INTERFACE_H
#define ROCKET_FILE_INTERFACE_H

#include "config_rocket.h"
#include "virtualFile.h"
#include <Rocket/Core/FileInterface.h>

class VirtualFileSystem;

/**
 * Implementation of FileInterface to allow libRocket to read files from the
 * virtual file system.
 */
class RocketFileInterface : public Rocket::Core::FileInterface {
public:
  RocketFileInterface(VirtualFileSystem *vfs = nullptr);
  virtual ~RocketFileInterface() {};

  Rocket::Core::FileHandle Open(const Rocket::Core::String& path);
  void Close(Rocket::Core::FileHandle file);

  size_t Read(void* buffer, size_t size, Rocket::Core::FileHandle file);
  bool Seek(Rocket::Core::FileHandle file, long offset, int origin);
  size_t Tell(Rocket::Core::FileHandle file);

  size_t Length(Rocket::Core::FileHandle file);

protected:
  struct VirtualFileHandle {
    PT(VirtualFile) _file;
    std::istream *_stream;
  };

  VirtualFileSystem* _vfs;
};

#endif
