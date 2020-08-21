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
 * @brief Returns the amount of shadow sources
 * @details This returns the amount of shadow sources attached to this light.
 *   In case the light has no shadows enabled, or the light was not attached
 *   yet, this returns 0.
 *
 * @return Amount of shadow sources
 */
inline size_t RPLight::get_num_shadow_sources() const {
  return _shadow_sources.size();
}

/**
 * @brief Returns the n-th shadow source
 * @details This returns the n-th attached shadow source. This ranges from
 *   0 .. RPLight::get_num_shadow_sources(). If an invalid index is passed,
 *   an assertion is thrown.
 *
 * @param index Index of the source
 * @return Handle to the shadow source
 */
inline ShadowSource* RPLight::get_shadow_source(size_t index) const {
  nassertr(index < _shadow_sources.size(), nullptr); // Invalid shadow source index
  return _shadow_sources[index];
}

/**
 * @brief Clears all shadow source
 * @details This removes and destructs all shadow sources attached to this light.
 *   This usually gets called when the light gets detached or destructed.
 *   All shadows sources are freed, and then removed from the shadow source list.
 */
inline void RPLight::clear_shadow_sources() {
  for (size_t i = 0; i < _shadow_sources.size(); ++i) {
    delete _shadow_sources[i];
  }
  _shadow_sources.clear();
}

/**
 * @brief Sets whether the light needs an update
 * @details This controls whether the light needs to get an update. This is the
 *   case when a property of the light changed, e.g. position or color. It does
 *   not affect the shadows (For that use RPLight::invalidate_shadows()).
 *   When this flag is set to true, the light will get resubmitted to the GPU
 *   in the next update cycle.
 *
 *   You should usually never set the flag to false manually. The
 *   InternalLightManager will do this when the data got sucessfully updated.
 *
 * @param flag Update-Flag
 */
inline void RPLight::set_needs_update(bool flag) {
  _needs_update = flag;
}

/**
 * @brief Returns whether the light needs an update
 * @details This returns whether the light needs an update. This might be the
 *   case when a property of the light was changed, e.g. position or color.
 *   It does not affect the shadows, you have to query the update flag of each
 *   individual source for that.
 *   The return value is the value previously set with RPLight::set_needs_update.
 *
 * @return Update-flag
 */
inline bool RPLight::get_needs_update() const {
  return _needs_update;
}

/**
 * @brief Returns whether the light has a slot
 * @details This returns wheter the light currently is attached, and thus has
 *   a slot in the InternalLightManagers light list. When the light is attached,
 *   this returns true, otherwise it will return false.
 *
 * @return true if the light has a slot, false otherwise
 */
inline bool RPLight::has_slot() const {
  return _slot >= 0;
}

/**
 * @brief Returns the slot of the light
 * @details This returns the slot of the light. This is the space on the GPU
 *   where the light is stored. If the light is not attached yet, this will
 *   return -1, otherwise the index of the light.
 *
 * @return Light-Slot
 */
inline int RPLight::get_slot() const {
  return _slot;
}

/**
 * @brief Removes the light slot
 * @details This is an internal method to remove the slot of the light. It gets
 *   called by the InternalLightManager when a light gets detached. It internally
 *   sets the slot to -1 to indicate the light is no longer attached.
 */
inline void RPLight::remove_slot() {
  _slot = -1;
}

/**
 * @brief Assigns a slot to the light
 * @details This assigns a slot to the light, marking it as attached. The slot
 *   relates to the index in the GPU's storage of lights. This is an internal
 *   method called by the InternalLightManager when the light got attached.
 *
 * @param slot Slot of the light
 */
inline void RPLight::assign_slot(int slot) {
  _slot = slot;
}

/**
 * @brief Invalidates the shadows
 * @details This invalidates all shadows of the light, causing them to get
 *   regenerated. This might be the case  when the lights position or similar
 *   changed. This will cause all shadow sources to be updated, emitting a
 *   shadow update. Be careful when calling this method if you don't want all
 *   sources to get updated. If you only have to invalidate a single shadow source,
 *   use get_shadow_source(n)->set_needs_update(true).
 */
