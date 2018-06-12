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


#include "rpSpotLight.h"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>


/**
 * @brief Creates a new spot light
 * @details This creates a new spot light with default properties set. You should
 *   set at least a direction, fov, radius and position to make the light useful.
 */
RPSpotLight::RPSpotLight() : RPLight(RPLight::LT_spot_light) {
  _radius = 10.0;
  _fov = 45.0;
  _direction.set(0, 0, -1);
}

/**
 * @brief Writes the light to a GPUCommand
 * @details This writes the spot light data to a GPUCommand.
 * @see RPLight::write_to_command
 *
 * @param cmd The target GPUCommand
 */
void RPSpotLight::write_to_command(GPUCommand &cmd) {
  RPLight::write_to_command(cmd);
  cmd.push_float(_radius);

  // Encode FOV as cos(fov)
  cmd.push_float(cos(_fov / 360.0 * M_PI));
  cmd.push_vec3(_direction);
}

/**
 * @brief Inits the shadow sources of the light
 * @details This inits all required shadow sources for the spot light.
 * @see RPLight::init_shadow_sources
 */
void RPSpotLight::init_shadow_sources() {
  nassertv(_shadow_sources.size() == 0);
  _shadow_sources.push_back(new ShadowSource());
}

/**
 * @brief Updates the shadow sources
 * @details This updates all shadow sources of the light.
 * @see RPLight::update_shadow_sources
 */
void RPSpotLight::update_shadow_sources() {
  _shadow_sources[0]->set_resolution(get_shadow_map_resolution());
  _shadow_sources[0]->set_perspective_lens(_fov, _near_plane, _radius, _position, _direction);
}

