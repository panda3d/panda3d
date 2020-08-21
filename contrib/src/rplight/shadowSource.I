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
 * @brief Returns whether the shadow source needs an update.
 * @details This returns the update flag, which was previously set with
 *   ShadowSource::set_needs_update. If the value is true, it means that the
 *   ShadowSource is invalid and should be regenerated. This can either be the
 *   case because the scene changed and affected the shadow source, or the light
 *   moved.
 * @return Update-Flag
 */
inline bool ShadowSource::get_needs_update() const {
  return !has_region() || _needs_update;
}

/**
 * @brief Returns the slot of the shadow source.
 * @details This returns the assigned slot of the ShadowSource, or -1 if no slot
 *   was assigned yet. You can check if a slot exists with ShadowSource::has_slot.
 *   The slot is the index of the ShadowSource in the global source buffer.
 * @return Slot, or -1 to indicate no slot.
 */
inline int ShadowSource::get_slot() const {
  return _slot;
}

/**
 * @brief Returns whether the source has a slot.
 * @details This returns whether the ShadowSource currently has an assigned slot.
 *   If the source has a slot assigned, this returns true, otherwise false. Cases
 *   where the source has no slot might be when the source just got attached, but
 *   never got rendered yet.
 * @return [description]
 */
inline bool ShadowSource::has_slot() const {
  return _slot >= 0;
}

/**
 * @brief Assigns the source a slot
 * @details This assigns a slot to the ShadowSource. This is called from the
 *   ShadowManager, when the source gets attached first time. This should not
 *   get called by the user.
 *
 * @param slot Slot of the source, or -1 to indicate no slot.
 */
inline void ShadowSource::set_slot(int slot) {
  _slot = slot;
}

/**
 * @brief Setups a perspective lens for the source.
 * @details This makes the shadow source behave like a perspective lens. The
 *   parameters are similar to the ones of a PerspectiveLens.
 *
 * @param fov FoV of the lens
 * @param near_plane The near plane of the lens, to avoid artifacts at low distance
 * @param far_plane The far plane of the lens
 * @param pos Position of the lens, in world space
 * @param direction Direction (Orientation) of the lens
 */
inline void ShadowSource::
set_perspective_lens(PN_stdfloat fov, PN_stdfloat near_plane,
                     PN_stdfloat far_plane, LVecBase3 pos,
                     LVecBase3 direction) {
  // Construct the transfo*rmation matrix
  LMatrix4 transform_mat = LMatrix4::translate_mat(-pos);

  // Construct a temporary lens to generate the lens matrix
  PerspectiveLens temp_lens = PerspectiveLens(fov, fov);
  temp_lens.set_film_offset(0, 0);
  temp_lens.set_near_far(near_plane, far_plane);
  temp_lens.set_view_vector(direction, LVector3::up());
  set_matrix_lens(transform_mat * temp_lens.get_projection_mat());

  // Set new bounds, approximate with sphere
  CPT(BoundingHexahedron) hexahedron = DCAST(BoundingHexahedron, temp_lens.make_bounds());
  LPoint3 center = (hexahedron->get_min() + hexahedron->get_max()) * 0.5f;
  _bounds = BoundingSphere(pos + center, (hexahedron->get_max() - center).length());
}

/**
 * @brief Sets a custom matrix for the source.
 * @details This tells the source to use a custom matrix for rendering, just like
 *   the matrix lens. The matrix should include all transformations, rotations and
 *   scales. No other matrices will be used for rendering this shadow source (not
 *   even a coordinate system conversion).
 *
 * @param mvp Custom View-Projection matrix
 */
inline void ShadowSource::set_matrix_lens(const LMatrix4& mvp) {
  _mvp = mvp;
  set_needs_update(true);
}

/**
 * @brief Sets the update flag of the source.
 * @details Sets whether the source is still valid, or needs to get regenerated.
 *   Usually you only want to flag the shadow source as invalid, by passing
 *   true as the flag. However, the ShadowManager will set the flag to false
 *   after updating the source.
 *
 * @param flag The update flag
 */
inline void ShadowSource::set_needs_update(bool flag) {
  _needs_update = flag;
}

/**
 * @brief Returns whether the source has a valid region.
 * @details This returns whether the ShadowSource has a valid shadow atlas region
 *   assigned. This might be not the case when the source never was rendered yet,
 *   or is about to get updated.
 * @return true if the source has a valid region, else false.
 */
