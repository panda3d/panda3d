/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rocketFileInterface.cxx
 * @author rdb
 * @date 2011-11-03
 */

#include "rocketFileInterface.h"
#include "virtualFileSystem.h"

/**
 * Constructs a RocketFileInterface for the given VFS, or the default if NULL
 * is given.
 */
RocketFileInterface::
RocketFileInterface(VirtualFileSystem *vfs) : _vfs(vfs) {
  if (_vfs == nullptr) {
    _vfs = VirtualFileSystem::get_global_ptr();
  }
}

/**
 *
 */
Rocket::Core::FileHandle RocketFileInterface::
Open(const Rocket::Core::String& path) {
  if (rocket_cat.is_debug()) {
    rocket_cat.debug() << "Opening " << path.CString() << "\n";
  }

  Filename fn = Filename::from_os_specific(path.CString());

  PT(VirtualFile) file = _vfs->get_file(fn);
  if (file == nullptr) {
    // failed?  Try model-path as a Panda-friendly fallback.
    if (!_vfs->resolve_filename(fn, get_model_path())) {
      rocket_cat.error() << "Could not resolve " << fn
          << " along the model-path (currently: " << get_model_path() << ")\n";
      return (Rocket::Core::FileHandle) nullptr;
    }

    file = _vfs->get_file(fn);
    if (file == nullptr) {
      rocket_cat.error() << "Failed to get " << fn << ", found on model-path\n";
      return (Rocket::Core::FileHandle) nullptr;
    }
  }

  std::istream *str = file->open_read_file(true);
  if (str == nullptr) {
    rocket_cat.error() << "Failed to open " << fn << " for reading\n";
    return (Rocket::Core::FileHandle) nullptr;
  }

  VirtualFileHandle *handle = new VirtualFileHandle;
  handle->_file = file;
  handle->_stream = str;

  // A FileHandle is actually just a void pointer.
  return (Rocket::Core::FileHandle) handle;
}

/**
 *
 */
void RocketFileInterface::
Close(Rocket::Core::FileHandle file) {
  VirtualFileHandle *handle = (VirtualFileHandle*) file;
  if (handle == nullptr) {
    return;
  }

  _vfs->close_read_file(handle->_stream);
  delete handle;
}

/**
 *
 */
size_t RocketFileInterface::
Read(void* buffer, size_t size, Rocket::Core::FileHandle file) {
  VirtualFileHandle *handle = (VirtualFileHandle*) file;
  if (handle == nullptr) {
    return 0;
  }

  handle->_stream->read((char*) buffer, size);
  return handle->_stream->gcount();
}

/**
 *
 */
bool RocketFileInterface::
Seek(Rocket::Core::FileHandle file, long offset, int origin) {
  VirtualFileHandle *handle = (VirtualFileHandle*) file;
  if (handle == nullptr) {
    return false;
  }

  switch(origin) {
  case SEEK_SET:
    handle->_stream->seekg(offset, std::ios::beg);
    break;
  case SEEK_CUR:
    handle->_stream->seekg(offset, std::ios::cur);
    break;
  case SEEK_END:
    handle->_stream->seekg(offset, std::ios::end);
  };

  return !handle->_stream->fail();
}

/**
 *
 */
size_t RocketFileInterface::
Tell(Rocket::Core::FileHandle file) {
  VirtualFileHandle *handle = (VirtualFileHandle*) file;
  if (handle == nullptr) {
    return 0;
  }

  return handle->_stream->tellg();
}

/**
 *
 */
size_t RocketFileInterface::
Length(Rocket::Core::FileHandle file) {
  VirtualFileHandle *handle = (VirtualFileHandle*) file;
  if (handle == nullptr) {
    return 0;
  }

  return handle->_file->get_file_size(handle->_stream);
}
