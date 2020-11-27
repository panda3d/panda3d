/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileMountZip.h
 * @author drose
 * @date 2002-08-03
 */

#ifndef VIRTUALFILEMOUNTZIP_H
#define VIRTUALFILEMOUNTZIP_H

#include "pandabase.h"

#include "virtualFileMount.h"
#include "zipArchive.h"
#include "pointerTo.h"

/**
 * Maps a .zip archive into the VirtualFileSystem.
 */
class EXPCL_PANDA_EXPRESS VirtualFileMountZip : public VirtualFileMount {
PUBLISHED:
  INLINE VirtualFileMountZip(ZipArchive *archive,
                             const Filename &directory = Filename());
  virtual ~VirtualFileMountZip();

  INLINE ZipArchive *get_archive() const;

public:
  virtual bool has_file(const Filename &file) const;
  virtual bool is_directory(const Filename &file) const;
  virtual bool is_regular_file(const Filename &file) const;

  virtual bool read_file(const Filename &file, bool do_uncompress,
                         vector_uchar &result) const;

  virtual std::istream *open_read_file(const Filename &file) const;
  virtual std::streamsize get_file_size(const Filename &file, std::istream *stream) const;
  virtual std::streamsize get_file_size(const Filename &file) const;
  virtual time_t get_timestamp(const Filename &file) const;
  virtual bool get_system_info(const Filename &file, SubfileInfo &info);

  virtual bool scan_directory(vector_string &contents,
                              const Filename &dir) const;

  virtual void output(std::ostream &out) const;

private:
  PT(ZipArchive) _archive;
  Filename _directory;

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
    register_type(_type_handle, "VirtualFileMountZip",
                  VirtualFileMount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "virtualFileMountZip.I"

#endif
