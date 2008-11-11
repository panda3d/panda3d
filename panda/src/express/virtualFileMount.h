// Filename: virtualFileMount.h
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

#ifndef VIRTUALFILEMOUNT_H
#define VIRTUALFILEMOUNT_H

#include "pandabase.h"

#include "virtualFile.h"
#include "filename.h"
#include "pointerTo.h"
#include "typedReferenceCount.h"

class VirtualFileSystem;

////////////////////////////////////////////////////////////////////
//       Class : VirtualFileMount
// Description : The abstract base class for a mount definition used
//               within a VirtualFileSystem.  Normally users don't
//               need to monkey with this class directly.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS VirtualFileMount : public TypedReferenceCount {
PUBLISHED:
  INLINE VirtualFileMount();
  virtual ~VirtualFileMount();

  INLINE VirtualFileSystem *get_file_system() const;
  INLINE const Filename &get_mount_point() const;
  INLINE int get_mount_flags() const;

public:
  virtual PT(VirtualFile) make_virtual_file(const Filename &local_filename,
                                            const Filename &original_filename,
                                            bool implicit_pz_file,
                                            bool status_only);

  virtual bool has_file(const Filename &file) const=0;
  virtual bool is_directory(const Filename &file) const=0;
  virtual bool is_regular_file(const Filename &file) const=0;

  virtual istream *open_read_file(const Filename &file) const=0;
  void close_read_file(istream *stream) const;
  virtual off_t get_file_size(const Filename &file, istream *stream) const=0;
  virtual off_t get_file_size(const Filename &file) const=0;
  virtual time_t get_timestamp(const Filename &file) const=0;

  virtual bool scan_directory(vector_string &contents, 
                              const Filename &dir) const=0;

PUBLISHED:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out) const;

protected:
  VirtualFileSystem *_file_system;
  Filename _mount_point;
  int _mount_flags;


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
    register_type(_type_handle, "VirtualFileMount",
                  TypedReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class VirtualFileSystem;
};

INLINE ostream &operator << (ostream &out, const VirtualFileMount &mount);

#include "virtualFileMount.I"

#endif
