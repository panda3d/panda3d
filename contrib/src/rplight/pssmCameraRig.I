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
 * @brief Sets the maximum pssm distance.
 * @details This sets the maximum distance in world space until which shadows
 *   are rendered. After this distance, no shadows will be rendered.
 *
 *   If the distance is below zero, an assertion is triggered.
 *
 * @param distance Maximum distance in world space
 */
inline void PSSMCameraRig::set_pssm_distance(float distance) {
  nassertv(distance > 0.0 && distance < 100000.0);
  _pssm_distance = distance;
}

/**
 * @brief Sets the suns distance
 * @details This sets the distance the cameras will have from the cameras frustum.
 *   This prevents far objects from having no shadows, which can occur when these
 *   objects are between the cameras frustum and the sun, but not inside of the
 *   cameras frustum. Setting the sun distance high enough will move the cameras
 *   away from the camera frustum, being able to cover those distant objects too.
 *
 *   If the sun distance is set too high, artifacts will occur due to the reduced
 *   range of depth. If a value below zero is passed, an assertion will get
 *   triggered.
 *
 * @param distance The sun distance
 */
inline void PSSMCameraRig::set_sun_distance(float distance) {
  nassertv(distance > 0.0 && distance < 100000.0);
  _sun_distance = distance;
}

/**
 * @brief Sets the logarithmic factor
 * @details This sets the logarithmic factor, which is the core of the algorithm.
 *   PSSM splits the camera frustum based on a linear and a logarithmic factor.
 *   While a linear factor provides a good distribution, it often is not applicable
 *   for wider distances. A logarithmic distribution provides a better distribution
 *   at distance, but suffers from splitting in the near areas.
 *
 *   The logarithmic factor mixes the logarithmic and linear split distribution,
 *   to get the best of both. A greater factor will make the distribution more
 *   logarithmic, while a smaller factor will make it more linear.
 *
 *   If the factor is below zero, an ssertion is triggered.
 *
 * @param factor The logarithmic factor
 */
inline void PSSMCameraRig::set_logarithmic_factor(float factor) {
  nassertv(factor > 0.0);
  _logarithmic_factor = factor;
}

/**
 * @brief Sets whether to use a fixed film size
 * @details This controls if a fixed film size should be used. This will cause
 *   the camera rig to cache the current film size, and only change it in case
 *   it gets too small. This provides less flickering when moving, because the
 *   film size will stay roughly constant. However, to prevent the cached film
 *   size getting too big, one should call PSSMCameraRig::reset_film_size
 *   once in a while, otherwise there might be a lot of wasted space.
 *
 * @param flag Whether to use a fixed film size
 */
inline void PSSMCameraRig::set_use_fixed_film_size(bool flag) {
  _use_fixed_film_size = flag;
}

/**
 * @brief Sets the resolution of each split
 * @details This sets the resolution of each split. Currently it is equal for
 *   each split. This is required when using PSSMCameraRig::set_use_stable_csm,
 *   to compute how bix a texel is.
 *
 *   It has to match the y-resolution of the pssm shadow map. If an invalid
 *   resolution is triggered, an assertion is thrown.
 *
 * @param resolution The resolution of each split.
 */
inline void PSSMCameraRig::set_resolution(size_t resolution) {
  nassertv(resolution >= 0 && resolution < 65535);
  _resolution = resolution;
}

/**
 * @brief Sets whether to use stable CSM snapping.
 * @details This option controls if stable CSM snapping should be used. When the
 *   option is enabled, all splits will snap to their texels, so that when moving,
 *   no flickering will occur. However, this only works when the splits do not
 *   change their film size, rotation and angle.
 *
 * @param flag Whether to use stable CSM snapping
 */
inline void PSSMCameraRig::set_use_stable_csm(bool flag) {
  _use_stable_csm = flag;
}

/**
 * @brief Sets the border bias for each split
 * @details This sets the border bias for every split. This increases each
 *   splits frustum by multiplying it by (1 + bias), and helps reducing artifacts
 *   at the borders of the splits. Artifacts can occur when the bias is too low,
 *   because then the filtering will go over the bounds of the split, producing
 *   invalid results.
 *
 *   If the bias is below zero, an assertion is thrown.
 *
 * @param bias Border bias
 */
