// Filename: virtualFileSystem.cxx
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

#include "virtualFileSystem.h"
#include "virtualFileMount.h"
#include "virtualFileMountMultifile.h"
#include "virtualFileMountSystem.h"
#include "dSearchPath.h"
#include "dcast.h"
#include "config_express.h"
#include "executionEnvironment.h"
#include "pset.h"

VirtualFileSystem *VirtualFileSystem::_global_ptr = NULL;


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
VirtualFileSystem::
VirtualFileSystem() {
  _cwd = "/";
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
VirtualFileSystem::
~VirtualFileSystem() {
  unmount_all();
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::mount
//       Access: Published
//  Description: Mounts the indicated Multifile at the given mount
//               point.
////////////////////////////////////////////////////////////////////
bool VirtualFileSystem::
mount(Multifile *multifile, const string &mount_point, int flags) {
  VirtualFileMountMultifile *mount = 
    new VirtualFileMountMultifile(this, multifile, 
                                  normalize_mount_point(mount_point),
                                  flags);
  _mounts.push_back(mount);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::mount
//       Access: Published
//  Description: Mounts the indicated system file or directory at the
//               given mount point.  If the named file is a directory,
//               mounts the directory.  If the named file is a
//               Multifile, mounts it as a Multifile.  Returns true on
//               success, false on failure.
//
//               A given system directory may be mounted to multiple
//               different mount point, and the same mount point may
//               share multiple system directories.  In the case of
//               ambiguities (that is, two different files with
//               exactly the same full pathname), the most-recently
//               mounted system wins.
//
//               Note that a mounted VirtualFileSystem directory is
//               fully case-sensitive, unlike the native Windows file
//               system, so you must refer to files within the virtual
//               file system with exactly the right case.
////////////////////////////////////////////////////////////////////
bool VirtualFileSystem::
mount(const Filename &physical_filename, const string &mount_point, 
      int flags, const string &password) {
  if (!physical_filename.exists()) {
    express_cat.warning()
      << "Attempt to mount " << physical_filename << ", not found.\n";
    return false;
  }

  if (physical_filename.is_directory()) {
    VirtualFileMountSystem *mount =
      new VirtualFileMountSystem(this, physical_filename, 
                                 normalize_mount_point(mount_point),
                                 flags);
    _mounts.push_back(mount);
    return true;
  } else {
    // It's not a directory; it must be a Multifile.
    PT(Multifile) multifile = new Multifile;

    multifile->set_encryption_password(password);

    // For now these are always opened read only.  Maybe later we'll
    // support read-write on Multifiles.
    flags |= MF_read_only;
    if (!multifile->open_read(physical_filename)) {
      delete multifile;
      return false;
    }

    return mount(multifile, mount_point, flags);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::unmount
//       Access: Published
//  Description: Unmounts all appearances of the indicated Multifile
//               from the file system.  Returns the number of
//               appearances unmounted.
////////////////////////////////////////////////////////////////////
int VirtualFileSystem::
unmount(Multifile *multifile) {
  Mounts::iterator ri, wi;
  wi = ri = _mounts.begin();
  while (ri != _mounts.end()) {
    VirtualFileMount *mount = (*ri);
    (*wi) = mount;

    if (mount->is_exact_type(VirtualFileMountMultifile::get_class_type())) {
      VirtualFileMountMultifile *mmount = 
        DCAST(VirtualFileMountMultifile, mount);
      if (mmount->get_multifile() == multifile) {
        // Remove this one.  Don't increment wi.
        delete mount;
      } else {
        // Don't remove this one.
        ++wi;
      }
    } else {
      // Don't remove this one.
      ++wi;
    }
    ++ri;
  }

  int num_removed = _mounts.end() - wi;
  _mounts.erase(wi, _mounts.end());
  return num_removed;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::unmount
//       Access: Published
//  Description: Unmounts all appearances of the indicated physical
//               filename (either a directory name or a Multifile
//               name) from the file system.  Returns the number of
//               appearances unmounted.
////////////////////////////////////////////////////////////////////
int VirtualFileSystem::
unmount(const Filename &physical_filename) {
  Mounts::iterator ri, wi;
  wi = ri = _mounts.begin();
  while (ri != _mounts.end()) {
    VirtualFileMount *mount = (*ri);
    (*wi) = mount;

    if (mount->get_physical_filename() == physical_filename) {
      // Remove this one.  Don't increment wi.
      delete mount;
    } else {
      // Don't remove this one.
      ++wi;
    }
    ++ri;
  }

  int num_removed = _mounts.end() - wi;
  _mounts.erase(wi, _mounts.end());
  return num_removed;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::unmount_point
//       Access: Published
//  Description: Unmounts all systems attached to the given mount
//               point from the file system.  Returns the number of
//               appearances unmounted.
////////////////////////////////////////////////////////////////////
int VirtualFileSystem::
unmount_point(const string &mount_point) {
  Filename nmp = normalize_mount_point(mount_point);
  Mounts::iterator ri, wi;
  wi = ri = _mounts.begin();
  while (ri != _mounts.end()) {
    VirtualFileMount *mount = (*ri);
    (*wi) = mount;

    if (mount->get_mount_point() == nmp) {
      // Remove this one.  Don't increment wi.
      delete mount;
    } else {
      // Don't remove this one.
      ++wi;
    }
    ++ri;
  }

  int num_removed = _mounts.end() - wi;
  _mounts.erase(wi, _mounts.end());
  return num_removed;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::unmount_all
//       Access: Published
//  Description: Unmounts all files from the file system.  Returns the
//               number of systems unmounted.
////////////////////////////////////////////////////////////////////
int VirtualFileSystem::
unmount_all() {
  Mounts::iterator mi;
  for (mi = _mounts.begin(); mi != _mounts.end(); ++mi) {
    VirtualFileMount *mount = (*mi);
    delete mount;
  }

  int num_removed = _mounts.size();
  _mounts.clear();
  return num_removed;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::chdir
//       Access: Published
//  Description: Changes the current directory.  This is used to
//               resolve relative pathnames in get_file() and/or
//               find_file().  Returns true if successful, false
//               otherwise.
//
//               This accepts a string rather than a Filename simply
//               for programmer convenience from the Python prompt.
////////////////////////////////////////////////////////////////////
bool VirtualFileSystem::
chdir(const string &new_directory) {
  if (new_directory == "/") {
    // We can always return to the root.
    _cwd = new_directory;
    return true;
  }

  PT(VirtualFile) file = get_file(new_directory);
  if (file != (VirtualFile *)NULL && file->is_directory()) {
    _cwd = file->get_filename();
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::get_cwd
//       Access: Published
//  Description: Returns the current directory name.  See chdir().
////////////////////////////////////////////////////////////////////
const Filename &VirtualFileSystem::
get_cwd() const {
  return _cwd;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::get_file
//       Access: Published
//  Description: Looks up the file by the indicated name in the file
//               system.  Returns a VirtualFile pointer representing
//               the file if it is found, or NULL if it is not.
////////////////////////////////////////////////////////////////////
PT(VirtualFile) VirtualFileSystem::
get_file(const Filename &filename) const {
  nassertr(!filename.empty(), NULL);
  Filename pathname(filename);
  if (pathname.is_local()) {
    pathname = Filename(_cwd, filename);
  }
  pathname.standardize();
  string strpath = pathname.get_filename_index(0).get_fullpath().substr(1);

  // Now scan all the mount points, from the back (since later mounts
  // override more recent ones), until a match is found.
  PT(VirtualFile) found_file = NULL;
  VirtualFileComposite *composite_file = NULL;

  Mounts::const_reverse_iterator rmi;
  for (rmi = _mounts.rbegin(); rmi != _mounts.rend(); ++rmi) {
    VirtualFileMount *mount = (*rmi);
    string mount_point = mount->get_mount_point();
    if (strpath == mount_point) {
      // Here's an exact match on the mount point.  This filename is
      // the root directory of this mount object.
      if (found_match(found_file, composite_file, mount, "", pathname)) {
        return found_file;
      }
    } else if (mount_point.empty()) {
      // This is the root mount point; all files are in here.
      if (mount->has_file(strpath)) {
        // Bingo!
        if (found_match(found_file, composite_file, mount, strpath, pathname)) {
          return found_file;
        }
      }            
    } else if (strpath.length() > mount_point.length() &&
               strpath.substr(0, mount_point.length()) == mount_point &&
               strpath[mount_point.length()] == '/') {
      // This pathname falls within this mount system.
      Filename local_filename = strpath.substr(mount_point.length() + 1);
      if (mount->has_file(local_filename)) {
        // Bingo!
        if (found_match(found_file, composite_file, mount, local_filename, pathname)) {
          return found_file;
        }
      }            
    }
  }
  return found_file;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::find_file
//       Access: Published
//  Description: Uses the indicated search path to find the file
//               within the file system.  Returns the first occurrence
//               of the file found, or NULL if the file cannot be
//               found.
////////////////////////////////////////////////////////////////////
PT(VirtualFile) VirtualFileSystem::
find_file(const Filename &filename, const DSearchPath &searchpath) const {
  if (!filename.is_local()) {
    return get_file(filename);
  }

  int num_directories = searchpath.get_num_directories();
  for (int i = 0; i < num_directories; ++i) {
    Filename match(searchpath.get_directory(i), filename);
    if (searchpath.get_directory(i) == "." && 
        filename.is_fully_qualified()) {
      // A special case for the "." directory: to avoid prefixing an
      // endless stream of ./ in front of files, if the filename
      // already has a ./ prefixed (i.e. is_fully_qualified() is
      // true), we don't prefix another one.
      match = filename;
    }
    PT(VirtualFile) found_file = get_file(match);
    if (found_file != (VirtualFile *)NULL) {
      return found_file;
    }
  }

  return NULL;
}


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::resolve_filename
//       Access: Public
//  Description: Searches the given search path for the filename.  If
//               it is found, updates the filename to the full
//               pathname found and returns true; otherwise, returns
//               false.
////////////////////////////////////////////////////////////////////
bool VirtualFileSystem::
resolve_filename(Filename &filename,
                 const DSearchPath &searchpath,
                 const string &default_extension) const {
  PT(VirtualFile) found;

  if (filename.is_local()) {
    found = find_file(filename, searchpath);

    if (found.is_null()) {
      // We didn't find it with the given extension; can we try the
      // default extension?
      if (filename.get_extension().empty() && !default_extension.empty()) {
        Filename try_ext = filename;
        try_ext.set_extension(default_extension);
        found = find_file(try_ext, searchpath);
      }
    }
  } else {
    if (exists(filename)) {
      // The full pathname exists.  Return true.
      return true;
    } else {
      // The full pathname doesn't exist with the given extension;
      // does it exist with the default extension?
      if (filename.get_extension().empty() && !default_extension.empty()) {
        Filename try_ext = filename;
        try_ext.set_extension(default_extension);
        found = get_file(try_ext);
      }
    }
  }

  if (!found.is_null()) {
    filename = found->get_original_filename();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::find_all_files
//       Access: Public
//  Description: Searches all the directories in the search list for
//               the indicated file, in order.  Fills up the results
//               list with *all* of the matching filenames found, if
//               any.  Returns the number of matches found.
//
//               It is the responsibility of the the caller to clear
//               the results list first; otherwise, the newly-found
//               files will be appended to the list.
////////////////////////////////////////////////////////////////////
int VirtualFileSystem::
find_all_files(const Filename &filename, const DSearchPath &searchpath,
               DSearchPath::Results &results) const {
  int num_added = 0;

  if (filename.is_local()) {
    int num_directories = searchpath.get_num_directories();
    for (int i = 0; i < num_directories; ++i) {
      Filename match(searchpath.get_directory(i), filename);
      if (exists(match)) {
        if (searchpath.get_directory(i) == "." &&
            filename.is_fully_qualified()) {
          // A special case for the "." directory: to avoid prefixing
          // an endless stream of ./ in front of files, if the
          // filename already has a ./ prefixed
          // (i.e. is_fully_fully_qualified() is true), we don't
          // prefix another one.
          results.add_file(filename);
        } else {
          results.add_file(match);
        }
        ++num_added;
      }
    }
  }

  return num_added;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::write
//       Access: Published
//  Description: Print debugging information.
//               (e.g. from Python or gdb prompt).
////////////////////////////////////////////////////////////////////
void VirtualFileSystem::
write(ostream &out) const {
  out << "_cwd" << _cwd << "\n_mounts:\n";
  Mounts::const_iterator mi;
  for (mi = _mounts.begin(); mi != _mounts.end(); ++mi) {
    VirtualFileMount *mount = (*mi);
    mount->write(out);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::get_global_ptr
//       Access: Published, Static
//  Description: Returns the default global VirtualFileSystem.  You
//               may create your own personal VirtualFileSystem
//               objects and use them for whatever you like, but Panda
//               will attempt to load models and stuff from this
//               default object.
//
//               Initially, the global VirtualFileSystem is set up to
//               mount the OS filesystem to root; i.e. it is
//               equivalent to the OS filesystem.  This may be
//               subsequently adjusted by the user.
////////////////////////////////////////////////////////////////////
VirtualFileSystem *VirtualFileSystem::
get_global_ptr() {
  if (_global_ptr == (VirtualFileSystem *)NULL) {
    _global_ptr = new VirtualFileSystem;
    
    // Set up the default mounts.  First, there is always the root
    // mount.
    _global_ptr->mount("/", "/", 0);

    // And our initial cwd comes from the environment.
    _global_ptr->chdir(ExecutionEnvironment::get_cwd());

    // Then, we add whatever mounts are listed in the Configrc file.
    ConfigVariableList mounts
      ("vfs-mount",
       PRC_DESC("vfs-mount system-filename mount-point [options]"));

    int num_unique_values = mounts.get_num_unique_values();
    for (int i = 0; i < num_unique_values; i++) {
      string mount_desc = mounts.get_unique_value(i);

      // The vfs-mount syntax is:
      
      // vfs-mount system-filename mount-point [options]
      
      // The last two spaces mark the beginning of the mount point,
      // and of the options, respectively.  There might be multiple
      // spaces in the system filename, which are part of the
      // filename.
      
      // The last space marks the beginning of the mount point.
      // Spaces before that are part of the system filename.
      size_t space = mount_desc.rfind(' ');
      if (space == string::npos) {
        express_cat.warning()
          << "No space in vfs-mount descriptor: " << mount_desc << "\n";
        
      } else {
        string mount_point = mount_desc.substr(space + 1);
        while (space > 0 && isspace(mount_desc[space - 1])) {
          space--;
        }
        mount_desc = mount_desc.substr(0, space);
        string options;
        
        space = mount_desc.rfind(' ');
        if (space != string::npos) {
          // If there's another space, we have the optional options field.
          options = mount_point;
          mount_point = mount_desc.substr(space + 1);
          while (space > 0 && isspace(mount_desc[space - 1])) {
            --space;
          }
          mount_desc = mount_desc.substr(0, space);
        }
        
        mount_desc = ExecutionEnvironment::expand_string(mount_desc);
        Filename physical_filename = Filename::from_os_specific(mount_desc);
        
        int flags = 0;
        string password;
        
        // Split the options up by commas.
        size_t p = 0;
        size_t q = options.find(',', p);
        while (q != string::npos) {
          parse_option(options.substr(p, q - p),
                       flags, password);
          p = q + 1;
          q = options.find(',', p);
        }
        parse_option(options.substr(p), flags, password);
        
        _global_ptr->mount(physical_filename, mount_point, flags, password);
      }
    }
  }

  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::close_read_file
//       Access: Published
//  Description: Closes a file opened by a previous call to
//               open_read_file().  This really just deletes the
//               istream pointer, but it is recommended to use this
//               interface instead of deleting it explicitly, to help
//               work around compiler issues.
////////////////////////////////////////////////////////////////////
void VirtualFileSystem::
close_read_file(istream *stream) const {
  if (stream != (istream *)NULL) {
    // For some reason--compiler bug in gcc 3.2?--explicitly deleting
    // the stream pointer does not call the appropriate global delete
    // function; instead apparently calling the system delete
    // function.  So we call the delete function by hand instead.
    #ifndef NDEBUG
    stream->~istream();
    (*global_operator_delete)(stream);
    #else
    delete stream;
    #endif
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::scan_mount_points
//       Access: Public
//  Description: Adds to names a list of all the mount points in use
//               that are one directory below path, if any.  That is,
//               these are the external files or directories mounted
//               directly to the indicated path.
//
//               The names vector is filled with a set of basenames,
//               the basename part of the mount point.
////////////////////////////////////////////////////////////////////
void VirtualFileSystem::
scan_mount_points(vector_string &names, const Filename &path) const {
  nassertv(!path.empty() && !path.is_local());
  string prefix = path.get_fullpath().substr(1);
  Mounts::const_iterator mi;
  for (mi = _mounts.begin(); mi != _mounts.end(); ++mi) {
    VirtualFileMount *mount = (*mi);
    
    string mount_point = mount->get_mount_point();
    if (prefix.empty()) {
      // The indicated path is the root.  Is the mount point on the
      // root?
      if (mount_point.find('/') == string::npos) {
        // No embedded slashes, so the mount point is only one
        // directory below the root.
        names.push_back(mount_point);
      }
    } else {
      if (mount_point.substr(0, prefix.length()) == prefix &&
          mount_point.length() > prefix.length() &&
          mount_point[prefix.length()] == '/') {
        // This mount point is below the indicated path.  Is it only one
        // directory below?
        string basename = mount_point.substr(prefix.length());
        if (basename.find('/') == string::npos) {
          // No embedded slashes, so it's only one directory below.
          names.push_back(basename);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::normalize_mount_point
//       Access: Private
//  Description: Converts the mount point string supplied by the user
//               to standard form (relative to the current directory,
//               with no double slashes, and not terminating with a
//               slash).  The initial slash is removed.
////////////////////////////////////////////////////////////////////
Filename VirtualFileSystem::
normalize_mount_point(const string &mount_point) const {
  Filename nmp = mount_point;
  if (nmp.is_local()) {
    nmp = Filename(_cwd, mount_point);
  }
  nmp.standardize();
  nassertr(!nmp.empty() && nmp[0] == '/', nmp);
  return nmp.get_fullpath().substr(1);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::found_match
//       Access: Private
//  Description: Evaluates one match found during a get_file()
//               operation.  There may be multiple matches for a
//               particular filename due to the ambiguities introduced
//               by allowing multiple mount points, so we may have to
//               keep searching even after the first match is found.
//
//               Returns true if the search should terminate now, or
//               false if it should keep iterating.
////////////////////////////////////////////////////////////////////
bool VirtualFileSystem::
found_match(PT(VirtualFile) &found_file, VirtualFileComposite *&composite_file,
            VirtualFileMount *mount, const string &local_filename,
            const Filename &original_filename) const {
  if (found_file == (VirtualFile *)NULL) {
    // This was our first match.  Save it.
    found_file = new VirtualFileSimple(mount, local_filename);
    found_file->set_original_filename(original_filename);
    if (!mount->is_directory(local_filename)) {
      // If it's not a directory, we're done.
      return true;
    }
  } else {
    // This was our second match.  The previous match(es) must
    // have been directories.
    if (!mount->is_directory(local_filename)) {
      // However, this one isn't a directory.  We're done.
      return true;
    }

    // At least two directories matched to the same path.  We
    // need a composite directory.
    if (composite_file == (VirtualFileComposite *)NULL) {
      composite_file =
        new VirtualFileComposite((VirtualFileSystem *)this, found_file->get_original_filename());
      composite_file->set_original_filename(original_filename);
      composite_file->add_component(found_file);
      found_file = composite_file;
    }
    composite_file->add_component(new VirtualFileSimple(mount, local_filename));
  }

  // Keep going, looking for more directories.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSystem::parse_option
//       Access: Private, Static
//  Description: Parses one of the option flags in the options list on
//               the vfs-mount Config.prc line.
////////////////////////////////////////////////////////////////////
void VirtualFileSystem::
parse_option(const string &option, int &flags, string &password) {
  if (option == "0" || option.empty()) {
    // 0 is the null option.
  } else if (option == "ro") {
    flags |= MF_read_only;
  } else if (option.substr(0, 3) == "pw:") {
    password = option.substr(3);
  } else {
    express_cat.warning()
      << "Invalid option on vfs-mount: \"" << option << "\"\n";
  }
}
