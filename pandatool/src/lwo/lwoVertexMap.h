// Filename: lwoVertexMap.h
// Created by:  drose (24Apr01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef LWOVERTEXMAP_H
#define LWOVERTEXMAP_H

#include "pandatoolbase.h"

#include "lwoChunk.h"

#include "pta_float.h"

////////////////////////////////////////////////////////////////////
//       Class : LwoVertexMap
// Description : A mapping of floating-point values per integer index.
//               The meaning of these values is determined by the
//               mapping type code and/or its name.
////////////////////////////////////////////////////////////////////
class LwoVertexMap : public LwoChunk {
public:
  bool has_value(int index) const;
  PTA_float get_value(int index) const;

  IffId _map_type;
  int _dimension;
  string _name;

public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(ostream &out, int indent_level = 0) const;

private:
  typedef pmap<int, PTA_float> VMap;
  VMap _vmap;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LwoChunk::init_type();
    register_type(_type_handle, "LwoVertexMap",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif


