set(P3LINMATH_HEADERS
  aa_luse.h
  compose_matrix.h compose_matrix_src.I
  compose_matrix_src.h config_linmath.h
  configVariableColor.h configVariableColor.I
  coordinateSystem.h dbl2fltnames.h dblnames.h
  deg_2_rad.h deg_2_rad.I
  flt2dblnames.h fltnames.h
  intnames.h
  lcast_to.h lcast_to_src.h lcast_to_src.I
  lmatrix.h lmatrix_ext.h
  lmat_ops.h lmat_ops_src.h lmat_ops_src.I
  lmatrix3_src.h lmatrix3_src.I
  lmatrix4_src.h lmatrix4_src.I
  lorientation.h lorientation_src.h lorientation_src.I
  lpoint2.h lpoint2_src.I lpoint2_src.h
  lpoint3.h lpoint3_src.I lpoint3_src.h
  lpoint4.h lpoint4_src.I lpoint4_src.h
  lquaternion.h lquaternion_src.I
  lquaternion_src.h lrotation.h lrotation_src.I
  lrotation_src.h
  lsimpleMatrix.h lsimpleMatrix.I
  luse.I luse.N luse.h
  lvec2_ops.h lvec2_ops_src.I lvec2_ops_src.h lvec3_ops.h
  lvec3_ops_src.I lvec3_ops_src.h lvec4_ops.h lvec4_ops_src.I
  lvec4_ops_src.h
  lvecBase2.h lvecBase2_src.I lvecBase2_src.h
  lvecBase3.h lvecBase3_src.I lvecBase3_src.h
  lvecBase4.h lvecBase4_src.I lvecBase4_src.h
  lvector2.h lvector2_src.I lvector2_src.h
  lvector3.h lvector3_src.I lvector3_src.h
  lvector4.h lvector4_src.I lvector4_src.h
  lvector4.h lvector4_src.I lvector4_src.h
  lvecBase2_ext.h lvecBase3_ext.h lvecBase4_ext.h
  lpoint2_ext.h lpoint3_ext.h lpoint4_ext.h
  lvector2_ext.h lvector3_ext.h lvector4_ext.h
  mathNumbers.h mathNumbers.I
)

set(P3LINMATH_SOURCES
  compose_matrix.cxx config_linmath.cxx configVariableColor.cxx
  coordinateSystem.cxx lmatrix.cxx
  lorientation.cxx lpoint2.cxx
  lpoint3.cxx lpoint4.cxx lquaternion.cxx lrotation.cxx
  luse.cxx lvecBase2.cxx lvecBase3.cxx lvecBase4.cxx
  lvector2.cxx lvector3.cxx lvector4.cxx mathNumbers.cxx
)

composite_sources(p3linmath P3LINMATH_SOURCES)
add_component_library(p3linmath SYMBOL BUILDING_PANDA_LINMATH
  ${P3LINMATH_HEADERS} ${P3LINMATH_SOURCES})
target_link_libraries(p3linmath p3pandabase pandaexpress PKG::EIGEN)
target_interrogate(p3linmath ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3linmath
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3LINMATH_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
