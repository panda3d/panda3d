// Filename: config_mathutil.cxx
// Created by:  drose (01Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "config_mathutil.h"
#include "boundingVolume.h"
#include "geometricBoundingVolume.h"
#include "finiteBoundingVolume.h"
#include "omniBoundingVolume.h"
#include "boundingSphere.h"
#include "boundingBox.h"
#include "boundingHexahedron.h"
#include "boundingLine.h"
#include "boundingPlane.h"
#include "linmath_events.h"
#include "dconfig.h"
#include "pandaSystem.h"

Configure(config_mathutil);
NotifyCategoryDef(mathutil, "");

ConfigVariableDouble fft_offset
("fft-offset", 0.001);

ConfigVariableDouble fft_factor
("fft-factor", 0.1);

ConfigVariableDouble fft_exponent
("fft-exponent", 4);

ConfigVariableDouble fft_error_threshold
("fft-error-threshold", 0.2);

ConfigVariableEnum<BoundingVolume::BoundsType> bounds_type
("bounds-type", BoundingVolume::BT_sphere,
 PRC_DESC("Specify the type of bounding volume that is created automatically "
          "by Panda to enclose geometry.  Use 'sphere' or 'box', or use "
          "'best' to let Panda decide which is most appropriate."));

ConfigureFn(config_mathutil) {
  BoundingHexahedron::init_type();
  BoundingSphere::init_type();
  BoundingBox::init_type();
  BoundingVolume::init_type();
  FiniteBoundingVolume::init_type();
  GeometricBoundingVolume::init_type();
  OmniBoundingVolume::init_type();
  BoundingLine::init_type();
  BoundingPlane::init_type();
  EventStoreVec2::init_type("EventStoreVec2");
  EventStoreVec3::init_type("EventStoreVec3");
  EventStoreMat4::init_type("EventStoreMat4");

  EventStoreVec2::register_with_read_factory();
  EventStoreVec3::register_with_read_factory();
  EventStoreMat4::register_with_read_factory();

#ifdef HAVE_FFTW
  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("fftw");
#endif  // FFTW
}

