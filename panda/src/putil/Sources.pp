#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m
#define LOCAL_LIBS express pandabase

#begin lib_target
  #define TARGET putil

  #define SOURCES \
    bam.h bamReader.I bamReader.N bamReader.cxx \
    bamReader.h bamReaderParam.I \
    bamReaderParam.cxx bamReaderParam.h bamWriter.I bamWriter.cxx \
    bamWriter.h bitMask.I bitMask.cxx bitMask.h buttonEvent.I \
    buttonEvent.cxx buttonEvent.h buttonHandle.I buttonHandle.cxx \
    buttonHandle.h buttonRegistry.I buttonRegistry.cxx buttonRegistry.h \
    config_util.N config_util.cxx config_util.h configurable.cxx \
    configurable.h factoryBase.I factoryBase.cxx factoryBase.h \
    factoryParam.I factoryParam.cxx factoryParam.h factoryParams.I \
    factoryParams.cxx factoryParams.h globPattern.I globPattern.cxx \
    globPattern.h globalPointerRegistry.I globalPointerRegistry.cxx \
    globalPointerRegistry.h ioPtaDatagramFloat.cxx ioPtaDatagramFloat.h \
    ioPtaDatagramInt.cxx ioPtaDatagramInt.h ioPtaDatagramShort.cxx \
    ioPtaDatagramShort.h keyboardButton.cxx keyboardButton.h \
    lineStream.I lineStream.cxx lineStream.h lineStreamBuf.I \
    lineStreamBuf.cxx lineStreamBuf.h modifierButtons.I \
    modifierButtons.cxx modifierButtons.h \
    mouseButton.cxx mouseButton.h mouseData.cxx mouseData.h \
    nameUniquifier.I nameUniquifier.cxx nameUniquifier.h pta_double.cxx \
    pta_double.h pta_float.cxx pta_float.h pta_int.cxx pta_int.h \
    pta_uchar.cxx pta_uchar.h pta_ushort.cxx pta_ushort.h \
    string_utils.I string_utils.N string_utils.cxx string_utils.h \
    timedCycle.I timedCycle.cxx timedCycle.h typedWriteable.I \
    typedWriteable.cxx typedWriteable.h typedWriteableReferenceCount.I \
    typedWriteableReferenceCount.cxx typedWriteableReferenceCount.h \
    updateSeq.I updateSeq.cxx updateSeq.h vector_double.cxx \
    vector_double.h vector_float.cxx vector_float.h \
    vector_typedWriteable.cxx vector_typedWriteable.h vector_uchar.cxx \
    vector_uchar.h vector_ushort.cxx vector_ushort.h \
    vector_writeable.cxx vector_writeable.h writeable.I writeable.cxx \
    writeable.h writeableConfigurable.cxx writeableConfigurable.h \
    writeableParam.I writeableParam.cxx writeableParam.h

  #define INSTALL_HEADERS \
    bam.h bamReader.I bamReader.h bamReaderParam.I bamReaderParam.h \
    bamWriter.I bamWriter.h bitMask.I bitMask.h buttonEvent.I \
    buttonEvent.h buttonHandle.I buttonHandle.h buttonRegistry.I \
    buttonRegistry.h config_util.h configurable.h factory.I factory.h \
    factoryBase.I factoryBase.h factoryParam.I factoryParam.h \
    factoryParams.I factoryParams.h globPattern.I globPattern.h \
    globalPointerRegistry.I globalPointerRegistry.h indirectCompareTo.I \
    indirectCompareTo.h ioPtaDatagramFloat.h ioPtaDatagramInt.h \
    ioPtaDatagramShort.h iterator_types.h keyboardButton.h lineStream.I \
    lineStream.h lineStreamBuf.I lineStreamBuf.h modifierButtons.I \
    modifierButtons.h mouseButton.h mouseData.h nameUniquifier.I \
    nameUniquifier.h pointerToArray.I pointerToArray.h pta_double.h \
    pta_float.h pta_int.h pta_uchar.h pta_ushort.h string_utils.I \
    string_utils.h timedCycle.I timedCycle.h typedWriteable.I \
    typedWriteable.h typedWriteableReferenceCount.I \
    typedWriteableReferenceCount.h updateSeq.I updateSeq.h \
    vector_double.h vector_float.h vector_typedWriteable.h \
    vector_uchar.h vector_ushort.h vector_writeable.h writeable.I \
    writeable.h writeableConfigurable.h writeableParam.I \
    writeableParam.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_bamRead
  #define LOCAL_LIBS \
    putil graph

  #define SOURCES \
    test_bam.cxx test_bam.h test_bamRead.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_bamWrite
  #define LOCAL_LIBS \
    putil graph

  #define SOURCES \
    test_bam.cxx test_bam.h test_bamWrite.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_filename

  #define SOURCES \
    test_filename.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_glob

  #define SOURCES \
    test_glob.cxx

  #define LOCAL_LIBS $[LOCAL_LIBS] putil

#end test_bin_target

#begin test_bin_target
  #define TARGET test_linestream

  #define SOURCES \
    test_linestream.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_types
  #define LOCAL_LIBS $[LOCAL_LIBS] putil

  #define SOURCES \
    test_types.cxx

#end test_bin_target

