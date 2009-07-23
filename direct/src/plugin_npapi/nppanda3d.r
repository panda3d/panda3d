/* Apparently Firefox ignores the Info.plist file,
   and looks only in the .rsrc file within the bundle,
   which is compiled from the following source file. */

#include <Carbon/Carbon.r>

resource 'STR#' (126) {
	{ "Runs 3-D games and interactive applets", 
          "Panda3D Game Engine Plug-In" }
};

resource 'STR#' (127) {
	{ "Panda3D applet" }
};

resource 'STR#' (128) {
	{ "application/x-panda3d", "p3d" }
};

