/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileMountRamdisk.cxx
 * @author drose
 * @date 2011-09-19
 */

#include "virtualFileMountRamdisk.h"
#include "subStream.h"
#include "dcast.h"

TypeHandle VirtualFileMountRamdisk::_type_handle;
TypeHandle VirtualFileMountRamdisk::FileBase::_type_handle;
TypeHandle VirtualFileMountRamdisk::File::_type_handle;
TypeHandle VirtualFileMountRamdisk::Directory::_type_handle;

/**

 */
VirtualFileMountRamdisk::
VirtualFileMountRamdisk() : _root("") {
}

/**
 * Returns true if the indicated file exists within the mount system.
 */
bool VirtualFileMountRamdisk::
has_file(const Filename &file) const {
  _lock.acquire();
  PT(FileBase) f = _root.do_find_file(file);
  _lock.release();
  return (f != NULL);
}

/**
 * Attempts to create the indicated file within the mount, if it does not
 * already exist.  Returns true on success (or if the file already exists), or
 * false if it cannot be created.
 */
bool VirtualFileMountRamdisk::
create_file(const Filename &file) {
  _lock.acquire();
  PT(File) f = _root.do_create_file(file);
  _lock.release();
  return (f != NULL);
}

/**
 * Attempts to delete the indicated file or directory within the mount.  This
 * can remove a single file or an empty directory.  It will not remove a
 * nonempty directory.  Returns true on success, false on failure.
 */
bool VirtualFileMountRamdisk::
delete_file(const Filename &file) {
  _lock.acquire();
  PT(FileBase) f = _root.do_delete_file(file);
  _lock.release();
  return (f != NULL);
}

/**
 * Attempts to rename the contents of the indicated file to the indicated file.
 * Both filenames will be within the mount.  Returns true on success, false on
 * failure.  If this returns false, this will be attempted again with a copy-
 * and-delete operation.
 */
bool VirtualFileMountRamdisk::
rename_file(const Filename &orig_filename, const Filename &new_filename) {
  _lock.acquire();
  PT(FileBase) orig_fb = _root.do_find_file(orig_filename);
  if (orig_fb == NULL) {
    _lock.release();
    return false;
  }

  if (orig_fb->is_directory()) {
    // Rename the directory.
    Directory *orig_d = DCAST(Directory, orig_fb);
    PT(Directory) new_d = _root.do_make_directory(new_filename);
    if (new_d == NULL || !new_d->_files.empty()) {
      _lock.release();
      return false;
    }

    if (express_cat.is_debug()) {
      express_cat.debug()
        << "Renaming ramdisk directory " << orig_filename << " to " << new_filename << "\n";
    }

    new_d->_files.swap(orig_d->_files);
    _root.do_delete_file(orig_filename);
    _lock.release();
    return true;
  }

  // Rename the file.
  File *orig_f = DCAST(File, orig_fb);
  PT(File) new_f = _root.do_create_file(new_filename);
  if (new_f == NULL) {
    _lock.release();
    return false;
  }

  if (express_cat.is_debug()) {
    express_cat.debug()
      << "Renaming ramdisk file " << orig_filename << " to " << new_filename << "\n";
  }

  new_f->_data.str(orig_f->_data.str());
  _root.do_delete_file(orig_filename);

  _lock.release();
  return true;
}

/**
 * Attempts to copy the contents of the indicated file to the indicated file.
 * Both filenames will be within the mount.  Returns true on success, false on
 * failure.  If this returns false, the copy will be performed by explicit read-
 * and-write operations.
 */
bool VirtualFileMountRamdisk::
copy_file(const Filename &orig_filename, const Filename &new_filename) {
  _lock.acquire();
  PT(FileBase) orig_fb = _root.do_find_file(orig_filename);
  if (orig_fb == NULL || orig_fb->is_directory()) {
    _lock.release();
    return false;
  }

  // Copy the file.
  File *orig_f = DCAST(File, orig_fb);
  PT(File) new_f = _root.do_create_file(new_filename);
  if (new_f == NULL) {
    _lock.release();
    return false;
  }

  if (express_cat.is_debug()) {
    express_cat.debug()
      << "Copying ramdisk file " << orig_filename << " to " << new_filename << "\n";
  }

  new_f->_data.str(orig_f->_data.str());

  _lock.release();
  return true;
}

/**
 * Attempts to create the indicated file within the mount, if it does not
 * already exist.  Returns true on success, or false if it cannot be created.
 * If the directory already existed prior to this call, may return either true
 * or false.
 */
