#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET chan
  #define LOCAL_LIBS \
    putil linmath mathutil event graph sgattrib

  #define SOURCES \
    animBundle.I animBundle.cxx animBundle.h animBundleNode.I \
    animBundleNode.cxx animBundleNode.h animChannel.I animChannel.cxx \
    animChannel.h animChannelBase.I animChannelBase.cxx \
    animChannelBase.h animChannelMatrixXfmTable.I \
    animChannelMatrixXfmTable.cxx animChannelMatrixXfmTable.h \
    animChannelScalarTable.I animChannelScalarTable.cxx \
    animChannelScalarTable.h animControl.I animControl.N \
    animControl.cxx animControl.h animControlCollection.I \
    animControlCollection.cxx animControlCollection.h animGroup.I \
    animGroup.cxx animGroup.h auto_bind.cxx auto_bind.h config_chan.cxx \
    config_chan.h movingPartBase.I movingPartBase.cxx movingPartBase.h \
    movingPartMatrix.I movingPartMatrix.cxx movingPartMatrix.h \
    movingPartScalar.I movingPartScalar.cxx movingPartScalar.h \
    partBundle.I partBundle.N partBundle.cxx partBundle.h \
    partBundleNode.I partBundleNode.cxx partBundleNode.h partGroup.I \
    partGroup.cxx partGroup.h vector_PartGroupStar.cxx \
    vector_PartGroupStar.h

  #define INSTALL_HEADERS \
    animBundle.I animBundle.h animBundleNode.I animBundleNode.h \
    animChannel.I animChannel.h animChannelBase.I animChannelBase.h \
    animChannelFixed.I animChannelFixed.h animChannelMatrixXfmTable.I \
    animChannelMatrixXfmTable.h animChannelScalarTable.I \
    animChannelScalarTable.h animControl.I animControl.h \
    animControlCollection.I animControlCollection.h animGroup.I \
    animGroup.h auto_bind.h config_chan.h \
    movingPart.I movingPart.h movingPartBase.I \
    movingPartBase.h movingPartMatrix.I movingPartMatrix.h \
    movingPartScalar.I movingPartScalar.h partBundle.I partBundle.h \
    partBundleNode.I partBundleNode.h partGroup.I partGroup.h \
    vector_PartGroupStar.h
    
  #define PRECOMPILED_HEADER chan_headers.h 

  #define IGATESCAN all

#end lib_target

