#define LOCAL_LIBS pandabase
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET express
  #define USE_NSPR yes
  #define USE_CRYPTO yes
  #define USE_NET yes

  #define SOURCES							\
    bigEndian.h buffer.I buffer.cxx buffer.h	\
    circBuffer.I circBuffer.h clockObject.I clockObject.cxx		\
    clockObject.h config_express.cxx config_express.h datagram.I	\
    datagram.cxx datagram.h datagramGenerator.I datagramGenerator.cxx	\
    datagramGenerator.h datagramInputFile.I datagramInputFile.h		\
    datagramInputFile.cxx datagramIterator.I \
    datagramIterator.cxx datagramIterator.h	\
    datagramOutputFile.I datagramOutputFile.h datagramOutputFile.cxx	\
    datagramSink.I datagramSink.cxx datagramSink.h			\
    get_config_path.cxx get_config_path.h				\
    hashVal.I hashVal.cxx hashVal.h \
    indent.I indent.cxx indent.h \
    littleEndian.h memoryUsage.I memoryUsage.cxx	\
    memoryUsage.h memoryUsagePointers.I memoryUsagePointers.cxx		\
    memoryUsagePointers.h multifile.I multifile.cxx multifile.h \
    namable.I namable.cxx namable.h \
    nativeNumericData.I nativeNumericData.cxx nativeNumericData.h \
    numeric_types.h 			\
    pointerTo.I pointerTo.h referenceCount.I	\
    referenceCount.cxx referenceCount.h \
    reversedNumericData.I reversedNumericData.cxx reversedNumericData.h \
    tokenBoard.I tokenBoard.h	\
    trueClock.I trueClock.cxx trueClock.h typeHandle.I typeHandle.cxx	\
    typeHandle.h \
    typedObject.I typedObject.cxx typedObject.h \
    typedReferenceCount.I typedReferenceCount.cxx		\
    typedReferenceCount.h typedef.h error_utils.cxx error_utils.h

  #define IF_CRYPTO_SOURCES 						\
    crypto_utils.cxx crypto_utils.h \
    patchfile.I patchfile.cxx patchfile.h

  #define INSTALL_HEADERS						\
    bigEndian.h buffer.I buffer.h circBuffer.I		\
    circBuffer.h clockObject.I clockObject.h config_express.h		\
    datagram.I datagram.h datagramInputFile.I datagramInputFile.h	\
    datagramIterator.I datagramIterator.h \
    datagramOutputFile.I datagramOutputFile.h	\
    datagramSink.I datagramSink.h datagramGenerator.I			\
    datagramGenerator.h get_config_path.h				\
    hashVal.I hashVal.h \
    indent.I indent.h littleEndian.h			\
    memoryUsage.I memoryUsage.h memoryUsagePointers.I			\
    memoryUsagePointers.h multifile.I multifile.h \
    nativeNumericData.I nativeNumericData.h \
    numeric_types.h	\
    pointerTo.I pointerTo.h referenceCount.I referenceCount.h		\
    reversedNumericData.I reversedNumericData.h \
    tokenBoard.h trueClock.I trueClock.h typeHandle.I typeHandle.h	\
    typedObject.h typedObject.I \
    typedReferenceCount.I typedReferenceCount.h typedef.h		\
    namable.I namable.h tokenBoard.I patchfile.h patchfile.I		\
    error_utils.h

  #define IGATESCAN all

#end lib_target
