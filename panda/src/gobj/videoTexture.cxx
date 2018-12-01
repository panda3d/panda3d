/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file videoTexture.cxx
 * @author drose
 * @date 2005-09-21
 */

#include "pandabase.h"

#include "videoTexture.h"
#include "clockObject.h"
#include "config_gobj.h"

TypeHandle VideoTexture::_type_handle;

/**
 *
 */
VideoTexture::
VideoTexture(const std::string &name) :
  Texture(name)
{
  // We don't want to try to compress each frame as it's loaded.
  Texture::CDWriter cdata(Texture::_cycler, true);
  cdata->_compression = CM_off;

  _video_width = 0;
  _video_height = 0;

  _last_frame_update = 0;
  _current_frame = -1;
}

/**
 *
 */
VideoTexture::
VideoTexture(const VideoTexture &copy) :
  Texture(copy),
  AnimInterface(copy),
  _video_width(copy._video_width),
  _video_height(copy._video_height),
  _last_frame_update(copy._last_frame_update),
  _current_frame(copy._current_frame)
{
}

/**
 * Returns the flag that indicates whether this Texture is eligible to have
 * its main RAM copy of the texture memory dumped when the texture is prepared
 * for rendering.  See set_keep_ram_image().
 */
bool VideoTexture::
get_keep_ram_image() const {
  // A VideoTexture should never dump its RAM image.
  return true;
}

/**
 * Should be overridden by derived classes to return true if cull_callback()
 * has been defined.  Otherwise, returns false to indicate cull_callback()
 * does not need to be called for this node during the cull traversal.
 */
bool VideoTexture::
has_cull_callback() const {
  return true;
}

/**
 * If has_cull_callback() returns true, this function will be called during
 * the cull traversal to perform any additional operations that should be
 * performed at cull time.
 *
 * This is called each time the Texture is discovered applied to a Geom in the
 * traversal.  It should return true if the Geom is visible, false if it
 * should be omitted.
 */
bool VideoTexture::
cull_callback(CullTraverser *, const CullTraverserData &) const {
  // Strictly speaking, the cull_callback() method isn't necessary for
  // VideoTexture, since the get_ram_image() function is already overloaded to
  // update itself if necessary.  However, we define it anyway, to move the
  // update calculation into the cull traversal rather than the draw
  // traversal.
  ((VideoTexture *)this)->reconsider_dirty();
  return true;
}

/**
 * Should be called by a derived class to set the size of the video when it is
 * loaded.  Assumes the lock is held.
 */
void VideoTexture::
set_video_size(int video_width, int video_height) {
  _video_width = video_width;
  _video_height = video_height;
  set_orig_file_size(video_width, video_height);

  Texture::CDWriter cdata(Texture::_cycler, true);
  do_set_pad_size(cdata,
                  std::max(cdata->_x_size - _video_width, 0),
                  std::max(cdata->_y_size - _video_height, 0),
                  0);
}

/**
 * Returns true if the Texture has its image contents available in main RAM,
 * false if it exists only in texture memory or in the prepared GSG context.
 */
bool VideoTexture::
do_has_ram_image(const Texture::CData *cdata) const {
  int this_frame = ClockObject::get_global_clock()->get_frame_count();
  if (this_frame != _last_frame_update) {
    return false;
  }
  return !cdata->_ram_images.empty() && !cdata->_ram_images[0]._image.empty();
}

/**
 * Called by TextureContext to give the Texture a chance to mark itself dirty
 * before rendering, if necessary.
 */
void VideoTexture::
reconsider_dirty() {
  consider_update();
}


/**
 * If the texture has a ram image already, this acquires the CData write lock
 * and returns it.
 *
 * If the texture lacks a ram image, this performs do_reload_ram_image(), but
 * without holding the lock on this particular Texture object, to avoid
 * holding the lock across what might be a slow operation.  Instead, the
 * reload is performed in a copy of the texture object, and then the lock is
 * acquired and the data is copied in.
 *
 * In any case, the return value is a locked CData object, which must be
 * released with an explicit call to release_write().  The CData object will
 * have a ram image unless for some reason do_reload_ram_image() fails.
 */
Texture::CData *VideoTexture::
unlocked_ensure_ram_image(bool allow_compression) {
  consider_update();

  Thread *current_thread = Thread::get_current_thread();
  Texture::CData *cdata = Texture::_cycler.write_upstream(false, current_thread);
  return cdata;
}

/**
 * Called when the Texture image is required but the ram image is not
 * available, this will reload it from disk or otherwise do whatever is
 * required to make it available, if possible.
 */
void VideoTexture::
do_reload_ram_image(Texture::CData *cdata, bool) {
  consider_update();
}

/**
 * Returns true if we can safely call do_unlock_and_reload_ram_image() in
 * order to make the image available, or false if we shouldn't do this
 * (because we know from a priori knowledge that it wouldn't work anyway).
 */
bool VideoTexture::
do_can_reload(const Texture::CData *cdata) const {
  return true;
}

/**
 * Works like adjust_size, but also considers the texture class.  Movie
 * textures, for instance, always pad outwards, never scale down.
 */
bool VideoTexture::
do_adjust_this_size(const Texture::CData *cdata_tex,
                    int &x_size, int &y_size, const std::string &name,
                    bool for_padding) const {
  AutoTextureScale ats = do_get_auto_texture_scale(cdata_tex);
  if (ats != ATS_none) {
    ats = ATS_pad;
  }

  return adjust_size(x_size, y_size, name, for_padding, ats);
}

/**
 * Calls update_frame() if the current frame has changed.
 */
void VideoTexture::
consider_update() {
  int this_frame = ClockObject::get_global_clock()->get_frame_count();
  if (this_frame != _last_frame_update) {
    int frame = get_frame();
    if (_current_frame != frame) {
      Texture::CDWriter cdata(Texture::_cycler, false);
      do_update_frame(cdata, frame);
      _current_frame = frame;
    }
    _last_frame_update = this_frame;
  }
}
