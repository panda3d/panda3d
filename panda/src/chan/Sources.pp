#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET chan
  #define LOCAL_LIBS \
    pgraph putil linmath mathutil event
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
    animBundle.I animBundle.h \
    animBundleNode.I animBundleNode.h \
    animChannel.I animChannel.h animChannelBase.I  \
    animChannelBase.h \
    animChannelMatrixDynamic.I animChannelMatrixDynamic.h \
    animChannelMatrixXfmTable.I animChannelMatrixXfmTable.h \
    animChannelScalarDynamic.I animChannelScalarDynamic.h \
    animChannelScalarTable.I animChannelScalarTable.h \
    animControl.I animControl.N  \
    animControl.h animControlCollection.I  \
    animControlCollection.h animGroup.I animGroup.h auto_bind.h  \
    config_chan.h movingPartBase.I movingPartBase.h  \
    movingPartMatrix.I movingPartMatrix.h movingPartScalar.I  \
    movingPartScalar.h partBundle.I partBundle.N partBundle.h  \
    partBundleNode.I partBundleNode.h \
    partGroup.I partGroup.h  \
    vector_PartGroupStar.h 

  #define INCLUDED_SOURCES  \
    animBundle.cxx \
    animBundleNode.cxx \
    animChannel.cxx  \
    animChannelBase.cxx \
    animChannelMatrixDynamic.cxx  \
    animChannelMatrixXfmTable.cxx  \
    animChannelScalarDynamic.cxx \
    animChannelScalarTable.cxx \
    animControl.cxx  \
    animControlCollection.cxx animGroup.cxx auto_bind.cxx  \
    config_chan.cxx movingPartBase.cxx movingPartMatrix.cxx  \
    movingPartScalar.cxx partBundle.cxx \
    partBundleNode.cxx \
    partGroup.cxx vector_PartGroupStar.cxx 

  #define INSTALL_HEADERS \
    animBundle.I animBundle.h \
    animBundleNode.I animBundleNode.h \
    animChannel.I animChannel.h animChannelBase.I animChannelBase.h \
    animChannelFixed.I animChannelFixed.h \
    animChannelMatrixDynamic.I animChannelMatrixDynamic.h \
    animChannelMatrixXfmTable.I animChannelMatrixXfmTable.h \
    animChannelScalarDynamic.I animChannelScalarDynamic.h \
    animChannelScalarTable.I animChannelScalarTable.h \
    animControl.I animControl.h \
    animControlCollection.I animControlCollection.h animGroup.I \
    animGroup.h auto_bind.h config_chan.h \
    movingPart.I movingPart.h movingPartBase.I \
    movingPartBase.h movingPartMatrix.I movingPartMatrix.h \
    movingPartScalar.I movingPartScalar.h partBundle.I partBundle.h \
    partBundleNode.I partBundleNode.h \
    partGroup.I partGroup.h \
    vector_PartGroupStar.h
    
  #define IGATESCAN all

#end lib_target

