#if $[and $[CTPROJS],$[WINDOWS_PLATFORM]]
  // This script is only useful if you're using the ctattach script on
  // Windows; therefore, we only bother to install it if you're using
  // the cattach scripts.
  #define INSTALL_SCRIPTS runPythonEmacs

#endif

