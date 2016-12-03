add_executable(bam-info bamInfo.cxx bamInfo.h)
target_link_libraries(bam-info p3progbase p3pgraphnodes p3recorder)
install(TARGETS bam-info DESTINATION bin)

add_executable(egg2bam eggToBam.cxx eggToBam.h)
target_link_libraries(egg2bam p3eggbase p3progbase)
install(TARGETS egg2bam DESTINATION bin)

add_executable(bam2egg bamToEgg.cxx bamToEgg.h)
target_link_libraries(bam2egg p3converter p3eggbase p3progbase)
install(TARGETS bam2egg DESTINATION bin)

add_executable(pts2bam ptsToBam.cxx ptsToBam.h)
target_link_libraries(pts2bam p3progbase p3pgraph p3pgraphnodes p3egg2pg)
install(TARGETS pts2bam DESTINATION bin)
