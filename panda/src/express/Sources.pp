#define LOCAL_LIBS pandabase
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET express
  #define USE_PACKAGES nspr crypto net
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
    bigEndian.h buffer.I buffer.h \
    checksumHashGenerator.I checksumHashGenerator.h circBuffer.I \
    circBuffer.h clockObject.I clockObject.h config_express.h \
    datagram.I datagram.h datagramGenerator.I \
    datagramGenerator.h datagramInputFile.I datagramInputFile.h \
    datagramIterator.I datagramIterator.h datagramOutputFile.I \
    datagramOutputFile.h datagramSink.I datagramSink.h \
    dcast.T dcast.h \
    error_utils.h \
    get_config_path.h hashGeneratorBase.I hashGeneratorBase.h \
    hashVal.I hashVal.h indent.I indent.h \
    indirectLess.I indirectLess.h \
    littleEndian.h \
    memoryInfo.I memoryInfo.h \
    memoryUsage.I memoryUsage.h \
    memoryUsagePointerCounts.I memoryUsagePointerCounts.h \
    memoryUsagePointers.I memoryUsagePointers.h \
    multifile.I multifile.h namable.I \
    namable.h nativeNumericData.I nativeNumericData.h \
    numeric_types.h \
    ordered_vector.h ordered_vector.I ordered_vector.T \
    pointerTo.I pointerTo.h \
    pointerToArray.I pointerToArray.h \
    profileTimer.I profileTimer.h \
    pta_uchar.h referenceCount.I referenceCount.h \
    register_type.I register_type.h \
    reversedNumericData.I reversedNumericData.h tokenBoard.I \
    tokenBoard.h trueClock.I trueClock.h typeHandle.I \
    typeHandle.h typedObject.I typedObject.h \
    typedReferenceCount.I typedReferenceCount.h typedef.h \
    typeRegistry.I typeRegistry.h \
    typeRegistryNode.I typeRegistryNode.h \
    vector_uchar.h \
    $[if $[HAVE_CRYPTO], \
       crypto_utils.cxx crypto_utils.h patchfile.I \
       patchfile.cxx patchfile.h ]

  #define INCLUDED_SOURCES  \
    buffer.cxx checksumHashGenerator.cxx clockObject.cxx \
    config_express.cxx datagram.cxx datagramGenerator.cxx \
    datagramInputFile.cxx datagramIterator.cxx \
    datagramOutputFile.cxx datagramSink.cxx dcast.cxx error_utils.cxx \
    get_config_path.cxx \
    hashGeneratorBase.cxx hashVal.cxx indent.cxx \
    memoryInfo.cxx memoryUsage.cxx memoryUsagePointerCounts.cxx \
    memoryUsagePointers.cxx multifile.cxx namable.cxx \
    nativeNumericData.cxx \
    ordered_vector.cxx \
    profileTimer.cxx \
    pta_uchar.cxx referenceCount.cxx register_type.cxx \
    reversedNumericData.cxx trueClock.cxx typeHandle.cxx \
    typedObject.cxx typedReferenceCount.cxx \
    typeRegistry.cxx typeRegistryNode.cxx vector_uchar.cxx

  #define INSTALL_HEADERS  \
    bigEndian.h buffer.I buffer.h checksumHashGenerator.I  \
    checksumHashGenerator.h circBuffer.I circBuffer.h clockObject.I \
    clockObject.h config_express.h datagram.I datagram.h \
    datagramGenerator.I datagramGenerator.h datagramInputFile.I \
    datagramInputFile.h datagramIterator.I datagramIterator.h \
    datagramOutputFile.I datagramOutputFile.h datagramSink.I \
    datagramSink.h dcast.T dcast.h \
    error_utils.h get_config_path.h hashGeneratorBase.I \
    hashGeneratorBase.h hashVal.I hashVal.h \
    indent.I indent.h \
    indirectLess.I indirectLess.h \
    littleEndian.h memoryInfo.I memoryInfo.h memoryUsage.I \
    memoryUsage.h memoryUsagePointerCounts.I \
    memoryUsagePointerCounts.h memoryUsagePointers.I \
    memoryUsagePointers.h multifile.I multifile.h namable.I namable.h \
    nativeNumericData.I nativeNumericData.h numeric_types.h \
    ordered_vector.h ordered_vector.I ordered_vector.T \
    patchfile.I patchfile.h pointerTo.I pointerTo.h \
    pointerToArray.I pointerToArray.h profileTimer.I \
    profileTimer.h pta_uchar.h referenceCount.I referenceCount.h \
    register_type.I register_type.h \
    reversedNumericData.I reversedNumericData.h tokenBoard.I \
    tokenBoard.h trueClock.I trueClock.h typeHandle.I typeHandle.h \
    typedObject.I typedObject.h typedReferenceCount.I \
    typedReferenceCount.h typedef.h \
    typeRegistry.I typeRegistry.h \
    typeRegistryNode.I typeRegistryNode.h \
    vector_uchar.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_types
  #define LOCAL_LIBS $[LOCAL_LIBS] express
  #define OTHER_LIBS pystub

  #define SOURCES \
    test_types.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_ordered_vector

  #define SOURCES \
    test_ordered_vector.cxx

  #define LOCAL_LIBS $[LOCAL_LIBS] putil
  #define OTHER_LIBS $[OTHER_LIBS] pystub

#end test_bin_target
