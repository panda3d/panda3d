add_definitions(-DBUILDING_PANDA)
set(PANDA_LINK_TARGETS p3pnmimagetypes p3device)

if(LINK_IN_PHYSX)
  add_definitions(-DBUILDING_PANDAPHYSX)
  set(PANDA_LINK_TARGETS
    ${PANDA_LINK_TARGETS}
    p3physx
  )
endif()

add_library(panda panda.cxx)
target_link_libraries(panda ${PANDA_LINK_TARGETS})

install(TARGETS panda DESTINATION lib)
