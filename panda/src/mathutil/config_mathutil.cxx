// Filename: config_mathutil.cxx
// Created by:  drose (01Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "config_mathutil.h"
#include "boundingVolume.h"
#include "geometricBoundingVolume.h"
#include "finiteBoundingVolume.h"
#include "omniBoundingVolume.h"
#include "boundingSphere.h"
#include "boundingHexahedron.h"
#include "boundingLine.h"
#include <dconfig.h>

Configure(config_mathutil);
NotifyCategoryDef(mathutil, "");

ConfigureFn(config_mathutil) {
  BoundingHexahedron::init_type();
  BoundingSphere::init_type();
  BoundingVolume::init_type();
  FiniteBoundingVolume::init_type();
  GeometricBoundingVolume::init_type();
  OmniBoundingVolume::init_type();
  BoundingLine::init_type();
}

