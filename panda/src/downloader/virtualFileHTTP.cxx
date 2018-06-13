/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileHTTP.cxx
 * @author drose
 * @date 2008-10-31
 */

#include "virtualFileHTTP.h"
#include "virtualFileMountHTTP.h"
#include "stringStream.h"
#include "zStream.h"

#ifdef HAVE_OPENSSL

using std::istream;
using std::ostream;
using std::string;

TypeHandle VirtualFileHTTP::_type_handle;


/**
 *
 */
VirtualFileHTTP::
VirtualFileHTTP(VirtualFileMountHTTP *mount, const Filename &local_filename,
                bool implicit_pz_file, int open_flags) :
  _mount(mount),
  _local_filename(local_filename),
  _implicit_pz_file(implicit_pz_file),
  _status_only(open_flags != 0)
{
  URLSpec url(_mount->get_root());
  url.set_path(_mount->get_root().get_path() + _local_filename.c_str());
  _channel = _mount->get_channel();
  if (_status_only) {
    _channel->get_header(url);
  } else {
    _channel->get_document(url);
  }
}

/**
 *
 */
VirtualFileHTTP::
~VirtualFileHTTP() {
  // Recycle the associated HTTPChannel, so we can use it again later without
  // having to close the connection to the server.
  _mount->recycle_channel(_channel);
}

/**
 * Returns the VirtualFileSystem this file is associated with.
 */
VirtualFileSystem *VirtualFileHTTP::
get_file_system() const {
  return _mount->get_file_system();
}

/**
 * Returns the full pathname to this file within the virtual file system.
 */
Filename VirtualFileHTTP::
get_filename() const {
  string mount_point = _mount->get_mount_point();
  if (_local_filename.empty()) {
    if (mount_point.empty()) {
      return "/";
    } else {
      return string("/") + mount_point;
    }

  } else {
    if (mount_point.empty()) {
      return string("/") + _local_filename.get_fullpath();
    } else {
      return string("/") + mount_point + string("/") + _local_filename.get_fullpath();
    }
  }
}

/**
 * Returns true if this file exists, false otherwise.
 */
bool VirtualFileHTTP::
has_file() const {
  return _channel->is_valid();
}

/**
 * Returns true if this file represents a directory (and scan_directory() may
 * be called), false otherwise.
 */
bool VirtualFileHTTP::
is_directory() const {
  return false;
}

/**
 * Returns true if this file represents a regular file (and read_file() may be
 * called), false otherwise.
 */
bool VirtualFileHTTP::
is_regular_file() const {
  return _channel->is_valid();
}

/**
 * Opens the file for reading.  Returns a newly allocated istream on success
 * (which you should eventually delete when you are done reading). Returns
 * NULL on failure.
 *
 * If auto_unwrap is true, an explicitly-named .pz file is automatically
 * decompressed and the decompressed contents are returned.  This is different
 * than vfs-implicit-pz, which will automatically decompress a file if the
 * extension .pz is *not* given.
 */
istream *VirtualFileHTTP::
open_read_file(bool auto_unwrap) const {
  if (_status_only) {
    return nullptr;
  }

  // We pre-download the file into a StringStream, then return a buffer to
  // that.  It seems safer, since we can guarantee the file comes all at once
  // without timeouts along the way.
  StringStream *strstream = new StringStream;
  if (!fetch_file(strstream)) {
    delete strstream;
    return nullptr;
  }

  return return_file(strstream, auto_unwrap);
}

/**
 * Downloads the entire file from the web server into the indicated iostream.
 * Returns true on success, false on failure.
 *
 * This seems to be safer than returning the socket stream directly, since
 * this way we can better control timeouts and other internet hiccups.  We can
 * also offer seeking on the resulting stream.
 */
bool VirtualFileHTTP::
fetch_file(ostream *buffer_stream) const {
  _channel->download_to_stream(buffer_stream, false);
  if (!_channel->is_download_complete()) {
    // Failure to download fully.  Try again to download more.

    URLSpec url(_mount->get_root());
    url.set_path(_mount->get_root().get_path() + _local_filename.c_str());

    size_t bytes_downloaded = _channel->get_bytes_downloaded();
    size_t last_byte = bytes_downloaded;

    while (bytes_downloaded > 0 && !_channel->is_download_complete()) {
      _channel->get_subdocument(url, last_byte, 0);
      _channel->download_to_stream(buffer_stream, true);
      bytes_downloaded = _channel->get_bytes_downloaded();
      last_byte = _channel->get_last_byte_delivered();
    }
  }

  return _channel->is_download_complete() && _channel->is_valid();
}

/**
 * After downloading the entire file via fetch_file(), rewinds the file stream
 * and returns it as its own readable stream.
 */
istream *VirtualFileHTTP::
return_file(istream *buffer_stream, bool auto_unwrap) const {
  // Will we be automatically unwrapping a .pz file?
  bool do_unwrap = (_implicit_pz_file || (auto_unwrap && _local_filename.get_extension() == "pz"));

  istream *result = buffer_stream;
#ifdef HAVE_ZLIB
  if (result != nullptr && do_unwrap) {
    // We have to slip in a layer to decompress the file on the fly.
    IDecompressStream *wrapper = new IDecompressStream(result, true);
    result = wrapper;
  }
#endif  // HAVE_ZLIB

  return result;
}

/**
 * Call this method after a reading the istream returned by open_read_file()
 * to completion.  If it returns true, the file was read completely and
 * without error; if it returns false, there may have been some errors or a
 * truncated file read.  This is particularly likely if the stream is a
 * VirtualFileHTTP.
 */
bool VirtualFileHTTP::
was_read_successful() const {
  return _channel->is_valid() && _channel->is_download_complete();
}

/**
 * Returns the current size on disk (or wherever it is) of the already-open
 * file.  Pass in the stream that was returned by open_read_file(); some
 * implementations may require this stream to determine the size.
 */
std::streamsize VirtualFileHTTP::
get_file_size(istream *stream) const {
  return _channel->get_file_size();
}

/**
 * Returns the current size on disk (or wherever it is) of the file before it
 * has been opened.
 */
std::streamsize VirtualFileHTTP::
get_file_size() const {
  return _channel->get_file_size();
}

/**
 * Returns a time_t value that represents the time the file was last modified,
 * to within whatever precision the operating system records this information
 * (on a Windows95 system, for instance, this may only be accurate to within 2
 * seconds).
 *
 * If the timestamp cannot be determined, either because it is not supported
 * by the operating system or because there is some error (such as file not
 * found), returns 0.
 */
time_t VirtualFileHTTP::
get_timestamp() const {
  const DocumentSpec &spec = _channel->get_document_spec();
  if (spec.has_date()) {
    return spec.get_date().get_time();
  }
  return 0;
}

#endif // HAVE_OPENSSL
