set(P3VIEWDISPLAY_HEADERS
  config_androiddisplay.h
  androidGraphicsPipe.h androidGraphicsPipe.I
  androidGraphicsStateGuardian.h androidGraphicsStateGuardian.I
  androidGraphicsWindow.h androidGraphicsWindow.I

)

set(P3VIEWDISPLAY_SOURCES
  config_androiddisplay.cxx
  androidGraphicsPipe.cxx
  androidGraphicsStateGuardian.cxx
  androidGraphicsWindow.cxx
)

composite_sources(p3androiddisplay P3VIEWDISPLAY_SOURCES)

if(HAVE_GLES1)
    add_component_library(p3androiddisplay_gles1 SYMBOL BUILDING_PANDAGLES
        INIT init_libandroiddisplay config_androiddisplay.h
        ${P3VIEWDISPLAY_HEADERS} ${P3VIEWDISPLAY_SOURCES})
    target_compile_definitions(p3androiddisplay_gles1 PUBLIC OPENGLES_1)
    target_link_libraries(p3androiddisplay_gles2 p3glesgsg p3glstuff EGL PKG::GLES)


    if(NOT BUILD_METALIBS)
        install(TARGETS p3androiddisplay_gles2
            EXPORT OpenGLES2 COMPONENT OpenGLES2
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/panda3d
            ARCHIVE COMPONENT OpenGLES2Devel)
    endif()

    install(FILES ${P3VIEWDISPLAY_HEADERS} COMPONENT OpenGLES2Devel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
endif()

if(HAVE_GLES2)
    add_component_library(p3androiddisplay_gles2 SYMBOL BUILDING_PANDAGLES2
        INIT init_libandroiddisplay config_androiddisplay.h
        ${P3VIEWDISPLAY_HEADERS} ${P3VIEWDISPLAY_SOURCES})
    target_compile_definitions(p3androiddisplay_gles2 PUBLIC OPENGLES_2)
    # panda from p3x11display ?
    target_link_libraries(p3androiddisplay_gles2 p3gles2gsg EGL PKG::GLES2)


    if(NOT BUILD_METALIBS)
        install(TARGETS p3androiddisplay_gles2
            EXPORT OpenGLES2 COMPONENT OpenGLES2
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/panda3d
            ARCHIVE COMPONENT OpenGLES2Devel)
    endif()

    install(FILES ${P3VIEWDISPLAY_HEADERS} COMPONENT OpenGLES2Devel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

endif()


