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


#include "tagStateManager.h"

using std::endl;


NotifyCategoryDef(tagstatemgr, "");

/**
 * @brief Constructs a new TagStateManager
 * @details This constructs a new TagStateManager. The #main_cam_node should
 *   refer to the main scene camera, and will most likely be base.cam.
 *   It is necessary to pass the camera because the C++ code does not have
 *   access to the showbase.
 *
 * @param main_cam_node The main scene camera
 */
TagStateManager::
TagStateManager(NodePath main_cam_node) {
  nassertv(!main_cam_node.is_empty());
  nassertv(DCAST(Camera, main_cam_node.node()) != nullptr);
  _main_cam_node = main_cam_node;

  // Set default camera mask
  DCAST(Camera, _main_cam_node.node())->set_camera_mask(BitMask32::bit(1));

  // Init containers
  _containers["shadow"]   = StateContainer("Shadows",  2, false);
  _containers["voxelize"] = StateContainer("Voxelize", 3, false);
  _containers["envmap"]   = StateContainer("Envmap",   4, true);
  _containers["forward"]  = StateContainer("Forward",  5, true);
}

/**
 * @brief Destructs the TagStateManager
 * @details This destructs the TagStateManager, and cleans up all resources used.
 */
TagStateManager::
~TagStateManager() {
  cleanup_states();
}

/**
 * @brief Applies a given state to a NodePath
 * @details This applies a shader to the given NodePath which is used when the
 *   NodePath is rendered by any registered camera of the container.
 *
 * @param container The container which is used to store the state
 * @param np The nodepath to apply the shader to
 * @param shader A handle to the shader to apply
 * @param name Name of the state, should be a unique identifier
 * @param sort Changes the sort with which the shader will be applied.
 */
void TagStateManager::
apply_state(StateContainer& container, NodePath np, Shader* shader,
            const std::string &name, int sort) {
  if (tagstatemgr_cat.is_spam()) {
    tagstatemgr_cat.spam() << "Constructing new state " << name
                 << " with shader " << shader << endl;
  }

  // Construct the render state
  CPT(RenderState) state = RenderState::make_empty();

  // Disable color write for all stages except the environment container
  if (!container.write_color) {
    state = state->set_attrib(ColorWriteAttrib::make(ColorWriteAttrib::C_off), 10000);
  }
  state = state->set_attrib(ShaderAttrib::make(shader, sort), sort);

  // Emit a warning if we override an existing state
  if (container.tag_states.count(name) != 0) {
    tagstatemgr_cat.warning() << "Overriding existing state " << name << endl;
  }

  // Store the state, this is required whenever we attach a new camera, so
  // it can also track the existing states
  container.tag_states[name] = state;

  // Save the tag on the node path
  np.set_tag(container.tag_name, name);

  // Apply the state on all cameras which are attached so far
  for (size_t i = 0; i < container.cameras.size(); ++i) {
    container.cameras[i]->set_tag_state(name, state);
  }
}

/**
 * @brief Cleans up all registered states.
 * @details This cleans up all states which were registered to the TagStateManager.
 *   It also calls Camera::clear_tag_states() on the main_cam_node and all attached
 *   cameras.
 */
void TagStateManager::
cleanup_states() {
  if (tagstatemgr_cat.is_info()) {
    tagstatemgr_cat.info() << "cleaning up states" << endl;
  }

  // Clear all tag states of the main camera
  DCAST(Camera, _main_cam_node.node())->clear_tag_states();

  // Clear the containers
  // XXX: Just iterate over the _container map
  cleanup_container_states(_containers["shadow"]);
  cleanup_container_states(_containers["voxelize"]);
  cleanup_container_states(_containers["envmap"]);
  cleanup_container_states(_containers["forward"]);
}

/**
 * @brief Cleans up the states of a given container
 * @details This cleans all tag states of the given container,
 *   and also calls Camera::clear_tag_states on every assigned camera.
 *
 * @param container Container to clear
 */
void TagStateManager::
cleanup_container_states(StateContainer& container) {
  for (size_t i = 0; i < container.cameras.size(); ++i) {
    container.cameras[i]->clear_tag_states();
  }
  container.tag_states.clear();
}

/**
 * @brief Registers a new camera to a given container
 * @details This registers a new camera to a container, and sets its initial
 *   state as well as the camera mask.
 *
 * @param container The container to add the camera to
 * @param source The camera to add
 */
void TagStateManager::
register_camera(StateContainer& container, Camera* source) {
  source->set_tag_state_key(container.tag_name);
  source->set_camera_mask(container.mask);

  // Construct an initial state which also disables color write, additionally
  // to the ColorWriteAttrib on each unique state.
  CPT(RenderState) state = RenderState::make_empty();

  if (!container.write_color) {
    state = state->set_attrib(ColorWriteAttrib::make(ColorWriteAttrib::C_off), 10000);
  }
  source->set_initial_state(state);

  // Store the camera so we can keep track of it
  container.cameras.push_back(source);
}

/**
 * @brief Unregisters a camera from a container
 * @details This unregisters a camera from the list of cameras of a given
 *   container. It also resets all tag states of the camera, and also its initial
 *   state.
 *
 * @param source Camera to unregister
 */
void TagStateManager::
unregister_camera(StateContainer& container, Camera* source) {
  CameraList& cameras = container.cameras;

  // Make sure the camera was attached so far
  if (std::find(cameras.begin(), cameras.end(), source) == cameras.end()) {
    tagstatemgr_cat.error()
      << "Called unregister_camera but camera was never registered!" << endl;
    return;
  }

  // Remove the camera from the list of attached cameras
  cameras.erase(std::remove(cameras.begin(), cameras.end(), source), cameras.end());

  // Reset the camera
  source->clear_tag_states();
  source->set_initial_state(RenderState::make_empty());
}
