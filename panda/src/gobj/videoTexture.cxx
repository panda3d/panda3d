// Filename: videoTexture.cxx
// Created by:  drose (21Sep05)
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
  _power_2 = (textures_power_2 != ATS_none);
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
  _power_2(copy._power_2),
  _video_width(copy._video_width),
  _video_height(copy._video_height),
  _last_frame_update(copy._last_frame_update),
  _current_frame(copy._current_frame)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VideoTexture::has_ram_image
//       Access: Published, Virtual
//  Description: Returns true if the Texture has its image contents
//               available in main RAM, false if it exists only in
//               texture memory or in the prepared GSG context.
////////////////////////////////////////////////////////////////////
bool VideoTexture::
has_ram_image() const {
  int this_frame = ClockObject::get_global_clock()->get_frame_count();
  if (this_frame != _last_frame_update) {
    return false;
  }
  return !_image.empty();
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
  // An VideoTexture should never dump its RAM image.
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
//     Function: VideoTexture::reload_ram_image
//       Access: Protected, Virtual
//  Description: Called when the Texture image is required but the ram
//               image is not available, this will reload it from disk
//               or otherwise do whatever is required to make it
//               available, if possible.
////////////////////////////////////////////////////////////////////
void VideoTexture::
reload_ram_image() {
  consider_update();
}

