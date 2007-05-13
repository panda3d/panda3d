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
  Filename dir;
  if (directory.empty()) {
    dir = Filename::get_temp_directory();
  } else {
    dir = directory;
  }

  _is_valid = false;

  // Try to open and lock a writable temporary filename.
  int index = 0;
  while (true) {
    ++index;
    ostringstream strm;
    strm << prefix << "_" << index << ".dat";

    string basename = strm.str();
    _filename = Filename(dir, basename);
    string os_specific = _filename.to_os_specific();

    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Creating vertex data save file " << os_specific << "\n"; 
    }

#ifdef _WIN32
    // Windows case.
    _handle = CreateFile(os_specific.c_str(), GENERIC_READ | GENERIC_WRITE,
                         0, NULL, CREATE_ALWAYS, 
                         FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE | FILE_FLAG_RANDOM_ACCESS,
                         NULL);
    if (_handle != INVALID_HANDLE_VALUE) {
      // The file was successfully opened and locked.
      break;

    } else {
      // Couldn't open the file.  Either the directory was bad, or the
      // file was already locked by another.
      DWORD err = GetLastError();

      if (err != ERROR_SHARING_VIOLATION) {
        // File couldn't be opened; permission problem or bogus directory.
        if (!dir.empty()) {
          // Try the current directory, once.
          dir = Filename();
        } else {
          gobj_cat.error()
            << "Couldn't open vertex data save file.\n";
          return;
        }
      }

      // Couldn't lock the file.  Try the next one.
    }

#else
    // Posix case.
    _fd = open(os_specific.c_str(), O_RDWR | O_CREAT, 0666);
    if (_fd == -1) {
      // Couldn't open the file: permissions problem or bad directory.
      if (!_filename.exists()) {
        // It must be a bad directory.
        if (!dir.empty()) {
          // Try the current directory, once.
          dir = Filename();
        } else {
          gobj_cat.error()
            << "Couldn't open vertex data save file.\n";
          return;
        }
      }

      // If it's a permissions problem, it might be a user-level
      // permissions issue.  Continue to the next.
      continue;
    }
    
    // Now try to lock the file, so we can be sure that no other
    // process is simultaneously writing to the same save file.
    int result = lockf(_fd, F_TLOCK, 0);
    if (result == 0) {
      // We've got the file.  Truncate it first, for good measure, in
      // case there's an old version of the file we picked up.
      ftruncate(_fd, 0);

      // On Unix, it's safe to unlink (delete) the temporary file
      // after it's been opened.  The file remains open, but
      // disappears from the directory.  This is kind of like
      // DELETE_ON_CLOSE, to ensure the temporary file won't
      // accidentally get left behind, except it's a little more
      // proactive.
      unlink(os_specific.c_str());
      _filename = Filename();
      break;
    }

    // Try the next file.
    close(_fd);
#endif  // _WIN32
  }

  _is_valid = true;
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

  // No need to remove the file, since in both above cases we have
  // already removed it.  And removing it now, after we have closed
  // and unlocked it, might accidentally remove someone else's copy.
  /*
  if (!_filename.empty()) {
    _filename.unlink();
  }
  */
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
  if (!_is_valid) {
    return NULL;
  }

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
  if (!_is_valid) {
    return false;
  }

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
