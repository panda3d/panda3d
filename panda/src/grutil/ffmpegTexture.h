// Filename: ffmpegTexture.h
// Created by:  zacpavlov (19Aug05)
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

#ifndef FFMPEGTEXTURE_H
#define FFMPEGTEXTURE_H

#include "pandabase.h"
#ifdef HAVE_FFMPEG

#include "videoTexture.h"

extern "C" {
  #include "avcodec.h"
  #include "avformat.h"
}

////////////////////////////////////////////////////////////////////
//       Class : FFMpegTexture
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GRUTIL FFMpegTexture : public VideoTexture {
PUBLISHED:
  FFMpegTexture(const string &name = string());
protected:
  FFMpegTexture(const FFMpegTexture &copy);
PUBLISHED:
  virtual ~FFMpegTexture();

public:
  static PT(Texture) make_texture();

protected:
  virtual PT(Texture) do_make_copy();
  void do_assign(const FFMpegTexture &copy);

  virtual void update_frame(int frame);
  virtual bool do_read_one(const Filename &fullpath, const Filename &alpha_fullpath,
                           int z, int n, int primary_file_num_channels, int alpha_file_channel,
                           const LoaderOptions &options,
                           bool header_only, BamCacheRecord *record);
  virtual bool do_load_one(const PNMImage &pnmimage, const string &name,
                           int z, int n, const LoaderOptions &options);

private:    
  class VideoPage;
  class VideoStream;

  VideoPage &modify_page(int z);
  bool do_reconsider_video_properties(const VideoStream &stream, 
                                      int num_components, int z, 
                                      const LoaderOptions &options);
  void do_update();
    
  class VideoStream {
  public:
    VideoStream();
    VideoStream(const VideoStream &copy);
    ~VideoStream();

    bool read(const Filename &filename);
    void clear();
    INLINE bool is_valid() const;
    INLINE bool is_from_file() const;
    bool get_frame_data(int frame);

  private:
    int read_video_frame(AVPacket *packet);

  public:
    AVCodecContext *_codec_context; 
    AVFormatContext *_format_context; 
    
    int _stream_number;
    AVFrame *_frame;
    AVFrame *_frame_out;

    Filename _filename;
    int _next_frame_number;
    int _image_size_bytes;

  private:
    unsigned char * _raw_data;
    AVCodec *_codec;
  };

  class VideoPage {
  public:
    INLINE VideoPage();
    INLINE VideoPage(const VideoPage &copy);
    INLINE ~VideoPage();

    VideoStream _color, _alpha;
  };

  typedef pvector<VideoPage> Pages;
  Pages _pages;

public:
  static void register_with_read_factory();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    VideoTexture::init_type();
    register_type(_type_handle, "FFMpegTexture",
                  VideoTexture::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "ffmpegTexture.I"



#endif  // HAVE_OPENCV

#endif
