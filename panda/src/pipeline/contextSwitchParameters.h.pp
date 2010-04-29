// This file is read and processed by ppremake to generate
// contextSwitchParameters.h, which is #included by configPageManager.cxx.
// This mechanism is used, rather than just putting the parameters in
// dtool_config.h, to (a) help keep the prc encryption key from
// getting spread around to too many places, and (b) minimize the need
// to rebuild the whole world just because you changed some low-level
// prc parameters.

#output contextSwitchParameters.h notouch
/* contextSwitchParameters.h.  Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[notdir $[THISFILENAME]]. */
/********************************** DO NOT EDIT ****************************/


/* Define if SIMPLE_THREADS should be implemented with the OS-provided
   threading layer (if available). */
$[cdefine OS_SIMPLE_THREADS]

#end contextSwitchParameters.h
