// Filename: config_char.cxx
// Created by:  drose (28Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


#include "config_char.h"
#include "character.h"
#include "characterJoint.h"
#include "characterJointBundle.h"
#include "characterSlider.h"
#include "computedVertices.h"
#include "dynamicVertices.h"
#include "jointVertexTransform.h"
#include "dconfig.h"
#include "lmatrix4.h"

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


////////////////////////////////////////////////////////////////////
//     Function: init_libchar
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
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
  CharacterSlider::init_type();
  ComputedVertices::init_type();
  DynamicVertices::init_type();
  JointVertexTransform::init_type();

  // This isn't defined in this package, but it *is* essential that it
  // be initialized.  We have to do it explicitly here since template
  // statics don't necessarily resolve very well across dynamic
  // libraries.
  LMatrix4f::init_type();

  //Registration of writeable object's creation
  //functions with BamReader's factory
  Character::register_with_read_factory();
  CharacterJoint::register_with_read_factory();
  CharacterJointBundle::register_with_read_factory();
  CharacterSlider::register_with_read_factory();
  ComputedVertices::register_with_read_factory();
  JointVertexTransform::register_with_read_factory();
}

