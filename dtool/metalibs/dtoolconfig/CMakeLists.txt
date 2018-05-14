set(DTOOLCONFIG_LINK_TARGETS p3prc p3dconfig p3interrogatedb)
add_metalib(p3dtoolconfig dtoolconfig.cxx COMPONENTS ${DTOOLCONFIG_LINK_TARGETS})
install(TARGETS p3dtoolconfig DESTINATION lib)
