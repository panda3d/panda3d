// Filename: lwoHeader.h
// Created by:  drose (24Apr01)
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

#ifndef LWOHEADER_H
#define LWOHEADER_H

#include "pandatoolbase.h"

#include "lwoGroupChunk.h"

////////////////////////////////////////////////////////////////////
//       Class : LwoHeader
// Description : The first chunk in a Lightwave Object file.
////////////////////////////////////////////////////////////////////
class LwoHeader : public LwoGroupChunk {
public:
  LwoHeader();

  IffId _lwid;

  INLINE bool is_valid() const;
  INLINE double get_version() const;

public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(ostream &out, int indent_level = 0) const;

private:
  bool _valid;
  double _version;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LwoGroupChunk::init_type();
    register_type(_type_handle, "LwoHeader",
                  LwoGroupChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "lwoHeader.I"

#endif


