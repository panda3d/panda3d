
#define INSTALL_CONFIG \
  panda.emacs panda.emacs.Xdefaults Configrc.panda


#if $[CTPROJS]
  // These files only matter to ctattach users.
  #define INSTALL_CONFIG $[INSTALL_CONFIG] panda.init
#endif

