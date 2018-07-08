/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileMount.cxx
 * @author drose
 * @date 2002-08-03
 */

#include "virtualFileMount.h"
#include "virtualFileSimple.h"
#include "virtualFileSystem.h"
#include "zStream.h"

using std::iostream;
using std::istream;
using std::ostream;
using std::string;

TypeHandle VirtualFileMount::_type_handle;


/**
 *
 */
VirtualFileMount::
~VirtualFileMount() {
  nassertv(_file_system == nullptr);
}

/**
 * Constructs and returns a new VirtualFile instance that corresponds to the
 * indicated filename within this mount point.  The returned VirtualFile
 * object does not imply that the given file actually exists; but if the file
 * does exist, then the handle can be used to read it.
 */
PT(VirtualFile) VirtualFileMount::
make_virtual_file(const Filename &local_filename,
                  const Filename &original_filename, bool implicit_pz_file,
                  int open_flags) {
  Filename local(local_filename);
  if (original_filename.is_text()) {
    local.set_text();
  } else {
    local.set_binary();
  }
  PT(VirtualFileSimple) file =
    new VirtualFileSimple(this, local, implicit_pz_file, open_flags);
  file->set_original_filename(original_filename);

  if ((open_flags & VirtualFileSystem::OF_create_file) != 0) {
    create_file(local);
  } else if ((open_flags & VirtualFileSystem::OF_make_directory) != 0) {
    make_directory(local);
  }

  return file;
}

/**
 * Attempts to create the indicated file within the mount, if it does not
 * already exist.  Returns true on success (or if the file already exists), or
 * false if it cannot be created.
 */
bool VirtualFileMount::
create_file(const Filename &file) {
  return false;
}

/**
 * Attempts to delete the indicated file or directory within the mount.  This
 * can remove a single file or an empty directory.  It will not remove a
 * nonempty directory.  Returns true on success, false on failure.
 */
bool VirtualFileMount::
delete_file(const Filename &file) {
  return false;
}

/**
 * Attempts to rename the contents of the indicated file to the indicated
 * file.  Both filenames will be within the mount.  Returns true on success,
 * false on failure.  If this returns false, this will be attempted again with
 * a copy-and-delete operation.
 */
bool VirtualFileMount::
rename_file(const Filename &orig_filename, const Filename &new_filename) {
  return false;
}

/**
 * Attempts to copy the contents of the indicated file to the indicated file.
 * Both filenames will be within the mount.  Returns true on success, false on
 * failure.  If this returns false, the copy will be performed by explicit
 * read-and-write operations.
 */
bool VirtualFileMount::
copy_file(const Filename &orig_filename, const Filename &new_filename) {
  return false;
}

/**
 * Attempts to create the indicated file within the mount, if it does not
 * already exist.  Returns true on success, or false if it cannot be created.
 * If the directory already existed prior to this call, may return either true
 * or false.
 */
bool VirtualFileMount::
make_directory(const Filename &file) {
  return false;
}

/**
 * Returns true if the named file or directory may be written to, false
 * otherwise.
 */
bool VirtualFileMount::
is_writable(const Filename &file) const {
  return false;
}

/**
 * Fills up the indicated pvector with the contents of the file, if it is a
 * regular file.  Returns true on success, false otherwise.
 */
bool VirtualFileMount::
read_file(const Filename &file, bool do_uncompress,
          vector_uchar &result) const {
  result.clear();

  istream *in = open_read_file(file, do_uncompress);
  if (in == nullptr) {
    express_cat.info()
      << "Unable to read " << file << "\n";
    return false;
  }

  std::streamsize file_size = get_file_size(file, in);
  if (file_size > 0) {
    result.reserve((size_t)file_size);
  }

  bool okflag = VirtualFile::simple_read_file(in, result);

  close_read_file(in);

  if (!okflag) {
    express_cat.info()
      << "Error while reading " << file << "\n";
  }
  return okflag;
}

/**
 * Writes the indicated data to the file, if it is a writable file.  Returns
 * true on success, false otherwise.
 */
bool VirtualFileMount::
write_file(const Filename &file, bool do_compress,
           const unsigned char *data, size_t data_size) {
  ostream *out = open_write_file(file, do_compress, true);
  if (out == nullptr) {
    express_cat.info()
      << "Unable to write " << file << "\n";
    return false;
  }

  out->write((const char *)data, data_size);
  bool okflag = (!out->fail());
  close_write_file(out);

  if (!okflag) {
    express_cat.info()
      << "Error while writing " << file << "\n";
  }
  return okflag;
}

