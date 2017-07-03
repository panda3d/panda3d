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

#include "dynamicHeightfield.h"

TypeHandle DynamicHeightfield::_type_handle;

/**
* @brief Adds an observer, i.e. an instance whose on_change() will be called.
*/
void DynamicHeightfield::
add_observer(Observer *observer) {
  grutil_cat.debug() << "Adding observer " << observer << endl;
  _observers.push_back(observer);
}

/**
* @brief Removes an observer.
*/
void DynamicHeightfield::
remove_observer(Observer *observer) {
  grutil_cat.debug() << "Removing observer " << observer << endl;
  _observers.erase(remove(_observers.begin(), _observers.end(), observer), _observers.end());
}

int DynamicHeightfield::
pull_spot(const LPoint4f &delta, float xc, float yc, float xr, float yr, float exponent) {
  
  int points = PfmFile::pull_spot(delta, xc, yc, xr, yr, exponent);
  update(xc - xr, xc + xr, yc - yr, yc + yr);
  return points;
}

void DynamicHeightfield::
copy_sub_image(const PfmFile &copy, int xto, int yto, int xfrom, int yfrom, int x_size, int y_size) {
  
  PfmFile::copy_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size);
  update(xto, xto + x_size, yto, yto + y_size);
}
  
void DynamicHeightfield::
add_sub_image(const PfmFile &copy, int xto, int yto, int xfrom, int yfrom, int x_size, int y_size, float pixel_scale) {
  
  PfmFile::add_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size, pixel_scale);
  update(xto, xto + x_size, yto, yto + y_size);
}
  
void DynamicHeightfield::
mult_sub_image(const PfmFile &copy, int xto, int yto, int xfrom, int yfrom, int x_size, int y_size, float pixel_scale) {
  
  PfmFile::mult_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size, pixel_scale);
  update(xto, xto + x_size, yto, yto + y_size);
}

void DynamicHeightfield::
divide_sub_image(const PfmFile &copy, int xto, int yto, int xfrom, int yfrom, int x_size, int y_size, float pixel_scale) {
  
  PfmFile::divide_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size, pixel_scale);
  update(xto, xto + x_size, yto, yto + y_size);
}
