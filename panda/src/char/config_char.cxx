// Filename: config_char.cxx
// Created by:  drose (28Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "config_char.h"
#include "character.h"
#include "characterJoint.h"
#include "characterJointBundle.h"
#include "characterSlider.h"
#include "computedVertices.h"
#include "dynamicVertices.h"

#include <dconfig.h>
#include <luse.h>

Configure(config_char);
NotifyCategoryDef(char, "");

ConfigureFn(config_char) {
  Character::init_type();
  CharacterJoint::init_type();
  CharacterJointBundle::init_type();
  CharacterSlider::init_type();
  ComputedVertices::init_type();
  DynamicVertices::init_type();

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
}

