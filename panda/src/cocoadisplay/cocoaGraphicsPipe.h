/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaGraphicsPipe.h
 * @author rdb
 * @date 2012-05-14
 */

#ifndef COCOAGRAPHICSPIPE_H
#define COCOAGRAPHICSPIPE_H

#include "pandabase.h"
#include "graphicsPipe.h"
#include "patomic.h"

#include <ApplicationServices/ApplicationServices.h>
#include <CoreVideo/CoreVideo.h>

/**
 * This graphics pipe represents the base class for pipes that create
 * windows on a Cocoa-based (e.g.  Mac OS X) client.
 */
class EXPCL_PANDA_COCOADISPLAY CocoaGraphicsPipe : public GraphicsPipe {
public:
  CocoaGraphicsPipe(CGDirectDisplayID display = CGMainDisplayID());
  virtual ~CocoaGraphicsPipe();

  virtual PreferredWindowThread get_preferred_window_thread() const;

  INLINE CGDirectDisplayID get_display_id() const;

  bool init_vsync(uint32_t &counter);
  void wait_vsync(uint32_t &counter, bool adaptive=false);

private:
  static CVReturn display_link_cb(CVDisplayLinkRef link, const CVTimeStamp *now,
                                  const CVTimeStamp *output_time,
                                  CVOptionFlags flags_in, CVOptionFlags *flags_out,
                                  void *context);

  void load_display_information();

  // This is the Quartz display identifier.
  CGDirectDisplayID _display;

  CVDisplayLinkRef _display_link = nullptr;
  patomic<int> _last_wait_frame {0};
  uint32_t _vsync_counter = 0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsPipe::init_type();
    register_type(_type_handle, "CocoaGraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cocoaGraphicsPipe.I"

#endif
