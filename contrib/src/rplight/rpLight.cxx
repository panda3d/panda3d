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


#include "rpLight.h"


/**
 * @brief Constructs a new light with the given type
 * @details This constructs a new base light with the given light type.
 *   Sub-Classes should call this to initialize all properties.
 *
 * @param light_type Type of the light
 */
RPLight::RPLight(LightType light_type) {
  _light_type = light_type;
  _needs_update = false;
  _casts_shadows = false;
  _slot = -1;
  _position.fill(0);
  _color.fill(1);
  _ies_profile = -1;
  _source_resolution = 512;
  _near_plane = 0.5;
  _energy = 20.0;
}

/**
 * @brief Writes the light to a GPUCommand
 * @details This writes all of the lights data to the given GPUCommand handle.
 *   Subclasses should first call this method, and then append their own
 *   data. This makes sure that for unpacking a light, no information about
 *   the type of the light is required.
 *
 * @param cmd The GPUCommand to write to
 */
void RPLight::write_to_command(GPUCommand &cmd) {
  cmd.push_int(_light_type);
  cmd.push_int(_ies_profile);

  if (_casts_shadows) {
    // If we casts shadows, write the index of the first source, we expect
    // them to be consecutive
    nassertv(_shadow_sources.size() >= 0);
    nassertv(_shadow_sources[0]->has_slot());
    cmd.push_int(_shadow_sources[0]->get_slot());
  } else {
    // If we cast no shadows, just push a negative number
    cmd.push_int(-1);
  }

  cmd.push_vec3(_position);

  // Get the lights color by multiplying color with energy. Divide by
  // 100, since 16bit floating point buffers only go up to 65000.0, which
  // prevents very bright lights 
  cmd.push_vec3(_color * _energy / 100.0);
}

/**
 * @brief Light destructor
 * @details This destructs the light, cleaning up all resourced used. The light
 *   should be detached at this point, because while the Light is attached,
 *   the InternalLightManager holds a reference to prevent it from being
 *   destructed.
 */
RPLight::~RPLight() {
  nassertv(!has_slot()); // Light still attached - should never happen
  clear_shadow_sources();
}

/**
 * @brief Sets the lights color from a given color temperature
 * @details This sets the lights color, given a temperature. This is more
 *   physically based than setting a user defined color. The color will be
 *   computed from the given temperature.
 *
 * @param temperature Light temperature
 */
void RPLight::set_color_from_temperature(float temperature) {

  // Thanks to rdb for this conversion script
  float mm = 1000.0 / temperature;
  float mm2 = mm * mm;
  float mm3 = mm2 * mm;
  float x, y;

  if (temperature < 4000) {
    x = -0.2661239 * mm3 - 0.2343580 * mm2 + 0.8776956 * mm + 0.179910;
  } else {
    x = -3.0258469 * mm3 + 2.1070379 * mm2 + 0.2226347 * mm + 0.240390;
  }

  float x2 = x * x;
  float x3 = x2 * x;
  if (temperature < 2222) {
    y = -1.1063814 * x3 - 1.34811020 * x2 + 2.18555832 * x - 0.20219683;
  } else if (temperature < 4000) {
    y = -0.9549476 * x3 - 1.37418593 * x2 + 2.09137015 * x - 0.16748867;
  } else {
    y =  3.0817580 * x3 - 5.87338670 * x2 + 3.75112997 * x - 0.37001483;
  }

  // xyY to XYZ, assuming Y=1.
  LVecBase3 xyz(x / y, 1, (1 - x - y) / y);

  // Convert XYZ to linearized sRGB.
  const static LMatrix3 xyz_to_rgb(
   3.2406, -0.9689,  0.0557,
  -1.5372,  1.8758, -0.2050,
  -0.4986,  0.0415,  1.0570);

  set_color(xyz_to_rgb.xform(xyz));
}
