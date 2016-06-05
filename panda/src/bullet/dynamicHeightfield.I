/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dynamicHeightfield.I
 * @author wolfgangp
 * @date 2016-05-23
 */

/**
 *
 */
INLINE DynamicHeightfield::
DynamicHeightfield() : PfmFile() {
}

/**
 *
 */
INLINE DynamicHeightfield::
~DynamicHeightfield() {
}

/**
 * @brief Validates that given rows and columns exist in the heightfield.
 */
INLINE void DynamicHeightfield::
make_update_region(int lower_x, int upper_x, int lower_y, int upper_y) {
  
  // bullet_cat.debug() << "make_update_region: " << lower_x << " " << upper_x  << " "  << lower_y << " " << upper_y << endl;
  // nassertv(lower_x >= 0 && lower_x < _x_size && upper_x >= 0 && upper_x < _x_size && lower_y >= 0 && lower_y < _y_size && upper_y >= 0 && upper_y < _y_size);
  if (lower_x < 0) {
    bullet_cat.debug() << "Adjusted lower_x to 0!" << endl;
    lower_x = 0;
  }
  if (lower_x >= _x_size) {
    bullet_cat.debug() << "Adjusted lower_x to x_size-1!" << endl;
    lower_x = _x_size-1;
  }
  if (upper_x < 0) {
    bullet_cat.debug() << "Adjusted upper_x to 0!" << endl;
    upper_x = 0;
  }
  if (upper_x >= _x_size) {
    bullet_cat.debug() << "Adjusted upper_x to x_size-1!" << endl;
    upper_x = _x_size-1;
  }
  if (lower_y < 0) {
    bullet_cat.debug() << "Adjusted lower_y to 0!" << endl;
    lower_y = 0;
  }
  if (lower_y >= _y_size) {
    bullet_cat.debug() << "Adjusted lower_y to y_size-1!" << endl;
    lower_y = _y_size-1;
  }
  if (upper_y < 0) {
    bullet_cat.debug() << "Adjusted upper_y to 0!" << endl;
    upper_y = 0;
  }
  if (upper_y >= _y_size) {
    bullet_cat.debug() << "Adjusted upper_y to y_size-1!" << endl;
    upper_y = _y_size-1;
  }
  
  // Ensure values are in 0..1 range.
  for (int row = lower_y; row <= upper_y; row++) {
    for (int col = lower_x; col <= upper_x; col++) {
      PN_stdfloat p = get_point1(col, row);
      nassertv(p >= 0.0 && p <= 1.0);
    }
  }
  _region_corners = LVector4i(lower_x, upper_x, lower_y, upper_y);
}

/**
 * @brief Validates that given rows and columns exist in the heightfield.
 */
INLINE void DynamicHeightfield::
make_update_region(float lower_x, float upper_x, float lower_y, float upper_y) {
  
  // bullet_cat.debug() << "make_update_region float: " << lower_x << " " << upper_x  << " "  << lower_y << " " << upper_y << endl;
  make_update_region((int)lower_x, (int)ceil(upper_x), (int)lower_y, (int)ceil(upper_y));
}

/**
 * @brief Propagates updates to listeners set with set_listener.
 */
INLINE void DynamicHeightfield::
update() {

  bool result;
  if (_bhfs_ptr) {
    result = _bhfs_ptr->update_region(_region_corners, *this);
    if (!result) {
      bullet_cat.error() << "Updating BulletHeightfieldShape failed." << endl;      
    }
  }
  if (_stm_ptr) {
    result = _stm_ptr->update_region(_region_corners, *this);
    if (!result) {
      bullet_cat.error() << "Updating ShaderTerrainMesh failed." << endl;      
    }
  }
  _region_corners.fill(0);
}