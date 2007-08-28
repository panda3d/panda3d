// Filename: inkblotVideoCursor.h
// Created by: jyelon (02Jul07)
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

#ifndef INKBLOTVIDEOCURSOR_H
#define INKBLOTVIDEOCURSOR_H

#include "pandabase.h"
#include "texture.h"
#include "pointerTo.h"
#include "inkblotVideo.h"

////////////////////////////////////////////////////////////////////
//       Class : InkblotVideoCursor
// Description : A cellular automaton that generates an amusing
//               pattern of swirling colors.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES InkblotVideoCursor : public MovieVideoCursor {

 PUBLISHED:
  InkblotVideoCursor(InkblotVideo *src);
  virtual ~InkblotVideoCursor();
  
 public:
  virtual void fetch_into_buffer(double time, unsigned char *block, bool rgba);
  
 protected:
  unsigned char *_cells;
  unsigned char *_cells2;
  int _fps;
  int _frames_read;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieVideoCursor::init_type();
    register_type(_type_handle, "InkblotVideoCursor",
                  MovieVideoCursor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "inkblotVideoCursor.I"

#endif
