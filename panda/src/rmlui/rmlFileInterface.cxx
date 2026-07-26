/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlFileInterface.cxx
 * @author rdb
 * @date 2011-11-03
 */

#include "rmlFileInterface.h"
#include "virtualFileSystem.h"

/**
 * If vfs is nullptr, uses the global VirtualFileSystem.
 */
RmlFileInterface::
RmlFileInterface(VirtualFileSystem *vfs) : _vfs(vfs) {
  if (_vfs == nullptr) {
    _vfs = VirtualFileSystem::get_global_ptr();
  }
}

/**
 * Opens path for reading via the VirtualFileSystem.  Falls back to the
 * model-path if the path is not found as an absolute filename.  Returns 0
 * on failure.
 */
Rml::FileHandle RmlFileInterface::
Open(const Rml::String &path) {
  if (rmlui_cat.is_debug()) {
    rmlui_cat.debug() << "Opening " << path << "\n";
  }

  Filename fn = Filename::from_os_specific(path);

  PT(VirtualFile) file = _vfs->get_file(fn);
  if (file == nullptr) {
    if (!_vfs->resolve_filename(fn, get_model_path())) {
      rmlui_cat.error() << "Could not resolve " << fn
        << " along the model-path\n";
      return 0;
    }
    file = _vfs->get_file(fn);
    if (file == nullptr) {
      rmlui_cat.error() << "Failed to get " << fn << "\n";
      return 0;
    }
  }

  std::istream *str = file->open_read_file(true);
  if (str == nullptr) {
    rmlui_cat.error() << "Failed to open " << fn << " for reading\n";
    return 0;
  }

  VirtualFileHandle *handle = new VirtualFileHandle;
  handle->_file = file;
  handle->_stream = str;
  return (Rml::FileHandle) handle;
}

/**
 * Closes an open file handle.
 */
void RmlFileInterface::
Close(Rml::FileHandle file) {
  VirtualFileHandle *handle = (VirtualFileHandle *) file;
  if (handle == nullptr) {
    return;
  }
  _vfs->close_read_file(handle->_stream);
  delete handle;
}

/**
 * Reads up to size bytes from the file into buffer.  Returns the number of
 * bytes actually read.
 */
size_t RmlFileInterface::
Read(void *buffer, size_t size, Rml::FileHandle file) {
  VirtualFileHandle *handle = (VirtualFileHandle *) file;
  if (handle == nullptr) {
    return 0;
  }
  handle->_stream->read((char *) buffer, size);
  return handle->_stream->gcount();
}

/**
 * Seeks within the file.  origin is one of SEEK_SET, SEEK_CUR, SEEK_END.
 * Returns true on success.
 */
bool RmlFileInterface::
Seek(Rml::FileHandle file, long offset, int origin) {
  VirtualFileHandle *handle = (VirtualFileHandle *) file;
  if (handle == nullptr) {
    return false;
  }
  // A prior Read() to end-of-file leaves eofbit (and failbit) set; seekg()
  // clears eofbit but not failbit, so clear the stream first or the seek is a
  // no-op that reports failure.  (RmlUi commonly reads to EOF then seeks back.)
  handle->_stream->clear();
  switch (origin) {
  case SEEK_SET:
    handle->_stream->seekg(offset, std::ios::beg);
    break;
  case SEEK_CUR:
    handle->_stream->seekg(offset, std::ios::cur);
    break;
  case SEEK_END:
    handle->_stream->seekg(offset, std::ios::end);
    break;
  }
  return !handle->_stream->fail();
}

/**
 * Returns the current read position within the file.
 */
size_t RmlFileInterface::
Tell(Rml::FileHandle file) {
  VirtualFileHandle *handle = (VirtualFileHandle *) file;
  if (handle == nullptr) {
    return 0;
  }
  // tellg() returns -1 on failure; report that as 0 rather than letting it wrap
  // to SIZE_MAX.
  std::streampos pos = handle->_stream->tellg();
  return pos < 0 ? 0 : (size_t)pos;
}

/**
 * Returns the total size of the file in bytes.
 */
size_t RmlFileInterface::
Length(Rml::FileHandle file) {
  VirtualFileHandle *handle = (VirtualFileHandle *) file;
  if (handle == nullptr) {
    return 0;
  }
  std::streamsize size = handle->_file->get_file_size(handle->_stream);
  return size < 0 ? 0 : (size_t)size;
}
