//
// Depends.pp
//
// This file is loaded and run after each Sources.pp is read.  It
// defines the inter-directory dependencies, which is useful for
// determining build order.
//

#if $[eq $[DIR_TYPE], toplevel]
  #define DEPENDABLE_HEADERS $[CONFIG_HEADER]

#elif $[or $[eq $[DIR_TYPE], src], $[eq $[DIR_TYPE], metalib]]
#if $[eq $[DEPEND_DIRS],]

  // DEPENDABLE_HEADERS is supposed to be the primary purpose of this
  // file: here we generate the list of source files that might be
  // included in some other source file, and hence is relevant to the
  // automatically-generated dependency chain.

  // We generate this variable by walking through all of the targets
  // and building it up.
  #define DEPENDABLE_HEADERS

  // We will also need to sort out the types files we will actually
  // compile within each directory, as well as the lists of files that
  // go into each composite.

  // We define $[c_sources], $[cxx_sources], $[yxx_sources], and
  // $[lxx_sources] within each target, which lists original files as
  // well as synthetic composite files.  There's also
  // $[compile_sources], which is the union of all the others: any
  // source files that are actually compiled and result in a generated
  // .o (or .obj) file.

  // Finally, we build up $[composite_list] out here to list all of
  // the composite files generated for all targets.

  // This is done at this point, within Depends.pp, so that the
  // various Template.*.pp files will be able to reliably access
  // $[compile_sources] from the different directories.
  #define composite_list
  // Tag all the static libraries by defining the "lib_is_static" variable.
  #if $[WINDOWS_PLATFORM]
    #forscopes static_lib_target ss_lib_target
      #define lib_is_static 1
    #end static_lib_target ss_lib_target
  #else
    #forscopes static_lib_target
      #define lib_is_static 1
    #end static_lib_target
  #endif

  #forscopes metalib_target lib_target noinst_lib_target test_lib_target static_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target
    // We can optimize quite a bit by evaluating now several of the key
    // deferred variables defined in Globals.pp.  This way they won't need
    // to get repeatedly reevaluated as each directory examines each
    // other.
    #define build_directory $[build_directory]
    #define build_target $[build_target]
    #define active_local_libs $[active_local_libs]
    #define active_component_libs $[active_component_libs]
    #define active_libs $[active_libs]
    #define get_sources $[get_sources]
    #define get_igatescan $[get_igatescan]
    #define get_igateoutput $[get_igateoutput]
    #define get_igatedb $[get_igatedb]
    #define get_igatemscan $[get_igatemscan]
    #define get_igatemout $[get_igatemout]

    // Report a warning for nonexisting dependencies.
    #define nonexisting $[unmapped all_libs,$[LOCAL_LIBS]]
    #if $[ne $[nonexisting],]
      #print Warning: Lib(s) $[nonexisting], referenced in $[DIRNAME]/$[TARGET], not found.
    #endif

    #set DEPENDABLE_HEADERS $[DEPENDABLE_HEADERS] $[filter %.h %.I %.T %_src.cxx,$[get_sources]] $[included_sources]

    // Now compute the source files.
    #define c_sources $[filter %.c,$[get_sources]]
    #define cxx_sources $[filter-out %_src.cxx,$[filter %.cxx %.cpp,$[get_sources]]]
    #define yxx_sources $[filter %.yxx,$[get_sources]]
    #define lxx_sources $[filter %.lxx,$[get_sources]]

    // Define what the object files are.
    #foreach file $[c_sources] $[cxx_sources] $[yxx_sources] $[lxx_sources]
      #define $[file]_obj $[patsubst %.c %.cxx %.cpp %.yxx %.lxx,$[ODIR]/$[obj_prefix]%$[OBJ],$[notdir $[file]]]
      #push 1 $[file]_obj
    #end file

    #if $[USE_SINGLE_COMPOSITE_SOURCEFILE]
      #if $[> $[words $[cxx_sources]], 1]
        // If we have multiple C++ files, put them together into one
        // composite file.
        #define composite_file $[ODIR]/$[TARGET]_composite.cxx
        #set composite_list $[composite_list] $[composite_file]
        #define $[composite_file]_sources $[cxx_sources]
        #define $[composite_file]_obj $[ODIR]/$[TARGET]_composite$[OBJ]
        #push 1 $[composite_file]_sources
        #push 1 $[composite_file]_obj
        #set cxx_sources $[composite_file]
      #endif
      #if $[> $[words $[c_sources]], 1]
        // If we have multiple C files, put them together into one
        // composite file also.
        #define composite_file $[ODIR]/$[TARGET]_composite_c.c
        #set composite_list $[composite_list] $[composite_file]
        #define $[composite_file]_sources $[c_sources]
        #define $[composite_file]_obj $[ODIR]/$[TARGET]_composite_c$[OBJ]
        #push 1 $[composite_file]_sources
        #push 1 $[composite_file]_obj
        #set c_sources $[composite_file]
      #endif
    #endif

    // Add the bison- and flex-generated .cxx files, as well as the
    // interrogate-generated files, to the compile list, too.  These
    // never get added to composite files, though, mainly because they
    // tend to be very large files themselves.
    #foreach source_file $[yxx_sources] $[lxx_sources]
      #define generated_file $[patsubst %.yxx %.lxx,%.cxx,$[source_file]]
      #define $[generated_file]_obj $[patsubst %.yxx %.lxx,$[ODIR]/$[TARGET]_%$[OBJ],$[source_file]]
      #define $[generated_file]_sources $[source_file]
      #push 1 $[generated_file]_obj
      #set cxx_sources $[cxx_sources] $[generated_file]
    #end source_file
    #if $[get_igateoutput]
      #define generated_file $[get_igateoutput]
      #define $[generated_file]_obj $[get_igateoutput:%.cxx=%$[OBJ]]
      #define $[generated_file]_sources $[get_igatescan]
      #push 1 $[generated_file]_obj
      #set cxx_sources $[cxx_sources] $[generated_file]
    #endif
    #if $[get_igatemout]
      #define generated_file $[get_igatemout]
      #define $[generated_file]_obj $[get_igatemout:%.cxx=%$[OBJ]]
      #define $[generated_file]_sources none
      #push 1 $[generated_file]_obj
      #set cxx_sources $[cxx_sources] $[generated_file]
    #endif

    #define compile_sources $[c_sources] $[cxx_sources]

  #end metalib_target lib_target noinst_lib_target test_lib_target static_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target

  // Allow the user to define additional EXTRA_DEPENDS targets in each
  // Sources.pp.
  #define DEPEND_DIRS \
    $[sort $[EXTRA_DEPENDS] $[all_libs $[DIRNAME],$[get_depend_libs]]]
  #set DEPENDABLE_HEADERS $[sort $[DEPENDABLE_HEADERS] $[EXTRA_DEPENDABLE_HEADERS]]
#endif

#endif // DIR_TYPE
