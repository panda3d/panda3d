/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureReloadRequest.cxx
 * @author drose
 * @date 2008-08-12
 */

#include "textureReloadRequest.h"
#include "textureContext.h"

TypeHandle TextureReloadRequest::_type_handle;

/**
 * Performs the task: that is, loads the one model.
 */
AsyncTask::DoneStatus TextureReloadRequest::
do_task() {
  // Don't reload the texture if it doesn't need it.
  if (_texture->was_image_modified(_pgo)) {
    double delay = async_load_delay;
    if (delay != 0.0) {
      Thread::sleep(delay);
    }

    if (_texture->was_image_modified(_pgo)) {
      if (_allow_compressed) {
        _texture->get_ram_image();
      } else {
        _texture->get_uncompressed_ram_image();
      }

      // Now that we've loaded the texture, we should ensure it actually gets
      // prepared--even if it's no longer visible in the frame--or it may
      // become a kind of a leak (if the texture is never rendered again on
      // this GSG, we'll just end up carrying the texture memory in RAM
      // forever, instead of dumping it as soon as it gets prepared).
      _pgo->enqueue_texture(_texture);
    }
  }

  // Don't continue the task; we're done.
  return DS_done;
}
