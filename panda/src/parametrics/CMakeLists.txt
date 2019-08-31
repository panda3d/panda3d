set(P3PARAMETRICS_HEADERS
  config_parametrics.h
  cubicCurveseg.h curveFitter.I curveFitter.h
  hermiteCurve.h
  nurbsCurve.I nurbsCurve.h
  nurbsCurveEvaluator.I nurbsCurveEvaluator.h
  nurbsCurveInterface.I nurbsCurveInterface.h
  nurbsCurveResult.I nurbsCurveResult.h
  nurbsBasisVector.I nurbsBasisVector.h
  nurbsSurfaceEvaluator.I nurbsSurfaceEvaluator.h
  nurbsSurfaceResult.I nurbsSurfaceResult.h
  nurbsVertex.h nurbsVertex.I
  parametricCurve.h
  parametricCurveCollection.I parametricCurveCollection.h
  piecewiseCurve.h
  ropeNode.I ropeNode.h
  sheetNode.I sheetNode.h
)

set(P3PARAMETRICS_SOURCES
  config_parametrics.cxx cubicCurveseg.cxx
  curveFitter.cxx hermiteCurve.cxx
  nurbsCurve.cxx
  nurbsCurveEvaluator.cxx
  nurbsCurveInterface.cxx
  nurbsCurveResult.cxx
  nurbsBasisVector.cxx
  nurbsSurfaceEvaluator.cxx
  nurbsSurfaceResult.cxx
  nurbsVertex.cxx
  parametricCurve.cxx parametricCurveCollection.cxx
  piecewiseCurve.cxx
  ropeNode.cxx
  sheetNode.cxx
)

composite_sources(p3parametrics P3PARAMETRICS_SOURCES)
add_component_library(p3parametrics NOINIT SYMBOL BUILDING_PANDA_PARAMETRICS
  ${P3PARAMETRICS_HEADERS} ${P3PARAMETRICS_SOURCES})
target_link_libraries(p3parametrics p3pgraph)
target_interrogate(p3parametrics ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3parametrics
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3PARAMETRICS_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
