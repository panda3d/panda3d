// Filename: config_parametrics.cxx
// Created by:  drose (19Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "classicNurbsCurve.h"
#include "config_parametrics.h"
#include "cubicCurveseg.h"
#include "curveFitter.h"
#include "hermiteCurve.h"
#include "nurbsCurveDrawer.h"
#include "nurbsCurveInterface.h"
#include "parametricCurve.h"
#include "parametricCurveDrawer.h"
#include "piecewiseCurve.h"

#ifdef HAVE_NURBSPP
#include "nurbsPPCurve.h"
#endif

#include "get_config_path.h"

Configure(config_parametrics);
NotifyCategoryDef(parametrics, "");

ConfigureFn(config_parametrics) {
  ClassicNurbsCurve::init_type();
  CubicCurveseg::init_type();
  CurveFitter::init_type();
  HermiteCurve::init_type();
  NurbsCurveDrawer::init_type();
  NurbsCurveInterface::init_type();
  ParametricCurve::init_type();
  ParametricCurveDrawer::init_type();
  PiecewiseCurve::init_type();

#ifdef HAVE_NURBSPP
  NurbsPPCurve::init_type();
#endif

  ClassicNurbsCurve::register_with_read_factory();
  CubicCurveseg::register_with_read_factory();
  HermiteCurve::register_with_read_factory();
}

const DSearchPath &
get_parametrics_path() {
  static DSearchPath *parametrics_path = NULL;
  return get_config_path("parametrics-path", parametrics_path);
}
