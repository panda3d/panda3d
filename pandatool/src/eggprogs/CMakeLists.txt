add_executable(egg-crop eggCrop.cxx eggCrop.h)
target_link_libraries(egg-crop p3eggbase)
install(TARGETS egg-crop DESTINATION bin)

add_executable(egg-list-textures eggListTextures.cxx eggListTextures.h)
target_link_libraries(egg-list-textures p3eggbase)
install(TARGETS egg-list-textures DESTINATION bin)

add_executable(egg-make-tube eggMakeTube.cxx eggMakeTube.h)
target_link_libraries(egg-make-tube p3eggbase)
install(TARGETS egg-make-tube DESTINATION bin)

add_executable(egg-rename eggRename.cxx eggRename.h)
target_link_libraries(egg-rename p3eggbase)
install(TARGETS egg-rename DESTINATION bin)

add_executable(egg-retarget-anim eggRetargetAnim.cxx eggRetargetAnim.h)
target_link_libraries(egg-retarget-anim p3eggcharbase)
install(TARGETS egg-retarget-anim DESTINATION bin)

add_executable(egg-texture-cards eggTextureCards.cxx eggTextureCards.h)
target_link_libraries(egg-texture-cards p3eggbase)
install(TARGETS egg-texture-cards DESTINATION bin)

add_executable(egg2c eggToC.cxx eggToC.h)
target_link_libraries(egg2c p3eggbase)
install(TARGETS egg2c DESTINATION bin)

add_executable(egg-topstrip eggTopstrip.cxx eggTopstrip.h)
target_link_libraries(egg-topstrip p3eggcharbase)
install(TARGETS egg-topstrip DESTINATION bin)

add_executable(egg-trans eggTrans.cxx eggTrans.h)
target_link_libraries(egg-trans p3eggbase)
install(TARGETS egg-trans DESTINATION bin)
