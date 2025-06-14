set(P3IGATERUNTIME_HEADERS
  interrogate_request.h
  py_compat.h
  py_panda.h py_panda.I
  py_wrappers.h
)

install(FILES ${P3IGATERUNTIME_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
