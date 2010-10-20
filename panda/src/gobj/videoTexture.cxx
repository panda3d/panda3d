// Filename: videoTexture.cxx
// Created by:  drose (21Sep05)
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

#include "pandabase.h"

#include "videoTexture.h"
#include "clockObject.h"
#include "config_gobj.h"

TypeHandle VideoTexture::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VideoTexture::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
VideoTexture::
VideoTexture(const string &name) : 
  Texture(name) 
{
  // We don't want to try to compress each frame as it's loaded.
  _compression = CM_off;

  _video_width = 0;
  _video_height = 0;

  _last_frame_update = 0;
  _current_frame = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: VideoTexture::Copy Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: VideoTexture::get_keep_ram_image
//       Access: Published, Virtual
//  Description: Returns the flag that indicates whether this Texture
//               is eligible to have its main RAM copy of the texture
//               memory dumped when the texture is prepared for
//               rendering.  See set_keep_ram_image().
////////////////////////////////////////////////////////////////////
bool VideoTexture::
get_keep_ram_image() const {
  // A VideoTexture should never dump its RAM image.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: VideoTexture::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool VideoTexture::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: VideoTexture::cull_callback
//       Access: Public, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.
//
//               This is called each time the Texture is discovered
//               applied to a Geom in the traversal.  It should return
//               true if the Geom is visible, false if it should be
//               omitted.
////////////////////////////////////////////////////////////////////
bool VideoTexture::
cull_callback(CullTraverser *, const CullTraverserData &) const {
  // Strictly speaking, the cull_callback() method isn't necessary for
  // VideoTexture, since the get_ram_image() function is already
  // overloaded to update itself if necessary.  However, we define it
  // anyway, to move the update calculation into the cull traversal
  // rather than the draw traversal.
  ((VideoTexture *)this)->reconsider_dirty();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: VideoTexture::set_video_size
//       Access: Protected
//  Description: Should be called by a derived class to set the size
//               of the video when it is loaded.  Assumes the lock is
//               held.
////////////////////////////////////////////////////////////////////
void VideoTexture::
set_video_size(int video_width, int video_height) {
  _video_width = video_width;
  _video_height = video_height;
  do_set_pad_size(max(_x_size - _video_width, 0), 
                  max(_y_size - _video_height, 0),
                  0);
}

////////////////////////////////////////////////////////////////////
//     Function: VideoTexture::do_has_ram_image
//       Access: Protected, Virtual
//  Description: Returns true if the Texture has its image contents
//               available in main RAM, false if it exists only in
//               texture memory or in the prepared GSG context.
////////////////////////////////////////////////////////////////////
bool VideoTexture::
do_has_ram_image() const {
  int this_frame = ClockObject::get_global_clock()->get_frame_count();
  if (this_frame != _last_frame_update) {
    return false;
  }
  return !_ram_images.empty() && !_ram_images[0]._image.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: VideoTexture::reconsider_dirty
//       Access: Protected, Virtual
//  Description: Called by TextureContext to give the Texture a chance
//               to mark itself dirty before rendering, if necessary.
////////////////////////////////////////////////////////////////////
void VideoTexture::
reconsider_dirty() {
  consider_update();
}

////////////////////////////////////////////////////////////////////
//     Function: VideoTexture::do_unlock_and_reload_ram_image
//       Access: Protected, Virtual
//  Description: This is similar to do_reload_ram_image(), except that
//               the lock is released during the actual operation, to
//               allow normal queries into the Texture object to
//               continue during what might be a slow operation.
//
//               In the case of a VideoTexture, this is exactly the
//               same as do_reload_ram_image().
////////////////////////////////////////////////////////////////////
void VideoTexture::
do_unlock_and_reload_ram_image(bool) {
  consider_update();
}

////////////////////////////////////////////////////////////////////
//     Function: VideoTexture::do_reload_ram_image
//       Access: Protected, Virtual
//  Description: Called when the Texture image is required but the ram
//               image is not available, this will reload it from disk
//               or otherwise do whatever is required to make it
//               available, if possible.
////////////////////////////////////////////////////////////////////
void VideoTexture::
do_reload_ram_image(bool) {
  consider_update();
}

////////////////////////////////////////////////////////////////////
//     Function: VideoTexture::do_can_reload
//       Access: Protected, Virtual
//  Description: Returns true if we can safely call
//               do_unlock_and_reload_ram_image() in order to make the
//               image available, or false if we shouldn't do this
//               (because we know from a priori knowledge that it
//               wouldn't work anyway).
////////////////////////////////////////////////////////////////////
bool VideoTexture::
do_can_reload() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: VideoTexture::consider_update
//       Access: Protected, Virtual
//  Description: Calls update_frame() if the current frame has
//               changed.
////////////////////////////////////////////////////////////////////
void VideoTexture::
consider_update() {
  int this_frame = ClockObject::get_global_clock()->get_frame_count();
  if (this_frame != _last_frame_update) {
    int frame = get_frame();
    if (_current_frame != frame) {
      update_frame(frame);
      _current_frame = frame;
    }
    _last_frame_update = this_frame;
  }
}
