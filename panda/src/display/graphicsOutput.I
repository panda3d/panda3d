// Filename: graphicsOutput.I
// Created by:  drose (06Feb04)
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


////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_gsg
//       Access: Published
//  Description: Returns the GSG that is associated with this window.
//               There is a one-to-one association between windows and
//               GSG's.
//
//               This may return NULL if the graphics context has not
//               yet been created for the window, e.g. before the
//               first frame has rendered; or after the window has
//               been closed.
////////////////////////////////////////////////////////////////////
INLINE GraphicsStateGuardian *GraphicsOutput::
get_gsg() const {
  return _gsg;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_pipe
//       Access: Published
//  Description: Returns the GraphicsPipe that this window is
//               associated with.  It is possible that the
//               GraphicsPipe might have been deleted while an
//               outstanding PT(GraphicsOutput) prevented all of its
//               children windows from also being deleted; in this
//               unlikely case, get_pipe() may return NULL.
////////////////////////////////////////////////////////////////////
INLINE GraphicsPipe *GraphicsOutput::
get_pipe() const {
  return _pipe;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_engine
//       Access: Published
//  Description: Returns the graphics engine that created this output.
//               Since there is normally only one GraphicsEngine
//               object in an application, this is usually the same as
//               the global GraphicsEngine.
////////////////////////////////////////////////////////////////////
INLINE GraphicsEngine *GraphicsOutput::
get_engine() const {
  return _engine;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_name
//       Access: Published
//  Description: Returns the name that was passed to the
//               GraphicsOutput constructor.
////////////////////////////////////////////////////////////////////
INLINE const string &GraphicsOutput::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::count_textures
//       Access: Published
//  Description: If the GraphicsOutput is set to render into a
//               texture, returns the number of textures that are
//               being rendered into.  Normally, the textures would
//               be associated with different buffers - a color
//               texture, a depth texture, and a stencil texture.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsOutput::
count_textures() const {
  return _textures.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::has_texture
//       Access: Published
//  Description: Returns true if the GraphicsOutput is rendering 
//               into any textures at all.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
has_texture() const {
  return (_textures.size() > 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_texture
//       Access: Published
//  Description: Returns the nth texture into which the GraphicsOutput
//               renders.  Returns NULL if there is no such texture.
//
//               If the texture is non-NULL, it may be applied to
//               geometry to be rendered for any other windows or
//               outputs that share the same GSG as this
//               GraphicsOutput.  The effect is undefined for windows
//               that share a different GSG; usually in these cases
//               the texture will be invalid.
////////////////////////////////////////////////////////////////////
INLINE Texture *GraphicsOutput::
get_texture(int i) const {
  if ((i < 0) || (i >= ((int)_textures.size()))) {
    return (Texture *)NULL;
  }
  return _textures[i]._texture;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_texture_plane
//       Access: Published
//  Description: Returns the RenderTexturePlane associated with the
//               nth render-texture.  Returns 0 if there is no such
//               texture.
////////////////////////////////////////////////////////////////////
INLINE GraphicsOutput::RenderTexturePlane GraphicsOutput::
get_texture_plane(int i) const {
  if ((i < 0) || (i >= ((int)_textures.size()))) {
    return (RenderTexturePlane)0;
  }
  return _textures[i]._plane;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_rtm_mode
//       Access: Published
//  Description: Returns the RenderTextureMode associated with the
//               nth render-texture.  Returns RTM_none if there is
//               no such texture.
////////////////////////////////////////////////////////////////////
INLINE GraphicsOutput::RenderTextureMode GraphicsOutput::
get_rtm_mode(int i) const {
  if ((i < 0) || (i >= ((int)_textures.size()))) {
    return RTM_none;
  }
  return _textures[i]._rtm_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_x_size
//       Access: Published
//  Description: Returns the visible width of the window or buffer, if
//               it is known.  In certain cases (e.g. fullscreen
//               windows), the size may not be known until after the
//               object has been fully created.  Check has_size()
//               first.
//
//               Certain objects (like windows) may change size
//               spontaneously; this method is not thread-safe.  To
//               get the size of a window in a thread-safe manner,
//               query get_properties().
////////////////////////////////////////////////////////////////////
INLINE int GraphicsOutput::
get_x_size() const {
  return _x_size;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_y_size
//       Access: Published
//  Description: Returns the visible height of the window or buffer,
//               if it is known.  In certain cases (e.g. fullscreen
//               windows), the size may not be known until after the
//               object has been fully created.  Check has_size()
//               first.
//
//               Certain objects (like windows) may change size
//               spontaneously; this method is not thread-safe.  To
//               get the size of a window in a thread-safe manner,
//               query get_properties().
////////////////////////////////////////////////////////////////////
INLINE int GraphicsOutput::
get_y_size() const {
  return _y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_fb_x_size
//       Access: Published
//  Description: Returns the internal width of the window or buffer.
//               This is almost always the same as get_x_size(),
//               except when a pixel_zoom is in effect--see
//               set_pixel_zoom().
////////////////////////////////////////////////////////////////////
INLINE int GraphicsOutput::
get_fb_x_size() const {
  return max(int(_x_size * get_pixel_factor()), 1);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_fb_y_size
//       Access: Published
//  Description: Returns the internal height of the window or buffer.
//               This is almost always the same as get_y_size(),
//               except when a pixel_zoom is in effect--see
//               set_pixel_zoom().
////////////////////////////////////////////////////////////////////
INLINE int GraphicsOutput::
get_fb_y_size() const {
  return max(int(_y_size * get_pixel_factor()), 1);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::has_size
//       Access: Published
//  Description: Returns true if the size of the window/frame buffer
//               is known, false otherwise.  In certain cases the size
//               may not be known until after the object has been
//               fully created.  Also, certain objects (like windows)
//               may change size spontaneously.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
has_size() const {
  return _has_size;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::is_valid
//       Access: Published
//  Description: Returns true if the output is fully created and ready
//               for rendering, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
is_valid() const {
  return _is_valid;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::set_one_shot
//       Access: Published
//  Description: Changes the current setting of the one-shot flag.
//               When this is true, the GraphicsOutput will render one
//               frame and then automatically set itself inactive.
//               This is particularly useful for buffers that are
//               created for the purposes of render-to-texture, for
//               static textures that don't need to be continually
//               re-rendered once they have been rendered the first
//               time.
//
//               Setting the buffer inactive is not the same thing as
//               destroying it.  You are still responsible for passing
//               this buffer to GraphicsEngine::remove_window() when
//               you no longer need the texture, in order to clean up
//               fully.  (However, you should not call remove_window()
//               on this buffer while the texture is still needed,
//               because depending on the render-to-texture mechanism
//               in use, this may invalidate the texture contents.)
////////////////////////////////////////////////////////////////////
INLINE void GraphicsOutput::
set_one_shot(bool one_shot) {
  _one_shot = one_shot;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_one_shot
//       Access: Published
//  Description: Returns the current setting of the one-shot flag.
//               When this is true, the GraphicsOutput will
//               automatically detach its texture (if it has one) and
//               remove itself from the GraphicsEngine after it
//               renders the next frame.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
get_one_shot() const {
  return _one_shot;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_inverted
//       Access: Published
//  Description: Returns the current setting of the inverted flag.
//               When this is true, the scene is rendered into the
//               window upside-down, flipped like a mirror along the X
//               axis.  See set_inverted().
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
get_inverted() const {
  return _inverted;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::set_red_blue_stereo
//       Access: Published
//  Description: Enables red-blue stereo mode on this particular
//               window.  When red-blue stereo mode is in effect,
//               DisplayRegions that have the "left" channel set will
//               render in the red (or specified) channel only, while
//               DisplayRegions that have the "right" channel set will
//               render in the blue (or specified) channel only.
//
//               The remaining two parameters specify the particular
//               color channel(s) to associate with each eye.  Use the
//               bits defined in ColorWriteAttrib::Channels.
//
//               This can be used to achieve a cheesy stereo mode in
//               the absence of hardware-supported stereo.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsOutput::
set_red_blue_stereo(bool red_blue_stereo,
                    unsigned int left_eye_color_mask,
                    unsigned int right_eye_color_mask) {
  _red_blue_stereo = red_blue_stereo;
  if (_red_blue_stereo) {
    _left_eye_color_mask = left_eye_color_mask;
    _right_eye_color_mask = right_eye_color_mask;
  } else {
    _left_eye_color_mask = 0x0f;
    _right_eye_color_mask = 0x0f;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_red_blue_stereo
//       Access: Published
//  Description: Returns whether red-blue stereo mode is in effect for
//               this particular window.  See set_red_blue_stereo().
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
get_red_blue_stereo() const {
  return _red_blue_stereo;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_left_eye_color_mask
//       Access: Published
//  Description: Returns the color mask in effect when rendering a
//               left-eye view in red_blue stereo mode.  This is one
//               or more bits defined in ColorWriteAttrib::Channels.
//               See set_red_blue_stereo().
////////////////////////////////////////////////////////////////////
INLINE unsigned int GraphicsOutput::
get_left_eye_color_mask() const {
  return _left_eye_color_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_right_eye_color_mask
//       Access: Published
//  Description: Returns the color mask in effect when rendering a
//               right-eye view in red_blue stereo mode.  This is one
//               or more bits defined in ColorWriteAttrib::Channels.
//               See set_red_blue_stereo().
////////////////////////////////////////////////////////////////////
INLINE unsigned int GraphicsOutput::
get_right_eye_color_mask() const {
  return _right_eye_color_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_fb_properties
//       Access: Published
//  Description: Returns the framebuffer properties of the window.
////////////////////////////////////////////////////////////////////
INLINE const FrameBufferProperties &GraphicsOutput::
get_fb_properties() const {
  return _fb_properties;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::is_stereo
//       Access: Published
//  Description: Returns Returns true if this window can render stereo
//               DisplayRegions, either through red-blue stereo (see
//               set_red_blue_stereo()) or through true hardware
//               stereo rendering.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
is_stereo() const {
  return _red_blue_stereo || _fb_properties.is_stereo();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::clear_delete_flag
//       Access: Published
//  Description: Resets the delete flag, so the GraphicsOutput will
//               not be automatically deleted before the beginning of
//               the next frame.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsOutput::
clear_delete_flag() {
  _delete_flag = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_delete_flag
//       Access: Published
//  Description: Returns the current setting of the delete flag.  When
//               this is true, the GraphicsOutput will automatically
//               be removed before the beginning of the next frame by
//               the GraphicsEngine.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
get_delete_flag() const {
  // We only delete the window or buffer automatically when it is
  // no longer associated with a texture.
  for (int i=0; i<(int)_hold_textures.size(); i++) {
    if (_hold_textures[i].is_valid_pointer()) {
      return false;
    }
  }
  return _delete_flag;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_sort
//       Access: Published
//  Description: Returns the sorting order of this particular
//               GraphicsOutput.  The various GraphicsOutputs within a
//               particular thread will be rendered in the indicated
//               order.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsOutput::
get_sort() const {
  return _sort;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::set_child_sort
//       Access: Published
//  Description: Specifies the sort value of future offscreen buffers
//               created by make_texture_sort().
//
//               The purpose of this method is to allow the user to
//               limit the sort value chosen for a buffer created via
//               make_texture_buffer().  Normally, this buffer will be
//               assigned a value of get_sort() - 1, so that it
//               will be rendered before this window is rendered; but
//               sometimes this isn't sufficiently early, especially
//               if other buffers also have a view into the same
//               scene.
//
//               If you specify a value here, then new buffers created
//               via make_texture_buffer() will be given that sort
//               value instead of get_sort() - 1.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsOutput::
set_child_sort(int child_sort) {
  _child_sort = child_sort;
  _got_child_sort = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::clear_child_sort
//       Access: Published
//  Description: Resets the sort value of future offscreen buffers
//               created by make_texture_sort() to the default value.
//               See set_child_sort().
////////////////////////////////////////////////////////////////////
INLINE void GraphicsOutput::
clear_child_sort() {
  _got_child_sort = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_child_sort
//       Access: Published
//  Description: Returns the sort value of future offscreen buffers
//               created by make_texture_sort(). See set_child_sort().
////////////////////////////////////////////////////////////////////
INLINE int GraphicsOutput::
get_child_sort() const {
  if (_got_child_sort) {
    return _child_sort;
  } else {
    return get_sort() - 1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::trigger_copy
//       Access: Published
//  Description: When the GraphicsOutput is in triggered copy mode,
//               this function triggers the copy (at the end of the
//               next frame).
////////////////////////////////////////////////////////////////////
INLINE void GraphicsOutput::
trigger_copy()  {
  _trigger_copy = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::make_display_region
//       Access: Published
//  Description: Creates a new DisplayRegion that covers the entire
//               window.
//
//               If is_stereo() is true for this window, and
//               default-stereo-camera is configured true, this
//               actually makes a StereoDisplayRegion.  Call
//               make_mono_display_region() or
//               make_stereo_display_region() if you want to insist on
//               one or the other.
////////////////////////////////////////////////////////////////////
INLINE DisplayRegion *GraphicsOutput::
make_display_region() {
  return make_display_region(0.0f, 1.0f, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::make_mono_display_region
//       Access: Published
//  Description: Creates a new DisplayRegion that covers the entire
//               window.
//
//               This always returns a mono DisplayRegion, even if
//               is_stereo() is true.
////////////////////////////////////////////////////////////////////
INLINE DisplayRegion *GraphicsOutput::
make_mono_display_region() {
  return make_mono_display_region(0.0f, 1.0f, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::make_stereo_display_region
//       Access: Published
//  Description: Creates a new DisplayRegion that covers the entire
//               window.
//
//               This always returns a stereo DisplayRegion, even if
//               is_stereo() is false.
////////////////////////////////////////////////////////////////////
INLINE StereoDisplayRegion *GraphicsOutput::
make_stereo_display_region() {
  return make_stereo_display_region(0.0f, 1.0f, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::make_screenshot_filename
//       Access: Published, Static
//  Description: Saves a screenshot of the region to a default
//               filename, and returns the filename, or empty string
//               if the screenshot failed.  The default filename is
//               generated from the supplied prefix and from the
//               Config variable screenshot-filename, which contains
//               the following strings:
//
//                 %~p - the supplied prefix
//                 %~f - the frame count
//                 %~e - the value of screenshot-extension
//                 All other % strings in strftime().
////////////////////////////////////////////////////////////////////
INLINE Filename GraphicsOutput::
make_screenshot_filename(const string &prefix) {
  return DisplayRegion::make_screenshot_filename(prefix);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::save_screenshot_default
//       Access: Published
//  Description: Saves a screenshot of the region to a default
//               filename, and returns the filename, or empty string
//               if the screenshot failed.  The filename is generated
//               by make_screenshot_filename().
////////////////////////////////////////////////////////////////////
INLINE Filename GraphicsOutput::
save_screenshot_default(const string &prefix) {
  return _default_display_region->save_screenshot_default(prefix);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::save_screenshot
//       Access: Published
//  Description: Saves a screenshot of the region to the indicated
//               filename.  The image comment is an optional user
//               readable string that will be saved with the header
//               of the image (if the file format supports embedded
//               data; for example jpg allows comments).  Returns
//               true on success, false on failure.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
save_screenshot(const Filename &filename, const string &image_comment) {
  return _default_display_region->save_screenshot(filename, image_comment);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_screenshot
//       Access: Published
//  Description: Captures the most-recently rendered image from the
//               framebuffer into the indicated PNMImage.  Returns
//               true on success, false on failure.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
get_screenshot(PNMImage &image) {
  return _default_display_region->get_screenshot(image);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::flip_ready
//       Access: Public
//  Description: Returns true if a frame has been rendered and needs
//               to be flipped, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
flip_ready() const {
  return _flip_ready;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::operator <
//       Access: Public
//  Description: The sorting operator is used to order the
//               GraphicsOutput object in order by their sort number,
//               so that they will render in the correct order in the
//               GraphicsEngine.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
operator < (const GraphicsOutput &other) const {
  if (_sort != other._sort) {
    return _sort < other._sort;
  }
  return _internal_sort_index < other._internal_sort_index;
}


////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::determine_display_regions
//       Access: Private
//  Description: Recomputes the list of active DisplayRegions within
//               the window, if they have changed recently.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsOutput::
determine_display_regions() const {
  // This function isn't strictly speaking const, but we pretend it is
  // because it only updates a transparent cache value.
  if (_display_regions_stale) {
    ((GraphicsOutput *)this)->do_determine_display_regions();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::win_display_regions_changed
//       Access: Private
//  Description: Intended to be called when the active state on a
//               nested display region changes, forcing the window to
//               recompute its list of active display regions.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsOutput::
win_display_regions_changed() {
  _display_regions_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_cull_window_pcollector
//       Access: Public
//  Description: Returns a PStatCollector for timing the cull
//               operation for just this GraphicsOutput.
////////////////////////////////////////////////////////////////////
INLINE PStatCollector &GraphicsOutput::
get_cull_window_pcollector() {
  return _cull_window_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_draw_window_pcollector
//       Access: Public
//  Description: Returns a PStatCollector for timing the draw
//               operation for just this GraphicsOutput.
////////////////////////////////////////////////////////////////////
INLINE PStatCollector &GraphicsOutput::
get_draw_window_pcollector() {
  return _draw_window_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::begin_frame_spam
//       Access: Public
//  Description: Display the spam message associated with begin_frame
////////////////////////////////////////////////////////////////////
INLINE void GraphicsOutput::
begin_frame_spam(FrameMode mode) {
  if (display_cat.is_spam()) {
    display_cat.spam()
      << "begin_frame(" << mode << "): " << get_type() << " "
      << get_name() << " " << (void *)this << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::end_frame_spam
//       Access: Public
//  Description: Display the spam message associated with end_frame
////////////////////////////////////////////////////////////////////
INLINE void GraphicsOutput::
end_frame_spam(FrameMode mode) {
  if (display_cat.is_spam()) {
    display_cat.spam()
      << "end_frame(" << mode << "): " << get_type() << " "
      << get_name() << " " << (void *)this << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::clear_cube_map_selection
//       Access: Public
//  Description: Clear the variables that select a cube-map face.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsOutput::
clear_cube_map_selection() {
  _cube_map_index = -1;
  _cube_map_dr = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::trigger_flip
//       Access: Public
//  Description: Set the flip_ready flag, only if legal to do so.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsOutput::
trigger_flip() {
  if (!_fb_properties.is_single_buffered()) {
    _flip_ready = true;
  }
}

