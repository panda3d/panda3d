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

// This symbol is predefined by the Panda3D build system to select
// whether we are using the OpenCV 2.3 or later interface, or if it is
// not defined, we are using the original interface.
#ifdef OPENCV_VER_23

#include <opencv2/core/core.hpp>
//#include <opencv2/video/video.hpp>
#include <opencv2/highgui/highgui.hpp>

#else
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#endif  // OPENCV_VER_23

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
                   int alpha_file_channel = 0,
                   const LoaderOptions &options = LoaderOptions());

public:
  static PT(Texture) make_texture();

protected:
  virtual void consider_update();
  virtual PT(Texture) make_copy_impl();
  void do_assign(Texture::CData *cdata_tex, const OpenCVTexture *copy, 
                 const Texture::CData *cdata_copy_tex);

  virtual void do_update_frame(Texture::CData *cdata_tex, int frame);
  virtual void do_update_frame(Texture::CData *cdata_tex, int frame, int z);

  virtual bool do_read_one(Texture::CData *cdata,
                           const Filename &fullpath, const Filename &alpha_fullpath,
                           int z, int n, int primary_file_num_channels, int alpha_file_channel,
                           const LoaderOptions &options,
                           bool header_only, BamCacheRecord *record);
  virtual bool do_load_one(Texture::CData *cdata,
                           const PNMImage &pnmimage, const string &name,
                           int z, int n, const LoaderOptions &options);

private:    
  class VideoPage;
  class VideoStream;

  VideoPage &do_modify_page(const Texture::CData *cdata, int z);
  bool do_reconsider_video_properties(Texture::CData *cdata, 
                                      const VideoStream &stream, 
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
    bool get_frame_data(int frame,
                        const unsigned char *&r,
                        const unsigned char *&g,
                        const unsigned char *&b,
                        int &x_pitch, int &y_pitch);

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
