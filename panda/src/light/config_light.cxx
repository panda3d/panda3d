// Filename: config_light.cxx
// Created by:  drose (19Mar00)
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
