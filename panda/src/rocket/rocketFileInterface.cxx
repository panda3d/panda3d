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
  void *ptr = (void*) _vfs->open_read_file(fn, true);

  if (ptr == NULL) {
    rocket_cat.error() << "Failed to open " << fn << "\n";
  }

  // A FileHandle is actually just a void pointer
  return (Rocket::Core::FileHandle) ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: RocketFileInterface::Close
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void RocketFileInterface::
Close(Rocket::Core::FileHandle file) {
  if ((istream*) file != (istream*) NULL) {
    _vfs->close_read_file((istream*) file);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RocketFileInterface::Read
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
size_t RocketFileInterface::
Read(void* buffer, size_t size, Rocket::Core::FileHandle file) {
  istream* const stream = (istream*) file;
  if (stream == (istream*) NULL) {
    return 0;
  }

  stream->read((char*) buffer, size);
  return stream->gcount();
}

////////////////////////////////////////////////////////////////////
//     Function: RocketFileInterface::Seek
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool RocketFileInterface::
Seek(Rocket::Core::FileHandle file, long offset, int origin) {
  istream* stream = (istream*) file;
  if (stream == (istream*) NULL) {
    return false;
  }

  switch(origin) {
  case SEEK_SET:
    stream->seekg(offset, ios::beg);
    break;
  case SEEK_CUR:
    stream->seekg(offset, ios::cur);
    break;
  case SEEK_END:
    stream->seekg(offset, ios::end);
  };

  return !stream->fail();
}

////////////////////////////////////////////////////////////////////
//     Function: RocketFileInterface::Tell
//       Access: Public   
//  Description: 
////////////////////////////////////////////////////////////////////
size_t RocketFileInterface::
Tell(Rocket::Core::FileHandle file) {
  if ((istream*) file == (istream*) NULL) {
    return -1;
  }
  return ((istream*) file)->tellg();
}
