// Filename: virtualFile.h
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

#ifndef VIRTUALFILE_H
#define VIRTUALFILE_H

#include "pandabase.h"

#include "filename.h"
#include "pointerTo.h"
#include "typedReferenceCount.h"
#include "ordered_vector.h"

class VirtualFileMount;
class VirtualFileList;
class VirtualFileSystem;


////////////////////////////////////////////////////////////////////
//       Class : VirtualFile
// Description : The abstract base class for a file or directory
//               within the VirtualFileSystem.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS VirtualFile : public TypedReferenceCount {
public:
  INLINE VirtualFile();

PUBLISHED:
  virtual VirtualFileSystem *get_file_system() const=0;
  virtual Filename get_filename() const=0;

  virtual bool is_directory() const;
  virtual bool is_regular_file() const;

  PT(VirtualFileList) scan_directory() const;

  void output(ostream &out) const;
  void ls(ostream &out = cout) const;
  void ls_all(ostream &out = cout) const;

  INLINE string read_file() const;
  virtual istream *open_read_file() const;
  void close_read_file(istream *stream) const;
  virtual streampos get_file_size(istream *stream) const;

public:
  bool read_file(string &result) const;
  static bool read_file(istream *stream, string &result);
  static bool read_file(istream *stream, string &result, size_t max_bytes);


protected:
  virtual bool scan_local_directory(VirtualFileList *file_list, 
                                    const ov_set<string> &mount_points) const;

private:
  void r_ls_all(ostream &out, const Filename &root) const;


public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "VirtualFile",
                  TypedReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class VirtualFileComposite;
};

INLINE ostream &operator << (ostream &out, const VirtualFile &file);


#include "virtualFile.I"

#endif
