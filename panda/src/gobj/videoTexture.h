/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file videoTexture.h
 * @author drose
 * @date 2005-09-21
 */

#ifndef VIDEOTEXTURE_H
#define VIDEOTEXTURE_H

#include "pandabase.h"
#include "texture.h"
#include "animInterface.h"
#include "clockObject.h"

/**
 * The base class for a family of animated Textures that take their input from
 * a video source, such as a movie file.  These Textures may be stopped,
 * started, etc.  using the AnimInterface controls, similar to an animated
 * character.
 */
class EXPCL_PANDA_GOBJ VideoTexture : public Texture, public AnimInterface {
protected:
  VideoTexture(const std::string &name);
  VideoTexture(const VideoTexture &copy);

PUBLISHED:
  virtual bool get_keep_ram_image() const;

  INLINE int get_video_width() const;
  INLINE int get_video_height() const;
  MAKE_PROPERTY(video_width, get_video_width);
  MAKE_PROPERTY(video_height, get_video_height);

public:
  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, const CullTraverserData &data) const;

protected:
  void set_video_size(int video_width, int video_height);

  virtual bool do_has_ram_image(const Texture::CData *cdata) const;

  virtual void reconsider_dirty();
  virtual Texture::CData *unlocked_ensure_ram_image(bool allow_compression);
  virtual void do_reload_ram_image(Texture::CData *cdata, bool allow_compression);
  virtual bool do_can_reload(const Texture::CData *cdata) const;

  virtual bool do_adjust_this_size(const Texture::CData *cdata,
                                   int &x_size, int &y_size, const std::string &name,
                                   bool for_padding) const;

  virtual void consider_update();
  INLINE void clear_current_frame();
  virtual void do_update_frame(Texture::CData *cdata_tex, int frame)=0;

protected:
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
