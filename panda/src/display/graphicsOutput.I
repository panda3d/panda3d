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
  CDReader cdata(_cycler);
  return cdata->_textures.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::has_texture
//       Access: Published
//  Description: Returns true if the GraphicsOutput is rendering 
//               into any textures at all.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
has_texture() const {
  CDReader cdata(_cycler);
  return (cdata->_textures.size() > 0);
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
  CDReader cdata(_cycler);
  if ((i < 0) || (i >= ((int)cdata->_textures.size()))) {
    return (Texture *)NULL;
  }
  return cdata->_textures[i]._texture;
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
  CDReader cdata(_cycler);
  if ((i < 0) || (i >= ((int)cdata->_textures.size()))) {
    return (RenderTexturePlane)0;
  }
  return cdata->_textures[i]._plane;
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
  CDReader cdata(_cycler);
  if ((i < 0) || (i >= ((int)cdata->_textures.size()))) {
    return RTM_none;
  }
  return cdata->_textures[i]._rtm_mode;
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
//     Function: GraphicsOutput::get_sbs_left_x_size
//       Access: Published
//  Description: If side-by-side stereo is enabled, this returns the
//               pixel width of the left eye, based on scaling
//               get_x_size() by get_sbs_left_dimensions().  If
//               side-by-side stereo is not enabled, this returns the
//               same as get_x_size().
////////////////////////////////////////////////////////////////////
INLINE int GraphicsOutput::
get_sbs_left_x_size() const {
  PN_stdfloat left_w = _sbs_left_dimensions[1] - _sbs_left_dimensions[0];
  return max(int(_x_size * left_w), 1);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_sbs_left_y_size
//       Access: Published
//  Description: If side-by-side stereo is enabled, this returns the
//               pixel height of the left eye, based on scaling
//               get_y_size() by get_sbs_left_dimensions().  If
//               side-by-side stereo is not enabled, this returns the
//               same as get_y_size().
////////////////////////////////////////////////////////////////////
INLINE int GraphicsOutput::
get_sbs_left_y_size() const {
  PN_stdfloat left_h = _sbs_left_dimensions[3] - _sbs_left_dimensions[2];
  return max(int(_y_size * left_h), 1);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_sbs_right_x_size
//       Access: Published
//  Description: If side-by-side stereo is enabled, this returns the
//               pixel width of the right eye, based on scaling
//               get_x_size() by get_sbs_right_dimensions().  If
//               side-by-side stereo is not enabled, this returns the
//               same as get_x_size().
////////////////////////////////////////////////////////////////////
INLINE int GraphicsOutput::
get_sbs_right_x_size() const {
  PN_stdfloat right_w = _sbs_right_dimensions[1] - _sbs_right_dimensions[0];
  return max(int(_x_size * right_w), 1);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_sbs_right_y_size
//       Access: Published
//  Description: If side-by-side stereo is enabled, this returns the
//               pixel height of the right eye, based on scaling
//               get_y_size() by get_sbs_right_dimensions().  If
//               side-by-side stereo is not enabled, this returns the
//               same as get_y_size().
////////////////////////////////////////////////////////////////////
INLINE int GraphicsOutput::
get_sbs_right_y_size() const {
  PN_stdfloat right_h = _sbs_right_dimensions[3] - _sbs_right_dimensions[2];
  return max(int(_y_size * right_h), 1);
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
  return _is_valid && _is_nonzero_size;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::is_nonzero_size
//       Access: Published
//  Description: Returns true if the output has a nonzero size in both
//               X and Y, or false if it is zero (and therefore
//               invalid).
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
is_nonzero_size() const {
  return _is_nonzero_size;
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
//     Function: GraphicsOutput::set_swap_eyes
//       Access: Public
//  Description: Changes the "swap eyes" flag.  This flag is normally
//               false.  When it is true, the left and right channels
//               of a stereo DisplayRegion are sent to the opposite
//               channels in the rendering backend.  This is meant to
//               work around hardware that inadvertently swaps the
//               output channels, or hardware for which it cannot be
//               determined which channel is which until runtime.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsOutput::
set_swap_eyes(bool swap_eyes) {
  _swap_eyes = swap_eyes;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_swap_eyes
//       Access: Public
//  Description: Returns the current setting of the "swap eyes" flag.
//               See set_swap_eyes().
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
get_swap_eyes() const {
  return _swap_eyes;
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
//     Function: GraphicsOutput::get_side_by_side_stereo
//       Access: Published
//  Description: Returns whether side-by-side stereo mode is in effect for
//               this particular window.  See set_side_by_side_stereo().
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
get_side_by_side_stereo() const {
  return _side_by_side_stereo;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_sbs_left_dimensions
//       Access: Published
//  Description: Returns the effective sub-region of the window for
//               displaying the left channel, if side-by-side stereo
//               mode is in effect for the window.  See
//               set_side_by_side_stereo().
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4 &GraphicsOutput::
get_sbs_left_dimensions() const {
  return _sbs_left_dimensions;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_sbs_right_dimensions
//       Access: Published
//  Description: Returns the effective sub-region of the window for
//               displaying the right channel, if side-by-side stereo
//               mode is in effect for the window.  See
//               set_side_by_side_stereo().
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4 &GraphicsOutput::
get_sbs_right_dimensions() const {
  return _sbs_right_dimensions;
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
  return _red_blue_stereo || _side_by_side_stereo || _fb_properties.is_stereo();
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
//     Function: GraphicsOutput::make_display_region
//       Access: Published
//  Description: Creates a new DisplayRegion that covers the indicated
//               sub-rectangle within the window.  The range on all
//               parameters is 0..1.
//
//               If is_stereo() is true for this window, and
//               default-stereo-camera is configured true, this
//               actually makes a StereoDisplayRegion.  Call
//               make_mono_display_region() or
//               make_stereo_display_region() if you want to insist on
//               one or the other.
////////////////////////////////////////////////////////////////////
DisplayRegion *GraphicsOutput::
make_display_region(PN_stdfloat l, PN_stdfloat r, PN_stdfloat b, PN_stdfloat t) {
  return make_display_region(LVecBase4(l, r, b, t));
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::make_mono_display_region
//       Access: Published
//  Description: Creates a new DisplayRegion that covers the entire
//               window.
//
//               This generally returns a mono DisplayRegion, even if
//               is_stereo() is true.  However, if side-by-side stereo
//               is enabled, this will return a StereoDisplayRegion
//               whose two eyes are both set to SC_mono.  (This is
//               necessary because in side-by-side stereo mode, it is
//               necessary to draw even mono DisplayRegions twice).
////////////////////////////////////////////////////////////////////
INLINE DisplayRegion *GraphicsOutput::
make_mono_display_region() {
  return make_mono_display_region(0.0f, 1.0f, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::make_mono_display_region
//       Access: Published
//  Description: Creates a new DisplayRegion that covers the entire
//               window.
//
//               This generally returns a mono DisplayRegion, even if
//               is_stereo() is true.  However, if side-by-side stereo
//               is enabled, this will return a StereoDisplayRegion
//               whose two eyes are both set to SC_mono.  (This is
//               necessary because in side-by-side stereo mode, it is
//               necessary to draw even mono DisplayRegions twice).
////////////////////////////////////////////////////////////////////
INLINE DisplayRegion *GraphicsOutput::
make_mono_display_region(PN_stdfloat l, PN_stdfloat r, PN_stdfloat b, PN_stdfloat t) {
  return make_mono_display_region(LVecBase4(l, r, b, t));
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
//     Function: GraphicsOutput::make_stereo_display_region
//       Access: Published
//  Description: Creates a new DisplayRegion that covers the entire
//               window.
//
//               This always returns a stereo DisplayRegion, even if
//               is_stereo() is false.
////////////////////////////////////////////////////////////////////
INLINE StereoDisplayRegion *GraphicsOutput::
make_stereo_display_region(PN_stdfloat l, PN_stdfloat r, PN_stdfloat b, PN_stdfloat t) {
  return make_stereo_display_region(LVecBase4(l, r, b, t));
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_overlay_display_region
//       Access: Published
//  Description: Returns the special "overlay" DisplayRegion that is
//               created for each window or buffer.  This
//               DisplayRegion covers the entire window, but cannot be
//               used for rendering.  It is a placeholder only, to
//               indicate the dimensions of the window, and is usually
//               used internally for purposes such as clearing the
//               window, or grabbing a screenshot of the window.
//
//               There are very few applications that require access
//               to this DisplayRegion.  Normally, you should create
//               your own DisplayRegion that covers the window, if you
//               want to render to the window.
////////////////////////////////////////////////////////////////////
INLINE DisplayRegion *GraphicsOutput::
get_overlay_display_region() const {
  return _overlay_display_region;
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
  return _overlay_display_region->save_screenshot_default(prefix);
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
  return _overlay_display_region->save_screenshot(filename, image_comment);
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
  return _overlay_display_region->get_screenshot(image);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_screenshot
//       Access: Published
//  Description: Captures the most-recently rendered image from the
//               framebuffer and returns it as Texture, or NULL on
//               failure.
////////////////////////////////////////////////////////////////////
INLINE PT(Texture) GraphicsOutput::
get_screenshot() {
  return _overlay_display_region->get_screenshot();
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
  CDLockedReader cdata(_cycler);
  if (cdata->_active_display_regions_stale) {
    CDWriter cdataw(((GraphicsOutput *)this)->_cycler, cdata, false);
    ((GraphicsOutput *)this)->do_determine_display_regions(cdataw);
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
  CDWriter cdata(_cycler, true);
  cdata->_active_display_regions_stale = true;
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
//  Description: Clear the variables that select a cube-map face (or
//               other multipage texture face).
////////////////////////////////////////////////////////////////////
INLINE void GraphicsOutput::
clear_cube_map_selection() {
  _target_tex_page = -1;
  _prev_page_dr = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::trigger_flip
//       Access: Protected
//  Description: To be called at the end of the frame, after the
//               window has successfully been drawn and is ready to be
//               flipped (if appropriate).
////////////////////////////////////////////////////////////////////
INLINE void GraphicsOutput::
trigger_flip() {
  if (!_fb_properties.is_single_buffered()) {
    _flip_ready = true;
  }
}