inline bool ShadowSource::has_region() const {
  return _region.get_x() >= 0 && _region.get_y() >= 0 && _region.get_z() >= 0 && _region.get_w() >= 0;
}

/**
 * @brief Returns the resolution of the source.
 * @details Returns the shadow map resolution of source, in pixels. This is the
 *   space the source takes in the shadow atlas, in pixels.
 * @return Resolution in pixels
 */
inline size_t ShadowSource::get_resolution() const {
  return _resolution;
}

/**
 * @brief Returns the assigned region of the source in atlas space.
 * @details This returns the region of the source, in atlas space. This is the
 *  region set by ShadowSource::set_region. If no region was set yet, returns
 *  a 4-component integer vector with all components set to -1. To check this,
 *  you should call ShadowSource::has_region() first.
 *
 * @return [description]
 */
inline const LVecBase4i& ShadowSource::get_region() const {
  return _region;
}

/**
 * @brief Returns the assigned region of the source in UV space.
 * @details This returns the region of the source, in UV space. This is the
 *  region set by ShadowSource::set_region. If no region was set yet, returns
 *  a 4-component integer vector with all components set to -1. To check this,
 *  you should call ShadowSource::has_region() first.
 *
 * @return [description]
 */
inline const LVecBase4& ShadowSource::get_uv_region() const {
  return _region_uv;
}

/**
 * @brief Sets the assigned region of the source in atlas and uv space.
 * @details This sets the assigned region of the ShadowSource. The region in
 *   atlas space should be the region returned from the
 *   ShadowAtlas::find_and_reserve_region. The uv-region should be the same region,
 *   but in the 0 .. 1 range (can be converted with ShadowAtlas::region_to_uv).
 *   This is required for the shaders, because they expect coordinates in the
 *   0 .. 1 range for sampling.
 *
 * @param region Atlas-Space region
 * @param region_uv UV-Space region
 */
inline void ShadowSource::set_region(const LVecBase4i& region, const LVecBase4& region_uv) {
  _region = region;
  _region_uv = region_uv;
}

/**
 * @brief Returns the View-Projection matrix of the source.
 * @details This returns the current view-projection matrix of the ShadowSource.
 *   If no matrix was set yet, returns a matrix with all components zero.
 *   If a matrix was set with ShadowSource::set_matrix_lens, returns the matrix
 *   set by that function call.
 *
 *   If a matrix was set with ShadowSource::set_perspective_lens, returns a
 *   perspective view-projection matrix setup by those parameters.
 *
 *   The matrix returned is the matrix used for rendering the shadow map, and
 *   includes the camera transform as well as the projection matrix.
 *
 * @return View-Projection matrix.
 */
inline const LMatrix4& ShadowSource::get_mvp() const {
  return _mvp;
}

/**
 * @brief Writes the source to a GPUCommand.
 * @details This writes the ShadowSource to a GPUCommand. This stores the
 *   mvp and the uv-region in the command.
 *
 * @param cmd GPUCommand to write to.
 */
inline void ShadowSource::write_to_command(GPUCommand &cmd) const {
  // When storing on the gpu, we should already have a valid slot
  nassertv(_slot >= 0);
  cmd.push_mat4(_mvp);
  cmd.push_vec4(_region_uv);
}

/**
 * @brief Sets the resolution of the source.
 * @details This sets the resolution of the ShadowSource, in pixels. It should be
 *   a multiple of the tile size of the ShadowAtlas, and greater than zero.
 *
 * @param resolution [description]
 */
inline void ShadowSource::set_resolution(size_t resolution) {
  nassertv(resolution > 0);
  _resolution = resolution;
  set_needs_update(true);
}

/**
 * @brief Returns the shadow sources bounds
 * @details This returns the bounds of the shadow source, approximated as a sphere
 * @return Bounds as a BoundingSphere
 */
inline const BoundingSphere& ShadowSource::get_bounds() const {
  return _bounds;
}

/**
 * @brief Clears the assigned region of the source
 * @details This unassigns any shadow atlas region from the source, previously
 *   set with set_region
 */
inline void ShadowSource::clear_region() {
  _region.fill(-1);
  _region_uv.fill(0);
}
