add_executable(image-fix-hidden-color imageFixHiddenColor.cxx imageFixHiddenColor.h)
target_link_libraries(image-fix-hidden-color p3imagebase)
install(TARGETS image-fix-hidden-color DESTINATION bin)

add_executable(image-info imageInfo.cxx imageInfo.h)
target_link_libraries(image-info p3imagebase)
install(TARGETS image-info DESTINATION bin)

add_executable(image-resize imageResize.cxx imageResize.h)
target_link_libraries(image-resize p3imagebase)
install(TARGETS image-resize DESTINATION bin)

add_executable(image-transform-colors imageTransformColors.cxx imageTransformColors.h)
target_link_libraries(image-transform-colors p3imagebase)
install(TARGETS image-transform-colors DESTINATION bin)

add_executable(image-trans imageTrans.cxx imageTrans.h)
target_link_libraries(image-trans p3imagebase)
install(TARGETS image-trans DESTINATION bin)
