// Filename: config_mathutil.cxx
// Created by:  drose (01Oct99)
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

#ifdef HAVE_FFTW
  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("fftw");
#endif  // FFTW
}

