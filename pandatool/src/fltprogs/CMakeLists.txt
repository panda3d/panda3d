add_executable(flt-info fltInfo.cxx fltInfo.h)
target_link_libraries(flt-info p3progbase p3flt)
install(TARGETS flt-info DESTINATION bin)

add_executable(flt-trans fltTrans.cxx fltTrans.h)
target_link_libraries(flt-trans p3progbase p3flt)
install(TARGETS flt-trans DESTINATION bin)

add_executable(fltcopy fltCopy.cxx fltCopy.h)
target_link_libraries(fltcopy p3progbase p3cvscopy p3flt)
install(TARGETS fltcopy DESTINATION bin)

add_executable(egg2flt eggToFlt.cxx eggToFlt.h)
target_link_libraries(egg2flt p3fltegg p3eggbase p3progbase)
install(TARGETS egg2flt DESTINATION bin)

add_executable(flt2egg fltToEgg.cxx fltToEgg.h)
target_link_libraries(flt2egg p3fltegg p3eggbase p3progbase)
install(TARGETS flt2egg DESTINATION bin)
