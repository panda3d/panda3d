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
#include "texture.h"
#include "pointerTo.h"
#include "movie.h"

////////////////////////////////////////////////////////////////////
//       Class : MovieVideo
// Description : A stream that generates a series of images.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES MovieVideo : public TypedWritableReferenceCount, public Namable {

 PUBLISHED:
  MovieVideo(const string &name, CPT(Movie) source);
  virtual ~MovieVideo();
  INLINE CPT(Movie) get_source() const;
  INLINE int size_x() const;
  INLINE int size_y() const;
  INLINE int get_num_components() const;
  INLINE int length() const;
  INLINE bool aborted() const;
  INLINE double last_start() const;
  INLINE double next_start() const;
  virtual void seek_ahead(double t);
  virtual void fetch_into_texture(Texture *t, int page);
  virtual void fetch_into_texture_alpha(Texture *t, int page, int alpha_src);
  virtual void fetch_into_buffer(unsigned char *block, bool rgba);

 private:
  void allocate_conversion_buffer();
  unsigned char *_conversion_buffer;
  
 protected:
  CPT(Movie) _source;
  bool _aborted;
  double _last_start;
  double _curr_start;
  double _next_start;
  
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

#endif
