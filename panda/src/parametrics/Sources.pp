#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#define USE_NURBSPP yes

#begin lib_target
  #define TARGET parametrics
  #define LOCAL_LIBS \
    grutil sgattrib linmath express putil pandabase

  #define SOURCES \
    classicNurbsCurve.I classicNurbsCurve.cxx classicNurbsCurve.h \
    config_parametrics.cxx config_parametrics.h \
    cubicCurveseg.cxx cubicCurveseg.h \
    parametricCurveDrawer.I parametricCurveDrawer.cxx parametricCurveDrawer.h \
    curveFitter.I curveFitter.cxx curveFitter.h \
    hermiteCurve.cxx hermiteCurve.h \
    nurbsCurve.h \
    nurbsCurveDrawer.I nurbsCurveDrawer.cxx nurbsCurveDrawer.h \
    nurbsCurveInterface.I nurbsCurveInterface.cxx nurbsCurveInterface.h \
    parametricCurve.cxx parametricCurve.h \
    parametricCurveCollection.I parametricCurveCollection.cxx \
    parametricCurveCollection.h \
    piecewiseCurve.cxx piecewiseCurve.h

  #define IF_NURBSPP_SOURCES \
    nurbsPPCurve.cxx nurbsPPCurve.I nurbsPPCurve.h

  #define INSTALL_HEADERS \
    classicNurbsCurve.I classicNurbsCurve.h \
    config_parametrics.h \
    cubicCurveseg.h \
    parametricCurveDrawer.I parametricCurveDrawer.h \
    curveFitter.I curveFitter.h \
    hermiteCurve.h \
    nurbsCurve.h \
    nurbsCurveDrawer.I nurbsCurveDrawer.h \
    nurbsCurveInterface.I nurbsCurveInterface.h \
    nurbsPPCurve.h nurbsPPCurve.I \
    parametricCurve.h \
    parametricCurveCollection.I parametricCurveCollection.h \
    piecewiseCurve.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_parametrics
  #define LOCAL_LIBS \
    parametrics

  #define SOURCES \
    test_parametrics.cxx

#end test_bin_target

