#define DIRECTORY_IF_INTERROGATE yes

#define LOCAL_LIBS cppParser pystub interrogatedb dconfig dtoolutil dtoolbase

#begin bin_target
  #define TARGET interrogate

// While I'm working on this, I'll temporarily take out the composite
// build.
//  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx   
  #define SOURCES \
     classBuilder.h classBuilderPythonObj.h \
     functionRemap.h \
     functionWriter.h \
     functionWriterPtrFromPython.h functionWriterPtrToPython.h \
     functionWriters.h \
     interfaceMaker.h \
     interfaceMakerC.h \
     interfaceMakerPython.h interfaceMakerPythonObj.h \
     interfaceMakerPythonSimple.h \
     interrogate.h interrogateBuilder.h parameterRemap.I  \
     parameterRemap.h parameterRemapBasicStringRefToString.h  \
     parameterRemapBasicStringToString.h  \
     parameterRemapCharStarToString.h  \
     parameterRemapConcreteToPointer.h  \
     parameterRemapConstToNonConst.h parameterRemapEnumToInt.h  \
     parameterRemapPTToPointer.h  \
     parameterRemapReferenceToConcrete.h  \
     parameterRemapReferenceToPointer.h parameterRemapThis.h  \
     parameterRemapToString.h parameterRemapUnchanged.h  \
     typeManager.h wrapperBuilder.h wrapperBuilderC.h  \
     wrapperBuilderPython.h wrapperBuilderPythonObj.h

// #define INCLUDED_SOURCES  \
 #define SOURCES $[SOURCES] \
     classBuilder.cxx classBuilderPythonObj.cxx \
     functionRemap.cxx \
     functionWriter.cxx \
     functionWriterPtrFromPython.cxx functionWriterPtrToPython.cxx \
     functionWriters.cxx \
     interfaceMaker.cxx \
     interfaceMakerC.cxx \
     interfaceMakerPython.cxx interfaceMakerPythonObj.cxx \
     interfaceMakerPythonSimple.cxx \
     interrogate.cxx interrogateBuilder.cxx parameterRemap.cxx  \
     parameterRemapBasicStringRefToString.cxx  \
     parameterRemapBasicStringToString.cxx  \
     parameterRemapCharStarToString.cxx  \
     parameterRemapConcreteToPointer.cxx  \
     parameterRemapConstToNonConst.cxx  \
     parameterRemapEnumToInt.cxx parameterRemapPTToPointer.cxx  \
     parameterRemapReferenceToConcrete.cxx  \
     parameterRemapReferenceToPointer.cxx parameterRemapThis.cxx  \
     parameterRemapToString.cxx parameterRemapUnchanged.cxx  \
     typeManager.cxx wrapperBuilder.cxx wrapperBuilderC.cxx  \
     wrapperBuilderPython.cxx wrapperBuilderPythonObj.cxx

#end bin_target

#begin bin_target
  #define TARGET parse_file
  #define SOURCES parse_file.cxx
#end bin_target

#begin bin_target
  #define TARGET interrogate_module
  #define SOURCES interrogate_module.cxx
#end bin_target
