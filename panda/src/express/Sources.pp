#define LOCAL_LIBS pandabase
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET express
  
  #define SOURCES							\
    bigEndian.I bigEndian.cxx bigEndian.h buffer.I buffer.cxx buffer.h	\
    circBuffer.I circBuffer.h clockObject.I clockObject.cxx		\
    clockObject.h config_express.cxx config_express.h datagram.I	\
    datagram.cxx datagram.h datagramGenerator.I datagramGenerator.cxx	\
    datagramGenerator.h datagramIterator.cxx datagramIterator.h		\
    datagramSink.I datagramSink.cxx datagramSink.h			\
    get_config_path.cxx get_config_path.h				\
    indent.I indent.cxx indent.h littleEndian.I				\
    littleEndian.cxx littleEndian.h memoryUsage.I memoryUsage.cxx	\
    memoryUsage.h memoryUsagePointers.I memoryUsagePointers.cxx		\
    memoryUsagePointers.h multifile.I multifile.cxx multifile.h		\
    namable.I namable.cxx namable.h numeric_types.h patchfile.I		\
    patchfile.cxx patchfile.h pointerTo.I pointerTo.h referenceCount.I	\
    referenceCount.cxx referenceCount.h tokenBoard.I tokenBoard.h	\
    trueClock.I trueClock.cxx trueClock.h typeHandle.I typeHandle.cxx	\
    typeHandle.h typedReferenceCount.I typedReferenceCount.cxx		\
    typedReferenceCount.h typedef.h

  #define INSTALL_HEADERS						\
    bigEndian.I bigEndian.h buffer.I buffer.h circBuffer.I		\
    circBuffer.h clockObject.I clockObject.h config_express.h		\
    datagram.I datagram.h datagramIterator.h				\
    datagramSink.I datagramSink.h datagramGenerator.I			\
    datagramGenerator.h get_config_path.h				\
    indent.I indent.h littleEndian.I littleEndian.h			\
    memoryUsage.I memoryUsage.h memoryUsagePointers.I			\
    memoryUsagePointers.h multifile.I multifile.h numeric_types.h	\
    pointerTo.I pointerTo.h referenceCount.I referenceCount.h		\
    tokenBoard.h trueClock.I trueClock.h typeHandle.I typeHandle.h	\
    typedReferenceCount.I typedReferenceCount.h typedef.h		\
    namable.I namable.h tokenBoard.I patchfile.h patchfile.I

  #define IGATESCAN all

  #define IGATESCAN all

#end lib_target
