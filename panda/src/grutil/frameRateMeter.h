/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file frameRateMeter.h
 * @author drose
 * @date 2003-12-23
 */

#ifndef FRAMERATEMETER_H
#define FRAMERATEMETER_H

#include "pandabase.h"
#include "textNode.h"
#include "nodePath.h"
#include "graphicsOutput.h"
#include "displayRegion.h"
#include "pointerTo.h"
#include "pStatCollector.h"

class GraphicsChannel;
class ClockObject;

/**
 * This is a special TextNode that automatically updates itself with the
 * current frame rate.  It can be placed anywhere in the world where you'd
 * like to see the frame rate.
 *
 * It also has a special mode in which it may be attached directly to a
 * channel or window.  If this is done, it creates a DisplayRegion for itself
 * and renders itself in the upper-right-hand corner.
 */
class EXPCL_PANDA_GRUTIL FrameRateMeter : public TextNode {
PUBLISHED:
  explicit FrameRateMeter(const std::string &name);
  virtual ~FrameRateMeter();

  void setup_window(GraphicsOutput *window);
  void clear_window();

  INLINE GraphicsOutput *get_window() const;
  INLINE DisplayRegion *get_display_region() const;

  INLINE void set_update_interval(double update_interval);
  INLINE double get_update_interval() const;

  INLINE void set_text_pattern(const std::string &text_pattern);
  INLINE const std::string &get_text_pattern() const;

  INLINE void set_clock_object(ClockObject *clock_object);
  INLINE ClockObject *get_clock_object() const;

  INLINE void update();

protected:
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

private:
  void do_update(Thread *current_thread);

private:
  PT(GraphicsOutput) _window;
  PT(DisplayRegion) _display_region;
  NodePath _root;

  bool _show_milliseconds;
  double _update_interval;
  double _last_update;
  std::string _text_pattern;
  ClockObject *_clock_object;

  PN_stdfloat _last_aspect_ratio;
  CPT(TransformState) _aspect_ratio_transform;

  static PStatCollector _show_fps_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextNode::init_type();
    register_type(_type_handle, "FrameRateMeter",
                  TextNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "frameRateMeter.I"

#endif
