// Filename: virtualFile.cxx
// Created by:  drose (03Aug02)
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

#include "virtualFile.h"
#include "virtualFileSystem.h"
#include "virtualFileList.h"
#include "config_express.h"
#include "pvector.h"
#include <iterator>

TypeHandle VirtualFile::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::has_file
//       Access: Published, Virtual
//  Description: Returns true if this file exists, false otherwise.
////////////////////////////////////////////////////////////////////
bool VirtualFile::
has_file() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::is_directory
//       Access: Published, Virtual
//  Description: Returns true if this file represents a directory (and
//               scan_directory() may be called), false otherwise.
////////////////////////////////////////////////////////////////////
bool VirtualFile::
is_directory() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::is_regular_file
//       Access: Published, Virtual
//  Description: Returns true if this file represents a regular file
//               (and read_file() may be called), false otherwise.
////////////////////////////////////////////////////////////////////
bool VirtualFile::
is_regular_file() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::is_writable
//       Access: Published, Virtual
//  Description: Returns true if this file may be written to, which
//               implies write_file() may be called (unless it is a
//               directory instead of a regular file).
////////////////////////////////////////////////////////////////////
bool VirtualFile::
is_writable() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::delete_file
//       Access: Public
//  Description: Attempts to delete this file or directory.  This can
//               remove a single file or an empty directory.  It will
//               not remove a nonempty directory.  Returns true on
//               success, false on failure.
////////////////////////////////////////////////////////////////////
bool VirtualFile::
delete_file() {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::rename_file
//       Access: Public
//  Description: Attempts to move or rename this file or directory.
//               If the original file is an ordinary file, it will
//               quietly replace any already-existing file in the new
//               filename (but not a directory).  If the original file
//               is a directory, the new filename must not already
//               exist.
//
//               If the file is a directory, the new filename must be
//               within the same mount point.  If the file is an
//               ordinary file, the new filename may be anywhere; but
//               if it is not within the same mount point then the
//               rename operation is automatically performed as a
//               two-step copy-and-delete operation.
////////////////////////////////////////////////////////////////////
bool VirtualFile::
rename_file(VirtualFile *new_file) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::copy_file
//       Access: Public
//  Description: Attempts to copy the contents of this file to the
//               indicated file.  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool VirtualFile::
copy_file(VirtualFile *new_file) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::scan_directory
//       Access: Published
//  Description: If the file represents a directory (that is,
//               is_directory() returns true), this returns the list
//               of files within the directory at the current time.
//               Returns NULL if the file is not a directory or if the
//               directory cannot be read.
////////////////////////////////////////////////////////////////////
PT(VirtualFileList) VirtualFile::
scan_directory() const {
  // First, we have to make sure there aren't any mount points attached
  // under this directory.  These will override any local filenames.
  VirtualFileSystem *file_system = get_file_system();
  Filename this_filename = get_filename();
  vector_string mount_points_flat;
  file_system->scan_mount_points(mount_points_flat, this_filename);

  // Copy the set of nested mount points to a sorted list so we can
  // search it quickly.
  ov_set<string> mount_points;
  copy(mount_points_flat.begin(), mount_points_flat.end(),
       back_inserter(mount_points));
  mount_points.sort();

  
  PT(VirtualFileList) file_list = new VirtualFileList;

  // Each of those mount points maps to a directory root or something
  // from the file system.
  ov_set<string>::const_iterator mi;
  for (mi = mount_points.begin(); mi != mount_points.end(); ++mi) {
    const string &basename = (*mi);
    Filename filename(this_filename, basename);
    PT(VirtualFile) file = file_system->get_file(filename);
    file_list->add_file(file);
  }

  // Now, get the actual local files in this directory.
  vector_string names;
  if (!scan_local_directory(file_list, mount_points)) {
    // Not a directory, or unable to read directory.
    if (file_list->get_num_files() == 0) {
      return NULL;
    }

    // We couldn't read the physical directory, but we do have some
    // mounted files to return.
    return file_list;
  }

  return file_list;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void VirtualFile::
output(ostream &out) const {
  out << get_filename();
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::ls
//       Access: Published
//  Description: If the file represents a directory, lists its
//               contents.
////////////////////////////////////////////////////////////////////
void VirtualFile::
ls(ostream &out) const {
  CPT(VirtualFileList) contents = scan_directory();
  if (contents == (VirtualFileList *)NULL) {
    if (!is_directory()) {
      out << get_filename() << "\n";
    } else {
      out << get_filename() << " cannot be read.\n";
    }
    return;
  }

  int num_files = contents->get_num_files();
  for (int i = 0; i < num_files; i++) {
    VirtualFile *file = contents->get_file(i);
    out << file->get_filename().get_basename() << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::ls_all
//       Access: Published
//  Description: If the file represents a directory, recursively lists
//               its contents and those of all subdirectories.
////////////////////////////////////////////////////////////////////
void VirtualFile::
ls_all(ostream &out) const {
  if (!is_directory()) {
    out << get_filename() << " is not a directory.\n";
  } else {
    r_ls_all(out, get_filename());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::open_read_file
//       Access: Published, Virtual
//  Description: Opens the file for reading.  Returns a newly
//               allocated istream on success (which you should
//               eventually delete when you are done reading).
//               Returns NULL on failure.
////////////////////////////////////////////////////////////////////
istream *VirtualFile::
open_read_file(bool auto_unwrap) const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::close_read_file
//       Access: Published
//  Description: Closes a file opened by a previous call to
//               open_read_file().  This really just deletes the
//               istream pointer, but it is recommended to use this
//               interface instead of deleting it explicitly, to help
//               work around compiler issues.
////////////////////////////////////////////////////////////////////
void VirtualFile::
close_read_file(istream *stream) const {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::was_read_successful
//       Access: Published, Virtual
//  Description: Call this method after a reading the istream returned
//               by open_read_file() to completion.  If it returns
//               true, the file was read completely and without error;
//               if it returns false, there may have been some errors
//               or a truncated file read.  This is particularly
//               likely if the stream is a VirtualFileHTTP.
////////////////////////////////////////////////////////////////////
bool VirtualFile::
was_read_successful() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::open_write_file
//       Access: Published, Virtual
//  Description: Opens the file for writing.  Returns a newly
//               allocated ostream on success (which you should
//               eventually delete when you are done writing).
//               Returns NULL on failure.
////////////////////////////////////////////////////////////////////
ostream *VirtualFile::
open_write_file(bool auto_wrap, bool truncate) {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::open_append_file
//       Access: Published, Virtual
//  Description: Works like open_write_file(), but the file is opened
//               in append mode.  Like open_write_file, the returned
//               pointer should eventually be passed to
//               close_write_file().
////////////////////////////////////////////////////////////////////
ostream *VirtualFile::
open_append_file() {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::close_write_file
//       Access: Published
//  Description: Closes a file opened by a previous call to
//               open_write_file().  This really just deletes the
//               ostream pointer, but it is recommended to use this
//               interface instead of deleting it explicitly, to help
//               work around compiler issues.
////////////////////////////////////////////////////////////////////
void VirtualFile::
close_write_file(ostream *stream) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::open_read_write_file
//       Access: Published, Virtual
//  Description: Opens the file for writing.  Returns a newly
//               allocated iostream on success (which you should
//               eventually delete when you are done writing).
//               Returns NULL on failure.
////////////////////////////////////////////////////////////////////
iostream *VirtualFile::
open_read_write_file(bool truncate) {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::open_read_append_file
//       Access: Published, Virtual
//  Description: Works like open_read_write_file(), but the file is opened
//               in append mode.  Like open_read_write_file, the returned
//               pointer should eventually be passed to
//               close_read_write_file().
////////////////////////////////////////////////////////////////////
iostream *VirtualFile::
open_read_append_file() {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::close_read_write_file
//       Access: Published
//  Description: Closes a file opened by a previous call to
//               open_read_write_file().  This really just deletes the
//               iostream pointer, but it is recommended to use this
//               interface instead of deleting it explicitly, to help
//               work around compiler issues.
////////////////////////////////////////////////////////////////////
void VirtualFile::
close_read_write_file(iostream *stream) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::get_file_size
//       Access: Published, Virtual
//  Description: Returns the current size on disk (or wherever it is)
//               of the already-open file.  Pass in the stream that
//               was returned by open_read_file(); some
//               implementations may require this stream to determine
//               the size.
////////////////////////////////////////////////////////////////////
streamsize VirtualFile::
get_file_size(istream *stream) const {
  return get_file_size();
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::get_file_size
//       Access: Published, Virtual
//  Description: Returns the current size on disk (or wherever it is)
//               of the file before it has been opened.
////////////////////////////////////////////////////////////////////
streamsize VirtualFile::
get_file_size() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::get_timestamp
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
time_t VirtualFile::
get_timestamp() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::get_system_info
//       Access: Published, Virtual
//  Description: Populates the SubfileInfo structure with the data
//               representing where the file actually resides on disk,
//               if this is knowable.  Returns true if the file might
//               reside on disk, and the info is populated, or false
//               if it does not (or it is not known where the file
//               resides), in which case the info is meaningless.
////////////////////////////////////////////////////////////////////
bool VirtualFile::
get_system_info(SubfileInfo &info) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::atomic_compare_and_exchange_contents
//       Access: Public, Virtual
//  Description: See Filename::atomic_compare_and_exchange_contents().
////////////////////////////////////////////////////////////////////
bool VirtualFile::
atomic_compare_and_exchange_contents(string &orig_contents,
                                     const string &old_contents, 
                                     const string &new_contents) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::atomic_read_contents
//       Access: Public, Virtual
//  Description: See Filename::atomic_read_contents().
////////////////////////////////////////////////////////////////////
bool VirtualFile::
atomic_read_contents(string &contents) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::read_file
//       Access: Public
//  Description: Fills up the indicated string with the contents of
//               the file, if it is a regular file.  Returns true on
//               success, false otherwise.
////////////////////////////////////////////////////////////////////
bool VirtualFile::
read_file(string &result, bool auto_unwrap) const {
  result = string();

  pvector<unsigned char> pv;
  if (!read_file(pv, auto_unwrap)) {
    return false;
  }

  if (!pv.empty()) {
    result.append((const char *)&pv[0], pv.size());
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::read_file
//       Access: Public, Virtual
//  Description: Fills up the indicated pvector with the contents of
//               the file, if it is a regular file.  Returns true on
//               success, false otherwise.
////////////////////////////////////////////////////////////////////
bool VirtualFile::
read_file(pvector<unsigned char> &result, bool auto_unwrap) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::write_file
//       Access: Public, Virtual
//  Description: Writes the indicated data to the file, if it is
//               writable.  Returns true on success, false otherwise.
////////////////////////////////////////////////////////////////////
bool VirtualFile::
write_file(const unsigned char *data, size_t data_size, bool auto_wrap) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::simple_read_file
//       Access: Public, Static
//  Description: Fills up the indicated pvector with the contents of
//               the just-opened file.  Returns true on success, false
//               otherwise.  If the pvector was not empty on entry, the
//               data read from the file will be appended onto it.
////////////////////////////////////////////////////////////////////
bool VirtualFile::
simple_read_file(istream *in, pvector<unsigned char> &result) {
  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  in->read(buffer, buffer_size);
  size_t count = in->gcount();
  while (count != 0) {
    thread_consider_yield();
    result.insert(result.end(), buffer, buffer + count);
    in->read(buffer, buffer_size);
    count = in->gcount();
  }

  return (!in->fail() || in->eof());
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::simple_read_file
//       Access: Public
//  Description: As in simple_read_file() with two parameters, above,
//               but only reads up to max_bytes bytes from the file.
////////////////////////////////////////////////////////////////////
bool VirtualFile::
simple_read_file(istream *in, pvector<unsigned char> &result, size_t max_bytes) {
  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  in->read(buffer, min(buffer_size, max_bytes));
  size_t count = in->gcount();
  while (count != 0) {
    thread_consider_yield();
    nassertr(count <= max_bytes, false);
    result.insert(result.end(), buffer, buffer + count);
    max_bytes -= count;
    in->read(buffer, min(buffer_size, max_bytes));
    count = in->gcount();
  }

  return (!in->fail() || in->eof());
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::scan_local_directory
//       Access: Protected, Virtual
//  Description: Fills file_list up with the list of files that are
//               within this directory, excluding those whose
//               basenames are listed in mount_points.  Returns true
//               if successful, false if the file is not a directory
//               or the directory cannot be read.
////////////////////////////////////////////////////////////////////
bool VirtualFile::
scan_local_directory(VirtualFileList *, const ov_set<string> &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::r_ls_all
//       Access: Private
//  Description: The recursive implementation of ls_all().
////////////////////////////////////////////////////////////////////
void VirtualFile::
r_ls_all(ostream &out, const Filename &root) const {
  CPT(VirtualFileList) contents = scan_directory();
  if (contents == (VirtualFileList *)NULL) {
    return;
  }

  int num_files = contents->get_num_files();
  for (int i = 0; i < num_files; i++) {
    VirtualFile *file = contents->get_file(i);
    Filename filename = file->get_filename();
    filename.make_relative_to(root);
    out << filename << "\n";
    if (file->is_directory()) {
      file->r_ls_all(out, root);
    }
  }
}
