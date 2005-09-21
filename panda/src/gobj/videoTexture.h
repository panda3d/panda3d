// Filename: videoTexture.h
// Created by:  drose (21Sep05)
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

#ifndef VIDEOTEXTURE_H
#define VIDEOTEXTURE_H

#include "pandabase.h"
#include "texture.h"
#include "animInterface.h"

////////////////////////////////////////////////////////////////////
//       Class : VideoTexture
// Description : The base class for a family of animated Textures that
//               take their input from a video source, such as a movie
//               file.  These Textures may be stopped, started,
//               etc. using the AnimInterface controls, similar to an
//               animated character.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA VideoTexture : public Texture, public AnimInterface {
protected:
  VideoTexture(const string &name);
  VideoTexture(const VideoTexture &copy);

PUBLISHED:
  INLINE void set_power_2(bool power_2);
  INLINE bool get_power_2() const;

  virtual bool has_ram_image() const;
  virtual bool get_keep_ram_image() const;

  INLINE int get_video_width() const;
  INLINE int get_video_height() const;
  INLINE LVecBase2f get_tex_scale() const;

public:
  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, const CullTraverserData &data) const;

protected:
  INLINE void set_video_size(int video_width, int video_height);

  virtual void reconsider_dirty();
  virtual void reload_ram_image();

  INLINE void consider_update();
  INLINE void clear_current_frame();
  virtual void update_frame(int frame)=0;

private:
  bool _power_2;
  int _video_width;
  int _video_height;
  int _last_frame_update;
  int _current_frame;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Texture::init_type();
    AnimInterface::init_type();
    register_type(_type_handle, "VideoTexture",
                  Texture::get_class_type(),
		  AnimInterface::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "videoTexture.I"

#endif
