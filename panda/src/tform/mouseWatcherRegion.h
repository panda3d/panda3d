// Filename: mouseWatcherRegion.h
// Created by:  drose (13Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef MOUSEWATCHERREGION_H
#define MOUSEWATCHERREGION_H

#include <pandabase.h>

#include <namable.h>
#include <referenceCount.h>
#include <luse.h>

////////////////////////////////////////////////////////////////////
//       Class : MouseWatcherRegion
// Description : This is the class that defines a rectangular region
//               on the screen for the MouseWatcher.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MouseWatcherRegion : public ReferenceCount, public Namable {
public:
  INLINE MouseWatcherRegion(const string &name, float left, float right,
			    float bottom, float top);
  INLINE MouseWatcherRegion(const string &name, const LVecBase4f &frame);

  INLINE void set_frame(float left, float right, float bottom, float top);
  INLINE void set_frame(const LVecBase4f &frame);
  INLINE const LVecBase4f &get_frame() const;
  INLINE float get_area() const;

  INLINE void set_active(bool active);
  INLINE bool get_active() const;

  INLINE void set_suppress_below(bool suppress_below);
  INLINE bool get_suppress_below() const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  LVecBase4f _frame;
  float _area;

  bool _active;
  bool _suppress_below;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "MouseWatcherRegion",
		  ReferenceCount::get_class_type(),
		  Namable::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const MouseWatcherRegion &region) {
  region.output(out);
  return out;
}

#include "mouseWatcherRegion.I"

#endif
