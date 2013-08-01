// Filename: virtualFileSimple.h
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
                           const Filename &local_filename,
                           bool implicit_pz_file,
                           int open_flags);

PUBLISHED:
  virtual VirtualFileSystem *get_file_system() const;
  INLINE VirtualFileMount *get_mount() const;
  virtual Filename get_filename() const;

  virtual bool has_file() const;
  virtual bool is_directory() const;
  virtual bool is_regular_file() const;
  virtual bool is_writable() const;
  INLINE bool is_implicit_pz_file() const;

  virtual bool delete_file();
  virtual bool rename_file(VirtualFile *new_file);
  virtual bool copy_file(VirtualFile *new_file);

  virtual istream *open_read_file(bool auto_unwrap) const;
  virtual void close_read_file(istream *stream) const;
  virtual ostream *open_write_file(bool auto_wrap, bool truncate);
  virtual ostream *open_append_file();
  virtual void close_write_file(ostream *stream);
  virtual iostream *open_read_write_file(bool truncate);
  virtual iostream *open_read_append_file();
  virtual void close_read_write_file(iostream *stream);

  virtual streamsize get_file_size(istream *stream) const;
  virtual streamsize get_file_size() const;
  virtual time_t get_timestamp() const;
  virtual bool get_system_info(SubfileInfo &info);

public:
  virtual bool atomic_compare_and_exchange_contents(string &orig_contents, const string &old_contents, const string &new_contents);
  virtual bool atomic_read_contents(string &contents) const;

  virtual bool read_file(pvector<unsigned char> &result, bool auto_unwrap) const;
  virtual bool write_file(const unsigned char *data, size_t data_size, bool auto_wrap);

protected:
  virtual bool scan_local_directory(VirtualFileList *file_list, 
                                    const ov_set<string> &mount_points) const;

private:
  VirtualFileMount *_mount;
  Filename _local_filename;
  bool _implicit_pz_file;
  int _open_flags;

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
