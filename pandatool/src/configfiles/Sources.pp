
#define INSTALL_CONFIG \
  72_pandatool.prc

#if $[CTPROJS]
  // These files only matter to ctattach users.
  #define INSTALL_CONFIG $[INSTALL_CONFIG] pandatool.init
#endif

 
#include $[THISDIRPREFIX]_pandatool.prc.pp
