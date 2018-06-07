/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileHTTP.h
 * @author drose
 * @date 2008-10-31
 */

#ifndef VIRTUALFILEHTTP_H
#define VIRTUALFILEHTTP_H

#include "pandabase.h"

#include "virtualFile.h"
#include "httpChannel.h"
#include "urlSpec.h"

#ifdef HAVE_OPENSSL

class VirtualFileMountHTTP;

/**
 * This maps a document retrieved from an HTTPClient into the
 * VirtualFileSystem, allowing models etc.  to be loaded directly from a web
 * page.
 */
class EXPCL_PANDA_DOWNLOADER VirtualFileHTTP : public VirtualFile {
public:
  VirtualFileHTTP(VirtualFileMountHTTP *mount,
                  const Filename &local_filename,
                  bool implicit_pz_file,
                  int open_flags);
  virtual ~VirtualFileHTTP();

  virtual VirtualFileSystem *get_file_system() const;
  virtual Filename get_filename() const;

  virtual bool has_file() const;
  virtual bool is_directory() const;
  virtual bool is_regular_file() const;
  INLINE bool is_implicit_pz_file() const;

  virtual std::istream *open_read_file(bool auto_unwrap) const;
  virtual bool was_read_successful() const;
  virtual std::streamsize get_file_size(std::istream *stream) const;
  virtual std::streamsize get_file_size() const;
  virtual time_t get_timestamp() const;

private:
  bool fetch_file(std::ostream *buffer_stream) const;
  std::istream *return_file(std::istream *buffer_stream, bool auto_unwrap) const;

  VirtualFileMountHTTP *_mount;
  Filename _local_filename;
  bool _implicit_pz_file;
  bool _status_only;
  URLSpec _url;
  PT(HTTPChannel) _channel;

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
    register_type(_type_handle, "VirtualFileHTTP",
                  VirtualFile::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "virtualFileHTTP.I"

#endif // HAVE_OPENSSL

#endif
