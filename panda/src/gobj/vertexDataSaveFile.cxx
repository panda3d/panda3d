// Filename: vertexDataSaveFile.cxx
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

#include "vertexDataSaveFile.h"

////////////////////////////////////////////////////////////////////
//     Function: VertexDataSaveFile::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
VertexDataSaveFile::
VertexDataSaveFile(const Filename &directory, const string &prefix,
                   size_t max_size) :
  _allocator(max_size)
{
  // Try to open and lock a writable temporary filename.
  int index = 0;
  while (true) {
    ++index;
    ostringstream strm;
    strm << prefix << "_" << index << ".dat";
    string basename = strm.str();
    _filename = Filename(directory, basename);
    string os_specific = _filename.to_os_specific();

#ifdef _WIN32
    // Windows case.
    _handle = CreateFile(os_specific.c_str(), GENERIC_READ | GENERIC_WRITE,
                         0, NULL, CREATE_ALWAYS, 
                         FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE | FILE_FLAG_RANDOM_ACCESS,
                         NULL);
    if (_handle == INVALID_HANDLE_VALUE) {
      // Couldn't open the file.  Either the directory was bad, or the
      // file was locked.
      DWORD err = GetLastError();
      if (err == ERROR_SHARING_VIOLATION) {
        // Try the next file.
        break;
      }

      // File couldn't be opened; permission problem or bogus directory.
      // TODO: handle this
    }

#else
    // Posix case.
    _fd = open(os_specific.c_str(), O_RDWR | O_CREAT, 0666);
    nassertv(_fd != -1);  // TODO: handle this.
    
    // Now try to lock the file, so we can be sure that no other
    // process is simultaneously writing to the same save file.
    int result = lockf(_fd, F_TLOCK, 0);
    if (result == 0) {
      // We've got the file.  Truncate it first, for good measure, in
      // case there's an old version of the file we picked up.
      ftruncate(_fd, 0);

      // On Unix, it's safe to unlink (delete) the temporary file after
      // it's been opened.  The file remains open, but disappears from
      // the directory.  This ensures it won't accidentally get left
      // behind should this process crash without closing and deleting
      // the file cleanly.
      //unlink(os_specific.c_str());
      //_filename = Filename();
      break;
    }

    // Try the next file.
    close(_fd);
#endif  // _WIN32
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataSaveFile::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
VertexDataSaveFile::
~VertexDataSaveFile() {
#ifdef _WIN32
  if (_handle != NULL) {
    CloseHandle(_handle);
  }
#else
  if (_fd != -1) {
    close(_fd);
  }
#endif  // _WIN32

  if (!_filename.empty()) {
    _filename.unlink();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataSaveFile::write_data
//       Access: Public
//  Description: Writes a block of data to the file, and returns a
//               handle to the block handle.  Returns NULL if the data
//               cannot be written (e.g. no remaining space on the
//               file).
////////////////////////////////////////////////////////////////////
SimpleAllocatorBlock *VertexDataSaveFile::
write_data(const unsigned char *data, size_t size) {
  SimpleAllocatorBlock *block = _allocator.alloc(size);
  if (block != (SimpleAllocatorBlock *)NULL) {

#ifdef _WIN32
    if (SetFilePointer(_handle, block->get_start(), NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
      gobj_cat.error()
        << "Error seeking to position " << block->get_start() << " in save file.\n";
      return false;
    }

    DWORD bytes_written;
    if (!WriteFile(_handle, data, size, &bytes_written, NULL) ||
        bytes_written != size) {
      gobj_cat.error()
        << "Error writing " << size << " bytes to save file.  Disk full?\n";
      delete block;
      return NULL;
    }

#else
    // Posix case.
    if (lseek(_fd, block->get_start(), SEEK_SET) == -1) {
      gobj_cat.error()
        << "Error seeking to position " << block->get_start() << " in save file.\n";
      return false;
    }

    ssize_t result = write(_fd, data, size);
    if (result != (ssize_t)size) {
      gobj_cat.error()
        << "Error writing " << size << " bytes to save file.  Disk full?\n";
      delete block;
      return NULL;
    }
#endif  // _WIN32

  }

  return block;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataSaveFile::read_data
//       Access: Public
//  Description: Reads a block of data from the file, and returns true
//               on success, false on failure.
////////////////////////////////////////////////////////////////////
bool VertexDataSaveFile::
read_data(unsigned char *data, size_t size, SimpleAllocatorBlock *block) {
  nassertr(size == block->get_size(), false);

#ifdef _WIN32
  if (SetFilePointer(_handle, block->get_start(), NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
    gobj_cat.error()
      << "Error seeking to position " << block->get_start() << " in save file.\n";
    return false;
  }

  DWORD bytes_read;
  if (!ReadFile(_handle, data, size, &bytes_read, NULL) ||
      bytes_read != size) {
    gobj_cat.error()
      << "Error writing " << size << " bytes to save file.  Disk full?\n";
    delete block;
    return NULL;
  }
  
#else
  // Posix case.
  if (lseek(_fd, block->get_start(), SEEK_SET) == -1) {
    gobj_cat.error()
      << "Error seeking to position " << block->get_start() << " in save file.\n";
    return false;
  }
  ssize_t result = read(_fd, data, size);
  if (result != (ssize_t)size) {
    gobj_cat.error()
      << "Error reading " << size << " bytes from save file.\n";
    return false;
  }
#endif  // _WIN32

  return true;
}