bool VirtualFileMountRamdisk::
make_directory(const Filename &file) {
  _lock.acquire();
  PT(Directory) f = _root.do_make_directory(file);
  _lock.release();
  return (f != NULL);
}

/**
 * Returns true if the indicated file exists within the mount system and is a
 * directory.
 */
bool VirtualFileMountRamdisk::
is_directory(const Filename &file) const {
  _lock.acquire();
  PT(FileBase) f = _root.do_find_file(file);
  _lock.release();
  return (f != NULL && f->is_directory());
}

/**
 * Returns true if the indicated file exists within the mount system and is a
 * regular file.
 */
bool VirtualFileMountRamdisk::
is_regular_file(const Filename &file) const {
  _lock.acquire();
  PT(FileBase) f = _root.do_find_file(file);
  _lock.release();
  return (f != NULL && !f->is_directory());
}

/**
 * Returns true if the named file or directory may be written to, false
 * otherwise.
 */
bool VirtualFileMountRamdisk::
is_writable(const Filename &file) const {
  return has_file(file);
}

/**
 * Opens the file for reading, if it exists.  Returns a newly allocated istream
 * on success (which you should eventually delete when you are done reading).
 * Returns NULL on failure.
 */
istream *VirtualFileMountRamdisk::
open_read_file(const Filename &file) const {
  _lock.acquire();
  PT(FileBase) f = _root.do_find_file(file);
  _lock.release();
  if (f == (FileBase *)NULL || f->is_directory()) {
    return NULL;
  }

  File *f2 = DCAST(File, f);
  return new ISubStream(&f2->_wrapper, 0, 0);
}

/**
 * Opens the file for writing.  Returns a newly allocated ostream on success
 * (which you should eventually delete when you are done writing). Returns NULL
 * on failure.
 */
ostream *VirtualFileMountRamdisk::
open_write_file(const Filename &file, bool truncate) {
  _lock.acquire();
  PT(File) f = _root.do_create_file(file);
  _lock.release();
  if (f == (File *)NULL) {
    return NULL;
  }

  if (truncate) {
    // Reset to an empty string.
    f->_data.str(string());
    f->_timestamp = time(NULL);
  }

  return new OSubStream(&f->_wrapper, 0, 0);
}

/**
 * Works like open_write_file(), but the file is opened in append mode.  Like
 * open_write_file, the returned pointer should eventually be passed to
 * close_write_file().
 */
ostream *VirtualFileMountRamdisk::
open_append_file(const Filename &file) {
  _lock.acquire();
  PT(File) f = _root.do_create_file(file);
  _lock.release();
  if (f == (File *)NULL) {
    return NULL;
  }

  return new OSubStream(&f->_wrapper, 0, 0, true);
}

/**
 * Opens the file for writing.  Returns a newly allocated iostream on success
 * (which you should eventually delete when you are done writing). Returns NULL
 * on failure.
 */
iostream *VirtualFileMountRamdisk::
open_read_write_file(const Filename &file, bool truncate) {
  _lock.acquire();
  PT(File) f = _root.do_create_file(file);
  _lock.release();
  if (f == (File *)NULL) {
    return NULL;
  }

  if (truncate) {
    // Reset to an empty string.
    f->_data.str(string());
    f->_timestamp = time(NULL);
  }

  return new SubStream(&f->_wrapper, 0, 0);
}

/**
 * Works like open_read_write_file(), but the file is opened in append mode.
 * Like open_read_write_file, the returned pointer should eventually be passed
 * to close_read_write_file().
 */
iostream *VirtualFileMountRamdisk::
open_read_append_file(const Filename &file) {
  _lock.acquire();
  PT(FileBase) f = _root.do_find_file(file);
  _lock.release();
  if (f == (FileBase *)NULL || f->is_directory()) {
    return NULL;
  }

  File *f2 = DCAST(File, f);
  return new SubStream(&f2->_wrapper, 0, 0, true);
}

/**
 * Returns the current size on disk (or wherever it is) of the already-open
 * file.  Pass in the stream that was returned by open_read_file(); some
 * implementations may require this stream to determine the size.
 */
streamsize VirtualFileMountRamdisk::
get_file_size(const Filename &file, istream *stream) const {
  _lock.acquire();
  PT(FileBase) f = _root.do_find_file(file);
  _lock.release();
  if (f == (FileBase *)NULL || f->is_directory()) {
    return 0;
  }

  File *f2 = DCAST(File, f);
  return f2->_data.str().length();
}

