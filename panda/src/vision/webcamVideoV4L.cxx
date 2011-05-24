// Filename: webcamVideoV4L.cxx
// Created by: rdb (11Jun2010)
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

#include "webcamVideoV4L.h"

#ifdef HAVE_VIDEO4LINUX

#include "webcamVideoCursorV4L.h"
#include "dcast.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

TypeHandle WebcamVideoV4L::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: find_all_webcams_v4l
//       Access: Public, Static
//  Description: Finds all Video4Linux webcams and adds them to
//               the global list _all_webcams.
////////////////////////////////////////////////////////////////////
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
            struct v4l2_frmsizeenum frmsizeenum;
            for (int j = 0;; j++) {
              memset(&frmsizeenum, 0, sizeof frmsizeenum);
              frmsizeenum.index = j;
              frmsizeenum.pixel_format = fmt.pixelformat;
              if (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsizeenum) == -1) {
                break;
              }
              if (frmsizeenum.type != V4L2_FRMSIZE_TYPE_DISCRETE) {
                continue;
              }
              struct v4l2_frmivalenum frmivalenum;
              for (int k = 0;; k++) {
                memset(&frmivalenum, 0, sizeof frmivalenum);
                frmivalenum.index = k;
                frmivalenum.pixel_format = fmt.pixelformat;
                frmivalenum.width = frmsizeenum.discrete.width;
                frmivalenum.height = frmsizeenum.discrete.height;
                if (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmivalenum) == -1) {
                  break;
                }
                if (frmivalenum.type != V4L2_FRMIVAL_TYPE_DISCRETE) {
                  continue;
                }

                // Create a new webcam video object
                PT(WebcamVideoV4L) wc = new WebcamVideoV4L;
                wc->set_name((const char*) cap2.card);
                wc->_device = *it;
                wc->_size_x = frmsizeenum.discrete.width;
                wc->_size_y = frmsizeenum.discrete.height;
                wc->_fps = ((double) frmivalenum.discrete.denominator) / ((double) frmivalenum.discrete.numerator);
                wc->_pformats.push_back(fmt.pixelformat);

                // Iterate through the webcams to make sure we don't put any duplicates in there
                pvector<PT(WebcamVideo)>::iterator wvi;
                for (wvi = WebcamVideoV4L::_all_webcams.begin(); wvi != WebcamVideoV4L::_all_webcams.end(); ++wvi) {
                  if ((*wvi)->is_of_type(WebcamVideoV4L::get_class_type())) {
                    PT(WebcamVideoV4L) wv_v4l = DCAST(WebcamVideoV4L, *wvi);
                    if (wv_v4l->_device == wc->_device &&
                        wv_v4l->_size_x == wc->_size_x &&
                        wv_v4l->_size_y == wc->_size_y &&
                        wv_v4l->_fps == wc->_fps) {
                      wv_v4l->_pformats.push_back(fmt.pixelformat);
                      break;
                    }
                  }
                }
                // Did the loop finish, meaning that a webcam of these
                // properties does not exist? Add it.
                if (wvi == WebcamVideoV4L::_all_webcams.end()) {
                  WebcamVideoV4L::_all_webcams.push_back(DCAST(WebcamVideo, wc));
                }
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

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideoV4L::open
//       Access: Published, Virtual
//  Description: Open this video, returning a MovieVideoCursor.
////////////////////////////////////////////////////////////////////
PT(MovieVideoCursor) WebcamVideoV4L::
open() {
  return new WebcamVideoCursorV4L(this);
}

#endif
