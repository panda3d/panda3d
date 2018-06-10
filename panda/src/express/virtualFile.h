/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFile.h
 * @author drose
 * @date 2002-08-03
 */

#ifndef VIRTUALFILE_H
#define VIRTUALFILE_H

#include "pandabase.h"

#include "filename.h"
#include "subfileInfo.h"
#include "pointerTo.h"
#include "typedReferenceCount.h"
#include "ordered_vector.h"
#include "vector_uchar.h"

class VirtualFileMount;
class VirtualFileList;
class VirtualFileSystem;


/**
 * The abstract base class for a file or directory within the
 * VirtualFileSystem.
 */
class EXPCL_PANDA_EXPRESS VirtualFile : public TypedReferenceCount {
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

  void output(std::ostream &out) const;
  BLOCKING void ls(std::ostream &out = std::cout) const;
  BLOCKING void ls_all(std::ostream &out = std::cout) const;

  EXTENSION(PyObject *read_file(bool auto_unwrap) const);
  BLOCKING virtual std::istream *open_read_file(bool auto_unwrap) const;
  BLOCKING virtual void close_read_file(std::istream *stream) const;
  virtual bool was_read_successful() const;

  EXTENSION(PyObject *write_file(PyObject *data, bool auto_wrap));
  BLOCKING virtual std::ostream *open_write_file(bool auto_wrap, bool truncate);
  BLOCKING virtual std::ostream *open_append_file();
  BLOCKING virtual void close_write_file(std::ostream *stream);

  BLOCKING virtual std::iostream *open_read_write_file(bool truncate);
  BLOCKING virtual std::iostream *open_read_append_file();
  BLOCKING virtual void close_read_write_file(std::iostream *stream);

  BLOCKING virtual std::streamsize get_file_size(std::istream *stream) const;
  BLOCKING virtual std::streamsize get_file_size() const;
  BLOCKING virtual time_t get_timestamp() const;

  virtual bool get_system_info(SubfileInfo &info);

public:
  virtual bool atomic_compare_and_exchange_contents(std::string &orig_contents, const std::string &old_contents, const std::string &new_contents);
  virtual bool atomic_read_contents(std::string &contents) const;

  INLINE std::string read_file(bool auto_unwrap) const;
  INLINE bool write_file(const std::string &data, bool auto_wrap);

  INLINE void set_original_filename(const Filename &filename);
  bool read_file(std::string &result, bool auto_unwrap) const;
  virtual bool read_file(vector_uchar &result, bool auto_unwrap) const;
  virtual bool write_file(const unsigned char *data, size_t data_size, bool auto_wrap);

  static bool simple_read_file(std::istream *stream, vector_uchar &result);
  static bool simple_read_file(std::istream *stream, vector_uchar &result, size_t max_bytes);

protected:
  virtual bool scan_local_directory(VirtualFileList *file_list,
                                    const ov_set<std::string> &mount_points) const;

private:
  void r_ls_all(std::ostream &out, const Filename &root) const;

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

INLINE std::ostream &operator << (std::ostream &out, const VirtualFile &file);


#include "virtualFile.I"

#endif
