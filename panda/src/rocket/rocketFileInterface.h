// Filename: rocketFileInterface.h
// Created by:  rdb (03Nov11)
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

#ifndef ROCKET_FILE_INTERFACE_H
#define ROCKET_FILE_INTERFACE_H

#include "config_rocket.h"

#include <Rocket/Core/FileInterface.h>

class VirtualFileSystem;

class RocketFileInterface : public Rocket::Core::FileInterface {
public:
  RocketFileInterface(VirtualFileSystem *vfs = NULL);
  virtual ~RocketFileInterface() {};

  Rocket::Core::FileHandle Open(const Rocket::Core::String& path);
  void Close(Rocket::Core::FileHandle file);

  size_t Read(void* buffer, size_t size, Rocket::Core::FileHandle file);
  bool Seek(Rocket::Core::FileHandle file, long offset, int origin);
  size_t Tell(Rocket::Core::FileHandle file);

protected:
  VirtualFileSystem* _vfs;
};

#endif
