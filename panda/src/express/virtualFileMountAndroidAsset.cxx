// Filename: virtualFileMountAndroidAsset.cxx
// Created by:  rdb (21Jan13)
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

#ifdef ANDROID

#include "virtualFileMountAndroidAsset.h"
#include "virtualFileSystem.h"

#ifndef NDEBUG
#include <sys/stat.h>
#endif

TypeHandle VirtualFileMountAndroidAsset::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
VirtualFileMountAndroidAsset::
~VirtualFileMountAndroidAsset() {
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::get_fd
//       Access: Public
//  Description: Returns a file descriptor that can be used to read
//               the asset if it was stored uncompressed and
//               unencrypted.  Returns a valid fd or -1.
////////////////////////////////////////////////////////////////////
int VirtualFileMountAndroidAsset::
get_fd(const Filename &file, off_t *start, off_t *length) const {
  AAsset* asset;
  asset = AAssetManager_open(_asset_mgr, file.c_str(), AASSET_MODE_UNKNOWN);

  int fd = AAsset_openFileDescriptor(asset, start, length);
  AAsset_close(asset);
  return fd;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::has_file
//       Access: Public, Virtual
//  Description: Returns true if the indicated file exists within the
//               mount system.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountAndroidAsset::
has_file(const Filename &file) const {
  return (file.empty() || /*is_directory(file) ||*/ is_regular_file(file));
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::is_directory
//       Access: Public, Virtual
//  Description: Returns true if the indicated file exists within the
//               mount system and is a directory.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountAndroidAsset::
is_directory(const Filename &file) const {
  // This is the only way - AAssetManager_openDir also works for files.
  //AAssetDir *dir = AAssetManager_openDir(_asset_mgr, file.c_str());

  //express_cat.error() << "is_directory " << file << " - " << dir << "\n";

  //if (dir == NULL) {
  //  return false;
  //}
  //AAssetDir_close(dir);

  // openDir doesn't return NULL for ordinary files!
  return !is_regular_file(file);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::is_regular_file
//       Access: Public, Virtual
//  Description: Returns true if the indicated file exists within the
//               mount system and is a regular file.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountAndroidAsset::
is_regular_file(const Filename &file) const {
  // I'm afraid the only way to see if it exists is to try and open it.
  AAsset* asset;
  asset = AAssetManager_open(_asset_mgr, file.c_str(), AASSET_MODE_UNKNOWN);

  express_cat.error() << "is_regular_file " << file << " - " << asset << "\n";

  if (asset == NULL) {
    return false;
  }
  AAsset_close(asset);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::read_file
//       Access: Public, Virtual
//  Description: Fills up the indicated pvector with the contents of
//               the file, if it is a regular file.  Returns true on
//               success, false otherwise.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountAndroidAsset::
read_file(const Filename &file, bool do_uncompress,
          pvector<unsigned char> &result) const {
  if (do_uncompress) {
    // If the file is to be decompressed, we'd better just use the
    // higher-level implementation, which includes support for
    // on-the-fly decompression.
    return VirtualFileMount::read_file(file, do_uncompress, result);
  }

  // But if we're just reading a straight file, let's just read
  // it here to avoid all of the streambuf nonsense.
  result.clear();

  AAsset* asset;
  asset = AAssetManager_open(_asset_mgr, file.c_str(), AASSET_MODE_STREAMING);
  if (asset == (AAsset *)NULL) {
    express_cat.info()
      << "Unable to read " << file << "\n";
    return false;
  }

  // Reserve enough space to hold the entire file.
  off_t file_size = AAsset_getLength(asset);
  if (file_size == 0) {
    return true;
  } else if (file_size > 0) {
    result.reserve((size_t)file_size);
  }

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  int count = AAsset_read(asset, buffer, buffer_size);
  while (count > 0) {
    thread_consider_yield();
    result.insert(result.end(), buffer, buffer + count);
    count = AAsset_read(asset, buffer, buffer_size);
  }

  AAsset_close(asset);
  return (count == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::open_read_file
//       Access: Public, Virtual
//  Description: Opens the file for reading, if it exists.  Returns a
//               newly allocated istream on success (which you should
//               eventually delete when you are done reading).
//               Returns NULL on failure.
////////////////////////////////////////////////////////////////////
istream *VirtualFileMountAndroidAsset::
open_read_file(const Filename &file) const {
  AAsset* asset;
  asset = AAssetManager_open(_asset_mgr, file.c_str(), AASSET_MODE_UNKNOWN);
  if (asset == (AAsset *)NULL) {
    return NULL;
  }

  AssetStream *stream = new AssetStream(asset);
  return (istream *) stream;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::get_file_size
//       Access: Published, Virtual
//  Description: Returns the current size on disk (or wherever it is)
//               of the already-open file.  Pass in the stream that
//               was returned by open_read_file(); some
//               implementations may require this stream to determine
//               the size.
////////////////////////////////////////////////////////////////////
streamsize VirtualFileMountAndroidAsset::
get_file_size(const Filename &file, istream *in) const {
  // If it's already open, get the AAsset pointer from the streambuf.
  const AssetStreamBuf *buf = (const AssetStreamBuf *) in->rdbuf();
  off_t length = AAsset_getLength(buf->_asset);
  return length;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::get_file_size
//       Access: Published, Virtual
//  Description: Returns the current size on disk (or wherever it is)
//               of the file before it has been opened.
////////////////////////////////////////////////////////////////////
streamsize VirtualFileMountAndroidAsset::
get_file_size(const Filename &file) const {
  AAsset* asset = AAssetManager_open(_asset_mgr, file.c_str(), AASSET_MODE_UNKNOWN);
  off_t length = AAsset_getLength(asset);
  AAsset_close(asset);
  return length;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::get_timestamp
//       Access: Published, Virtual
//  Description: Returns a time_t value that represents the time the
//               file was last modified, to within whatever precision
//               the operating system records this information (on a
//               Windows95 system, for instance, this may only be
//               accurate to within 2 seconds).
//
//               If the timestamp cannot be determined, either because
//               it is not supported by the operating system or
//               because there is some error (such as file not found),
//               returns 0.
////////////////////////////////////////////////////////////////////
time_t VirtualFileMountAndroidAsset::
get_timestamp(const Filename &file) const {
  // There's no obvious way to get a timestamp from an Android asset.
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::get_system_info
//       Access: Public, Virtual
//  Description: Populates the SubfileInfo structure with the data
//               representing where the file actually resides on disk,
//               if this is knowable.  Returns true if the file might
//               reside on disk, and the info is populated, or false
//               if it might not (or it is not known where the file
//               resides), in which case the info is meaningless.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountAndroidAsset::
get_system_info(const Filename &file, SubfileInfo &info) {
  off_t start, length;
  int fd = get_fd(file, &start, &length);

#ifndef NDEBUG
  // Double-check that this fd actually points to the apk.
  struct stat st1, st2;
  nassertr(fstat(fd, &st1) == 0, false);
  nassertr(stat(_apk_path.c_str(), &st2) == 0, false);
  nassertr(st1.st_dev == st2.st_dev, false);
  nassertr(st1.st_ino == st2.st_ino, false);
#endif

  // We don't actually need the file descriptor, so close it.
  close(fd);

  info = SubfileInfo(_apk_path, start, length);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::scan_directory
//       Access: Public, Virtual
//  Description: Fills the given vector up with the list of filenames
//               that are local to this directory, if the filename is
//               a directory.  Returns true if successful, or false if
//               the file is not a directory or cannot be read.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountAndroidAsset::
scan_directory(vector_string &contents, const Filename &dir) const {
  AAssetDir *asset_dir = AAssetManager_openDir(_asset_mgr, dir.c_str());
  if (asset_dir == NULL) {
    return false;
  }

  // Note: this returns the full path.
  const char *fullpath = AAssetDir_getNextFileName(asset_dir);

  while (fullpath != NULL) {
    express_cat.error() << fullpath << "\n"; // DEBUG
    // Determine the basename and add it to the vector.
    Filename fname (fullpath);
    contents.push_back(fname.get_basename());
    fullpath = AAssetDir_getNextFileName(asset_dir);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::AssetStream::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
VirtualFileMountAndroidAsset::AssetStream::
~AssetStream() {
  delete rdbuf();
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::AssetStreamBuf::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
VirtualFileMountAndroidAsset::AssetStreamBuf::
AssetStreamBuf(AAsset *asset) :
  _asset(asset) {

#ifdef PHAVE_IOSTREAM
  char *buf = new char[4096];
  char *ebuf = buf + 4096;
  setg(buf, ebuf, ebuf);

#else
  allocate();
  setg(base(), ebuf(), ebuf());
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::AssetStreamBuf::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
VirtualFileMountAndroidAsset::AssetStreamBuf::
~AssetStreamBuf() {
  AAsset_close(_asset);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::AssetStreamBuf::seekoff
//       Access: Public, Virtual
//  Description: Implements seeking within the stream.
////////////////////////////////////////////////////////////////////
streampos VirtualFileMountAndroidAsset::AssetStreamBuf::
seekoff(streamoff off, ios_seekdir dir, ios_openmode which) {
  size_t n = egptr() - gptr();

  int whence;
  switch (dir) {
  case ios_base::beg:
    whence = SEEK_SET;
    break;
  case ios_base::cur:
    if (off == 0) {
      // Just requesting the current position,
      // no need to void the buffer.
      return AAsset_seek(_asset, 0, SEEK_CUR) - n;

    } else if (gptr() + off >= eback() && gptr() + off < egptr()) {
      // We can seek around within the buffer.
      gbump(off);
      return AAsset_seek(_asset, 0, SEEK_CUR) - n + off;
    }
    whence = SEEK_CUR;
    break;
  case ios_base::end:
    whence = SEEK_END;
    break;
  default:
    return pos_type(-1);
  }
  gbump(n);
  return AAsset_seek(_asset, off, whence);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::AssetStreamBuf::seekpos
//       Access: Public, Virtual
//  Description: A variant on seekoff() to implement seeking within a
//               stream.
//
//               The MSDN Library claims that it is only necessary to
//               redefine seekoff(), and not seekpos() as well, as the
//               default implementation of seekpos() is supposed to
//               map to seekoff() exactly as I am doing here; but in
//               fact it must do something else, because seeking
//               didn't work on Windows until I redefined this
//               function as well.
////////////////////////////////////////////////////////////////////
streampos VirtualFileMountAndroidAsset::AssetStreamBuf::
seekpos(streampos pos, ios_openmode which) {
  size_t n = egptr() - gptr();
  gbump(n);
  return AAsset_seek(_asset, pos, SEEK_SET);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountAndroidAsset::AssetStreamBuf::underflow
//       Access: Protected, Virtual
//  Description: Called by the system istream implementation when its
//               internal buffer needs more characters.
////////////////////////////////////////////////////////////////////
int VirtualFileMountAndroidAsset::AssetStreamBuf::
underflow() {
  // Sometimes underflow() is called even if the buffer is not empty.
  if (gptr() >= egptr()) {
    // Mark the buffer filled (with buffer_size bytes).
    size_t buffer_size = egptr() - eback();
    gbump(-(int)buffer_size);

    streamsize read_count;
    read_count = AAsset_read(_asset, gptr(), buffer_size);

    if (read_count != buffer_size) {
      // Oops, we didn't read what we thought we would.
      if (read_count == 0) {
        gbump(buffer_size);
        return EOF;
      }

      // Slide what we did read to the top of the buffer.
      nassertr(read_count < buffer_size, EOF);
      size_t delta = buffer_size - read_count;
      memmove(gptr() + delta, gptr(), read_count);
      gbump(delta);
    }
  }

  return (unsigned char)*gptr();
}

#endif  // ANDROID
