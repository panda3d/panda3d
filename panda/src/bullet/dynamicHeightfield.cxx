/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dynamicHeightfield.cxx
 * @author wolfgangp
 * @date 2016-05-23
 */

#include "dynamicHeightfield.h"

/**
 * @brief Sets a ShaderTerrainMesh instance to propagate changes to.
 * @details If the ShaderTerrainMesh doesn't have a heightfield set already,
 *  the current one will be set on it and generate() called.
 */
void DynamicHeightfield::
set_listener(ShaderTerrainMesh *stm) {

  if (_x_size == 0 || _y_size == 0) {
    bullet_cat.error() << "No heightfield set! Can't set listener." << endl;
    return;
  }
  // If no (suitable) heightfield is currently set on ShaderTerrainMesh, set the current one.
  if (!stm->get_heightfield()) {
    Texture *tex = new Texture("STM_dynamic_heightfield");
    // it seems an intermediate PNMImage is required to load a PfmFile as a Texture with "unsigned short" component type.
    PNMImage *intermediate = new PNMImage();
    this->store(*intermediate);
    tex->load(*intermediate);
    stm->set_heightfield(tex);
    if (!stm->generate()) {
      bullet_cat.error() << "Can't use current heightfield with ShaderTerrainMesh!" << endl;
      return;
    }
  } else {  // a heightfield is set on STM already, check it for size.
    Texture *tex = stm->get_heightfield();
    if (_x_size != tex->get_x_size() || _y_size != tex->get_y_size()) {
      bullet_cat.error() << "Size of ShaderTerrainMesh heightfield doesn't match current heightfield!" << endl;
      return;
    }
  }

  _stm_ptr = stm;
}

/**
 * @brief Sets a BulletHeightfieldShape instance to propagate changes to.
 * @details The BulletHeightfieldShape should be constructed using the same
 *  heightfield as the current one.
 */
void DynamicHeightfield::
set_listener(BulletHeightfieldShape *bhfs) {

  if (_x_size == 0 || _y_size == 0) {
    bullet_cat.error() << "No heightfield set! Can't set listener." << endl;
    return;
  }
  // If the BulletHeightfieldShape was not constructed with a heightfield of correct size, abort.
  if (bhfs->get_y_size() != _x_size + 1 || bhfs->get_x_size() != _y_size + 1) {
    bullet_cat.error() << "Can't use current heightfield with BulletHeightfieldShape!" << endl;
    return;
  }

  _bhfs_ptr = bhfs;
}

int DynamicHeightfield::
pull_spot(const LPoint4f &delta, float xc, float yc, float xr, float yr, float exponent) {
  
  int points = PfmFile::pull_spot(delta, xc, yc, xr, yr, exponent);
  make_update_region(xc - xr, xc + xr, yc - yr, yc + yr);
  update();
  return points;
}

void DynamicHeightfield::
copy_sub_image(const PfmFile &copy, int xto, int yto, int xfrom, int yfrom, int x_size, int y_size) {
  
  PfmFile::copy_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size);
  make_update_region(xto, xto + x_size, yto, yto + y_size);
  update();
}
  
void DynamicHeightfield::
add_sub_image(const PfmFile &copy, int xto, int yto, int xfrom, int yfrom, int x_size, int y_size, float pixel_scale) {
  
  PfmFile::add_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size, pixel_scale);
  make_update_region(xto, xto + x_size, yto, yto + y_size);
  update();
}
  
void DynamicHeightfield::
mult_sub_image(const PfmFile &copy, int xto, int yto, int xfrom, int yfrom, int x_size, int y_size, float pixel_scale) {
  
  PfmFile::mult_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size, pixel_scale);
  make_update_region(xto, xto + x_size, yto, yto + y_size);
  update();
}

void DynamicHeightfield::
divide_sub_image(const PfmFile &copy, int xto, int yto, int xfrom, int yfrom, int x_size, int y_size, float pixel_scale) {
  
  PfmFile::divide_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size, pixel_scale);
  make_update_region(xto, xto + x_size, yto, yto + y_size);
  update();
}
