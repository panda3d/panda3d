#begin ss_lib_target
  #define TARGET eggcharbase
  #define LOCAL_LIBS \
    eggbase progbase
  #define OTHER_LIBS \
    egg:c panda:m

  #define SOURCES \
    eggBackPointer.cxx eggBackPointer.h \
    eggCharacterCollection.cxx eggCharacterCollection.h \
    eggCharacterCollection.I \
    eggCharacterData.cxx eggCharacterData.h eggCharacterData.I \
    eggCharacterFilter.cxx eggCharacterFilter.h \
    eggComponentData.cxx eggComponentData.h eggComponentData.I \
    eggJointData.cxx eggJointData.h eggJointData.I \
    eggJointPointer.cxx eggJointPointer.h \
    eggMatrixTablePointer.cxx eggMatrixTablePointer.h \
    eggSliderData.cxx eggSliderData.h eggSliderData.I \
    eggVertexPointer.cxx eggVertexPointer.h

  #define INSTALL_HEADERS \
    eggBackPointer.h \
    eggCharacterCollection.I eggCharacterCollection.h \
    eggCharacterData.I eggCharacterData.h eggCharacterFilter.h \
    eggComponentData.I eggComponentData.h \
    eggJointData.h eggJointData.I \
    eggJointPointer.h \
    eggMatrixTablePointer.h \
    eggSliderData.I eggSliderData.h \
    eggVertexPointer.h

#end ss_lib_target

