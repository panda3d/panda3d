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
 * @brief Registers a new camera which renders a certain pass
 * @details This registers a new camera which will be used to render the given
 *   pass. The TagStateManager will keep track of the camera and
 *   applies all registered states onto the camera with Camera::set_tag_state.
 *   It also applies the appropriate camera mask to the camera,
 *   and sets an initial state to disable color write depending on the pass.
 *
 * @param source Camera which will be used to render shadows
 */
inline void TagStateManager::
register_camera(const std::string& name, Camera* source) {
  ContainerList::iterator entry = _containers.find(name);
  nassertv(entry != _containers.end());
  register_camera(entry->second, source);
}

/**
 * @brief Unregisters a camera from the list of shadow cameras
 * @details This unregisters a camera from the list of shadows cameras. It also
 *   resets all tag states of the camera, and also its initial state.
 *
 * @param source Camera to unregister
 */
inline void TagStateManager::
unregister_camera(const std::string& name, Camera* source) {
  ContainerList::iterator entry = _containers.find(name);
  nassertv(entry != _containers.end());
  unregister_camera(entry->second, source);
}

/**
 * @brief Applies a given state for a pass to a NodePath
 * @details This applies a shader to the given NodePath which is used when the
 *   NodePath is rendered by any registered camera for that pass.
 *   It also disables color write depending on the pass.
 *
 * @param np The nodepath to apply the shader to
 * @param shader A handle to the shader to apply
 * @param name Name of the state, should be a unique identifier
 * @param sort Determines the sort with which the shader will be applied.
 */
inline void TagStateManager::
apply_state(const std::string& state, NodePath np, Shader* shader,
            const std::string &name, int sort) {
  ContainerList::iterator entry = _containers.find(state);
  nassertv(entry != _containers.end());
  apply_state(entry->second, np, shader, name, sort);
}

/**
 * @brief Returns the render mask for the given state
 * @details This returns the mask of a given render pass, which can be used
 *   to either show or hide objects from this pass.
 *
 * @param container_name Name of the render-pass
 * @return Bit mask of the render pass
 */
inline BitMask32 TagStateManager::
get_mask(const std::string &container_name) {
  if (container_name == "gbuffer") {
    return BitMask32::bit(1);
  }
  ContainerList::iterator entry = _containers.find(container_name);
  nassertr(entry != _containers.end(), BitMask32());
  return entry->second.mask;
}
