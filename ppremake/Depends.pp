//
// Depends.pp
//
// This file is loaded and run after each Sources.pp is read.  It
// defines the inter-directory dependencies, which is useful for
// determining build order.
//

#if $[eq $[DIR_TYPE], src]

#if $[eq $[DEPENDS],]
  #map local_libs TARGET(*/lib_target */noinst_lib_target)

  // Allow the user to define additional DEPENDS targets in each
  // Sources.pp.
  #define DEPENDS
  #set DEPENDS $[EXTRA_DEPENDS]

  #forscopes lib_target bin_target noinst_bin_target
    #set DEPENDS $[DEPENDS] $[local_libs $[DIRNAME],$[LOCAL_LIBS]] $[LOCAL_INCS]
  #end lib_target bin_target noinst_bin_target

  #set DEPENDS $[sort $[DEPENDS]]
#endif

#endif // DIR_TYPE
