#print Generating for $[USE_COMPILER]


//////////////////////////////
// MICROSOFT VISUAL C++ 6.0 //
//////////////////////////////
#if $[eq $[USE_COMPILER], MSVC]
  #define COMPILER cl
  #define LINKER link
  #define LIBBER lib
  #define COMMONFLAGS /Gi-

  // use "unsafe" QIfist flt->int rounding only if FAST_FLT_TO_INT is defined
  #define OPTFLAGS /O2 /Ob1 /G7 $[if $[ne $[FAST_FLT_TO_INT],], /QIfist,]
  #define OPT1FLAGS /GZ

  // Note: Zi cannot be used on multiproc builds with precomp hdrs, Z7 must be used instead
  #defer DEBUGPDBFLAGS /Zi /Fd"$[osfilename $[patsubst %.obj,%.pdb,$[target]]]"
  #defer DEBUGFLAGS /MDd $[BROWSEINFO_FLAG] $[DEBUGINFOFLAGS] $[DEBUGPDBFLAGS]
  #defer RELEASEFLAGS $[if $[ne $[LINK_FORCE_STATIC_RELEASE_C_RUNTIME],],/MT, /MD]
  #define WARNING_LEVEL_FLAG /Wall

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
  #define LINKER_FLAGS /DEBUG /DEBUGTYPE:CV $[PROFILE_FLAG] /MAP $[MAPINFOFLAGS] /fixed:no /incremental:no /stack:4194304 /WARN:3

  // Added to avoid old iostream reference problems
  #define LINKER_FLAGS $[LINKER_FLAGS] /NODEFAULTLIB:LIBCI.LIB

// for mixed intel/msvc build, add these
//  #define EXTRA_LIBPATH /ia32/lib
//  #define EXTRA_INCPATH /ia32/include

  #if $[or $[ne $[FORCE_INLINING],],$[>= $[OPTIMIZE],2]]
      #define EXTRA_CDEFS FORCE_INLINING $[EXTRA_CDEFS]
  #endif

  // ensure pdbs are copied to install dir
  #define build_pdbs yes


