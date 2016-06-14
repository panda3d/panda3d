/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @author wolfgangp
 * @date 2016-05-23
 */

#ifndef DYNAMIC_HEIGHTFIELD_H
#define DYNAMIC_HEIGHTFIELD_H

#include "pandabase.h"
#include "pfmFile.h"

/**
 * This class holds a heightfield and propagates changes to it to classes which
 * manage visualisation, collision, etc.
 */
class EXPCL_PANDA_GRUTIL DynamicHeightfield : public PfmFile, public TypedWritableReferenceCount {

PUBLISHED:
  INLINE DynamicHeightfield();
  INLINE ~DynamicHeightfield();

  class Observer {
    public: virtual void on_change()=0;
  };

  INLINE void update(int lower_x, int upper_x, int lower_y, int upper_y);
  INLINE void update(float lower_x, float upper_x, float lower_y, float upper_y);

  BLOCKING int pull_spot(const LPoint4f &delta, float xc, float yc, float xr, float yr, float exponent);
  void copy_sub_image(const PfmFile &copy, int xto, int yto, int xfrom, int yfrom,
                      int x_size, int y_size);
  void add_sub_image(const PfmFile &copy, int xto, int yto, int xfrom, int yfrom,
                     int x_size, int y_size, float pixel_scale = 1.0);
  void mult_sub_image(const PfmFile &copy, int xto, int yto, int xfrom, int yfrom,
                     int x_size, int y_size, float pixel_scale = 1.0);
  void divide_sub_image(const PfmFile &copy, int xto, int yto, int xfrom, int yfrom,
                     int x_size, int y_size, float pixel_scale = 1.0);

public:
  void add_observer(Observer *observer);
  void remove_observer(Observer *observer);
  LVector4i region_corners;

private:
  pvector<Observer*> _observers;

// Type handle stuff
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "DynamicHeightfield", TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "dynamicHeightfield.I"

#endif //DYNAMIC_HEIGHTFIELD_H
