add_executable(bam-info bamInfo.cxx bamInfo.h)
target_link_libraries(bam-info p3progbase panda)
install(TARGETS bam-info EXPORT Tools COMPONENT Tools DESTINATION bin)

add_executable(egg2bam eggToBam.cxx eggToBam.h)
target_link_libraries(egg2bam p3eggbase p3progbase panda)
install(TARGETS egg2bam EXPORT Tools COMPONENT Tools DESTINATION bin)

add_executable(bam2egg bamToEgg.cxx bamToEgg.h)
target_link_libraries(bam2egg p3converter p3eggbase p3progbase panda)
install(TARGETS bam2egg EXPORT Tools COMPONENT Tools DESTINATION bin)

add_executable(pts2bam ptsToBam.cxx ptsToBam.h)
target_link_libraries(pts2bam p3progbase pandaegg panda)
install(TARGETS pts2bam EXPORT Tools COMPONENT Tools DESTINATION bin)