////////////////////////////////////////////////////////////////////
// MICROSOFT VISUAL C++ 7.0, 7.1, & 8.0 (.NET, .NET 2003, & 2005) //
////////////////////////////////////////////////////////////////////
#elif $[or $[or $[eq $[USE_COMPILER], MSVC7], $[eq $[USE_COMPILER], MSVC7_1]], $[eq $[USE_COMPILER],MSVC8]]
  #define COMPILER cl
  #define LINKER link
  #define LIBBER lib

  #if $[eq $[NO_CROSSOBJ_OPT],]
     #define DO_CROSSOBJ_OPT 1
  #endif

  #if $[DO_CROSSOBJ_OPT]
     #define OPT4FLAGS /GL
     #define LDFLAGS_OPT4 /LTCG
     #if $[>= $[OPTIMIZE],4]
        #define LIBBER $[LIBBER] /LTCG
     #endif
  #endif

  #define CDEFINES_OPT1
  #define CDEFINES_OPT2
  #define CDEFINES_OPT3
  #define CDEFINES_OPT4

  // NODEFAULTLIB ensures static libs linked in will connect to the correct msvcrt, so no debug/release mixing occurs

  #define LDFLAGS_OPT1 /NODEFAULTLIB:MSVCRT.LIB
  #define LDFLAGS_OPT2 /NODEFAULTLIB:MSVCRT.LIB
  #define LDFLAGS_OPT3 /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF
  #define LDFLAGS_OPT4 /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF $[LDFLAGS_OPT4]

  #define COMMONFLAGS /DHAVE_DINKUM /Zc:forScope

  // use "unsafe" QIfist flt->int rounding only if FAST_FLT_TO_INT is defined
  #if $[or $[eq $[USE_COMPILER], MSVC7], $[eq $[USE_COMPILER], MSVC7_1]]
    #define REGULAR_OPTFLAGS /O2 /Ob2 /G7 $[if $[ne $[FAST_FLT_TO_INT],], /QIfist,]
    #defer OPTFLAGS $[if $[OPT_MINSIZE],/Ox /Og /Ob1 /Oi /Os /Oy /GL /G7,$[REGULAR_OPTFLAGS]]
  #else
    #define REGULAR_OPTFLAGS /O2 /Ob2 $[if $[ne $[FAST_FLT_TO_INT],], /QIfist,]
    #defer OPTFLAGS $[if $[OPT_MINSIZE],/Ox /Og /Ob1 /Oi /Os /Oy /GL,$[REGULAR_OPTFLAGS]]
  #endif

  //  #define OPT1FLAGS /RTCsu /GS  removing /RTCu because it crashes in dxgsg with internal compiler bug
  #define OPT1FLAGS /RTCs /GS

  #define WARNING_LEVEL_FLAG /W3   // WL
  //#define WARNING_LEVEL_FLAG /Wall
  //#define WARNING_LEVEL_FLAG /W4 /WX

  // Note: Zi cannot be used on multiproc builds with precomp hdrs, Z7 must be used instead
  #defer DEBUGPDBFLAGS /Zi /Fd"$[osfilename $[patsubst %.obj,%.pdb, $[target]]]"

  // if LINK_FORCE_STATIC_C_RUNTIME is defined, it always links with static c runtime (release version
  // for both Opt1 and Opt4!) instead of the msvcrt dlls

  #defer DEBUGFLAGS $[if $[ne $[LINK_FORCE_STATIC_RELEASE_C_RUNTIME],],/MTd, /MDd] $[BROWSEINFO_FLAG] $[DEBUGINFOFLAGS] $[DEBUGPDBFLAGS]
  #defer RELEASEFLAGS $[if $[ne $[LINK_FORCE_STATIC_RELEASE_C_RUNTIME],],/MT, /MD]

  #if $[or $[eq $[USE_COMPILER], MSVC7], $[eq $[USE_COMPILER], MSVC7_1]]
    #define MAPINFOFLAGS /MAPINFO:EXPORTS /MAPINFO:LINES
  #else
    #define MAPINFOFLAGS /MAPINFO:EXPORTS
  #endif

  #if $[ENABLE_PROFILING]
    #define PROFILE_FLAG /FIXED:NO
  #else
    #define PROFILE_FLAG
  #endif

  #if $[or $[ne $[FORCE_INLINING],],$[>= $[OPTIMIZE],2]]
      #defer EXTRA_CDEFS $[EXTRA_CDEFS] $[if $[OPT_MINSIZE],,FORCE_INLINING]
  #endif

  // Note: all Opts will link w/debug info now
  #define LINKER_FLAGS /DEBUG $[PROFILE_FLAG] /MAP $[MAPINFOFLAGS] /fixed:no /incremental:no /stack:4194304

  // Added to avoid old iostream reference problems
  #define LINKER_FLAGS $[LINKER_FLAGS] /NODEFAULTLIB:LIBCI.LIB
  #if $[eq $[USE_COMPILER],MSVC8]
    #define LINKER_FLAGS $[LINKER_FLAGS] /NOD:MFC80.LIB /NOD:libcmtd /NOD:libc
  #endif
//  #define LINKER_FLAGS $[LINKER_FLAGS] /NODEFAULTLIB:LIBCI.LIB /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:msvcprt.lib

// in case we have mixed intel/msvc build
//  #define EXTRA_LIBPATH /ia32/lib
//  #define EXTRA_INCPATH /ia32/include

  // ensure pdbs are copied to install dir
  #define build_pdbs yes


