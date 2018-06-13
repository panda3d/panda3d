/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file webcamVideoV4L.cxx
 * @author rdb
 * @date 2010-06-11
 */

#include "webcamVideoV4L.h"

#if defined(HAVE_VIDEO4LINUX) && !defined(CPPPARSER)

#include "webcamVideoCursorV4L.h"
#include "dcast.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#ifndef CPPPARSER
#ifndef VIDIOC_ENUM_FRAMESIZES
enum v4l2_frmsizetypes {
  V4L2_FRMSIZE_TYPE_DISCRETE = 1,
  V4L2_FRMSIZE_TYPE_CONTINUOUS = 2,
  V4L2_FRMSIZE_TYPE_STEPWISE = 3,
};

struct v4l2_frmsize_discrete {
  __u32 width;
  __u32 height;
};

struct v4l2_frmsize_stepwise {
  __u32 min_width;
  __u32 max_width;
  __u32 step_width;
  __u32 min_height;
  __u32 max_height;
  __u32 step_height;
};

struct v4l2_frmsizeenum {
  __u32 index;
  __u32 pixel_format;
  __u32 type;
  union {
    struct v4l2_frmsize_discrete discrete;
    struct v4l2_frmsize_stepwise stepwise;
  };
  __u32 reserved[2];
};

#define VIDIOC_ENUM_FRAMESIZES _IOWR('V', 74, struct v4l2_frmsizeenum)
#endif

#ifndef VIDIOC_ENUM_FRAMEINTERVALS
enum v4l2_frmivaltypes {
  V4L2_FRMIVAL_TYPE_DISCRETE = 1,
  V4L2_FRMIVAL_TYPE_CONTINUOUS = 2,
  V4L2_FRMIVAL_TYPE_STEPWISE = 3,
};

struct v4l2_frmival_stepwise {
  struct v4l2_fract min;
  struct v4l2_fract max;
  struct v4l2_fract step;
};

struct v4l2_frmivalenum {
  __u32 index;
  __u32 pixel_format;
  __u32 width;
  __u32 height;
  __u32 type;
  union {
    struct v4l2_fract               discrete;
    struct v4l2_frmival_stepwise    stepwise;
  };
  __u32 reserved[2];
};

#define VIDIOC_ENUM_FRAMEINTERVALS _IOWR('V', 75, struct v4l2_frmivalenum)
#endif
#endif

TypeHandle WebcamVideoV4L::_type_handle;

/**
 *
 */
void WebcamVideoV4L::
add_options_for_size(int fd, const std::string &dev, const char *name, unsigned width, unsigned height, unsigned pixelformat) {
  struct v4l2_frmivalenum frmivalenum;
  for (int k = 0;; k++) {
    memset(&frmivalenum, 0, sizeof frmivalenum);
    frmivalenum.index = k;
    frmivalenum.pixel_format = pixelformat;
    frmivalenum.width = width;
    frmivalenum.height = height;
    if (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmivalenum) == -1) {
      break;
    }
    double fps = 0.0;
    switch (frmivalenum.type) {
    case V4L2_FRMIVAL_TYPE_DISCRETE:
      fps = ((double) frmivalenum.discrete.denominator) / ((double) frmivalenum.discrete.numerator);
      break;

    case V4L2_FRMIVAL_TYPE_CONTINUOUS:
    case V4L2_FRMIVAL_TYPE_STEPWISE:
      {
        // Select the maximum framerate.
        double max_fps =  ((double) frmivalenum.stepwise.max.denominator) / ((double) frmivalenum.stepwise.max.numerator);
        fps = max_fps;
      }
      break;

    default:
      continue;
    }

    // Create a new webcam video object
    PT(WebcamVideoV4L) wc = new WebcamVideoV4L;
    wc->set_name(name);
    wc->_device = dev;
    wc->_size_x = width;
    wc->_size_y = height;
    wc->_fps = fps;
    wc->_pformat = pixelformat;
    wc->_pixel_format = std::string((char*) &pixelformat, 4);

    WebcamVideoV4L::_all_webcams.push_back(DCAST(WebcamVideo, wc));
  }
}

