// Arguably, this directory doesn't really belong in direct at all,
// since it's so very Toontown-specific.  The scripts here certainly
// won't be useful to people outside the VR Studio.

// In the short term, we'll just disable the installation of this
// directory unless you have the ctattach tools in effect (which
// generally indicates that you're a member of the VR Studio).

#define BUILD_DIRECTORY $[CTPROJS]

// Install scripts for building zipfiles (leveleditor and RobotToonManager)
#if $[CTPROJS]
  #define INSTALL_SCRIPTS printdir printlib copyfiles zipfiles
#endif

