
#if $[eq $[USE_COMPILER], MSVC]
  #define COMPILER cl
  #define LINKER link
  #define LIBBER lib
  #define COMMONFLAGS /Gi-
  #define OPTFLAGS /O2 /Ob1 /G6 /QIfist
  #define OPT1FLAGS /GZ 

  // Note: Zi cannot be used on multiproc builds with precomp hdrs, Z7 must be used instead
  #defer DEBUGPDBFLAGS /Zi /Fd"$[osfilename $[target:%.obj=%.pdb]]"  
  #defer DEBUGFLAGS /MDd $[BROWSEINFO_FLAG] $[DEBUGINFOFLAGS] $[DEBUGPDBFLAGS]
  #define RELEASEFLAGS /MD
  #define WARNING_LEVEL_FLAG /W3  
  
  #define CDEFINES_OPT4 UNKNOWN_ALLOCATOR

  // NODEFAULTLIB ensures static libs linked in will connect to the correct msvcrt, so no debug/release mixing occurs    
  #define LDFLAGS_OPT1 /NODEFAULTLIB:MSVCRT.LIB 
  #define LDFLAGS_OPT2 /NODEFAULTLIB:MSVCRT.LIB 
  #define LDFLAGS_OPT3 /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF
  #define LDFLAGS_OPT4 /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF $[LDFLAGS_OPT4]
  
  #define MAPINFOFLAGS /MAPINFO:EXPORTS /MAPINFO:FIXUPS /MAPINFO:LINES
  
  #if $[ENABLE_PROFILING]
      // note according to docs, this should force /PDB:none /DEBUGTYPE:cv, so no pdb file is generated for debug??  (doesnt seem to be true)
    #define PROFILE_FLAG /PROFILE 
  #else
    #define PROFILE_FLAG 
  #endif
 
  // Note: all Opts will link w/debug info now 
  #define LINKER_FLAGS /DEBUG /DEBUGTYPE:CV $[PROFILE_FLAG] /MAP $[MAPINFOFLAGS] /fixed:no /incremental:no /WARN:3 
  
// in case we have mixed intel/msvc build
  #define EXTRA_LIBPATH /ia32/lib
  #define EXTRA_INCPATH /ia32/include    

  #if $[or $[ne $[FORCE_INLINING],],$[>= $[OPTIMIZE],2]]
      #define EXTRA_CDEFS FORCE_INLINING $[EXTRA_CDEFS]
  #endif 
  
#elif $[eq $[USE_COMPILER], MSVC7]

  #define COMPILER cl
  #define LINKER link
  #define LIBBER lib
  
  #define DO_CROSSOBJ_OPT 1
  
  #if $[DO_CROSSOBJ_OPT]
     #define OPT4FLAGS /GL
     #define LDFLAGS_OPT4 /LTCG
     #define LIBBER $[LIBBER] /LTCG
  #endif 
  
  // remove 1-3 when allocator stuff is rewritten to build with VC7 STL
  #define CDEFINES_OPT1 UNKNOWN_ALLOCATOR
  #define CDEFINES_OPT2 UNKNOWN_ALLOCATOR
  #define CDEFINES_OPT3 UNKNOWN_ALLOCATOR
  #define CDEFINES_OPT4 UNKNOWN_ALLOCATOR      

  // NODEFAULTLIB ensures static libs linked in will connect to the correct msvcrt, so no debug/release mixing occurs  
  #define LDFLAGS_OPT1 /NODEFAULTLIB:MSVCRT.LIB 
  #define LDFLAGS_OPT2 /NODEFAULTLIB:MSVCRT.LIB 
  #define LDFLAGS_OPT3 /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF
  #define LDFLAGS_OPT4 /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF $[LDFLAGS_OPT4]

  #define COMMONFLAGS /DHAVE_DINKUM 
  
  #define OPTFLAGS /O2 /Ob2 /G6 /QIfist
  #define OPT1FLAGS /GZ /GS
  
//  #define WARNING_LEVEL_FLAG /Wall  //this is scary
  #define WARNING_LEVEL_FLAG /W3   // WL

  // Note: Zi cannot be used on multiproc builds with precomp hdrs, Z7 must be used instead
  #defer DEBUGPDBFLAGS /Zi /Fd"$[osfilename $[target:%.obj=%.pdb]]"  
  #defer DEBUGFLAGS /MDd $[BROWSEINFO_FLAG] $[DEBUGINFOFLAGS] $[DEBUGPDBFLAGS]
  #define RELEASEFLAGS /MD
  
  #define MAPINFOFLAGS /MAPINFO:EXPORTS /MAPINFO:LINES
  
  #if $[ENABLE_PROFILING]
    #define PROFILE_FLAG /FIXED:NO
  #else
    #define PROFILE_FLAG 
  #endif
  
  #if $[or $[ne $[FORCE_INLINING],],$[>= $[OPTIMIZE],2]]
      #define EXTRA_CDEFS FORCE_INLINING $[EXTRA_CDEFS]
  #endif 
 
  // Note: all Opts will link w/debug info now 
  #define LINKER_FLAGS /DEBUG $[PROFILE_FLAG] /MAP $[MAPINFOFLAGS] /fixed:no /incremental:no 
  
