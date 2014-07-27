set(P3CONVERTER_HEADERS
    somethingToEggConverter.I somethingToEggConverter.h
    eggToSomethingConverter.I eggToSomethingConverter.h)

set(P3CONVERTER_SOURCES
    somethingToEggConverter.cxx eggToSomethingConverter.cxx)

add_library(p3converter STATIC ${P3CONVERTER_HEADERS} ${P3CONVERTER_SOURCES})
target_link_libraries(p3converter p3pandatoolbase pandaegg p3pystub)
