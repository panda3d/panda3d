// Filename: openCVTexture.h
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
class EXPCL_PANDA OpenCVTexture : public VideoTexture {
PUBLISHED:
  OpenCVTexture(const string &name = string());
protected:
  OpenCVTexture(const OpenCVTexture &copy);
PUBLISHED:
  virtual ~OpenCVTexture();

  virtual PT(Texture) make_copy();

  bool from_camera(int camera_index = -1, int z = 0);

  virtual bool read(const Filename &fullpath, int z = 0,
		    int primary_file_num_channels = 0);
  virtual bool read(const Filename &fullpath, const Filename &alpha_fullpath, 
		    int z = 0,
		    int primary_file_num_channels = 0, int alpha_file_channel = 0);
  virtual bool load(const PNMImage &pnmimage, int z = 0);

public:
  static PT(Texture) make_texture();

protected:
  virtual void update_frame(int frame);

private:    
  class VideoPage;
  class VideoStream;

  VideoPage &modify_page(int z);
  bool reconsider_video_properties(const VideoStream &stream, 
				   int num_components, int z);
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