// in case we have mixed intel/msvc build
  #define EXTRA_LIBPATH /ia32/lib
  #define EXTRA_INCPATH /ia32/include  
    
#elif $[eq $[USE_COMPILER], INTEL]
  #define COMPILER icl
  #define LINKER xilink
  #define LIBBER xilib
  #define COMMONFLAGS /Gi- /Qwd985
  
  // Note: Zi cannot be used on multiproc builds with precomp hdrs, Z7 must be used instead
  #defer DEBUGPDBFLAGS /Zi /Qinline_debug_info /Fd"$[osfilename $[target:%.obj=%.pdb]]" 
  // Oy- needed for MS debugger
  #defer DEBUGFLAGS /Oy- /MDd $[BROWSEINFO_FLAG] $[DEBUGINFOFLAGS] $[DEBUGPDBFLAGS] 
  #define RELEASEFLAGS /MD
  #define WARNING_LEVEL_FLAG /W3    
  
  #if $[DO_CROSSOBJ_OPT]
     #define OPT4FLAGS /Qipo
     #define LDFLAGS_OPT4 /Qipo
  #endif   
  
  #define CDEFINES_OPT4 UNKNOWN_ALLOCATOR  

  // NODEFAULTLIB ensures static libs linked in will connect to the correct msvcrt, so no debug/release mixing occurs
  #define LDFLAGS_OPT1 /NODEFAULTLIB:MSVCRT.LIB 
  #define LDFLAGS_OPT2 /NODEFAULTLIB:MSVCRT.LIB 
  #define LDFLAGS_OPT3 /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF
  #define LDFLAGS_OPT4 /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF $[LDFLAGS_OPT4]
  
//  #define OPTFLAGS /O3 /G6 /Qvc6 /Qipo /QaxW /Qvec_report1 
  #define OPTFLAGS /O3 /G6 /Qvc6 /Qip /QIfist
  #define OPT1FLAGS /GZ /Od
  // We assume the Intel compiler installation dir is mounted as /ia32.
  #define EXTRA_LIBPATH /ia32/lib
  #define EXTRA_INCPATH /ia32/include  
  
  #if $[or $[ne $[FORCE_INLINING],],$[>= $[OPTIMIZE],2]]
      #define EXTRA_CDEFS FORCE_INLINING $[EXTRA_CDEFS]
  #endif   
  
  // Note: all Opts will link w/debug info now 
  #define LINKER_FLAGS /DEBUG /DEBUGTYPE:CV $[PROFILE_FLAG] /MAP $[MAPINFOFLAGS] /fixed:no /incremental:no /WARN:3
  
#elif $[eq $[USE_COMPILER], BOUNDS] // NuMega BoundsChecker
  #define COMPILER nmcl
  #define LINKER nmlink
  #define LIBBER lib
  #define COMMONFLAGS
  #define OPTFLAGS /O2 /Ogity /G6
  #define OPT1FLAGS /GZ   
  #defer DEBUGFLAGS /MDd /Zi $[BROWSEINFO_FLAG] /Fd"$[osfilename $[target:%.obj=%.pdb]]"
  #define RELEASEFLAGS /MD
  #define EXTRA_LIBPATH
  #define EXTRA_INCPATH
  #if $[BOUNDS_TRUETIME] // NuMega BoundsChecker TrueTime Profiler
    // This may look like a bad thing (to extend the compiler 
    // and linker with a switch), but I think it's the right 
    // thing to do in this case -- skyler.
    #define COMPILER $[COMPILER] /NMttOn
    #define LINKER $[LINKER] /NMttOn
  #endif 

#elif $[eq $[USE_COMPILER], TRUETIME] // NuMega TrueTime Profiler
  // This may look like a bad thing (to extend the compiler 
  // and linker with a switch), but I think it's the right 
  // thing to do in this case -- skyler.
  #define COMPILER nmcl /NMttOn
  #define LINKER nmlink /NMttOn
  #define LIBBER lib
  #define COMMONFLAGS
  #define OPTFLAGS /O2 /Ogity /G6
  #define OPT1FLAGS /GZ   
  #defer DEBUGFLAGS /MDd /Zi $[BROWSEINFO_FLAG] /Fd"$[osfilename $[target:%.obj=%.pdb]]"
  #define RELEASEFLAGS /MD
  #define EXTRA_LIBPATH
  #define EXTRA_INCPATH
#else
  #error Invalid value specified for USE_COMPILER.
#endif

#if $[CHECK_SYNTAX_ONLY]
#define END_CFLAGS $[END_CFLAGS] /Zs 
#endif 
  
#if $[GEN_ASSEMBLY]
#define END_CFLAGS $[END_CFLAGS] /FAs
#endif 

#if $[PREPROCESSOR_OUTPUT]
#define END_CFLAGS $[END_CFLAGS] /E 
#endif 