/////////////////////////////////////
// MICROSOFT VISUAL C++ 9.0 (2008) //
/////////////////////////////////////
#elif $[eq $[USE_COMPILER], MSVC9]
  #define COMPILER cl
  #define LINKER link
  #define LIBBER lib
  #define MT_BIN mt

  #if $[eq $[NO_CROSSOBJ_OPT],]
     #define DO_CROSSOBJ_OPT 1
  #endif

  #if $[DO_CROSSOBJ_OPT]
     #define OPT4FLAGS /GL
     #define LDFLAGS_OPT4 /LTCG
     #if $[>= $[OPTIMIZE],4]
        #define LIBBER $[LIBBER] /LTCG
     #endif
  #endif

  #define CDEFINES_OPT1
  #define CDEFINES_OPT2
  #define CDEFINES_OPT3
  #define CDEFINES_OPT4

  // NODEFAULTLIB ensures static libs linked in will connect to the correct msvcrt, so no debug/release mixing occurs

  #define LDFLAGS_OPT1 /NODEFAULTLIB:MSVCRT.LIB
  #define LDFLAGS_OPT2 /NODEFAULTLIB:MSVCRT.LIB
  #define LDFLAGS_OPT3 /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF
  #define LDFLAGS_OPT4 /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF $[LDFLAGS_OPT4]

  #define COMMONFLAGS /DHAVE_DINKUM /Zc:forScope /bigobj

  // use "unsafe" QIfist flt->int rounding only if FAST_FLT_TO_INT is defined
  #define REGULAR_OPTFLAGS /O2 /Ob2 $[if $[ne $[FAST_FLT_TO_INT],], /QIfist,]

  #defer OPTFLAGS $[if $[OPT_MINSIZE],/Ox /Og /Ob1 /Oi /Os /Oy /GL,$[REGULAR_OPTFLAGS]]

  //  #define OPT1FLAGS /RTCsu /GS  removing /RTCu because it crashes in dxgsg with internal compiler bug
  #define OPT1FLAGS /RTCs /GS

  #define WARNING_LEVEL_FLAG /W3   // WL

  // Note: Zi cannot be used on multiproc builds with precomp hdrs, Z7 must be used instead
  #defer DEBUGPDBFLAGS /Zi /Fd"$[osfilename $[patsubst %.obj,%.pdb, $[target]]]"

  // if LINK_FORCE_STATIC_C_RUNTIME is defined, it always links with static c runtime (release version
  // for both Opt1 and Opt4!) instead of the msvcrt dlls

  #defer DEBUGFLAGS $[if $[ne $[LINK_FORCE_STATIC_RELEASE_C_RUNTIME],],/MTd, /MDd] $[BROWSEINFO_FLAG] $[DEBUGINFOFLAGS] $[DEBUGPDBFLAGS]
  #defer RELEASEFLAGS $[if $[ne $[LINK_FORCE_STATIC_RELEASE_C_RUNTIME],],/MT, /MD]

  #define MAPINFOFLAGS /MAPINFO:EXPORTS

  #if $[ENABLE_PROFILING]
    #define PROFILE_FLAG /FIXED:NO
  #else
    #define PROFILE_FLAG
  #endif

  #if $[or $[ne $[FORCE_INLINING],],$[>= $[OPTIMIZE],2]]
      #defer EXTRA_CDEFS $[EXTRA_CDEFS] $[if $[OPT_MINSIZE],,FORCE_INLINING]
  #endif

  // Note: all Opts will link w/debug info now
  #define LINKER_FLAGS /DEBUG $[PROFILE_FLAG] /MAP $[MAPINFOFLAGS] /fixed:no /incremental:no /stack:4194304

  // Added to avoid old iostream reference problems
  #define LINKER_FLAGS $[LINKER_FLAGS] /NODEFAULTLIB:LIBCI.LIB
  // Added to make pandatool function in VS 9
  #define LINKER_FLAGS $[LINKER_FLAGS] /NOD:MFC80.LIB /NOD:libcmtd /NOD:libc
  // Added to generate manifest files even when no dependencies exist
  #define LINKER_FLAGS $[LINKER_FLAGS] /MANIFEST

  // ensure pdbs are copied to install dir
  #define build_pdbs yes



