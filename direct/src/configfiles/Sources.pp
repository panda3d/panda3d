#define INSTALL_CONFIG \
  70_direct.prc

#if $[CTPROJS]
  // These files only matter to ctattach users.
  #define INSTALL_CONFIG $[INSTALL_CONFIG] direct.init
#endif


#include $[THISDIRPREFIX]_direct.prc.pp
