/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggRenderMode.h
 * @author drose
 * @date 1999-01-20
 */

#ifndef EGGRENDERMODE_H
#define EGGRENDERMODE_H

#include "pandabase.h"
#include "typedObject.h"


/**
 * This class stores miscellaneous rendering properties that is associated
 * with geometry, and which may be set on the geometry primitive level, on the
 * group above it, or indirectly via a texture.  It's intended to be a base
 * class for egg objects that can have these properties set.
 *
 * This class cannot inherit from EggObject, because it causes problems at the
 * EggPolygon level with multiple appearances of the EggObject base class.
 * And making EggObject a virtual base class is just no fun.
 */
class EXPCL_PANDA_EGG EggRenderMode {
PUBLISHED:
  EggRenderMode();
  INLINE EggRenderMode(const EggRenderMode &copy);
  EggRenderMode &operator = (const EggRenderMode &copy);

  void write(std::ostream &out, int indent_level) const;

  enum AlphaMode {  // Specifies implementation of transparency.
    AM_unspecified,
    AM_off,     // No transparency.
    AM_on,      // Use whatever the default model is.
    AM_blend,   // Normal alpha blending, e.g. TransparencyAttrib::M_alpha.
    AM_blend_no_occlude,  // Alpha blending w/o depth write.
    AM_ms,      // TransparencyAttrib::M_multisample
    AM_ms_mask, // TransparencyAttrib::M_multisample_mask
    AM_binary,  // TransparencyAttrib::M_binary
    AM_dual,    // TransparencyAttrib::M_dual
    AM_premultiplied // TransparencyAttrib::M_premultiplied_alpha
  };

  enum DepthWriteMode {
    DWM_unspecified, DWM_off, DWM_on
  };

  enum DepthTestMode {
    DTM_unspecified, DTM_off, DTM_on
  };

  enum VisibilityMode {
    VM_unspecified, VM_hidden, VM_normal
  };

  INLINE void set_alpha_mode(AlphaMode mode);
  INLINE AlphaMode get_alpha_mode() const;

  INLINE void set_depth_write_mode(DepthWriteMode mode);
  INLINE DepthWriteMode get_depth_write_mode() const;

  INLINE void set_depth_test_mode(DepthTestMode mode);
  INLINE DepthTestMode get_depth_test_mode() const;

  INLINE void set_visibility_mode(VisibilityMode mode);
  INLINE VisibilityMode get_visibility_mode() const;

  INLINE void set_depth_offset(int bias);
  INLINE int get_depth_offset() const;
  INLINE bool has_depth_offset() const;
  INLINE void clear_depth_offset();

  INLINE void set_draw_order(int order);
  INLINE int get_draw_order() const;
  INLINE bool has_draw_order() const;
  INLINE void clear_draw_order();

  INLINE void set_bin(const std::string &bin);
  INLINE std::string get_bin() const;
  INLINE bool has_bin() const;
  INLINE void clear_bin();

  // Comparison operators are handy.
  bool operator == (const EggRenderMode &other) const;
  INLINE bool operator != (const EggRenderMode &other) const;
  bool operator < (const EggRenderMode &other) const;

  static AlphaMode string_alpha_mode(const std::string &string);
  static DepthWriteMode string_depth_write_mode(const std::string &string);
  static DepthTestMode string_depth_test_mode(const std::string &string);
  static VisibilityMode string_visibility_mode(const std::string &string);

private:
  AlphaMode _alpha_mode;
  DepthWriteMode _depth_write_mode;
  DepthTestMode _depth_test_mode;
  VisibilityMode _visibility_mode;
  int _depth_offset;
  bool _has_depth_offset;
  int _draw_order;
  bool _has_draw_order;
  std::string _bin;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "EggRenderMode");
  }

private:
  static TypeHandle _type_handle;
};

EXPCL_PANDA_EGG std::ostream &operator << (std::ostream &out, EggRenderMode::AlphaMode mode);
EXPCL_PANDA_EGG std::istream &operator >> (std::istream &in, EggRenderMode::AlphaMode &mode);

EXPCL_PANDA_EGG std::ostream &operator << (std::ostream &out, EggRenderMode::DepthWriteMode mode);
EXPCL_PANDA_EGG std::ostream &operator << (std::ostream &out, EggRenderMode::DepthTestMode mode);
EXPCL_PANDA_EGG std::ostream &operator << (std::ostream &out, EggRenderMode::VisibilityMode mode);

#include "eggRenderMode.I"

#endif
