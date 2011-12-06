#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define TARGET p3chan
  #define LOCAL_LIBS \
    p3pgraph p3putil p3linmath p3mathutil p3event
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
    animBundle.I animBundle.h \
    animBundleNode.I animBundleNode.h \
    animChannel.I animChannel.h animChannelBase.I  \
    animChannelBase.h \
    animChannelMatrixDynamic.I animChannelMatrixDynamic.h \
    animChannelMatrixFixed.I animChannelMatrixFixed.h \
    animChannelMatrixXfmTable.I animChannelMatrixXfmTable.h \
    animChannelScalarDynamic.I animChannelScalarDynamic.h \
    animChannelScalarTable.I animChannelScalarTable.h \
    animControl.I animControl.N  \
    animControl.h animControlCollection.I  \
    animControlCollection.h animGroup.I animGroup.h \
    animPreloadTable.I animPreloadTable.h \
    auto_bind.h  \
    bindAnimRequest.I bindAnimRequest.h \
    config_chan.h \
    movingPart.I movingPart.h \
    movingPartBase.I movingPartBase.h  \
    movingPartMatrix.I movingPartMatrix.h movingPartScalar.I  \
    movingPartScalar.h partBundle.I partBundle.N partBundle.h  \
    partBundleHandle.I partBundleHandle.h \
    partBundleNode.I partBundleNode.h \
    partGroup.I partGroup.h  \
    partSubset.I partSubset.h \
    vector_PartGroupStar.h 

  #define INCLUDED_SOURCES  \
    animBundle.cxx \
    animBundleNode.cxx \
    animChannel.cxx  \
    animChannelBase.cxx \
    animChannelMatrixDynamic.cxx  \
    animChannelMatrixFixed.cxx  \
    animChannelMatrixXfmTable.cxx  \
    animChannelScalarDynamic.cxx \
    animChannelScalarTable.cxx \
    animControl.cxx  \
    animControlCollection.cxx animGroup.cxx \
    animPreloadTable.cxx \
    auto_bind.cxx  \
    bindAnimRequest.cxx \
    config_chan.cxx movingPartBase.cxx movingPartMatrix.cxx  \
    movingPartScalar.cxx partBundle.cxx \
    partBundleHandle.cxx \
    partBundleNode.cxx \
    partGroup.cxx \
    partSubset.cxx \
    vector_PartGroupStar.cxx 

  #define INSTALL_HEADERS \
    animBundle.I animBundle.h \
    animBundleNode.I animBundleNode.h \
    animChannel.I animChannel.h animChannelBase.I animChannelBase.h \
    animChannelFixed.I animChannelFixed.h \
    animChannelMatrixDynamic.I animChannelMatrixDynamic.h \
    animChannelMatrixFixed.I animChannelMatrixFixed.h \
    animChannelMatrixXfmTable.I animChannelMatrixXfmTable.h \
    animChannelScalarDynamic.I animChannelScalarDynamic.h \
    animChannelScalarTable.I animChannelScalarTable.h \
    animControl.I animControl.h \
    animControlCollection.I animControlCollection.h animGroup.I \
    animGroup.h \
    animPreloadTable.I animPreloadTable.h \
    auto_bind.h  \
    bindAnimRequest.I bindAnimRequest.h \
    config_chan.h \
    movingPart.I movingPart.h movingPartBase.I \
    movingPartBase.h movingPartMatrix.I movingPartMatrix.h \
    movingPartScalar.I movingPartScalar.h partBundle.I partBundle.h \
    partBundleHandle.I partBundleHandle.h \
    partBundleNode.I partBundleNode.h \
    partGroup.I partGroup.h \
    partSubset.I partSubset.h \
    vector_PartGroupStar.h
    
  #define IGATESCAN all

#end lib_target

