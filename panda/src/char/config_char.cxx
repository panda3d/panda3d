/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_char.cxx
 * @author drose
 * @date 2000-02-28
 */

#include "config_char.h"
#include "character.h"
#include "characterJoint.h"
#include "characterJointBundle.h"
#include "characterJointEffect.h"
#include "characterSlider.h"
#include "characterVertexSlider.h"
#include "jointVertexTransform.h"
#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_CHAR)
  #error Buildsystem error: BUILDING_PANDA_CHAR not defined
#endif

Configure(config_char);
NotifyCategoryDef(char, "");

ConfigureFn(config_char) {
  init_libchar();
}

ConfigVariableBool even_animation
("even-animation", false,
 PRC_DESC("When this is true, characters' vertices will be recomputed "
          "every frame, whether they need it or not.  This will tend to "
          "balance out the frame rate so that it is more uniformly slow.  "
          "The default is to compute vertices only when they need to be "
          "computed, which can lead to an uneven frame rate."));


/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libchar() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  Character::init_type();
  CharacterJoint::init_type();
  CharacterJointBundle::init_type();
  CharacterJointEffect::init_type();
  CharacterSlider::init_type();
  CharacterVertexSlider::init_type();
  JointVertexTransform::init_type();

  // Registration of writeable object's creation functions with BamReader's
  // factory
  Character::register_with_read_factory();
  CharacterJoint::register_with_read_factory();
  CharacterJointBundle::register_with_read_factory();
  CharacterJointEffect::register_with_read_factory();
  CharacterSlider::register_with_read_factory();
  CharacterVertexSlider::register_with_read_factory();
  JointVertexTransform::register_with_read_factory();
}
