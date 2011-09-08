// Filename: ffmpegVideoCursor.h
// Created by: jyelon (01Aug2007)
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

#ifndef FFMPEGVIDEOCURSOR_H
#define FFMPEGVIDEOCURSOR_H

#include "pandabase.h"

#ifdef HAVE_FFMPEG

#include "movieVideoCursor.h"
#include "texture.h"
#include "pointerTo.h"
#include "ffmpegVirtualFile.h"

struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct AVPacket;
struct AVFrame;

////////////////////////////////////////////////////////////////////
//       Class : FfmpegVideoCursor
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES FfmpegVideoCursor : public MovieVideoCursor {
protected:
  FfmpegVideoCursor();
  void init_from(FfmpegVideo *src);

PUBLISHED:
  FfmpegVideoCursor(FfmpegVideo *src);
  virtual ~FfmpegVideoCursor();
  
public:
  virtual void fetch_into_texture(double time, Texture *t, int page);
  virtual void fetch_into_buffer(double time, unsigned char *block, bool rgba);

protected:
  bool fetch_packet(double default_time);
  bool fetch_frame(double time);
  void seek(double t);
  void fetch_time(double time);
  void export_frame(unsigned char *data, bool bgra, int bufx);
  void cleanup();
  
  Filename _filename;
  AVPacket *_packet;
  double _packet_time;
  AVFormatContext *_format_ctx;
  AVCodecContext *_video_ctx;
  FfmpegVirtualFile _ffvfile;
  int _video_index;
  double _video_timebase;
  AVFrame *_frame;
  AVFrame *_frame_out;
  int _initial_dts;
  double _min_fseek;
  
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

  virtual void finalize(BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieVideoCursor::init_type();
    register_type(_type_handle, "FfmpegVideoCursor",
                  MovieVideoCursor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class FfmpegVideo;
};

#include "ffmpegVideoCursor.I"

#endif // HAVE_FFMPEG
#endif // FFMPEGVIDEO_H
