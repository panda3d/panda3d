#define INSTALL_CONFIG \
  40_direct.prc

#if $[CTPROJS]
  // These files only matter to ctattach users.
  #define INSTALL_CONFIG $[INSTALL_CONFIG] direct.init
#endif


#include $[THISDIRPREFIX]direct.prc.pp
