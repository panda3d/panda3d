
#define INSTALL_CONFIG \
  _pandatool.prc

#if $[CTPROJS]
  // These files only matter to ctattach users.
  #define INSTALL_CONFIG $[INSTALL_CONFIG] pandatool.init
#endif
