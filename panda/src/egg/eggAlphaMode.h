// Filename: eggAlphaMode.h
// Created by:  drose (20Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGALPHAMODE_H
#define EGGALPHAMODE_H

#include <pandabase.h>

#include <typeHandle.h>

#include <string>


////////////////////////////////////////////////////////////////////
// 	 Class : EggAlphaMode
// Description : This is a base class for things that can have the
//               alpha mode and draw order set on them.  This includes
//               textures, primitives, and groups.
//
//               This class cannot inherit from EggObject, because it
//               causes problems at the EggPolygon level with multiple
//               appearances of the EggObject base class.  And making
//               EggObject a virtual base class is just no fun.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggAlphaMode {
public:
  INLINE EggAlphaMode();
  INLINE EggAlphaMode(const EggAlphaMode &copy);
  INLINE EggAlphaMode &operator = (const EggAlphaMode &copy);

  void write(ostream &out, int indent_level) const;

  enum AlphaMode {
    AM_unspecified, AM_off, AM_on,
    AM_blend, AM_blend_no_occlude, AM_ms, AM_ms_mask
  };

  INLINE void set_alpha_mode(AlphaMode mode);
  INLINE AlphaMode get_alpha_mode() const;

  INLINE void set_draw_order(double order);
  INLINE double get_draw_order() const;
  INLINE bool has_draw_order() const;
  INLINE void clear_draw_order();

  // Comparison operators are handy.
  bool operator == (const EggAlphaMode &other) const;
  INLINE bool operator != (const EggAlphaMode &other) const;
  bool operator < (const EggAlphaMode &other) const;

  static AlphaMode string_alpha_mode(const string &string);

private:
  AlphaMode _alpha_mode;
  double _draw_order;
  bool _has_draw_order;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "EggAlphaMode");
  }

private:
  static TypeHandle _type_handle;
};

ostream &operator << (ostream &out, EggAlphaMode::AlphaMode mode);

#include "eggAlphaMode.I"

#endif

