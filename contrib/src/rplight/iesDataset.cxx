/**
 *
 * RenderPipeline
 *
 * Copyright (c) 2014-2016 tobspr <tobias.springer1@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */


#include "iesDataset.h"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

NotifyCategoryDef(iesdataset, "")

/**
 * @brief Constructs a new empty dataset.
 * @details This constructs a new IESDataset with no data set.
 */
IESDataset::IESDataset() {
}

/**
 * @brief Sets the vertical angles of the dataset.
 * @details This sets the list of vertical angles of the dataset.
 *
 * @param vertical_angles Vector of all vertical angles.
 */
void IESDataset::set_vertical_angles(const PTA_float &vertical_angles) {
  nassertv(vertical_angles.size() > 0);
	_vertical_angles = vertical_angles;
}

/**
 * @brief Sets the horizontal angles of the dataset.
 * @details This sets the list of horizontal angles of the dataset.
 *
 * @param horizontal_angles Vector of all horizontal angles.
 */
void IESDataset::set_horizontal_angles(const PTA_float &horizontal_angles) {
  nassertv(horizontal_angles.size() > 0);
	_horizontal_angles = horizontal_angles;
}

/**
 * @brief Sets the candela values.
 * @details This sets the candela values of the dataset. They should be an
 *   interleaved 2D array with the dimensions vertical_angles x horizontal_angles.
 *   They also should be normalized by dividing by the maximum entry.
 * @param candela_values Interleaved 2D-vector of candela values.
 */
void IESDataset::set_candela_values(const PTA_float &candela_values) {
	nassertv(candela_values.size() == _horizontal_angles.size() * _vertical_angles.size());
	_candela_values = candela_values;
}

/**
 * @brief Internal method to access the candela data.
 * @details This lookups a candela value in the candela values. It converts a
 *   two dimensional index to a onedimensional index and then returns the candela
 *   value at that position.
 *
 * @param vertical_angle_idx Index of the vertical angle
 * @param horizontal_angle_idx Index of the horizontal angle
 *
 * @return Candela value between 0 .. 1
 */
float IESDataset::get_candela_value_from_index(size_t vertical_angle_idx, size_t horizontal_angle_idx) const {
	size_t index = vertical_angle_idx + horizontal_angle_idx * _vertical_angles.size();
	nassertr(index >= 0 && index < _candela_values.size(), 0.0);
	return _candela_values[index];
}

/**
 * @brief Samples the dataset at the given position
 * @details This looks up a value in the dataset, by specifying a horizontal and
 *   vertical angle. This is used for generating the LUT. The vertical and horizontal
 *   angle should be inside of the bounds of the vertical and horizontal angle arrays.
 *
 * @param vertical_angle Vertical angle, from 0 .. 90 or 0 .. 180 depending on the dataset
 * @param horizontal_angle Horizontal angle, from 0 .. 180 or 0 .. 360 depending on the dataset.
 *
 * @return Candela value between 0 .. 1
 */
float IESDataset::get_candela_value(float vertical_angle, float horizontal_angle) const {

  // Special case for datasets without horizontal angles
  if (_horizontal_angles.size() == 1) {
    return get_vertical_candela_value(0, vertical_angle);
  }

  float max_angle = _horizontal_angles[_horizontal_angles.size() - 1];

  // Wrap angle to fit from 0 .. 360 degree. Most profiles only distribute
  // candela values from 0 .. 180 or even 0 .. 90. We have to mirror the
  // values at those borders (so 2 times for 180 degree and 4 times for 90 degree)
  horizontal_angle = fmod(horizontal_angle, 2.0f * max_angle);
  if (horizontal_angle > max_angle) {
    horizontal_angle = 2.0 * max_angle - horizontal_angle;
  }

  // Simlar to the vertical step, we now try interpolating a horizontal angle,
  // but we need to evaluate the vertical value for each row instead of fetching
  // the value directly
  for (size_t horizontal_index = 1; horizontal_index < _horizontal_angles.size(); ++horizontal_index) {
    float curr_angle = _horizontal_angles[horizontal_index];

    if (curr_angle >= horizontal_angle) {

      // Get previous angle data
      float prev_angle = _horizontal_angles[horizontal_index - 1];
      float prev_value = get_vertical_candela_value(horizontal_index - 1, vertical_angle);
      float curr_value = get_vertical_candela_value(horizontal_index, vertical_angle);

      // Interpolate lineary
      float lerp = (horizontal_angle - prev_angle) / (curr_angle - prev_angle);

      // Should never occur, but to be safe:
      if (lerp < 0.0 || lerp > 1.0) {
        iesdataset_cat.error() << "Invalid horizontal lerp: " << lerp
                     << ", requested angle was " << horizontal_angle
                     << ", prev = " << prev_angle << ", cur = " << curr_angle
                     << std::endl;
      }

      return curr_value * lerp + prev_value * (1-lerp);
    }
  }

  return 0.0;
}

