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


/**
 * @brief Sets the radius of the light
 * @details This sets the radius of the light. It controls the lights
 *   influence. After a distance greater than this radius, the light influence
 *   is zero.
 *
 * @param radius Light radius in world space
 */
inline void RPPointLight::set_radius(float radius) {
  nassertv(radius > 0); // Invalid light radius
  _radius = radius;
  set_needs_update(true);
  invalidate_shadows();
}

/**
 * @brief Returns the lights radius
 * @details This returns the lights radius previously set with
 *   RPPointLight::set_radius
 * @return Light radius in world space
 */
inline float RPPointLight::get_radius() const {
  return _radius;
}

/**
 * @brief Sets the inner radius of the light
 * @details This sets the inner radius of the light. Anything greater than
 *   zero causes the light to get an area light. This has influence on the
 *   specular highlights of the light aswell as the shadows.
 *
 *   The inner radius controls the size of the lights sphere size in world
 *   space units. A radius of 0 means the light has no inner radius, and the
 *   light will be have like an infinite small point light source.
 *   A radius greater than zero will cause the light to behave like it would be
 *   an emissive sphere with the given inner radius emitting light. This is
 *   more physically correct.
 *
 * @param inner_radius Inner-radius in world space
 */
inline void RPPointLight::set_inner_radius(float inner_radius) {
  nassertv(inner_radius >= 0.01); // Invalid inner radius
  _inner_radius = inner_radius;
  set_needs_update(true);
}

/**
 * @brief Returns the inner radius of the light
 * @details This returns the inner radius of the light, previously set with
 *   RPPointLight::get_inner_radius.
 * @return [description]
 */
inline float RPPointLight::get_inner_radius() const {
  return _inner_radius;
}
