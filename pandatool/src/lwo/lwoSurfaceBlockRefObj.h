// Filename: lwoSurfaceBlockRefObj.h
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

#ifndef LWOSURFACEBLOCKREFOBJ_H
#define LWOSURFACEBLOCKREFOBJ_H

#include "pandatoolbase.h"

#include "lwoChunk.h"

////////////////////////////////////////////////////////////////////
//       Class : LwoSurfaceBlockRefObj
// Description : Specifies a reference object that the texture UV's
//               are to be computed relative to.
////////////////////////////////////////////////////////////////////
class LwoSurfaceBlockRefObj : public LwoChunk {
public:
  string _name;

public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(ostream &out, int indent_level = 0) const;

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
    register_type(_type_handle, "LwoSurfaceBlockRefObj",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif


