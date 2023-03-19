/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file screenshotRequest.cxx
 * @author rdb
 * @date 2022-12-26
 */

#include "screenshotRequest.h"
#include "lightMutexHolder.h"
#include "pnmImage.h"
#include "texture.h"

TypeHandle ScreenshotRequest::_type_handle;

/**
 *
 */
void ScreenshotRequest::
set_view_data(int view, const void *ptr) {
  const int z = 0;

  Texture *tex = get_result();
  PTA_uchar new_image = tex->modify_ram_image();
  unsigned char *image_ptr = new_image.p();
  size_t image_size;
  if (z >= 0 || view > 0) {
    image_size = tex->get_expected_ram_page_size();
    if (z >= 0) {
      image_ptr += z * image_size;
    }
    if (view > 0) {
      image_ptr += (view * tex->get_z_size()) * image_size;
      nassertd(view < tex->get_num_views()) {
        if (set_future_state(FS_cancelled)) {
          notify_done(false);
        }
        return;
      }
    }
  } else {
    image_size = tex->get_ram_image_size();
  }
  memcpy(image_ptr, ptr, image_size);
}

/**
 *
 */
void ScreenshotRequest::
finish() {
  Texture *tex = get_result();

  ++_got_num_views;
  if (_got_num_views < tex->get_num_views()) {
    return;
  }

  {
    LightMutexHolder holder(_lock);
    if (!_output_files.empty()) {
      PNMImage image;
      tex->store(image);

      for (const auto &item : _output_files) {
        image.set_comment(item.second);
        image.write(item.first);
      }
    }

    AsyncFuture::set_result(tex);
    _output_files.clear();

    if (!set_future_state(FS_finished)) {
      return;
    }
  }

  notify_done(true);
}

/**
 * Adds a filename to write the screenshot to when it is available.  If the
 * request is already done, performs the write synchronously.
 */
void ScreenshotRequest::
add_output_file(const Filename &filename, const std::string &image_comment) {
  if (!done()) {
    LightMutexHolder holder(_lock);
    if (!done()) {
      _output_files[filename] = image_comment;
      return;
    }
  }
  // Was already done, write it right away.
  Texture *tex = get_result();
  PNMImage image;
  tex->store(image);
  image.set_comment(image_comment);
  image.write(filename);
}
