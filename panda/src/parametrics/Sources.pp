#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET parametrics
  #define LOCAL_LIBS \
    grutil linmath express putil pandabase

  #define SOURCES \
    config_parametrics.cxx config_parametrics.h \
    curve.cxx curve.h \
    curveDrawer.cxx curveDrawer.h \
    nurbsCurve.cxx nurbsCurve.h \
    nurbsCurveDrawer.cxx nurbsCurveDrawer.h \
    hermiteCurve.cxx hermiteCurve.h \
    curveFitter.cxx curveFitter.h

  #define INSTALL_HEADERS \
    config_parametrics.h \
    curve.h \
    curveDrawer.h \
    nurbsCurve.h \
    nurbsCurveDrawer.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_parametrics
  #define LOCAL_LIBS \
    parametrics

  #define SOURCES \
    test_parametrics.cxx

#end test_bin_target

