
#define INSTALL_CONFIG \
  panda.emacs panda.emacs.Xdefaults _panda.prc


#if $[CTPROJS]
  // These files only matter to ctattach users.
  #define INSTALL_CONFIG $[INSTALL_CONFIG] panda.init
#endif


#include $[THISDIRPREFIX]_panda.prc.pp
