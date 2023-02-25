/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file screenshotRequest.h
 * @author rdb
 * @date 2022-12-26
 */

#ifndef SCREENSHOTREQUEST_H
#define SCREENSHOTREQUEST_H

#include "pandabase.h"

#include "asyncFuture.h"
#include "clockObject.h"
#include "filename.h"
#include "lightMutex.h"
#include "pmap.h"
#include "texture.h"

/**
 * A class representing an asynchronous request to save a screenshot.
 */
class EXPCL_PANDA_DISPLAY ScreenshotRequest : public AsyncFuture {
public:
  INLINE ScreenshotRequest(Texture *tex);

  INLINE int get_frame_number() const;
  INLINE Texture *get_result() const;

  void set_view_data(int view, const void *ptr);
  void finish();

PUBLISHED:
  void add_output_file(const Filename &filename,
                       const std::string &image_comment = "");

private:
  // It's possible to call save_screenshot multiple times in the same frame, so
  // rather than have to store a vector of request objects, we just allow
  // storing multiple filenames to handle this corner case.
  LightMutex _lock;
  pmap<Filename, std::string> _output_files;
  int _got_num_views = 0;

  int _frame_number = 0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AsyncFuture::init_type();
    register_type(_type_handle, "ScreenshotRequest",
                  AsyncFuture::get_class_type());
    }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "screenshotRequest.I"

#endif
