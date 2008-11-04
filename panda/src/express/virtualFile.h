// Filename: virtualFile.h
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

#ifndef VIRTUALFILE_H
#define VIRTUALFILE_H

#include "pandabase.h"

#include "filename.h"
#include "pointerTo.h"
#include "typedReferenceCount.h"
#include "ordered_vector.h"
#include "pvector.h"

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
  INLINE const Filename &get_original_filename() const;

  virtual bool has_file() const;
  virtual bool is_directory() const;
  virtual bool is_regular_file() const;

  BLOCKING PT(VirtualFileList) scan_directory() const;

  void output(ostream &out) const;
  BLOCKING void ls(ostream &out = cout) const;
  BLOCKING void ls_all(ostream &out = cout) const;

  BLOCKING INLINE string read_file(bool auto_unwrap) const;
  BLOCKING virtual istream *open_read_file(bool auto_unwrap) const;
  BLOCKING void close_read_file(istream *stream) const;
  virtual bool was_read_successful() const;
  BLOCKING virtual off_t get_file_size(istream *stream) const;
  BLOCKING virtual off_t get_file_size() const;
  BLOCKING virtual time_t get_timestamp() const;

public:
  INLINE void set_original_filename(const Filename &filename);
  bool read_file(string &result, bool auto_unwrap) const;
  bool read_file(pvector<unsigned char> &result, bool auto_unwrap) const;
  static bool read_file(istream *stream, pvector<unsigned char> &result);
  static bool read_file(istream *stream, pvector<unsigned char> &result, size_t max_bytes);


protected:
  virtual bool scan_local_directory(VirtualFileList *file_list, 
                                    const ov_set<string> &mount_points) const;

private:
  void r_ls_all(ostream &out, const Filename &root) const;

  Filename _original_filename;


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
