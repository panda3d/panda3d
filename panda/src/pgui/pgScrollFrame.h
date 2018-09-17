/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgScrollFrame.h
 * @author drose
 * @date 2005-08-17
 */

#ifndef PGSCROLLFRAME_H
#define PGSCROLLFRAME_H

#include "pandabase.h"

#include "pgVirtualFrame.h"
#include "pgSliderBarNotify.h"
#include "pgSliderBar.h"

#ifdef PHAVE_ATOMIC
#include <atomic>
#endif

/**
 * This is a special kind of frame that pretends to be much larger than it
 * actually is.  You can scroll through the frame, as if you're looking
 * through a window at the larger frame beneath.  All children of this frame
 * node are scrolled and clipped as if they were children of the larger,
 * virtual frame.
 *
 * This is implemented as a specialization of PGVirtualFrame, which handles
 * the meat of the virtual canvas.  This class adds automatic support for
 * scroll bars, and restricts the virtual transform to translate only (no
 * scale or rotate).
 */
class EXPCL_PANDA_PGUI PGScrollFrame : public PGVirtualFrame, public PGSliderBarNotify {
PUBLISHED:
  explicit PGScrollFrame(const std::string &name = "");
  virtual ~PGScrollFrame();

protected:
  PGScrollFrame(const PGScrollFrame &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual void xform(const LMatrix4 &mat);

PUBLISHED:
  void setup(PN_stdfloat width, PN_stdfloat height,
             PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top,
             PN_stdfloat slider_width, PN_stdfloat bevel);

  INLINE void set_virtual_frame(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top);
  INLINE void set_virtual_frame(const LVecBase4 &virtual_frame);
  INLINE const LVecBase4 &get_virtual_frame() const;
  INLINE bool has_virtual_frame() const;
  INLINE void clear_virtual_frame();

  INLINE void set_manage_pieces(bool manage_pieces);
  INLINE bool get_manage_pieces() const;

  INLINE void set_auto_hide(bool auto_hide);
  INLINE bool get_auto_hide() const;

  INLINE void set_horizontal_slider(PGSliderBar *horizontal_slider);
  INLINE void clear_horizontal_slider();
  INLINE PGSliderBar *get_horizontal_slider() const;

  INLINE void set_vertical_slider(PGSliderBar *vertical_slider);
  INLINE void clear_vertical_slider();
  INLINE PGSliderBar *get_vertical_slider() const;

  void remanage();
  INLINE void recompute();

protected:
  virtual void frame_changed();

  virtual void item_transform_changed(PGItem *item);
  virtual void item_frame_changed(PGItem *item);
  virtual void item_draw_mask_changed(PGItem *item);
  virtual void slider_bar_adjust(PGSliderBar *slider_bar);

private:
  void recompute_clip();

  void recompute_canvas();
  PN_stdfloat interpolate_canvas(PN_stdfloat clip_min, PN_stdfloat clip_max,
                           PN_stdfloat canvas_min, PN_stdfloat canvas_max,
                           PGSliderBar *slider_bar);

private:
  bool _needs_remanage;
  bool _needs_recompute_clip;
  std::atomic_flag _canvas_computed;

  bool _has_virtual_frame;
  LVecBase4 _virtual_frame;

  bool _manage_pieces;
  bool _auto_hide;

  PT(PGSliderBar) _horizontal_slider;
  PT(PGSliderBar) _vertical_slider;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PGVirtualFrame::init_type();
    register_type(_type_handle, "PGScrollFrame",
                  PGVirtualFrame::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "pgScrollFrame.I"

#endif
