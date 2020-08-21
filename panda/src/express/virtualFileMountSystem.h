/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileMountSystem.h
 * @author drose
 * @date 2002-08-03
 */

#ifndef VIRTUALFILEMOUNTSYSTEM_H
#define VIRTUALFILEMOUNTSYSTEM_H

#include "pandabase.h"

#include "virtualFileMount.h"

/**
 * Maps an actual OS directory into the VirtualFileSystem.
 */
class EXPCL_PANDA_EXPRESS VirtualFileMountSystem : public VirtualFileMount {
PUBLISHED:
  INLINE VirtualFileMountSystem(const Filename &physical_filename);

  INLINE const Filename &get_physical_filename() const;

public:
  virtual bool has_file(const Filename &file) const;
  virtual bool create_file(const Filename &file);
  virtual bool make_directory(const Filename &file);
  virtual bool delete_file(const Filename &file);
  virtual bool rename_file(const Filename &orig_filename, const Filename &new_filename);
  virtual bool copy_file(const Filename &orig_filename, const Filename &new_filename);
  virtual bool is_directory(const Filename &file) const;
  virtual bool is_regular_file(const Filename &file) const;
  virtual bool is_writable(const Filename &file) const;

  virtual std::istream *open_read_file(const Filename &file) const;
  virtual std::ostream *open_write_file(const Filename &file, bool truncate);
  virtual std::ostream *open_append_file(const Filename &file);
  virtual std::iostream *open_read_write_file(const Filename &file, bool truncate);
  virtual std::iostream *open_read_append_file(const Filename &file);

  virtual std::streamsize get_file_size(const Filename &file, std::istream *stream) const;
  virtual std::streamsize get_file_size(const Filename &file) const;
  virtual time_t get_timestamp(const Filename &file) const;
  virtual bool get_system_info(const Filename &file, SubfileInfo &info);

  virtual bool scan_directory(vector_string &contents,
                              const Filename &dir) const;

  virtual bool atomic_compare_and_exchange_contents(const Filename &file, std::string &orig_contents, const std::string &old_contents, const std::string &new_contents);
  virtual bool atomic_read_contents(const Filename &file, std::string &contents) const;

  virtual void output(std::ostream &out) const;

private:
  Filename _physical_filename;

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
    register_type(_type_handle, "VirtualFileMountSystem",
                  VirtualFileMount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "virtualFileMountSystem.I"

#endif
