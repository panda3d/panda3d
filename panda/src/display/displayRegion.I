// Filename: displayRegion.I
// Created by:  frang (07Mar99)
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
//     Function: DisplayRegion::operator <
//       Access: Public
//  Description: Returns true if this DisplayRegion should be sorted
//               before the other one, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool DisplayRegion::
operator < (const DisplayRegion &other) const {
  return get_sort() < other.get_sort();
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_lens_index
//       Access: Public
//  Description: Returns the specific lens of the associated Camera
//               that will be used for rendering this scene.  Most
//               Cameras hold only one lens, but for multiple lenses
//               this method may be used to selected between them.
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegion::
get_lens_index() const {
  CDReader cdata(_cycler);
  return cdata->_lens_index;
}

///////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_num_regions
//       Access: Published
//  Description: Returns the number of regions, see set_num_regions.
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegion::
get_num_regions() const {
  CDReader cdata(_cycler);
  return cdata->_regions.size();
}

///////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_num_regions
//       Access: Published
//  Description: Sets the number of regions that this DisplayRegion
//               indicates.  Usually, this number is 1 (and it is
//               always at least 1), and only the first is used for
//               rendering.  However, if more than one is provided,
//               you may select which one to render into using a
//               geometry shader (gl_ViewportIndex in GLSL).
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
set_num_regions(int i) {
  nassertv(i >= 1);
  CDWriter cdata(_cycler);
  cdata->_regions.resize(i);
}

///////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_dimensions
//       Access: Published
//  Description: Retrieves the coordinates of the DisplayRegion's
//               rectangle within its GraphicsOutput.  These numbers
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
get_dimensions(PN_stdfloat &l, PN_stdfloat &r, PN_stdfloat &b, PN_stdfloat &t) const {
  get_dimensions(0, l, r, b, t);
}

///////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_dimensions
//       Access: Published
//  Description: Retrieves the coordinates of the DisplayRegion's
//               rectangle within its GraphicsOutput.  These numbers
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
get_dimensions(int i, PN_stdfloat &l, PN_stdfloat &r, PN_stdfloat &b, PN_stdfloat &t) const {
  CDReader cdata(_cycler);
  const Region &region = cdata->_regions[i];
  l = region._dimensions[0];
  r = region._dimensions[1];
  b = region._dimensions[2];
  t = region._dimensions[3];
}

///////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_dimensions
//       Access: Published
//  Description: Retrieves the coordinates of the DisplayRegion's
//               rectangle within its GraphicsOutput.  These numbers
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE LVecBase4 DisplayRegion::
get_dimensions(int i) const {
  CDReader cdata(_cycler);
  return cdata->_regions[i]._dimensions;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_left
//       Access: Published
//  Description: Retrieves the x coordinate of the left edge of the
//               rectangle within its GraphicsOutput.  This number
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DisplayRegion::
get_left(int i) const {
  CDReader cdata(_cycler);
  return cdata->_regions[i]._dimensions[0];
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_right
//       Access: Published
//  Description: Retrieves the x coordinate of the right edge of the
//               rectangle within its GraphicsOutput.  This number
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DisplayRegion::
get_right(int i) const {
  CDReader cdata(_cycler);
  return cdata->_regions[i]._dimensions[1];
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_bottom
//       Access: Published
//  Description: Retrieves the y coordinate of the bottom edge of 
//               the rectangle within its GraphicsOutput.  This 
//               number will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DisplayRegion::
get_bottom(int i) const {
  CDReader cdata(_cycler);
  return cdata->_regions[i]._dimensions[2];
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_top
//       Access: Published
//  Description: Retrieves the y coordinate of the top edge of the
//               rectangle within its GraphicsOutput.  This number
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DisplayRegion::
get_top(int i) const {
  CDReader cdata(_cycler);
  return cdata->_regions[i]._dimensions[3];
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_dimensions
//       Access: Published, Virtual
//  Description: Changes the portion of the framebuffer this
//               DisplayRegion corresponds to.  The parameters range
//               from 0 to 1, where 0,0 is the lower left corner and
//               1,1 is the upper right; (0, 1, 0, 1) represents the
//               whole screen.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
set_dimensions(PN_stdfloat l, PN_stdfloat r, PN_stdfloat b, PN_stdfloat t) {
  set_dimensions(0, LVecBase4(l, r, b, t));
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_dimensions
//       Access: Published, Virtual
//  Description: Changes the portion of the framebuffer this
//               DisplayRegion corresponds to.  The parameters range
//               from 0 to 1, where 0,0 is the lower left corner and
//               1,1 is the upper right; (0, 1, 0, 1) represents the
//               whole screen.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
set_dimensions(int i, PN_stdfloat l, PN_stdfloat r, PN_stdfloat b, PN_stdfloat t) {
  set_dimensions(i, LVecBase4(l, r, b, t));
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_dimensions
//       Access: Published, Virtual
//  Description: Changes the portion of the framebuffer this
//               DisplayRegion corresponds to.  The parameters range
//               from 0 to 1, where 0,0 is the lower left corner and
//               1,1 is the upper right; (0, 1, 0, 1) represents the
//               whole screen.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
set_dimensions(const LVecBase4 &dimensions) {
  set_dimensions(0, dimensions);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_window
//       Access: Published
//  Description: Returns the GraphicsOutput that this DisplayRegion is
//               ultimately associated with, or NULL if no window is
//               associated.
////////////////////////////////////////////////////////////////////
INLINE GraphicsOutput *DisplayRegion::
get_window() const {
  return _window;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_camera
//       Access: Published
//  Description: Returns the camera associated with this
//               DisplayRegion, or an empty NodePath if no camera is
//               associated.
////////////////////////////////////////////////////////////////////
INLINE NodePath DisplayRegion::
get_camera(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_camera;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::is_active
//       Access: Published
//  Description: Returns the active flag associated with the
//               DisplayRegion.
////////////////////////////////////////////////////////////////////
INLINE bool DisplayRegion::
is_active() const {
  CDReader cdata(_cycler);
  return cdata->_active;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_sort
//       Access: Published
//  Description: Returns the sort value associated with the
//               DisplayRegion.
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegion::
get_sort() const {
  CDReader cdata(_cycler);
  return cdata->_sort;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_stereo_channel
//       Access: Published
//  Description: Returns whether the DisplayRegion is specified as the
//               left or right channel of a stereo pair, or whether it
//               is a normal, monocular image.  See
//               set_stereo_channel().
////////////////////////////////////////////////////////////////////
INLINE Lens::StereoChannel DisplayRegion::
get_stereo_channel() const {
  CDReader cdata(_cycler);
  return cdata->_stereo_channel;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_tex_view_offset
//       Access: Public
//  Description: Returns the current texture view offset for this
//               DisplayRegion.  This is normally set to zero.  If
//               nonzero, it is used to select a particular view of
//               any multiview textures that are rendered within this
//               DisplayRegion.
//
//               For a StereoDisplayRegion, this is normally 0 for the
//               left eye, and 1 for the right eye, to support stereo
//               textures.
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegion::
get_tex_view_offset() const {
  CDReader cdata(_cycler);
  return cdata->_tex_view_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_incomplete_render
//       Access: Published
//  Description: Returns the incomplete_render flag.  See
//               set_incomplete_render().
////////////////////////////////////////////////////////////////////
INLINE bool DisplayRegion::
get_incomplete_render() const {
  return _incomplete_render;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_texture_reload_priority
//       Access: Published
//  Description: Returns the priority which is assigned to
//               asynchronous texture reload requests.  See
//               set_texture_reload_priority().
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegion::
get_texture_reload_priority() const {
  return _texture_reload_priority;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_cube_map_index
//       Access: Published
//  Description: Deprecated; replaced by set_target_tex_page().
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
set_cube_map_index(int cube_map_index) {
  set_target_tex_page(cube_map_index);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_target_tex_page
//       Access: Published
//  Description: Returns the target page number associated with this
//               particular DisplayRegion, or -1 if it is not
//               associated with a page.  See
//               set_target_tex_page().
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegion::
get_target_tex_page() const {
  CDReader cdata(_cycler);
  return cdata->_target_tex_page;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_scissor_enabled
//       Access: Published
//  Description: Sets whether or not scissor testing is enabled
//               for this region.  The default is true, except for
//               the overlay display region.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
set_scissor_enabled(bool scissor_enabled) {
  CDWriter cdata(_cycler);
  cdata->_scissor_enabled = scissor_enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_scissor_enabled
//       Access: Published
//  Description: Returns whether or not scissor testing is enabled
//               for this region.  The default is true, except for
//               the overlay display region.
////////////////////////////////////////////////////////////////////
INLINE bool DisplayRegion::
get_scissor_enabled() const {
  CDReader cdata(_cycler);
  return cdata->_scissor_enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_cull_callback
//       Access: Published
//  Description: Sets the CallbackObject that will be notified when
//               the DisplayRegion is visited during the cull
//               traversal.  This callback will be made during the
//               cull thread.
//
//               The cull traversal is responsible for determining
//               which nodes are visible and within the view frustum,
//               and for accumulating state and transform, and
//               generally building up the list of CullableObjects
//               that are to be eventually passed to the draw
//               traversal for rendering.
//
//               At the time the cull traversal callback is made, the
//               traversal for this DisplayRegion has not yet started.
//
//               The callback is passed an instance of a
//               DisplayRegionCullCallbackData, which contains
//               pointers to the current scene information, as well as
//               the current DisplayRegion and GSG.  The callback
//               *replaces* the normal cull behavior, so if your
//               callback does nothing, the scene graph will not be
//               traversed and therefore nothing will be drawn.  If
//               you wish the normal cull traversal to be performed
//               for this DisplayRegion, you must call
//               cbdata->upcall() from your callback.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
set_cull_callback(CallbackObject *object) {
  CDWriter cdata(_cycler);
  cdata->_cull_callback = object;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::clear_cull_callback
//       Access: Published
//  Description: Removes the callback set by an earlier call to
//               set_cull_callback().
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
clear_cull_callback() {
  set_cull_callback(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_cull_callback
//       Access: Published
//  Description: Returns the CallbackObject set by set_cull_callback().
////////////////////////////////////////////////////////////////////
INLINE CallbackObject *DisplayRegion::
get_cull_callback() const {
  CDReader cdata(_cycler);
  return cdata->_cull_callback;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_draw_callback
//       Access: Published
//  Description: Sets the CallbackObject that will be notified when
//               the contents of DisplayRegion is drawn during the
//               draw traversal.  This callback will be made during
//               the draw thread.
//
//               The draw traversal is responsible for actually
//               issuing the commands to the graphics engine to draw
//               primitives.  Its job is to walk through the list of
//               CullableObjects build up by the cull traversal, as
//               quickly as possible, issuing the appropriate commands
//               to draw each one.
//
//               At the time the draw traversal callback is made, the
//               graphics state is in the initial state, and no
//               projection matrix or modelview matrix is in effect.
//               begin_scene() has not yet been called, and no objects
//               have yet been drawn.  However, the viewport has
//               already been set to the appropriate part of the
//               window, and the clear commands for this DisplayRegion
//               (if any) have been issued.
//
//               The callback is passed an instance of a
//               DisplayRegionDrawCallbackData, which contains
//               pointers to the current scene information, as well as
//               the current DisplayRegion and GSG.  The callback
//               *replaces* the normal draw behavior, so if your
//               callback does nothing, nothing in the DisplayRegion
//               will be drawn.  If you wish the draw traversal to
//               continue to draw the contents of this DisplayRegion,
//               you must call cbdata->upcall() from your callback.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
set_draw_callback(CallbackObject *object) {
  CDWriter cdata(_cycler);
  cdata->_draw_callback = object;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::clear_draw_callback
//       Access: Published
//  Description: Removes the callback set by an earlier call to
//               set_draw_callback().
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
clear_draw_callback() {
  set_draw_callback(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_draw_callback
//       Access: Published
//  Description: Returns the CallbackObject set by set_draw_callback().
////////////////////////////////////////////////////////////////////
INLINE CallbackObject *DisplayRegion::
get_draw_callback() const {
  CDReader cdata(_cycler);
  return cdata->_draw_callback;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_pixel_width
//       Access: Published
//  Description: Returns the width of the DisplayRegion in pixels.
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegion::
get_pixel_width(int i) const {
  CDReader cdata(_cycler);
  return cdata->_regions[i]._pixels[1] - cdata->_regions[i]._pixels[0];
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_pixel_height
//       Access: Published
//  Description: Returns the height of the DisplayRegion in pixels.
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegion::
get_pixel_height(int i) const {
  CDReader cdata(_cycler);
  return cdata->_regions[i]._pixels[3] - cdata->_regions[i]._pixels[2];
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_pixels
//       Access: Public
//  Description: Retrieves the coordinates of the DisplayRegion within
//               its window, in pixels.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
get_pixels(int &pl, int &pr, int &pb, int &pt) const {
  get_pixels(0, pl, pr, pb, pt);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_pixels
//       Access: Public
//  Description: Retrieves the coordinates of the DisplayRegion within
//               its window, in pixels.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
get_pixels(int i, int &pl, int &pr, int &pb, int &pt) const {
  CDReader cdata(_cycler);
  const Region &region = cdata->_regions[i];
  pl = region._pixels[0];
  pr = region._pixels[1];
  pb = region._pixels[2];
  pt = region._pixels[3];
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_region_pixels
//       Access: Public
//  Description: Retrieves the coordinates of the DisplayRegion within
//               its window, as the pixel location of its bottom-left
//               corner, along with a pixel width and height.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
get_region_pixels(int &xo, int &yo, int &w, int &h) const {
  get_region_pixels(0, xo, yo, w, h);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_region_pixels
//       Access: Public
//  Description: Retrieves the coordinates of the DisplayRegion within
//               its window, as the pixel location of its bottom-left
//               corner, along with a pixel width and height.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
get_region_pixels(int i, int &xo, int &yo, int &w, int &h) const {
  CDReader cdata(_cycler);
  const Region &region = cdata->_regions[i];
  xo = region._pixels[0];
  yo = region._pixels[2];
  w = region._pixels[1] - xo;
  h = region._pixels[3] - yo;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_region_pixels_i
//       Access: Public
//  Description: Similar to get_region_pixels(), but returns the upper
//               left corner, and the pixel numbers are numbered from
//               the top-left corner down, in the DirectX way of
//               things.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
get_region_pixels_i(int &xo, int &yo, int &w, int &h) const {
  get_region_pixels_i(0, xo, yo, w, h);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_region_pixels_i
//       Access: Public
//  Description: Similar to get_region_pixels(), but returns the upper
//               left corner, and the pixel numbers are numbered from
//               the top-left corner down, in the DirectX way of
//               things.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
get_region_pixels_i(int i, int &xo, int &yo, int &w, int &h) const {
  CDReader cdata(_cycler);
  const Region &region = cdata->_regions[i];
  xo = region._pixels_i[0];
  yo = region._pixels_i[3];
  w = region._pixels_i[1] - xo;
  h = region._pixels_i[2] - yo;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_cull_result
//       Access: Public
//  Description: Stores the result of performing a cull operation on
//               this DisplayRegion.  Normally, this will only be
//               called by the GraphicsEngine; you should not call
//               this directly.
//
//               The stored result will automatically be applied back
//               to all upstream pipeline stages.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
set_cull_result(CullResult *cull_result, SceneSetup *scene_setup,
                Thread *current_thread) {
  CDCullWriter cdata(_cycler_cull, true, current_thread);
  cdata->_cull_result = cull_result;
  cdata->_scene_setup = scene_setup;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_cull_result
//       Access: Public
//  Description: Returns the CullResult value that was stored on this
//               DisplayRegion, presumably by the last successful cull
//               operation.  This method is for the benefit of the
//               GraphicsEngine; normally you shouldn't call this
//               directly.
////////////////////////////////////////////////////////////////////
INLINE CullResult *DisplayRegion::
get_cull_result(Thread *current_thread) const {
  CDCullReader cdata(_cycler_cull, current_thread);
  return cdata->_cull_result;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_scene_setup
//       Access: Public
//  Description: Returns the SceneSetup value that was stored on this
//               DisplayRegion, presumably by the last successful cull
//               operation.  This method is for the benefit of the
//               GraphicsEngine; normally you shouldn't call this
//               directly.
////////////////////////////////////////////////////////////////////
INLINE SceneSetup *DisplayRegion::
get_scene_setup(Thread *current_thread) const {
  CDCullReader cdata(_cycler_cull, current_thread);
  return cdata->_scene_setup;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_cull_region_pcollector
//       Access: Public
//  Description: Returns a PStatCollector for timing the cull
//               operation for just this DisplayRegion.
////////////////////////////////////////////////////////////////////
INLINE PStatCollector &DisplayRegion::
get_cull_region_pcollector() {
  return _cull_region_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_draw_region_pcollector
//       Access: Public
//  Description: Returns a PStatCollector for timing the draw
//               operation for just this DisplayRegion.
////////////////////////////////////////////////////////////////////
INLINE PStatCollector &DisplayRegion::
get_draw_region_pcollector() {
  return _draw_region_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::Region::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DisplayRegion::Region::
Region() :
  _dimensions(0, 1, 0, 1),
  _pixels(0),
  _pixels_i(0) {
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::CDataCull::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DisplayRegion::CDataCull::
CDataCull() {
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::CDataCull::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DisplayRegion::CDataCull::
CDataCull(const DisplayRegion::CDataCull &copy) :
  _cull_result(copy._cull_result),
  _scene_setup(copy._scene_setup)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DisplayRegionPipelineReader::
DisplayRegionPipelineReader(DisplayRegion *object, Thread *current_thread) :
  _object(object),
  _current_thread(current_thread),
  _cdata(object->_cycler.read(current_thread))
{
#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#ifdef DO_PIPELINING
  nassertv(_cdata->test_ref_count_nonzero());
#endif  // DO_PIPELINING
#endif // _DEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::Copy Constructor
//       Access: Private
//  Description: Don't attempt to copy these objects.
////////////////////////////////////////////////////////////////////
INLINE DisplayRegionPipelineReader::
DisplayRegionPipelineReader(const DisplayRegionPipelineReader &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::Copy Assignment Operator
//       Access: Private
//  Description: Don't attempt to copy these objects.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegionPipelineReader::
operator = (const DisplayRegionPipelineReader &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DisplayRegionPipelineReader::
~DisplayRegionPipelineReader() {
#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#ifdef DO_PIPELINING
  nassertv(_cdata->test_ref_count_nonzero());
#endif  // DO_PIPELINING
#endif // _DEBUG
  _object->_cycler.release_read(_cdata);

#ifdef _DEBUG
  _object = NULL;
  _cdata = NULL;
#endif  // _DEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_object
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DisplayRegion *DisplayRegionPipelineReader::
get_object() const {
  return _object;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_current_thread
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Thread *DisplayRegionPipelineReader::
get_current_thread() const {
  return _current_thread;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::is_any_clear_active
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool DisplayRegionPipelineReader::
is_any_clear_active() const {
  return _object->is_any_clear_active();
}

///////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_num_regions
//       Access: Published
//  Description: Returns the number of regions, see set_num_regions.
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegionPipelineReader::
get_num_regions() const {
  return _cdata->_regions.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_dimensions
//       Access: Public
//  Description: Retrieves the coordinates of the DisplayRegion's
//               rectangle within its GraphicsOutput.  These numbers
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegionPipelineReader::
get_dimensions(PN_stdfloat &l, PN_stdfloat &r, PN_stdfloat &b, PN_stdfloat &t) const {
  return get_dimensions(0, l, r, b, t);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_dimensions
//       Access: Public
//  Description: Retrieves the coordinates of the DisplayRegion's
//               rectangle within its GraphicsOutput.  These numbers
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegionPipelineReader::
get_dimensions(int i, PN_stdfloat &l, PN_stdfloat &r, PN_stdfloat &b, PN_stdfloat &t) const {
  const DisplayRegion::Region &region = _cdata->_regions[i];
  l = region._dimensions[0];
  r = region._dimensions[1];
  b = region._dimensions[2];
  t = region._dimensions[3];
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_dimensions
//       Access: Public
//  Description: Retrieves the coordinates of the DisplayRegion's
//               rectangle within its GraphicsOutput.  These numbers
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4 &DisplayRegionPipelineReader::
get_dimensions(int i) const {
  return _cdata->_regions[i]._dimensions;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_left
//       Access: Public
//  Description: Retrieves the x coordinate of the left edge of the
//               rectangle within its GraphicsOutput.  This number
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DisplayRegionPipelineReader::
get_left(int i) const {
  return _cdata->_regions[i]._dimensions[0];
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_right
//       Access: Public
//  Description: Retrieves the x coordinate of the right edge of the
//               rectangle within its GraphicsOutput.  This number
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DisplayRegionPipelineReader::
get_right(int i) const {
  return _cdata->_regions[i]._dimensions[1];
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_bottom
//       Access: Public
//  Description: Retrieves the y coordinate of the bottom edge of 
//               the rectangle within its GraphicsOutput.  This 
//               number will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DisplayRegionPipelineReader::
get_bottom(int i) const {
  return _cdata->_regions[i]._dimensions[2];
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_top
//       Access: Public
//  Description: Retrieves the y coordinate of the top edge of the
//               rectangle within its GraphicsOutput.  This number
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DisplayRegionPipelineReader::
get_top(int i) const {
  return _cdata->_regions[i]._dimensions[3];
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_window
//       Access: Public
//  Description: Returns the GraphicsOutput that this DisplayRegion is
//               ultimately associated with, or NULL if no window is
//               associated.
////////////////////////////////////////////////////////////////////
INLINE GraphicsOutput *DisplayRegionPipelineReader::
get_window() const {
  return _object->_window;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_camera
//       Access: Public
//  Description: Returns the camera associated with this
//               DisplayRegion, or an empty NodePath if no camera is
//               associated.
////////////////////////////////////////////////////////////////////
INLINE NodePath DisplayRegionPipelineReader::
get_camera() const {
  return _cdata->_camera;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::is_active
//       Access: Public
//  Description: Returns the active flag associated with the
//               DisplayRegion.
////////////////////////////////////////////////////////////////////
INLINE bool DisplayRegionPipelineReader::
is_active() const {
  return _cdata->_active;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_sort
//       Access: Public
//  Description: Returns the sort value associated with the
//               DisplayRegion.
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegionPipelineReader::
get_sort() const {
  return _cdata->_sort;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_stereo_channel
//       Access: Public
//  Description: Returns whether the DisplayRegion is specified as the
//               left or right channel of a stereo pair, or whether it
//               is a normal, monocular image.  See
//               set_stereo_channel().
////////////////////////////////////////////////////////////////////
INLINE Lens::StereoChannel DisplayRegionPipelineReader::
get_stereo_channel() const {
  return _cdata->_stereo_channel;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_tex_view_offset
//       Access: Public
//  Description: Returns the current texture view offset for this
//               DisplayRegion.  This is normally set to zero.  If
//               nonzero, it is used to select a particular view of
//               any multiview textures that are rendered within this
//               DisplayRegion.
//
//               For a StereoDisplayRegion, this is normally 0 for the
//               left eye, and 1 for the right eye, to support stereo
//               textures.
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegionPipelineReader::
get_tex_view_offset() {
  return _cdata->_tex_view_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_target_tex_page
//       Access: Published
//  Description: Returns the target page number associated with this
//               particular DisplayRegion, or -1 if it is not
//               associated with a page.  See
//               set_target_tex_page().
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegionPipelineReader::
get_target_tex_page() const {
  return _cdata->_target_tex_page;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_scissor_enabled
//       Access: Published
//  Description: Returns whether or not scissor testing is enabled
//               for this region.  The default is true, except for
//               the overlay display region.
////////////////////////////////////////////////////////////////////
INLINE bool DisplayRegionPipelineReader::
get_scissor_enabled() const {
  return _cdata->_scissor_enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_draw_callback
//       Access: Published
//  Description: Returns the CallbackObject set by set_draw_callback().
////////////////////////////////////////////////////////////////////
INLINE CallbackObject *DisplayRegionPipelineReader::
get_draw_callback() const {
  return _cdata->_draw_callback;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_pixels
//       Access: Public
//  Description: Retrieves the coordinates of the DisplayRegion within
//               its window, in pixels.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegionPipelineReader::
get_pixels(int &pl, int &pr, int &pb, int &pt) const {
  get_pixels(0, pl, pr, pb, pt);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_pixels
//       Access: Public
//  Description: Retrieves the coordinates of the DisplayRegion within
//               its window, in pixels.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegionPipelineReader::
get_pixels(int i, int &pl, int &pr, int &pb, int &pt) const {
  const DisplayRegion::Region &region = _cdata->_regions[i];
  pl = region._pixels[0];
  pr = region._pixels[1];
  pb = region._pixels[2];
  pt = region._pixels[3];
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_region_pixels
//       Access: Public
//  Description: Retrieves the coordinates of the DisplayRegion within
//               its window, as the pixel location of its bottom-left
//               corner, along with a pixel width and height.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegionPipelineReader::
get_region_pixels(int &xo, int &yo, int &w, int &h) const {
  get_region_pixels(0, xo, yo, w, h);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_region_pixels
//       Access: Public
//  Description: Retrieves the coordinates of the DisplayRegion within
//               its window, as the pixel location of its bottom-left
//               corner, along with a pixel width and height.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegionPipelineReader::
get_region_pixels(int i, int &xo, int &yo, int &w, int &h) const {
  const DisplayRegion::Region &region = _cdata->_regions[i];
  xo = region._pixels[0];
  yo = region._pixels[2];
  w = region._pixels[1] - xo;
  h = region._pixels[3] - yo;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_region_pixels_i
//       Access: Public
//  Description: Similar to get_region_pixels(), but returns the upper
//               left corner, and the pixel numbers are numbered from
//               the top-left corner down, in the DirectX way of
//               things.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegionPipelineReader::
get_region_pixels_i(int &xo, int &yo, int &w, int &h) const {
  get_region_pixels_i(0, xo, yo, w, h);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_region_pixels_i
//       Access: Public
//  Description: Similar to get_region_pixels(), but returns the upper
//               left corner, and the pixel numbers are numbered from
//               the top-left corner down, in the DirectX way of
//               things.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegionPipelineReader::
get_region_pixels_i(int i, int &xo, int &yo, int &w, int &h) const {
  const DisplayRegion::Region &region = _cdata->_regions[i];
  xo = region._pixels_i[0];
  yo = region._pixels_i[3];
  w = region._pixels_i[1] - xo;
  h = region._pixels_i[2] - yo;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_pixel_width
//       Access: Public
//  Description: Returns the width of the DisplayRegion in pixels.
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegionPipelineReader::
get_pixel_width(int i) const {
  return _cdata->_regions[i]._pixels[1] - _cdata->_regions[i]._pixels[0];
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_lens_index
//       Access: Public
//  Description: Gets the index into a lens_node lens array. 0 default
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegionPipelineReader::
get_lens_index() const {
  return _cdata->_lens_index;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionPipelineReader::get_pixel_height
//       Access: Public
//  Description: Returns the height of the DisplayRegion in pixels.
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegionPipelineReader::
get_pixel_height(int i) const {
  return _cdata->_regions[i]._pixels[3] - _cdata->_regions[i]._pixels[2];
}
