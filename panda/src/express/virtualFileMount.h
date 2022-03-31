/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileMount.h
 * @author drose
 * @date 2002-08-03
 */

#ifndef VIRTUALFILEMOUNT_H
#define VIRTUALFILEMOUNT_H

#include "pandabase.h"

#include "virtualFile.h"
#include "filename.h"
#include "pointerTo.h"
#include "typedReferenceCount.h"

class VirtualFileSystem;

/**
 * The abstract base class for a mount definition used within a
 * VirtualFileSystem.  Normally users don't need to monkey with this class
 * directly.
 */
class EXPCL_PANDA_EXPRESS VirtualFileMount : public TypedReferenceCount {
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
                                            int open_flags);

  virtual bool has_file(const Filename &file) const=0;
  virtual bool create_file(const Filename &file);
  virtual bool make_directory(const Filename &file);
  virtual bool delete_file(const Filename &file);
  virtual bool rename_file(const Filename &orig_filename, const Filename &new_filename);
  virtual bool copy_file(const Filename &orig_filename, const Filename &new_filename);
  virtual bool is_directory(const Filename &file) const=0;
  virtual bool is_regular_file(const Filename &file) const=0;
  virtual bool is_writable(const Filename &file) const;

  virtual bool read_file(const Filename &file, bool do_uncompress,
                         vector_uchar &result) const;
  virtual bool write_file(const Filename &file, bool do_compress,
                          const unsigned char *data, size_t data_size);

  virtual std::istream *open_read_file(const Filename &file) const=0;
  std::istream *open_read_file(const Filename &file, bool do_uncompress) const;
  virtual void close_read_file(std::istream *stream) const;

  virtual std::ostream *open_write_file(const Filename &file, bool truncate);
  std::ostream *open_write_file(const Filename &file, bool do_compress, bool truncate);
  virtual std::ostream *open_append_file(const Filename &file);
  virtual void close_write_file(std::ostream *stream);

  virtual std::iostream *open_read_write_file(const Filename &file, bool truncate);
  virtual std::iostream *open_read_append_file(const Filename &file);
  virtual void close_read_write_file(std::iostream *stream);

  virtual std::streamsize get_file_size(const Filename &file, std::istream *stream) const=0;
  virtual std::streamsize get_file_size(const Filename &file) const=0;
  virtual time_t get_timestamp(const Filename &file) const=0;
  virtual bool get_system_info(const Filename &file, SubfileInfo &info);

  virtual bool scan_directory(vector_string &contents,
                              const Filename &dir) const=0;

  virtual bool atomic_compare_and_exchange_contents(const Filename &file, std::string &orig_contents, const std::string &old_contents, const std::string &new_contents);
  virtual bool atomic_read_contents(const Filename &file, std::string &contents) const;

PUBLISHED:
  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out) const;

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

INLINE std::ostream &operator << (std::ostream &out, const VirtualFileMount &mount);

#include "virtualFileMount.I"

#endif
