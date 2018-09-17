/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pgraphnodes.cxx
 * @author drose
 * @date 2008-11-05
 */

#include "config_pgraphnodes.h"

#include "ambientLight.h"
#include "callbackData.h"
#include "callbackNode.h"
#include "callbackObject.h"
#include "computeNode.h"
#include "directionalLight.h"
#include "fadeLodNode.h"
#include "fadeLodNodeData.h"
#include "lightLensNode.h"
#include "lightNode.h"
#include "lodNode.h"
#include "nodeCullCallbackData.h"
#include "pointLight.h"
#include "rectangleLight.h"
#include "selectiveChildNode.h"
#include "sequenceNode.h"
#include "shaderGenerator.h"
#include "sphereLight.h"
#include "spotlight.h"
#include "switchNode.h"
#include "uvScrollNode.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_PGRAPHNODES)
  #error Buildsystem error: BUILDING_PANDA_PGRAPHNODES not defined
#endif

ConfigureDef(config_pgraphnodes);
NotifyCategoryDef(pgraphnodes, "");

ConfigureFn(config_pgraphnodes) {
  init_libpgraphnodes();
}

ConfigVariableEnum<LODNodeType> default_lod_type
("default-lod-type", LNT_pop,
 PRC_DESC("Set this to either 'pop' or 'fade' to determine the type of "
          "LODNode that is created by LODNode::make_default_lod()."));

ConfigVariableBool support_fade_lod
("support-fade-lod", true,
 PRC_DESC("Set this false to make FadeLOD nodes behave like regular LOD nodes "
          "(ignoring the fade time).  This may be useful, for instance, to "
          "test the performance impact of using FadeLOD nodes."));

ConfigVariableDouble lod_fade_time
("lod-fade-time", 0.5,
 PRC_DESC("The default amount of time (in seconds) over which a FadeLODNode "
          "transitions between its different levels."));

ConfigVariableString lod_fade_bin_name
("lod-fade-bin-name", "fixed",
 PRC_DESC("The default bin name in which to place the fading part of a "
          "FadeLODNode transition."));

ConfigVariableInt lod_fade_bin_draw_order
("lod-fade-bin-draw-order", 0,
 PRC_DESC("The default bin draw order to assign the fading part of a "
          "FadeLODNode transition."));

ConfigVariableInt lod_fade_state_override
("lod-fade-state-override", 1000,
 PRC_DESC("The default override value to assign to the fade attribs "
          "in order to effect a FadeLODNode transition."));

ConfigVariableBool verify_lods
("verify-lods", false,
 PRC_DESC("When this is true, LODNodes will test when they are rendered to "
          "ensure that each child's geometry fits entirely within the radius "
          "defined by its switch-out distance.  When it is false, LODNodes "
          "may have any switch in and out distances, regardless of the "
          "actual size of their geometry.  This test is only made in NDEBUG "
          "mode (the variable is ignored in a production build)."));

ConfigVariableInt parallax_mapping_samples
("parallax-mapping-samples", 3,
 PRC_DESC("Sets the amount of samples to use in the parallax mapping "
          "implementation. A value of 0 means to disable it entirely."));

ConfigVariableDouble parallax_mapping_scale
("parallax-mapping-scale", 0.1,
 PRC_DESC("Sets the strength of the effect of parallax mapping, that is, "
          "how much influence the height values have on the texture "
          "coordinates."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpgraphnodes() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  AmbientLight::init_type();
  CallbackData::init_type();
  CallbackNode::init_type();
  CallbackObject::init_type();
  ComputeNode::init_type();
  DirectionalLight::init_type();
  FadeLODNode::init_type();
  FadeLODNodeData::init_type();
  LightLensNode::init_type();
  LightNode::init_type();
  LODNode::init_type();
  NodeCullCallbackData::init_type();
  PointLight::init_type();
  RectangleLight::init_type();
  SelectiveChildNode::init_type();
  SequenceNode::init_type();
  ShaderGenerator::init_type();
  SphereLight::init_type();
  Spotlight::init_type();
  SwitchNode::init_type();
  UvScrollNode::init_type();

  AmbientLight::register_with_read_factory();
  CallbackNode::register_with_read_factory();
  ComputeNode::register_with_read_factory();
  DirectionalLight::register_with_read_factory();
  FadeLODNode::register_with_read_factory();
  LightNode::register_with_read_factory();
  LODNode::register_with_read_factory();
  PointLight::register_with_read_factory();
  RectangleLight::register_with_read_factory();
  SelectiveChildNode::register_with_read_factory();
  SequenceNode::register_with_read_factory();
  SphereLight::register_with_read_factory();
  Spotlight::register_with_read_factory();
  SwitchNode::register_with_read_factory();
  UvScrollNode::register_with_read_factory();
}
