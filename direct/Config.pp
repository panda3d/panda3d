//
// Config.pp
//
// This file defines certain configuration variables that are written
// into the various make scripts.  It is processed by ppremake (along
// with the Sources.pp files in each of the various directories) to
// generate build scripts appropriate to each environment.
//
// There are not too many variables to declare at this level; most of
// them are defined in the DTOOL-specific Config.pp.


// Where should we find PANDA?  This will come from the environment
// variable if it is set.
#if $[eq $[PANDA],]
  #define PANDA /usr/local/panda
#endif

