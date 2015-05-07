// Filename: rocketFileInterface.cxx
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

#include "rocketFileInterface.h"
#include "virtualFileSystem.h"

////////////////////////////////////////////////////////////////////
//     Function: RocketFileInterface::Constructor
//       Access: Public
//  Description: Constructs a RocketFileInterface for the given
//               VFS, or the default if NULL is given.
////////////////////////////////////////////////////////////////////
RocketFileInterface::
RocketFileInterface(VirtualFileSystem *vfs) : _vfs(vfs) {
  if (_vfs == NULL) {
    _vfs = VirtualFileSystem::get_global_ptr();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RocketFileInterface::Open
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Rocket::Core::FileHandle RocketFileInterface::
Open(const Rocket::Core::String& path) {
  rocket_cat.debug() << "Opening " << path.CString() << "\n";

  Filename fn = Filename::from_os_specific(path.CString());

  PT(VirtualFile) file = _vfs->get_file(fn);
  if (file == NULL) {
    // failed?  Try model-path as a Panda-friendly fallback.
    if (!_vfs->resolve_filename(fn, get_model_path())) {
      rocket_cat.error() << "Could not resolve " << fn
          << " along the model-path (currently: " << get_model_path() << ")\n";
      return (Rocket::Core::FileHandle) NULL;
    }

    file = _vfs->get_file(fn);
    if (file == NULL) {
      rocket_cat.error() << "Failed to get " << fn << ", found on model-path\n";
      return (Rocket::Core::FileHandle) NULL;
    }
  }

  istream *str = file->open_read_file(true);
  if (str == NULL) {
    rocket_cat.error() << "Failed to open " << fn << " for reading\n";
    return (Rocket::Core::FileHandle) NULL;
  }

  VirtualFileHandle *handle = new VirtualFileHandle;
  handle->_file = file;
  handle->_stream = str;

  // A FileHandle is actually just a void pointer.
  return (Rocket::Core::FileHandle) handle;
}

////////////////////////////////////////////////////////////////////
//     Function: RocketFileInterface::Close
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void RocketFileInterface::
Close(Rocket::Core::FileHandle file) {
  VirtualFileHandle *handle = (VirtualFileHandle*) file;
  if (handle == NULL) {
    return;
  }

  _vfs->close_read_file(handle->_stream);
  delete handle;
}

////////////////////////////////////////////////////////////////////
//     Function: RocketFileInterface::Read
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
size_t RocketFileInterface::
Read(void* buffer, size_t size, Rocket::Core::FileHandle file) {
  VirtualFileHandle *handle = (VirtualFileHandle*) file;
  if (handle == NULL) {
    return 0;
  }

  handle->_stream->read((char*) buffer, size);
  return handle->_stream->gcount();
}

////////////////////////////////////////////////////////////////////
//     Function: RocketFileInterface::Seek
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool RocketFileInterface::
Seek(Rocket::Core::FileHandle file, long offset, int origin) {
  VirtualFileHandle *handle = (VirtualFileHandle*) file;
  if (handle == NULL) {
    return false;
  }

  switch(origin) {
  case SEEK_SET:
    handle->_stream->seekg(offset, ios::beg);
    break;
  case SEEK_CUR:
    handle->_stream->seekg(offset, ios::cur);
    break;
  case SEEK_END:
    handle->_stream->seekg(offset, ios::end);
  };

  return !handle->_stream->fail();
}

////////////////////////////////////////////////////////////////////
//     Function: RocketFileInterface::Tell
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
size_t RocketFileInterface::
Tell(Rocket::Core::FileHandle file) {
  VirtualFileHandle *handle = (VirtualFileHandle*) file;
  if (handle == NULL) {
    return 0;
  }

  return handle->_stream->tellg();
}

////////////////////////////////////////////////////////////////////
//     Function: RocketFileInterface::Length
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
size_t RocketFileInterface::
Length(Rocket::Core::FileHandle file) {
  VirtualFileHandle *handle = (VirtualFileHandle*) file;
  if (handle == NULL) {
    return 0;
  }

  return handle->_file->get_file_size(handle->_stream);
}
