#define LOCAL_LIBS cppParser pystub interrogatedb dconfig dtoolutil dtoolbase

#begin bin_target
  #define TARGET interrogate
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx   

  #define SOURCES \
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
     wrapperBuilderPython.h      

 #define INCLUDED_SOURCES  \
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
     wrapperBuilderPython.cxx  

#end bin_target

#begin bin_target
  #define TARGET parse_file
  #define SOURCES parse_file.cxx
#end bin_target

#begin bin_target
  #define TARGET interrogate_module
  #define SOURCES interrogate_module.cxx
#end bin_target
