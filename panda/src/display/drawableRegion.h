// Filename: drawableRegion.h
// Created by:  drose (11Jul02)
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

#ifndef DRAWABLEREGION_H
#define DRAWABLEREGION_H

#include "pandabase.h"
#include "luse.h"
#include "renderBuffer.h"

////////////////////////////////////////////////////////////////////
//       Class : DrawableRegion
// Description : This is a base class for GraphicsWindow (actually,
//               GraphicsOutput) and DisplayRegion, both of which are
//               conceptually rectangular regions into which drawing
//               commands may be issued.  Sometimes you want to deal
//               with a single display region, and sometimes you want
//               to deal with the whole window at once, particularly
//               for issuing clear commands and capturing screenshots.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DrawableRegion {
public:
  INLINE DrawableRegion();
  INLINE DrawableRegion(const DrawableRegion &copy);
  INLINE void operator = (const DrawableRegion &copy);

  INLINE void copy_clear_settings(const DrawableRegion &copy);

PUBLISHED:
  INLINE void set_clear_color_active(bool clear_color_active);
  INLINE bool get_clear_color_active() const;

  INLINE void set_clear_depth_active(bool clear_depth_active);
  INLINE bool get_clear_depth_active() const;

  INLINE void set_clear_color(const Colorf &color);
  INLINE const Colorf &get_clear_color() const;

  INLINE void set_clear_depth(float depth);
  INLINE float get_clear_depth() const;

  INLINE bool is_any_clear_active() const;

public:
  INLINE int get_screenshot_buffer_type() const;
  INLINE int get_draw_buffer_type() const;

protected:
  int _screenshot_buffer_type;
  int _draw_buffer_type;

private:
  // This data needs to be cycled.
  enum Flags {
    F_clear_color_active = 0x0001,
    F_clear_depth_active = 0x0002,
    F_clear_all          = 0x0003, // = all of the above
  };
  int _flags;

  Colorf _clear_color;
  float _clear_depth;
};

#include "drawableRegion.I"

#endif
