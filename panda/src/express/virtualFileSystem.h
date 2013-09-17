// Filename: virtualFileSystem.h
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

#ifndef VIRTUALFILESYSTEM_H
#define VIRTUALFILESYSTEM_H

#include "pandabase.h"

#include "virtualFile.h"
#include "virtualFileMount.h"
#include "virtualFileList.h"
#include "filename.h"
#include "dSearchPath.h"
#include "pointerTo.h"
#include "config_express.h"
#include "mutexImpl.h"
#include "pvector.h"

class Multifile;
class VirtualFileComposite;

////////////////////////////////////////////////////////////////////
//       Class : VirtualFileSystem
// Description : A hierarchy of directories and files that appears to
//               be one continuous file system, even though the files
//               may originate from several different sources that may
//               not be related to the actual OS's file system.
//
//               For instance, a VirtualFileSystem can transparently
//               mount one or more Multifiles as their own
//               subdirectory hierarchies.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS VirtualFileSystem {
PUBLISHED:
  VirtualFileSystem();
  ~VirtualFileSystem();

  enum MountFlags {
    MF_read_only      = 0x0002,
  };

  BLOCKING bool mount(Multifile *multifile, const Filename &mount_point, int flags);
  BLOCKING bool mount(const Filename &physical_filename, const Filename &mount_point, 
                      int flags, const string &password = "");
  BLOCKING bool mount_loop(const Filename &virtual_filename, const Filename &mount_point, 
                      int flags, const string &password = "");
  bool mount(VirtualFileMount *mount, const Filename &mount_point, int flags);
  BLOCKING int unmount(Multifile *multifile);
  BLOCKING int unmount(const Filename &physical_filename);
  int unmount(VirtualFileMount *mount);
  BLOCKING int unmount_point(const Filename &mount_point);
  BLOCKING int unmount_all();

  int get_num_mounts() const;
  PT(VirtualFileMount) get_mount(int n) const;
  MAKE_SEQ(get_mounts, get_num_mounts, get_mount);

  BLOCKING bool chdir(const Filename &new_directory);
  BLOCKING Filename get_cwd() const;
  BLOCKING bool make_directory(const Filename &filename);
  BLOCKING bool make_directory_full(const Filename &filename);

  BLOCKING PT(VirtualFile) get_file(const Filename &filename, bool status_only = false) const;
  BLOCKING PT(VirtualFile) create_file(const Filename &filename);
  BLOCKING PT(VirtualFile) find_file(const Filename &filename, 
                                     const DSearchPath &searchpath,
                                     bool status_only = false) const;
  BLOCKING bool delete_file(const Filename &filename);
  BLOCKING bool rename_file(const Filename &orig_filename, const Filename &new_filename);
  BLOCKING bool copy_file(const Filename &orig_filename, const Filename &new_filename);

  BLOCKING bool resolve_filename(Filename &filename, const DSearchPath &searchpath,
                                 const string &default_extension = string()) const;
  BLOCKING int find_all_files(const Filename &filename, const DSearchPath &searchpath,
                              DSearchPath::Results &results) const;

  BLOCKING INLINE bool exists(const Filename &filename) const;
  BLOCKING INLINE bool is_directory(const Filename &filename) const;
  BLOCKING INLINE bool is_regular_file(const Filename &filename) const;

  BLOCKING INLINE PT(VirtualFileList) scan_directory(const Filename &filename) const;

  INLINE void ls(const Filename &filename) const;
  INLINE void ls_all(const Filename &filename) const;

  void write(ostream &out) const;

  static VirtualFileSystem *get_global_ptr();

  EXTENSION(BLOCKING PyObject *read_file(const Filename &filename, bool auto_unwrap) const);
  BLOCKING istream *open_read_file(const Filename &filename, bool auto_unwrap) const;
  BLOCKING static void close_read_file(istream *stream);

  EXTENSION(BLOCKING PyObject *write_file(const Filename &filename, PyObject *data, bool auto_wrap));
  BLOCKING ostream *open_write_file(const Filename &filename, bool auto_wrap, bool truncate);
  BLOCKING ostream *open_append_file(const Filename &filename);
  BLOCKING static void close_write_file(ostream *stream);

  BLOCKING iostream *open_read_write_file(const Filename &filename, bool truncate);
  BLOCKING iostream *open_read_append_file(const Filename &filename);
  BLOCKING static void close_read_write_file(iostream *stream);

public:
  // We provide Python versions of these as efficient extension methods, above.
  BLOCKING INLINE string read_file(const Filename &filename, bool auto_unwrap) const;
  BLOCKING INLINE bool write_file(const Filename &filename, const string &data, bool auto_wrap);

  bool atomic_compare_and_exchange_contents(const Filename &filename, string &orig_contents, const string &old_contents, const string &new_contents);
  bool atomic_read_contents(const Filename &filename, string &contents) const;

  INLINE bool read_file(const Filename &filename, string &result, bool auto_unwrap) const;
  INLINE bool read_file(const Filename &filename, pvector<unsigned char> &result, bool auto_unwrap) const;
  INLINE bool write_file(const Filename &filename, const unsigned char *data, size_t data_size, bool auto_wrap);

  void scan_mount_points(vector_string &names, const Filename &path) const;
 
  static void parse_options(const string &options,
                            int &flags, string &password);
  static void parse_option(const string &option,
                          int &flags, string &password);

public:
  // These flags are passed to do_get_file() and
  // VirtualFileMount::make_virtual_file() to quality the kind of
  // VirtualFile pointer we want to get.
  enum OpenFlags {
    OF_status_only    = 0x0001,
    OF_create_file    = 0x0002,
    OF_make_directory = 0x0004,
    OF_allow_nonexist = 0x0008,
  };

  // These are declared as class instances, instead of as globals, to
  // guarantee they will be initialized by the time the
  // VirtualFileSystem's constructor runs.
  ConfigVariableBool vfs_case_sensitive;
  ConfigVariableBool vfs_implicit_pz;
  ConfigVariableBool vfs_implicit_mf;

private:
  Filename normalize_mount_point(const Filename &mount_point) const;
  bool do_mount(VirtualFileMount *mount, const Filename &mount_point, int flags);
  PT(VirtualFile) do_get_file(const Filename &filename, int open_flags) const;

  bool consider_match(PT(VirtualFile) &found_file, VirtualFileComposite *&composite_file,
                      VirtualFileMount *mount, const Filename &local_filename,
                      const Filename &original_filename, bool implicit_pz_file,
                      int open_flags) const;
  bool consider_mount_mf(const Filename &filename);

  MutexImpl _lock;
  typedef pvector<PT(VirtualFileMount) > Mounts;
  Mounts _mounts;
  unsigned int _mount_seq;

  Filename _cwd;

  static VirtualFileSystem *_global_ptr;
};

#include "virtualFileSystem.I"

#endif
