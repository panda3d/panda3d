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
 * @brief Sets the maximum amount of updates per frame.
 * @details This controls the maximum amount of updated ShadowSources per frame.
 *   The ShadowManager will take the first <max_updates> ShadowSources, and
 *   generate shadow maps for them every frame. If there are more ShadowSources
 *   waiting to get updated than available updates, the sources are sorted by
 *   priority, and the update of the less important sources is delayed to the
 *   next frame.
 *
 *   If the update count is set too low, and there are a lot of ShadowSources
 *   waiting to get updated, artifacts will occur, and there might be ShadowSources
 *   which never get updated, due to low priority.
 *
 *   If an update count of 0 is passed, no updates will happen. This also means
 *   that there are no shadows. This is not recommended.
 *
 *   If an update count < 0 is passed, undefined behaviour occurs.
 *
 *   This method has to get called before ShadowManager::init, otherwise an
 *   assertion will get triggered.
 *
 * @param max_updates Maximum amoumt of updates
 */
inline void ShadowManager::set_max_updates(size_t max_updates) {
  nassertv(max_updates >= 0);
  nassertv(_atlas == nullptr);  // ShadowManager was already initialized
  if (max_updates == 0) {
    shadowmanager_cat.warning() << "max_updates set to 0, no shadows will be updated." << std::endl;
  }
  _max_updates = max_updates;
}

/**
 * @brief Sets the shadow atlas size
 * @details This sets the desired shadow atlas size. It should be big enough
 *   to store all important shadow sources, with some buffer, because the shadow
 *   maps usually won't be fitted perfectly, so gaps can occur.
 *
 *   This has to get called before calling ShadowManager::init. When calling this
 *   method after initialization, an assertion will get triggered.
 *
 * @param atlas_size Size of the shadow atlas in pixels
 */
inline void ShadowManager::set_atlas_size(size_t atlas_size) {
  nassertv(atlas_size >= 16 && atlas_size <= 16384);
  nassertv(_atlas == nullptr);  // ShadowManager was already initialized
  _atlas_size = atlas_size;
}

/**
 * @brief Returns the shadow atlas size.
 * @details This returns the shadow atlas size previously set with
 *   ShadowManager::set_atlas_size.
 * @return Shadow atlas size in pixels
 */
inline size_t ShadowManager::get_atlas_size() const {
  return _atlas_size;
}


/**
 * @brief Returns a handle to the shadow atlas.
 * @details This returns a handle to the internal shadow atlas instance. This
 *   is only valid after calling ShadowManager::init. Calling this earlier will
 *   trigger an assertion and undefined behaviour.
 * @return The internal ShadowAtlas instance
 */
inline ShadowAtlas* ShadowManager::get_atlas() const {
  nassertr(_atlas != nullptr, nullptr); // Can't hurt to check
  return _atlas;
}

/**
 * @brief Sets the target scene
 * @details This sets the target scene for rendering shadows. All shadow cameras
 *   will be parented to this scene to render shadows.
 *
 *   Usually the scene will be ShowBase.render. If the scene is an empty or
 *   invalid NodePath, an assertion will be triggered.
 *
 *   This method has to get called before calling ShadowManager::init, or an
 *   assertion will get triggered.
 *
 * @param scene_parent The target scene
 */
inline void ShadowManager::set_scene(NodePath scene_parent) {
  nassertv(!scene_parent.is_empty());
  nassertv(_atlas == nullptr);  // ShadowManager was already initialized
  _scene_parent = scene_parent;
}

/**
 * @brief Sets the handle to the TagStageManager.
 * @details This sets the handle to the TagStateManager used by the pipeline.
 *   Usually this is RenderPipeline.get_tag_mgr().
 *
 *   This has to get called before ShadowManager::init, otherwise an assertion
 *   will get triggered.
 *
 * @param tag_mgr [description]
 */
inline void ShadowManager::set_tag_state_manager(TagStateManager* tag_mgr) {
  nassertv(tag_mgr != nullptr);
  nassertv(_atlas == nullptr);  // ShadowManager was already initialized
  _tag_state_mgr = tag_mgr;
}

/**
 * @brief Sets the handle to the Shadow targets output
 * @details This sets the handle to the GraphicsOutput of the shadow atlas.
 *   Usually this is RenderTarget.get_internal_buffer(), whereas the RenderTarget
 *   is the target of the ShadowStage.
 *
 *   This is used for creating display regions and attaching cameras to them,
 *   for performing shadow updates.
 *
 *   This has to get called before ShadowManager::init, otherwise an assertion
 *   will be triggered.
 *
 * @param graphics_output [description]
 */
inline void ShadowManager::set_atlas_graphics_output(GraphicsOutput* graphics_output) {
  nassertv(graphics_output != nullptr);
  nassertv(_atlas == nullptr);  // ShadowManager was already initialized
  _atlas_graphics_output = graphics_output;
}


/**
 * @brief Adds a new shadow update
 * @details This adds a new update to the update queue. When the queue is already
 *   full, this method returns false, otherwise it returns true. The next time
 *   the manager is updated, the shadow source will recieve an update of its
 *   shadow map.
 *
 * @param source The shadow source to update
 *
 * @return Whether the shadow source udpate was sucessfully queued.
 */
inline bool ShadowManager::add_update(const ShadowSource* source) {
  nassertr(_atlas != nullptr, false); // ShadowManager::init not called yet.
  nassertr(source != nullptr, false); // nullptr-Pointer passed

  if (_queued_updates.size() >= _max_updates) {
    if (shadowmanager_cat.is_debug()) {
      shadowmanager_cat.debug() << "cannot update source, out of update slots" << std::endl;
    }
    return false;
  }

  // Add the update to the queue
  _queued_updates.push_back(source);
  return true;
}

/**
 * @brief Returns how many update slots are left.
 * @details This returns how many update slots are left. You can assume the
 *   next n calls to add_update will succeed, whereas n is the value returned
 *   by this function.
 * @return Number of update slots left.
 */
inline size_t ShadowManager::get_num_update_slots_left() const {
  return _max_updates - _queued_updates.size();
}
