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


#include "shadowManager.h"

NotifyCategoryDef(shadowmanager, "");

/**
 * @brief Constructs a new shadow atlas
 * @details This constructs a new shadow atlas. There are a set of properties
 *   which should be set before calling ShadowManager::init, see the set-Methods.
 *   After all properties are set, ShadowManager::init should get called.
 *   ShadowManager::update should get called on a per frame basis.
 */
ShadowManager::ShadowManager() {
  _max_updates = 10;
  _atlas = nullptr;
  _atlas_size = 4096;
  _tag_state_mgr = nullptr;
  _atlas_graphics_output = nullptr;
}

/**
 * @brief Destructs the ShadowManager
 * @details This destructs the shadow manager, clearing all resources used
 */
ShadowManager::~ShadowManager() {
  delete _atlas;

  // Todo: Could eventually unregister all shadow cameras. Since the tag state
  // manager does this on cleanup already, and we get destructed at the same
  // time (if at all), this is not really necessary
}


/**
 * @brief Initializes the ShadowManager.
 * @details This initializes the ShadowManager. All properties should have
 *   been set before calling this, otherwise assertions will get triggered.
 *
 *   This setups everything required for rendering shadows, including the
 *   shadow atlas and the various shadow cameras. After calling this method,
 *   no properties can be changed anymore.
 */
void ShadowManager::init() {
  nassertv(!_scene_parent.is_empty());    // Scene parent not set, call set_scene_parent before init!
  nassertv(_tag_state_mgr != nullptr);     // TagStateManager not set, call set_tag_state_mgr before init!
  nassertv(_atlas_graphics_output != nullptr); // AtlasGraphicsOutput not set, call set_atlas_graphics_output before init!

  _cameras.resize(_max_updates);
  _display_regions.resize(_max_updates);
  _camera_nps.reserve(_max_updates);

  // Create the cameras and regions
  for(size_t i = 0; i < _max_updates; ++i) {

    // Create the camera
    PT(Camera) camera = new Camera("ShadowCam-" + format_string(i));
    camera->set_lens(new MatrixLens());
    camera->set_active(false);
    camera->set_scene(_scene_parent);
    _tag_state_mgr->register_camera("shadow", camera);
    _camera_nps.push_back(_scene_parent.attach_new_node(camera));
    _cameras[i] = camera;

    // Create the display region
    PT(DisplayRegion) region = _atlas_graphics_output->make_display_region();
    region->set_sort(1000);
    region->set_clear_depth_active(true);
    region->set_clear_depth(1.0);
    region->set_clear_color_active(false);
    region->set_camera(_camera_nps[i]);
    region->set_active(false);
    _display_regions[i] = region;
  }

  // Create the atlas
  _atlas = new ShadowAtlas(_atlas_size);

  // Reserve enough space for the updates
  _queued_updates.reserve(_max_updates);
}


/**
 * @brief Updates the ShadowManager
 * @details This updates the ShadowManager, processing all shadow sources which
 *   need to get updated.
 *
 *   This first collects all sources which require an update, sorts them by priority,
 *   and then processes the first <max_updates> ShadowSources.
 *
 *   This may not get called before ShadowManager::init, or an assertion will be
 *   thrown.
 */
void ShadowManager::update() {
  nassertv(_atlas != nullptr);             // ShadowManager::init not called yet
  nassertv(_queued_updates.size() <= _max_updates); // Internal error, should not happen

  // Disable all cameras and regions which will not be used
  for (size_t i = _queued_updates.size(); i < _max_updates; ++i) {
    _cameras[i]->set_active(false);
    _display_regions[i]->set_active(false);
  }

  // Iterate over all queued updates
  for (size_t i = 0; i < _queued_updates.size(); ++i) {
    const ShadowSource* source = _queued_updates[i];

    // Enable the camera and display region, so they perform a render
    _cameras[i]->set_active(true);
    _display_regions[i]->set_active(true);

    // Set the view projection matrix
    DCAST(MatrixLens, _cameras[i]->get_lens())->set_user_mat(source->get_mvp());

    // Optional: Show the camera frustum for debugging
    // _cameras[i]->show_frustum();

    // Set the correct dimensions on the display region
    const LVecBase4& uv = source->get_uv_region();
    _display_regions[i]->set_dimensions(
      uv.get_x(),        // left
      uv.get_x() + uv.get_z(), // right
      uv.get_y(),        // bottom
      uv.get_y() + uv.get_w()  // top
    );
  }

  // Clear the update list
  _queued_updates.clear();
  _queued_updates.reserve(_max_updates);
}