inline void PSSMCameraRig::set_border_bias(float bias) {
  nassertv(bias >= 0.0);
  _border_bias = bias;
}

/**
 * @brief Resets the film size cache
 * @details In case PSSMCameraRig::set_use_fixed_film_size is used, this resets
 *   the film size cache. This might lead to a small "jump" in the shadows,
 *   because the film size changes, however it leads to a better shadow distribution.
 *
 *   This is the case because when using a fixed film size, the cache will get
 *   bigger and bigger, whenever the camera moves to a grazing angle. However,
 *   when moving back to a normal angle, the film size cache still stores this
 *   big angle, and thus the splits will have a much bigger film size than actualy
 *   required. To prevent this, call this method once in a while, so an optimal
 *   distribution is ensured.
 */
inline void PSSMCameraRig::reset_film_size_cache() {
  for (size_t i = 0; i < _max_film_sizes.size(); ++i) {
    _max_film_sizes[i].fill(0);
  }
}

/**
 * @brief Returns the n-th camera
 * @details This returns the n-th camera of the camera rig, which can be used
 *   for various stuff like showing its frustum, passing it as a shader input,
 *   and so on.
 *
 *   The first camera is the camera which is the camera of the first split,
 *   which is the split closest to the camera. All cameras follow in descending
 *   order until to the last camera, which is the split furthest away from the
 *   camera.
 *
 *   If an invalid index is passed, an assertion is thrown.
 *
 * @param index Index of the camera.
 * @return [description]
 */
inline NodePath PSSMCameraRig::get_camera(size_t index) const {
  nassertr(index >= 0 && index < _cam_nodes.size(), NodePath());
  return _cam_nodes[index];
}

/**
 * @brief Internal method to compute the distance of a split
 * @details This is the internal method to perform the weighting of the
 *   logarithmic and linear distribution. It computes the distance to the
 *   camera from which a given split starts, by weighting the logarithmic and
 *   linear factor.
 *
 *   The return value is a value ranging from 0 .. 1. To get the distance in
 *   world space, the value has to get multiplied with the maximum shadow distance.
 *
 * @param split_index The index of the split
 * @return Distance of the split, ranging from 0 .. 1
 */
inline float PSSMCameraRig::get_split_start(size_t split_index) const {
  float x = (float)split_index / (float)_cam_nodes.size();
  return (exp(_logarithmic_factor*x)-1) / (exp(_logarithmic_factor)-1);
}

/**
 * @brief Internal method for interpolating a point along the camera frustum
 * @details This method takes a given distance in the 0 .. 1 range, whereas
 *   0 denotes the camera near plane, and 1 denotes the camera far plane,
 *   and lineary interpolates between them.
 *
 * @param origin Edge of the frustum
 * @param depth Distance in the 0 .. 1 range
 *
 * @return interpolated point in world space
 */
inline LPoint3 PSSMCameraRig::get_interpolated_point(CoordinateOrigin origin, float depth) const {
  nassertr(depth >= 0.0 && depth <= 1.0, LPoint3());
  return _curr_near_points[origin] * (1.0 - depth) + _curr_far_points[origin] * depth;
}

/**
 * @brief Returns a handle to the MVP array
 * @details This returns a handle to the array of view-projection matrices
 *   of the different splits. This can be used for computing shadows. The array
 *   is a PTALMatrix4 and thus can be directly bound to a shader.
 *
 * @return view-projection matrix array
 */
inline const PTA_LMatrix4 &PSSMCameraRig::get_mvp_array() const {
  return _camera_mvps;
}

/**
 * @brief Returns a handle to the near and far planes array
 * @details This returns a handle to the near and far plane array. Each split
 *   has an entry in the array, whereas the x component of the vecto denotes the
 *   near plane, and the y component denotes the far plane of the split.
 *
 *   This is required because the near and far planes of the splits change
 *   constantly. To access them in a shader, the shader needs access to the
 *   array.
 *
 * @return Array of near and far planes
 */
inline const PTA_LVecBase2 &PSSMCameraRig::get_nearfar_array() const {
  return _camera_nearfar;
}