///////////////////////////////////////////////////
// MICROSOFT VISUAL C++ 9.0 (2008) 64-bit output //
///////////////////////////////////////////////////
#elif $[eq $[USE_COMPILER], MSVC9x64]
  #define COMPILER cl
  #define LINKER link
  #define LIBBER lib
  #define MT_BIN mt

  #if $[eq $[NO_CROSSOBJ_OPT],]
     #define DO_CROSSOBJ_OPT 1
  #endif

  #if $[DO_CROSSOBJ_OPT]
     #define OPT4FLAGS /GL
     #define LDFLAGS_OPT4 /LTCG
     #if $[>= $[OPTIMIZE],4]
        #define LIBBER $[LIBBER] /LTCG
     #endif
  #endif

  #define CDEFINES_OPT1
  #define CDEFINES_OPT2
  #define CDEFINES_OPT3
  #define CDEFINES_OPT4

  // NODEFAULTLIB ensures static libs linked in will connect to the correct msvcrt, so no debug/release mixing occurs

  #define LDFLAGS_OPT1 /NODEFAULTLIB:MSVCRT.LIB
  #define LDFLAGS_OPT2 /NODEFAULTLIB:MSVCRT.LIB
  #define LDFLAGS_OPT3 /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF
  #define LDFLAGS_OPT4 /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF $[LDFLAGS_OPT4]

  #define COMMONFLAGS /DHAVE_DINKUM /Zc:forScope /bigobj

  // use "unsafe" QIfist flt->int rounding only if FAST_FLT_TO_INT is defined
  #define REGULAR_OPTFLAGS /O2 /Ob2 $[if $[ne $[FAST_FLT_TO_INT],], /QIfist,]

  #defer OPTFLAGS $[if $[OPT_MINSIZE],/Ox /Og /Ob1 /Oi /Os /Oy /GL,$[REGULAR_OPTFLAGS]]

  //  #define OPT1FLAGS /RTCsu /GS  removing /RTCu because it crashes in dxgsg with internal compiler bug
  #define OPT1FLAGS /RTCs /GS

  #define WARNING_LEVEL_FLAG /W3   // WL

  // Note: Zi cannot be used on multiproc builds with precomp hdrs, Z7 must be used instead
  #defer DEBUGPDBFLAGS /Zi /Fd"$[osfilename $[patsubst %.obj,%.pdb, $[target]]]"

  // if LINK_FORCE_STATIC_C_RUNTIME is defined, it always links with static c runtime (release version
  // for both Opt1 and Opt4!) instead of the msvcrt dlls

  #defer DEBUGFLAGS $[if $[ne $[LINK_FORCE_STATIC_RELEASE_C_RUNTIME],],/MTd, /MDd] $[BROWSEINFO_FLAG] $[DEBUGINFOFLAGS] $[DEBUGPDBFLAGS]
  #defer RELEASEFLAGS $[if $[ne $[LINK_FORCE_STATIC_RELEASE_C_RUNTIME],],/MT, /MD]

  #define MAPINFOFLAGS /MAPINFO:EXPORTS

  #if $[ENABLE_PROFILING]
    #define PROFILE_FLAG /FIXED:NO
  #else
    #define PROFILE_FLAG
  #endif

  #if $[or $[ne $[FORCE_INLINING],],$[>= $[OPTIMIZE],2]]
      #defer EXTRA_CDEFS $[EXTRA_CDEFS] $[if $[OPT_MINSIZE],,FORCE_INLINING]
  #endif

  // Note: all Opts will link w/debug info now
  #define LINKER_FLAGS /DEBUG $[PROFILE_FLAG] $[if $[eq $[USE_COMPILER],MSVC9x64], /MACHINE:X64,] /MAP $[MAPINFOFLAGS] /fixed:no /incremental:no /stack:4194304

  // Added to avoid old iostream reference problems
  #define LINKER_FLAGS $[LINKER_FLAGS] /NODEFAULTLIB:LIBCI.LIB
  // Added to make pandatool function in VS 9
  #define LINKER_FLAGS $[LINKER_FLAGS] /NOD:MFC80.LIB /NOD:libcmtd /NOD:libc
  // Added to generate manifest files even when no dependencies exist
  #define LINKER_FLAGS $[LINKER_FLAGS] /MANIFEST


  // ensure pdbs are copied to install dir
  #define build_pdbs yes


////////////////////
// INTEL COMPILER //
////////////////////
#elif $[eq $[USE_COMPILER], INTEL]
  #define COMPILER icl
  #define LINKER xilink
  #define LIBBER xilib
  #define COMMONFLAGS /DHAVE_DINKUM /Gi- /Qwd985 /Qvc7 /G6

  // Note: Zi cannot be used on multiproc builds with precomp hdrs, Z7 must be used instead
  #defer DEBUGPDBFLAGS /Zi /Qinline_debug_info /Fd"$[osfilename $[patsubst %.obj,%.pdb,$[target]]]"
  // Oy- needed for MS debugger
  #defer DEBUGFLAGS /Oy- /MDd $[BROWSEINFO_FLAG] $[DEBUGINFOFLAGS] $[DEBUGPDBFLAGS]
  #defer RELEASEFLAGS $[if $[ne $[LINK_FORCE_STATIC_RELEASE_C_RUNTIME],],/MT, /MD]
  #define WARNING_LEVEL_FLAG /W3

  #if $[DO_CROSSOBJ_OPT]
     #define OPT4FLAGS /Qipo
     #define LDFLAGS_OPT4 /Qipo
  #endif

  // NODEFAULTLIB ensures static libs linked in will connect to the correct msvcrt, so no debug/release mixing occurs
  #define LDFLAGS_OPT1 /NODEFAULTLIB:MSVCRT.LIB
  #define LDFLAGS_OPT2 /NODEFAULTLIB:MSVCRT.LIB
  #define LDFLAGS_OPT3 /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF
  #define LDFLAGS_OPT4 /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF $[LDFLAGS_OPT4]

