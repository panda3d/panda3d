#define LOCAL_LIBS cppParser pystub interrogatedb dconfig dtoolutil dtoolbase

#begin bin_target
  #define TARGET interrogate

  #define SOURCES \
    interrogate.cxx interrogate.h					\
    interrogateBuilder.cxx interrogateBuilder.h				\
    parameterRemap.I parameterRemap.cxx parameterRemap.h			\
    parameterRemapBasicStringRefToString.cxx				\
    parameterRemapBasicStringRefToString.h				\
    parameterRemapBasicStringToString.cxx				\
    parameterRemapBasicStringToString.h					\
    parameterRemapCharStarToString.cxx parameterRemapCharStarToString.h	\
    parameterRemapConcreteToPointer.cxx					\
    parameterRemapConcreteToPointer.h parameterRemapConstToNonConst.cxx	\
    parameterRemapConstToNonConst.h parameterRemapEnumToInt.cxx		\
    parameterRemapEnumToInt.h parameterRemapPTToPointer.cxx		\
    parameterRemapPTToPointer.h parameterRemapReferenceToConcrete.cxx	\
    parameterRemapReferenceToConcrete.h					\
    parameterRemapReferenceToPointer.cxx					\
    parameterRemapReferenceToPointer.h parameterRemapThis.cxx		\
    parameterRemapThis.h parameterRemapToString.cxx			\
    parameterRemapToString.h parameterRemapUnchanged.cxx			\
    parameterRemapUnchanged.h typeManager.cxx				\
    typeManager.h wrapperBuilder.cxx wrapperBuilder.h			\
    wrapperBuilderC.cxx wrapperBuilderC.h wrapperBuilderPython.cxx	\
    wrapperBuilderPython.h

#end bin_target

#begin bin_target
  #define TARGET parse_file
  #define SOURCES parse_file.cxx
#end bin_target

#begin bin_target
  #define TARGET interrogate_module
  #define SOURCES interrogate_module.cxx
#end bin_target
