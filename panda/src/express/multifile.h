// Filename: multifile.h
// Created by:  mike (09Jan97)
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
#ifndef MULTIFILE_H
#define MULTIFILE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "typedef.h"
#include "datagram.h"
#include "datagramIterator.h"

#include <notify.h>
#include <filename.h>
#include "plist.h"

////////////////////////////////////////////////////////////////////
//       Class : Multifile
// Description : A file that contains a set of files.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Multifile {
PUBLISHED:
  enum Type {
    T_unknown,
    T_valid,
    T_invalid,
  };

  Multifile(void);
  ~Multifile(void);

  INLINE int get_header_length(void) const;

  static int evaluate(const char *start, int size);

  bool add(const Filename &name);
  bool remove(const Filename &name);
  bool has_file(const Filename &name);

  bool read(Filename &name);
  bool write(Filename name);
  int write(char *&start, int &size, const Filename &rel_path = "");
  bool write_extract(char *&start, int &size, const Filename &rel_path = "");
  bool extract(const Filename &name, const Filename &rel_path = "");
  void extract_all(const Filename &rel_path = "");

  void reset(void);
  int parse_header(char *&start, int &size);

private:

  INLINE void write_header(ofstream &write_stream);

public:
  // This nested class must be public so we can make a list of its
  // pointers.  Weird.
  class Memfile {
  public:
    Memfile(void);
    ~Memfile(void);
    bool parse_header_length(char *&start, int &size);
    bool parse_header(char *&start, int &size);

    bool read(const Filename &name);
    bool read_from_multifile(ifstream &read_stream);
    bool write(const Filename &rel_path);
    void write_to_multifile(ofstream &write_stream);
    int write(char *&start, int &size, const Filename &rel_path = "");
    void reset(void);

  public:
    bool _header_length_parsed;
    bool _header_parsed;
    Datagram _datagram;
    char *_header_length_buf;
    int _header_length_buf_length;

    PN_int32 _header_length;
    Filename _name;
    PN_int32 _buffer_length;
    char* _buffer;

    bool _file_open;
    ofstream _write_stream;
    int _bytes_written;
  };

private:
  typedef plist<Memfile *> MemfileList;

  class MemfileMatch {
  public:
    MemfileMatch(const Filename &name) {
      _want_name = name;
    }
    bool operator()(Memfile *mfile) const {
      return mfile->_name == _want_name;
    }
    Filename _want_name;
  };

private:
  MemfileList _files;
  PN_int32 _num_mfiles;
  bool _header_parsed;
  Memfile *_current_mfile;
  Datagram _datagram;
  int _header_length;
  int _mfiles_written;

  static PN_uint32 _magic_number;
};

#include "multifile.I"

#endif
