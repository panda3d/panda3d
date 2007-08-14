// Filename: ffmpegVideo.h
// Created by: jyelon (01Aug2007)
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

#ifndef FFMPEGVIDEO_H
#define FFMPEGVIDEO_H
#ifdef HAVE_FFMPEG

#include "pandabase.h"
#include "texture.h"
#include "pointerTo.h"

struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct AVPacket;
struct AVFrame;

////////////////////////////////////////////////////////////////////
//       Class : FfmpegVideo
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES FfmpegVideo : public MovieVideo {

 PUBLISHED:
  FfmpegVideo(const Filename &name);
  virtual ~FfmpegVideo();
  virtual PT(MovieVideo) make_copy() const;
  
 public:
  virtual void fetch_into_buffer(double time, unsigned char *block, bool rgba);

 protected:
  INLINE double get_time_correction();
  INLINE void update_time_correction(double diff);
  void fetch_packet(double default_time);
  void fetch_frame();
  void seek(double t);
  void export_frame(unsigned char *data, bool bgra);
  void cleanup();
  
  Filename _filename;
  AVPacket *_packet;
  double _packet_time;
  AVFormatContext *_format_ctx;
  AVCodecContext *_video_ctx;
  AVCodecContext *_audio_ctx;
  int    _video_index;
  double _video_timebase;
  AVFrame *_frame;
  AVFrame *_frame_out;
  int _initial_dts;
  double _min_fseek;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieVideo::init_type();
    register_type(_type_handle, "FfmpegVideo",
                  MovieVideo::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "ffmpegVideo.I"

#endif // HAVE_FFMPEG
#endif // FFMPEGVIDEO_H
