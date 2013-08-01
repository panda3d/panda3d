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
#include "subfileInfo.h"
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
  virtual bool is_writable() const;

  BLOCKING virtual bool delete_file();
  BLOCKING virtual bool rename_file(VirtualFile *new_file);
  BLOCKING virtual bool copy_file(VirtualFile *new_file);

  BLOCKING PT(VirtualFileList) scan_directory() const;

  void output(ostream &out) const;
  BLOCKING void ls(ostream &out = cout) const;
  BLOCKING void ls_all(ostream &out = cout) const;

  BLOCKING INLINE string read_file(bool auto_unwrap) const;
  BLOCKING virtual istream *open_read_file(bool auto_unwrap) const;
  BLOCKING virtual void close_read_file(istream *stream) const;
  virtual bool was_read_successful() const;

  BLOCKING INLINE bool write_file(const string &data, bool auto_wrap);
  BLOCKING virtual ostream *open_write_file(bool auto_wrap, bool truncate);
  BLOCKING virtual ostream *open_append_file();
  BLOCKING virtual void close_write_file(ostream *stream);

  BLOCKING virtual iostream *open_read_write_file(bool truncate);
  BLOCKING virtual iostream *open_read_append_file();
  BLOCKING virtual void close_read_write_file(iostream *stream);

  BLOCKING virtual streamsize get_file_size(istream *stream) const;
  BLOCKING virtual streamsize get_file_size() const;
  BLOCKING virtual time_t get_timestamp() const;

  virtual bool get_system_info(SubfileInfo &info);

public:
  virtual bool atomic_compare_and_exchange_contents(string &orig_contents, const string &old_contents, const string &new_contents);
  virtual bool atomic_read_contents(string &contents) const;

  INLINE void set_original_filename(const Filename &filename);
  bool read_file(string &result, bool auto_unwrap) const;
  virtual bool read_file(pvector<unsigned char> &result, bool auto_unwrap) const;
  virtual bool write_file(const unsigned char *data, size_t data_size, bool auto_wrap);

  static bool simple_read_file(istream *stream, pvector<unsigned char> &result);
  static bool simple_read_file(istream *stream, pvector<unsigned char> &result, size_t max_bytes);

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
