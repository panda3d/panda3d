#define INSTALL_CONFIG \
  _direct.prc

#if $[CTPROJS]
  // These files only matter to ctattach users.
  #define INSTALL_CONFIG $[INSTALL_CONFIG] direct.init
#endif