inline void RPLight::invalidate_shadows() {
  for (size_t i = 0; i < _shadow_sources.size(); ++i) {
    _shadow_sources[i]->set_needs_update(true);
  }
}

/**
 * @brief Sets the position of the light
 * @details This sets the position of the light in world space. It will cause
 *   the light to get invalidated, and resubmitted to the GPU.
 *
 * @param pos Position in world space
 */
inline void RPLight::set_pos(const LVecBase3 &pos) {
  set_pos(pos.get_x(), pos.get_y(), pos.get_z());
}

/**
 * @brief Sets the position of the light
 * @details @copydetails RPLight::set_pos(const LVecBase3 &pos)
 *
 * @param x X-component of the position
 * @param y Y-component of the position
 * @param z Z-component of the position
 */
inline void RPLight::set_pos(float x, float y, float z) {
  _position.set(x, y, z);
  set_needs_update(true);
  invalidate_shadows();
}

/**
 * @brief Returns the position of the light
 * @details This returns the position of the light previously set with
 *   RPLight::set_pos(). The returned position is in world space.
 * @return Light-position
 */
inline const LVecBase3& RPLight::get_pos() const {
  return _position;
}

/**
 * @brief Sets the lights color
 * @details This sets the lights color. The color should not include the brightness
 *   of the light, you should control that with the energy. The color specifies
 *   the lights "tint" and will get multiplied with its specular and diffuse
 *   contribution.
 *
 *   The color will be normalized by dividing by the colors luminance. Setting
 *   higher values than 1.0 will have no effect.
 *
 * @param color Light color
 */
inline void RPLight::set_color(const LVecBase3 &color) {
  _color = color;
  _color /= 0.2126 * color.get_x() + 0.7152 * color.get_y() + 0.0722 * color.get_z();
  set_needs_update(true);
}

/**
 * @brief Sets the lights color
 * @details @copydetails RPLight::set_color(const LVecBase3 &color)
 *
 * @param r Red-component of the color
 * @param g Green-component of the color
 * @param b Blue-component of the color
 */
inline void RPLight::set_color(float r, float g, float b) {
  set_color(LVecBase3(r, g, b));
}

/**
 * @brief Returns the lights color
 * @details This returns the light color, previously set with RPLight::set_color.
 *   This does not include the energy of the light. It might differ from what
 *   was set with set_color, because the color is normalized by dividing it
 *   by its luminance.
 * @return Light-color
 */
inline const LVecBase3& RPLight::get_color() const {
  return _color;
}

/**
 * @brief Sets the energy of the light
 * @details This sets the energy of the light, which can be seen as the brightness
 *   of the light. It will get multiplied with the normalized color.
 *
 * @param energy energy of the light
 */
inline void RPLight::set_energy(float energy) {
  _energy = energy;
  set_needs_update(true);
}

/**
 * @brief Returns the energy of the light
 * @details This returns the energy of the light, previously set with
 *   RPLight::set_energy.
 *
 * @return energy of the light
 */
inline float RPLight::get_energy() const {
  return _energy;
}

/**
 * @brief Returns the type of the light
 * @details This returns the internal type of the light, which was specified
 *   in the lights constructor. This can be used to distinguish between light
 *   types.
 * @return Type of the light
 */
inline RPLight::LightType RPLight::get_light_type() const {
  return _light_type;
}

/**
 * @brief Controls whether the light casts shadows
 * @details This sets whether the light casts shadows. You can not change this
 *   while the light is attached. When flag is set to true, the light will be
 *   setup to cast shadows, spawning shadow sources based on the lights type.
 *   If the flag is set to false, the light will be inddicated to cast no shadows.
 *
 * @param flag Whether the light casts shadows
 */
inline void RPLight::set_casts_shadows(bool flag) {
  if (has_slot()) {
    std::cerr << "Light is already attached, can not call set_casts_shadows!" << std::endl;
    return;
  }
  _casts_shadows = flag;
}

