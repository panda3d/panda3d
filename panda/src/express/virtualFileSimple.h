// Filename: virtualFileSimple.h
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

#ifndef VIRTUALFILESIMPLE_H
#define VIRTUALFILESIMPLE_H

#include "pandabase.h"

#include "virtualFile.h"

////////////////////////////////////////////////////////////////////
//       Class : VirtualFileSimple
// Description : A simple file or directory within the
//               VirtualFileSystem: this maps to exactly one file on
//               one mount point.  Most directories, and all regular
//               files, are of this kind.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS VirtualFileSimple : public VirtualFile {
public:
  INLINE VirtualFileSimple(VirtualFileMount *mount,
                           const Filename &local_filename);

  virtual VirtualFileSystem *get_file_system() const;
  virtual Filename get_filename() const;

  virtual bool is_directory() const;
  virtual bool is_regular_file() const;

  virtual istream *open_read_file() const;
  virtual streampos get_file_size(istream *stream) const;

protected:
  virtual bool scan_local_directory(VirtualFileList *file_list, 
                                    const ov_set<string> &mount_points) const;

private:
  VirtualFileMount *_mount;
  Filename _local_filename;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    VirtualFile::init_type();
    register_type(_type_handle, "VirtualFileSimple",
                  VirtualFile::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "virtualFileSimple.I"

#endif
