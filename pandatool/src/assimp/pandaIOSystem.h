/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandaIOSystem.h
 * @author rdb
 * @date 2011-03-29
 */

#ifndef PANDAIOSYSTEM_H
#define PANDAIOSYSTEM_H

#include "config_assimp.h"
#include "virtualFileSystem.h"

#include <assimp/IOSystem.hpp>

/**
 * Custom implementation of Assimp::IOSystem.
 */
class PandaIOSystem : public Assimp::IOSystem {
public:
  PandaIOSystem(VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr());
  virtual ~PandaIOSystem() {};

  void Close(Assimp::IOStream *file);
  bool ComparePaths(const char *p1, const char *p2) const;
  bool Exists(const char *file) const;
  char getOsSeparator() const;
  Assimp::IOStream *Open(const char *file, const char *mode);

private:
  VirtualFileSystem *_vfs;
};

#endif
