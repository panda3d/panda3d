// This file is read and processed by ppremake to generate
// speedtree_parameters.h, which is #included by speedtree_api.h.

#output speedtree_parameters.h notouch
/* speedtree_parameters.h.  Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[notdir $[THISFILENAME]]. */
/********************************** DO NOT EDIT ****************************/

/* We need to define the appropriate macro to tell the SpeedTree
   headers which API we intend to use.  This should be one of
   SPEEDTREE_OPENGL or SPEEDTREE_DIRECTX9 (or, when Panda supports it,
   SPEEDTREE_DIRECTX10). */
# define SPEEDTREE_$[upcase $[SPEEDTREE_API]]

/* The default directory in which to search for SpeedTree's provided
   shaders and terrain files. */
# define SPEEDTREE_BIN_DIR "$[SPEEDTREE_BIN_DIR]"

#end speedtree_parameters.h
