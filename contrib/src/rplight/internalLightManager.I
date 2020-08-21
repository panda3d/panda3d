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
 * @brief Returns the maximum light index
 * @details This returns the maximum light index (also called slot). Any lights
 *   after that slot are guaranteed to be zero-lights. This is useful when
 *   iterating over the list of lights, because iteration can be stopped when
 *   the maximum light index is reached.
 *
 *   The maximum light index points to the last slot which is used. If no lights
 *   are attached, -1 is returned. If one light is attached at slot 0, the index
 *   is 0, if two are attached at the slots 0 and 1, the index is 1, and so on.
 *
 *   If, for example, two lights are attached at the slots 2 and 5, then the
 *   index will be 5. Keep in mind that the max-index is not an indicator for
 *   how many lights are attached. Also, zero lights still may occur when iterating
 *   over the light lists
 *
 * @return Maximum light index
 */
inline int InternalLightManager::get_max_light_index() const {
  return _lights.get_max_index();
}

/**
 * @brief Returns the amount of stored lights.
 * @details This returns the amount of stored lights. This behaves unlike
 *   InternalLightManager::get_max_light_index, and instead returns the true
 *   amount of lights, which is completely unrelated to the amount of used slots.
 *
 * @return Amount of stored lights
 */
inline size_t InternalLightManager::get_num_lights() const {
  return _lights.get_num_entries();
}

/**
 * @brief Returns the amount of shadow sources.
 * @details This returns the total amount of stored shadow sources. This does
 *   not denote the amount of updated sources, but instead takes into account
 *   all sources, even those out of frustum.
 * @return Amount of shadow sources.
 */
inline size_t InternalLightManager::get_num_shadow_sources() const {
  return _shadow_sources.get_num_entries();
}

/**
 * @brief Sets the handle to the shadow manager
 * @details This sets the handle to the global shadow manager. It is usually
 *   constructed on the python side, so we need to get a handle to it.
 *
 *   The manager should be a handle to a ShadowManager instance, and will be
 *   stored somewhere on the python side most likely. The light manager does not
 *   keep a reference to it, so the python side should make sure to keep one.
 *
 *   Be sure to call this before the InternalLightManager::update() method is
 *   called, otherwise an assertion will get triggered.
 *
 * @param mgr The ShadowManager instance
 */
inline void InternalLightManager::set_shadow_manager(ShadowManager* mgr) {
  _shadow_manager = mgr;
}

/**
 * @brief Sets a handle to the command list
 * @details This sets a handle to the global GPUCommandList. This is required to
 *   emit GPUCommands, which are used for attaching and detaching lights, as well
 *   as shadow source updates.
 *
 *   The cmd_list should be a handle to a GPUCommandList handle, and will be
 *   stored somewhere on the python side most likely. The light manager does not
 *   keep a reference to it, so the python side should make sure to keep one.
 *
 *   Be sure to call this before the InternalLightManager::update() method is
 *   called, otherwise an assertion will get triggered.
 *
 * @param cmd_list The GPUCommandList instance
 */
inline void InternalLightManager::set_command_list(GPUCommandList *cmd_list) {
  _cmd_list = cmd_list;
}

/**
 * @brief Sets the camera position
 * @details This sets the camera position, which will be used to determine which
 *   shadow sources have to get updated
 *
 * @param mat View projection mat
 */
inline void InternalLightManager::set_camera_pos(const LPoint3 &pos) {
  _camera_pos = pos;
}

/**
 * @brief Sets the maximum shadow update distance
 * @details This controls the maximum distance until which shadows are updated.
 *   If a shadow source is past that distance, it is ignored and no longer recieves
 *   updates until it is in range again
 *
 * @param dist Distance in world space units
 */
inline void InternalLightManager::set_shadow_update_distance(PN_stdfloat dist) {
  _shadow_update_distance = dist;
}

/**
 * @brief Returns the internal used ShadowManager
 * @details This returns a handle to the internally used shadow manager
 * @return Shadow manager
 */
inline ShadowManager* InternalLightManager::get_shadow_manager() const {
  return _shadow_manager;
}
