/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlFileInterface.h
 * @author rdb
 * @date 2011-11-03
 */

#ifndef RML_FILE_INTERFACE_H
#define RML_FILE_INTERFACE_H

#include "config_rmlui.h"
#include "virtualFile.h"

#ifndef CPPPARSER
#include <RmlUi/Core/FileInterface.h>
#endif

class VirtualFileSystem;

/**
 * Bridges RmlUi file I/O to Panda's VirtualFileSystem.
 */
class RmlFileInterface
#ifndef CPPPARSER
  : public Rml::FileInterface
#endif
{
public:
  RmlFileInterface(VirtualFileSystem *vfs = nullptr);

  Rml::FileHandle Open(const Rml::String &path) override;
  void Close(Rml::FileHandle file) override;
  size_t Read(void *buffer, size_t size, Rml::FileHandle file) override;
  bool Seek(Rml::FileHandle file, long offset, int origin) override;
  size_t Tell(Rml::FileHandle file) override;
  size_t Length(Rml::FileHandle file) override;

protected:
  struct VirtualFileHandle {
    PT(VirtualFile) _file;
    std::istream *_stream;
  };

  VirtualFileSystem *_vfs;
};

#endif
