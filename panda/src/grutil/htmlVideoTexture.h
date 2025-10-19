/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file htmlVideoTexture.h
 * @author rdb
 * @date 2025-02-24
 */

#ifndef HTMLVIDEOTEXTURE_H
#define HTMLVIDEOTEXTURE_H

#include "pandabase.h"

#ifdef __EMSCRIPTEN__

/**
 * A texture that renders video frames from an HTMLVideoElement.
 * Interface aims to be (nearly) identical to MovieTexture, with some
 * limitations:
 * - No RAM access of video data is possible
 * - No explicit synchronization with audio
 * - No loop count
 */
class EXPCL_PANDA_GRUTIL HTMLVideoTexture : public Texture {
PUBLISHED:
  explicit HTMLVideoTexture(const std::string &name);
  HTMLVideoTexture(const HTMLVideoTexture &copy) = delete;
  virtual ~HTMLVideoTexture();

  double get_video_length() const;
  int get_video_width() const;
  int get_video_height() const;

  void restart();
  void stop();
  void play();
  void set_time(double t);
  double get_time() const;
  void set_loop(bool enable);
  bool get_loop() const;
  void set_play_rate(double play_rate);
  double get_play_rate() const;
  bool is_playing() const;

PUBLISHED:
  MAKE_PROPERTY(video_length, get_video_length);
  MAKE_PROPERTY(video_width, get_video_width);
  MAKE_PROPERTY(video_height, get_video_height);

  MAKE_PROPERTY(time, get_time, set_time);
  MAKE_PROPERTY(loop, get_loop, set_loop);
  MAKE_PROPERTY(play_rate, get_play_rate, set_play_rate);
  MAKE_PROPERTY(playing, is_playing);

public:
  static PT(Texture) make_texture();

  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, const CullTraverserData &data) const;

  void video_frame_callback(int width, int height, double time);

protected:
  bool check_update();

  virtual void do_clear(Texture::CData *cdata);
  virtual bool do_can_reload(const Texture::CData *cdata) const;

  virtual bool do_adjust_this_size(const Texture::CData *cdata,
                                   int &x_size, int &y_size, const std::string &name,
                                   bool for_padding) const;

  virtual bool do_read_one(Texture::CData *cdata,
                           const Filename &fullpath, const Filename &alpha_fullpath,
                           int z, int n, int primary_file_num_channels, int alpha_file_channel,
                           const LoaderOptions &options,
                           bool header_only, BamCacheRecord *record);
  virtual bool do_load_one(Texture::CData *cdata,
                           const PNMImage &pnmimage, const std::string &name,
                           int z, int n, const LoaderOptions &options);
  virtual bool do_load_one(Texture::CData *cdata,
                           const PfmFile &pfm, const std::string &name,
                           int z, int n, const LoaderOptions &options);

private:
  double _last_time = 0.0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Texture::init_type();
    register_type(_type_handle, "HTMLVideoTexture",
                  Texture::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "htmlVideoTexture.I"

#endif  // __EMSCRIPTEN__

#endif