/**
 * Returns the current size on disk (or wherever it is) of the file before it
 * has been opened.
 */
streamsize VirtualFileMountRamdisk::
get_file_size(const Filename &file) const {
  _lock.acquire();
  PT(FileBase) f = _root.do_find_file(file);
  _lock.release();
  if (f == (FileBase *)NULL || f->is_directory()) {
    return 0;
  }

  File *f2 = DCAST(File, f);
  return f2->_data.str().length();
}

/**
 * Returns a time_t value that represents the time the file was last modified,
 * to within whatever precision the operating system records this information
 * (on a Windows95 system, for instance, this may only be accurate to within 2
 * seconds).  If the timestamp cannot be determined, either because it is not
 * supported by the operating system or because there is some error (such as
 * file not found), returns 0.
 */
time_t VirtualFileMountRamdisk::
get_timestamp(const Filename &file) const {
  _lock.acquire();
  PT(FileBase) f = _root.do_find_file(file);
  if (f.is_null()) {
    _lock.release();
    return 0;
  }
  time_t timestamp = f->_timestamp;
  _lock.release();
  return timestamp;
}

/**
 * Fills the given vector up with the list of filenames that are local to this
 * directory, if the filename is a directory.  Returns true if successful, or
 * false if the file is not a directory or cannot be read.
 */
bool VirtualFileMountRamdisk::
scan_directory(vector_string &contents, const Filename &dir) const {
  _lock.acquire();
  PT(FileBase) f = _root.do_find_file(dir);
  if (f == (FileBase *)NULL || !f->is_directory()) {
    _lock.release();
    return false;
  }

  Directory *f2 = DCAST(Directory, f);
  bool result = f2->do_scan_directory(contents);

  _lock.release();
  return result;
}

/**
 * See Filename::atomic_compare_and_exchange_contents().
 */
bool VirtualFileMountRamdisk::
atomic_compare_and_exchange_contents(const Filename &file, string &orig_contents,
                                     const string &old_contents,
                                     const string &new_contents) {
  _lock.acquire();
  PT(FileBase) f = _root.do_find_file(file);
  if (f == (FileBase *)NULL || f->is_directory()) {
    _lock.release();
    return false;
  }

  bool retval = false;
  File *f2 = DCAST(File, f);
  orig_contents = f2->_data.str();
  if (orig_contents == old_contents) {
    f2->_data.str(new_contents);
    f2->_timestamp = time(NULL);
    retval = true;
  }

  _lock.release();
  return retval;
}

/**
 * See Filename::atomic_read_contents().
 */
bool VirtualFileMountRamdisk::
atomic_read_contents(const Filename &file, string &contents) const {
  _lock.acquire();
  PT(FileBase) f = _root.do_find_file(file);
  if (f == (FileBase *)NULL || f->is_directory()) {
    _lock.release();
    return false;
  }

  File *f2 = DCAST(File, f);
  contents = f2->_data.str();

  _lock.release();
  return true;
}


/**

 */
void VirtualFileMountRamdisk::
output(ostream &out) const {
  out << "VirtualFileMountRamdisk";
}

/**

 */
VirtualFileMountRamdisk::FileBase::
~FileBase() {
}

/**

 */
bool VirtualFileMountRamdisk::FileBase::
is_directory() const {
  return false;
}

/**

 */
bool VirtualFileMountRamdisk::Directory::
is_directory() const {
  return true;
}

/**
 * Recursively search for the file with the indicated name in this directory
 * hierarchy.
 */
PT(VirtualFileMountRamdisk::FileBase) VirtualFileMountRamdisk::Directory::
do_find_file(const string &filename) const {
  size_t slash = filename.find('/');
  if (slash == string::npos) {
    // Search for a file within the local directory.
    FileBase tfile(filename);
    tfile.local_object();
    Files::const_iterator fi = _files.find(&tfile);
    if (fi != _files.end()) {
      return (*fi);
    }
    return NULL;
  }

  // A nested directory.  Search for the directory name, then recurse.
  string dirname = filename.substr(0, slash);
  string remainder = filename.substr(slash + 1);
  FileBase tfile(dirname);
  tfile.local_object();
  Files::const_iterator fi = _files.find(&tfile);
  if (fi != _files.end()) {
    PT(FileBase) file = (*fi);
    if (file->is_directory()) {
      return DCAST(Directory, file.p())->do_find_file(remainder);
    }
  }

  return NULL;
}

/**
 * Recursively search for the file with the indicated name in this directory
 * hierarchy.  If not found, creates a new file.
 */
