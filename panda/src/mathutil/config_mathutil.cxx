// Filename: config_mathutil.cxx
// Created by:  drose (01Oct99)
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

#include "config_mathutil.h"
#include "boundingVolume.h"
#include "geometricBoundingVolume.h"
#include "finiteBoundingVolume.h"
#include "omniBoundingVolume.h"
#include "boundingSphere.h"
#include "boundingHexahedron.h"
#include "boundingLine.h"
#include "linmath_events.h"
#include "dconfig.h"

Configure(config_mathutil);
NotifyCategoryDef(mathutil, "");

const double fft_offset = config_mathutil.GetDouble("fft-offset", 0.001);
const double fft_factor = config_mathutil.GetDouble("fft-factor", 0.1);
const double fft_exponent = config_mathutil.GetDouble("fft-exponent", 4);
const double fft_error_threshold = config_mathutil.GetDouble("fft-error-threshold", 0.2);

ConfigureFn(config_mathutil) {
  BoundingHexahedron::init_type();
  BoundingSphere::init_type();
  BoundingVolume::init_type();
  FiniteBoundingVolume::init_type();
  GeometricBoundingVolume::init_type();
  OmniBoundingVolume::init_type();
  BoundingLine::init_type();
  EventStoreVec2::init_type("EventStoreVec2");
  EventStoreVec3::init_type("EventStoreVec3");
  EventStoreMat4::init_type("EventStoreMat4");

  EventStoreVec2::register_with_read_factory();
  EventStoreVec3::register_with_read_factory();
  EventStoreMat4::register_with_read_factory();
}

