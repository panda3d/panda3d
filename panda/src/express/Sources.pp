#define LOCAL_LIBS pandabase
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET express
  #define USE_NSPR yes
  #define USE_CRYPTO yes
  #define USE_NET yes
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
     bigEndian.h buffer.I buffer.h \
     checksumHashGenerator.I checksumHashGenerator.h circBuffer.I \
     circBuffer.h clockObject.I clockObject.h config_express.h \
     datagram.I datagram.h datagramGenerator.I \
     datagramGenerator.h datagramInputFile.I datagramInputFile.h \
     datagramIterator.I datagramIterator.h datagramOutputFile.I \
     datagramOutputFile.h datagramSink.I datagramSink.h \
     error_utils.h \
     get_config_path.h hashGeneratorBase.I hashGeneratorBase.h \
     hashVal.I hashVal.h indent.I indent.h littleEndian.h \
     memoryInfo.I memoryInfo.h \
     memoryUsage.I memoryUsage.h \
     memoryUsagePointerCounts.I memoryUsagePointerCounts.h \
     memoryUsagePointers.I memoryUsagePointers.h \
     multifile.I multifile.h namable.I \
     namable.h nativeNumericData.I nativeNumericData.h \
     numeric_types.h pointerTo.I pointerTo.h \
     pointerToArray.I pointerToArray.h \
     profileTimer.I profileTimer.h \
     pta_uchar.h referenceCount.I referenceCount.h \
     reversedNumericData.I reversedNumericData.h tokenBoard.I \
     tokenBoard.h trueClock.I trueClock.h typeHandle.I \
     typeHandle.h typedObject.I typedObject.h \
     typedReferenceCount.I typedReferenceCount.h typedef.h \
     vector_uchar.h
    
  #define INCLUDED_SOURCES  \
     buffer.cxx checksumHashGenerator.cxx clockObject.cxx \
     config_express.cxx datagram.cxx datagramGenerator.cxx \
     datagramInputFile.cxx datagramIterator.cxx \
     datagramOutputFile.cxx datagramSink.cxx error_utils.cxx \
     get_config_path.cxx \
     hashGeneratorBase.cxx hashVal.cxx indent.cxx \
     memoryInfo.cxx memoryUsage.cxx memoryUsagePointerCounts.cxx \
     memoryUsagePointers.cxx multifile.cxx namable.cxx \
     nativeNumericData.cxx profileTimer.cxx \
     pta_uchar.cxx referenceCount.cxx \
     reversedNumericData.cxx trueClock.cxx typeHandle.cxx \
     typedObject.cxx typedReferenceCount.cxx \
     vector_uchar.cxx

  #define IF_CRYPTO_SOURCES                         \
    crypto_utils.cxx crypto_utils.h \
    patchfile.I patchfile.cxx patchfile.h

  #define INSTALL_HEADERS                       \
    bigEndian.h buffer.I buffer.h checksumHashGenerator.I  \
    checksumHashGenerator.h circBuffer.I circBuffer.h clockObject.I \
    clockObject.h config_express.h datagram.I datagram.h \
    datagramGenerator.I datagramGenerator.h datagramInputFile.I \
    datagramInputFile.h datagramIterator.I datagramIterator.h \
    datagramOutputFile.I datagramOutputFile.h datagramSink.I \
    datagramSink.h error_utils.h get_config_path.h hashGeneratorBase.I \
    hashGeneratorBase.h hashVal.I hashVal.h indent.I indent.h \
    littleEndian.h memoryInfo.I memoryInfo.h memoryUsage.I \
    memoryUsage.h memoryUsagePointerCounts.I \
    memoryUsagePointerCounts.h memoryUsagePointers.I \
    memoryUsagePointers.h multifile.I multifile.h namable.I namable.h \
    nativeNumericData.I nativeNumericData.h numeric_types.h \
    patchfile.I patchfile.h pointerTo.I pointerTo.h \
    pointerToArray.I pointerToArray.h profileTimer.I \
    profileTimer.h pta_uchar.h referenceCount.I referenceCount.h \
    reversedNumericData.I reversedNumericData.h tokenBoard.I \
    tokenBoard.h trueClock.I trueClock.h typeHandle.I typeHandle.h \
    typedObject.I typedObject.h typedReferenceCount.I \
    typedReferenceCount.h typedef.h vector_uchar.h

  #define IGATESCAN all

#end lib_target
