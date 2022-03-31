/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stereoDisplayRegion.h
 * @author drose
 * @date 2009-02-19
 */

#ifndef STEREODISPLAYREGION_H
#define STEREODISPLAYREGION_H

#include "pandabase.h"

#include "displayRegion.h"

/**
 * This is a special DisplayRegion wrapper that actually includes a pair of
 * DisplayRegions internally: the left and right eyes.  The DisplayRegion
 * represented here does not have a physical association with the window, but
 * it pretends it does.  Instead, it maintains a pointer to the left and right
 * DisplayRegions separately.
 *
 * Operations on the StereoDisplayRegion object affect both left and right
 * eyes together.  To access the left or right eyes independently, use
 * get_left_eye() and get_right_eye().
 */
class EXPCL_PANDA_DISPLAY StereoDisplayRegion : public DisplayRegion {
protected:
  StereoDisplayRegion(GraphicsOutput *window,
                      const LVecBase4 &dimensions,
                      DisplayRegion *left, DisplayRegion *right);

public:
  virtual ~StereoDisplayRegion();

PUBLISHED:
  // Inherited from DrawableRegion
  virtual void set_clear_active(int n, bool clear_aux_active);
  virtual void set_clear_value(int n, const LColor &clear_value);
  virtual void disable_clears();
  virtual void set_pixel_zoom(PN_stdfloat pixel_zoom);

  // Inherited from DisplayRegion
  virtual void set_dimensions(int i, const LVecBase4 &dimensions);
  virtual bool is_stereo() const;
  virtual void set_camera(const NodePath &camera);
  virtual void set_active(bool active);
  virtual void set_sort(int sort);
  virtual void set_stereo_channel(Lens::StereoChannel stereo_channel);
  virtual void set_tex_view_offset(int tex_view_offset);
  virtual void set_incomplete_render(bool incomplete_render);
  virtual void set_texture_reload_priority(int texture_reload_priority);
  virtual void set_cull_traverser(CullTraverser *trav);
  virtual void set_target_tex_page(int page);

  virtual void output(std::ostream &out) const;
  virtual PT(PandaNode) make_cull_result_graph();

  INLINE DisplayRegion *get_left_eye();
  INLINE DisplayRegion *get_right_eye();
  MAKE_PROPERTY(left_eye, get_left_eye);
  MAKE_PROPERTY(right_eye, get_right_eye);

private:
  PT(DisplayRegion) _left_eye;
  PT(DisplayRegion) _right_eye;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DisplayRegion::init_type();
    register_type(_type_handle, "StereoDisplayRegion",
                  DisplayRegion::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GraphicsOutput;
  friend class DisplayRegionPipelineReader;
};

#include "stereoDisplayRegion.I"

#endif
