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


#include "internalLightManager.h"

#include <algorithm>

using std::endl;

NotifyCategoryDef(lightmgr, "");


/**
 * @brief Constructs the light manager
 * @details This constructs the light manager, initializing the light and shadow
 *   storage. You should set a command list and shadow manager before calling
 *   InternalLightManager::update. s
 */
InternalLightManager::InternalLightManager() {
  _shadow_update_distance = 100.0;
  _cmd_list = nullptr;
  _shadow_manager = nullptr;
}

/**
 * @brief Adds a new light.
 * @details This adds a new light to the list of lights. This will throw an
 *   error and return if the light is already attached. You may only call
 *   this after the ShadowManager was already set.
 *
 *   While the light is attached, the light manager keeps a reference to it, so
 *   the light does not get destructed.
 *
 *   This also setups the shadows on the light, in case shadows are enabled.
 *   While a light is attached, you can not change whether it casts shadows or not.
 *   To do so, detach the light, change the setting, and re-add the light.
 *
 *   In case no free light slot is available, an error will be printed and no
 *   action will be performed.
 *
 *   If no shadow manager was set, an assertion will be triggered.
 *
 * @param light The light to add.
 */
void InternalLightManager::add_light(PT(RPLight) light) {
  nassertv(_shadow_manager != nullptr); // Shadow manager not set yet!

  // Don't attach the light in case its already attached
  if (light->has_slot()) {
    lightmgr_cat.error() << "could not add light because it already is attached! "
               << "Detach the light first, then try it again." << endl;
    return;
  }

  // Find a free slot
  size_t slot;
  if (!_lights.find_slot(slot)) {
    lightmgr_cat.error() << "Light limit of " << MAX_LIGHT_COUNT << " reached, "
               << "all light slots used!" << endl;
    return;
  }

  // Reference the light because we store it, to avoid it getting destructed
  // on the python side while we still work with it. The reference will be
  // removed when the light gets detached.
  light->ref();

  // Reserve the slot
  light->assign_slot(slot);
  _lights.reserve_slot(slot, light);

  // Setup the shadows in case the light uses them
  if (light->get_casts_shadows()) {
    setup_shadows(light);
  }

  // Store the light on the gpu, to make sure the GPU directly knows about it.
  // We could wait until the next update cycle, but then we might be one frame
  // too late already.
  gpu_update_light(light);
}

/**
 * @brief Internal method to setup shadows for a light
 * @details This method gets called by the InternalLightManager::add_light method
 *   to setup a lights shadow sources, in case shadows are enabled on that light.
 *
 *   It finds a slot for all shadow sources of the ilhgt, and inits the shadow
 *   sources as well. If no slot could be found, an error is printed an nothing
 *   happens.
 *
 * @param light The light to init the shadow sources for
 */
void InternalLightManager::setup_shadows(RPLight* light) {

  // Init the lights shadow sources, and also call update once to make sure
  // the sources are properly initialized
  light->init_shadow_sources();
  light->update_shadow_sources();

  // Find consecutive slots, this is important for PointLights so we can just
  // store the first index of the source, and get the other slots by doing
  // first_index + 1, +2 and so on.
  size_t base_slot;
  size_t num_sources = light->get_num_shadow_sources();
  if (!_shadow_sources.find_consecutive_slots(base_slot, num_sources)) {
    lightmgr_cat.error() << "Failed to find slot for shadow sources! "
               << "Shadow-Source limit of " << MAX_SHADOW_SOURCES
               << " reached!" << endl;
    return;
  }

  // Init all sources
  for (size_t i = 0; i < num_sources; ++i) {
    ShadowSource* source = light->get_shadow_source(i);

    // Set the source as dirty, so it gets updated in the beginning
    source->set_needs_update(true);

    // Assign the slot to the source. Since we got consecutive slots, we can
    // just do base_slot + N.
    size_t slot = base_slot + i;
    _shadow_sources.reserve_slot(slot, source);
    source->set_slot(slot);
  }
}

/**
 * @brief Removes a light
 * @details This detaches a light. This prevents it from being rendered, and also
 *   cleans up all resources used by that light. If no reference is kept on the
 *   python side, the light will also get destructed.
 *
 *   If the light was not previously attached with InternalLightManager::add_light,
 *   an error will be triggered and nothing happens.
 *
 *   In case the light was set to cast shadows, all shadow sources are cleaned
 *   up, and their regions in the shadow atlas are freed.
 *
 *   All resources used by the light in the light and shadow storage are also
 *   cleaned up, by emitting cleanup GPUCommands.
 *
 *   If no shadow manager was set, an assertion will be triggered.
 *
 * @param light [description]
 */
