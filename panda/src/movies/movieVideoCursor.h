// Filename: movieVideoCursor.h
// Created by: jyelon (02Jul07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef MOVIEVIDEOCURSOR_H
#define MOVIEVIDEOCURSOR_H

#include "pandabase.h"
#include "texture.h"
#include "pointerTo.h"
class MovieVideo;

////////////////////////////////////////////////////////////////////
//       Class : MovieVideoCursor
// Description : A MovieVideo is actually any source that provides
//               a sequence of video frames.  That could include an
//               AVI file, a digital camera, or an internet TV station.
//               A MovieVideoCursor is a handle that lets you read
//               data sequentially from a MovieVideo.
//
//               Thread safety: each individual MovieVideoCursor
//               must be owned and accessed by a single thread.
//               It is OK for two different threads to open
//               the same file at the same time, as long as they
//               use separate MovieVideoCursor objects.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES MovieVideoCursor : public TypedWritableReferenceCount {

 PUBLISHED:
  MovieVideoCursor(MovieVideo *src);
  virtual ~MovieVideoCursor();
  PT(MovieVideo) get_source() const;
  INLINE int size_x() const;
  INLINE int size_y() const;
  INLINE int get_num_components() const;
  INLINE double length() const;
  INLINE bool can_seek() const;
  INLINE bool can_seek_fast() const;
  INLINE bool aborted() const;
  INLINE double last_start() const;
  INLINE double next_start() const;
  INLINE bool ready() const;
  INLINE bool streaming() const;
  void setup_texture(Texture *tex) const;
  virtual void fetch_into_bitbucket(double time);
  virtual void fetch_into_texture(double time, Texture *t, int page);
  virtual void fetch_into_texture_rgb(double time, Texture *t, int page);
  virtual void fetch_into_texture_alpha(double time, Texture *t, int page, int alpha_src);

 public:
  virtual void fetch_into_buffer(double time, unsigned char *block, bool bgra);
  
 private:
  void allocate_conversion_buffer();
  unsigned char *_conversion_buffer;
  
 protected:
  PT(MovieVideo) _source;
  int _size_x;
  int _size_y;
  int _num_components;
  double _length;
  bool _can_seek;
  bool _can_seek_fast;
  bool _aborted;
  double _last_start;
  double _next_start;
  bool _streaming;
  bool _ready;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "MovieVideoCursor",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "movieVideoCursor.I"
#include "movieVideo.h"

#endif
