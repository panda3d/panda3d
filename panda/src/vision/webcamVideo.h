/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file webcamVideo.h
 * @author jyelon
 * @date 2007-11-01
 */

#ifndef WEBCAMVIDEO_H
#define WEBCAMVIDEO_H

#include "movieVideo.h"

/**
 * Allows you to open a webcam or other video capture device as a video
 * stream.
 */
class EXPCL_VISION WebcamVideo : public MovieVideo {

PUBLISHED:
  virtual ~WebcamVideo();

  static int             get_num_options();
  static PT(WebcamVideo) get_option(int n);
  MAKE_SEQ(get_options, get_num_options, get_option);
  MAKE_SEQ_PROPERTY(options, get_num_options, get_option);

  INLINE int get_size_x() const;
  INLINE int get_size_y() const;
  INLINE double get_fps() const;
  INLINE const std::string &get_pixel_format() const;

  virtual PT(MovieVideoCursor) open() = 0;

  INLINE void output(std::ostream &out) const;

public:
  static void find_all_webcams();

protected:
  int _size_x;
  int _size_y;
  double _fps;
  std::string _pixel_format;

  static pvector<PT(WebcamVideo)> _all_webcams;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieVideo::init_type();
    register_type(_type_handle, "WebcamVideo",
                  MovieVideo::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const WebcamVideo &n);

#include "webcamVideo.I"

#endif