PT(VirtualFileMountRamdisk::File) VirtualFileMountRamdisk::Directory::
do_create_file(const string &filename) {
  size_t slash = filename.find('/');
  if (slash == string::npos) {
    // Search for a file within the local directory.
    FileBase tfile(filename);
    tfile.local_object();
    Files::iterator fi = _files.find(&tfile);
    if (fi != _files.end()) {
      PT(FileBase) file = (*fi);
      if (!file->is_directory()) {
        return DCAST(File, file.p());
      }
      // Cannot create: a directory by the same name already exists.
      return NULL;
    }

    // Create a new file.
    if (express_cat.is_debug()) {
      express_cat.debug()
        << "Making ramdisk file " << filename << "\n";
    }
    PT(File) file = new File(filename);
    _files.insert(file.p());
    _timestamp = time(NULL);
    return file;
  }

  // A nested directory.  Search for the directory name, then recurse.
  string dirname = filename.substr(0, slash);
  string remainder = filename.substr(slash + 1);
  FileBase tfile(dirname);
  tfile.local_object();
  Files::iterator fi = _files.find(&tfile);
  if (fi != _files.end()) {
    PT(FileBase) file = (*fi);
    if (file->is_directory()) {
      return DCAST(Directory, file.p())->do_create_file(remainder);
    }
  }

  return NULL;
}

/**
 * Recursively search for the file with the indicated name in this directory
 * hierarchy.  If not found, creates a new directory.
 */
PT(VirtualFileMountRamdisk::Directory) VirtualFileMountRamdisk::Directory::
do_make_directory(const string &filename) {
  size_t slash = filename.find('/');
  if (slash == string::npos) {
    // Search for a file within the local directory.
    FileBase tfile(filename);
    tfile.local_object();
    Files::iterator fi = _files.find(&tfile);
    if (fi != _files.end()) {
      PT(FileBase) file = (*fi);
      if (file->is_directory()) {
        return DCAST(Directory, file.p());
      }
      // Cannot create: a file by the same name already exists.
      return NULL;
    }

    // Create a new directory.
    if (express_cat.is_debug()) {
      express_cat.debug()
        << "Making ramdisk directory " << filename << "\n";
    }
    PT(Directory) file = new Directory(filename);
    _files.insert(file.p());
    _timestamp = time(NULL);
    return file;
  }

  // A nested directory.  Search for the directory name, then recurse.
  string dirname = filename.substr(0, slash);
  string remainder = filename.substr(slash + 1);
  FileBase tfile(dirname);
  tfile.local_object();
  Files::iterator fi = _files.find(&tfile);
  if (fi != _files.end()) {
    PT(FileBase) file = (*fi);
    if (file->is_directory()) {
      return DCAST(Directory, file.p())->do_make_directory(remainder);
    }
  }

  return NULL;
}

/**
 * Recursively search for the file with the indicated name in this directory
 * hierarchy, and removes it.  Returns the removed FileBase object.
 */
PT(VirtualFileMountRamdisk::FileBase) VirtualFileMountRamdisk::Directory::
do_delete_file(const string &filename) {
  size_t slash = filename.find('/');
  if (slash == string::npos) {
    // Search for a file within the local directory.
    FileBase tfile(filename);
    tfile.local_object();
    Files::iterator fi = _files.find(&tfile);
    if (fi != _files.end()) {
      PT(FileBase) file = (*fi);
      if (file->is_directory()) {
        Directory *dir = DCAST(Directory, file.p());
        if (!dir->_files.empty()) {
          // Can't delete a nonempty directory.
          return NULL;
        }
      }
      _files.erase(fi);
      _timestamp = time(NULL);
      return file;
    }
    return NULL;
  }

  // A nested directory.  Search for the directory name, then recurse.
  string dirname = filename.substr(0, slash);
  string remainder = filename.substr(slash + 1);
  FileBase tfile(dirname);
  tfile.local_object();
  Files::iterator fi = _files.find(&tfile);
  if (fi != _files.end()) {
    PT(FileBase) file = (*fi);
    if (file->is_directory()) {
      return DCAST(Directory, file.p())->do_delete_file(remainder);
    }
  }

  return NULL;
}

/**

 */
bool VirtualFileMountRamdisk::Directory::
do_scan_directory(vector_string &contents) const {
  Files::const_iterator fi;
  for (fi = _files.begin(); fi != _files.end(); ++fi) {
    FileBase *file = (*fi);
    contents.push_back(file->_basename);
  }

  return true;
}
