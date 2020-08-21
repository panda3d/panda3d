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

#ifndef IESDATASET_H
#define IESDATASET_H

#include "pandabase.h"
#include "pta_float.h"
#include "pointerToArray.h"
#include "texture.h"
#include "pnmImage.h"

NotifyCategoryDecl(iesdataset, EXPORT_CLASS, EXPORT_TEMPL);


/**
 * @brief This class generates a LUT from IES data.
 * @details This class is used by the IESLoader to generate a LUT texture which
 *   is used in the shaders to perform IES lighting. It takes a set of vertical
 *   and horizontal angles, as well as a set of candela values, which then are
 *   lineary interpolated onto a 2D LUT Texture.
 */
class IESDataset {
PUBLISHED:
  IESDataset();

  void set_vertical_angles(const PTA_float &vertical_angles);
  void set_horizontal_angles(const PTA_float &horizontal_angles);
  void set_candela_values(const PTA_float &candela_values);

  void generate_dataset_texture_into(Texture* dest_tex, size_t z) const;

public:

  float get_candela_value(float vertical_angle, float horizontal_angle) const;
  float get_candela_value_from_index(size_t vertical_angle_idx, size_t horizontal_angle_idx) const;
  float get_vertical_candela_value(size_t horizontal_angle_idx, float vertical_angle) const;

private:
  PTA_float _vertical_angles;
  PTA_float _horizontal_angles;
  PTA_float _candela_values;
};

#endif // IESDATASET_H