void InternalLightManager::remove_light(PT(RPLight) light) {
  nassertv(_shadow_manager != nullptr);

  if (!light->has_slot()) {
    lightmgr_cat.error() << "Could not detach light, light was not attached!" << endl;
    return;
  }

  // Free the lights slot in the light storage
  _lights.free_slot(light->get_slot());

  // Tell the GPU we no longer need the lights data
  gpu_remove_light(light);

  // Mark the light as detached. After this call, we can not call get_slot
  // anymore, so its important we do this after we unregistered the light
  // from everywhere.
  light->remove_slot();

  // Clear shadow related stuff, in case the light casts shadows
  if (light->get_casts_shadows()) {

    // Free the slots of all sources, and also unregister their regions from
    // the shadow atlas.
    for (size_t i = 0; i < light->get_num_shadow_sources(); ++i) {
      ShadowSource* source = light->get_shadow_source(i);
      if (source->has_slot()) {
        _shadow_sources.free_slot(source->get_slot());
      }
      if (source->has_region()) {
        _shadow_manager->get_atlas()->free_region(source->get_region());
        source->clear_region();
      }
    }

    // Remove all sources of the light by emitting a consecutive remove command
    gpu_remove_consecutive_sources(light->get_shadow_source(0),
                     light->get_num_shadow_sources());

    // Finally remove all shadow sources. This is important in case the light
    // will be re-attached. Otherwise an assertion will get triggered.
    light->clear_shadow_sources();
  }

  // Since we referenced the light when we stored it, we have to decrease
  // the reference now. In case no reference was kept on the python side,
  // the light will get destructed soon.
  light->unref();
}

/**
 * @brief Internal method to remove consecutive sources from the GPU.
 * @details This emits a GPUCommand to consecutively remove shadow sources from
 *   the GPU. This is called when a light gets removed, to free the space its
 *   shadow sources took. Its not really required, because as long as the light
 *   is not used, there is no reference to the sources. However, it can't hurt to
 *   cleanup the memory.
 *
 *   All sources starting at first_source->get_slot() until
 *   first_source->get_slot() + num_sources will get cleaned up.
 *
 * @param first_source First source of the light
 * @param num_sources Amount of consecutive sources to clear
 */
void InternalLightManager::gpu_remove_consecutive_sources(ShadowSource *first_source,
                              size_t num_sources) {
  nassertv(_cmd_list != nullptr);    // No command list set yet
  nassertv(first_source->has_slot()); // Source has no slot!
  GPUCommand cmd_remove(GPUCommand::CMD_remove_sources);
  cmd_remove.push_int(first_source->get_slot());
  cmd_remove.push_int(num_sources);
  _cmd_list->add_command(cmd_remove);
}

/**
 * @brief Internal method to remove a light from the GPU.
 * @details This emits a GPUCommand to clear a lights data. This sets the data
 *   to all zeros, marking that no light is stored anymore.
 *
 *   This throws an assertion in case the light is not currently attached. Be
 *   sure to call this before detaching the light.
 *
 * @param light The light to remove, must be attached.
 */
void InternalLightManager::gpu_remove_light(RPLight* light) {
  nassertv(_cmd_list != nullptr);  // No command list set yet
  nassertv(light->has_slot());  // Light has no slot!
  GPUCommand cmd_remove(GPUCommand::CMD_remove_light);
  cmd_remove.push_int(light->get_slot());
  _cmd_list->add_command(cmd_remove);
}

/**
 * @brief Updates a lights data on the GPU
 * @details This method emits a GPUCommand to update a lights data. This can
 *   be used to initially store the lights data, or to update the data whenever
 *   the light changed.
 *
 *   This throws an assertion in case the light is not currently attached. Be
 *   sure to call this after attaching the light.
 *
 * @param light The light to update
 */
void InternalLightManager::gpu_update_light(RPLight* light) {
  nassertv(_cmd_list != nullptr);  // No command list set yet
  nassertv(light->has_slot());  // Light has no slot!
  GPUCommand cmd_update(GPUCommand::CMD_store_light);
  cmd_update.push_int(light->get_slot());
  light->write_to_command(cmd_update);
  light->set_needs_update(false);
  _cmd_list->add_command(cmd_update);
}

/**
 * @brief Updates a shadow source data on the GPU
 * @details This emits a GPUCommand to update a given shadow source, storing all
 *   data of the source on the GPU. This can also be used to initially store a
 *   ShadowSource, since all data will be overridden.
 *
 *   This throws an assertion if the source has no slot yet.
 *
 * @param source The source to update
 */
void InternalLightManager::gpu_update_source(ShadowSource* source) {
  nassertv(_cmd_list != nullptr);  // No command list set yet
  nassertv(source->has_slot()); // Source has no slot!
  GPUCommand cmd_update(GPUCommand::CMD_store_source);
  cmd_update.push_int(source->get_slot());
  source->write_to_command(cmd_update);
  _cmd_list->add_command(cmd_update);
}

/**
 * @brief Internal method to update all lights
 * @details This is called by the main update method, and iterates over the list
 *   of lights. If a light is marked as dirty, it will recieve an update of its
 *   data and its shadow sources.
 */
