#define INSTALL_CONFIG \
  Configrc.direct

#if $[CTPROJS]
  // These files only matter to ctattach users.
  #define INSTALL_CONFIG $[INSTALL_CONFIG] direct.init
#endif

