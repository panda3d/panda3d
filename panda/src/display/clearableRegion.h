// Filename: clearableRegion.h
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

#ifndef CLEARABLEREGION_H
#define CLEARABLEREGION_H

#include "pandabase.h"
#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : ClearableRegion
// Description : This is just an interface definition for a
//               rectangular region of the screen that might or might
//               not need to be cleared every frame before rendering.
//               This includes DisplayRegions and GraphicsWindows.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ClearableRegion {
public:
  INLINE ClearableRegion();
  INLINE ClearableRegion(const ClearableRegion &copy);
  INLINE void operator = (const ClearableRegion &copy);

  INLINE void copy_clear_settings(const ClearableRegion &copy);

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

private:
  // This data needs to be cycled.
  enum Flags {
    F_clear_color_active = 0x0001,
    F_clear_depth_active = 0x0002,
  };
  int _flags;

  Colorf _clear_color;
  float _clear_depth;
};

#include "clearableRegion.I"

#endif
