// Filename: virtualFileMountMultifile.h
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

#ifndef VIRTUALFILEMOUNTMULTIFILE_H
#define VIRTUALFILEMOUNTMULTIFILE_H

#include "pandabase.h"

#include "virtualFileMount.h"
#include "multifile.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : VirtualFileMountMultifile
// Description : Maps a Multifile's contents into the
//               VirtualFileSystem.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS VirtualFileMountMultifile : public VirtualFileMount {
public:
  INLINE VirtualFileMountMultifile(VirtualFileSystem *file_system,
                                   Multifile *multifile, 
                                   const Filename &mount_point,
                                   int mount_flags);
  virtual ~VirtualFileMountMultifile();

  INLINE Multifile *get_multifile() const;

  virtual bool has_file(const Filename &file) const;
  virtual bool is_directory(const Filename &file) const;
  virtual bool is_regular_file(const Filename &file) const;

  virtual istream *open_read_file(const Filename &file) const;
  virtual streampos get_file_size(const Filename &file, istream *stream) const;

  virtual bool scan_directory(vector_string &contents, 
                              const Filename &dir) const;


private:
  PT(Multifile) _multifile;


public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    VirtualFileMount::init_type();
    register_type(_type_handle, "VirtualFileMountMultifile",
                  VirtualFileMount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "virtualFileMountMultifile.I"

#endif
