// Filename: virtualFileComposite.h
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

#ifndef VIRTUALFILECOMPOSITE_H
#define VIRTUALFILECOMPOSITE_H

#include "pandabase.h"

#include "virtualFile.h"

////////////////////////////////////////////////////////////////////
//       Class : VirtualFileComposite
// Description : A composite directory within the VirtualFileSystem:
//               this maps to more than one directory on different
//               mount points.  The resulting directory appears to be
//               the union of all the individual simple directories.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS VirtualFileComposite : public VirtualFile {
public:
  INLINE VirtualFileComposite(VirtualFileSystem *file_system,
                              const Filename &filename);

  INLINE void add_component(VirtualFile *file);

  virtual VirtualFileSystem *get_file_system() const;
  virtual Filename get_filename() const;

  virtual bool is_directory() const;

protected:
  virtual bool scan_local_directory(VirtualFileList *file_list, 
                                    const ov_set<string> &mount_points) const;

private:
  VirtualFileSystem *_file_system;
  Filename _filename;
  typedef pvector< PT(VirtualFile) > Components;
  Components _components;

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
    register_type(_type_handle, "VirtualFileComposite",
                  VirtualFile::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "virtualFileComposite.I"

#endif
