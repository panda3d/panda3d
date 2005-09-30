// Filename: virtualFile.cxx
// Created by:  drose (03Aug02)
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

#include "virtualFile.h"
#include "virtualFileSystem.h"
#include "virtualFileList.h"
#include "config_express.h"
#include "pvector.h"

TypeHandle VirtualFile::_type_handle;

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
open_read_file() const {
  return NULL;
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
streampos VirtualFile::
get_file_size(istream *stream) const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::close_read_file
//       Access: Public
//  Description: Closes a file opened by a previous call to
//               open_read_file().  This really just deletes the
//               istream pointer, but it is recommended to use this
//               interface instead of deleting it explicitly, to help
//               work around compiler issues.
////////////////////////////////////////////////////////////////////
void VirtualFile::
close_read_file(istream *stream) const {
  if (stream != (istream *)NULL) {
    // For some reason--compiler bug in gcc 3.2?--explicitly deleting
    // the stream pointer does not call the appropriate global delete
    // function; instead apparently calling the system delete
    // function.  So we call the delete function by hand instead.
#ifndef USE_MEMORY_NOWRAPPERS
    stream->~istream();
    (*global_operator_delete)(stream);
#else
    delete stream;
#endif
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::read_file
//       Access: Public
//  Description: Fills up the indicated string with the contents of
//               the file, if it is a regular file.  Returns true on
//               success, false otherwise.
////////////////////////////////////////////////////////////////////
bool VirtualFile::
read_file(string &result) const {
  result = string();

  istream *in = open_read_file();
  if (in == (istream *)NULL) {
    express_cat.info()
      << "Unable to read " << get_filename() << "\n";
    return false;
  }

  bool okflag = read_file(in, result);

  close_read_file(in);

  if (!okflag) {
    express_cat.info()
      << "Error while reading " << get_filename() << "\n";
  }
  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::read_file
//       Access: Public, Static
//  Description: Fills up the indicated string with the contents of
//               the just-opened file.  Returns true on success, false
//               otherwise.  If the string was not empty on entry, the
//               data read from the file will be concatenated onto it.
////////////////////////////////////////////////////////////////////
bool VirtualFile::
read_file(istream *in, string &result) {
  // Repeatedly appending into a string seems to be prohibitively
  // expensive on MSVC7's implementation of string, but the vector
  // implementation works much better.  Even still, it seems to be
  // better to add the data in large chunks, rather than one byte at a
  // time.
  pvector<char> result_vec;

  static const size_t buffer_size = 1024;
  char buffer[buffer_size];

  in->read(buffer, buffer_size);
  size_t count = in->gcount();
  while (count != 0) {
    result_vec.insert(result_vec.end(), buffer, buffer + count);
    in->read(buffer, buffer_size);
    count = in->gcount();
  }
  result.append(&result_vec[0], result_vec.size());

  return (!in->fail() || in->eof());
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::read_file
//       Access: Public, Static
//  Description: As in read_file() with two parameters, above, but
//               only reads up to max_bytes bytes from the file.
////////////////////////////////////////////////////////////////////
bool VirtualFile::
read_file(istream *in, string &result, size_t max_bytes) {
  pvector<char> result_vec;

  static const size_t buffer_size = 1024;
  char buffer[buffer_size];

  in->read(buffer, min(buffer_size, max_bytes));
  size_t count = in->gcount();
  while (count != 0) {
    nassertr(count <= max_bytes, false);
    result_vec.insert(result_vec.end(), buffer, buffer + count);
    max_bytes -= count;
    in->read(buffer, min(buffer_size, max_bytes));
    count = in->gcount();
  }
  result.append(&result_vec[0], result_vec.size());

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
