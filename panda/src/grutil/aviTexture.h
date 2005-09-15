// Filename: aviTexture.h
// Created by:  zacpavlov (19Aug05)
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

#ifndef AVITEXTURE_H
#define AVITEXTURE_H

#include "pandabase.h"
#ifdef HAVE_CV

#include "texture.h"
#include <cxcore.h>
#include <cv.h>
#include <highgui.h>

////////////////////////////////////////////////////////////////////
//       Class : AviTexture
// Description : A specialization on Texture that takes its input
//               using the CV library, to produce an animated texture,
//               with its source taken from an .avi file or from a
//               camera input.
//
//               Presently, it is necessary for the application to
//               call update() periodically to advance to the next
//               frame.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AviTexture : public Texture {
PUBLISHED:
  AviTexture();
  AviTexture(const string &filename);
  ~AviTexture();

  INLINE void set_time(float t);
  INLINE float get_time();
  INLINE float get_fps();
  INLINE void set_fps(float fps);
  INLINE int get_total_frames();
  INLINE int get_current_frame();
  INLINE void set_current_frame(int frame);
  bool update();

private:    
  void gen_tex(int magicNum);
  INLINE bool obtain_ram();

  int _magicNum;
  PTA_uchar _buf;
  CvCapture * _capture;
  float _time;
  bool _isCamera;
  float _fps;
  int _total_frames;
  int _current_frame;
  int _width;
  int _height;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Texture::init_type();
    register_type(_type_handle, "AviTexture",
                  Texture::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;
};

#include "aviTexture.I"

#endif  // HAVE_CV

#endif
