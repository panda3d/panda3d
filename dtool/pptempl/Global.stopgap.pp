//
// Global.stopgap.pp
//
// This file is read in before any of the individual Sources.pp files
// are read.  It defines a few global variables to assist
// Template.stopgap.pp.
//

// This subroutine fills sources, alt_cflags, alt_ipath, alt_lpath,
// alt_libs, and alt_ld as appropriate for the current target.
#define sources
#define alt_cflags
#define alt_ipath
#define alt_lpath
#define alt_libs
#define alt_ld
#defsub get_sources
  #set sources $[get_sources]
  #set alt_cflags $[get_cflags]
  #set alt_ipath $[get_ipath]
  #set alt_lpath $[get_lpath]
  #set alt_libs $[get_libs]
  #set alt_ld $[get_ld]
#end get_sources

// This subroutine will set when_defer, when_no_defer, and when_either
// correctly to the set of libs we should link with for the current
// target.
#define when_defer
#define when_no_defer
#define when_either
#defsub get_libs
  // For the WHEN_DEFER case, we need to know the complete set of
  // metalibs that encapsulates each of our LOCAL_LIBS.  In the case
  // where a particular library is not part of a metalib, we include the
  // library itself.
  
  #set when_defer
  #foreach lib $[LOCAL_LIBS]
    // Only consider libraries that we're actually building.
    #if $[all_libs $[build_directory],$[lib]]
      #define modmeta $[module $[TARGET],$[lib]]
      #if $[ne $[modmeta],]
        #set when_defer $[when_defer] $[modmeta]
      #else
        #set when_defer $[when_defer] $[lib]
      #endif
    #endif
  #end lib
  #set when_defer $[unique $[when_defer]] $[patsubst %:m,%,$[filter %:m,$[OTHER_LIBS]]]
  
  // Also filter out the libraries we don't want from when_no_defer, although
  // we don't need to translate these to metalibs.
  #set when_no_defer
  #foreach lib $[COMPONENT_LIBS] $[LOCAL_LIBS]
    #if $[all_libs $[build_directory],$[lib]]
      #set when_no_defer $[when_no_defer] $[lib]
    #endif
  #end lib
  #set when_no_defer $[unique $[when_no_defer]] $[patsubst %:c,%,$[filter %:c,$[OTHER_LIBS]]]
  
  // Finally, get the set of libraries that we want in either case.  At
  // the moment, this is just the set of libraries in OTHER_LIBS that's
  // not flagged with either a :c or a :m.
  #set when_either $[filter-out %:m %:c,$[OTHER_LIBS]]
#end get_libs


// This subroutine converts depend_libs from a list of plain library names
// to a list of the form libname.so or libname.a, according to whether the
// named libraries are static or dynamic.
#defsub convert_depend_libs
  #define new_depend_libs
  #foreach lib $[depend_libs]
    // Make sure the library is something we're actually building.
    #if $[all_libs $[build_directory],$[lib]]
      #define libname $[static_libs lib$[TARGET].a,$[lib]] $[dynamic_libs lib$[TARGET].so,$[lib]]
      #if $[eq $[libname],]
  Warning: No such library $[lib], dependency of $[DIRNAME].
      #else
        #set new_depend_libs $[new_depend_libs] $[libname]
      #endif
    #endif
  #end lib
  #set depend_libs $[sort $[new_depend_libs]]
#end convert_depend_libs


// This subroutine determines the set of libraries our various targets
// depend on.  This is a complicated definition.  It is the union of
// all of our targets' dependencies, except:

// If a target is part of a metalib, it depends (a) directly on all of
// its normal library dependencies that are part of the same metalib,
// and (b) indirectly on all of the metalibs that every other library
// dependency is part of.  If a target is not part of a metalib, it is
// the same as case (b) above.
#define depend_libs
#defsub get_depend_libs
  #set depend_libs
  #forscopes lib_target noinst_lib_target
    #define metalib $[module $[TARGET],$[TARGET]]
    #if $[ne $[metalib],]
      // This library is included on a metalib.
      #foreach depend $[LOCAL_LIBS]
        #define depend_metalib $[module $[TARGET],$[depend]]
        #if $[eq $[depend_metalib],$[metalib]]
          // Here's a dependent library in the *same* metalib.
          #set depend_libs $[depend_libs] $[depend]
        #elif $[ne $[depend_metalib],]
          // This dependent library is in a *different* metalib.
          #set depend_libs $[depend_libs] $[depend_metalib]
        #else
          // This dependent library is not in any metalib.
          #set depend_libs $[depend_libs] $[depend]
        #endif
      #end depend
    #else
      // This library is *not* included on a metalib.
      #foreach depend $[LOCAL_LIBS]
        #define depend_metalib $[module $[TARGET],$[depend]]
        #if $[ne $[depend_metalib],]
          // This dependent library is on a metalib.
          #set depend_libs $[depend_libs] $[depend_metalib]
        #else
          // This dependent library is not in any metalib.
          #set depend_libs $[depend_libs] $[depend]
        #endif
      #end depend
    #endif
  #end lib_target noinst_lib_target
  
  // These will never be part of a metalib.
  #forscopes static_lib_target bin_target noinst_bin_target metalib_target
    #foreach depend $[LOCAL_LIBS]
      #define depend_metalib $[module $[TARGET],$[depend]]
      #if $[ne $[depend_metalib],]
        // This dependent library is on a metalib.
        #set depend_libs $[depend_libs] $[depend_metalib]
      #else
        // This dependent library is not in any metalib.
        #set depend_libs $[depend_libs] $[depend]
      #endif
    #end depend
  #end static_lib_target bin_target noinst_bin_target metalib_target

  // In case we're defining any metalibs, these depend directly on
  // their components as well.
  #set depend_libs $[depend_libs] $[COMPONENT_LIBS(metalib_target)]

  // Now correct all the libraries listed in depend_libs to refer to a
  // real library name.
  #define new_depend_libs
  #foreach lib $[sort $[depend_libs]]
    // Make sure the library is something we're actually building.
    #if $[all_libs $[build_directory],$[lib]]
      #define libname $[static_libs lib$[TARGET].a,$[lib]] $[dynamic_libs lib$[TARGET].so,$[lib]]
      #if $[eq $[libname],]
  Warning: No such library $[lib], dependency of $[DIRNAME].
      #else
        #set new_depend_libs $[new_depend_libs] $[libname]
      #endif
    #endif
  #end lib
  #set depend_libs $[sort $[new_depend_libs]]

#end get_depend_libs
