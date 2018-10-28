add_executable(egg2x eggToX.cxx eggToX.h)
target_link_libraries(egg2x p3xfileegg p3eggbase p3progbase)
install(TARGETS egg2x DESTINATION bin)

add_executable(x-trans xFileTrans.cxx xFileTrans.h)
target_link_libraries(x-trans p3progbase p3xfile)
install(TARGETS x-trans DESTINATION bin)

add_executable(x2egg xFileToEgg.cxx xFileToEgg.h)
target_link_libraries(x2egg p3xfileegg p3eggbase p3progbase)
install(TARGETS x2egg DESTINATION bin)
