
#define INSTALL_CONFIG \
  30_pandatool.prc

#if $[CTPROJS]
  // These files only matter to ctattach users.
  #define INSTALL_CONFIG $[INSTALL_CONFIG] pandatool.init
#endif

 
#include $[THISDIRPREFIX]pandatool.prc.pp
