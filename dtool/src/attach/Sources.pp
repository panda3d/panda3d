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
    ctattch.pl ctattch.pl.rnd ctunattach.pl ctquery.pl ctdelta.pl \
    ctdelta.pl.rnd unco.pl ctvspec.pl ctcm.pl ctccase.pl ctntool.pl \
    ctcvs.pl ctproj.pl ctutils.pl

  #define INSTALL_SCRIPTS \
    ctattach.drv ctunattach.drv ctquery ctdelta ctihave ctallihave \
    ctunco ctattachcc cttimewarp get-cttree update-cttree get-delta \
    ctsanity ctmkelem ctmkdir ctci ctco ctrm ctmv ctmake neartool

  #define INSTALL_CONFIG \
    dtool.cshrc dtool.init dtool.emacs dtool.alias \
    dtool.sh dtool.alias-sh
#endif