/**
 * @brief Fetches a vertical candela value
 * @details Fetches a vertical candela value, using a given horizontal position.
 *   This does an 1D interpolation in the candela values array.
 *
 * @param horizontal_angle_idx The index of the horizontal angle in the horizontal
 *   angle array.
 * @param vertical_angle The vertical angle. Interpolation will be done if the
 *   vertical angle is not in the vertical angles array.
 *
 * @return Candela value between 0 .. 1
 */
float IESDataset::get_vertical_candela_value(size_t horizontal_angle_idx, float vertical_angle) const {
  nassertr(horizontal_angle_idx >= 0 && horizontal_angle_idx < _horizontal_angles.size(), 0.0);

  // Lower bound
  if (vertical_angle < 0.0) return 0.0;

  // Upper bound
  if (vertical_angle > _vertical_angles[_vertical_angles.size() - 1] ) return 0.0;

  // Find lowest enclosing angle
  for (size_t vertical_index = 1; vertical_index < _vertical_angles.size(); ++vertical_index) {
    float curr_angle = _vertical_angles[vertical_index];

    // Found value
    if (curr_angle > vertical_angle) {

      // Get previous angle data
      float prev_angle = _vertical_angles[vertical_index - 1];
      float prev_value = get_candela_value_from_index(vertical_index - 1, horizontal_angle_idx);
      float curr_value = get_candela_value_from_index(vertical_index, horizontal_angle_idx);

      // Interpolate lineary
      float lerp = (vertical_angle - prev_angle) / (curr_angle - prev_angle);

      // Should never occur, but to be safe:
      if (lerp < 0.0 || lerp > 1.0) {
        iesdataset_cat.error() << "ERROR: Invalid vertical lerp: " << lerp
                     << ", requested angle was " << vertical_angle
                     << ", prev = " << prev_angle << ", cur = " << curr_angle
                     << std::endl;
      }

      return curr_value * lerp + prev_value * (1-lerp);
    }
  }
  return 0.0;
}

/**
 * @brief Generates the IES LUT
 * @details This generates the LUT into a given dataset texture. The x-axis
 *   referes to the vertical_angle, whereas the y-axis refers to the
 *   horizontal angle.
 *
 * @param dest_tex Texture to write the LUT into
 * @param z Layer to write the LUT into, in case the texture is a 3D Texture or
 *   2D Texture Array.
 */
void IESDataset::generate_dataset_texture_into(Texture* dest_tex, size_t z) const {

  size_t resolution_vertical = dest_tex->get_y_size();
  size_t resolution_horizontal = dest_tex->get_x_size();

  // Candla values are stored flippped - vertical angles in the x - Axis
  // and horizontal angles in the y - Axis
  PNMImage dest = PNMImage(resolution_vertical, resolution_horizontal, 1, 65535);

  for (size_t vert = 0; vert < resolution_vertical; ++vert) {
    for (size_t horiz = 0; horiz < resolution_horizontal; ++horiz) {
      float vert_angle = (float)vert / (float)(resolution_vertical-1);
      vert_angle = cos(vert_angle * M_PI) * 90.0 + 90.0;
      float horiz_angle = (float)horiz / (float)(resolution_horizontal-1) * 360.0;
      float candela = get_candela_value(vert_angle, horiz_angle);
      dest.set_xel(vert, horiz, candela);
    }
  }


  dest_tex->load(dest, z, 0);
}
