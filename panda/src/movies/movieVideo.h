// Filename: movieVideo.h
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

#ifndef MOVIEVIDEO_H
#define MOVIEVIDEO_H

#include "pandabase.h"
#include "namable.h"
#include "pointerTo.h"
#include "typedWritableReferenceCount.h"
class MovieVideoCursor;

////////////////////////////////////////////////////////////////////
//       Class : MovieVideo
// Description : A MovieVideo is actually any source that provides
//               a sequence of video frames.  That could include an
//               AVI file, a digital camera, or an internet TV station.
//
//               The difference between a MovieVideo and a
//               MovieVideoCursor is like the difference between a
//               filename and a file handle.  The MovieVideo just
//               indicates a particular movie.  The MovieVideoCursor
//               is what allows access.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES MovieVideo : public TypedWritableReferenceCount, public Namable {

 PUBLISHED:
  MovieVideo(const string &name = "Blank Video");
  virtual ~MovieVideo();
  virtual PT(MovieVideoCursor) open();
  static PT(MovieVideo) get(const Filename &name);
  INLINE const Filename &get_filename() const;
  
 protected:
  Filename _filename;
  
 public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "MovieVideo",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

 private:
  static TypeHandle _type_handle;
};

#include "movieVideo.I"
#include "movieVideoCursor.h"

#endif
