// Filename: httpDocument.h
// Created by:  drose (24Sep02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef HTTPDOCUMENT_H
#define HTTPDOCUMENT_H

#include "pandabase.h"

// This module requires OpenSSL to compile, even if you do not intend
// to use this to establish https connections; this is because it uses
// the OpenSSL library to portably handle all of the socket
// communications.

#ifdef HAVE_SSL

#include "urlSpec.h"
#include "virtualFile.h"
#include "pmap.h"
#include <openssl/ssl.h>

class IBioStream;

////////////////////////////////////////////////////////////////////
//       Class : HTTPDocument
// Description : A single document retrieved from an HTTP server.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS HTTPDocument : public VirtualFile {
public:
  HTTPDocument(BIO *bio, bool owns_bio);
  virtual ~HTTPDocument();

  virtual VirtualFileSystem *get_file_system() const;
  virtual Filename get_filename() const;

  virtual bool is_regular_file() const;
  virtual istream *open_read_file() const;

PUBLISHED:
  INLINE bool is_valid() const;
  INLINE const string &get_http_version() const;
  INLINE int get_status_code() const;
  INLINE const string &get_status_string() const;
  string get_header_value(const string &key) const;

  INLINE size_t get_file_size() const;

  void write_headers(ostream &out) const;

private:
  void read_headers();
  void determine_content_length();

  IBioStream *_source;

  string _http_version;
  int _status_code;
  string _status_string;

  typedef pmap<string, string> Headers;
  Headers _headers;

  size_t _file_size;


public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    VirtualFile::init_type();
    register_type(_type_handle, "HTTPDocument",
                  VirtualFile::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class ChunkedStreamBuf;
};

#include "httpDocument.I"

#endif  // HAVE_SSL

#endif


