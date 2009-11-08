// Filename: openCVTexture.h
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

#ifndef OPENCVTEXTURE_H
#define OPENCVTEXTURE_H

#include "pandabase.h"
#ifdef HAVE_OPENCV

#include "videoTexture.h"

#include <cxcore.h>
#include <cv.h>
#include <highgui.h>

////////////////////////////////////////////////////////////////////
//       Class : OpenCVTexture
// Description : A specialization on VideoTexture that takes its input
//               using the CV library, to produce an animated texture,
//               with its source taken from an .avi file or from a
//               camera input.
////////////////////////////////////////////////////////////////////
class EXPCL_VISION OpenCVTexture : public VideoTexture {
PUBLISHED:
  OpenCVTexture(const string &name = string());
protected:
  OpenCVTexture(const OpenCVTexture &copy);
PUBLISHED:
  virtual ~OpenCVTexture();

  bool from_camera(int camera_index = -1, int z = 0,
                   const LoaderOptions &options = LoaderOptions());

public:
  static PT(Texture) make_texture();

protected:
  virtual INLINE void consider_update();
  virtual PT(Texture) do_make_copy();
  void do_assign(const OpenCVTexture &copy);

  virtual void update_frame(int frame);
  virtual void update_frame(int frame, int z);

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
    bool from_camera(int camera_index);
    void clear();
    INLINE bool is_valid() const;
    INLINE bool is_from_file() const;
    const unsigned char *get_frame_data(int frame);

    CvCapture *_capture;
    Filename _filename;
    int _camera_index;
    int _next_frame;
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
    register_type(_type_handle, "OpenCVTexture",
                  VideoTexture::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "openCVTexture.I"

#endif  // HAVE_OPENCV

#endif
