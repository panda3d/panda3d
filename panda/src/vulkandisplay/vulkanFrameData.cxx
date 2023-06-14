/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanFrameData.cxx
 * @author rdb
 * @date 2023-06-14
 */

#include "vulkanFrameData.h"

/**
 *
 */
void VulkanFrameData::
finish_downloads(VkDevice device) {
  for (QueuedDownload &down : _download_queue) {
    PTA_uchar target = down._texture->modify_ram_image();
    size_t view_size = down._texture->get_ram_view_size();

    if (auto data = down._block.map()) {
      // The texture is upside down, so invert it.
      size_t row_size = down._texture->get_x_size()
                      * down._texture->get_num_components()
                      * down._texture->get_component_width();
      unsigned char *dst = target.p() + view_size * (down._view + 1) - row_size;
      unsigned char *src = (unsigned char *)data;
      unsigned char *src_end = src + view_size;
      while (src < src_end) {
        memcpy(dst, src, row_size);
        src += row_size;
        dst -= row_size;
      }
    } else {
      vulkandisplay_cat.error()
        << "Failed to map memory for RAM transfer.\n";
    }

    // We won't need this buffer any more.
    vkDestroyBuffer(device, down._buffer, nullptr);

    if (down._request != nullptr) {
      down._request->finish();
    }
  }
  _download_queue.clear();
  _wait_for_finish = false;
}
