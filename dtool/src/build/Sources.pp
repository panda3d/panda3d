// This directory defines the ctattach tools, which are completely
// undocumented and are only intended for use by the VR Studio as a
// convenient way to manage development by multiple people within the
// various Panda source trees.  These tools are not recommended for
// use by the rest of the world; it's probably not worth the headache
// of learning how to set them up.

// Therefore, we only install the stuff in this directory if the
// builder is already using the ctattach tools.  Otherwise, it's safe
// to assume s/he doesn't need the ctattach tools.

#define BUILD_DIRECTORY $[CTPROJS]
#if $[CTPROJS]
  #define INSTALL_HEADERS \
    ctinstmake.pl ctproj.pl ctutils.pl

  #define INSTALL_SCRIPTS \
    ctaddpkg ctaddtgt ctinitproj ctproj ctpathadjust

  #define EXTRA_DIST \
    initialize
#endif
