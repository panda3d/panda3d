set(PANDA_LINK_TARGETS
  p3chan p3char p3collide p3cull p3device p3dgraph p3display p3dxml
  p3event p3gobj p3grutil p3gsgbase p3linmath p3mathutil
  p3movies p3parametrics p3pgraph p3pgraphnodes p3pgui p3pipeline
  p3pnmimage p3pnmimagetypes p3pstatclient p3putil p3recorder p3text p3tform
)

if(WANT_NATIVE_NET)
  list(APPEND PANDA_LINK_TARGETS p3nativenet)
  if(HAVE_NET)
    list(APPEND PANDA_LINK_TARGETS p3net)
  endif()
endif()

if(HAVE_AUDIO)
  list(APPEND PANDA_LINK_TARGETS p3audio)
endif()

if(HAVE_FREETYPE)
  list(APPEND PANDA_LINK_TARGETS p3pnmtext)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "CoreDevel")
add_metalib(panda INIT init_libpanda panda.h COMPONENTS ${PANDA_LINK_TARGETS})
unset(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME)

install(TARGETS panda
  EXPORT Core COMPONENT Core
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT CoreDevel)
