/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFileMountHTTP.h
 * @author drose
 * @date 2008-10-30
 */

#ifndef VIRTUALFILEMOUNTHTTP_H
#define VIRTUALFILEMOUNTHTTP_H

#include "pandabase.h"

#ifdef HAVE_OPENSSL

#include "virtualFileMount.h"
#include "httpClient.h"
#include "httpChannel.h"
#include "urlSpec.h"
#include "pointerTo.h"
#include "mutexImpl.h"

/**
 * Maps a web page (URL root) into the VirtualFileSystem.
 */
class EXPCL_PANDA_DOWNLOADER VirtualFileMountHTTP : public VirtualFileMount {
PUBLISHED:
  explicit VirtualFileMountHTTP(const URLSpec &root, HTTPClient *http = HTTPClient::get_global_ptr());
  virtual ~VirtualFileMountHTTP();

  INLINE HTTPClient *get_http_client() const;
  INLINE const URLSpec &get_root() const;

  static void reload_vfs_mount_url();

public:
  virtual PT(VirtualFile) make_virtual_file(const Filename &local_filename,
                                            const Filename &original_filename,
                                            bool implicit_pz_file,
                                            int open_flags);

  virtual bool has_file(const Filename &file) const;
  virtual bool is_directory(const Filename &file) const;
  virtual bool is_regular_file(const Filename &file) const;

  virtual std::istream *open_read_file(const Filename &file) const;
  virtual std::streamsize get_file_size(const Filename &file, std::istream *stream) const;
  virtual std::streamsize get_file_size(const Filename &file) const;
  virtual time_t get_timestamp(const Filename &file) const;

  virtual bool scan_directory(vector_string &contents,
                              const Filename &dir) const;

  virtual void output(std::ostream &out) const;

  PT(HTTPChannel) get_channel();
  void recycle_channel(HTTPChannel *channel);

private:
  PT(HTTPClient) _http;
  URLSpec _root;

  MutexImpl _channels_lock;
  typedef pvector< PT(HTTPChannel) > Channels;
  Channels _channels;

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
    register_type(_type_handle, "VirtualFileMountHTTP",
                  VirtualFileMount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "virtualFileMountHTTP.I"

#endif  // HAVE_OPENSSL

#endif
