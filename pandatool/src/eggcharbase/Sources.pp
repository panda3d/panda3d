#begin ss_lib_target
  #define TARGET p3eggcharbase
  #define LOCAL_LIBS \
    p3eggbase p3progbase
  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3event:c p3linmath:c p3mathutil:c p3pnmimage:c p3putil:c \
    p3pipeline:c p3pstatclient:c p3downloader:c p3net:c p3nativenet:c \
    panda:m \
    p3pandabase:c p3express:c pandaexpress:m \
    p3interrogatedb:c p3prc:c p3dconfig:c dtooolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m
  #define USE_PACKAGES zlib
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \ 
     config_eggcharbase.h eggBackPointer.h \
     eggCharacterCollection.h eggCharacterCollection.I \
     eggCharacterData.h eggCharacterData.I \
     eggCharacterDb.I eggCharacterDb.h \
     eggCharacterFilter.h \
     eggComponentData.h eggComponentData.I \
     eggJointData.h \
     eggJointData.I eggJointPointer.h eggJointPointer.I \
     eggJointNodePointer.h \
     eggMatrixTablePointer.h eggScalarTablePointer.h \
     eggSliderData.h eggSliderData.I \
     eggVertexPointer.h
    
  #define INCLUDED_SOURCES \
     config_eggcharbase.cxx eggBackPointer.cxx \
     eggCharacterCollection.cxx eggCharacterData.cxx \
     eggCharacterDb.cxx \
     eggCharacterFilter.cxx eggComponentData.cxx eggJointData.cxx \
     eggJointPointer.cxx eggJointNodePointer.cxx \
     eggMatrixTablePointer.cxx eggScalarTablePointer.cxx \
     eggSliderData.cxx \
     eggSliderPointer.cxx \
     eggVertexPointer.cxx 

  #define INSTALL_HEADERS \
    eggBackPointer.h \
    eggCharacterCollection.I eggCharacterCollection.h \
    eggCharacterData.I eggCharacterData.h \
    eggCharacterDb.I eggCharacterDb.h \
    eggCharacterFilter.h \
    eggComponentData.I eggComponentData.h \
    eggJointData.h eggJointData.I \
    eggJointPointer.h eggJointPointer.I \
    eggJointNodePointer.h \
    eggMatrixTablePointer.h \
    eggScalarTablePointer.h \
    eggSliderData.I eggSliderData.h \
    eggVertexPointer.h

#end ss_lib_target

