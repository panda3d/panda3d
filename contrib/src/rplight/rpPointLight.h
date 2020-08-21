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

#ifndef RPPOINTLIGHT_H
#define RPPOINTLIGHT_H

#include "pandabase.h"
#include "rpLight.h"

/**
 * @brief PointLight class
 * @details This represents a point light, a light which has a position and
 *   radius. Checkout the RenderPipeline documentation for more information
 *   about this type of light.
 */
class RPPointLight : public RPLight {
PUBLISHED:
  RPPointLight();

  inline void set_radius(float radius);
  inline float get_radius() const;
  MAKE_PROPERTY(radius, get_radius, set_radius);

  inline void set_inner_radius(float inner_radius);
  inline float get_inner_radius() const;
  MAKE_PROPERTY(inner_radius, get_inner_radius, set_inner_radius);

public:
  virtual void write_to_command(GPUCommand &cmd);
  virtual void update_shadow_sources();
  virtual void init_shadow_sources();

protected:
  float _radius;
  float _inner_radius;
};

#include "rpPointLight.I"

#endif // RPPOINTLIGHT_H
