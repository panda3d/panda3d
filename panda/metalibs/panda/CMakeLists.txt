set(PANDA_LINK_TARGETS
    p3chan p3char p3collide p3cull p3device p3dgraph p3display p3dxml
    p3event p3gobj p3grutil p3gsgbase p3linmath p3mathutil p3nativenet
    p3net p3movies p3parametrics p3pgraph p3pgraphnodes p3pgui p3pipeline
    p3pnmimage p3pnmimagetypes p3pstatclient p3putil p3recorder p3text p3tform)

if(HAVE_AUDIO)
  list(APPEND PANDA_LINK_TARGETS p3audio)
endif()
if(HAVE_FREETYPE)
  list(APPEND PANDA_LINK_TARGETS p3pnmtext)
endif()

add_metalib(panda INIT init_libpanda panda.h COMPONENTS ${PANDA_LINK_TARGETS})

install(TARGETS panda DESTINATION lib RUNTIME DESTINATION bin)
