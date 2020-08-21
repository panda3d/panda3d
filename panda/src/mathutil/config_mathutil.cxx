/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_mathutil.cxx
 * @author drose
 * @date 1999-10-01
 */

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
#include "unionBoundingVolume.h"
#include "intersectionBoundingVolume.h"
#include "dconfig.h"
#include "pandaSystem.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_MATHUTIL)
  #error Buildsystem error: BUILDING_PANDA_MATHUTIL not defined
#endif

Configure(config_mathutil);
NotifyCategoryDef(mathutil, "");

ConfigureFn(config_mathutil) {
  init_libmathutil();
}

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
          "'best' to let Panda decide which is most appropriate.  You can "
          "also use 'fastest' if you don't want Panda to waste much time "
          "computing the most optimal bounding volume."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libmathutil() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  BoundingHexahedron::init_type();
  BoundingSphere::init_type();
  BoundingBox::init_type();
  BoundingVolume::init_type();
  FiniteBoundingVolume::init_type();
  GeometricBoundingVolume::init_type();
  OmniBoundingVolume::init_type();
  UnionBoundingVolume::init_type();
  IntersectionBoundingVolume::init_type();
  BoundingLine::init_type();
  BoundingPlane::init_type();

#ifdef HAVE_FFTW
  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("fftw");
#endif  // FFTW
}
