#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#define USE_NURBSPP yes

#begin lib_target
  #define TARGET parametrics
  #define LOCAL_LIBS \
    pgraph grutil sgattrib linmath express putil pandabase
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
     classicNurbsCurve.I classicNurbsCurve.h config_parametrics.h  \
     cubicCurveseg.h parametricCurveDrawer.I  \
     parametricCurveDrawer.h curveFitter.I curveFitter.h  \
     hermiteCurve.h nurbsCurve.h nurbsCurveDrawer.I  \
     nurbsCurveDrawer.h nurbsCurveInterface.I  \
     nurbsCurveInterface.h parametricCurve.h  \
     parametricCurveCollection.I parametricCurveCollection.h  \
     piecewiseCurve.h

  #define INCLUDED_SOURCES \
     classicNurbsCurve.cxx config_parametrics.cxx cubicCurveseg.cxx  \
     parametricCurveDrawer.cxx curveFitter.cxx hermiteCurve.cxx  \
     nurbsCurveDrawer.cxx nurbsCurveInterface.cxx  \
     parametricCurve.cxx parametricCurveCollection.cxx  \
     piecewiseCurve.cxx 

  #define IF_NURBSPP_SOURCES \
    nurbsPPCurve.cxx nurbsPPCurve.h

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
    nurbsPPCurve.h \
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