/**
 * @brief Returns whether the light casts shadows
 * @details This returns whether the light casts shadows, the returned value
 *   is the one previously set with RPLight::set_casts_shadows.
 *
 * @return true if the light casts shadows, false otherwise
 */
inline bool RPLight::get_casts_shadows() const {
  return _casts_shadows;
}

/**
 * @brief Sets the lights shadow map resolution
 * @details This sets the lights shadow map resolution. This has no effect
 *   when the light is not told to cast shadows (Use RPLight::set_casts_shadows).
 *
 *   When calling this on a light with multiple shadow sources (e.g. PointLight),
 *   this controls the resolution of each source. If the light has 6 shadow sources,
 *   and you use a resolution of 512x512, the lights shadow map will occur a
 *   space of 6 * 512x512 maps in the shadow atlas.
 *
 * @param resolution Resolution of the shadow map in pixels
 */
inline void RPLight::set_shadow_map_resolution(size_t resolution) {
  nassertv(resolution >= 32 && resolution <= 16384);
  _source_resolution = resolution;
  invalidate_shadows();
}

/**
 * @brief Returns the shadow map resolution
 * @details This returns the shadow map resolution of each source of the light.
 *   If the light is not setup to cast shadows, this value is meaningless.
 *   The returned value is the one previously set with RPLight::set_shadow_map_resolution.
 *
 * @return Shadow map resolution in pixels
 */
inline size_t RPLight::get_shadow_map_resolution() const {
  return _source_resolution;
}

/**
 * @brief Sets the ies profile
 * @details This sets the ies profile of the light. The parameter should be a
 *   handle previously returned by RenderPipeline.load_ies_profile. Using a
 *   value of -1 indicates no ies profile.
 *
 *   Notice that for ies profiles which cover a whole range, you should use
 *   PointLights, whereas for ies profiles which only cover the lower hemisphere
 *   you should use SpotLights for the best performance.
 *
 * @param profile IES Profile handle
 */
inline void RPLight::set_ies_profile(int profile) {
  _ies_profile = profile;
  set_needs_update(true);
}

/**
 * @brief Returns the lights ies profile
 * @details This returns the ies profile of a light, previously set with
 *   RPLight::set_ies_profile. In case no ies profile was set, returns -1.
 *
 * @return IES Profile handle
 */
inline int RPLight::get_ies_profile() const {
  return _ies_profile;
}

/**
 * @brief Returns whether the light has an ies profile assigned
 * @details This returns whether the light has an ies profile assigned,
 *   previously done with RPLight::set_ies_profile.
 *
 * @return true if the light has an ies profile assigned, false otherwise
 */
inline bool RPLight::has_ies_profile() const {
  return _ies_profile >= 0;
}

/**
 * @brief Clears the ies profile
 * @details This clears the ies profile of the light, telling it to no longer
 *   use an ies profile, and instead use the default attenuation.
 */
inline void RPLight::clear_ies_profile() {
  set_ies_profile(-1);
}

/**
 * @brief Sets the near plane of the light
 * @details This sets the near plane of all shadow sources of the light. It has
 *   no effects if the light does not cast shadows. This prevents artifacts from
 *   objects near to the light. It behaves like Lens::set_near_plane.
 *
 *   It can also help increasing shadow map precision, low near planes will
 *   cause the precision to suffer. Try setting the near plane as big as possible.
 *
 *   If a negative or zero near plane is passed, an assertion is thrown.
 *
 * @param near_plane Near-plane
 */
inline void RPLight::set_near_plane(float near_plane) {
  nassertv(near_plane > 0.00001);
  _near_plane = near_plane;
  invalidate_shadows();
}

/**
 * @brief Returns the near plane of the light
 * @details This returns the lights near plane, previously set with
 *   RPLight::set_near_plane. If the light does not cast shadows, this value
 *   is meaningless.
 *
 * @return Near-plane
 */
inline float RPLight::get_near_plane() const {
  return _near_plane;
}

