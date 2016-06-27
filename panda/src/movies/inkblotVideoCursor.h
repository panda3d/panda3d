/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file inkblotVideoCursor.h
 * @author jyelon
 * @date 2007-07-02
 */

#ifndef INKBLOTVIDEOCURSOR_H
#define INKBLOTVIDEOCURSOR_H

#include "pandabase.h"
#include "texture.h"
#include "pointerTo.h"
#include "inkblotVideo.h"
#include "movieVideoCursor.h"

/**
 * A cellular automaton that generates an amusing pattern of swirling colors.
 */
class EXPCL_PANDA_MOVIES InkblotVideoCursor : public MovieVideoCursor {

 PUBLISHED:
  InkblotVideoCursor(InkblotVideo *src);
  virtual ~InkblotVideoCursor();

 public:
  virtual bool set_time(double time, int loop_count);
  virtual PT(Buffer) fetch_buffer();

 protected:
  int _current_frame;
  int _last_frame;
  unsigned char *_cells;
  unsigned char *_cells2;
  int _fps;

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
