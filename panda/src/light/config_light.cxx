// Filename: config_light.cxx
// Created by:  drose (19Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "config_light.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "light.h"
#include "lightTransition.h"
#include "lightAttribute.h"
#include "pointLight.h"
#include "spotlight.h"

#include <dconfig.h>

Configure(config_light);
NotifyCategoryDef(light, "");

ConfigureFn(config_light) {
  AmbientLight::init_type();
  DirectionalLight::init_type();
  Light::init_type();
  LightTransition::init_type();
  LightAttribute::init_type();
  PointLight::init_type();
  Spotlight::init_type();
}