/**
 * Opens the file for reading.  Returns a newly allocated istream on success
 * (which you should eventually delete when you are done reading). Returns
 * NULL on failure.
 *
 * If do_uncompress is true, the file is also decompressed on-the-fly using
 * zlib.
 */
istream *VirtualFileMount::
open_read_file(const Filename &file, bool do_uncompress) const {
  istream *result = open_read_file(file);

#ifdef HAVE_ZLIB
  if (result != nullptr && do_uncompress) {
    // We have to slip in a layer to decompress the file on the fly.
    IDecompressStream *wrapper = new IDecompressStream(result, true);
    result = wrapper;
  }
#endif  // HAVE_ZLIB

  return result;
}

/**
 * Closes a file opened by a previous call to open_read_file().  This really
 * just deletes the istream pointer, but it is recommended to use this
 * interface instead of deleting it explicitly, to help work around compiler
 * issues.
 */
void VirtualFileMount::
close_read_file(istream *stream) const {
  VirtualFileSystem::close_read_file(stream);
}

/**
 * Opens the file for writing.  Returns a newly allocated ostream on success
 * (which you should eventually delete when you are done writing). Returns
 * NULL on failure.
 */
ostream *VirtualFileMount::
open_write_file(const Filename &file, bool truncate) {
  return nullptr;
}

/**
 * Opens the file for writing.  Returns a newly allocated ostream on success
 * (which you should eventually delete when you are done writing). Returns
 * NULL on failure.
 *
 * If do_compress is true, the file is also compressed on-the-fly using zlib.
 */
ostream *VirtualFileMount::
open_write_file(const Filename &file, bool do_compress, bool truncate) {
  ostream *result = open_write_file(file, truncate);

#ifdef HAVE_ZLIB
  if (result != nullptr && do_compress) {
    // We have to slip in a layer to compress the file on the fly.
    OCompressStream *wrapper = new OCompressStream(result, true);
    result = wrapper;
  }
#endif  // HAVE_ZLIB

  return result;
}

/**
 * Works like open_write_file(), but the file is opened in append mode.  Like
 * open_write_file, the returned pointer should eventually be passed to
 * close_write_file().
 */
ostream *VirtualFileMount::
open_append_file(const Filename &file) {
  return nullptr;
}

/**
 * Closes a file opened by a previous call to open_write_file().  This really
 * just deletes the ostream pointer, but it is recommended to use this
 * interface instead of deleting it explicitly, to help work around compiler
 * issues.
 */
void VirtualFileMount::
close_write_file(ostream *stream) {
  VirtualFileSystem::close_write_file(stream);
}

/**
 * Opens the file for writing.  Returns a newly allocated iostream on success
 * (which you should eventually delete when you are done writing). Returns
 * NULL on failure.
 */
iostream *VirtualFileMount::
open_read_write_file(const Filename &file, bool truncate) {
  return nullptr;
}

/**
 * Works like open_read_write_file(), but the file is opened in append mode.
 * Like open_read_write_file, the returned pointer should eventually be passed
 * to close_read_write_file().
 */
iostream *VirtualFileMount::
open_read_append_file(const Filename &file) {
  return nullptr;
}

/**
 * Closes a file opened by a previous call to open_read_write_file().  This
 * really just deletes the iostream pointer, but it is recommended to use this
 * interface instead of deleting it explicitly, to help work around compiler
 * issues.
 */
void VirtualFileMount::
close_read_write_file(iostream *stream) {
  VirtualFileSystem::close_read_write_file(stream);
}

/**
 * Populates the SubfileInfo structure with the data representing where the
 * file actually resides on disk, if this is knowable.  Returns true if the
 * file might reside on disk, and the info is populated, or false if it does
 * not (or it is not known where the file resides), in which case the info is
 * meaningless.
 */
bool VirtualFileMount::
get_system_info(const Filename &file, SubfileInfo &info) {
  return false;
}

/**
 * See Filename::atomic_compare_and_exchange_contents().
 */
bool VirtualFileMount::
atomic_compare_and_exchange_contents(const Filename &file, string &orig_contents,
                                     const string &old_contents,
                                     const string &new_contents) {
  return false;
}

/**
 * See Filename::atomic_read_contents().
 */
bool VirtualFileMount::
atomic_read_contents(const Filename &file, string &contents) const {
  return false;
}

/**
 *
 */
void VirtualFileMount::
output(ostream &out) const {
  out << get_type();
}

/**
 *
 */
void VirtualFileMount::
write(ostream &out) const {
  out << *this << " on /" << get_mount_point() << "\n";
}
