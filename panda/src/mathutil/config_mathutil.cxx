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

const double fft_offset = config_mathutil.GetDouble("fft-offset", 0.001);
const double fft_factor = config_mathutil.GetDouble("fft-factor", 0.1);
const double fft_exponent = config_mathutil.GetDouble("fft-exponent", 4);

ConfigureFn(config_mathutil) {
  BoundingHexahedron::init_type();
  BoundingSphere::init_type();
  BoundingVolume::init_type();
  FiniteBoundingVolume::init_type();
  GeometricBoundingVolume::init_type();
  OmniBoundingVolume::init_type();
  BoundingLine::init_type();
}