/**
 * Finds all Video4Linux webcams and adds them to the global list
 * _all_webcams.
 */
void find_all_webcams_v4l() {
  struct v4l2_capability cap2;

  vector_string devs;
  GlobPattern pattern ("/dev/video*");
  pattern.match_files(devs);
  for (vector_string::iterator it = devs.begin(); it != devs.end(); ++it) {
    int fd = open(it->c_str(), O_RDWR);
    if (fd != -1) {
      // Check for Video4Linux2 capabilities
      if (ioctl(fd, VIDIOC_QUERYCAP, &cap2) != -1) {
        if ((cap2.capabilities & V4L2_CAP_VIDEO_CAPTURE) &&
            (cap2.capabilities & V4L2_CAP_STREAMING)) {
          struct v4l2_fmtdesc fmt;
          for (int i = 0;; i++) {
            memset(&fmt, 0, sizeof fmt);
            fmt.index = i;
            fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == -1) {
              break;
            }

            // Only accept supported formats.
            switch (fmt.pixelformat) {
#ifdef HAVE_JPEG
            case V4L2_PIX_FMT_MJPEG:
#endif
            case V4L2_PIX_FMT_YUYV:
            case V4L2_PIX_FMT_BGR24:
            case V4L2_PIX_FMT_BGR32:
            case V4L2_PIX_FMT_RGB24:
            case V4L2_PIX_FMT_RGB32:
            case V4L2_PIX_FMT_GREY:
              break;

            default:
              continue;
            }

            struct v4l2_frmsizeenum frmsizeenum;
            for (int j = 0;; j++) {
              memset(&frmsizeenum, 0, sizeof frmsizeenum);
              frmsizeenum.index = j;
              frmsizeenum.pixel_format = fmt.pixelformat;
              if (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsizeenum) == -1) {
                break;
              }

              switch (frmsizeenum.type) {
              case V4L2_FRMSIZE_TYPE_DISCRETE:
                // Easy, add the options with this discrete size.
                WebcamVideoV4L::
                add_options_for_size(fd, *it, (const char *)cap2.card,
                                     frmsizeenum.discrete.width,
                                     frmsizeenum.discrete.height,
                                     fmt.pixelformat);
                break;

              case V4L2_FRMSIZE_TYPE_CONTINUOUS:
                {
                  // Okay, er, we don't have a proper handling of this, so
                  // let's add all powers of two in this range.

                  __u32 width = Texture::up_to_power_2(frmsizeenum.stepwise.min_width);
                  for (; width <= frmsizeenum.stepwise.max_width; width *= 2) {
                    __u32 height = Texture::up_to_power_2(frmsizeenum.stepwise.min_height);
                    for (; height <= frmsizeenum.stepwise.max_height; height *= 2) {
                      WebcamVideoV4L::
                      add_options_for_size(fd, *it, (const char *)cap2.card, width, height, fmt.pixelformat);
                    }
                  }
                }
                break;

              case V4L2_FRMSIZE_TYPE_STEPWISE:
                {
                  __u32 width = Texture::up_to_power_2(frmsizeenum.stepwise.min_width);
                  for (; width <= frmsizeenum.stepwise.max_width; width *= 2) {
                    __u32 height = Texture::up_to_power_2(frmsizeenum.stepwise.min_height);
                    for (; height <= frmsizeenum.stepwise.max_height; height *= 2) {
                      if ((width - frmsizeenum.stepwise.min_width) % frmsizeenum.stepwise.step_width == 0 &&
                          (height - frmsizeenum.stepwise.min_height) % frmsizeenum.stepwise.step_height == 0) {
                        WebcamVideoV4L::
                        add_options_for_size(fd, *it, (const char *)cap2.card, width, height, fmt.pixelformat);
                      }
                    }
                  }
                }
                break;
              }
            }
          }
          continue;
        }
      }
    }
    close(fd);
  }
}

/**
 * Open this video, returning a MovieVideoCursor.
 */
PT(MovieVideoCursor) WebcamVideoV4L::
open() {
  return new WebcamVideoCursorV4L(this);
}

#endif
