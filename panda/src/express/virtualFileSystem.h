// Filename: virtualFileSystem.h
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

#ifndef VIRTUALFILESYSTEM_H
#define VIRTUALFILESYSTEM_H

#include "pandabase.h"

#include "virtualFile.h"
#include "filename.h"
#include "dSearchPath.h"
#include "pointerTo.h"
#include "pmap.h"
#include "config_express.h"

class Multifile;
class VirtualFileMount;
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
    MF_owns_pointer   = 0x0001,
    MF_read_only      = 0x0002,
  };

  bool mount(Multifile *multifile, const string &mount_point, int flags);
  bool mount(const Filename &physical_filename, const string &mount_point, int flags);
  int unmount(Multifile *multifile);
  int unmount(const Filename &physical_filename);
  int unmount_point(const string &mount_point);
  int unmount_all();

  bool chdir(const string &new_directory);
  const Filename &get_cwd() const;

  PT(VirtualFile) get_file(const Filename &filename) const;
  PT(VirtualFile) find_file(const Filename &filename, 
                            const DSearchPath &searchpath) const;
  bool resolve_filename(Filename &filename, const DSearchPath &searchpath,
                        const string &default_extension = string()) const;
  int find_all_files(const Filename &filename, const DSearchPath &searchpath,
                     DSearchPath::Results &results) const;

  INLINE bool exists(const Filename &filename) const;
  INLINE bool is_directory(const Filename &filename) const;
  INLINE bool is_regular_file(const Filename &filename) const;

  INLINE void ls(const string &filename) const;
  INLINE void ls_all(const string &filename) const;

  void write(ostream &out) const;

  static VirtualFileSystem *get_global_ptr();

  INLINE string read_file(const Filename &filename) const;
  INLINE istream *open_read_file(const Filename &filename) const;

public:
  INLINE bool read_file(const Filename &filename, string &result) const;

  void scan_mount_points(vector_string &names, const Filename &path) const;

private:
  Filename normalize_mount_point(const string &mount_point) const;
  bool found_match(PT(VirtualFile) &found_file, VirtualFileComposite *&composite_file,
                   VirtualFileMount *mount, const string &local_filename) const;

  typedef pvector<VirtualFileMount *> Mounts;
  Mounts _mounts;
  Filename _cwd;

  static VirtualFileSystem *_global_ptr;
};

#include "virtualFileSystem.I"

#endif
