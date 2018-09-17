/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_parametrics.cxx
 * @author drose
 * @date 2000-03-19
 */

#include "nurbsCurve.h"
#include "config_parametrics.h"
#include "cubicCurveseg.h"
#include "curveFitter.h"
#include "hermiteCurve.h"
#include "nurbsCurveInterface.h"
#include "parametricCurve.h"
#include "piecewiseCurve.h"
#include "ropeNode.h"
#include "sheetNode.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_PARAMETRICS)
  #error Buildsystem error: BUILDING_PANDA_PARAMETRICS not defined
#endif

Configure(config_parametrics);
NotifyCategoryDef(parametrics, "");

ConfigureFn(config_parametrics) {
  NurbsCurve::init_type();
  CubicCurveseg::init_type();
  CurveFitter::init_type();
  HermiteCurve::init_type();
  NurbsCurveInterface::init_type();
  ParametricCurve::init_type();
  PiecewiseCurve::init_type();
  RopeNode::init_type();
  SheetNode::init_type();

  NurbsCurve::register_with_read_factory();
  CubicCurveseg::register_with_read_factory();
  HermiteCurve::register_with_read_factory();
  RopeNode::register_with_read_factory();
  SheetNode::register_with_read_factory();
}
