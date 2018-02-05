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



inline void RPSpotLight::set_radius(float radius) {
  _radius = radius;
  set_needs_update(true);
  invalidate_shadows();
}

inline float RPSpotLight::get_radius() const {
  return _radius;
}


inline void RPSpotLight::set_fov(float fov) {
  _fov = fov;
  set_needs_update(true);
  invalidate_shadows();
}

inline float RPSpotLight::get_fov() const {
  return _fov;
}

inline void RPSpotLight::set_direction(LVecBase3 direction) {
  _direction = direction;
  _direction.normalize();
  set_needs_update(true);
  invalidate_shadows();
}

inline void RPSpotLight::set_direction(float dx, float dy, float dz) {
  _direction.set(dx, dy, dz);
  _direction.normalize();
  set_needs_update(true);
  invalidate_shadows();
}

inline const LVecBase3& RPSpotLight::get_direction() const {
  return _direction;
}

inline void RPSpotLight::look_at(LVecBase3 point) {
  set_direction(point - _position);
}

inline void RPSpotLight::look_at(float x, float y, float z) {
  set_direction(LVecBase3(x, y, z) - _position);
}