void InternalLightManager::update_lights() {
  for (auto iter = _lights.begin(); iter != _lights.end(); ++iter) {
    RPLight* light = *iter;
    if (light && light->get_needs_update()) {
      if (light->get_casts_shadows()) {
        light->update_shadow_sources();
      }
      gpu_update_light(light);
    }
  }
}

/**
 * @brief Compares shadow sources by their priority
 * @details Returns if a has a greater priority than b. This depends on the
 *   resolution of the source, and also if the source has a region or not.
 *   This method can be passed to std::sort.
 *
 * @param a First source
 * @param b Second source
 *
 * @return true if a is more important than b, else false
 */
bool InternalLightManager::compare_shadow_sources(const ShadowSource* a, const ShadowSource* b) const {

  // Make sure that sources which already have a region (but maybe outdated)
  // come after sources which have no region at all.
  if (a->has_region() != b->has_region()) {
    return b->has_region();
  }

  // Compare sources based on their distance to the camera
  PN_stdfloat dist_a = (_camera_pos - a->get_bounds().get_center()).length_squared();
  PN_stdfloat dist_b = (_camera_pos - a->get_bounds().get_center()).length_squared();

  // XXX: Should also compare based on source size, so that huge sources recieve
  // more updates

  return dist_b > dist_a;
}

/**
 * @brief Internal method to update all shadow sources
 * @details This updates all shadow sources which are marked dirty. It will sort
 *   the list of all dirty shadow sources by their resolution, take the first
 *   n entries, and update them. The amount of sources processed depends on the
 *   max_updates of the ShadowManager.
 */
void InternalLightManager::update_shadow_sources() {

  // Find all dirty shadow sources and make a list of them
  std::vector<ShadowSource*> sources_to_update;
   for (auto iter = _shadow_sources.begin(); iter != _shadow_sources.end(); ++iter) {
    ShadowSource* source = *iter;
    if (source) {
      const BoundingSphere& bounds = source->get_bounds();

      // Check if source is in range
      PN_stdfloat distance_to_camera = (_camera_pos - bounds.get_center()).length() - bounds.get_radius();
      if (distance_to_camera < _shadow_update_distance) {
        if (source->get_needs_update()) {
          sources_to_update.push_back(source);
        }
      } else {

        // Free regions of sources which are out of the update radius,
        // to make space for other regions
        if (source->has_region()) {
          _shadow_manager->get_atlas()->free_region(source->get_region());
          source->clear_region();
        }
      }
    }

  }

  // Sort the sources based on their importance, so that sources with a bigger
  // priority come first. This helps to get a better packing on the shadow atlas.
  // However, we also need to prioritize sources which have no current region,
  // because no shadows are worse than outdated-shadows.
  std::sort(sources_to_update.begin(), sources_to_update.end(), [this](const ShadowSource* a, const ShadowSource* b) {
    return this->compare_shadow_sources(a, b);
  });

  // Get a handle to the atlas, will be frequently used
  ShadowAtlas *atlas = _shadow_manager->get_atlas();

  // Free the regions of all sources which will get updated. We have to take into
  // account that only a limited amount of sources can get updated per frame.
  size_t update_slots = std::min(sources_to_update.size(),
                _shadow_manager->get_num_update_slots_left());
  for(size_t i = 0; i < update_slots; ++i) {
    if (sources_to_update[i]->has_region()) {
       atlas->free_region(sources_to_update[i]->get_region());
    }
  }

  // Find an atlas spot for all regions which are supposed to get an update
  for (size_t i = 0; i < update_slots; ++i) {
    ShadowSource *source = sources_to_update[i];

    if(!_shadow_manager->add_update(source)) {
      // In case the ShadowManager lied about the number of updates left
      lightmgr_cat.error() << "ShadowManager ensured update slot, but slot is taken!" << endl;
      break;
    }

    // We have an update slot, and are guaranteed to get updated as soon
    // as possible, so we can start getting a new atlas position.
    size_t region_size = atlas->get_required_tiles(source->get_resolution());
    LVecBase4i new_region = atlas->find_and_reserve_region(region_size, region_size);
    LVecBase4 new_uv_region = atlas->region_to_uv(new_region);
    source->set_region(new_region, new_uv_region);

    // Mark the source as updated
    source->set_needs_update(false);
    gpu_update_source(source);
  }
}

/**
 * @brief Main update method
 * @details This is the main update method of the InternalLightManager. It
 *   processes all lights and shadow sources, updates them, and notifies the
 *   GPU about it. This should be called on a per-frame basis.
 *
 *   If the InternalLightManager was not initialized yet, an assertion is thrown.
 */
void InternalLightManager::update() {
  nassertv(_shadow_manager != nullptr); // Not initialized yet!
  nassertv(_cmd_list != nullptr);     // Not initialized yet!

  update_lights();
  update_shadow_sources();
}