//  #define OPTFLAGS /O3 /Qipo /QaxW /Qvec_report1
  #define OPTFLAGS /O3 /Qip

  // use "unsafe" QIfist flt->int rounding only if FAST_FLT_TO_INT is defined
  #define OPTFLAGS $[OPTFLAGS] $[if $[ne $[FAST_FLT_TO_INT],], /QIfist,]

  #define OPT1FLAGS /GZ /Od
  // We assume the Intel compiler installation dir is mounted as /ia32.
  #define EXTRA_LIBPATH /ia32/lib
  #define EXTRA_INCPATH /ia32/include

  #if $[or $[ne $[FORCE_INLINING],],$[>= $[OPTIMIZE],2]]
      #define EXTRA_CDEFS FORCE_INLINING $[EXTRA_CDEFS]
  #endif

  // Note: all Opts will link w/debug info now
  #define LINKER_FLAGS /DEBUG /DEBUGTYPE:CV $[PROFILE_FLAG] /MAP $[MAPINFOFLAGS] /fixed:no /incremental:no /stack:4194304

  // Added to avoid old iostream reference problems
  #define LINKER_FLAGS $[LINKER_FLAGS] /NODEFAULTLIB:LIBCI.LIB

  // ensure pdbs are copied to install dir
  #define build_pdbs yes

#elif $[eq $[USE_COMPILER], BOUNDS] // NuMega BoundsChecker
  #define COMPILER nmcl
  #define LINKER nmlink
  #define LIBBER lib
  #define COMMONFLAGS
  #define OPTFLAGS /O2 /Ogity /G6
  #define OPT1FLAGS /GZ
  #defer DEBUGPDBFLAGS /MDd /Zi $[BROWSEINFO_FLAG] /Fd"$[osfilename $[patsubst %.obj,%.pdb,$[target]]]"
  #defer RELEASEFLAGS $[if $[ne $[LINK_FORCE_STATIC_RELEASE_C_RUNTIME],],/MT, /MD]
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
  #defer DEBUGPDBFLAGS /MDd /Zi $[BROWSEINFO_FLAG] /Fd"$[osfilename $[patsubst %.obj,%.pdb,$[target]]]"
  #defer RELEASEFLAGS $[if $[ne $[LINK_FORCE_STATIC_RELEASE_C_RUNTIME],],/MT, /MD]
  #define EXTRA_LIBPATH
  #define EXTRA_INCPATH
#else
  #error Invalid value specified for USE_COMPILER.
#endif

#if $[CHECK_SYNTAX_ONLY]
#define END_CFLAGS $[END_CFLAGS] /Zs
#endif

#if $[GEN_ASSEMBLY]
// Note:  Opt4 /GL will cause /FAs to not generate .asm!   Must remove /GL for /FAs to work!
#define END_CFLAGS $[END_CFLAGS] /FAs
#endif

#if $[PREPROCESSOR_OUTPUT]
#define END_CFLAGS $[END_CFLAGS] /E
#endif

#defer tau_opts $[decygwin %,-I"%",$[EXTRA_INCPATH] $[ipath] $[WIN32_PLATFORMSDK_INCPATH] $[tau_ipath]] $[building_var:%=-D%]
#if $[eq $[USE_COMPILER], MSVC9x64]
  #defer TAU_MAKE_IL $[PDT_ROOT]/Windows/bin/edgcpfe -o $[il_source] $[tau_opts] $[cdefines:%=-D%] $[C++FLAGS] -DWIN64=1 $[TAU_INSTRUMENTOR_FLAGS] $[source]
#else
  #defer TAU_MAKE_IL $[PDT_ROOT]/Windows/bin/edgcpfe -o $[il_source] $[tau_opts] $[cdefines:%=-D%] $[C++FLAGS] -DWIN32=1 $[TAU_INSTRUMENTOR_FLAGS] $[source]
#endif
#defer TAU_MAKE_PDB $[PDT_ROOT]/Windows/bin/taucpdisp $[il_source] > $[pdb_source]
#defer TAU_MAKE_INST $[TAU_ROOT]/bin/tau_instrumentor $[pdb_source] $[source] -o $[inst_source]
