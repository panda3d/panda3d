set(P3PYSTUB_HEADERS pystub.h)
set(P3PYSTUB_SOURCES pystub.cxx)

if(WIN32)
  # Win32 doesn't actually need this hack
  add_library(p3pystub INTERFACE)

else()
  add_library(p3pystub ${P3PYSTUB_HEADERS} ${P3PYSTUB_SOURCES})
  target_link_libraries(p3pystub p3dtool)
  install(FILES ${P3PYSTUB_HEADERS} COMPONENT CoreDevel DESTINATION include/panda3d)
  install(TARGETS p3pystub
    EXPORT Core COMPONENT Core
    DESTINATION lib
    RUNTIME DESTINATION bin
    ARCHIVE COMPONENT CoreDevel)

endif()
