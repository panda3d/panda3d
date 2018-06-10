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

#ifndef TAGSTATEMANAGER_H
#define TAGSTATEMANAGER_H

#include "pandabase.h"
#include "bitMask.h"
#include "camera.h"
#include "nodePath.h"
#include "shader.h"
#include "renderState.h"
#include "shaderAttrib.h"
#include "colorWriteAttrib.h"

NotifyCategoryDecl(tagstatemgr, EXPORT_CLASS, EXPORT_TEMPL);

/**
 * @brief This class handles all different tag states
 * @details The TagStateManager stores a list of RenderStates assigned to different
 *   steps in the pipeline. For example, there are a list of shadow states, which
 *   are applied whenever objects are rendered from a shadow camera.
 *
 *   The Manager also stores a list of all cameras used in the different stages,
 *   to keep track of the states used and to be able to attach new states.
 */
class TagStateManager {
PUBLISHED:
  TagStateManager(NodePath main_cam_node);
  ~TagStateManager();

  inline void apply_state(const std::string& state, NodePath np, Shader* shader, const std::string &name, int sort);
  void cleanup_states();

  inline void register_camera(const std::string& state, Camera* source);
  inline void unregister_camera(const std::string& state, Camera* source);
  inline BitMask32 get_mask(const std::string &container_name);

private:
  typedef std::vector<Camera*> CameraList;
  typedef pmap<std::string, CPT(RenderState)> TagStateList;

  struct StateContainer {
    CameraList cameras;
    TagStateList tag_states;
    std::string tag_name;
    BitMask32 mask;
    bool write_color;

    StateContainer() {};
    StateContainer(const std::string &tag_name, size_t mask, bool write_color)
      : tag_name(tag_name), mask(BitMask32::bit(mask)), write_color(write_color) {};
  };

  void apply_state(StateContainer& container, NodePath np, Shader* shader,
                   const std::string& name, int sort);
  void cleanup_container_states(StateContainer& container);
  void register_camera(StateContainer &container, Camera* source);
  void unregister_camera(StateContainer &container, Camera* source);

  typedef pmap<std::string, StateContainer> ContainerList;
  ContainerList _containers;

  NodePath _main_cam_node;
};


#include "tagStateManager.I"

#endif // TAGSTATEMANAGER_H
